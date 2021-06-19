#!/bin/bash

SUITE="${1:-def}"

./run-benchmark.sh $SUITE 10
python3 plot_benchmark.py ./logs/$SUITE
