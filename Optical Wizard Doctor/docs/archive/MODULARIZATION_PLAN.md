# MainWindow Modularization Plan

## Problem
MainWindow.cpp is 2355 lines long, making it difficult to:
- Maintain and debug
- Optimize performance
- Add new features
- Test individual components

## Solution
Break MainWindow into smaller, focused widget classes:

1. **ToolbarWidget** - Mode, Zoom, Groups, Undo/Redo controls (~200 lines)
2. **ControlPanelWidget** - File controls, Region input, Actions (~200 lines)
3. **SidePanelWidget** - Regions tab, Groups tab, Info display, Help (~300 lines)
4. **FileOperationsManager** (future) - File I/O operations (non-UI)
5. **GroupOperationsManager** (future) - Group operations (non-UI)

## Benefits
- **Maintainability**: Each widget is self-contained (~200-300 lines)
- **Performance**: Easier to optimize individual components
- **Testability**: Can test widgets independently
- **Reusability**: Widgets can be reused in other contexts
- **Clarity**: Clear separation of concerns

## Implementation Steps

### Phase 1: Extract ToolbarWidget ✅
- [x] Create ToolbarWidget.h
- [x] Create ToolbarWidget.cpp
- [ ] Update MainWindow to use ToolbarWidget
- [ ] Test toolbar functionality

### Phase 2: Extract ControlPanelWidget ✅
- [x] Create ControlPanelWidget.h
- [x] Create ControlPanelWidget.cpp
- [ ] Update MainWindow to use ControlPanelWidget
- [ ] Test control panel functionality

### Phase 3: Extract SidePanelWidget
- [x] Create SidePanelWidget.h
- [ ] Create SidePanelWidget.cpp
- [ ] Update MainWindow to use SidePanelWidget
- [ ] Test side panel functionality

### Phase 4: Refactor MainWindow
- [ ] Remove extracted code from MainWindow
- [ ] Connect signals/slots between widgets
- [ ] Test all functionality
- [ ] Verify MainWindow.cpp is < 1000 lines

### Phase 5: Performance Optimization
- [ ] Profile each widget independently
- [ ] Optimize rendering in each widget
- [ ] Add viewport culling where needed
- [ ] Cache expensive calculations

## File Structure After Modularization

```
src/ui/
├── MainWindow.h          (~400 lines - orchestration)
├── MainWindow.cpp        (~800 lines - connections & logic)
├── ToolbarWidget.h       (~100 lines)
├── ToolbarWidget.cpp     (~200 lines)
├── ControlPanelWidget.h  (~100 lines)
├── ControlPanelWidget.cpp (~200 lines)
├── SidePanelWidget.h     (~120 lines)
├── SidePanelWidget.cpp   (~300 lines)
├── Canvas.h              (existing)
└── Canvas.cpp            (existing)
```

## Signal/Slot Architecture

```
MainWindow (orchestrator)
├── ToolbarWidget
│   ├── createModeSelected() → MainWindow::onCreateModeSelected()
│   ├── zoomInClicked() → MainWindow::onZoomIn()
│   └── ...
├── ControlPanelWidget
│   ├── loadPdfClicked() → MainWindow::onLoadPdf()
│   ├── exportCoordinatesClicked() → MainWindow::onExportCoordinates()
│   └── ...
└── SidePanelWidget
    ├── regionSelectionChanged() → MainWindow::onRegionListBoxSelectionChanged()
    ├── createGroupClicked() → MainWindow::onCreateGroup()
    └── ...
```

## Notes
- All widgets maintain their own UI setup
- MainWindow coordinates between widgets
- DocumentState remains in MainWindow (shared resource)
- Canvas remains separate (already modular)

