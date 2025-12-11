# OCR-Orc

## Overview

OCR-Orc is a visual coordinate calibration tool designed to facilitate accurate OCR (Optical Character Recognition) extraction from scanned PDF documents, particularly those containing handwritten forms with character cells.

## Purpose

The primary purpose of OCR-Orc is to allow users to:

1. **Visually define extraction regions** on PDF documents by drawing bounding boxes around specific areas (e.g., individual character cells, form fields)
2. **Calibrate coordinate systems** using normalized percentages (0.0-1.0) that work across different document resolutions
3. **Export coordinate data** in multiple formats (JSON, CSV, TXT, XML, YAML) for use in automated OCR extraction pipelines
4. **Generate mask images** that black out everything except labeled regions, improving OCR accuracy by eliminating visual noise

## Target Use Case

OCR-Orc is specifically designed for processing scanned handwritten forms where:
- Forms contain individual character cells (like government ID forms)
- OCR accuracy is critical and needs to be optimized
- Multiple documents of the same form type need to be processed consistently
- Coordinate calibration must be reusable across different scan resolutions

## Key Features

- **Interactive Region Selection**: Click and drag to define extraction regions
- **Multi-Selection Support**: Select, move, and manipulate multiple regions simultaneously
- **Group Management**: Organize regions into logical groups (e.g., "lastname", "firstname")
- **Normalized Coordinates**: Export coordinates as percentages (0.0-1.0) for resolution independence
- **Multiple Export Formats**: JSON, CSV, TXT, XML, YAML
- **Mask Generation**: Create black/white mask images for OCR preprocessing
- **Undo/Redo System**: Full history tracking for all operations
- **Zoom and Pan**: Navigate large documents with zoom and scroll controls
- **Visual Feedback**: Hover highlighting, selection indicators, resize handles

## Application Type

OCR-Orc is a **desktop GUI application** that provides a visual interface for coordinate calibration. It is designed to be:
- Standalone (no server required)
- Cross-platform compatible
- Responsive and intuitive
- Professional-grade for production use

## Documentation Structure

This documentation package contains:

1. **README.md** (this file) - Overview and introduction
2. **SOFTWARE_DESIGN_SPECIFICATION.md** - High-level architecture and design
3. **FUNCTIONAL_REQUIREMENTS.md** - Detailed feature requirements
4. **USER_INTERFACE_SPECIFICATION.md** - UI components and layout
5. **FUNCTION_REFERENCE.md** - Complete function documentation
6. **DATA_STRUCTURES.md** - Data models and internal structures
7. **EXPORT_FORMATS.md** - Export format specifications
8. **WORKFLOW_DOCUMENTATION.md** - User workflows and use cases
9. **KEYBOARD_SHORTCUTS.md** - Complete hotkey reference
10. **COORDINATE_SYSTEM.md** - Coordinate system explanation

## Status

**Current Version**: 1.0.0 - Production Release (C++/Qt6)  
**Status**: ✅ Production Ready  
**Platform**: macOS (with cross-platform support planned)

## Recent Updates (v1.0.0)

- ✅ Complete rotation functionality for regions
- ✅ Comprehensive unit test suite
- ✅ Production-ready build configuration
- ✅ Clean codebase with conditional debug output
- ✅ Full macOS menu bar integration
- ✅ Modern UI with theme support
- ✅ Consolidated export dialog
- ✅ Real-time coordinate editing

## Quick Start

1. **Load a PDF**: File → Load PDF (or ⌘O)
2. **Create Regions**: Toggle to "Create" mode, click and drag to define regions
3. **Edit Regions**: Toggle to "Select/Move" mode, select regions to edit
4. **Export Coordinates**: Click Export button, choose format and options
5. **Generate Masks**: Use Export dialog to generate mask images for OCR

## Building from Source

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build

# Run
./build/bin/ocr-orc.app/Contents/MacOS/ocr-orc
```

## Testing

```bash
# Run all tests
cd build && ctest --output-on-failure

# Run specific test
./tests/test_canvas_rotation
```

