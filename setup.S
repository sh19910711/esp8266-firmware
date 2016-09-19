.globl setup
.extern boot. __stack
setup:
    j 1f
.literal_position
.align 4
1:
    movi   sp, __stack
    movi   a4, boot
    callx0 a4
    ret.n

