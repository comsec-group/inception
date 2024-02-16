#!/bin/bash

zens=$(cpuid | grep '(uarch synth)' | head -n 1)

if [[ $zens == *"AMD Zen"* ]]; then
  echo "PASS: AMD Zen"
else
  echo "FAIL: AMD Zen: you cannot evaluate this artifact on this machine"
  exit
fi

if [[ $zens == *"AMD Zen 3"* ]]; then
  echo "FAIL: AMD Zen 1/2/4: you cannot evaluate Experiment 3 on this machine"
else
  echo "PASS: AMD Zen 1/2/4"
fi

linux=$(uname -r)

if [[ $linux == *"5.19.0-28-generic"* ]]; then
  echo "PASS: Linux kernel version"
else
  echo "FAIL: Linux kernel version (you cannot evaluate Experiment 3 on this  machine)"
  echo "  Find copy: https://jknr.me/filez/ubuntu-5.19.0-28-generic.tar.gz"
fi
