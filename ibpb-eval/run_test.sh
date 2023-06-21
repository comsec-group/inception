#!/bin/bash

set -x

if [ -z "$1" ]; then
        exit 1
fi


if [ -e /data/${USER}/byte-unixbench/UnixBench ]; then
        exit 1
fi

rsync -az /data/${USER}/byte-unixbench/UnixBench "$1":

DIR=baseline
if ssh $1 "grep -q 'Mitigation: IBPB' /sys/devices/system/cpu/vulnerabilities/retbleed"; then
        DIR=ibpb
fi
## handle ibpb+nosmt ..

for a in {1..5}; do
        echo $a ------------------------
        ssh $1 "cd UnixBench && ./Run"
done

mkdir -p $1/$DIR

rsync -az $1:UnixBench/results/ $1/$DIR/

ssh $1 "rm -r UnixBench/results" # clean
