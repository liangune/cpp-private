#!/bin/bash
mkdir build_linux
cd build_linux
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
make install