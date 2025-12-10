# Quick Reference Cheat Sheet
## OCR-Orc - At a Glance

---

## Class Names

```
CoordinateSystem    - Coordinate transformations
RegionData          - Region data structure
GroupData           - Group data structure
DocumentState       - Application state
MainWindow          - Main application window
Canvas              - PDF display widget
PdfLoader           - PDF to image utility
JsonExporter        - JSON export
CsvExporter         - CSV export
MaskGenerator       - Mask image generation
```

---

## Function Patterns

### Coordinate Transformations
```
normalizedToImage(norm, width, height) → ImageCoords
imageToNormalized(img, width, height) → NormalizedCoords
imageToCanvas(img, scale, offset) → CanvasCoords
canvasToImage(canvas, scale, offset) → ImageCoords
```

### Region Operations
```
createRegion(name, rect) → void
selectRegion(name) → void
moveRegion(name, delta) → void
resizeRegion(name, handle, pos) → void
deleteRegion(name) → void
```

### State Management
```
addRegion(name, data) → void
removeRegion(name) → void
createGroup(name) → void
addRegionToGroup(region, group) → void
```

---

## Variable Patterns

### State Variables
```
mouseMode              - MouseMode enum
selectedRegions        - QSet<QString>
primarySelectedRegion  - QString
hoveredRegion          - QString
isDragging             - bool
isResizing             - bool
zoomLevel              - double
scaleFactor            - double
```

### UI Variables
```
regionListBox    - QListWidget*
groupListBox     - QListWidget*
infoText         - QTextEdit*
statusLabel      - QLabel*
zoomLabel        - QLabel*
```

---

## Constants

```
MIN_ZOOM = 0.2
MAX_ZOOM = 5.0
ZOOM_STEP = 1.2
MIN_REGION_SIZE = 10
MAX_UNDO_LEVELS = 50
HANDLE_SIZE = 8
HANDLE_TOLERANCE = 13
PDF_DPI = 150
```

---

## Keyboard Shortcuts

```
1              - Create mode
2              - Select mode
Cmd+O          - Load PDF
Cmd+S          - Export
Cmd+Z          - Undo
Cmd+Shift+Z    - Redo
Delete         - Delete selected
E              - Edit region
M              - Toggle mask
+/-            - Zoom in/out
0              - Reset zoom
Cmd+A          - Select all
Cmd+D          - Deselect all
Cmd+I          - Invert selection
```

---

## Color Values

```
blue    #0066ff
red     #ff0000
green   #00cc00
yellow  #ffcc00
purple  #9900cc
orange  #ff6600
cyan    #00cccc
```

---

## Coordinate Systems

```
Normalized (0.0-1.0) ← Source of Truth
    ↕ multiply/divide by image dimensions
Image (pixels)
    ↕ multiply/divide by scale, add offset
Canvas (display pixels)
```

---

## File Structure

```
src/
├── core/          - CoordinateSystem, business logic
├── models/        - RegionData, GroupData, DocumentState
├── ui/            - MainWindow, Canvas
├── export/        - Exporters
└── utils/         - PdfLoader
```

---

## Naming Rules

- **Classes**: PascalCase (`MainWindow`)
- **Functions**: camelCase (`createRegion()`)
- **Variables**: camelCase (`selectedRegions`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_ZOOM`)
- **Files**: PascalCase.h/cpp (`MainWindow.h`)

---

*Quick reference for daily development*

