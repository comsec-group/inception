if [ "$#" -lt 3 ]; then
  echo "Usage: $0 {ZEN3/ZEN4} CORE1 CORE2 OUTPUT_DIR" >&2
  exit 1
fi

mkdir -p $4

sudo bash -c "echo 0 > /sys/devices/system/cpu/cpu$3/online"

taskset -c $2 ./out/recursive_pcall > $4/no_ht.txt

sudo bash -c "echo 1 > /sys/devices/system/cpu/cpu$3/online"
