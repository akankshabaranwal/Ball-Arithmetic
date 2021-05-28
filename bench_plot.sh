#!/bin/bash

SUITE="${1:-def}"

./run-benchmark.sh $SUITE
python3 plot_benchmark.py ./logs/$SUITE
