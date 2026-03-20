#pragma once

// r65emu — subset para PET 2001 VGA32
// Elimina: i8080, z80, uz80, ACIA, RIOT, SpiRAM,
//          socket_filer, serial_filer, serial_kbd,
//          ps2_serial_kbd, hw_serial_kbd, hw_serial_dsp

#include <functional>
#include "hardware.h"
#include "machine.h"
#include "arduinomachine.h"
#include "memory.h"
#include "CPU.h"
#include "ram.h"
#include "prom.h"
#include "display.h"
#include "serialio.h"
#include "filer.h"
#include "flash_filer.h"
#include "sound_dac.h"
#include "ps2_raw_kbd.h"
#include "debugging.h"
