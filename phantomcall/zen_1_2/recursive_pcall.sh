if [ "$#" -lt 4 ]; then
  echo "Usage: $0 {ZEN/ZEN2} CORE1 CORE2 OUTPUT_DIR [CLANG_ARGS]" >&2
  exit 1
fi

mkdir -p $4 

clang -DSET=0 -DWAYS=9 workload.c -o workload $5 > /dev/null

sudo bash -c "echo 0 > /sys/devices/system/cpu/cpu$3/online"

clang -D$1 recursive_pcall.c -o recursive_pcall $5 > /dev/null

taskset -c $2 ./recursive_pcall > $4/no_ht.txt

sudo bash -c "echo 1 > /sys/devices/system/cpu/cpu$3/online"

taskset -c $3 ./workload & 
taskset -c $2 ./recursive_pcall > $4/ht.txt & 

wait
