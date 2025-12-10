# Changelog

All notable changes to OCR-Orc will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-01-XX

### Added
- **Rotation Functionality**: Complete rotation system for regions
  - Rotate icon at bottom-right corner of selected regions
  - Real-time rotation during drag operations
  - Visual rotation feedback
  - Proper cursor changes when hovering over rotate icon
- **Shape Variety**: Support for multiple region shapes
  - Rectangle (default)
  - Circle/Ellipse
  - Triangle
  - Polygon (planned, not yet fully implemented)
- **Context Menu**: Right-click context menu for shape selection in Create mode
- **Comprehensive Test Suite**: Unit tests for rotation functionality
  - Rotate icon hit detection tests
  - Rotation angle calculation tests
  - Rotation state management tests
- **Production Documentation**:
  - RELEASE_NOTES.md
  - PRODUCTION_CHECKLIST.md
  - Updated README.md with current status

### Changed
- **Namespace Rebranding**: Complete rebrand from `owd` to `ocr_orc` namespace
- **Debug Code**: All debug statements now use conditional compilation
  - `OCR_ORC_DEBUG` macro only compiles in debug builds
  - Production builds have zero debug overhead
- **Export Consolidation**: All export functionality consolidated into single Export dialog
- **Mode Toggle**: Redesigned mode selector as toggle slider with custom icons
- **UI Improvements**:
  - Coordinate editor with +/- buttons for real-time adjustment
  - Black background with neon blue numbers for coordinate display
  - Improved spacing and layout consistency
  - Theme-aware icons (white on dark, black on light)

### Fixed
- Region resize/move glitches with proper coordinate cache invalidation
- Cursor feedback for resize handles and drag operations
- Region editor not updating on canvas selection
- Coordinate editing not updating canvas visualization
- Box selection primary region selection
- Rotation functionality with proper state management
- Resize handles persisting on previously selected regions
- Region editor not updating after move/resize operations

### Technical
- **Build System**: Production-ready CMake configuration
- **Code Quality**: All debug code properly conditional
- **Testing**: 14/14 rotation tests passing
- **Documentation**: Comprehensive production documentation
- **Git Configuration**: Updated .gitignore for production

### Known Issues
- Polygon shape creation not fully implemented (planned feature)
- Some test executables need build configuration fixes (non-critical)
- Performance optimizations (viewport culling) planned for future release

---

## [Unreleased]

### Planned
- Full polygon shape implementation
- Performance optimizations (viewport culling, dirty regions)
- Additional export formats
- Cross-platform support (Windows, Linux)

