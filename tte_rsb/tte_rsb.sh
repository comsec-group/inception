if [ "$#" -lt 3 ]; then
  echo "Usage: $0 CORE1 CORE2 OUTPUT_DIR [CLANG_ARGS]" >&2
  exit 1
fi

mkdir -p $3 

sudo bash -c "echo 0 > /sys/devices/system/cpu/cpu$2/online"

clang -DBTB -DCALLS_CNT=16 tte_rsb.c -o tte_rsb $4

taskset -c $1 ./tte_rsb > $3/btb_16_calls.txt $4

clang -DBTB -DCALLS_CNT=32 tte_rsb.c -o tte_rsb $4

taskset -c $1 ./tte_rsb > $3/btb_32_calls.txt $4

clang -DRSB -DCALLS_CNT=16 tte_rsb.c -o tte_rsb $4

taskset -c $1 ./tte_rsb > $3/rsb_16_calls.txt $4

clang -DRSB -DCALLS_CNT=32 tte_rsb.c -o tte_rsb $4

taskset -c $1 ./tte_rsb > $3/rsb_32_calls.txt $4

clang -DPHT -DCALLS_CNT=16 tte_rsb.c -o tte_rsb $4

taskset -c $1 ./tte_rsb > $3/pht_16_calls.txt $4

clang -DPHT -DCALLS_CNT=32 tte_rsb.c -o tte_rsb $4

taskset -c $1 ./tte_rsb > $3/pht_32_calls.txt $4

sudo bash -c "echo 1 > /sys/devices/system/cpu/cpu$2/online"
