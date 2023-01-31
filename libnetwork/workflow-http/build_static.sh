#!/bin/bash
mkdir build_linux
cd build_linux
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
make -j4
make install
