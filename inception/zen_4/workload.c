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

#define OFF3 128

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
            ".rept 10000\n\t"
            "mov "xstr(((SET * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 0) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 0) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 0) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
            "mov "xstr(((SET * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 8) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 8) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 8) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
            "mov "xstr(((SET * WAYS + 16) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 16) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 16) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 16) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
            "mov "xstr(((SET * WAYS + 67) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 67) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 67) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 67) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
            "mov "xstr(((SET * WAYS + 16) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 16) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 16) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 16) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
            "mov "xstr(((SET * WAYS + 88) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 88) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 88) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 88) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
            "mov "xstr(((SET * WAYS + 23) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 23) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 23) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 23) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
            "mov "xstr(((SET * WAYS + 90) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#ifdef OFF1
            "mov "xstr(((SET * WAYS + 90) * 8) + OFF1) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF2
            "mov "xstr(((SET * WAYS + 90) * 8) + OFF2) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif
#ifdef OFF3
            "mov "xstr(((SET * WAYS + 90) * 8) + OFF3) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
#endif

            ".endr\n\t"
            :: [addrs_addr]"r"(addrs) : "r8");
        // clang-format on
    }

    return 0;
}
