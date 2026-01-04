#!/bin/bash
# Build script for Linux
# Requires: CMake, GCC/Clang, Vulkan SDK

echo "======================================"
echo "Building Cache Blocking Demo (Linux)"
echo "======================================"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo ""
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build the project
echo ""
echo "Building project..."
cmake --build . -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "======================================"
echo "Build successful!"
echo "Executable: build/bin/CacheBlockingDemo"
echo "======================================"
