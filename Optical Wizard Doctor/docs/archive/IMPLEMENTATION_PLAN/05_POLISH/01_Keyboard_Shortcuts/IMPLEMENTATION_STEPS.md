# Keyboard Shortcuts - Implementation Steps

## Objective

Implement all keyboard shortcuts from the specification.

## Prerequisites

- All features implemented
- Event handling working
- Platform detection ready

## Shortcut Categories

1. **Mode Switching** - 1, 2
2. **File Operations** - Cmd+O, Cmd+S, etc.
3. **Zoom Operations** - +, -, 0
4. **Region Operations** - Delete, E, M, T
5. **Selection Operations** - Cmd+A, Cmd+D, Cmd+I
6. **Group Operations** - Cmd+G, Cmd+Shift+G
7. **Undo/Redo** - Cmd+Z, Cmd+Shift+Z, Cmd+Y

## Steps

### Step 1: Set Up Platform Detection

1. Detect platform:
   ```cpp
   #ifdef Q_OS_MAC
       #define MODIFIER_KEY Qt::MetaModifier  // Cmd
   #else
       #define MODIFIER_KEY Qt::ControlModifier  // Ctrl
   #endif
   ```
2. Or use Qt's platform detection at runtime

### Step 2: Register Mode Shortcuts

1. In MainWindow or Canvas:
   ```cpp
   // Mode switching
   QShortcut* createMode = new QShortcut(QKeySequence("1"), this);
   connect(createMode, &QShortcut::activated, this, &MainWindow::setModeCreate);
   
   QShortcut* selectMode = new QShortcut(QKeySequence("2"), this);
   connect(selectMode, &QShortcut::activated, this, &MainWindow::setModeSelect);
   ```

### Step 3: Register File Operation Shortcuts

1. Load PDF:
   ```cpp
   QShortcut* loadPdf = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_O), this);
   connect(loadPdf, &QShortcut::activated, this, &MainWindow::loadPdf);
   ```
2. Export Coordinates:
   ```cpp
   QShortcut* export = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_S), this);
   connect(export, &QShortcut::activated, this, &MainWindow::exportCoordinates);
   ```
3. Load Coordinates:
   ```cpp
   QShortcut* loadCoords = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::ShiftModifier | Qt::Key_S), this);
   connect(loadCoords, &QShortcut::activated, this, &MainWindow::loadCoordinates);
   ```
4. Export Mask:
   ```cpp
   QShortcut* exportMask = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_M), this);
   connect(exportMask, &QShortcut::activated, this, &MainWindow::exportMask);
   ```

### Step 4: Register Zoom Shortcuts

1. In Canvas keyPressEvent():
   ```cpp
   void Canvas::keyPressEvent(QKeyEvent* event) {
       if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
           zoomIn();
       } else if (event->key() == Qt::Key_Minus) {
           zoomOut();
       } else if (event->key() == Qt::Key_0) {
           resetZoom();
       }
   }
   ```

### Step 5: Register Region Operation Shortcuts

1. Delete:
   ```cpp
   QShortcut* deleteShortcut = new QShortcut(QKeySequence::Delete, this);
   connect(deleteShortcut, &QShortcut::activated, 
           this, &MainWindow::deleteSelectedRegion);
   ```
2. Edit (E key):
   ```cpp
   QShortcut* editShortcut = new QShortcut(QKeySequence("E"), this);
   connect(editShortcut, &QShortcut::activated, 
           this, &MainWindow::editSelectedRegion);
   ```
3. Toggle Mask (M key):
   ```cpp
   QShortcut* maskShortcut = new QShortcut(QKeySequence("M"), this);
   connect(maskShortcut, &QShortcut::activated, 
           this, &MainWindow::toggleMask);
   ```

### Step 6: Register Selection Shortcuts

1. Select All:
   ```cpp
   QShortcut* selectAll = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_A), this);
   connect(selectAll, &QShortcut::activated, 
           canvas, &Canvas::selectAll);
   ```
2. Deselect All:
   ```cpp
   QShortcut* deselectAll = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_D), this);
   connect(deselectAll, &QShortcut::activated, 
           canvas, &Canvas::deselectAll);
   ```
3. Invert Selection:
   ```cpp
   QShortcut* invertSelection = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_I), this);
   connect(invertSelection, &QShortcut::activated, 
           canvas, &Canvas::invertSelection);
   ```

### Step 7: Register Group Shortcuts

1. Group Selected:
   ```cpp
   QShortcut* group = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_G), this);
   connect(group, &QShortcut::activated, 
           this, &MainWindow::createGroupFromSelection);
   ```
2. Ungroup Selected:
   ```cpp
   QShortcut* ungroup = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::ShiftModifier | Qt::Key_G), this);
   connect(ungroup, &QShortcut::activated, 
           this, &MainWindow::ungroupSelection);
   ```

### Step 8: Register Undo/Redo Shortcuts

1. Undo:
   ```cpp
   QShortcut* undo = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_Z), this);
   connect(undo, &QShortcut::activated, 
           documentState, &DocumentState::undoAction);
   ```
2. Redo (Cmd+Shift+Z):
   ```cpp
   QShortcut* redo = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::ShiftModifier | Qt::Key_Z), this);
   connect(redo, &QShortcut::activated, 
           documentState, &DocumentState::redoAction);
   ```
3. Redo (Cmd+Y, alternative):
   ```cpp
   QShortcut* redoAlt = new QShortcut(
       QKeySequence(Qt::MODIFIER_KEY | Qt::Key_Y), this);
   connect(redoAlt, &QShortcut::activated, 
           documentState, &DocumentState::redoAction);
   ```

### Step 9: Handle Context-Sensitive Shortcuts

1. Some shortcuts require selection:
   - Delete, Edit, Group operations
   - Check if selection exists before executing
   - Show warning if no selection
2. Some shortcuts always work:
   - Mode switching, zoom, file operations

### Step 10: Display Shortcuts in Menus

1. Add shortcuts to menu items:
   ```cpp
   QAction* loadAction = fileMenu->addAction("Load PDF");
   loadAction->setShortcut(QKeySequence(Qt::MODIFIER_KEY | Qt::Key_O));
   ```
2. Shortcuts display automatically in menus
3. Helps users discover shortcuts

### Step 11: Test All Shortcuts

1. Test each shortcut individually
2. Test on different platforms (macOS, Windows, Linux)
3. Test context sensitivity
4. Test modifier key handling
5. Verify no conflicts between shortcuts

## Acceptance Criteria

- [ ] All shortcuts from spec are implemented
- [ ] Shortcuts work on all platforms
- [ ] Context-sensitive shortcuts check selection
- [ ] Shortcuts display in menus
- [ ] No shortcut conflicts
- [ ] Platform-specific modifiers work correctly

## Code Structure

```cpp
class MainWindow {
    void setupKeyboardShortcuts();
    
private:
    QList<QShortcut*> shortcuts;  // Store for cleanup
};
```

## Notes

- Use QShortcut for most shortcuts
- Handle keyPressEvent for special cases
- Test on all target platforms
- Document platform differences

