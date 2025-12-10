# CMake Configuration - Implementation Steps

## Objective

Create a modern CMake configuration that supports cross-platform builds, proper dependency management, and multiple build types.

## Prerequisites

- CMake 3.20 or higher installed
- C++20/23 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)

## Steps

### Step 1: Create Root CMakeLists.txt

1. Create `CMakeLists.txt` in project root
2. Set minimum CMake version: `cmake_minimum_required(VERSION 3.20)`
3. Set project name: `project(OCR-Orc VERSION 1.0.0 LANGUAGES CXX)`
4. Set C++ standard: `set(CMAKE_CXX_STANDARD 23)` and `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
5. Enable position-independent code: `set(CMAKE_POSITION_INDEPENDENT_CODE ON)`

### Step 2: Configure Build Types

1. Set default build type if not specified:
   ```cmake
   if(NOT CMAKE_BUILD_TYPE)
       set(CMAKE_BUILD_TYPE Release)
   endif()
   ```
2. Configure compiler-specific flags for Debug and Release

### Step 3: Set Up vcpkg Integration

1. Check if vcpkg toolchain file exists
2. If found, use it: `set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")`
3. Add note in README about vcpkg setup

### Step 4: Create Source Directory Structure

1. Add `src/` directory
2. Add `include/` directory for headers
3. Add `tests/` directory
4. Add `docs/` directory

### Step 5: Configure Qt

1. Use `find_package(Qt6 REQUIRED COMPONENTS Core Widgets)`
2. Enable Qt's MOC, UIC, RCC
3. Set Qt6_DIR if needed

### Step 6: Add Executable Target

1. Create executable target: `add_executable(owd ...)`
2. Link Qt libraries: `target_link_libraries(owd Qt6::Core Qt6::Widgets)`
3. Set output directory: `set_target_properties(owd PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)`

### Step 7: Configure Installation

1. Set install prefix
2. Add install rules for executable
3. Add install rules for resources (if any)

### Step 8: Add Testing Support

1. Enable testing: `enable_testing()`
2. Add test subdirectory (for later)

### Step 9: Create Build Scripts

1. Create `build.sh` for Unix/macOS
2. Create `build.bat` for Windows
3. Add instructions for building

### Step 10: Test Build

1. Create a minimal `main.cpp` that just prints "Hello World"
2. Run CMake configuration: `cmake -B build -S .`
3. Build: `cmake --build build`
4. Verify executable runs

## Acceptance Criteria

- [ ] CMake configuration completes without errors
- [ ] Project builds successfully
- [ ] Executable runs and prints "Hello World"
- [ ] Build scripts work on target platforms
- [ ] Qt is properly detected and linked

## Notes

- Use modern CMake practices (target-based, no global variables)
- Keep configuration simple; add complexity only when needed
- Document any platform-specific requirements

