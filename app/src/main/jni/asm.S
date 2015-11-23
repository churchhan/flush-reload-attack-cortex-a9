.arm
.text
.align 2

.global memread

memread:
    ldr r1, [r0]
    bx lr


.end