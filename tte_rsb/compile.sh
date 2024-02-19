if [ "$#" -lt 0 ]; then
  echo "Usage: $0 [CLANG_ARGS]" >&2
  exit 1
fi

mkdir -p out

clang -DBTB -DCALLS_CNT=16 tte_rsb.c -o out/tte_btb_rsb_16 $1
clang -DBTB -DCALLS_CNT=32 tte_rsb.c -o out/tte_btb_rsb_32 $1
clang -DRSB -DCALLS_CNT=16 tte_rsb.c -o out/tte_rsb_rsb_16 $1
clang -DRSB -DCALLS_CNT=32 tte_rsb.c -o out/tte_rsb_rsb_32 $1
clang -DPHT -DCALLS_CNT=16 tte_rsb.c -o out/tte_pht_rsb_16 $1
clang -DPHT -DCALLS_CNT=32 tte_rsb.c -o out/tte_pht_rsb_32 $1
