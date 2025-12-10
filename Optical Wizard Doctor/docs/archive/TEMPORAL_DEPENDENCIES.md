# Temporal Dependencies - Implementation Sequence and Dependencies
## What Must Be Built Before What

**Purpose**: Define the order of implementation, dependencies between components, and what can be built in parallel.

---

## Dependency Graph

```
Phase 1: Foundation
    │
    ├── Project Setup (no dependencies)
    │   ├── CMake Configuration
    │   ├── Dependency Management
    │   └── Directory Structure
    │
    ├── Core Modules (depends on: Project Setup)
    │   ├── Coordinate System (no internal dependencies)
    │   └── Data Models (depends on: Coordinate System)
    │
    └── UI Framework (depends on: Project Setup, Core Modules)
        ├── Window Setup
        └── Basic Layout (depends on: Window Setup)

Phase 2: Interaction (depends on: Phase 1)
    │
    ├── Canvas (depends on: UI Framework, Core Modules)
    │   ├── PDF Display (depends on: PdfLoader utility)
    │   └── Region Rendering (depends on: PDF Display, Coordinate System)
    │
    ├── Region Operations (depends on: Canvas, Region Rendering)
    │   ├── Creation (depends on: Region Rendering)
    │   ├── Selection (depends on: Region Rendering)
    │   ├── Movement (depends on: Selection)
    │   └── Resize (depends on: Selection)
    │
    └── Navigation (depends on: Canvas)
        ├── Zoom (depends on: PDF Display)
        └── Pan/Scroll (depends on: Zoom)

Phase 3: Organization (depends on: Phase 2)
    │
    ├── Groups (depends on: Region Operations)
    │   └── Group Management (depends on: Selection)
    │
    ├── Editing (depends on: Selection)
    │   ├── Name Editing
    │   └── Color Editing
    │
    └── Undo/Redo (depends on: All Region Operations)
        └── State Management (depends on: Data Models)

Phase 4: Export/Import (depends on: Phase 3)
    │
    ├── Export (depends on: Data Models)
    │   ├── JSON (depends on: Data Models)
    │   ├── CSV (depends on: Data Models)
    │   └── Other Formats (depends on: JSON)
    │
    ├── Import (depends on: Export JSON)
    │   └── JSON Import (depends on: Data Models)
    │
    └── Mask (depends on: Data Models, PDF Display)
        └── Mask Generation (depends on: Image Coordinates)

Phase 5: Polish (depends on: Phase 4)
    │
    ├── Keyboard Shortcuts (depends on: All Features)
    ├── Help System (depends on: All Features)
    ├── Performance (depends on: All Features)
    └── Theming (depends on: UI Framework)
```

---

## Critical Path (Must Build in Order)

### Foundation Path

1. **CMake Configuration** → No dependencies
2. **Dependency Management** → Depends on: CMake
3. **Directory Structure** → Depends on: CMake, Dependencies
4. **Coordinate System** → Depends on: Directory Structure
5. **Data Models** → Depends on: Coordinate System
6. **Window Setup** → Depends on: Directory Structure, Qt
7. **Basic Layout** → Depends on: Window Setup

### Core Functionality Path

8. **PDF Display** → Depends on: Basic Layout, Poppler
9. **Region Rendering** → Depends on: PDF Display, Coordinate System
10. **Region Creation** → Depends on: Region Rendering
11. **Region Selection** → Depends on: Region Rendering
12. **Region Movement** → Depends on: Selection
13. **Region Resize** → Depends on: Selection
14. **Zoom** → Depends on: PDF Display
15. **Pan/Scroll** → Depends on: Zoom

### Organization Path

16. **Group Management** → Depends on: Selection
17. **Name Editing** → Depends on: Selection
18. **Color Editing** → Depends on: Selection
19. **Undo/Redo** → Depends on: All Region Operations

### Export Path

20. **JSON Export** → Depends on: Data Models
21. **CSV Export** → Depends on: Data Models
22. **JSON Import** → Depends on: JSON Export
23. **Mask Generation** → Depends on: Data Models, PDF Display

### Polish Path

24. **Keyboard Shortcuts** → Depends on: All Features
25. **Help System** → Depends on: All Features
26. **Performance** → Depends on: All Features
27. **Theming** → Depends on: UI Framework

---

## Parallelizable Components

### Can Build in Parallel (After Dependencies Met)

**Phase 1**:
- Coordinate System + Window Setup (after Project Setup)
- Data Models + Basic Layout (after Coordinate System + Window Setup)

**Phase 2**:
- PDF Display + Region Rendering (after Canvas setup)
- Zoom + Pan/Scroll (after PDF Display)
- Creation + Selection (after Region Rendering)

**Phase 3**:
- Group Management + Name Editing + Color Editing (after Selection)
- Undo/Redo (after all Region Operations)

**Phase 4**:
- JSON Export + CSV Export (after Data Models)
- JSON Import (after JSON Export)
- Mask Generation (after Data Models)

**Phase 5**:
- All polish features can be built in parallel (after Phase 4)

---

## Dependency Details

### Coordinate System Dependencies

**Requires**:
- C++ compiler
- Basic math libraries
- No external dependencies

**Provides**:
- Transformation functions
- Coordinate validation
- Used by: Data Models, Canvas, Region Operations

### Data Models Dependencies

**Requires**:
- Coordinate System
- Qt data types (QString, QMap, QList)
- No UI dependencies

**Provides**:
- RegionData structure
- GroupData structure
- DocumentState class
- Used by: Canvas, Export/Import, Undo/Redo

### Canvas Dependencies

**Requires**:
- Qt Widgets
- DocumentState
- CoordinateSystem
- PdfLoader utility

**Provides**:
- PDF display
- Region rendering
- Mouse/keyboard interaction
- Used by: All Region Operations, Navigation

### Region Operations Dependencies

**Requires**:
- Canvas (for rendering and events)
- DocumentState (for data)
- CoordinateSystem (for transformations)

**Provides**:
- Region creation
- Selection
- Movement
- Resize
- Used by: Groups, Editing, Undo/Redo

### Export/Import Dependencies

**Requires**:
- DocumentState (reads data)
- Qt JSON/XML support
- File I/O

**Provides**:
- Data serialization
- File export/import
- Used by: User workflows

---

## Build Order Checklist

### Phase 1: Foundation

- [ ] **Step 1**: CMake Configuration
- [ ] **Step 2**: Dependency Management (Qt, Poppler)
- [ ] **Step 3**: Directory Structure
- [ ] **Step 4**: Coordinate System (can start after Step 3)
- [ ] **Step 5**: Data Models (after Step 4)
- [ ] **Step 6**: Window Setup (after Step 3)
- [ ] **Step 7**: Basic Layout (after Step 6)

**Blocking**: Steps 1-3 must complete before anything else
**Parallel**: Steps 4-5 can run parallel with Steps 6-7 (after Step 3)

### Phase 2: Interaction

- [ ] **Step 8**: PDF Display (after Steps 5, 7)
- [ ] **Step 9**: Region Rendering (after Step 8)
- [ ] **Step 10**: Region Creation (after Step 9)
- [ ] **Step 11**: Region Selection (after Step 9)
- [ ] **Step 12**: Region Movement (after Step 11)
- [ ] **Step 13**: Region Resize (after Step 11)
- [ ] **Step 14**: Zoom (after Step 8)
- [ ] **Step 15**: Pan/Scroll (after Step 14)

**Blocking**: Step 8 must complete before Steps 9-15
**Parallel**: Steps 10-11 can run in parallel, Steps 12-13 can run in parallel, Steps 14-15 can run in parallel

### Phase 3: Organization

- [ ] **Step 16**: Group Management (after Steps 10-13)
- [ ] **Step 17**: Name Editing (after Step 11)
- [ ] **Step 18**: Color Editing (after Step 11)
- [ ] **Step 19**: Undo/Redo (after Steps 10-13)

**Blocking**: Steps 10-13 must complete before Phase 3
**Parallel**: Steps 16-18 can run in parallel, Step 19 after all

### Phase 4: Export/Import

- [ ] **Step 20**: JSON Export (after Step 5)
- [ ] **Step 21**: CSV Export (after Step 5)
- [ ] **Step 22**: JSON Import (after Step 20)
- [ ] **Step 23**: Mask Generation (after Steps 5, 8)

**Blocking**: Step 5 must complete before Phase 4
**Parallel**: Steps 20-21 can run in parallel, Step 22 after Step 20, Step 23 can run parallel with Steps 20-21

### Phase 5: Polish

- [ ] **Step 24**: Keyboard Shortcuts (after all features)
- [ ] **Step 25**: Help System (after all features)
- [ ] **Step 26**: Performance (after all features)
- [ ] **Step 27**: Theming (after Step 7)

**Blocking**: Most features must complete before Steps 24-26
**Parallel**: All steps can run in parallel (Step 27 can start earlier)

---

## Testing Dependencies

### Unit Test Dependencies

```
Coordinate System Tests
    └── Requires: Coordinate System implementation

Data Models Tests
    ├── Requires: Coordinate System (for transformations)
    └── Requires: Data Models implementation

Export Tests
    └── Requires: Data Models, Export implementation
```

### Integration Test Dependencies

```
Canvas + DocumentState Tests
    ├── Requires: Canvas implementation
    ├── Requires: DocumentState implementation
    └── Requires: Coordinate System

Export/Import Round-Trip Tests
    ├── Requires: Export implementation
    ├── Requires: Import implementation
    └── Requires: Data Models
```

---

## Risk Areas (High Dependencies)

### Critical Path Components

1. **Coordinate System** - Used by everything
   - **Risk**: If broken, nothing works
   - **Mitigation**: Extensive unit tests

2. **DocumentState** - Central data store
   - **Risk**: If broken, data loss
   - **Mitigation**: Validation, tests

3. **Canvas** - Core UI component
   - **Risk**: If broken, no user interaction
   - **Mitigation**: Incremental development, testing

### High Dependency Count

Components that many others depend on:
- **CoordinateSystem**: 8+ dependencies
- **DocumentState**: 10+ dependencies
- **Canvas**: 6+ dependencies

**Strategy**: Build and test these thoroughly before building dependents.

---

## Parallel Development Opportunities

### After Phase 1 Complete

**Team A**: Canvas + Region Operations
**Team B**: Export/Import (can start early, only needs Data Models)

### After Phase 2 Complete

**Team A**: Groups + Editing
**Team B**: Undo/Redo
**Team C**: Additional Export Formats

### After Phase 3 Complete

**Team A**: Keyboard Shortcuts
**Team B**: Help System
**Team C**: Performance Optimization
**Team D**: Theming

---

## Milestone Checkpoints

### Milestone 1: Foundation Complete
**Dependencies Met**: Project Setup, Core Modules, UI Framework
**Can Proceed To**: Phase 2 (Interaction)

### Milestone 2: Interaction Complete
**Dependencies Met**: Canvas, Region Operations, Navigation
**Can Proceed To**: Phase 3 (Organization)

### Milestone 3: Organization Complete
**Dependencies Met**: Groups, Editing, Undo/Redo
**Can Proceed To**: Phase 4 (Export/Import)

### Milestone 4: Export/Import Complete
**Dependencies Met**: All Export Formats, Import, Mask
**Can Proceed To**: Phase 5 (Polish)

### Milestone 5: Production Ready
**Dependencies Met**: All Features Complete
**Status**: Ready for release

---

## Rollback Points

If a component fails, what can be rolled back:

1. **Coordinate System**: Can't roll back (foundation)
2. **Data Models**: Can roll back to Coordinate System
3. **Canvas**: Can roll back to Data Models
4. **Region Operations**: Can roll back to Canvas
5. **Export/Import**: Can roll back independently
6. **Polish**: Can roll back independently

**Strategy**: Keep working versions at each milestone.

---

*This temporal dependencies document ensures components are built in the correct order with all dependencies satisfied.*

