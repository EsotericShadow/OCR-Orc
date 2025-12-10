# Build Instructions

## Prerequisites

- **CMake**: 3.20 or later
- **C++ Compiler**: C++23 compatible (Clang 15+, GCC 13+, or MSVC 2022+)
- **Qt**: 6.9.3 or later
- **vcpkg**: For dependency management (Poppler)
- **macOS**: 10.15 (Catalina) or later (for macOS builds)

## Quick Build

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build

# Run
./build/bin/ocr-orc.app/Contents/MacOS/ocr-orc
```

## Detailed Build Steps

### 1. Install Dependencies

#### macOS (using Homebrew)
```bash
brew install qt@6 cmake
```

#### vcpkg Setup
```bash
# Install vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Install Poppler
./vcpkg install poppler
```

### 2. Configure Build

```bash
# Release build (default)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Debug build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# With vcpkg toolchain
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### 3. Build

```bash
# Build all targets
cmake --build build

# Build specific target
cmake --build build --target ocr-orc

# Build with multiple jobs (faster)
cmake --build build -j$(sysctl -n hw.ncpu)
```

### 4. Run Tests

```bash
cd build
ctest --output-on-failure

# Run specific test
./tests/test_canvas_rotation
```

## Build Options

### CMake Variables

- `CMAKE_BUILD_TYPE`: `Release` (default) or `Debug`
- `CMAKE_TOOLCHAIN_FILE`: Path to vcpkg toolchain file
- `Qt6_DIR`: Path to Qt6 CMake config (auto-detected if Qt is in PATH)

### Release Build Configuration

Release builds are optimized with:
- `-O3` optimization
- `-DNDEBUG` flag
- No debug symbols
- Conditional debug output disabled

## Troubleshooting

### Qt Not Found
```bash
# Set Qt6_DIR explicitly
cmake -S . -B build -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6
```

### Poppler Not Found
```bash
# Ensure vcpkg is configured
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Build Errors
1. Clean build directory: `rm -rf build && cmake -S . -B build`
2. Check compiler version: `clang++ --version` (should support C++23)
3. Verify Qt installation: `qmake --version`

## Production Build

For production release:

```bash
# Clean build
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target ocr-orc

# Verify
./build/bin/ocr-orc.app/Contents/MacOS/ocr-orc --version
```

## Distribution

The application bundle is located at:
```
build/bin/ocr-orc.app
```

To distribute:
1. Copy `ocr-orc.app` to target location
2. Ensure all Qt libraries are available (or bundle them)
3. Test on clean system

