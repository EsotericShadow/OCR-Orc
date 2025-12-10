# MainWindow Refactoring & UI Directory Reorganization Plan

## Current State Analysis

### MainWindow.cpp Status
- **Current LOC**: 775 lines
- **Target LOC**: <500 lines
- **Reduction Needed**: ~275 lines
- **Methods Remaining**: 28 methods with heavy lambda callbacks

### Handler Modules Created
1. **MainWindowActionHandlers** (114 lines) - ✅ Refactored (takes only MainWindow*)
2. **MainWindowRegionGroupHandlers** (404 lines) - ⚠️ Partially refactored (1/14 methods done)
3. **MainWindowViewHandlers** (257 lines) - ❌ Not refactored yet

### Current UI Directory Structure
```
src/ui/
├── MainWindow.cpp (775 lines - TOO LARGE)
├── MainWindow.h
├── canvas/
│   ├── Canvas.cpp
│   ├── Canvas.h
│   ├── CanvasMouseMode.h
│   ├── core/ (8 files)
│   ├── input/ (6 files)
│   ├── managers/ (5 files)
│   └── ui/ (2 files)
├── mainwindow/ (15 files - MIXED CONCERNS)
│   ├── MainWindowActionHandlers.*
│   ├── MainWindowRegionGroupHandlers.*
│   ├── MainWindowViewHandlers.*
│   ├── MainWindowFileOperations.*
│   ├── MainWindowRegionOperations.*
│   ├── MainWindowGroupOperations.*
│   ├── MainWindowUndoRedo.*
│   ├── MainWindowUIUpdates.*
│   ├── MainWindowSelectionOperations.*
│   ├── MainWindowKeyboardShortcuts.*
│   ├── MainWindowHelpDialog.*
│   ├── MainWindowUISetup.*
│   ├── MainWindowWidgetConnections.*
│   └── MainWindowEventHandlers.*
└── widgets/ (3 widget files)
```

## Phase 1: Handler Refactoring (Reduce MainWindow.cpp LOC)

### Strategy
Refactor all handler methods to take only `MainWindow*` and create lambdas internally by accessing MainWindow's members directly (friend class access).

### Methods to Refactor

#### MainWindowRegionGroupHandlers (13 remaining)
1. ✅ `onCreateGroup` - DONE
2. `getSelectedRegions` - Simplify signature
3. `onAddToGroup` - Move lambdas inside
4. `onRemoveFromGroup` - Move lambdas inside
5. `onDeleteGroup` - Move lambdas inside
6. `showEditNameDialog` - Already simple, verify
7. `onEditSelected` - Move lambdas inside
8. `showChangeColorDialog` - Already simple, verify
9. `onRegionCreationRequested` - Move lambdas inside
10. `onRegionCreated` - Move lambdas inside
11. `onRegionsDuplicated` - Move lambdas inside
12. `generateRegionName` - Simplify signature
13. `autoIncrementRegionName` - Move lambdas inside
14. `onDeleteSelected` - Move lambdas inside

#### MainWindowViewHandlers (14 methods)
1. `onCreateModeSelected` - Already simple, verify
2. `onSelectModeSelected` - Already simple, verify
3. `onZoomIn` - Simplify signature
4. `onZoomOut` - Simplify signature
5. `onZoomReset` - Simplify signature
6. `updateZoomLabel` - Move lambdas inside
7. `onUndo` - Move lambdas inside
8. `onRedo` - Move lambdas inside
9. `updateUndoRedoButtons` - Move lambdas inside
10. `updateRegionListBox` - Move lambdas inside
11. `updateGroupListBox` - Move lambdas inside
12. `onSelectAll` - Move lambdas inside
13. `onInvertSelection` - Move lambdas inside
14. `showHelp` - Already simple, verify
15. `setupKeyboardShortcuts` - Already simple, verify

### Expected LOC Reduction
- Each method refactored: ~10-40 lines reduction in MainWindow.cpp
- Total expected reduction: ~275-350 lines
- Final MainWindow.cpp: ~400-500 lines ✅

### Execution Order
1. **Batch 1**: Simple methods (getSelectedRegions, generateRegionName, showEditNameDialog, showChangeColorDialog)
2. **Batch 2**: Medium complexity (onAddToGroup, onRemoveFromGroup, onDeleteGroup, onEditSelected)
3. **Batch 3**: Complex methods (onRegionCreationRequested, onDeleteSelected, onUndo, onRedo)
4. **Batch 4**: UI update methods (updateZoomLabel, updateUndoRedoButtons, updateRegionListBox, updateGroupListBox)
5. **Batch 5**: Selection methods (onSelectAll, onInvertSelection)
6. **Batch 6**: Remaining simple methods (verify and finalize)

## Phase 2: UI Directory Reorganization

### Proposed Structure (Based on Best Practices)

```
src/ui/
├── MainWindow.cpp (<500 LOC)
├── MainWindow.h
│
├── components/          # Reusable UI components
│   ├── widgets/         # Custom widgets
│   │   ├── ToolbarWidget.*
│   │   ├── ControlPanelWidget.*
│   │   └── SidePanelWidget.*
│   └── dialogs/         # Dialog components
│       └── HelpDialog.*
│
├── canvas/              # Canvas widget and related
│   ├── Canvas.cpp
│   ├── Canvas.h
│   ├── CanvasMouseMode.h
│   ├── core/            # Core canvas functionality
│   │   ├── coordinate/  # Coordinate-related
│   │   │   ├── CanvasCoordinateCache.*
│   │   │   └── CanvasHitTester.*
│   │   ├── rendering/   # Rendering-related
│   │   │   └── CanvasRenderer.*
│   │   ├── zoom/        # Zoom-related
│   │   │   └── CanvasZoomController.*
│   │   ├── selection/   # Selection-related
│   │   │   └── CanvasSelectionManager.*
│   │   └── regions/     # Region operations
│   │       ├── CanvasRegionOperations.*
│   │       └── CanvasRegionCreator.*
│   ├── input/           # Input handling
│   │   ├── CanvasInputHandler.*
│   │   ├── CanvasMouseHandler.*
│   │   ├── CanvasKeyboardHandler.*
│   │   ├── CanvasWheelHandler.*
│   │   └── CanvasHoverManager.*
│   ├── managers/        # State managers
│   │   ├── CanvasRegionManager.*
│   │   ├── CanvasRegionCreationManager.*
│   │   ├── CanvasZoomManager.*
│   │   └── CanvasStateManager.*
│   └── ui/              # Canvas UI components
│       └── CanvasContextMenuBuilder.*
│
└── mainwindow/          # MainWindow-specific code
    ├── handlers/        # Event/slot handlers (3 categories)
    │   ├── actions/     # File/Export operations
    │   │   └── MainWindowActionHandlers.*
    │   ├── regions/     # Region/Group operations
    │   │   └── MainWindowRegionGroupHandlers.*
    │   └── view/        # UI State/View operations
    │       └── MainWindowViewHandlers.*
    ├── operations/      # Business logic operations
    │   ├── file/        # File operations
    │   │   └── MainWindowFileOperations.*
    │   ├── region/       # Region operations
    │   │   └── MainWindowRegionOperations.*
    │   ├── group/       # Group operations
    │   │   └── MainWindowGroupOperations.*
    │   ├── selection/   # Selection operations
    │   │   └── MainWindowSelectionOperations.*
    │   └── undo/        # Undo/Redo operations
    │       └── MainWindowUndoRedo.*
    ├── setup/           # Setup and initialization
    │   ├── MainWindowUISetup.*
    │   └── MainWindowWidgetConnections.*
    ├── events/          # Event handling
    │   └── MainWindowEventHandlers.*
    ├── ui/              # UI updates
    │   └── MainWindowUIUpdates.*
    └── shortcuts/       # Keyboard shortcuts
        └── MainWindowKeyboardShortcuts.*
```

### Reorganization Principles
1. **Separation of Concerns**: Handlers vs Operations vs Setup
2. **Logical Grouping**: Related functionality together
3. **Clear Hierarchy**: Components → Canvas → MainWindow
4. **Scalability**: Easy to add new features
5. **Maintainability**: Clear where to find things

### File Movement Plan

#### Step 1: Create New Directory Structure
- Create all new subdirectories
- Keep old structure temporarily

#### Step 2: Move Files by Category
1. **Components** (widgets, dialogs)
2. **Canvas** (reorganize core subdirectories)
3. **MainWindow handlers** (3 categories)
4. **MainWindow operations** (5 categories)
5. **MainWindow setup/events/ui/shortcuts**

#### Step 3: Update Includes
- Update all `#include` paths
- Update CMakeLists.txt
- Verify compilation

#### Step 4: Cleanup
- Remove old empty directories
- Update documentation

### Risk Assessment
- **Low Risk**: Moving files (mechanical)
- **Medium Risk**: Include path updates (need careful verification)
- **High Risk**: CMakeLists.txt updates (must be correct)

## Execution Timeline

### Phase 1: Handler Refactoring (~2-3 hours)
- Batch 1-6: Refactor all 28 methods
- Verify MainWindow.cpp <500 LOC
- Build and test

### Phase 2: Directory Reorganization (~1-2 hours)
- Create new structure
- Move files systematically
- Update includes
- Update CMakeLists.txt
- Build and verify

### Total Estimated Time: 3-5 hours

## Success Criteria

### Phase 1 Success
- ✅ MainWindow.cpp <500 LOC
- ✅ All handlers take only MainWindow*
- ✅ Application builds successfully
- ✅ All functionality preserved

### Phase 2 Success
- ✅ Clear, logical directory structure
- ✅ Easy to navigate and find files
- ✅ Application builds successfully
- ✅ All functionality preserved
- ✅ No broken includes

## Notes
- Maintain friend class declarations for handlers
- Preserve all functionality
- Test after each batch
- Keep git history clean (consider staging)

