#include <stdint.h>

void DCInvalidateRange(void* addr, uint32_t size) {
    if (size == 0) return;

    /* Direct PowerPC Assembly:
       'dcbi' stands for Data Cache Block Invalidate.
       We loop through memory in 32-byte chunks (the Wii's cache line size).
    */
    uint32_t a = (uint32_t)addr & ~31;
    uint32_t end = (uint32_t)addr + size;

    while (a < end) {
        asm volatile ("dcbi 0,%0" : : "r"(a));
        a += 32;
    }
    asm volatile ("sync; isync");
}