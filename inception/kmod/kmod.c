#include <asm/kvm_host.h>
#include <linux/kernel.h>
#include <linux/kvm_host.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static unsigned long kernel_load_address = 0xffffffff81000000;

volatile char secret[4096];

module_param(kernel_load_address, ulong, 0);

static int __init my_init(void) {
    memset((void *) secret, 'A', 4096);
    memset((void *) secret + 1024, 'B', 1024);
    memset((void *) secret + 2048, 'C', 1024);
    memset((void *) secret + 3072, 'D', 1024);

    printk("secret_ptr: %px\n", secret);
    return 0;
}

static void __exit my_exit(void) {
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
