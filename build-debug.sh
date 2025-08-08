#!/bin/bash

# Debug build script for PineappleWriter

set -e  # Exit on any error

echo "Building PineappleWriter in Debug mode..."

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root directory."
    exit 1
fi

# Create build-debug directory if it doesn't exist
if [ ! -d "build-debug" ]; then
    echo "Creating build-debug directory..."
    mkdir build-debug
fi

# Navigate to build-debug directory
cd build-debug

# Configure with CMake in Debug mode
echo "Configuring with CMake (Debug mode)..."
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build the project
echo "Building the project (Debug mode)..."
make -j$(nproc)

echo "Debug build completed successfully!"