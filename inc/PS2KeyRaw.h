#pragma once

// PS2KeyRaw — implementación bit-bang para ESP32
// ISR como función libre estática (evita el error de l32r relocation
// que ocurre cuando IRAM_ATTR se aplica a métodos de clase en ESP32)

#include <Arduino.h>

// Estado global de la ISR — fuera de la clase para que el linker
// pueda colocar el literal correctamente en IRAM
static volatile uint8_t _ps2_data_pin;
static volatile uint8_t _ps2_buf[32];
static volatile uint8_t _ps2_head;
static volatile uint8_t _ps2_tail;
static volatile uint8_t _ps2_bitcount;
static volatile uint8_t _ps2_shift_reg;

static void IRAM_ATTR _ps2_isr() {
    uint8_t bit = digitalRead(_ps2_data_pin);
    _ps2_bitcount++;

    // PS/2 frame: bitcount 1=START, 2-9=DATA(D0..D7), 10=PARITY, 11=STOP
    // Skip START bit (always 0); capture only the 8 data bits.
    if (_ps2_bitcount >= 2 && _ps2_bitcount <= 9) {
        _ps2_shift_reg >>= 1;
        if (bit) _ps2_shift_reg |= 0x80;
    } else if (_ps2_bitcount == 11) {
        uint8_t next = (_ps2_head + 1) & 0x1f;
        if (next != _ps2_tail) {
            _ps2_buf[_ps2_head] = _ps2_shift_reg;
            _ps2_head = next;
        }
        _ps2_bitcount  = 0;
        _ps2_shift_reg = 0;
    }
    // bit 9 = paridad, bit 10 = stop — se ignoran
}

class PS2KeyRaw {
public:
    PS2KeyRaw() {}

    void begin(uint8_t data_pin, uint8_t clk_pin) {
        _ps2_data_pin  = data_pin;
        _ps2_head      = 0;
        _ps2_tail      = 0;
        _ps2_bitcount  = 0;
        _ps2_shift_reg = 0;

        pinMode(clk_pin,  INPUT_PULLUP);
        pinMode(data_pin, INPUT_PULLUP);

        attachInterrupt(digitalPinToInterrupt(clk_pin), _ps2_isr, FALLING);
    }

    bool available() {
        return _ps2_head != _ps2_tail;
    }

    int read() {
        if (_ps2_head == _ps2_tail) return -1;
        uint8_t b = _ps2_buf[_ps2_tail];
        _ps2_tail = (_ps2_tail + 1) & 0x1f;
        return b;
    }
};
