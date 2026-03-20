#include <Arduino.h>
#include <stdint.h>

#include "inc/machine.h"
#include "inc/memory.h"
#include "inc/serialio.h"
#include "inc/filer.h"
#include "inc/pia.h"
#include "inc/via.h"
#include "inc/debugging.h"

#include "petio.h"
#include "sound.h"

// Refs: http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/PET-Interfaces.txt
//       http://www.6502.org/users/andre/petindex/progmod.html

// Offsets desde base $E800
#define PIA1_OFFSET     0x0010
#define PIA2_OFFSET     0x0020
#define VIA_OFFSET      0x0040

// VIA port-B bit 5: señal de retrace de vídeo
#define VIDEO_RETRACE   0x20

// Interrupción de sistema a 50Hz: un tick cada 1ms, 20 ticks = 20ms = 50Hz
#define SYS_TICKS       20
#define TICK_PERIOD     1000

sound sound;

petio::petio(filer &files): _ticks(0), _timer(-1), files(files) {
    put(pia1, PIA1_OFFSET);
    put(pia2, PIA2_OFFSET);
    put(via,  VIA_OFFSET);
}

void petio::reset() {

    sound.reset();
    pia1.reset();
    pia2.reset();
    via.reset();

    _ticks = 0;
    // Estado inicial seguro tras reset: retrace en LOW e IRQ de teclado inactiva.
    pia1.write_cb1(false);
    via.write_portb_in_bit(VIDEO_RETRACE, false);

    // Reiniciar siempre el temporizador para evitar IDs obsoletos tras reconfiguración.
    if (_timer >= 0) {
        _machine->cancel_timer(_timer);
        _timer = -1;
    }

    _timer = _machine->oneshot_timer(TICK_PERIOD, [this]() { tick(); });
}

void petio::tick() {

    DBG_EMU("tick: %d", _ticks);

    if (_ticks++ == SYS_TICKS) {
        _ticks = 0;
        pia1.write_cb1(true);
        via.write_portb_in_bit(VIDEO_RETRACE, true);
    } else {
        pia1.write_cb1(false);
        via.write_portb_in_bit(VIDEO_RETRACE, false);
    }

    _timer = _machine->oneshot_timer(TICK_PERIOD, [this]() { tick(); });
}

void petio::operator=(uint8_t r) {

    Devices::operator=(r);

    // Sonido via VIA T2/SR/ACR
    switch (_acc - VIA_OFFSET) {
    case 0x08:
        sound.frequency(r);
        break;
    case 0x0a:
        sound.octave(r);
        break;
    case 0x0b:
        sound.on_off((r & VIA::ACR_SHIFT_MASK) == VIA::ACR_SO_T2_RATE);
        break;
    }
}

bool petio::load_prg()
{
    // Sin SD: no hay ficheros que cargar
    return false;
}
