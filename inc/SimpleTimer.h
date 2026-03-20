#pragma once

#include <Arduino.h>
#include <functional>

#define SIMPLETIMER_MAX_TIMERS  10
#define SIMPLETIMER_INVALID_ID  -1

class SimpleTimer {
public:
    SimpleTimer();

    void run();
    int  setTimeout(unsigned long delay, std::function<void()> f);
    int  setInterval(unsigned long period, std::function<void()> f);
    void deleteTimer(int id);
    bool isEnabled(int id);
    void enable(int id);
    void disable(int id);
    void restartTimer(int id);
    int  getNumTimers();

private:
    static const int RUN_FOREVER = 0;
    static const int RUN_ONCE    = 1;

    struct timer_t {
        unsigned long prev_millis;
        std::function<void()> callback;
        unsigned long delay;
        int  maxNumRuns;
        int  numRuns;
        bool enabled;
        int  toBeCalled;
    };

    int      numTimers;
    timer_t  timer[SIMPLETIMER_MAX_TIMERS];

    int findFirstFreeSlot();
    int setupTimer(unsigned long d, std::function<void()> f, int n);
};
