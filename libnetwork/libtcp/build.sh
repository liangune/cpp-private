#!/bin/bash
mkdir build_linux
cd build_linux
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
make -j4
make install