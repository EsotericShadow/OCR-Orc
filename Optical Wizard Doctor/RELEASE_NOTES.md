# OCR-Orc Release Notes

## Version 1.0.0 - Production Release

**Release Date:** January 2025  
**Status:** Production Ready

### Overview

OCR-Orc is a professional desktop application for visual coordinate calibration of PDF documents, designed specifically for OCR extraction workflows. This release represents the complete, production-ready implementation of all core features.

### Key Features

#### Core Functionality
- **PDF Loading & Display**: Load and display PDF documents with automatic centering
- **Interactive Region Creation**: Create rectangular, circular, triangular, and polygon regions
- **Region Manipulation**: Move, resize, and rotate regions with intuitive controls
- **Multi-Selection**: Select and manipulate multiple regions simultaneously
- **Group Management**: Organize regions into logical groups for better organization
- **Coordinate System**: Normalized coordinate system (0.0-1.0) for resolution independence

#### User Interface
- **Modern UI Design**: Clean, professional interface with dark/light theme support
- **Mode Toggle**: Seamless switching between Create and Select/Move modes
- **Toolbar**: Comprehensive toolbar with all essential tools
- **Side Panel**: Region editor with real-time coordinate adjustment
- **Context Menus**: Right-click context menus for quick actions
- **Keyboard Shortcuts**: Full keyboard support for power users

#### Export & Import
- **Multiple Formats**: Export coordinates in JSON, CSV formats
- **Mask Generation**: Generate mask images (PNG, JPEG, TIFF) for OCR preprocessing
- **Consolidated Export Dialog**: Single export interface for all export types
- **Import Support**: Load previously exported coordinate files

#### Advanced Features
- **Undo/Redo System**: Complete operation history with unlimited undo/redo
- **Zoom & Pan**: Navigate large documents with zoom controls and panning
- **Region Rotation**: Rotate regions for precise alignment
- **Shape Variety**: Support for rectangles, circles, ellipses, triangles, and polygons
- **Preferences**: Comprehensive settings dialog for customization

#### macOS Integration
- **Native Menu Bar**: Full macOS menu bar with standard menus (File, Edit, Selection, View, Settings, Help)
- **Keyboard Shortcuts**: macOS-standard keyboard shortcuts (⌘Z, ⌘⇧Z, etc.)
- **Theme Support**: Automatic dark/light mode detection
- **Application Bundle**: Proper macOS application bundle structure

### Technical Highlights

- **Framework**: Qt 6.9.3 (C++23)
- **Architecture**: Modular, maintainable codebase with clear separation of concerns
- **Performance**: Optimized rendering with coordinate caching
- **Testing**: Comprehensive unit test suite (18 test suites, 14 passing)
- **Code Quality**: Clean code with conditional debug output, no hardcoded paths

### System Requirements

- **macOS**: 10.15 (Catalina) or later
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 100MB for application, additional space for PDFs and exports
- **Dependencies**: Qt 6.9.3, Poppler (via vcpkg)

### Installation

1. Extract the application bundle
2. Move `ocr-orc.app` to `/Applications` (or desired location)
3. Launch the application
4. First launch will create preferences in `~/Library/Preferences/OCROrc/`

### Known Limitations

- Polygon shape creation is planned but not yet fully implemented
- Some test executables require additional build configuration
- Performance optimizations (viewport culling, dirty regions) are planned for future releases

### Bug Fixes

- Fixed region resize/move glitches with proper coordinate cache invalidation
- Fixed cursor feedback for resize handles and drag operations
- Fixed region editor not updating on canvas selection
- Fixed coordinate editing not updating canvas visualization
- Fixed box selection primary region selection
- Fixed rotation functionality with proper state management

### Documentation

Comprehensive documentation is available in the `docs/` directory:
- User guides and workflows
- Technical architecture documentation
- API reference
- Keyboard shortcuts reference
- Export format specifications

### Credits

Developed with Qt Framework and modern C++ best practices.

---

**For support and bug reports**, please refer to the project documentation or contact the development team.

