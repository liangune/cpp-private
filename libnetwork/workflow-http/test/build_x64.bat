mkdir build_windows
cd build_windows
cmake -G "Visual Studio 15 2017 Win64"  -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release ../ 