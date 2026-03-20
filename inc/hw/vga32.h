#pragma once

// LilyGO / TTGO VGA32 v1.x — Bitluni ESP32Lib VGA1BitI

#define USE_VGA
#define VGA_BIT_DEPTH       3
#define VGA_RESOLUTION      MODE400x300

// RAM emulada por defecto: 4KB.
#define RAM_SIZE            0x1000

// VGA32 TTGO/LilyGO confirmado:
// R: 21,22  G: 18,19  B: 4,5  HSYNC: 23  VSYNC: 15
// En este modo 1-bit se usan las lineas seleccionadas por el proyecto.
#define R0      22
#define G0      19
#define B0      5
#define HSYNC   23
#define VSYNC   15

// PS/2 teclado
#define USE_PS2_KBD
#define PS2_KBD_DATA    32
#define PS2_KBD_IRQ     33

// Sin SD — solo BASIC prompt
#define SIMPLE_TIMER_MICROS
#define DEBUGGING   DEBUG_NONE
