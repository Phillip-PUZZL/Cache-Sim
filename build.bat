mkdir cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=ninja.exe -G Ninja -S . -B cmake-build-release
cmake --build cmake-build-release
mkdir output