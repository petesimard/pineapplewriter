#!/bin/bash

# Build script for PineappleWriter

set -e  # Exit on any error

echo "Building PineappleWriter..."

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root directory."
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building the project..."
make -j$(nproc)

echo "Build completed successfully!"