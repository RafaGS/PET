#include "SimpleTimer.h"

#if defined(SIMPLE_TIMER_MICROS)
#define TIMER_NOW() micros()
#else
#define TIMER_NOW() millis()
#endif

SimpleTimer::SimpleTimer() {
    numTimers = 0;
    for (int i = 0; i < SIMPLETIMER_MAX_TIMERS; i++) {
        timer[i].callback    = nullptr;
        timer[i].enabled     = false;
        timer[i].numRuns     = 0;
        timer[i].maxNumRuns  = RUN_FOREVER;
        timer[i].toBeCalled  = 0;
        timer[i].delay       = 0;
        timer[i].prev_millis = 0;
    }
}

int SimpleTimer::findFirstFreeSlot() {
    for (int i = 0; i < SIMPLETIMER_MAX_TIMERS; i++)
        if (!timer[i].callback)
            return i;
    return SIMPLETIMER_INVALID_ID;
}

int SimpleTimer::setupTimer(unsigned long d, std::function<void()> f, int n) {
    int id = findFirstFreeSlot();
    if (id < 0) return SIMPLETIMER_INVALID_ID;
    timer[id].delay       = d;
    timer[id].callback    = f;
    timer[id].maxNumRuns  = n;
    timer[id].numRuns     = 0;
    timer[id].enabled     = true;
    timer[id].toBeCalled  = 0;
    timer[id].prev_millis = TIMER_NOW();
    numTimers++;
    return id;
}

int SimpleTimer::setTimeout(unsigned long delay, std::function<void()> f) {
    return setupTimer(delay, f, RUN_ONCE);
}

int SimpleTimer::setInterval(unsigned long period, std::function<void()> f) {
    return setupTimer(period, f, RUN_FOREVER);
}

void SimpleTimer::deleteTimer(int id) {
    if (id < 0 || id >= SIMPLETIMER_MAX_TIMERS) return;
    if (!timer[id].callback) return;
    timer[id].callback   = nullptr;
    timer[id].enabled    = false;
    timer[id].numRuns    = 0;
    timer[id].maxNumRuns = RUN_FOREVER;
    timer[id].toBeCalled = 0;
    timer[id].delay      = 0;
    numTimers--;
}

void SimpleTimer::enable(int id)    { if (id >= 0 && id < SIMPLETIMER_MAX_TIMERS) timer[id].enabled = true; }
void SimpleTimer::disable(int id)   { if (id >= 0 && id < SIMPLETIMER_MAX_TIMERS) timer[id].enabled = false; }
bool SimpleTimer::isEnabled(int id) { return (id >= 0 && id < SIMPLETIMER_MAX_TIMERS) && timer[id].enabled; }
void SimpleTimer::restartTimer(int id) { if (id >= 0 && id < SIMPLETIMER_MAX_TIMERS) timer[id].prev_millis = TIMER_NOW(); }
int  SimpleTimer::getNumTimers()    { return numTimers; }

void SimpleTimer::run() {
    unsigned long now = TIMER_NOW();
    for (int i = 0; i < SIMPLETIMER_MAX_TIMERS; i++) {
        timer[i].toBeCalled = 0;
        if (!timer[i].callback || !timer[i].enabled) continue;
        if ((now - timer[i].prev_millis) >= timer[i].delay) {
            timer[i].prev_millis = now;
            timer[i].toBeCalled  = 1;
        }
    }
    for (int i = 0; i < SIMPLETIMER_MAX_TIMERS; i++) {
        if (timer[i].toBeCalled == 1) {
            timer[i].toBeCalled = 0;
            timer[i].callback();
            if (timer[i].maxNumRuns == RUN_ONCE)
                deleteTimer(i);
        }
    }
}
