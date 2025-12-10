# Handoff Prompt for New Agent - Optical Wizard Doctor (OWD)

**Date:** December 2024  
**Status:** Phase 4 In Progress - JSON Export Complete  
**Next Agent Role:** Continue Phase 4 Implementation

---

## Executive Summary

You are taking over the **Optical Wizard Doctor (OWD)** project, a C++/Qt6 application for annotating PDF documents with rectangular regions for OCR coordinate extraction. The project is currently in **Phase 4 (Export/Import)**, with JSON export recently completed and tested. Your task is to continue implementing the remaining Phase 4 features: CSV export, JSON import, and mask generation.

**Critical Context:**
- This is a **sequence-dependent** project - phases must be completed in order
- **Always test thoroughly** before moving to next steps
- **Never start the server** unless explicitly instructed
- **Always build after every TypeScript/Next.js change** (though this is a C++ project, the principle applies)
- The user emphasizes **proven methods** and **Stack Overflow citations** for all implementations

---

## Project Overview

### What is Optical Wizard Doctor?

A desktop application (C++/Qt6) that allows users to:
1. Load PDF documents
2. Create rectangular regions on the PDF
3. Organize regions into groups
4. Export coordinates in multiple formats (JSON, CSV, etc.)
5. Import saved coordinate data
6. Generate mask images for OCR processing

### Technology Stack

- **Language:** C++23
- **Framework:** Qt6 (QWidgets, Core, GUI)
- **Build System:** CMake
- **Package Manager:** vcpkg (optional, Qt6 via Homebrew on macOS)
- **PDF Rendering:** Poppler (via `poppler-cpp` bindings)
- **Platform:** macOS (primary), cross-platform support planned

---

## Current Implementation Status

### ✅ Phase 1: Foundation - COMPLETE
- Project setup (CMake, directory structure)
- Core coordinate system (3-tier: Normalized, Image, Canvas)
- Data models (`RegionData`, `GroupData`, `DocumentState`)
- UI framework (MainWindow, Canvas, SidePanel)
- PDF loading and display

### ✅ Phase 2: Interaction - COMPLETE
- Canvas with PDF rendering
- Region creation (click and drag)
- Region selection (single, multi-select, box selection)
- Region movement (drag with Shift modifier)
- Region resize (8 resize handles)
- Zoom (UI buttons, Cmd+Wheel, zoom to cursor)
- Pan/Scroll (middle-click drag, Alt+Left-click drag, wheel scroll)
- Context menus
- Keyboard shortcuts (Cmd+D for duplicate, etc.)

### ✅ Phase 3: Organization - COMPLETE
- Group management (create, add/remove regions)
- Name editing (double-click, context menu, Edit button)
- Color editing (context menu, color dialog)
- Undo/Redo (Cmd+Z, Cmd+Shift+Z, toolbar buttons, 50-level stack)
- State management with snapshot-based undo/redo

### 🚧 Phase 4: Export/Import - IN PROGRESS

**Completed:**
- ✅ JSON Export (`JsonExporter` class)
  - Exports normalized coordinates (0.0-1.0)
  - Includes metadata (PDF path, image size, aspect ratio, version)
  - Atomic file writes (temp file + rename pattern)
  - Integrated into MainWindow with "Export Coordinates" button
  - File dialog with format selection

**Remaining:**
- ⏳ CSV Export (`CsvExporter` class - files exist but need implementation)
- ⏳ JSON Import (`JsonImporter` class - files exist but need implementation)
- ⏳ Mask Generation (`MaskGenerator` class - files exist but need implementation)
- ⏳ Other export formats (TXT, XML, YAML - lower priority)

### ⏳ Phase 5: Polish - NOT STARTED
- Keyboard shortcuts (comprehensive)
- Help system
- Performance optimization
- Theming

---

## Critical Architecture Decisions

### 1. Coordinate System (3-Tier)

**Normalized Coordinates (0.0-1.0) - Source of Truth**
- Always stored in `RegionData.normalizedCoords`
- Used for export/import
- Independent of image/display size
- Format: `{x1, y1, x2, y2}` where all values are 0.0-1.0

**Image Coordinates (Pixels)**
- Calculated from normalized coordinates
- Used for mask generation
- Stored in `RegionData.imageCoords`
- Format: `{x1, y1, x2, y2}` in pixels

**Canvas Coordinates (Display Pixels)**
- Calculated from normalized coordinates
- Used for rendering on screen
- Affected by zoom/pan
- Format: `{x1, y1, x2, y2}` in display pixels

**Key Methods:**
- `DocumentState::synchronizeCoordinates()` - Updates image/canvas coords from normalized
- Always call `synchronizeCoordinates()` after coordinate changes
- Always export/import normalized coordinates only

### 2. State Management

**DocumentState Class** - Central repository for all application state
- Location: `src/models/DocumentState.h/cpp`
- Contains: `regions` (QMap<QString, RegionData>), `groups` (QMap<QString, GroupData>), `pdfPath`, `image`
- Methods: `addRegion()`, `removeRegion()`, `getRegion()`, `createGroup()`, `addRegionToGroup()`, etc.
- Undo/Redo: `saveState()`, `undo()`, `redo()`, `canUndo()`, `canRedo()`

**Undo/Redo Pattern:**
- Snapshot-based (stores complete DocumentState copies)
- 50-level stack limit
- Always call `documentState->saveState()` before state-changing operations
- Stack is cleared on import (fresh start)

### 3. File I/O Pattern

**Atomic Writes:**
- Always write to temp file first (`.tmp` extension)
- Verify write succeeded
- Remove existing file if present
- Rename temp file to final name (atomic operation)
- Clean up temp file on any error

**Example Pattern:**
```cpp
QString tempPath = filePath + ".tmp";
QFile tempFile(tempPath);
// ... write to tempFile ...
tempFile.close();
if (QFile::exists(filePath)) {
    QFile::remove(filePath);
}
QFile::rename(tempPath, filePath);
```

### 4. Export Format Priority

1. **JSON** (Primary) - ✅ Complete
2. **CSV** (Common) - ⏳ Next
3. **JSON Import** - ⏳ After CSV
4. **Mask Generation** - ⏳ After Import
5. **Other Formats** (TXT, XML, YAML) - Lower priority

---

## Key Files and Locations

### Core Models
- `src/models/DocumentState.h/cpp` - Central state management
- `src/models/RegionData.h` - Region data structure
- `src/models/GroupData.h` - Group data structure
- `src/core/CoordinateSystem.h` - Coordinate system definitions

### UI Components
- `src/ui/MainWindow.h/cpp` - Main application window
- `src/ui/Canvas.h/cpp` - PDF display and region rendering/interaction
- `src/ui/SidePanel.h/cpp` - Region/group lists and controls

### Export/Import (Phase 4)
- `src/export/JsonExporter.h/cpp` - ✅ Complete
- `src/export/CsvExporter.h/cpp` - ⏳ Needs implementation
- `src/export/JsonImporter.h/cpp` - ⏳ Needs implementation
- `src/export/MaskGenerator.h/cpp` - ⏳ Needs implementation

### Utilities
- `src/utils/PdfLoader.h/cpp` - PDF loading via Poppler
- `src/utils/InputValidator.h/cpp` - Input validation utilities

### Documentation
- `ORCHESTRATOR_PROMPT.md` - Master orchestrator instructions
- `docs/PHASE_4_RISK_ANALYSIS.md` - Comprehensive risk analysis
- `docs/PHASE_4_PROVEN_METHODS.md` - Proven implementation methods with Stack Overflow citations
- `docs/CODE_REVIEW_REPORT.md` - Code review findings and fixes

---

## Implementation Patterns and Standards

### 1. Naming Conventions
- **Classes:** PascalCase (`DocumentState`, `JsonExporter`)
- **Functions:** camelCase (`exportToFile`, `buildRegionsObject`)
- **Variables:** camelCase (`regionName`, `filePath`)
- **Constants:** UPPER_SNAKE_CASE (`MIN_NORMALIZED`, `MAX_DPI`)
- **Namespaces:** `owd` (Optical Wizard Doctor)

### 2. Code Organization
- Header files in `include/` or alongside `.cpp` files in `src/`
- Implementation files in `src/`
- Tests in `tests/` (when implemented)
- Documentation in `docs/`

### 3. Qt Patterns
- Use Qt's signal/slot mechanism for UI communication
- Use `QFileDialog` for file selection
- Use `QMessageBox` for user notifications
- Use `QJsonDocument` for JSON operations
- Use `QTextStream` with UTF-8 encoding for text files

### 4. Error Handling
- Use exceptions (`std::runtime_error`) for file I/O errors
- Show user-friendly error messages via `QMessageBox`
- Log errors with `qWarning()` or `qDebug()`
- Always clean up resources on error (temp files, etc.)

### 5. Testing Approach
- **User Testing:** User provides test checklists, you track logs
- **Incremental Testing:** Test each feature thoroughly before moving on
- **Regression Testing:** Back-check that new features don't break existing ones
- **Edge Cases:** Test with empty data, large files, special characters, etc.

---

## Known Issues and TODOs

### From Code Review Report

1. **Input Validation** - ✅ Fixed (missing `QList` include)
2. **Code Duplication** - Some long functions could be refactored (low priority)
3. **Debug Statements** - Some `qDebug()` calls remain (acceptable for now)

### Current TODOs

1. **CSV Export Implementation**
   - Files exist: `src/export/CsvExporter.h/cpp`
   - Need to implement RFC 4180 compliant CSV export
   - See `docs/PHASE_4_PROVEN_METHODS.md` for pattern
   - Must escape commas, quotes, newlines
   - Use 6 decimal places for coordinates

2. **JSON Import Implementation**
   - Files exist: `src/export/JsonImporter.h/cpp`
   - Need to implement JSON parsing and validation
   - Must handle coordinate validation (0.0-1.0 range)
   - Must handle duplicate names (auto-rename)
   - Must handle orphaned groups
   - Must call `synchronizeCoordinates()` after import
   - Must clear undo/redo stack on import
   - See `docs/PHASE_4_PROVEN_METHODS.md` for pattern

3. **Mask Generation Implementation**
   - Files exist: `src/export/MaskGenerator.h/cpp`
   - Need to implement mask image generation
   - Must use **image coordinates** (not normalized or canvas)
   - Black background, white rectangles for regions
   - Must match original image dimensions exactly
   - Must export coordinate JSON alongside mask
   - See `docs/PHASE_4_PROVEN_METHODS.md` for pattern

---

## User Preferences and Rules

### Critical Rules

1. **Never start the server** - User explicitly stated "never ever start the server, never ever run processes on local host unless explicitly instructed"
2. **Always build after changes** - Build, lint, and typecheck before delivering
3. **Batch processing is not allowed** - When asked to manually process one by one, do not batch
4. **Admit uncertainty** - Never lie or make up results
5. **Manually check data** - Never assume data contents, ensure data is meaningful
6. **No one-size-fits-all** - Approach each category independently, respect nuances
7. **PDF extraction uses Python** - When extracting data from PDFs, always use Python, not JavaScript
8. **Test thoroughly** - Always test and ensure we're using the right stuff before moving on

### Testing Workflow

1. User requests to "run it"
2. You build the application
3. You provide a test checklist
4. User tests and provides feedback
5. You read logs and address issues
6. Repeat until feature is complete

### Code Quality Standards

- **Codacy Analysis:** Run `codacy_cli_analyze` after every file edit
- **Build Success:** Project must build without errors
- **Linting:** Fix all linter errors
- **Type Checking:** Ensure type safety

---

## Phase 4 Implementation Guide

### Next Steps (In Order)

1. **CSV Export** (Highest Priority)
   - Reference: `docs/PHASE_4_PROVEN_METHODS.md` section 3
   - Pattern: RFC 4180 compliant CSV with proper escaping
   - Files: `src/export/CsvExporter.h/cpp` (already exist)
   - Integration: Add CSV option to export file dialog in `MainWindow.cpp`
   - Testing: Export CSV, open in Excel/Google Sheets/LibreOffice, verify format

2. **JSON Import** (After CSV)
   - Reference: `docs/PHASE_4_PROVEN_METHODS.md` section 2
   - Pattern: Parse JSON, validate structure, restore state
   - Files: `src/export/JsonImporter.h/cpp` (already exist)
   - Integration: Add "Import Coordinates" button to MainWindow
   - Testing: Round-trip test (export → import → export, compare files)

3. **Mask Generation** (After Import)
   - Reference: `docs/PHASE_4_PROVEN_METHODS.md` section 5
   - Pattern: QImage + QPainter, use image coordinates
   - Files: `src/export/MaskGenerator.h/cpp` (already exist)
   - Integration: Add "Export Mask" button to MainWindow
   - Testing: Generate mask, verify dimensions match image, verify regions are white

### Risk Mitigation (From PHASE_4_RISK_ANALYSIS.md)

**Critical Risks to Address:**

1. **Coordinate System Confusion**
   - ✅ JSON Export uses normalized coordinates (correct)
   - ⚠️ CSV Export must use normalized coordinates
   - ⚠️ Mask Generation must use **image coordinates** (not normalized!)

2. **Round-Trip Data Loss**
   - ⚠️ JSON Import must validate coordinates (0.0-1.0)
   - ⚠️ JSON Import must handle duplicate names
   - ⚠️ JSON Import must reconstruct groups correctly

3. **File I/O Errors**
   - ✅ JSON Export uses atomic writes (correct)
   - ⚠️ CSV Export must use atomic writes
   - ⚠️ Mask Generation must handle file errors gracefully

4. **Mask Coordinate Synchronization**
   - ⚠️ Must call `synchronizeCoordinates()` before mask generation
   - ⚠️ Must validate image exists before generating mask

---

## Proven Methods Reference

All Phase 4 implementations should follow patterns documented in `docs/PHASE_4_PROVEN_METHODS.md`:

1. **JSON Export** - ✅ Complete (Qt QJsonDocument)
2. **JSON Import** - Qt QJsonDocument::fromJson with validation
3. **CSV Export** - RFC 4180 standard with proper escaping
4. **File I/O** - Atomic write pattern (temp file + rename)
5. **Mask Generation** - QImage + QPainter with image coordinates
6. **File Dialogs** - QFileDialog::getSaveFileName/getOpenFileName

**All methods are proven** - No custom solutions required. All backed by Stack Overflow citations and Qt documentation.

---

## Build and Run Instructions

### Build Commands

```bash
cd "/Users/main/Desktop/OWD-OCR_Coordinates/Optical Wizard Doctor"
./build.sh
```

### Run Command

```bash
build/bin/owd
```

### Build System

- **CMake:** Configured in `CMakeLists.txt`
- **Qt6:** Found via Homebrew on macOS
- **Poppler:** Found via Homebrew on macOS
- **Output:** `build/bin/owd.app/Contents/MacOS/owd` (macOS bundle)

---

## Testing Checklist Template

When implementing new features, provide a test checklist like this:

```markdown
## Test Checklist for [Feature Name]

### Basic Functionality
- [ ] Test 1: Description
- [ ] Test 2: Description

### Edge Cases
- [ ] Test 3: Description
- [ ] Test 4: Description

### Integration
- [ ] Test 5: Description
- [ ] Test 6: Description

### Regression
- [ ] Test 7: Verify existing feature X still works
- [ ] Test 8: Verify existing feature Y still works
```

---

## Communication Style

- **Be direct and to the point** - User prefers concise communication
- **Cite code references** - Use `startLine:endLine:filepath` format for existing code
- **Show progress** - Update TODOs as you work
- **Ask for clarification** - If unsure, ask rather than guess
- **Document decisions** - Explain why you chose a particular approach

---

## Success Criteria for Phase 4

- [x] JSON Export works correctly
- [ ] CSV Export works and opens correctly in Excel/Google Sheets/LibreOffice
- [ ] JSON Import restores state correctly (round-trip test passes)
- [ ] Mask Generation produces correct mask images (black background, white regions)
- [ ] All exports use normalized coordinates (except mask uses image coordinates)
- [ ] All file operations use atomic writes
- [ ] Error handling is robust (permissions, disk full, invalid files)
- [ ] UI integration is seamless (file dialogs, success/error messages)

---

## Important Notes

1. **Sequence Matters** - Complete CSV export before JSON import (easier to test)
2. **Coordinate System Discipline** - Always use normalized for export/import, image for mask
3. **Test Round-Trips** - Export → Import → Export, compare files
4. **User Testing** - User will test and provide feedback, you track logs
5. **Incremental Progress** - One feature at a time, test thoroughly before moving on
6. **Proven Methods Only** - Use documented patterns, no custom solutions

---

## Questions to Ask User

If you're unsure about:
- Implementation approach → Check `docs/PHASE_4_PROVEN_METHODS.md` first
- Risk mitigation → Check `docs/PHASE_4_RISK_ANALYSIS.md` first
- Architecture decisions → Check this document and `ORCHESTRATOR_PROMPT.md`
- Testing approach → Provide checklist, user will test and provide feedback

---

## Final Reminders

1. **Always build after changes** - Never deliver without building
2. **Run Codacy analysis** - After every file edit
3. **Test incrementally** - One feature at a time
4. **Use proven methods** - Reference Stack Overflow and Qt docs
5. **Coordinate system discipline** - Normalized for export/import, image for mask
6. **Atomic file writes** - Always use temp file + rename pattern
7. **User testing workflow** - Build → Provide checklist → User tests → Fix issues → Repeat

---

**You are ready to continue Phase 4 implementation. Start with CSV Export, then JSON Import, then Mask Generation. Good luck!**

