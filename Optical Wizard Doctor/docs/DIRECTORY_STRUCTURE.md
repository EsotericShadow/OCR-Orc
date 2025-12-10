# Directory Structure

This document describes the directory structure of the OCR-Orc project.

## Overview

```
OCR-Orc/
├── CMakeLists.txt          # Root CMake configuration
├── vcpkg.json              # vcpkg dependency manifest
├── build/                  # Build output directory (generated)
├── src/                    # Source code
│   ├── main.cpp           # Application entry point
│   ├── core/              # Core business logic
│   ├── models/            # Data structures
│   ├── ui/                # UI components
│   ├── export/            # Export/import functionality
│   └── utils/             # Utility functions
├── include/                # Public headers (if needed)
├── tests/                  # Test files
├── docs/                   # Documentation
└── resources/              # Resources (icons, styles, fonts)
```

## Directory Descriptions

### `src/`
Contains all source code files (.cpp and .h).

#### `src/core/`
Core business logic and algorithms:
- `CoordinateSystem` - Coordinate transformation functions
- `Region` - Region business logic (if class needed)
- `Group` - Group business logic (if class needed)
- `Document` - Document management

#### `src/models/`
Data structures and models:
- `RegionData` - Region data structure
- `GroupData` - Group data structure
- `DocumentState` - Application state management

#### `src/ui/`
UI components (Qt widgets):
- `MainWindow` - Main application window
- `Canvas` - PDF display and region rendering widget
- `Toolbar` - Toolbar component
- `SidePanel` - Side panel component

#### `src/export/`
Export and import functionality:
- `JsonExporter` - JSON export
- `CsvExporter` - CSV export
- `TxtExporter`, `XmlExporter`, `YamlExporter` - Other formats
- `MaskGenerator` - Mask image generation
- Import handlers

#### `src/utils/`
Utility functions:
- `PdfLoader` - PDF to image conversion

### `include/`
Public headers (if needed for library interface). Currently empty as headers are co-located with source files.

### `tests/`
Test files:
- Unit tests for core modules
- Integration tests
- Test CMakeLists.txt

### `docs/`
Documentation:
- Architecture documentation
- API documentation
- This file

### `resources/`
Application resources:
- `icons/` - Icon files
- `styles/` - Stylesheet files
- `fonts/` - Custom fonts (if needed)

## Naming Conventions

### Files
- **Headers**: PascalCase (e.g., `MainWindow.h`)
- **Sources**: PascalCase (e.g., `MainWindow.cpp`)
- **Directories**: lowercase (e.g., `src/core/`)

### Code
- **Classes**: PascalCase (e.g., `CoordinateSystem`)
- **Functions**: camelCase (e.g., `calculateCoordinates`)
- **Variables**: camelCase (e.g., `regionData`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_UNDO_LEVELS`)

## File Organization Principles

1. **Separation of Concerns**: UI, core logic, and utilities are separated
2. **Co-location**: Headers and sources are in the same directory
3. **Logical Grouping**: Related functionality is grouped together
4. **Scalability**: Structure supports future growth

## Future Considerations

- If the project grows, consider moving public headers to `include/`
- Tests mirror the source structure for easy navigation
- Resources are organized by type for easy management

