#pragma once

// Pantalla
#define BG_COLOUR       BLACK
#define FG_COLOUR       GREEN
#define CHARS_PER_LINE  40
#define SCREEN_LINES    25
#define SCREEN_RAM_SIZE 0x0400

// portrait = rotation 0, Adafruit_GFX ve 400x300 — correcto para MODE400x300
// landscape = rotation 1, Adafruit_GFX hace swap → 300x400 — incorrecto
#define ORIENT          portrait

// Sin SPIRAM, sin SD
#define PROGRAMS        "/"
// RAM máxima: 32 páginas × 1 KB = 32 KB  (ram<> = ram<1024>)
#define RAM_PAGES_MAX   32

// ROM set: 1 = PET 2001 (BASIC 1, 7×2KB), 2 = PET 2001N (BASIC 2, 4×ROM)
#if !defined(ROM_SERIES)
#define ROM_SERIES      1
#endif

// Charset
#if !defined(CHARSET_ROM)
#define CHARSET_ROM     "roms/characters2.h"
#endif
