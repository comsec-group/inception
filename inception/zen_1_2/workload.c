#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define WAYS     9
#define SETS     1024
#define HUGEPAGE 2097152

#define str(s)  #s
#define xstr(s) str(s)

volatile char *base;

void get_addrs(uint64_t *buf, uint64_t set, int amount) {
    for (int i = 0; i < amount; i++) {
        buf[i] = ((((((uint64_t) base) >> 16) + i) << 10) + set) << 6;
    }
}

int main() {
    base = aligned_alloc(HUGEPAGE, HUGEPAGE);
    madvise((void *) base, HUGEPAGE, MADV_HUGEPAGE);
    mprotect((void *) base, HUGEPAGE, PROT_READ | PROT_WRITE | PROT_EXEC);

    uint64_t addrs[SETS * WAYS];
    for (int i = 0; i < SETS; i++) {
        get_addrs(addrs + (WAYS * i), i, WAYS);
    }

    while (1) {
        // clang-format off
        asm(
            ".rept 1000\n\t"
            "mov "xstr(((SET * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            ".endr\n\t"
            :: [addrs_addr]"r"(addrs) : "r8");
        // clang-format on
    }

    return 0;
}
