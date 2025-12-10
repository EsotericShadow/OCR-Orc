# Meta Architecture - System Relationships and Dependencies
## How Everything Works Together

**Purpose**: Map all components, their relationships, dependencies, and data flow.

---

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Optical Wizard Doctor                     │
│                         Application                          │
└─────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                       │
        ▼                     ▼                       ▼
┌──────────────┐    ┌──────────────┐      ┌──────────────┐
│  UI Layer     │    │  Core Layer  │      │  Data Layer  │
│  (Qt Widgets) │◄───┤  (Business   │◄─────┤  (State &    │
│               │    │   Logic)     │      │   Models)    │
└──────────────┘    └──────────────┘      └──────────────┘
        │                     │                       │
        └─────────────────────┼───────────────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │  Export/Import   │
                    │     Layer        │
                    └─────────────────┘
```

---

## Layer Architecture

### Layer 1: UI Layer (Presentation)

**Components**:
- `MainWindow` - Main application window
- `Canvas` - PDF display and region rendering
- `Toolbar` - Mode, zoom, group controls
- `SidePanel` - Region list, group list, info display

**Responsibilities**:
- User interaction (mouse, keyboard)
- Visual rendering
- Event handling
- UI state management

**Dependencies**:
- Core Layer (business logic)
- Data Layer (state access)
- Qt Framework

---

### Layer 2: Core Layer (Business Logic)

**Components**:
- `CoordinateSystem` - Coordinate transformations
- `Region` - Region operations (if class needed)
- `Group` - Group operations (if class needed)
- `Document` - Document management

**Responsibilities**:
- Coordinate transformations
- Region/group operations
- Validation
- Business rules

**Dependencies**:
- Data Layer (data structures)
- No UI dependencies (pure logic)

---

### Layer 3: Data Layer (State Management)

**Components**:
- `DocumentState` - Application state
- `RegionData` - Region data structure
- `GroupData` - Group data structure
- Undo/Redo system

**Responsibilities**:
- State storage
- Data structures
- State persistence
- Undo/redo management

**Dependencies**:
- None (foundation layer)

---

### Layer 4: Export/Import Layer

**Components**:
- `JsonExporter` - JSON export
- `CsvExporter` - CSV export
- `MaskGenerator` - Mask image generation
- Import handlers

**Responsibilities**:
- Data serialization
- Format conversion
- File I/O

**Dependencies**:
- Data Layer (reads/writes state)
- Qt JSON/XML support

---

## Component Relationships

### MainWindow → Canvas

**Relationship**: Contains
- MainWindow creates and manages Canvas widget
- Canvas is child widget in MainWindow layout
- MainWindow forwards events to Canvas

**Data Flow**:
```
User Action → MainWindow → Canvas → DocumentState → Update
```

### Canvas → DocumentState

**Relationship**: Uses
- Canvas reads from DocumentState (regions, groups, image)
- Canvas writes to DocumentState (new regions, updates)
- Canvas triggers state changes

**Data Flow**:
```
Canvas Event → Update DocumentState → Redraw Canvas
```

### CoordinateSystem → RegionData

**Relationship**: Transforms
- CoordinateSystem transforms RegionData coordinates
- RegionData stores coordinates
- CoordinateSystem calculates transformations

**Data Flow**:
```
RegionData (normalized) → CoordinateSystem → RegionData (image/canvas)
```

### DocumentState → Export/Import

**Relationship**: Serializes
- Export reads from DocumentState
- Import writes to DocumentState
- State is serialized/deserialized

**Data Flow**:
```
DocumentState → Exporter → File
File → Importer → DocumentState
```

---

## Data Flow Diagrams

### Region Creation Flow

```
User Click/Drag
    ↓
Canvas::mousePressEvent()
    ↓
Canvas::mouseMoveEvent() [draw temp rectangle]
    ↓
Canvas::mouseReleaseEvent()
    ↓
Convert Canvas Coords → Image Coords → Normalized Coords
    ↓
Create RegionData
    ↓
DocumentState::addRegion()
    ↓
Update UI (listbox, canvas redraw)
    ↓
Save to Undo Stack
```

### Region Movement Flow

```
User Drag Selected Region
    ↓
Canvas::mouseMoveEvent()
    ↓
Calculate Delta (canvas coordinates)
    ↓
For each selected region:
    Convert Delta → Image Delta → Normalized Delta
    Update all three coordinate systems
    ↓
Canvas::redrawRegions()
    ↓
Canvas::mouseReleaseEvent()
    ↓
Save to Undo Stack
```

### Zoom Flow

```
User Zooms (button/keyboard/scroll)
    ↓
Canvas::zoomIn() / zoomOut()
    ↓
Update DocumentState::zoomLevel
    ↓
Recalculate DocumentState::scaleFactor
    ↓
For each region:
    Recalculate canvas coordinates from normalized
    ↓
Canvas::redrawRegions()
    ↓
Update zoom display label
```

### Export Flow

```
User Clicks Export
    ↓
MainWindow::exportCoordinates()
    ↓
Show file dialog
    ↓
Determine format from extension
    ↓
Create appropriate Exporter (JsonExporter, etc.)
    ↓
Exporter reads DocumentState
    ↓
Serialize to file format
    ↓
Write to file
    ↓
Show success message
```

---

## Dependency Graph

```
DocumentState (foundation)
    ↑
    ├── CoordinateSystem (uses for transformations)
    ├── Canvas (reads/writes)
    ├── MainWindow (reads/writes)
    └── Exporters (reads)

CoordinateSystem (standalone)
    ↑
    └── Canvas (uses for coordinate conversions)

Canvas (depends on)
    ├── DocumentState
    ├── CoordinateSystem
    └── Qt Widgets

MainWindow (depends on)
    ├── Canvas
    ├── DocumentState
    └── Qt Widgets

Exporters (depends on)
    ├── DocumentState
    └── Qt JSON/XML
```

---

## State Synchronization

### Coordinate Synchronization

**Rule**: Normalized coordinates are source of truth

**Synchronization Points**:
1. **Region Creation**: Canvas → Image → Normalized
2. **Region Movement**: Update all three with delta
3. **Region Resize**: Update all three with new bounds
4. **Zoom Change**: Recalculate Canvas from Normalized
5. **Image Load**: Calculate Image from Normalized

**Synchronization Function**:
```cpp
void synchronizeCoordinates(RegionData& region, 
                            const CoordinateSystem& cs,
                            int imageWidth, int imageHeight,
                            double scaleFactor, QPointF offset) {
    // 1. Ensure normalized is valid (source of truth)
    // 2. Calculate image from normalized
    // 3. Calculate canvas from normalized
    // 4. Update region data
}
```

### UI State Synchronization

**Rule**: UI reflects DocumentState

**Synchronization Points**:
1. **Region Created**: Update listbox, redraw canvas
2. **Region Selected**: Update info display, redraw canvas
3. **Group Created**: Update group listbox
4. **Zoom Changed**: Update zoom label, redraw canvas
5. **State Restored**: Update all UI elements

---

## Event Flow

### Mouse Event Flow

```
Mouse Event
    ↓
Canvas::mousePressEvent()
    ↓
Check Mode (Create/Select)
    ↓
    ├── Create Mode:
    │   └── Start region creation
    │
    └── Select Mode:
        ├── Check resize handle → Start resize
        ├── Check region → Start drag
        └── Empty space → Start box selection
    ↓
Canvas::mouseMoveEvent()
    ↓
Update operation (create/drag/resize/box)
    ↓
Canvas::mouseReleaseEvent()
    ↓
Finalize operation
    ↓
Update DocumentState
    ↓
Save to Undo Stack
    ↓
Redraw Canvas
```

### Keyboard Event Flow

```
Keyboard Event
    ↓
MainWindow::keyPressEvent() or Canvas::keyPressEvent()
    ↓
Check key combination
    ↓
    ├── Mode switching (1, 2)
    ├── Zoom (+,-,0)
    ├── File ops (Cmd+O, Cmd+S)
    ├── Selection (Cmd+A, Cmd+D, Cmd+I)
    ├── Undo/Redo (Cmd+Z, Cmd+Shift+Z)
    └── Other shortcuts
    ↓
Execute action
    ↓
Update DocumentState (if needed)
    ↓
Update UI
```

---

## Memory Management

### Ownership Model

```
MainWindow
    ├── owns → Canvas (QWidget parent-child)
    ├── owns → Toolbar widgets
    ├── owns → SidePanel widgets
    └── owns → DocumentState (unique_ptr or direct)

DocumentState
    ├── owns → QImage (value, not pointer)
    ├── owns → QMap<QString, RegionData> (values)
    └── owns → QMap<QString, GroupData> (values)

Canvas
    ├── references → DocumentState (pointer or reference)
    └── references → CoordinateSystem (can be static or instance)

Exporters
    └── reads → DocumentState (const reference)
```

### Smart Pointers Usage

- **DocumentState**: `std::unique_ptr<DocumentState>` in MainWindow
- **CoordinateSystem**: Can be static utility or instance
- **Qt Widgets**: Qt's parent-child ownership (automatic cleanup)

---

## Thread Safety

### Current Design: Single Threaded

- **UI Thread**: All operations on main thread
- **No Background Threads**: PDF loading, export on UI thread
- **Future Consideration**: Background PDF loading for large files

### Thread Safety Rules

1. **No concurrent access**: Single thread only
2. **Qt Requirements**: All Qt operations on UI thread
3. **State Access**: Direct access (no locking needed)

---

## Error Handling Strategy

### Error Propagation

```
Operation
    ↓
Try/Catch or Return Error Code
    ↓
    ├── Success → Continue
    │
    └── Error → Show Message
                Maintain State
                Log Error
```

### Error Types

1. **User Input Errors**: Validation errors, show warnings
2. **File I/O Errors**: Show error dialog, preserve state
3. **State Errors**: Validate and correct, log issues
4. **System Errors**: Show error, attempt recovery

---

## Performance Considerations

### Optimization Points

1. **Coordinate Calculations**: Cache when possible
2. **Canvas Redraw**: Only redraw dirty regions
3. **Region Rendering**: Viewport culling (only visible regions)
4. **Undo Stack**: Limit to 50 states
5. **Image Caching**: Cache rendered PDF image

### Performance Targets

- **Canvas Redraw**: < 100ms for 50 regions
- **Zoom Operation**: < 50ms
- **Region Movement**: Real-time (60 FPS)
- **Export**: < 1s for 500 regions

---

## Testing Strategy

### Unit Tests

- **CoordinateSystem**: All transformation functions
- **Data Models**: Region/Group operations
- **Exporters**: Serialization correctness

### Integration Tests

- **Canvas + DocumentState**: Region creation/editing
- **Export/Import**: Round-trip correctness
- **Undo/Redo**: State restoration

### UI Tests

- **Manual Testing**: Mouse/keyboard interactions
- **Visual Testing**: Rendering correctness
- **Workflow Testing**: Complete user workflows

---

## Extension Points

### Future Enhancements

1. **OCR Integration**: Optional OCR testing
2. **Multiple Pages**: Support multi-page PDFs
3. **Templates**: Save/load region templates
4. **Batch Processing**: Process multiple PDFs
5. **Plugins**: Export format plugins

### Extension Architecture

- **Interface-Based**: Define interfaces for extensions
- **Plugin System**: Load extensions dynamically
- **Configuration**: Enable/disable features

---

*This meta architecture document defines how all components interact and depend on each other.*

