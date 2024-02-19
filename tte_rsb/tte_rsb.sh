if [ "$#" -lt 3 ]; then
  echo "Usage: $0 CORE1 CORE2 OUTPUT_DIR" >&2
  exit 1
fi

sudo bash -c "echo 0 > /sys/devices/system/cpu/cpu$2/online"

mkdir -p $3

taskset -c $1 ./out/tte_btb_rsb_16 > $3/btb_16_calls.txt $4
taskset -c $1 ./out/tte_btb_rsb_32 > $3/btb_32_calls.txt $4
taskset -c $1 ./out/tte_rsb_rsb_16 > $3/rsb_16_calls.txt $4
taskset -c $1 ./out/tte_rsb_rsb_32 > $3/rsb_32_calls.txt $4
taskset -c $1 ./out/tte_pht_rsb_16 > $3/pht_16_calls.txt $4
taskset -c $1 ./out/tte_pht_rsb_32 > $3/pht_32_calls.txt $4

sudo bash -c "echo 1 > /sys/devices/system/cpu/cpu$2/online"
