zens=$(cpuid | grep '(uarch synth)' | head -n 1)

if [[ $zens == *"AMD Zen"* ]]; then
  echo "AMD Zen: PASS"
else
  echo "AMD Zen: FAIL (you cannot evaluate this artifact on this machine)"
  exit
fi

if [[ $zens == *"AMD Zen 3"* ]]; then
  echo "AMD Zen 1/2/4: PASS (you cannot evaluate Experiment 3 on this machine)"
else
  echo "AMD Zen 1/2/4: OK"
fi

linux=$(uname -r)

if [[ $linux == *"5.19.0-28-generic"* ]]; then
  echo "Linux kernel version: PASS"
else
  echo "Linux kernel version: FAIL (you cannot evaluate Experiment 3 on this  machine)"
fi
