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

    while(1){
        asm(
            ".rept 10000\n\t"
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

	        "mov "xstr((((SET+1) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+1) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

	        "mov "xstr((((SET+2) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+2) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

	        "mov "xstr((((SET+3) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+3) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

            "mov "xstr((((SET+4) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+4) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

            "mov "xstr((((SET+5) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+5) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

            "mov "xstr((((SET+6) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+6) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

            "mov "xstr((((SET+7) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+7) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
	    
            "mov "xstr((((SET+8) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+8) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

            "mov "xstr((((SET+9) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+9) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

            "mov "xstr((((SET+10) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+10) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"

            "mov "xstr((((SET+11) * WAYS + 0) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 1) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 2) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 3) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 4) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 5) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 6) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 7) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
            "mov "xstr((((SET+11) * WAYS + 8) * 8)) "(%[addrs_addr]), %%r8\n\t"
            "mov (%%r8), %%r8\n\t"
	
	    ".endr\n\t"

            ::[addrs_addr] "r"(addrs)
            : "r8");
    }

    return 0;
}