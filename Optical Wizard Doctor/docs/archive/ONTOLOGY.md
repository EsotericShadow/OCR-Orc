# Optical Wizard Doctor - Complete Ontology
## Terminology, Concepts, and Naming Dictionary

**Purpose**: Define every term, concept, class, function, and variable used in the project to prevent naming conflicts and ensure consistency.

---

## Core Concepts

### Application Domain

| Term | Definition | Usage Context | Alternative Names (DO NOT USE) |
|------|-----------|--------------|-------------------------------|
| **Region** | A rectangular area on a PDF document defined by coordinates | Core concept | Cell, Box, Area, Zone, Field |
| **Group** | A collection of related regions | Organization | Category, Set, Collection |
| **Coordinate System** | System for representing region positions | Core concept | Coords, Position System |
| **Normalized Coordinates** | Coordinates as percentages (0.0-1.0) | Coordinate system | Percent Coords, Relative Coords |
| **Image Coordinates** | Pixel positions in original PDF image | Coordinate system | Pixel Coords, Absolute Coords |
| **Canvas Coordinates** | Display positions on screen | Coordinate system | Screen Coords, Display Coords |
| **Mask** | Black/white image showing only labeled regions | Export | Overlay, Filter Image |
| **Calibration** | Process of defining regions on a document | Workflow | Setup, Configuration |

---

## Data Structures

### Region Data

| Structure Name | Fields | Type | Purpose |
|---------------|--------|------|---------|
| **RegionData** | normalizedCoords | NormalizedCoords | Source of truth coordinates |
| | imageCoords | ImageCoords | Pixel positions |
| | canvasCoords | CanvasCoords | Display positions |
| | name | QString | Unique identifier |
| | color | QString | Visual color ("blue", "red", etc.) |
| | group | QString | Group name or empty |

### Coordinate Types

| Type Name | Fields | Type | Range |
|-----------|--------|------|-------|
| **NormalizedCoords** | x1, y1, x2, y2 | double | 0.0 to 1.0 |
| **ImageCoords** | x1, y1, x2, y2 | int | 0 to image dimensions |
| **CanvasCoords** | x1, y1, x2, y2 | double | 0 to canvas dimensions |

### Group Data

| Structure Name | Fields | Type | Purpose |
|---------------|--------|------|---------|
| **GroupData** | name | QString | Group identifier |
| | regionNames | QList<QString> | List of region names in group |

### Document State

| Structure Name | Fields | Type | Purpose |
|---------------|--------|------|---------|
| **DocumentState** | pdfPath | QString | Path to PDF file |
| | image | QImage | First page as image |
| | regions | QMap<QString, RegionData> | All regions (key: name) |
| | groups | QMap<QString, GroupData> | All groups (key: name) |
| | zoomLevel | double | Current zoom (1.0 = 100%) |
| | scaleFactor | double | Display scale factor |
| | imageOffset | QPointF | Image position on canvas |

---

## Classes and Components

### Core Classes

| Class Name | Purpose | Location | Key Methods |
|------------|---------|----------|-------------|
| **CoordinateSystem** | Coordinate transformations | src/core/ | normalizedToImage(), imageToCanvas(), etc. |
| **Region** | Region business logic | src/core/ | (if needed, otherwise use RegionData) |
| **Group** | Group business logic | src/core/ | (if needed, otherwise use GroupData) |
| **Document** | Document management | src/core/ | loadPdf(), saveState() |

### UI Classes

| Class Name | Purpose | Location | Key Methods |
|------------|---------|----------|-------------|
| **MainWindow** | Main application window | src/ui/ | setupUI(), loadPdf() |
| **Canvas** | PDF display and region rendering | src/ui/ | paintEvent(), mousePressEvent() |
| **Toolbar** | Top toolbar with controls | src/ui/ | (or part of MainWindow) |
| **SidePanel** | Right side panel with lists | src/ui/ | (or part of MainWindow) |

### Utility Classes

| Class Name | Purpose | Location | Key Methods |
|------------|---------|----------|-------------|
| **PdfLoader** | PDF to image conversion | src/utils/ | loadPdfFirstPage() |
| **JsonExporter** | JSON export | src/export/ | exportRegions() |
| **CsvExporter** | CSV export | src/export/ | exportRegions() |

---

## Functions and Methods

### Coordinate Transformations

| Function Name | Parameters | Return | Purpose |
|---------------|------------|--------|---------|
| **normalizedToImage** | NormalizedCoords, int width, int height | ImageCoords | Convert normalized to image |
| **imageToNormalized** | ImageCoords, int width, int height | NormalizedCoords | Convert image to normalized |
| **imageToCanvas** | ImageCoords, double scale, QPointF offset | CanvasCoords | Convert image to canvas |
| **canvasToImage** | CanvasCoords, double scale, QPointF offset | ImageCoords | Convert canvas to image |
| **normalizedToCanvas** | NormalizedCoords, int width, int height, double scale, QPointF offset | CanvasCoords | Convert normalized to canvas |

### Region Operations

| Function Name | Parameters | Return | Purpose |
|---------------|------------|--------|---------|
| **createRegion** | QString name, QRectF canvasRect | void | Create new region |
| **selectRegion** | QString name | void | Select single region |
| **selectRegions** | QSet<QString> names | void | Select multiple regions |
| **moveRegion** | QString name, QPointF delta | void | Move region by delta |
| **resizeRegion** | QString name, QString handle, QPointF newPos | void | Resize region using handle |
| **deleteRegion** | QString name | void | Delete region |

### Group Operations

| Function Name | Parameters | Return | Purpose |
|---------------|------------|--------|---------|
| **createGroup** | QString name, QList<QString> regionNames | void | Create new group |
| **addToGroup** | QString regionName, QString groupName | void | Add region to group |
| **removeFromGroup** | QString regionName | void | Remove region from group |
| **deleteGroup** | QString name | void | Delete group |

---

## Variables and State

### Application State

| Variable Name | Type | Scope | Purpose |
|---------------|------|-------|---------|
| **mouseMode** | enum {Create, Select} | Canvas | Current interaction mode |
| **selectedRegions** | QSet<QString> | Canvas | All selected regions |
| **primarySelectedRegion** | QString | Canvas | Region showing resize handles |
| **hoveredRegion** | QString | Canvas | Currently hovered region |
| **isDragging** | bool | Canvas | Drag operation in progress |
| **isResizing** | bool | Canvas | Resize operation in progress |
| **isCreating** | bool | Canvas | Region creation in progress |
| **zoomLevel** | double | DocumentState | Current zoom (1.0 = 100%) |
| **scaleFactor** | double | DocumentState | Display scale factor |
| **imageOffset** | QPointF | DocumentState | Image position on canvas |

### UI State

| Variable Name | Type | Scope | Purpose |
|---------------|------|-------|---------|
| **regionListBox** | QListWidget* | MainWindow | Regions list widget |
| **groupListBox** | QListWidget* | MainWindow | Groups list widget |
| **infoText** | QTextEdit* | MainWindow | Info display widget |
| **statusLabel** | QLabel* | MainWindow | Status message label |
| **maskEnabled** | bool | Canvas | Mask overlay enabled |

---

## Constants

| Constant Name | Value | Purpose |
|--------------|-------|---------|
| **MIN_ZOOM** | 0.2 | Minimum zoom level (20%) |
| **MAX_ZOOM** | 5.0 | Maximum zoom level (500%) |
| **ZOOM_STEP** | 1.2 | Zoom increment/decrement factor |
| **MIN_REGION_SIZE** | 10 | Minimum region size in pixels |
| **MAX_UNDO_LEVELS** | 50 | Maximum undo history |
| **HANDLE_SIZE** | 8 | Resize handle size in pixels |
| **HANDLE_TOLERANCE** | 13 | Handle click tolerance (8 + 5) |
| **PDF_DPI** | 150 | Default DPI for PDF rendering |

---

## Enumerations

| Enum Name | Values | Purpose |
|-----------|--------|---------|
| **MouseMode** | Create, Select | Interaction mode |
| **ResizeHandle** | nw, ne, sw, se, n, s, w, e | Resize handle positions |
| **RegionColor** | blue, red, green, yellow, purple, orange, cyan | Available region colors |
| **ExportFormat** | JSON, CSV, TXT, XML, YAML | Export format types |

---

## File Naming Conventions

### Source Files

| Pattern | Example | Purpose |
|---------|---------|---------|
| **PascalCase.h** | `MainWindow.h` | Header files |
| **PascalCase.cpp** | `MainWindow.cpp` | Source files |
| **PascalCase** | `CoordinateSystem` | Class names |

### Directories

| Pattern | Example | Purpose |
|---------|---------|---------|
| **lowercase/** | `src/core/` | Source directories |
| **PascalCase/** | `IMPLEMENTATION_PLAN/` | Documentation directories |

---

## Reserved Terms (DO NOT USE)

These terms are reserved or ambiguous - use alternatives:

| Avoid | Use Instead | Reason |
|-------|-------------|--------|
| `cell` | `region` | More generic, less ambiguous |
| `box` | `region` | More specific term |
| `area` | `region` | More specific term |
| `field` | `region` | Less ambiguous |
| `coords` | `coordinates` | Full word preferred |
| `pos` | `position` | Full word preferred |
| `rect` | `rectangle` | Full word in variable names |
| `img` | `image` | Full word preferred |
| `pdf` | `pdf` (in compound names) | OK in compound: `pdfPath` |

---

## Color Definitions

| Color Name | Hex Value | RGB | Usage |
|------------|-----------|-----|-------|
| **blue** | #0066ff | (0, 102, 255) | Default region color |
| **red** | #ff0000 | (255, 0, 0) | Region color option |
| **green** | #00cc00 | (0, 204, 0) | Region color option |
| **yellow** | #ffcc00 | (255, 204, 0) | Region color option |
| **purple** | #9900cc | (153, 0, 204) | Region color option |
| **orange** | #ff6600 | (255, 102, 0) | Region color option |
| **cyan** | #00cccc | (0, 204, 204) | Region color option |

---

## UI Color Scheme

| Element | Light Mode | Dark Mode | Purpose |
|---------|------------|-----------|---------|
| **Toolbar Background** | #e0e0e0 | #2d2d2d | Toolbar background |
| **Canvas Background** | #d0d0d0 | #1e1e1e | Canvas background |
| **Selection Box** | #0066ff | #58a6ff | Selection rectangle |
| **Mask Overlay** | (128,128,128,200) | (64,64,64,200) | Mask transparency |

---

## Keyboard Shortcuts

| Shortcut | Action | Function Name | Platform |
|----------|--------|---------------|----------|
| **1** | Switch to Create mode | `setModeCreate()` | All |
| **2** | Switch to Select mode | `setModeSelect()` | All |
| **Cmd+O** | Load PDF | `loadPdf()` | macOS |
| **Ctrl+O** | Load PDF | `loadPdf()` | Windows/Linux |
| **Cmd+S** | Export coordinates | `exportCoordinates()` | macOS |
| **Ctrl+S** | Export coordinates | `exportCoordinates()` | Windows/Linux |
| **Cmd+Z** | Undo | `undoAction()` | macOS |
| **Ctrl+Z** | Undo | `undoAction()` | Windows/Linux |
| **Cmd+Shift+Z** | Redo | `redoAction()` | macOS |
| **Ctrl+Shift+Z** | Redo | `redoAction()` | Windows/Linux |
| **Delete** | Delete selected | `deleteSelectedRegion()` | All |
| **E** | Edit region name | `editSelectedRegion()` | All |
| **M** | Toggle mask | `toggleMask()` | All |

---

## Export Format Structures

### JSON Format

```json
{
  "pdf_path": "string",
  "image_size": [width, height],
  "aspect_ratio": 0.6667,
  "regions": {
    "Region Name": {
      "normalized_coords": {"x1": 0.25, "y1": 0.10, "x2": 0.35, "y2": 0.20},
      "color": "blue",
      "group": "lastname"
    }
  },
  "groups": {
    "lastname": ["Region Name 1", "Region Name 2"]
  }
}
```

### CSV Format

| Column | Type | Description |
|--------|------|-------------|
| **Region Name** | string | Unique region identifier |
| **Group** | string | Group name or empty |
| **Color** | string | Color identifier |
| **X1 (%)** | double | Left edge (0.0-1.0) |
| **Y1 (%)** | double | Top edge (0.0-1.0) |
| **X2 (%)** | double | Right edge (0.0-1.0) |
| **Y2 (%)** | double | Bottom edge (0.0-1.0) |

---

## Naming Rules

### Classes
- **PascalCase**: `CoordinateSystem`, `MainWindow`, `RegionData`
- **No abbreviations**: Use full words
- **Descriptive**: Name describes purpose

### Functions
- **camelCase**: `createRegion()`, `normalizedToImage()`, `updateZoomDisplay()`
- **Verb first**: Action-oriented names
- **Clear purpose**: Name indicates what function does

### Variables
- **camelCase**: `selectedRegions`, `zoomLevel`, `isDragging`
- **Boolean prefix**: `is`, `has`, `can` (e.g., `isDragging`, `hasRegions`)
- **Descriptive**: Name indicates content/purpose

### Constants
- **UPPER_SNAKE_CASE**: `MAX_ZOOM`, `MIN_REGION_SIZE`, `HANDLE_SIZE`
- **All caps**: Clearly indicates constant
- **Underscore separated**: Readable

### Enums
- **PascalCase for type**: `MouseMode`, `ResizeHandle`
- **lowercase for values**: `Create`, `Select`, `nw`, `ne`

---

## Relationships

### Region → Group
- **Many-to-One**: Region belongs to at most one group
- **Optional**: Region can have no group (group = empty string or null)

### Group → Regions
- **One-to-Many**: Group contains multiple regions
- **Ordered**: QList preserves insertion order

### Document → Regions
- **One-to-Many**: Document contains multiple regions
- **Keyed by name**: QMap for fast lookup

### Document → Groups
- **One-to-Many**: Document contains multiple groups
- **Keyed by name**: QMap for fast lookup

---

## Coordinate System Relationships

```
Normalized (0.0-1.0)
    ↕ (multiply/divide by image dimensions)
Image (pixels)
    ↕ (multiply/divide by scale factor, add offset)
Canvas (display pixels)
```

**Source of Truth**: Normalized coordinates
**Recalculation Trigger**: Zoom or pan change
**Synchronization**: All three must stay in sync

---

## State Management

### Undo/Redo State

| Structure | Content | Purpose |
|-----------|---------|---------|
| **undoStack** | QList<StateSnapshot> | Previous states |
| **redoStack** | QList<StateSnapshot> | Undone states |
| **StateSnapshot** | {regions, groups} | Deep copy of state |

### Operation States

| State | Trigger | Action |
|-------|---------|--------|
| **isCreating** | Mouse press in Create mode | Start region creation |
| **isDragging** | Mouse press on selected region | Start region movement |
| **isResizing** | Mouse press on resize handle | Start region resize |
| **isBoxSelecting** | Mouse press in empty space | Start box selection |

---

## Error Conditions

| Error | Condition | Handling |
|-------|-----------|----------|
| **Duplicate Region Name** | Name already exists | Show error, reject creation |
| **Empty Region Name** | Name is empty | Show warning, reject creation |
| **Region Too Small** | Size < MIN_REGION_SIZE | Show warning, reject creation |
| **Invalid Coordinates** | Out of bounds | Clamp to bounds or reject |
| **PDF Load Failed** | File invalid or missing | Show error, maintain current state |
| **Export Failed** | File write error | Show error, preserve data |

---

## Version Information

- **Document Version**: 1.0
- **Last Updated**: December 2025
- **Maintained By**: Development Team

---

## Usage Guidelines

1. **Always use terms from this ontology** - Don't create new terms without updating this document
2. **Check before naming** - Verify term doesn't already exist
3. **Update ontology** - When adding new concepts, update this document
4. **Consistent naming** - Follow naming conventions strictly
5. **Document exceptions** - If you must deviate, document why

---

*This ontology is the single source of truth for all terminology in the Optical Wizard Doctor project.*

