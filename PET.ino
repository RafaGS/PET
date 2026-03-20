#include <stdarg.h>
#include <SPI.h>

// ESP32Video incluido ANTES de r65emu para evitar colisión de macros A/C/D/E/O
// con Graphics.h de bitluni — aquí aún no están definidas las macros de CPU.h
#include <ESP32Video.h>
#include "vga_instance.h"

// r65emu y dependencias — todo local en inc/
#include "inc/r65emu.h"
#include "inc/r6502.h"
#include "inc/pia.h"
#include "inc/via.h"

// Arduino IDE no compila .cpp en subdirectorios — incluir explícitamente
// PS/2 — implementación nativa ESP32 (sin librería PS2KeyRaw externa)
#include "inc/PS2KeyRaw.h"
#include "inc/SimpleTimer.cpp"
#include "inc/machine.cpp"
#include "inc/arduinomachine.cpp"
#include "inc/memory.cpp"
#include "inc/r6502.cpp"
#include "inc/pia.cpp"
#include "inc/via.cpp"
#include "inc/display.cpp"
#include "inc/ps2_raw_kbd.cpp"
#include "inc/flash_filer.cpp"
#include "inc/sound_dac.cpp"
#include "inc/spiram.cpp"

#include "config.h"
#include "screen.h"
#include "kbd.h"
#include "petio.h"

// ROMs serie 1 — 7 × 2 KB (PET 2001 original)
#include "roms/basic1_c000.h"
#include "roms/basic1_c800.h"
#include "roms/basic1_d000.h"
#include "roms/basic1_d800.h"
#include "roms/basic1_e000.h"
#include "roms/basic1_f000.h"
#include "roms/basic1_f800.h"
prom s1_c000(rom1_c000, 2048);
prom s1_c800(rom1_c800, 2048);
prom s1_d000(rom1_d000, 2048);
prom s1_d800(rom1_d800, 2048);
prom s1_e000(rom1_e000, 2048);
prom s1_f000(rom1_f000, 2048);
prom s1_f800(rom1_f800, 2048);

// ROMs serie 2 — 4 × ROM (PET 2001N / BASIC 2)
#include "roms/basic2_c000.h"
#include "roms/basic2_d000.h"
#include "roms/kernal2.h"
#include "roms/edit2.h"
prom s2_c000(basic2_c000, 4096);
prom s2_d000(basic2_d000, 4096);
prom s2_e000(edit2,       2048);
prom s2_f000(kernal2,     4096);

// RAM máxima: 8 páginas × 4 KB = 32 KB
ram<> pages[RAM_PAGES_MAX];
// Null device para limpiar el rango RAM antes de remapear (devuelve 0x00 → BASIC detecta sin RAM)
static Memory::Null nullram(0x8000);
flash_filer files(PROGRAMS);
petio io(files);
kbd keyboard;
ps2_raw_kbd ps2(keyboard);
screen display;
Memory memory;
r6502 cpu(memory);
Arduino machine(cpu);

static uint8_t current_series = ROM_SERIES;
static uint8_t current_ram_kb = RAM_SIZE / 1024;
static volatile bool cfg_pending = false;
static volatile uint8_t pending_series = ROM_SERIES;
static volatile uint8_t pending_ram_kb = RAM_SIZE / 1024;

static void clear_all_ram() {
    for (unsigned p = 0; p < RAM_PAGES_MAX; p++) {
        for (unsigned o = 0; o < ram<>::page_size; o++)
            pages[p].set((Memory::address)o, 0x00);
    }
}

static void map_roms_and_ram(uint8_t series, uint8_t ram_kb) {
    // Borrar zona RAM (0x0000-0x7FFF) con null device (lee 0x00 → BASIC detecta sin RAM)
    memory.put(nullram, 0x0000);
    // ram<> = ram<1024>: cada página es 1 KB; paso entre páginas = ram<>::page_size
    unsigned n = (ram_kb * 1024u) / ram<>::page_size;
    if (n > RAM_PAGES_MAX) n = RAM_PAGES_MAX;
    for (unsigned i = 0; i < n; i++)
        memory.put(pages[i], i * ram<>::page_size);
    if (series == 1) {
        memory.put(s1_c000, 0xc000);
        memory.put(s1_c800, 0xc800);
        memory.put(s1_d000, 0xd000);
        memory.put(s1_d800, 0xd800);
        memory.put(s1_e000, 0xe000);
        memory.put(s1_f000, 0xf000);
        memory.put(s1_f800, 0xf800);
    } else {
        memory.put(s2_c000, 0xc000);
        memory.put(s2_d000, 0xd000);
        memory.put(s2_e000, 0xe000);
        memory.put(s2_f000, 0xf000);
    }
}

static void configure(uint8_t series, uint8_t ram_kb) {
    current_series = series;
    current_ram_kb = ram_kb;
    // Cambiar perfil ROM/RAM como arranque en frio para evitar estado residual.
    clear_all_ram();
    map_roms_and_ram(series, ram_kb);
    machine.reset();
}

static void request_config(uint8_t series, uint8_t ram_kb) {
    pending_series = series;
    pending_ram_kb = ram_kb;
    cfg_pending = true;
}

static void reset(bool sd) {
    io.reset();
    ps2.reset();
    display.begin();
}

static void function_keys(uint8_t key) {
    if (keyboard.super_held()) {
        // Super+F1..F5: pedir cambio de configuración (se aplica en loop)
        switch (key) {
        case 1: request_config(1,  4); break;  // Super+F1: ROM1  4 KB
        case 2: request_config(1,  8); break;  // Super+F2: ROM1  8 KB
        case 3: request_config(2,  8); break;  // Super+F3: ROM2  8 KB
        case 4: request_config(2, 16); break;  // Super+F4: ROM2 16 KB
        case 5: request_config(2, 32); break;  // Super+F5: ROM2 32 KB
        }
    } else {
        switch (key) {
        case 1:  machine.reset();     break;
        case 10: machine.debug_cpu(); break;
        }
    }
}

static void interrupt(bool irq) { if (irq) cpu.raise(0); }

void setup() {

    machine.begin();

    // RAM + ROMs (configuración de arranque)
    clear_all_ram();
    map_roms_and_ram(current_series, current_ram_kb);

    // VRAM: $8000–$83FF
    memory.put(display, 0x8000);

    // I/O: PIA1, PIA2, VIA en $E800
    memory.put(io, 0xe800);

    // PIA1: teclado
    io.pia1.register_porta_write_handler([](uint8_t b) { keyboard.write(b & 0x0f); });
    io.pia1.register_porta_read_handler([]() -> uint8_t { return keyboard.row() | 0x80; });
    io.pia1.register_portb_read_handler([]() -> uint8_t { return keyboard.read(); });
    io.pia1.register_irqa_handler(interrupt);
    io.pia1.register_irqb_handler(interrupt);

    // PIA2: sin periféricos IEEE/cassette conectados.
    // En hardware real estas líneas suelen quedar en pull-up, no en 0x00.
    io.pia2.register_porta_read_handler([]() -> uint8_t { return 0xff; });
    io.pia2.register_portb_read_handler([]() -> uint8_t { return 0xff; });

    // VIA: retrace + charset upper/lower
    io.via.register_irq_handler(interrupt);
    io.via.register_ca2_handler([](bool ca2) { display.set_upper(ca2); });

    // Teclado PS/2
    ps2.register_fnkey_handler(function_keys);
    machine.register_pollable(ps2);

    machine.register_reset_handler(reset);
    machine.reset();
}

void loop() {
    if (cfg_pending) {
        uint8_t s = pending_series;
        uint8_t r = pending_ram_kb;
        cfg_pending = false;

        if (s != current_series || r != current_ram_kb)
            configure(s, r);
    }

    machine.run();
}
