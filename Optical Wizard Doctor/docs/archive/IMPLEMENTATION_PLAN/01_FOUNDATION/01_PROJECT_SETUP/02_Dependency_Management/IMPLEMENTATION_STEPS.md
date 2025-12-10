# Dependency Management - Implementation Steps

## Objective

Set up vcpkg for dependency management and install required libraries.

## Prerequisites

- vcpkg installed and configured
- CMake configuration from previous step

## Required Dependencies

1. **Qt6** - UI framework
   - Components: Core, Widgets
   - Optional: PDF (if available)

2. **Poppler** - PDF rendering
   - Poppler-qt6 for Qt integration

3. **yaml-cpp** - YAML export support (optional, can add later)

## Steps

### Step 1: Install vcpkg (if not already installed)

1. Clone vcpkg repository
2. Run bootstrap script (bootstrap-vcpkg.sh on Unix, bootstrap-vcpkg.bat on Windows)
3. Add vcpkg to PATH or set VCPKG_ROOT environment variable

### Step 2: Create vcpkg.json Manifest

1. Create `vcpkg.json` in project root
2. Define dependencies:
   ```json
   {
     "dependencies": [
       {
         "name": "qt6",
         "features": ["widgets"]
       },
       "poppler"
     ]
   }
   ```
3. Set minimum baseline version

### Step 3: Configure CMake to Use vcpkg

1. Ensure CMAKE_TOOLCHAIN_FILE points to vcpkg
2. Test that Qt6 is found via vcpkg

### Step 4: Install Dependencies

1. Run: `vcpkg install` (or `vcpkg install --triplet x64-windows` on Windows)
2. Wait for compilation (this may take 30-60 minutes)
3. Verify packages are installed

### Step 5: Verify Qt6 Installation

1. Check that Qt6::Core and Qt6::Widgets are available
2. Test linking in CMake configuration
3. Verify Qt version: `Qt6_VERSION`

### Step 6: Verify Poppler Installation

1. Check that Poppler is found
2. Test basic Poppler functionality (create a simple test)

### Step 7: Handle Optional Dependencies

1. Make yaml-cpp optional (use if available, skip if not)
2. Add feature flags for optional components

### Step 8: Document Dependencies

1. Create `DEPENDENCIES.md` listing all dependencies
2. Document installation process
3. Note any platform-specific requirements

### Step 9: Test Dependency Integration

1. Create a minimal Qt application
2. Test Poppler PDF loading
3. Verify everything links correctly

## Acceptance Criteria

- [ ] vcpkg is installed and configured
- [ ] All required dependencies are installed
- [ ] CMake finds all dependencies
- [ ] A minimal Qt application compiles and runs
- [ ] Poppler can load a test PDF
- [ ] Dependencies are documented

## Troubleshooting

- If Qt6 not found, check vcpkg triplet and Qt6 installation
- If Poppler not found, ensure poppler-qt6 is installed
- On Windows, may need to specify triplet explicitly

## Notes

- vcpkg compilation can take a long time; be patient
- Consider caching vcpkg packages for CI/CD
- Document any manual dependency installations needed

