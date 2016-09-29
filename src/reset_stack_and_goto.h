#ifndef __RESET_STACK_AND_GOTO_H__
#define __RESET_STACK_AND_GOTO_H__

extern "C" {

void reset_stack_and_goto(void (*func)());

}

#endif
