Simply run ./tte_rsb.sh CORE1 CORE2 OUTPUT_DIR [CLANG_ARGS].

For example, if cores 1 and 9 are sibling hyperthreads, you may want to run:

./tte_rsb.sh 1 9 out

In folder out, you will find 6 files:

- btb_16_calls.txt: This is the output of the TTE_BTB_RSB experiment, where we attmept to execute 16 transient calls. 
- btb_32_calls.txt: This is the output of the TTE_BTB_RSB experiment, where we attmept to execute 32 transient calls. 
- pht_16_calls.txt: This is the output of the TTE_PHT_RSB experiment, where we attmept to execute 16 transient calls 
- pht_32_calls.txt: This is the output of the TTE_PHT_RSB experiment, where we attmept to execute 32 transient calls.
- rsb_16_calls.txt: This is the output of the TTE_RSB_RSB experiment, where we attmept to execute 16 transient calls. 
- rsb_32_calls.txt: This is the output of the TTE_RSB_RSB experiment, where we attmept to execute 32 transient calls.

On Zen 3/4, the *_16_calls.txt output files should show that some returns are hijacked. 
The *_32_calls.txt output files should show hijacked returns for all AMD Zen microarchitectures.
