#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "memtools.h"
#define RB_STRIDE 0x10000
#include "rb_tools.h"
#include <fcntl.h>
#include <sys/ioctl.h>

//#define USE_IBPB
#ifdef USE_IBPB
#include "kmod_ibpb/ibpb_ioctl.h"
#endif

#define str(s) #s
#define xstr(s) str(s)

#define int3 asm("int3\n")

#define IRET(reg, pfx)                          \
    "movq   " #pfx "%ss, " #pfx "%rax     \n\t" \
    "pushq  " #pfx "%rax                  \n\t" \
    "lea    8(" #pfx "%rsp), " #pfx "%rax \n\t" \
    "pushq  " #pfx "%rax                  \n\t" \
    "pushf                                \n\t" \
    "movq   " #pfx "%cs, " #pfx "%rax     \n\t" \
    "pushq  " #pfx "%rax                  \n\t" \
    "pushq  " #pfx "%" #reg "             \n\t" \
    "iretq                                \n\t"

#define IRET_R8 IRET(r8, )

//
// ----------------------------------------------------------------------------
//

/* #define BHB_LEN (829) */
#define BHB_LEN (129)
/* #define BHB_LEN (128) */
#define BHB_ALIGN (0x40-5) // the dir jmp is assumed to be 5 bytes
                           //
#define BHB_ASM_STR \
        ".align 0x40                      \n"\
        ".rept " xstr(BHB_LEN)           "\n"\
        "jmp 1f                           \n"\
        ".skip " xstr(BHB_ALIGN) ",   0x90\n"\
        "1:                               \n"\
        ".endr                            \n"\

#define STR_JMP_PATH(n,dst) \
    ".rept " xstr(n) "\n"\
    "jmp 1f\n"\
    ".skip " xstr(dst-5) ", 0xcc\n" \
    "1:\n" \
    ".endr\n"

#define BHB_ASM asm(BHB_ASM_STR)

#define NOP2  ".byte 0x66,0x90\n\t"
#define NOP15 ".byte 0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x0F,0x1F,0x84,0x00,0x00,0x00,0x00,0x00\n\t"

#define ONE_PATTERN     0x000080080000
#define ANOTHER_PATTERN 0x000200200000
#define A_PTR 0x1000000
#define A2_PTR (A_PTR^ANOTHER_PATTERN)
#define V_PTR (A_PTR^ONE_PATTERN)
#define DJMP_DIST 0x2000000

#define TTE_SIGNAL 4

mk_tmpl(a,
    BHB_ASM_STR
    "mov (%rdi), %rdi                \n"
    "test %rdi, %rdi                 \n"
    "jnz a__tmpl + "xstr(DJMP_DIST) "\n"  // cond==1 => jmp to C
    "jmp *(%rsi)\n"  // TTE here
    NOP15 NOP15 NOP2 // 32
    NOP15 NOP15 NOP2 // 32
    "pop %r8         \n"
    IRET_R8);

mk_tmpl(c,
    "lfence          \n" // synchronize here
    "mov $0xff, %rdi \n"
    "pop %r8         \n"
    IRET_R8);

// never arch executed.
mk_tmpl(d,
    "mov "xstr(RB_PTR + TTE_SIGNAL*RB_STRIDE)",%rax\n" // signal "bti target was used"
    "int3\n"
    "lfence          \n" // synchronize here
    "mov $0xff, %rdi \n"
    "pop %r8         \n"
    IRET_R8);

#ifndef mb
#define mb() asm("mfence" ::: "memory")
#endif

#define REPS 100000

#define S 2
#define S2 2
#define S3 4
#define THRES 0


u64 slowmem[1000];

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
        case 'h':
        default:
            fprintf(stderr, "Usage: %s [-h]\n", argv[0]);
            exit(1);
        }
    }
    rb_init();
#ifdef USE_IBPB
    ibpb_init();
#endif

    struct j_malloc a, c, d;
    srand(getpid());
    if (alloc_tmpl(&a, a, A_PTR)) err(1,"alloc_tmpl");
    if (alloc_tmpl(&c, c, A_PTR + DJMP_DIST)) err(1,"alloc_tmpl");
    if (alloc_tmpl(&d, d, 0)) err(1,"alloc_tmpl");

    u64 *cond = &slowmem[500];
    u64 *slow_bti_ptr = &slowmem[540];
    *slow_bti_ptr = d.ptr_u64;

    u64 *slow_c_ptr2 = &slowmem[580];
    *slow_c_ptr2 = c.ptr_u64;

    printf("Arch: %s\n",xstr(ARCH));
    printf("Reps: %d\n", REPS);

    rb_reset();
    for (int i = 0; i < REPS; ++i)  {
        /* *cond = 0;                                            */
        /* a.fptr(cond, &c.ptr); // T1 taken => C; !taken => C */
        *cond = 1;
        flush(cond);
        a.fptr(cond, &c.ptr); // T2 taken => C; !taken => C
        *cond = 0;
        a.fptr(cond, &c.ptr); // T1 taken => C; !taken => C
        *cond = 1;
        flush(cond);
        a.fptr(cond, &d.ptr); // T2 taken => C; !taken => D
        mb();
        rb_flush();
        // mb();
        *cond = 0;
        /* mb(); */
        /* flush(slow_c_ptr2); */
        /* flush(cond); */
        /* mb(); */
        a.fptr(cond, slow_c_ptr2);
        rb_reload();
#ifdef USE_IBPB
        IBPB();
        mb();
#endif
    }
    printf("sig_spec_bti  %ld\n", rb_hist[TTE_SIGNAL]);
    rb_print();
}
