#include <Arduino.h>
#include "SimpleTimer.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "machine.h"
#include "memory.h"
#include "debugging.h"
#include "hardware.h"
#include "arduinomachine.h"
#include "CPU.h"

// Sin SD, sin SPIFFS, sin SPIRAM para el PET VGA32

static SimpleTimer timers;

bool Arduino::reset() {

    DBG_INI("machine reset");

    _cpu.reset();
    if (_reset_handler) _reset_handler(false);  // false = sin SD, es correcto
    return false;
}

void Arduino::begin() {

    Serial.begin(115200);
    delay(500);  // dar tiempo al USB-UART a conectarse

    _halted_handler = [this]() {
        ERR("CPU halted at %04x", _cpu.pc());
        for (;;) yield();
    };

    DBG_INI("machine init");

#if defined(PWM_SOUND) && PWM_SOUND >= 0
    pinMode(PWM_SOUND, OUTPUT);
#endif
}

#define MAX_POLLABLE 5
static Pollable *devices[MAX_POLLABLE];
static uint8_t num_pollable = 0;

void Arduino::register_pollable(Pollable &p) {
    devices[num_pollable++] = &p;
}

void Arduino::run(unsigned instructions) {

    timers.run();

    for (uint8_t i = 0; i < num_pollable; i++)
        devices[i]->poll();

    if (instructions > 0) {
#if DEBUGGING & DEBUG_CPU
        if (_debug_handler()) {
            char buf[256];
            DBG_CPU(_cpu.status(buf, sizeof(buf)));
        }
        _cpu.run(1);
#else
        _cpu.run(instructions);
#endif
    }

    if (_cpu.halted())
        _halted_handler();
}

int Arduino::interval_timer(uint32_t interval, std::function<void(void)> cb) {
    return timers.setInterval(interval, cb);
}

int Arduino::oneshot_timer(uint32_t interval, std::function<void(void)> cb) {
    return timers.setTimeout(interval, cb);
}

void Arduino::cancel_timer(int timer) {
    timers.deleteTimer(timer);
}

uint32_t Arduino::microseconds() { return micros(); }

void Arduino::debug(const char *lvlstr, const char *fmt, ...) {
#if DEBUGGING != DEBUG_NONE
    char buf[128];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n >= 0) {
        buf[sizeof(buf)-1] = 0;
        Serial.print(lvlstr);
        Serial.print('\t');
        Serial.println(buf);
    }
#endif
}
