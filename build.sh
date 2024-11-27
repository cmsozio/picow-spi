#!/bin/bash

BOARD=pico_w

mkdir build
cd build
cmake .. -DPICO_BOARD=$(BOARD)
make