#!/bin/bash
# Build script for C++ code

cd cpp
mkdir -p build
cd build
cmake ..
make

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "  - Benchmark: cpp/build/benchmark"
    echo "  - Tests:     cpp/build/test_algorithms"
    echo ""
    echo "Running tests..."
    ./test_algorithms
else
    echo "Build failed!"
    exit 1
fi

