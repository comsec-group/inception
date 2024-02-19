#!/usr/bin/env bash

mkdir -p out

CORE1="${CORE1:-3}"
CORE2="${CORE2:-195}"

# used for guest to host
BLOCKSZ="${BLOCKSZ:-0x80000000}"
NRBLOCKS="${NRBLOCKS:-385}"

SET=40

clang -DCORE1=$CORE1 -DCORE2=$CORE2 -DBLOCKSZ=$BLOCKSZ -DNRBLOCKS=$NRBLOCKS inception_kaslr.c -o out/inception_kaslr
clang -DCORE1=$CORE1 -DCORE2=$CORE2 -DBLOCKSZ=$BLOCKSZ -DNRBLOCKS=$NRBLOCKS inception.c -o out/inception

clang -DSET=$SET workload.c -o out/workload
clang -DSET=$SET workload2.c -o out/workload2
clang -DSET=$SET workload3.c -o out/workload3
