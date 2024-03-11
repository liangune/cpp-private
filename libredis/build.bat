mkdir build_windows32
cd build_windows32
cmake -G "Visual Studio 15 2017"  -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release ../ 