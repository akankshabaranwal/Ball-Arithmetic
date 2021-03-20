#!/bin/bash

sudo apt-get install libflint-2.5.2 libflint-dev

git submodule init
git submodule update

cd arb || exit
./configure
make -j4
make check
sudo make install
sudo ldconfig