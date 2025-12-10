# Keyboard Shortcuts Quick Reference

## Mode Switching

| Key | Action | Function |
|-----|--------|----------|
| `1` | Create Mode | `setModeCreate()` |
| `2` | Select Mode | `setModeSelect()` |

---

## File Operations

| Shortcut | Action | Function | Platform |
|----------|--------|----------|----------|
| `Cmd+O` | Load PDF | `loadPdf()` | macOS |
| `Ctrl+O` | Load PDF | `loadPdf()` | Windows/Linux |
| `Cmd+S` | Export Coordinates | `exportCoordinates()` | macOS |
| `Ctrl+S` | Export Coordinates | `exportCoordinates()` | Windows/Linux |
| `Cmd+Shift+S` | Load Coordinates | `loadCoordinates()` | macOS |
| `Ctrl+Shift+S` | Load Coordinates | `loadCoordinates()` | Windows/Linux |
| `Cmd+M` | Export Mask | `exportMask()` | macOS |
| `Ctrl+M` | Export Mask | `exportMask()` | Windows/Linux |

---

## Zoom Operations

| Key | Action | Function |
|-----|--------|----------|
| `+` or `=` | Zoom In | `zoomIn()` |
| `-` | Zoom Out | `zoomOut()` |
| `0` | Reset Zoom | `resetZoom()` |
| `Cmd+Scroll` | Zoom In/Out | `zoomIn()` / `zoomOut()` |

---

## Region Operations

| Key | Action | Function |
|-----|--------|----------|
| `Delete` | Delete Selected | `deleteSelectedRegion()` |
| `Backspace` | Delete Selected | `deleteSelectedRegion()` |
| `E` | Edit Region Name | `editSelectedRegion()` |
| `M` | Toggle Mask | `toggleMask()` |
| `T` | Test Extraction | `testExtraction()` (optional) |

---

## Selection Operations

| Shortcut | Action | Function | Platform |
|----------|--------|----------|----------|
| `Cmd+A` | Select All | `selectAll()` | macOS |
| `Ctrl+A` | Select All | `selectAll()` | Windows/Linux |
| `Cmd+D` | Deselect All | `deselectAll()` | macOS |
| `Ctrl+D` | Deselect All | `deselectAll()` | Windows/Linux |
| `Cmd+I` | Invert Selection | `invertSelection()` | macOS |
| `Ctrl+I` | Invert Selection | `invertSelection()` | Windows/Linux |
| `Cmd+Click` | Toggle Multi-Select | `onCmdClick()` | macOS |
| `Ctrl+Click` | Toggle Multi-Select | `onCmdClick()` | Windows/Linux |

---

## Group Operations

| Shortcut | Action | Function | Platform |
|----------|--------|----------|----------|
| `Cmd+G` | Group Selected | `createGroup()` | macOS |
| `Ctrl+G` | Group Selected | `createGroup()` | Windows/Linux |
| `Cmd+Shift+G` | Ungroup Selected | `removeFromGroup()` | macOS |
| `Ctrl+Shift+G` | Ungroup Selected | `removeFromGroup()` | Windows/Linux |

---

## Undo/Redo

| Shortcut | Action | Function | Platform |
|----------|--------|----------|----------|
| `Cmd+Z` | Undo | `undoAction()` | macOS |
| `Ctrl+Z` | Undo | `undoAction()` | Windows/Linux |
| `Cmd+Shift+Z` | Redo | `redoAction()` | macOS |
| `Ctrl+Shift+Z` | Redo | `redoAction()` | Windows/Linux |
| `Cmd+Y` | Redo (Alt) | `redoAction()` | macOS |
| `Ctrl+Y` | Redo (Alt) | `redoAction()` | Windows/Linux |

---

## Mouse Interactions

### Mouse Wheel

| Action | Behavior | Function |
|--------|----------|----------|
| Scroll | Vertical Scroll | `onMouseWheel()` |
| `Shift+Scroll` | Horizontal Scroll | `onMouseWheelShift()` |
| `Cmd+Scroll` | Zoom In/Out | `onMouseWheelCmd()` |

### Mouse Click Modifiers

| Action | Behavior | Function |
|--------|----------|----------|
| Click | Select/Move | `onClick()` |
| `Cmd+Click` | Toggle Multi-Select | `onCmdClick()` |
| `Shift+Drag` | Box Select | `onShiftDrag()` |
| Double-Click | Edit Name | `onDoubleClick()` |
| Right-Click | Context Menu | `onRightClick()` |

---

## Platform Differences

### macOS
- Use `Cmd` key
- Menu bar at top of screen
- Native file dialogs

### Windows/Linux
- Use `Ctrl` key
- Menu bar in window
- Native file dialogs

### Implementation
```cpp
#ifdef Q_OS_MAC
    #define MODIFIER_KEY Qt::MetaModifier  // Cmd
#else
    #define MODIFIER_KEY Qt::ControlModifier  // Ctrl
#endif
```

---

## Shortcut Registration

### In MainWindow
```cpp
// File operations
QShortcut* loadShortcut = new QShortcut(
    QKeySequence("Ctrl+O"), this);
connect(loadShortcut, &QShortcut::activated, 
        this, &MainWindow::loadPdf);
```

### In Canvas
```cpp
// Override keyPressEvent
void Canvas::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete) {
        deleteSelectedRegion();
    }
    // ...
}
```

---

## Context-Sensitive Shortcuts

### Require Selection
- `Delete` / `Backspace` - Delete selected
- `E` - Edit selected
- `Cmd+G` - Group selected
- `Cmd+Shift+G` - Ungroup selected

### Always Available
- `1`, `2` - Mode switching
- `+`, `-`, `0` - Zoom
- `Cmd+O`, `Cmd+S` - File operations
- `Cmd+Z`, `Cmd+Shift+Z` - Undo/Redo

---

*Quick reference for all keyboard shortcuts*

