#pragma once

// Sonido del PET via VIA T2/SR/ACR
// Sin PWM_SOUND definido en hw/vga32.h, las funciones compilan pero no emiten nada.
// Para activar sonido: descomentar #define PWM_SOUND en hw/vga32.h y conectar altavoz.

class sound {
public:
    sound(): _octave(0), _freq(0) {}

    void on_off(bool o) { if (o) on(); else off(); }
    void frequency(uint8_t);
    void octave(uint8_t);
    void reset() { off(); _freq = 0; _octave = 0; }

private:
    void on();
    void off();

    uint8_t  _octave;
    uint32_t _freq;
};
