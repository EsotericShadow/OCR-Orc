#!/bin/bash
# Build script for Unix/macOS
# Usage: ./build.sh [Debug|Release]

set -e

# Default build type
BUILD_TYPE="${1:-Release}"

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo "Error: Build type must be 'Debug' or 'Release'"
    exit 1
fi

echo "Building OWD in $BUILD_TYPE mode..."

# Create build directory
BUILD_DIR="build"
mkdir -p "$BUILD_DIR"

# Configure with CMake
cmake -B "$BUILD_DIR" \
    -S . \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

echo ""
echo "Build complete! Executable: $BUILD_DIR/bin/owd"
echo ""
echo "To run: $BUILD_DIR/bin/owd"

