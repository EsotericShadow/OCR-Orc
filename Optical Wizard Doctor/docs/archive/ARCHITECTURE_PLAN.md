# UI Directory Architecture Refactoring Plan

## Executive Summary

This document outlines a comprehensive architectural refactoring plan for the `src/ui` directory. The goal is to create a clear, scalable, maintainable structure that follows Qt/C++ best practices and makes the codebase easier to navigate, understand, and extend.

## Current State Analysis

### Current Structure
```
src/ui/
├── MainWindow.cpp (568 lines - ✅ Under 500 LOC target)
├── MainWindow.h
├── canvas/
│   ├── Canvas.cpp, Canvas.h
│   ├── CanvasMouseMode.h
│   ├── core/ (8 files - coordinate, rendering, zoom, selection, regions)
│   ├── input/ (6 files - mouse, keyboard, wheel, hover, input handler)
│   ├── managers/ (5 files - region, zoom, state, creation managers)
│   └── ui/ (2 files - context menu builder)
├── mainwindow/ (15 files - mixed concerns)
│   ├── handlers/ (3 category handlers)
│   ├── operations/ (5 operation modules)
│   ├── setup/ (2 setup modules)
│   ├── events/ (1 event handler)
│   ├── ui/ (1 UI updates module)
│   └── shortcuts/ (1 keyboard shortcuts module)
└── widgets/ (3 custom widget files)
```

### Issues with Current Structure
1. **Mixed Concerns**: `mainwindow/` contains handlers, operations, setup, events, UI updates, and shortcuts all at the same level
2. **Unclear Hierarchy**: Hard to understand what depends on what
3. **Scalability**: Adding new features requires understanding the entire structure
4. **Naming Inconsistency**: Some modules use "Operations", others use "Handlers"
5. **Canvas Structure**: Good, but could be better organized

## Proposed Architecture

### Design Principles
1. **Separation of Concerns**: Clear boundaries between UI, logic, and data
2. **Layered Architecture**: Presentation → Handlers → Operations → Models
3. **Single Responsibility**: Each module has one clear purpose
4. **Dependency Direction**: Higher layers depend on lower layers, never reverse
5. **Discoverability**: Easy to find where code lives

### New Structure

```
src/ui/
│
├── MainWindow.cpp          # Main entry point (<500 LOC)
├── MainWindow.h            # Main window declaration
│
├── components/             # Reusable UI components
│   ├── widgets/            # Custom widgets
│   │   ├── ToolbarWidget.*
│   │   ├── ControlPanelWidget.*
│   │   └── SidePanelWidget.*
│   └── dialogs/            # Dialog components
│       └── HelpDialog.*
│
├── canvas/                 # Canvas widget and related functionality
│   ├── Canvas.cpp          # Main canvas widget
│   ├── Canvas.h
│   ├── CanvasMouseMode.h   # Enums
│   │
│   ├── core/               # Core canvas functionality
│   │   ├── coordinate/     # Coordinate-related
│   │   │   ├── CanvasCoordinateCache.*
│   │   │   └── CanvasHitTester.*
│   │   ├── rendering/      # Rendering-related
│   │   │   └── CanvasRenderer.*
│   │   ├── zoom/           # Zoom-related
│   │   │   └── CanvasZoomController.*
│   │   ├── selection/      # Selection-related
│   │   │   └── CanvasSelectionManager.*
│   │   └── regions/        # Region operations
│   │       ├── CanvasRegionOperations.*
│   │       └── CanvasRegionCreator.*
│   │
│   ├── input/              # Input handling
│   │   ├── CanvasInputHandler.*      # Main input coordinator
│   │   ├── CanvasMouseHandler.*      # Mouse events
│   │   ├── CanvasKeyboardHandler.*   # Keyboard events
│   │   ├── CanvasWheelHandler.*      # Wheel events
│   │   └── CanvasHoverManager.*      # Hover state
│   │
│   ├── managers/           # State managers
│   │   ├── CanvasRegionManager.*         # Region manipulation
│   │   ├── CanvasRegionCreationManager.* # Creation workflow
│   │   ├── CanvasZoomManager.*           # Zoom state
│   │   └── CanvasStateManager.*          # General state
│   │
│   └── ui/                 # Canvas UI components
│       └── CanvasContextMenuBuilder.*
│
└── mainwindow/             # MainWindow-specific code
    │
    ├── handlers/           # Event/slot handlers (3 categories)
    │   ├── actions/       # File/Export operations
    │   │   └── MainWindowActionHandlers.*
    │   ├── regions/       # Region/Group operations
    │   │   └── MainWindowRegionGroupHandlers.*
    │   └── view/          # UI State/View operations
    │       └── MainWindowViewHandlers.*
    │
    ├── operations/        # Business logic operations
    │   ├── file/          # File operations
    │   │   └── MainWindowFileOperations.*
    │   ├── region/        # Region operations
    │   │   └── MainWindowRegionOperations.*
    │   ├── group/         # Group operations
    │   │   └── MainWindowGroupOperations.*
    │   ├── selection/     # Selection operations
    │   │   └── MainWindowSelectionOperations.*
    │   └── undo/          # Undo/Redo operations
    │       └── MainWindowUndoRedo.*
    │
    ├── setup/             # Setup and initialization
    │   ├── MainWindowUISetup.*
    │   └── MainWindowWidgetConnections.*
    │
    ├── events/            # Event handling
    │   └── MainWindowEventHandlers.*
    │
    ├── ui/                # UI updates
    │   └── MainWindowUIUpdates.*
    │
    └── shortcuts/         # Keyboard shortcuts
        └── MainWindowKeyboardShortcuts.*
```

## Detailed Breakdown

### 1. `/components` - Reusable UI Components

**Purpose**: Widgets and dialogs that could potentially be reused elsewhere

**Structure**:
- `widgets/` - Custom Qt widgets (ToolbarWidget, ControlPanelWidget, SidePanelWidget)
- `dialogs/` - Dialog components (HelpDialog)

**Rationale**: Separates reusable components from MainWindow-specific code

### 2. `/canvas` - Canvas Widget

**Purpose**: All canvas-related functionality

**Structure**:
- **Root**: Main canvas widget files
- **`core/`**: Core functionality organized by concern
  - `coordinate/` - Coordinate transformations and caching
  - `rendering/` - Rendering logic
  - `zoom/` - Zoom functionality
  - `selection/` - Selection management
  - `regions/` - Region operations and creation
- **`input/`**: Input event handling (mouse, keyboard, wheel, hover)
- **`managers/`**: State management (region, zoom, creation, general state)
- **`ui/`**: Canvas-specific UI components (context menus)

**Rationale**: Clear separation of concerns within canvas, easy to find related functionality

### 3. `/mainwindow` - MainWindow-Specific Code

**Purpose**: All code specific to MainWindow

**Structure**:
- **`handlers/`**: Thin wrappers that delegate to operations
  - `actions/` - File/export handlers
  - `regions/` - Region/group handlers
  - `view/` - UI state/view handlers
- **`operations/`**: Business logic organized by domain
  - `file/` - File I/O operations
  - `region/` - Region manipulation logic
  - `group/` - Group management logic
  - `selection/` - Selection logic
  - `undo/` - Undo/redo logic
- **`setup/`**: Initialization code
- **`events/`**: Window event handling (close, drag-drop)
- **`ui/`**: UI update logic
- **`shortcuts/`**: Keyboard shortcut setup

**Rationale**: Clear separation between handlers (thin wrappers) and operations (business logic)

## File Movement Map

### Phase 1: Create New Structure
```
Create directories:
- components/widgets/
- components/dialogs/
- canvas/core/coordinate/
- canvas/core/rendering/
- canvas/core/zoom/
- canvas/core/selection/
- canvas/core/regions/
- mainwindow/handlers/actions/
- mainwindow/handlers/regions/
- mainwindow/handlers/view/
- mainwindow/operations/file/
- mainwindow/operations/region/
- mainwindow/operations/group/
- mainwindow/operations/selection/
- mainwindow/operations/undo/
- mainwindow/setup/
- mainwindow/events/
- mainwindow/ui/
- mainwindow/shortcuts/
```

### Phase 2: Move Components
```
widgets/* → components/widgets/
MainWindowHelpDialog.* → components/dialogs/HelpDialog.*
```

### Phase 3: Reorganize Canvas
```
canvas/core/CanvasCoordinateCache.* → canvas/core/coordinate/
canvas/core/CanvasHitTester.* → canvas/core/coordinate/
canvas/core/CanvasRenderer.* → canvas/core/rendering/
canvas/core/CanvasZoomController.* → canvas/core/zoom/
canvas/core/CanvasSelectionManager.* → canvas/core/selection/
canvas/core/CanvasRegionOperations.* → canvas/core/regions/
canvas/core/CanvasRegionCreator.* → canvas/core/regions/
```

### Phase 4: Reorganize MainWindow Handlers
```
mainwindow/MainWindowActionHandlers.* → mainwindow/handlers/actions/
mainwindow/MainWindowRegionGroupHandlers.* → mainwindow/handlers/regions/
mainwindow/MainWindowViewHandlers.* → mainwindow/handlers/view/
```

### Phase 5: Reorganize MainWindow Operations
```
mainwindow/MainWindowFileOperations.* → mainwindow/operations/file/
mainwindow/MainWindowRegionOperations.* → mainwindow/operations/region/
mainwindow/MainWindowGroupOperations.* → mainwindow/operations/group/
mainwindow/MainWindowSelectionOperations.* → mainwindow/operations/selection/
mainwindow/MainWindowUndoRedo.* → mainwindow/operations/undo/
```

### Phase 6: Reorganize MainWindow Setup/Events/UI/Shortcuts
```
mainwindow/MainWindowUISetup.* → mainwindow/setup/
mainwindow/MainWindowWidgetConnections.* → mainwindow/setup/
mainwindow/MainWindowEventHandlers.* → mainwindow/events/
mainwindow/MainWindowUIUpdates.* → mainwindow/ui/
mainwindow/MainWindowKeyboardShortcuts.* → mainwindow/shortcuts/
```

## Include Path Updates

### Pattern Changes
- Old: `#include "mainwindow/MainWindowActionHandlers.h"`
- New: `#include "mainwindow/handlers/actions/MainWindowActionHandlers.h"`

- Old: `#include "canvas/core/CanvasCoordinateCache.h"`
- New: `#include "canvas/core/coordinate/CanvasCoordinateCache.h"`

### Files Requiring Updates
1. **MainWindow.h/cpp**: All handler includes
2. **All handler files**: Operation includes
3. **All operation files**: Other operation includes (if any)
4. **CMakeLists.txt**: All source file paths

## CMakeLists.txt Updates

### Current Pattern
```cmake
src/ui/mainwindow/MainWindowActionHandlers.cpp
```

### New Pattern
```cmake
src/ui/mainwindow/handlers/actions/MainWindowActionHandlers.cpp
src/ui/components/widgets/ToolbarWidget.cpp
src/ui/canvas/core/coordinate/CanvasCoordinateCache.cpp
```

## Execution Plan

### Step 1: Preparation (30 min)
1. Create all new directories
2. Backup current structure (git commit)
3. Document current include dependencies

### Step 2: Move Components (15 min)
1. Move widgets to `components/widgets/`
2. Move HelpDialog to `components/dialogs/`
3. Update includes in moved files
4. Build and verify

### Step 3: Reorganize Canvas Core (30 min)
1. Create subdirectories in `canvas/core/`
2. Move files to appropriate subdirectories
3. Update includes
4. Build and verify

### Step 4: Reorganize MainWindow Handlers (20 min)
1. Create handler subdirectories
2. Move handler files
3. Update includes in MainWindow.h/cpp
4. Build and verify

### Step 5: Reorganize MainWindow Operations (20 min)
1. Create operation subdirectories
2. Move operation files
3. Update includes in handlers
4. Build and verify

### Step 6: Reorganize Remaining MainWindow (15 min)
1. Move setup files
2. Move event files
3. Move UI files
4. Move shortcuts files
5. Update includes
6. Build and verify

### Step 7: Update CMakeLists.txt (20 min)
1. Update all source file paths
2. Update all header file paths
3. Reconfigure CMake
4. Build and verify

### Step 8: Cleanup (10 min)
1. Remove empty directories
2. Verify no broken includes
3. Final build and test
4. Update documentation

**Total Estimated Time: ~2.5 hours**

## Risk Assessment

### Low Risk
- Moving files (mechanical operation)
- Creating directories

### Medium Risk
- Include path updates (need careful verification)
- CMakeLists.txt updates (must be correct)

### High Risk
- Breaking build (mitigated by incremental moves and builds)
- Missing include updates (mitigated by systematic approach)

## Mitigation Strategies

1. **Incremental Moves**: Move one category at a time, build after each
2. **Git Commits**: Commit after each successful phase
3. **Systematic Includes**: Use grep to find all includes, update systematically
4. **Build Verification**: Build after each phase before proceeding
5. **Test Verification**: Run application after each phase

## Success Criteria

1. ✅ All files moved to new locations
2. ✅ All includes updated correctly
3. ✅ CMakeLists.txt updated
4. ✅ Application builds successfully
5. ✅ Application runs correctly
6. ✅ No broken includes
7. ✅ Clear, logical structure
8. ✅ Easy to navigate and find files

## Benefits

1. **Clarity**: Clear separation of concerns
2. **Scalability**: Easy to add new features
3. **Maintainability**: Easy to find and modify code
4. **Discoverability**: New developers can navigate easily
5. **Consistency**: Consistent naming and organization
6. **Professional**: Follows industry best practices

## Notes

- Maintain friend class declarations for handlers
- Preserve all functionality
- Keep MainWindow.cpp under 500 LOC
- All files should remain under 500 LOC
- Consider future additions (e.g., plugins, themes)

