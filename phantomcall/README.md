Navigate to either ./zen_1_2 or ./zen_3_4.

Simply run ./recursive_pcall.sh {ZEN/ZEN2/ZEN3/ZEN4} CORE1 CORE2 OUTPUT_DIR [CLANG_ARGS].

For example, if cores 1 and 9 are sibling hyperthreads, and you are running on Zen 2, you may want to run:

./recursive_pcall.sh ZEN2 1 9 out

In folder out, you will find up 2 files:

- no_ht.txt: which shows the output when disabling the co-resident sibling hyperthread
- ht.txt (only for Zen 1(+)/2, which shows the output when running a workload on the sibling hyperthread.

For Zen 3/4, no_ht.txt should show hijacked returns. For Zen 1(+)/2, only ht.txt should show hijacked returns.
