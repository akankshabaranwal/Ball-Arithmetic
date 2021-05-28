#!/bin/bash

FILE="${1:-benchmark}.log"
ITERATIONS="${2:-5}"

truncate -s 0 $FILE
for (( i=0; i<$ITERATIONS; i++ ))
do
  ./benchmark.bin | tee -a $FILE
done
