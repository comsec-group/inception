#include <linux/module.h>
#include <linux/proc_fs.h>
#include "ibpb_ioctl.h"

static struct proc_dir_entry *procfs_file;

static inline void my_indirect_branch_prediction_barrier(void)
{
	u64 val = PRED_CMD_IBPB;

	alternative_msr_write(MSR_IA32_PRED_CMD, val, X86_FEATURE_USE_IBPB);
}


static long handle_ioctl(struct file *filp, unsigned int request, unsigned long argp) {
    if (request == REQ_IBPB) {
        asm volatile("lfence");
        my_indirect_branch_prediction_barrier();
    }
    return 0;
}

static struct proc_ops pops = {
    .proc_ioctl = handle_ioctl,
    .proc_open = nonseekable_open,
    .proc_lseek = no_llseek,
};

static void mod_ibpb_exit(void) {
    proc_remove(procfs_file);
}

static int mod_ibpb_init(void) {
    procfs_file = proc_create(PROC_IBPB, 0, NULL, &pops);
    return 0;
}

module_init(mod_ibpb_init);
module_exit(mod_ibpb_exit);

MODULE_LICENSE("GPL");
