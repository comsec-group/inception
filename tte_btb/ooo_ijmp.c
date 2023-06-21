#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "memtools.h"
#define RB_STRIDE 0x10000
#include "rb_tools.h"

//
// ooo -> ijmp
//
//
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

// untrackable call.
#define IRET_CALL(regCallee, regRet, pfx) \
    "lea 1f("#pfx"%rip), %"#regRet"\n"\
    "push "#pfx"%"#regRet" \n"\
    IRET(regCallee, pfx) \
    "1:\n"


//
// ----------------------------------------------------------------------------
//

/* #define BHB_LEN (829) */
#define BHB_LEN (196)
/* #define BHB_LEN (29) */
/* #define BHB_LEN (1) */
/* #define BHB_LEN (128) */
#define BHB_ALIGN (0x80-5) // the dir jmp is assumed to be 5 bytes
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

#define TTE_SIGNAL 2

mk_tmpl(a,
    BHB_ASM_STR
    "mfence\n"
    "mov (%rdi), %r8\n" // crash you!
    "jmp *%rsi\n"// Now train the btb
);

// never arch executed.
mk_tmpl(d,
    "mov "xstr(RB_PTR + TTE_SIGNAL * RB_STRIDE)",%rax\n" // signal "bti target was used"
    "int3\n"
    "lfence          \n" // synchronize here
    "mov $0xff, %rdi \n"
    "pop %r8         \n"
    IRET_R8);

mk_tmpl(c,
    "lfence          \n" // synchronize here
    "mov $0xff, %rdi \n"
    "pop %r8         \n"
    IRET_R8);

#ifndef mb
#define mb() asm("mfence" ::: "memory")
#endif

#define REPS 1000000

#define THRES 0

#include <setjmp.h>
#include <signal.h>
static int should_segfault = 0;
static sigjmp_buf env;
static void handle_segv(int sig, siginfo_t *si, void *unused)
{
    if (should_segfault) {
        siglongjmp(env, 12);
        return;
    };

    fprintf(stderr, "Not handling SIGSEGV\n");
    exit(sig);
}

static void inline init_segv_handler() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = &handle_segv;
    sigaction (SIGSEGV, &sa, NULL);
}


#include <fcntl.h>
#include <sys/ioctl.h>

/* #define USE_IBPB */

#ifdef USE_IBPB
#include "kmod_ibpb/ibpb_ioctl.h"
#endif

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

    init_segv_handler();
    rb_init();
#ifdef USE_IBPB
    ibpb_init();
#endif

    struct j_malloc a, d, c;
    if (alloc_tmpl(&a, a, 0)) err(1,"alloc_tmpl");


    if (alloc_tmpl(&d, d, 0)) err(1,"alloc_tmpl");
    if (alloc_tmpl(&c, c, 0)) err(1,"alloc_tmpl");

    printf("Arch: %s\n",xstr(ARCH));
    printf("Reps: %d\n", REPS);
    rb_reset();
    for (int i = 0; i < REPS; ++i)  {
        // rdi=0 => *NULL.
        should_segfault=1;
        if (!sigsetjmp(env, 1)) {
            a.fptr(0, d.fptr);
        }
        should_segfault=0;
        rb_flush();
        a.fptr(slowmem, c.fptr);

        rb_reload();
#ifdef USE_IBPB
        IBPB();
#endif
    }
    printf("sig_hit_ooo_bti %ld\n", rb_hist[TTE_SIGNAL]);
    rb_print();
}
