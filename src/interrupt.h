#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

void accept_interrupt(int pin, void (*cb)(int));

#endif
