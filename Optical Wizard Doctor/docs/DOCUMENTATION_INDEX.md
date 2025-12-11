# Documentation Index

## Quick Reference

This index provides a guide to all documentation files in the OCR-Orc specification package.

## Documentation Files

### 1. README.md
**Purpose:** Overview and introduction to the application

**Contents:**
- Application overview
- Purpose and target use case
- Key features
- Documentation structure
- Status information

**Read this first** to understand what OCR-Orc is and what it does.

---

### 2. SOFTWARE_DESIGN_SPECIFICATION.md
**Purpose:** High-level architecture and design specifications

**Contents:**
- Application architecture
- Component structure
- Design patterns
- Core data structures
- Coordinate system architecture
- UI component specifications
- Event handling architecture
- Undo/Redo system design
- Export/Import system design
- Performance requirements
- Error handling requirements
- Platform requirements

**Use this for:** Understanding the overall system design and architecture decisions.

---

### 3. FUNCTIONAL_REQUIREMENTS.md
**Purpose:** Detailed feature requirements and specifications

**Contents:**
- Document loading and display requirements
- Region creation and management requirements
- Group management requirements
- Zoom and navigation requirements
- Visual feedback requirements
- Export and import requirements
- Undo/Redo requirements
- Keyboard shortcuts requirements
- Error handling requirements
- Performance requirements

**Use this for:** Understanding what features must be implemented and how they should behave.

---

### 4. USER_INTERFACE_SPECIFICATION.md
**Purpose:** Complete UI component and layout specifications

**Contents:**
- Window layout and dimensions
- Component hierarchy
- Toolbar components
- Control panel components
- Canvas area specifications
- Side panel components
- Help window specifications
- Dialog windows
- Visual feedback systems
- Color scheme
- Responsive design requirements

**Use this for:** Building the user interface exactly as specified.

---

### 5. FUNCTION_REFERENCE.md
**Purpose:** Complete function-by-function documentation

**Contents:**
- All 78 functions documented
- Function purposes
- Parameters and return values
- Behavior specifications
- Dependencies
- Keyboard shortcuts
- Called functions
- State changes

**Use this for:** Implementing each function with exact specifications.

---

### 6. DATA_STRUCTURES.md
**Purpose:** Data model and structure specifications

**Contents:**
- Region data structure
- Groups data structure
- Application state structure
- Export data structures (JSON, CSV, TXT, XML, YAML)
- Color mapping
- Resize handle positions
- Visual state indicators
- Mask image structure
- Import format compatibility
- Data validation rules

**Use this for:** Understanding data models and implementing data storage.

---

### 7. EXPORT_FORMATS.md
**Purpose:** Complete export format specifications

**Contents:**
- JSON format specification
- CSV format specification
- TXT format specification
- XML format specification
- YAML format specification
- Mask image export specification
- Export requirements
- Import compatibility
- Export workflow
- Format selection guidelines

**Use this for:** Implementing export/import functionality.

---

### 8. COORDINATE_SYSTEM.md
**Purpose:** Coordinate system architecture and usage

**Contents:**
- Three coordinate systems explained
- Coordinate transformation rules
- Coordinate synchronization requirements
- Coordinate validation rules
- Export format (normalized only)
- Usage in OCR pipeline
- Aspect ratio requirements

**Use this for:** Understanding and implementing the coordinate system.

---

### 9. KEYBOARD_SHORTCUTS.md
**Purpose:** Complete keyboard shortcut reference

**Contents:**
- Mode switching shortcuts
- File operation shortcuts
- Zoom operation shortcuts
- Region operation shortcuts
- Selection operation shortcuts
- Group operation shortcuts
- Undo/Redo shortcuts
- Mouse interactions
- Mouse wheel behaviors

**Use this for:** Implementing keyboard shortcuts and mouse interactions.

---

### 10. WORKFLOW_DOCUMENTATION.md
**Purpose:** User workflows and use cases

**Contents:**
- Initial setup workflow
- Region creation workflow
- Region editing workflow
- Selection workflow
- Group management workflow
- Navigation workflow
- Export workflow
- Import workflow
- Quality assurance workflow
- Complete calibration workflow
- Common workflow patterns
- Error recovery workflow
- Best practices
- Integration workflow
- Maintenance workflow

**Use this for:** Understanding how users will interact with the application.

---

## Additional Documentation

### Production & Testing
- **PRODUCTION_CHECKLIST.md** - Production readiness checklist
- **TEST_CHECKLIST.md** - Region feature test checklist

### Root Level Documentation
Production release information is located in the `Optical Wizard Doctor/` root directory:
- **RELEASE_NOTES.md** - Release notes for v1.0.0
- **BUILD_INSTRUCTIONS.md** - Build and compilation instructions
- **CHANGELOG.md** - Version history

### Archived Documentation
Development, planning, and historical documentation has been moved to `archive/` for reference. This includes:
- Analysis and research documents
- Planning documents
- Deployment and distribution docs
- Implementation plans
- Phase reports
- Debug documentation

See `archive/README.md` for a complete list of archived documents.

## Documentation Navigation Guide

### For Architects and Designers

**Start with:**
1. README.md (root) - Understand the application
2. SOFTWARE_DESIGN_SPECIFICATION.md - Understand architecture
3. FUNCTIONAL_REQUIREMENTS.md - Understand features

**Then review:**
- USER_INTERFACE_SPECIFICATION.md - UI design
- DATA_STRUCTURES.md - Data models
- COORDINATE_SYSTEM.md - Core system

### For Developers

**Start with:**
1. README.md - Application overview
2. SOFTWARE_DESIGN_SPECIFICATION.md - Architecture
3. FUNCTION_REFERENCE.md - Function specifications

**Then review:**
- DATA_STRUCTURES.md - Data models
- COORDINATE_SYSTEM.md - Coordinate transformations
- EXPORT_FORMATS.md - Export implementations
- KEYBOARD_SHORTCUTS.md - Event handling

### For UI Developers

**Start with:**
1. USER_INTERFACE_SPECIFICATION.md - Complete UI specs
2. FUNCTIONAL_REQUIREMENTS.md - Feature requirements
3. WORKFLOW_DOCUMENTATION.md - User workflows

**Then review:**
- KEYBOARD_SHORTCUTS.md - Interactions
- SOFTWARE_DESIGN_SPECIFICATION.md - Component structure

### For QA/Testing

**Start with:**
1. FUNCTIONAL_REQUIREMENTS.md - What to test
2. WORKFLOW_DOCUMENTATION.md - How users will use it
3. KEYBOARD_SHORTCUTS.md - All interactions

**Then review:**
- EXPORT_FORMATS.md - Export validation
- COORDINATE_SYSTEM.md - Coordinate validation

### For Integration Developers

**Start with:**
1. EXPORT_FORMATS.md - Export formats
2. COORDINATE_SYSTEM.md - Coordinate usage
3. WORKFLOW_DOCUMENTATION.md - Integration workflow

**Then review:**
- DATA_STRUCTURES.md - Data models
- FUNCTION_REFERENCE.md - Export functions

## Key Concepts Quick Reference

### Coordinate Systems
- **Normalized (0.0-1.0):** Source of truth, resolution-independent
- **Image (pixels):** Pixel positions in original image
- **Canvas (display):** Screen positions for rendering

See: **COORDINATE_SYSTEM.md**

### Data Models
- **Region:** Dictionary with 3 coordinate systems, color, group
- **Groups:** Dictionary mapping group names to region lists
- **State:** Document, selection, operation, undo/redo state

See: **DATA_STRUCTURES.md**

### UI Components
- **Toolbar:** Mode, zoom, groups, undo/redo
- **Control Panel:** File ops, region input, actions
- **Canvas:** PDF display, region visualization
- **Side Panel:** Lists, info, help

See: **USER_INTERFACE_SPECIFICATION.md**

### Export Formats
- **JSON:** Primary format, structured data
- **CSV:** Spreadsheet-friendly
- **TXT:** Human-readable
- **XML:** Structured XML
- **YAML:** YAML format
- **Mask:** Black/white image for OCR

See: **EXPORT_FORMATS.md**

### Key Features
- Region creation and editing
- Multi-selection
- Group management
- Zoom and navigation
- Undo/Redo (50 levels)
- Multi-format export
- Mask generation

See: **FUNCTIONAL_REQUIREMENTS.md**

## Implementation Checklist

### Phase 1: Core Infrastructure
- [ ] Window and UI framework setup
- [ ] Coordinate system implementation
- [ ] Data structure implementations
- [ ] Basic canvas rendering

### Phase 2: Document Handling
- [ ] PDF loading
- [ ] Image display
- [ ] Zoom functionality
- [ ] Scroll functionality

### Phase 3: Region Management
- [ ] Region creation
- [ ] Region selection
- [ ] Region movement
- [ ] Region resizing
- [ ] Region editing
- [ ] Region deletion

### Phase 4: Advanced Features
- [ ] Multi-selection
- [ ] Group management
- [ ] Undo/Redo system
- [ ] Visual feedback (hover, selection)

### Phase 5: Export/Import
- [ ] JSON export
- [ ] CSV export
- [ ] TXT export
- [ ] XML export
- [ ] YAML export
- [ ] Mask export
- [ ] JSON import

### Phase 6: Polish
- [ ] Keyboard shortcuts
- [ ] Tooltips
- [ ] Help system
- [ ] Error handling
- [ ] Performance optimization

## Questions and Answers

### Q: What coordinate system should I use for exports?
**A:** Always use normalized coordinates (0.0-1.0). See COORDINATE_SYSTEM.md.

### Q: How many coordinate systems are maintained?
**A:** Three: normalized, image, and canvas. All must be kept in sync. See COORDINATE_SYSTEM.md.

### Q: What's the maximum undo history?
**A:** 50 states. See SOFTWARE_DESIGN_SPECIFICATION.md section 6.

### Q: What export formats are required?
**A:** JSON, CSV, TXT, XML, YAML. See EXPORT_FORMATS.md.

### Q: How does the mask work?
**A:** Black image with white rectangles for regions. See EXPORT_FORMATS.md section 6.

### Q: What's the zoom range?
**A:** 20% (0.2x) to 500% (5.0x). See FUNCTIONAL_REQUIREMENTS.md section 4.

### Q: How does multi-selection work?
**A:** Command+Click toggles, Shift+Drag for box selection. See FUNCTIONAL_REQUIREMENTS.md section 2.3.

### Q: What happens when a region is moved?
**A:** All three coordinate systems are updated. See FUNCTION_REFERENCE.md move_region().

## Version Information

**Documentation Version:** 1.0  
**Prototype Version:** Python/Tkinter  
**Target Version:** C++ Production  
**Last Updated:** Based on prototype analysis

## Contact and Support

For questions about this specification:
- Refer to specific documentation files
- Check FUNCTION_REFERENCE.md for function details
- Review WORKFLOW_DOCUMENTATION.md for usage patterns
- See SOFTWARE_DESIGN_SPECIFICATION.md for architecture questions

## Document Maintenance

**When to Update:**
- New features added
- Requirements change
- Implementation reveals issues
- User feedback indicates changes needed

**Update Process:**
1. Update relevant documentation file
2. Update this index if structure changes
3. Maintain version information
4. Keep all documents synchronized

