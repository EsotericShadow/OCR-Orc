# Dependencies

This document lists all dependencies required for building OCR-Orc.

## Required Dependencies

### Qt6
- **Version**: 6.7 or higher
- **Components**: Core, Widgets
- **Purpose**: UI framework for the desktop application
- **Installation Methods**:
  1. **vcpkg** (recommended): `vcpkg install qt6[widgets]`
  2. **Homebrew** (macOS): `brew install qt@6`
  3. **Qt Installer**: Download from https://www.qt.io/download

### Poppler
- **Version**: Latest stable
- **Purpose**: PDF rendering and manipulation
- **Installation**: `vcpkg install poppler`

## Optional Dependencies

### yaml-cpp
- **Purpose**: YAML export format support
- **Installation**: `vcpkg install yaml-cpp`
- **Note**: Can be added later if YAML export is needed

## Dependency Management

### Using vcpkg (Recommended)

1. **Install vcpkg**:
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh  # On macOS/Linux
   # or
   .\bootstrap-vcpkg.bat  # On Windows
   ```

2. **Set VCPKG_ROOT environment variable**:
   ```bash
   export VCPKG_ROOT=$PWD/vcpkg  # Adjust path as needed
   ```

3. **Install dependencies**:
   ```bash
   vcpkg install qt6[widgets] poppler
   ```

4. **Build the project**:
   ```bash
   cmake -B build -S .
   cmake --build build
   ```

### Using Homebrew (macOS only)

```bash
brew install qt@6
```

Then configure CMake with Qt6 path:
```bash
cmake -B build -S . -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6
```

### Using Qt Installer

1. Download Qt6 from https://www.qt.io/download
2. Install Qt6 with Widgets component
3. Configure CMake with Qt6 path:
   ```bash
   cmake -B build -S . -DQt6_DIR=/path/to/Qt/6.x.x/macos/lib/cmake/Qt6
   ```

## Platform-Specific Notes

### macOS
- Qt6 can be installed via Homebrew or Qt Installer
- vcpkg works well for managing all dependencies
- May need to set `CMAKE_OSX_ARCHITECTURES` for Apple Silicon

### Windows
- vcpkg is recommended
- May need to specify triplet: `vcpkg install qt6[widgets]:x64-windows`
- Ensure Visual Studio 2022 or later is installed

### Linux
- Qt6 available via package managers (apt, yum, etc.)
- vcpkg works well for consistent dependency management
- May need development packages: `sudo apt-get install qt6-base-dev`

## Verification

After installing dependencies, verify they are found:

```bash
cmake -B build -S .
```

You should see:
- `Qt6 found: <version>`
- `Using vcpkg toolchain: <path>` (if using vcpkg)

## Troubleshooting

### Qt6 not found
- Check that Qt6_DIR is set correctly
- Verify Qt6 is installed: `find /usr/local /opt -name "Qt6Config.cmake" 2>/dev/null`
- On macOS with Homebrew: `brew list qt@6`

### vcpkg not found
- Ensure VCPKG_ROOT is set: `echo $VCPKG_ROOT`
- Or place vcpkg in a standard location (home directory or /usr/local)

### Poppler not found
- Ensure poppler is installed via vcpkg: `vcpkg list poppler`
- May need poppler-qt6 for Qt integration

## Build Scripts

The project includes build scripts that handle dependency detection:
- `build.sh` (Unix/macOS)
- `build.bat` (Windows)

These scripts will attempt to find dependencies automatically.

