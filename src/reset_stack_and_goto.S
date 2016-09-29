.globl reset_stack_and_goto
.extern boot, __stack
reset_stack_and_goto:
    j 1f
.literal_position
.align 4
1:
    movi   sp, __stack
    jx     a2
