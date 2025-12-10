# MainWindow Modularization Plan

## Current State
- **MainWindow.cpp**: 1979 lines (target: <500 lines)
- **MainWindow.h**: 326 lines
- **Total**: 2305 lines

## Goal
Break down MainWindow into focused modules, each under 500 LOC, while maintaining all functionality.

---

## Module Breakdown Strategy

### 1. **File Operations Module** (HIGH RISK - Complex dependencies)
**Target**: `MainWindowFileOperations.h/cpp`
- `onLoadPdf()` - ~55 lines
- `onExportCoordinates()` - ~65 lines
- `onLoadCoordinates()` - ~55 lines
- `onExportMask()` - ~65 lines
- **Estimated**: ~240 lines
- **Dependencies**: DocumentState, Canvas, Export/Import classes, QFileDialog, QMessageBox
- **Risk**: Medium - File I/O, error handling, state updates

### 2. **Drag & Drop Handler** (LOW RISK - Isolated)
**Target**: `MainWindowDragDropHandler.h/cpp`
- `dragEnterEvent()` - ~20 lines
- `dragMoveEvent()` - ~20 lines
- `dropEvent()` - ~80 lines (includes PDF loading logic)
- **Estimated**: ~120 lines
- **Dependencies**: QDragEnterEvent, QDropEvent, Canvas
- **Risk**: Low - Self-contained event handling

### 3. **Region Operations Manager** (HIGH RISK - Complex logic)
**Target**: `MainWindowRegionOperations.h/cpp`
- `onRegionCreationRequested()` - ~40 lines
- `onRegionCreated()` - ~10 lines
- `onRegionsDuplicated()` - ~15 lines
- `generateRegionName()` - ~15 lines
- `autoIncrementRegionName()` - ~35 lines
- `onEditSelected()` - ~90 lines
- `showEditNameDialog()` - ~40 lines
- `showChangeColorDialog()` - ~55 lines
- `onDeleteSelected()` - ~45 lines
- **Estimated**: ~345 lines
- **Dependencies**: DocumentState, Canvas, ControlPanelWidget, QDialog, QMessageBox
- **Risk**: High - Complex validation, dialog management, state synchronization

### 4. **Group Operations Manager** (MEDIUM RISK - State management)
**Target**: `MainWindowGroupOperations.h/cpp`
- `onCreateGroup()` - ~90 lines
- `onAddToGroup()` - ~55 lines
- `onRemoveFromGroup()` - ~80 lines
- `onDeleteGroup()` - ~55 lines
- `getSelectedRegions()` - ~20 lines
- **Estimated**: ~300 lines
- **Dependencies**: DocumentState, Canvas, SidePanelWidget, QInputDialog, QMessageBox
- **Risk**: Medium - Group state management, validation

### 5. **Selection Operations** (LOW RISK - Simple logic)
**Target**: `MainWindowSelectionOperations.h/cpp`
- `onSelectAll()` - ~20 lines
- `onInvertSelection()` - ~20 lines
- **Estimated**: ~40 lines
- **Dependencies**: DocumentState, Canvas
- **Risk**: Low - Simple set operations

### 6. **Zoom Operations** (LOW RISK - Simple delegation)
**Target**: `MainWindowZoomOperations.h/cpp`
- `onZoomIn()` - ~5 lines
- `onZoomOut()` - ~5 lines
- `onZoomReset()` - ~5 lines
- `updateZoomLabel()` - ~5 lines
- **Estimated**: ~20 lines
- **Dependencies**: Canvas, ToolbarWidget
- **Risk**: Low - Simple delegation

### 7. **Mode Operations** (LOW RISK - Trivial)
**Target**: `MainWindowModeOperations.h/cpp`
- `onCreateModeSelected()` - ~5 lines
- `onSelectModeSelected()` - ~5 lines
- **Estimated**: ~10 lines
- **Dependencies**: Canvas
- **Risk**: Low - Single method calls

### 8. **Undo/Redo Manager** (MEDIUM RISK - State synchronization)
**Target**: `MainWindowUndoRedoManager.h/cpp`
- `onUndo()` - ~30 lines
- `onRedo()` - ~30 lines
- `updateUndoRedoButtons()` - ~15 lines
- **Estimated**: ~75 lines
- **Dependencies**: DocumentState, ToolbarWidget, Canvas
- **Risk**: Medium - State synchronization, UI updates

### 9. **UI Update Manager** (LOW RISK - Simple updates)
**Target**: `MainWindowUIUpdateManager.h/cpp`
- `updateRegionListBox()` - ~30 lines
- `updateGroupListBox()` - ~15 lines
- **Estimated**: ~45 lines
- **Dependencies**: DocumentState, Canvas, SidePanelWidget
- **Risk**: Low - Simple data formatting

### 10. **Keyboard Shortcuts Manager** (MEDIUM RISK - Many bindings)
**Target**: `MainWindowKeyboardShortcuts.h/cpp`
- `setupKeyboardShortcuts()` - ~135 lines
- **Estimated**: ~135 lines
- **Dependencies**: QAction, QKeySequence, all slot methods
- **Risk**: Medium - Many connections, platform-specific logic

### 11. **Help System** (LOW RISK - Static content)
**Target**: `MainWindowHelpSystem.h/cpp`
- `showHelp()` - ~35 lines
- **Estimated**: ~35 lines
- **Dependencies**: QDialog, QTextEdit
- **Risk**: Low - Static HTML content

### 12. **UI Setup Manager** (MEDIUM RISK - Initialization order)
**Target**: `MainWindowUISetup.h/cpp`
- `setWindowProperties()` - ~15 lines
- `setupUI()` - ~30 lines (orchestration)
- `setupMenuBar()` - ~10 lines
- `setupStatusBar()` - ~5 lines
- `setupLayout()` - ~25 lines
- `setupCanvas()` - ~150 lines (includes signal connections)
- `connectWidgets()` - ~60 lines
- **Estimated**: ~295 lines
- **Dependencies**: All widgets, Canvas, signal/slot connections
- **Risk**: High - Initialization order, signal/slot connections

---

## Proposed Directory Structure

```
src/ui/
├── MainWindow.h/cpp                    # Main orchestrator (~200 lines)
├── mainwindow/                         # MainWindow modules
│   ├── MainWindowFileOperations.h/cpp
│   ├── MainWindowDragDropHandler.h/cpp
│   ├── MainWindowRegionOperations.h/cpp
│   ├── MainWindowGroupOperations.h/cpp
│   ├── MainWindowSelectionOperations.h/cpp
│   ├── MainWindowZoomOperations.h/cpp
│   ├── MainWindowModeOperations.h/cpp
│   ├── MainWindowUndoRedoManager.h/cpp
│   ├── MainWindowUIUpdateManager.h/cpp
│   ├── MainWindowKeyboardShortcuts.h/cpp
│   ├── MainWindowHelpSystem.h/cpp
│   └── MainWindowUISetup.h/cpp
```

---

## Risk Assessment & Priority

### HIGH RISK (Do First - Most Complex)
1. **MainWindowRegionOperations** - Complex validation, dialogs, state sync
2. **MainWindowUISetup** - Initialization order, signal/slot connections
3. **MainWindowFileOperations** - File I/O, error handling

### MEDIUM RISK (Do Second)
4. **MainWindowGroupOperations** - State management, validation
5. **MainWindowUndoRedoManager** - State synchronization
6. **MainWindowKeyboardShortcuts** - Many connections

### LOW RISK (Do Last - Simple)
7. **MainWindowDragDropHandler** - Isolated event handling
8. **MainWindowSelectionOperations** - Simple set operations
9. **MainWindowZoomOperations** - Simple delegation
10. **MainWindowModeOperations** - Trivial
11. **MainWindowUIUpdateManager** - Simple formatting
12. **MainWindowHelpSystem** - Static content

---

## Implementation Strategy

### Phase 1: Low-Risk Modules (Build Confidence)
1. MainWindowModeOperations
2. MainWindowZoomOperations
3. MainWindowSelectionOperations
4. MainWindowUIUpdateManager
5. MainWindowHelpSystem

### Phase 2: Medium-Risk Modules
6. MainWindowDragDropHandler
7. MainWindowUndoRedoManager
8. MainWindowKeyboardShortcuts
9. MainWindowGroupOperations

### Phase 3: High-Risk Modules (Most Complex)
10. MainWindowFileOperations
11. MainWindowRegionOperations
12. MainWindowUISetup

---

## Key Design Patterns

### 1. Callback Pattern (Like Canvas modules)
- Pass callbacks for UI updates, state changes
- Use `std::function` for flexibility
- Example: `UpdateUICallback`, `ShowMessageCallback`

### 2. Dependency Injection
- Pass required widgets/state as constructor parameters
- Avoid direct member access where possible

### 3. Signal/Slot Preservation
- Keep all existing signals/slots
- Modules can emit signals through callbacks

---

## Estimated Final Line Counts

| Module | Lines | Status |
|--------|-------|--------|
| MainWindow.cpp | ~200 | Orchestrator only |
| MainWindow.h | ~150 | Declarations only |
| MainWindowFileOperations | ~240 | ✅ Under 500 |
| MainWindowDragDropHandler | ~120 | ✅ Under 500 |
| MainWindowRegionOperations | ~345 | ✅ Under 500 |
| MainWindowGroupOperations | ~300 | ✅ Under 500 |
| MainWindowSelectionOperations | ~40 | ✅ Under 500 |
| MainWindowZoomOperations | ~20 | ✅ Under 500 |
| MainWindowModeOperations | ~10 | ✅ Under 500 |
| MainWindowUndoRedoManager | ~75 | ✅ Under 500 |
| MainWindowUIUpdateManager | ~45 | ✅ Under 500 |
| MainWindowKeyboardShortcuts | ~135 | ✅ Under 500 |
| MainWindowHelpSystem | ~35 | ✅ Under 500 |
| MainWindowUISetup | ~295 | ✅ Under 500 |
| **Total** | **~1875** | **All under 500** |

---

## Critical Dependencies to Manage

1. **DocumentState*** - Used by almost all modules
2. **Canvas*** - Used by many modules
3. **Widget pointers** - ToolbarWidget, ControlPanelWidget, SidePanelWidget
4. **Status bar updates** - Shared across modules
5. **UI update callbacks** - updateRegionListBox, updateGroupListBox

---

## Testing Strategy

1. **Unit tests** for each module (where possible)
2. **Integration tests** for signal/slot connections
3. **Manual testing** for UI workflows
4. **Regression testing** for all existing functionality

---

## Notes

- All modules will use callbacks to communicate with MainWindow
- MainWindow will remain the central orchestrator
- Signal/slot connections will be preserved
- No functionality will be removed or changed
- Follow the same patterns used in Canvas modularization

