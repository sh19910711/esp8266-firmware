#ifndef __LOOP_H__
#define __LOOP_H__

#define GLOBAL_INTERVAL 1000

struct timer {
    int current;
    int reset;
    void (*callback)();
};

void set_interval(int ms, void (*callback)());
void start_loop();

#endif
