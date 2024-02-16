#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define WAYS     100
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
        // *((volatile char *)addrs[(SET * WAYS) + (rand() % WAYS)]);

        // clang-format off
        asm(
            ".rept 100000\n\t"
            "mov "xstr(((SET * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 16) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 67) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 16) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 88) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 23) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr(((SET * WAYS + 90) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 20) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 21) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 54) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 17) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 99) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"

	        // "mov "xstr(((SET * WAYS + 10) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 43) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"


            // "mov "xstr(((SET * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 10) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 20) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 30) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 40) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 50) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 60) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 70) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 80) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 90) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 10) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 11) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 13) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 14) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 15) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 16) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 17) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 18) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 19) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 20) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 21) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 22) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 23) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 24) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"



            // "mov "xstr(((((SET+1)) * WAYS + 90) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 80) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 70) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 60) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 50) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 40) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 30) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 20) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 10) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"

            // "mov "xstr(((((SET+1)) * WAYS + 10) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 11) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 13) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 14) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+1)) * WAYS + 15) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"

            // "mov "xstr(((((SET+2)) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 9) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 10) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 11) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 13) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 14) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((((SET+2)) * WAYS + 15) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"

            // "mov "xstr(((SET * WAYS + 16) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 17) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 18) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 19) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 20) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 21) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 22) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 23) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"
            // "mov "xstr(((SET * WAYS + 24) * 8)) "(%[addrs_addr]), %%r8\n\t"
            // "mov (%%r8), %%r8\n\t"

            ".endr\n\t"
            :: [addrs_addr]"r"(addrs) : "r8");
        // clang-format on
    }

    return 0;
}
