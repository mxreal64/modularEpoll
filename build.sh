#!/usr/bin/env bash
set -e

# 1. Locate and compile the native GCC standard library module source if not cached
if [ ! -d "gcm.cache" ]; then
    echo "Building standard library module cache..."
    # Dynamically find the internal system path for GCC's std.cc
    GCC_VERSION=$(g++ -dumpversion)
    STD_MODULE_PATH="/usr/include/c++/${GCC_VERSION}/bits/std.cc"
    
    if [ -f "${STD_MODULE_PATH}" ]; then
        g++ -std=c++23 -fmodules-ts -O3 -c "${STD_MODULE_PATH}"
    else
        echo "Error: Could not locate system std.cc file automatically."
        exit 1
    fi
fi

# 2. Compile your isolated epoll engine module
echo "Building modularEpoll object..."
g++ -std=c++23 -fmodules-ts -O3 -c modularEpoll.cppm -o modularEpoll.o

echo "Done! Ready for integration."
