# roseserver

## Build (Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build gdb
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/Server/Server
./build/DummyClient/DummyClient




grep CMAKE_BUILD_TYPE build/CMakeCache.txt
