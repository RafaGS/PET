#include <Arduino.h>
#include <stdint.h>

#include "inc/machine.h"
#include "inc/memory.h"
#include "inc/display.h"
#include "inc/hardware.h"

#include "config.h"
#include "screen.h"
#include CHARSET_ROM

#if defined(USE_VGA) && VGA_BIT_DEPTH == 1
#include "vga_instance.h"
#endif

// VGA1BitI con BLpx8sz8swyshy:
// - 8 pixels verticales por byte (eje Y empaquetado)
// - backBuffer[y/8][x] contiene los pixels (x, y..y+7)
// - Un carácter 8x8 = 8 bytes consecutivos en X, cada byte = columna vertical
// Escribimos cada carácter directamente en el framebuffer para evitar
// el problema de las rayas que ocurre con dot() pixel a pixel

static struct resolution {
    const char *name;
    const unsigned cw, ch;
} resolutions[] = {
    {"40x25", 8, 8},
};

void screen::begin()
{
    struct resolution &r = resolutions[_resolution];
    Display::begin(BG_COLOUR, FG_COLOUR, ORIENT, CHARS_PER_LINE*r.cw, SCREEN_LINES*r.ch, CENTER_NONE);
    clear();
    memset(_mem, 0, sizeof(_mem));
}

void screen::_set(Memory::address a, uint8_t c)
{
    if (a >= CHARS_PER_LINE * SCREEN_LINES)
        return;

    uint8_t cm = _mem[a];
    if (c == cm)
        return;

    // Log solo caracteres visibles (no espacio)
    if (c != 0x20 && c != 0x00) {
        static int n = 0;
        if (n++ < 20) Serial.printf("SET[%03x]=%02x\n", a, c);
    }

    struct resolution &r = resolutions[_resolution];
    unsigned x = r.cw * (a % CHARS_PER_LINE);
    unsigned y = r.ch * (a / CHARS_PER_LINE);

    uint8_t ch = (c & 0x7f);
    bool invert = (c & 0x80);
    if (_upr) ch |= 0x80;

#if defined(USE_VGA) && VGA_BIT_DEPTH == 1
    // Escritura directa al framebuffer: cada columna de 8 pixels = 1 byte
    // Los 8 bytes del charset para este char son 8 filas horizontales,
    // pero el buffer quiere 8 columnas verticales — hay que transponer
    // charset[ch][j] = fila j del carácter (bit 7=col0 ... bit0=col7)
    // buffer[y/8][x+col] = columna col del carácter (bit7=row0 ... bit0=row7)
    
    uint8_t cols[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int row = 0; row < 8; row++) {
        uint8_t rowbits = pgm_read_byte(&charset[ch][row]);
        if (invert) rowbits = ~rowbits;
        // Distribuir cada bit de esta fila en la columna correspondiente
        for (int col = 0; col < 8; col++) {
            if (rowbits & (0x80 >> col))
                cols[col] |= (0x80 >> row);
        }
    }
    int by_row = y / 8;  // fila del buffer (grupos de 8 pixels Y)
    if (!vga.backBuffer || !vga.backBuffer[by_row]) {
        Serial.printf("NULL backBuffer at a=%03x by_row=%d\n", a, by_row);
        return;
    }
    static int fb_log = 0;
    if (fb_log < 3) {
        Serial.printf("FB: ch=%02x x=%d y=%d by_row=%d cols=%02x%02x%02x%02x%02x%02x%02x%02x\n",
            ch, x, y, by_row, cols[0],cols[1],cols[2],cols[3],cols[4],cols[5],cols[6],cols[7]);
        fb_log++;
    }
    for (int col = 0; col < 8; col++)
        vga.backBuffer[by_row][x + col] = cols[col];
#else
    // Fallback para otros modos VGA: drawPixel pixel a pixel
    uint8_t cm_ch = (_mem[a] & 0x7f);
    if (_upr) cm_ch |= 0x80;
    bool inverted = (_mem[a] & 0x80);

    for (unsigned j = 0; j < r.ch; j++) {
        uint8_t b = pgm_read_byte(&charset[ch][j]);
        if (invert) b = ~b;
        uint8_t m = pgm_read_byte(&charset[cm_ch][j]);
        if (inverted) m = ~m;
        if (b != m) {
            uint8_t d = (b ^ m);
            if (d & 1)   drawPixel(x + 7, y + j, (b & 1)?   FG_COLOUR: BG_COLOUR);
            if (d & 2)   drawPixel(x + 6, y + j, (b & 2)?   FG_COLOUR: BG_COLOUR);
            if (d & 4)   drawPixel(x + 5, y + j, (b & 4)?   FG_COLOUR: BG_COLOUR);
            if (d & 8)   drawPixel(x + 4, y + j, (b & 8)?   FG_COLOUR: BG_COLOUR);
            if (d & 16)  drawPixel(x + 3, y + j, (b & 16)?  FG_COLOUR: BG_COLOUR);
            if (d & 32)  drawPixel(x + 2, y + j, (b & 32)?  FG_COLOUR: BG_COLOUR);
            if (d & 64)  drawPixel(x + 1, y + j, (b & 64)?  FG_COLOUR: BG_COLOUR);
            if (d & 128) drawPixel(x + 0, y + j, (b & 128)? FG_COLOUR: BG_COLOUR);
        }
    }
#endif
    _mem[a] = c;
}

void screen::checkpoint(Checkpoint &s)
{
    s.write(_resolution);
    s.write(_mem, sizeof(_mem));
}

void screen::restore(Checkpoint &s)
{
    _resolution = s.read();
    for (Memory::address p = 0; p < sizeof(_mem); p += Memory::page_size) {
        uint8_t buf[Memory::page_size];
        s.read(buf, sizeof(buf));
        for (unsigned i = 0; i < Memory::page_size; i++)
            _set(p + i, buf[i]);
    }
}
