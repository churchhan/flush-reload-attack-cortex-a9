#include "ClearCache.h"

void clearcache(char *begin, char *end)
{
    const int syscall = 0xf0002;

    __asm __volatile (
            "mov r0, %0\n"
            "mov r1, %1\n"
            "mov r7, %2\n"
            "mov r2, #0x0\n"
            "svc 0x00000000\n"
            :
            : "r" (begin), "r" (end), "r" (syscall)
            : "r0", "r1", "r7"
            );
}
