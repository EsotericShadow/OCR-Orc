# Nomenclature - Naming Conventions and Glossary
## Complete Reference for All Naming Rules

**Purpose**: Define exact naming conventions to prevent conflicts and ensure consistency.

---

## Naming Rules Summary

### Classes
- **Format**: PascalCase
- **Example**: `CoordinateSystem`, `MainWindow`, `RegionData`
- **Rule**: No abbreviations, descriptive names

### Functions/Methods
- **Format**: camelCase
- **Example**: `createRegion()`, `normalizedToImage()`, `updateZoomDisplay()`
- **Rule**: Verb first, action-oriented

### Variables
- **Format**: camelCase
- **Example**: `selectedRegions`, `zoomLevel`, `isDragging`
- **Rule**: Descriptive, boolean prefix with `is`/`has`/`can`

### Constants
- **Format**: UPPER_SNAKE_CASE
- **Example**: `MAX_ZOOM`, `MIN_REGION_SIZE`, `HANDLE_SIZE`
- **Rule**: All caps, underscore separated

### Enums
- **Type Name**: PascalCase (`MouseMode`, `ResizeHandle`)
- **Values**: lowercase (`Create`, `Select`, `nw`, `ne`)

### Files
- **Headers**: PascalCase.h (`MainWindow.h`)
- **Sources**: PascalCase.cpp (`MainWindow.cpp`)
- **Directories**: lowercase (`src/core/`)

---

## Complete Class Name List

### Core Classes
- `CoordinateSystem` - Coordinate transformations
- `RegionData` - Region data structure
- `GroupData` - Group data structure
- `DocumentState` - Application state management

### UI Classes
- `MainWindow` - Main application window
- `Canvas` - PDF display and region rendering widget
- `Toolbar` - Toolbar component (or part of MainWindow)
- `SidePanel` - Side panel component (or part of MainWindow)

### Utility Classes
- `PdfLoader` - PDF to image conversion
- `JsonExporter` - JSON export functionality
- `CsvExporter` - CSV export functionality
- `TxtExporter` - TXT export functionality
- `XmlExporter` - XML export functionality
- `YamlExporter` - YAML export functionality
- `MaskGenerator` - Mask image generation

---

## Complete Function Name List

### Coordinate System Functions

| Function Name | Parameters | Return Type |
|---------------|------------|-------------|
| `normalizedToImage` | NormalizedCoords, int width, int height | ImageCoords |
| `imageToNormalized` | ImageCoords, int width, int height | NormalizedCoords |
| `imageToCanvas` | ImageCoords, double scale, QPointF offset | CanvasCoords |
| `canvasToImage` | CanvasCoords, double scale, QPointF offset | ImageCoords |
| `normalizedToCanvas` | NormalizedCoords, int width, int height, double scale, QPointF offset | CanvasCoords |
| `isValidNormalized` | NormalizedCoords | bool |
| `isValidImage` | ImageCoords, int width, int height | bool |
| `isValidCanvas` | CanvasCoords | bool |
| `normalizeCoords` | NormalizedCoords& | void |
| `clampToBounds` | NormalizedCoords& | void |
| `calculateAspectRatio` | int width, int height | double |

### Document State Functions

| Function Name | Parameters | Return Type |
|---------------|------------|-------------|
| `addRegion` | QString name, RegionData region | void |
| `removeRegion` | QString name | void |
| `hasRegion` | QString name | bool |
| `getRegion` | QString name | RegionData |
| `getAllRegionNames` | void | QList<QString> |
| `createGroup` | QString name | void |
| `deleteGroup` | QString name | void |
| `addRegionToGroup` | QString regionName, QString groupName | void |
| `removeRegionFromGroup` | QString regionName | void |
| `getGroupNames` | void | QList<QString> |
| `getGroup` | QString name | GroupData |
| `synchronizeCoordinates` | CoordinateSystem& cs | void |
| `isValid` | void | bool |
| `toJson` | void | QJsonObject |
| `fromJson` | QJsonObject json | static DocumentState |

### Canvas Functions

| Function Name | Parameters | Return Type |
|---------------|------------|-------------|
| `loadPdf` | QString filePath | void |
| `setImage` | QImage image | void |
| `setMode` | MouseMode mode | void |
| `setModeCreate` | void | void |
| `setModeSelect` | void | void |
| `createRegion` | QString name, QRectF canvasRect | void |
| `selectRegion` | QString name | void |
| `selectRegions` | QSet<QString> names | void |
| `deselectAll` | void | void |
| `selectAll` | void | void |
| `invertSelection` | void | void |
| `moveRegion` | QString name, QPointF delta | void |
| `resizeRegion` | QString name, QString handle, QPointF newPos | void |
| `deleteRegion` | QString name | void |
| `zoomIn` | void | void |
| `zoomOut` | void | void |
| `resetZoom` | void | void |
| `updateZoomDisplay` | void | void |
| `renderRegions` | QPainter& painter | void |
| `drawResizeHandles` | QPainter& painter, QRectF rect | void |
| `drawRegionLabel` | QPainter& painter, QString name, QRectF rect, bool isSelected | void |
| `getRegionAt` | QPointF canvasPos | QString |
| `getHandleAt` | QPointF pos, QRectF rect | QString |
| `toggleMask` | void | void |

### MainWindow Functions

| Function Name | Parameters | Return Type |
|---------------|------------|-------------|
| `setupUI` | void | void |
| `setupToolbar` | void | void |
| `setupControlPanel` | void | void |
| `setupMainFrame` | void | void |
| `setupSidePanel` | void | void |
| `loadPdf` | void | void |
| `exportCoordinates` | void | void |
| `exportMask` | void | void |
| `loadCoordinates` | void | void |
| `updateRegionList` | void | void |
| `updateGroupList` | void | void |
| `showHelp` | void | void |

### Export Functions

| Function Name | Parameters | Return Type |
|---------------|------------|-------------|
| `exportJson` | QString filePath, DocumentState state | void |
| `exportCsv` | QString filePath, DocumentState state | void |
| `exportTxt` | QString filePath, DocumentState state | void |
| `exportXml` | QString filePath, DocumentState state | void |
| `exportYaml` | QString filePath, DocumentState state | void |
| `generateMask` | QString filePath, DocumentState state | void |

### Import Functions

| Function Name | Parameters | Return Type |
|---------------|------------|-------------|
| `importJson` | QString filePath | DocumentState |
| `validateCoordinates` | QJsonObject json | bool |

---

## Complete Variable Name List

### Application State Variables

| Variable Name | Type | Scope | Purpose |
|---------------|------|-------|---------|
| `mouseMode` | MouseMode | Canvas | Current interaction mode |
| `selectedRegions` | QSet<QString> | Canvas | All selected regions |
| `primarySelectedRegion` | QString | Canvas | Region showing resize handles |
| `hoveredRegion` | QString | Canvas | Currently hovered region |
| `isDragging` | bool | Canvas | Drag operation in progress |
| `isResizing` | bool | Canvas | Resize operation in progress |
| `isCreating` | bool | Canvas | Region creation in progress |
| `isBoxSelecting` | bool | Canvas | Box selection in progress |
| `zoomLevel` | double | DocumentState | Current zoom (1.0 = 100%) |
| `scaleFactor` | double | DocumentState | Display scale factor |
| `imageOffset` | QPointF | DocumentState | Image position on canvas |
| `maskEnabled` | bool | Canvas | Mask overlay enabled |

### Operation State Variables

| Variable Name | Type | Scope | Purpose |
|---------------|------|-------|---------|
| `dragStartPos` | QPointF | Canvas | Starting position of drag |
| `lastDragPos` | QPointF | Canvas | Last position during drag |
| `resizeStartPos` | QPointF | Canvas | Starting position of resize |
| `resizeHandle` | QString | Canvas | Which handle being dragged |
| `originalRect` | QRectF | Canvas | Original region bounds before resize |
| `selectionBoxStart` | QPointF | Canvas | Starting point of selection box |
| `selectionBox` | QRectF | Canvas | Current selection box rectangle |
| `tempRect` | QGraphicsRectItem* | Canvas | Temporary rectangle during creation |

### UI Component Variables

| Variable Name | Type | Scope | Purpose |
|---------------|------|-------|---------|
| `regionListBox` | QListWidget* | MainWindow | Regions list widget |
| `groupListBox` | QListWidget* | MainWindow | Groups list widget |
| `infoText` | QTextEdit* | MainWindow | Info display widget |
| `statusLabel` | QLabel* | MainWindow | Status message label |
| `zoomLabel` | QLabel* | MainWindow | Zoom percentage label |
| `regionNameEntry` | QLineEdit* | MainWindow | Region name input |
| `groupEntry` | QLineEdit* | MainWindow | Group name input |
| `colorComboBox` | QComboBox* | MainWindow | Color selection |

---

## Complete Constant List

| Constant Name | Value | Type | Purpose |
|---------------|-------|------|---------|
| `MIN_ZOOM` | 0.2 | constexpr double | Minimum zoom level (20%) |
| `MAX_ZOOM` | 5.0 | constexpr double | Maximum zoom level (500%) |
| `ZOOM_STEP` | 1.2 | constexpr double | Zoom increment factor |
| `MIN_REGION_SIZE` | 10 | constexpr int | Minimum region size in pixels |
| `MAX_UNDO_LEVELS` | 50 | constexpr int | Maximum undo history |
| `HANDLE_SIZE` | 8 | constexpr int | Resize handle size in pixels |
| `HANDLE_TOLERANCE` | 13 | constexpr int | Handle click tolerance |
| `PDF_DPI` | 150 | constexpr int | Default DPI for PDF rendering |
| `CANVAS_BACKGROUND` | #d0d0d0 | const QString | Canvas background color |
| `TOOLBAR_BACKGROUND` | #e0e0e0 | const QString | Toolbar background color |
| `SELECTION_COLOR` | #0066ff | const QString | Selection box color |

---

## Enumeration Definitions

### MouseMode Enum

```cpp
enum class MouseMode {
    Create,   // Create new regions
    Select    // Select and manipulate regions
};
```

### ResizeHandle Enum

```cpp
enum class ResizeHandle {
    nw,  // Top-left corner
    ne,  // Top-right corner
    sw,  // Bottom-left corner
    se,  // Bottom-right corner
    n,   // Top edge
    s,   // Bottom edge
    w,   // Left edge
    e    // Right edge
};
```

### RegionColor Enum (or use QString)

```cpp
// Option 1: Use QString directly
// "blue", "red", "green", "yellow", "purple", "orange", "cyan"

// Option 2: Enum (if type safety needed)
enum class RegionColor {
    Blue,
    Red,
    Green,
    Yellow,
    Purple,
    Orange,
    Cyan
};
```

### ExportFormat Enum

```cpp
enum class ExportFormat {
    JSON,
    CSV,
    TXT,
    XML,
    YAML
};
```

---

## File Naming Patterns

### Source Files

| Pattern | Example | Location |
|---------|---------|----------|
| `ClassName.h` | `MainWindow.h` | `src/ui/` |
| `ClassName.cpp` | `MainWindow.cpp` | `src/ui/` |
| `ClassName.h` | `CoordinateSystem.h` | `src/core/` |
| `ClassName.cpp` | `CoordinateSystem.cpp` | `src/core/` |

### Test Files

| Pattern | Example | Location |
|---------|---------|----------|
| `test_classname.cpp` | `test_coordinate_system.cpp` | `tests/` |
| `test_feature.cpp` | `test_region_creation.cpp` | `tests/` |

### Documentation Files

| Pattern | Example | Location |
|---------|---------|----------|
| `FEATURE.md` | `COORDINATE_SYSTEM.md` | `docs/` |
| `IMPLEMENTATION_STEPS.md` | `IMPLEMENTATION_STEPS.md` | `IMPLEMENTATION_PLAN/` |

---

## Namespace Usage

### Recommended Namespace

```cpp
namespace owd {
    // All application code
}

// Usage
using namespace owd;
// or
owd::MainWindow window;
```

### Alternative: No Namespace

- Use class prefixes if needed
- Keep names descriptive and unique

---

## Reserved Words (DO NOT USE)

### C++ Keywords
- `class`, `struct`, `enum`, `namespace`, `template`, etc.

### Qt Reserved
- `QObject`, `QWidget`, `QMainWindow` (use as base classes only)

### Common Conflicts
- `region` - Use `RegionData` or `regionData`
- `group` - Use `GroupData` or `groupData`
- `document` - Use `DocumentState` or `documentState`
- `state` - Use `DocumentState` or `appState`

### Ambiguous Terms
- `cell` - Use `region`
- `box` - Use `region` or `rectangle`
- `area` - Use `region`
- `field` - Use `region`

---

## Naming Patterns by Context

### Getter Functions
- **Pattern**: `get` + Noun
- **Example**: `getRegion()`, `getGroup()`, `getZoomLevel()`
- **Alternative**: Just noun if obvious (`zoomLevel()`)

### Setter Functions
- **Pattern**: `set` + Noun
- **Example**: `setMode()`, `setZoomLevel()`, `setImage()`

### Boolean Functions
- **Pattern**: `is`/`has`/`can` + Adjective/Noun
- **Example**: `isValid()`, `hasRegion()`, `canUndo()`

### Action Functions
- **Pattern**: Verb + Object
- **Example**: `createRegion()`, `deleteRegion()`, `moveRegion()`

### Transformation Functions
- **Pattern**: `source` + `To` + `Destination`
- **Example**: `normalizedToImage()`, `imageToCanvas()`

---

## Color Name Mapping

| Display Name | Internal Value | Hex Code |
|--------------|----------------|----------|
| Blue | "blue" | #0066ff |
| Red | "red" | #ff0000 |
| Green | "green" | #00cc00 |
| Yellow | "yellow" | #ffcc00 |
| Purple | "purple" | #9900cc |
| Orange | "orange" | #ff6600 |
| Cyan | "cyan" | #00cccc |

**Rule**: Always use lowercase string internally, convert to QColor when rendering.

---

## Coordinate Naming

### Coordinate Fields
- **x1, y1**: Top-left corner (always)
- **x2, y2**: Bottom-right corner (always)
- **Rule**: Always ensure x1 < x2, y1 < y2

### Coordinate Variables
- **normalizedCoords**: NormalizedCoords type
- **imageCoords**: ImageCoords type
- **canvasCoords**: CanvasCoords type
- **Rule**: Use full name, not abbreviation

---

## File Path Variables

| Variable Name | Type | Purpose |
|---------------|------|---------|
| `pdfPath` | QString | Path to loaded PDF file |
| `exportPath` | QString | Path for export file |
| `importPath` | QString | Path for import file |
| `maskPath` | QString | Path for mask image |

**Rule**: Always use `Path` suffix, not `File` or `Filename`.

---

## State Variable Naming

### Boolean State
- **Prefix**: `is`, `has`, `can`
- **Example**: `isDragging`, `hasRegions`, `canUndo`

### Collection State
- **Suffix**: Plural form
- **Example**: `selectedRegions`, `regionNames`, `groupNames`

### Current/Active State
- **Prefix**: `current`, `active`, `selected`
- **Example**: `currentRegion`, `activeMode`, `selectedRegion`

---

## Event Handler Naming

### Qt Event Handlers
- **Pattern**: `on` + EventName
- **Example**: `onClick()`, `onDrag()`, `onRelease()`
- **Qt Override**: Use Qt naming (`mousePressEvent()`)

### Custom Event Handlers
- **Pattern**: `handle` + EventName
- **Example**: `handleRegionClick()`, `handleZoomChange()`

---

## Error Handling Naming

### Error Functions
- **Pattern**: `validate` + Object or `check` + Object
- **Example**: `validateRegion()`, `checkCoordinates()`

### Error Variables
- **Pattern**: `error` + Description or `is` + ErrorType
- **Example**: `errorMessage`, `isValid`

---

## Testing Naming

### Test Functions
- **Pattern**: `test` + FeatureName
- **Example**: `testNormalizedToImage()`, `testRegionCreation()`

### Test Data
- **Pattern**: `test` + DataName
- **Example**: `testRegion`, `testCoordinates`

---

## Quick Reference Cheat Sheet

### Class → File
```
ClassName → ClassName.h, ClassName.cpp
```

### Function → Implementation
```
camelCase() → void camelCase() { }
```

### Variable → Usage
```
camelCase → Type camelCase;
```

### Constant → Definition
```
UPPER_SNAKE_CASE → constexpr Type UPPER_SNAKE_CASE = value;
```

---

*This nomenclature document is the definitive reference for all naming in the project.*

