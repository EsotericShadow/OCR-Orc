# Directory Structure - Implementation Steps

## Objective

Create a logical, maintainable directory structure for the source code.

## Prerequisites

- CMake configuration complete
- Dependencies installed

## Target Structure

```
OCR-Orc/
├── CMakeLists.txt
├── vcpkg.json
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── CoordinateSystem.h
│   │   ├── CoordinateSystem.cpp
│   │   ├── Region.h
│   │   ├── Region.cpp
│   │   ├── Group.h
│   │   ├── Group.cpp
│   │   ├── Document.h
│   │   └── Document.cpp
│   ├── models/
│   │   ├── RegionData.h
│   │   ├── GroupData.h
│   │   └── DocumentState.h
│   ├── ui/
│   │   ├── MainWindow.h
│   │   ├── MainWindow.cpp
│   │   ├── Canvas.h
│   │   ├── Canvas.cpp
│   │   ├── Toolbar.h
│   │   ├── Toolbar.cpp
│   │   ├── SidePanel.h
│   │   └── SidePanel.cpp
│   ├── export/
│   │   ├── JsonExporter.h
│   │   ├── JsonExporter.cpp
│   │   └── ...
│   └── utils/
│       ├── PdfLoader.h
│       └── PdfLoader.cpp
├── include/
│   └── (public headers if needed)
├── tests/
│   ├── CMakeLists.txt
│   ├── test_coordinate_system.cpp
│   └── ...
├── docs/
│   └── (documentation)
└── resources/
    └── (icons, stylesheets, etc.)
```

## Steps

### Step 1: Create Source Directories

1. Create `src/` directory
2. Create `src/core/` for core business logic
3. Create `src/models/` for data structures
4. Create `src/ui/` for UI components
5. Create `src/export/` for export/import functionality
6. Create `src/utils/` for utility functions

### Step 2: Create Header Directories

1. Create `include/` for public headers (if needed)
2. For now, headers can live alongside .cpp files

### Step 3: Create Test Directory

1. Create `tests/` directory
2. Create `tests/CMakeLists.txt` (empty for now)
3. Add placeholder test file

### Step 4: Create Documentation Directory

1. Create `docs/` directory
2. Add README.md with project overview
3. Add architecture documentation (later)

### Step 5: Create Resources Directory

1. Create `resources/` directory
2. Create subdirectories:
   - `resources/icons/` for icons
   - `resources/styles/` for stylesheets
   - `resources/fonts/` for custom fonts (if needed)

### Step 6: Create Main Entry Point

1. Create `src/main.cpp`
2. Add minimal Qt application:
   ```cpp
   #include <QApplication>
   #include <QMainWindow>
   
   int main(int argc, char *argv[]) {
       QApplication app(argc, argv);
       QMainWindow window;
       window.setWindowTitle("OCR-Orc");
       window.resize(1200, 800);
       window.show();
       return app.exec();
   }
   ```

### Step 7: Update CMakeLists.txt

1. Add source files to executable target
2. Set include directories
3. Add resource files (if any)

### Step 8: Create Placeholder Files

1. Create empty header and source files for each module
2. Add basic includes and namespace declarations
3. Add TODO comments for implementation

### Step 9: Verify Structure

1. Build the project
2. Ensure all directories are created
3. Verify no missing file errors

### Step 10: Document Structure

1. Create `docs/DIRECTORY_STRUCTURE.md`
2. Explain the purpose of each directory
3. Document naming conventions

## Acceptance Criteria

- [ ] All directories are created
- [ ] Main application compiles and runs
- [ ] Placeholder files are in place
- [ ] Directory structure is documented
- [ ] No build errors

## Naming Conventions

- Headers: PascalCase (e.g., `MainWindow.h`)
- Sources: PascalCase (e.g., `MainWindow.cpp`)
- Classes: PascalCase (e.g., `CoordinateSystem`)
- Functions: camelCase (e.g., `calculateCoordinates`)
- Variables: camelCase (e.g., `regionData`)
- Constants: UPPER_SNAKE_CASE (e.g., `MAX_UNDO_LEVELS`)

## Notes

- Keep related files together
- Separate concerns (UI, core logic, utilities)
- Make it easy to find files
- Consider future growth

