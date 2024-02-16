#include "./ibpb_cost_ioc.h"
#include <linux/module.h>
#include <linux/proc_fs.h>

#define INFO(...) pr_info("ibpb_cost: " __VA_ARGS__)
#define ERR(...)  pr_err("ibpb_cost: " __VA_ARGS__)

static struct proc_dir_entry *procfs_file;
static struct ibpb_cost_ioc_msg msg;

// start measure.
static __always_inline u64 tick(void) {
    u64 lo, hi;
    asm volatile("CPUID\n\t"
                 "RDTSC\n\t"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

// read clock cycle counter via APERF
static __always_inline u64 tick_a(void) {
    u64 lo, hi;
    asm volatile("CPUID\n\t"
                 "movq $0xc00000e8, %%rcx\n"
                 "rdmsr\n"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

static __always_inline u64 tock(void) {
    u64 lo, hi;
    asm volatile("rdtscp\n"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 "CPUID\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}
static __always_inline u64 tock_a(void) {
    u64 lo, hi;
    asm volatile("movq $0xc00000e8, %%rcx\n"
                 "rdmsr\n"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 "CPUID\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

noinline unsigned long run_dry(void) {
    u64 t;
    t = tick_a();
    // clang-format off
    asm("movq $"__stringify(MSR_IA32_PRED_CMD) ", %%rcx\n"
        "movq $"__stringify(PRED_CMD_IBPB) ", %%rax\n"
        "xor %%rdx,%%rdx\n" ::: "rcx", "rax", "rdx");
    // clang-format on
    return tock_a() - t;
}

noinline unsigned long run(void) {
    u64 t;
    t = tick_a();
    // clang-format off
    asm("movq $"__stringify(MSR_IA32_PRED_CMD) ", %%rcx\n"
        "movq $"__stringify(PRED_CMD_IBPB) ", %%rax\n"
        "xor %%rdx,%%rdx\n"
        "wrmsr\n" :: : "rcx", "rax", "rdx");
    // clang-format on
    return tock_a() - t;
}

static long handle_ioctl(struct file *filp, unsigned int request, unsigned long argp) {
    if (copy_from_user(&msg, (void *) argp, sizeof(msg)) != 0) {
        ERR("Cannot copy from user\n");
        return -EINVAL;
    }

    switch (request) {
    case IBPB_COST_IOC_HELLO:
        INFO("Hello\n");
        break;

    case IBPB_COST_IOC_MEASSURE:
        msg.result = run();
        if (copy_to_user((void *) argp, &msg, sizeof(msg)) != 0) {
            return -EIO;
        }
        break;
    case IBPB_COST_IOC_DRY:
        msg.result = run_dry();
        if (copy_to_user((void *) argp, &msg, sizeof(msg)) != 0) {
            return -EIO;
        }
        break;
    default:
        return -ENOIOCTLCMD;
    }
    return 0;
}

static struct proc_ops pops = {
    .proc_ioctl = handle_ioctl,
    .proc_open = nonseekable_open,
    .proc_lseek = no_llseek,
};

static int ibpb_cost_init(void) {
    procfs_file = proc_create(PROC_IBPB_COST, 0, NULL, &pops);
    INFO("init\n");
    return 0;
}

static void ibpb_cost_exit(void) {
    INFO("exit\n");
    proc_remove(procfs_file);
}

module_init(ibpb_cost_init);
module_exit(ibpb_cost_exit);
MODULE_LICENSE("GPL");
