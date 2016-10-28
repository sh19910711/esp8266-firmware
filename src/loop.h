#ifndef __LOOP_H__
#define __LOOP_H__

#define GLOBAL_INTERVAL 1000
#define TIMERS_MAX 8

struct timer {
    int current;
    int reset;
    void (*callback)();
};

void set_interval(int ms, void (*callback)());
void start_loop();
void init_timers();

#endif
