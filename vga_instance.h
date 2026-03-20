#pragma once

// Acceso compartido al objeto vga definido en inc/display.cpp
// Este header incluye ESP32Video.h si es necesario

#if defined(USE_VGA)
#pragma push_macro("A")
#pragma push_macro("C")
#pragma push_macro("D")
#pragma push_macro("E")
#pragma push_macro("O")
#undef A
#undef C
#undef D
#undef E
#undef O
#include <ESP32Video.h>
#pragma pop_macro("O")
#pragma pop_macro("E")
#pragma pop_macro("D")
#pragma pop_macro("C")
#pragma pop_macro("A")

#if VGA_BIT_DEPTH == 1
extern VGA1BitI vga;
#elif VGA_BIT_DEPTH == 3
extern VGA3Bit vga;
#elif VGA_BIT_DEPTH == 6
extern VGA6Bit vga;
#endif
#endif
