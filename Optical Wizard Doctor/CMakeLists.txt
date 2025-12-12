# OCR-Orc - CMake Configuration
# Modern CMake configuration for cross-platform C++ application

cmake_minimum_required(VERSION 3.20)

# Project definition
project(OCR_Orc 
    VERSION 1.0.0 
    DESCRIPTION "OCR-Orc - PDF Coordinate Calibration Tool"
    LANGUAGES CXX
)

# C++ standard configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clangd and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable position-independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC flags
    add_compile_options(/W4 /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Zi /Od)
    else()
        add_compile_options(/O2 /DNDEBUG)
    endif()
endif()

# vcpkg integration
# Try multiple methods to find vcpkg
set(VCPKG_TOOLCHAIN_FILE "")

# Method 1: VCPKG_ROOT environment variable
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg from VCPKG_ROOT: ${VCPKG_TOOLCHAIN_FILE}")
# Method 2: Common installation locations
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg from sibling directory: ${VCPKG_TOOLCHAIN_FILE}")
elseif(EXISTS "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_TOOLCHAIN_FILE "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg from home directory: ${VCPKG_TOOLCHAIN_FILE}")
elseif(EXISTS "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg from /usr/local: ${VCPKG_TOOLCHAIN_FILE}")
endif()

if(VCPKG_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_TOOLCHAIN_FILE}" CACHE STRING "")
else()
    message(WARNING "vcpkg not found. Dependencies may need to be installed manually.")
    message(WARNING "To use vcpkg:")
    message(WARNING "  1. Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git")
    message(WARNING "  2. Bootstrap: ./vcpkg/bootstrap-vcpkg.sh")
    message(WARNING "  3. Set VCPKG_ROOT environment variable or place vcpkg in a standard location")
endif()

# Qt6 configuration
# Try to find Qt6 in common locations first
set(Qt6_DIR "" CACHE PATH "Path to Qt6Config.cmake")

# Common Qt6 installation paths (macOS)
if(APPLE)
    # Homebrew Qt6 (Apple Silicon)
    if(EXISTS "/opt/homebrew/lib/cmake/Qt6")
        set(Qt6_DIR "/opt/homebrew/lib/cmake/Qt6" CACHE PATH "" FORCE)
        message(STATUS "Auto-detected Qt6 from Homebrew (Apple Silicon): ${Qt6_DIR}")
    # Homebrew Qt6 (Intel)
    elseif(EXISTS "/usr/local/lib/cmake/Qt6")
        set(Qt6_DIR "/usr/local/lib/cmake/Qt6" CACHE PATH "" FORCE)
        message(STATUS "Auto-detected Qt6 from Homebrew (Intel): ${Qt6_DIR}")
    # Qt Installer location
    elseif(EXISTS "$ENV{HOME}/Qt/6.*/macos/lib/cmake/Qt6")
        file(GLOB QT6_PATHS "$ENV{HOME}/Qt/6.*/macos/lib/cmake/Qt6")
        if(QT6_PATHS)
            list(GET QT6_PATHS 0 Qt6_DIR)
            set(Qt6_DIR "${Qt6_DIR}" CACHE PATH "" FORCE)
            message(STATUS "Auto-detected Qt6 from Qt Installer: ${Qt6_DIR}")
        endif()
    endif()
endif()

# Try to find Qt6 (will use Qt6_DIR if set, or search system paths)
find_package(Qt6 QUIET COMPONENTS Core Widgets Svg DBus Concurrent)

if(NOT Qt6_FOUND)
    message(FATAL_ERROR "
================================================================================
Qt6 not found!

Please install Qt6 using one of these methods:

1. Using vcpkg (recommended):
   - Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git
   - Bootstrap: ./vcpkg/bootstrap-vcpkg.sh
   - Set VCPKG_ROOT: export VCPKG_ROOT=\$PWD/vcpkg
   - Install Qt6: vcpkg install qt6[widgets]
   - Then run cmake again

2. Using Homebrew (macOS):
   - brew install qt@6
   - Then set Qt6_DIR: cmake -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6 ...

3. Using Qt Installer:
   - Download from https://www.qt.io/download
   - Install Qt6
   - Set Qt6_DIR: cmake -DQt6_DIR=/path/to/Qt/6.x.x/macos/lib/cmake/Qt6 ...

Current Qt6_DIR: ${Qt6_DIR}
================================================================================
")
endif()

message(STATUS "Qt6 found: ${Qt6_VERSION}")
message(STATUS "Qt6_DIR: ${Qt6_DIR}")

# Find OpenCV
find_package(OpenCV REQUIRED)

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "
================================================================================
OpenCV not found!

Please install OpenCV using one of these methods:

1. Using Homebrew (macOS):
   - brew install opencv
   - Then run cmake again

2. Using vcpkg:
   - vcpkg install opencv4
   - Then run cmake again

3. Build from source:
   - Download from https://opencv.org/releases/
   - Build and install
   - Set OpenCV_DIR: cmake -DOpenCV_DIR=/path/to/opencv/build ...

Current OpenCV_DIR: ${OpenCV_DIR}
================================================================================
")
endif()

# Check OpenCV version (require 4.5+)
if(OpenCV_VERSION VERSION_LESS "4.5.0")
    message(WARNING "OpenCV version ${OpenCV_VERSION} found, but 4.5.0+ is recommended")
endif()

message(STATUS "OpenCV found: ${OpenCV_VERSION}")
message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")

# Enable Qt's MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source directory structure
set(OCR_ORC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Source files
file(GLOB_RECURSE OCR_ORC_SOURCES
    "${OCR_ORC_SOURCE_DIR}/*.cpp"
)

# Header files (headers are co-located with source files)
file(GLOB_RECURSE OCR_ORC_HEADERS
    "${OCR_ORC_SOURCE_DIR}/*.h"
)

# Executable target
add_executable(ocr-orc
    ${OCR_ORC_SOURCES}
    ${OCR_ORC_HEADERS}
)

# Find Poppler
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)

# Find Tesseract OCR
pkg_check_modules(TESSERACT REQUIRED tesseract)

# Link Qt libraries
target_link_libraries(ocr-orc
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    Qt6::DBus
    Qt6::Concurrent
    ${POPPLER_CPP_LIBRARIES}
    ${TESSERACT_LIBRARIES}
    ${OpenCV_LIBS}
)

# Include Poppler directories
target_include_directories(ocr-orc PRIVATE ${POPPLER_CPP_INCLUDE_DIRS})
target_include_directories(ocr-orc PRIVATE ${TESSERACT_INCLUDE_DIRS})
target_include_directories(ocr-orc PRIVATE ${OpenCV_INCLUDE_DIRS})
target_compile_options(ocr-orc PRIVATE ${POPPLER_CPP_CFLAGS_OTHER})

# Add library search paths for Poppler and Tesseract (macOS Homebrew)
if(APPLE)
    target_link_directories(ocr-orc PRIVATE ${POPPLER_CPP_LIBRARY_DIRS})
    target_link_directories(ocr-orc PRIVATE ${TESSERACT_LIBRARY_DIRS})
endif()

# Set output directory
set_target_properties(ocr-orc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME ocr-orc
)

# Installation configuration
install(TARGETS ocr-orc
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Testing support
enable_testing()
find_package(Qt6 QUIET COMPONENTS Test)
if(Qt6Test_FOUND)
    add_subdirectory(tests)
    message(STATUS "Qt Test framework found - tests enabled")
else()
    message(WARNING "Qt Test framework not found - tests disabled")
endif()

# Platform-specific notes
if(APPLE)
    message(STATUS "Building for macOS")
    set_target_properties(ocr-orc PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "OCR-Orc"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.ocrorc.ocr-orc"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2024 OCR-Orc. All rights reserved."
        MACOSX_BUNDLE_INFO_STRING "OCR-Orc ${PROJECT_VERSION} - PDF Coordinate Calibration Tool"
    )
    
    # Set minimum macOS version (10.15 Catalina)
    set_target_properties(ocr-orc PROPERTIES
        MACOSX_DEPLOYMENT_TARGET "10.15"
    )
    
    # Add custom Info.plist entries
    set_target_properties(ocr-orc PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
elseif(WIN32)
    message(STATUS "Building for Windows")
elseif(UNIX)
    message(STATUS "Building for Linux/Unix")
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== OCR-Orc Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Qt version: ${Qt6_VERSION}")
message(STATUS "Source directory: ${OCR_ORC_SOURCE_DIR}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "==============================")
message(STATUS "")

