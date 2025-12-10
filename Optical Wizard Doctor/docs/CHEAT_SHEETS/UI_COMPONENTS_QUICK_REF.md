# UI Components Quick Reference

## MainWindow Layout

```
MainWindow
├── Toolbar (top, full width)
│   ├── Mode buttons (Create/Select)
│   ├── Zoom controls (+/-/Reset)
│   ├── Group buttons
│   └── Undo/Redo buttons
│
├── Control Panel (below toolbar)
│   ├── File buttons (Load PDF, Export, etc.)
│   ├── Region input (Name, Group, Color)
│   └── Action buttons
│
└── Main Frame (below control panel)
    ├── Canvas (left, expandable)
    │   ├── Status label
    │   └── Canvas widget (scrollable)
    │
    └── Side Panel (right, fixed 300px)
        ├── Tabs (Regions/Groups)
        ├── Info display
        └── Help button
```

---

## Widget Hierarchy

```
MainWindow (QMainWindow)
└── Central Widget (QWidget)
    └── Main Layout (QVBoxLayout)
        ├── Toolbar Frame (QFrame)
        ├── Control Panel Frame (QFrame)
        └── Main Frame (QFrame)
            └── Horizontal Layout (QHBoxLayout)
                ├── Canvas Container (QWidget)
                │   └── Canvas (QWidget) in QScrollArea
                └── Side Panel Frame (QFrame)
                    └── Vertical Layout (QVBoxLayout)
                        ├── Tab Widget (QTabWidget)
                        ├── Info Text (QTextEdit)
                        └── Help Button (QPushButton)
```

---

## Canvas Visual States

### Region States

| State | Border | Fill | Label | Handles |
|-------|--------|------|-------|---------|
| Normal | 2px, color | None | 8pt, normal | No |
| Hovered | 3px, color | 10% opacity | 10pt, normal | No |
| Selected | 4px, color | 20% opacity | 10pt, bold | Yes (if single) |

### Selection Box
- **Color**: #0066ff (blue)
- **Style**: Dashed, 2px width
- **Pattern**: 5px dash, 5px gap

### Resize Handles
- **Size**: 8x8 pixels
- **Style**: White fill, black outline
- **Count**: 8 per region (4 corners, 4 edges)
- **Tolerance**: 13 pixels for clicking

---

## Color Scheme

### Light Mode
- Toolbar: #e0e0e0
- Canvas: #d0d0d0
- Selection: #0066ff

### Dark Mode
- Toolbar: #2d2d2d
- Canvas: #1e1e1e
- Selection: #58a6ff

---

## Event Handling

### Mouse Events
```
mousePressEvent()   - Start operation
mouseMoveEvent()    - Update operation
mouseReleaseEvent() - Finalize operation
wheelEvent()        - Scroll or zoom
```

### Keyboard Events
```
keyPressEvent()     - Handle shortcuts
```

### Paint Events
```
paintEvent()        - Render PDF and regions
```

---

## Mode Switching

### Create Mode
- **Cursor**: Crosshair
- **Action**: Click and drag to create regions
- **Status**: "Mode: Create - Click and drag to create new regions"

### Select Mode
- **Cursor**: Arrow (or fleur over region)
- **Action**: Select, move, resize regions
- **Status**: "Mode: Select/Move - Click to select..."

---

## List Widgets

### Regions List
- **Widget**: QListWidget
- **Content**: Region names (alphabetically sorted)
- **Selection**: Single selection
- **Action**: Click to select region

### Groups List
- **Widget**: QListWidget
- **Content**: "Group Name (N regions)" format
- **Selection**: Single selection
- **Action**: Click to view group details

---

## Info Display

### Region Info
```
Region: [name]
Image Coords: [x1, y1, x2, y2]
Normalized: [x1, y1, x2, y2]
Color: [color]
Group: [group]
Size: [width] x [height] pixels
Normalized Size: [width] x [height]
```

### Group Info
```
Group: [name]
Regions (N):
  - [region name 1]
  - [region name 2]
  ...
```

---

*Quick reference for UI components and layout*

