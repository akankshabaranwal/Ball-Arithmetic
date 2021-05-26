#!/bin/bash

echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

for i in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
	echo performance | sudo tee $i
done

cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq
