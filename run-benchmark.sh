#!/bin/bash

SUITE="${1:-def}"
FILE="./logs/${SUITE}.log"
ITERATIONS="${2:-5}"

truncate -s 0 $FILE
for (( i=0; i<$ITERATIONS; i++ ))
do
  ./benchmark.bin $SUITE | tee -a $FILE
done
