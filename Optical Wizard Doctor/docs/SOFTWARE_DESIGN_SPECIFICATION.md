# Software Design Specification

## 1. Application Architecture

### 1.1 High-Level Overview

OCR-Orc (OCR-Orc) is a **single-window desktop GUI application** with the following architectural components:

1. **Main Application Window** - Primary container for all UI elements
2. **Document Canvas** - Interactive drawing surface for PDF display and region manipulation
3. **Region Management System** - Data structures and logic for coordinate regions
4. **Export/Import System** - Multi-format coordinate serialization
5. **Undo/Redo System** - State management for operation history
6. **Event Handling System** - Mouse, keyboard, and window event processing

### 1.2 Component Structure

```
OCR-Orc Application
├── UI Layer
│   ├── Toolbar (Mode, Zoom, Group, Undo/Redo controls)
│   ├── Control Panel (File ops, Region input, Color selection)
│   ├── Main Canvas (PDF display, region visualization)
│   └── Side Panel (Region list, Group list, Info display, Help)
├── Data Layer
│   ├── Region Storage (Dictionary: name -> region data)
│   ├── Group Storage (Dictionary: group_name -> [region_names])
│   ├── Document State (PDF path, image, scale factors)
│   └── UI State (Selection, zoom, mode, hover)
└── Service Layer
    ├── Coordinate Transformation (Canvas <-> Image <-> Normalized)
    ├── Export/Import Services (JSON, CSV, TXT, XML, YAML)
    ├── Mask Generation Service
    └── Undo/Redo Service
```

### 1.3 Design Patterns

- **Model-View-Controller (MVC)**: 
  - Model: Region data structures, groups, document state
  - View: UI components (canvas, lists, controls)
  - Controller: Event handlers, coordinate transformations

- **State Pattern**: 
  - Mouse modes (Create vs Select)
  - Operation states (Dragging, Resizing, Selecting)

- **Command Pattern**: 
  - Undo/Redo system stores state snapshots
  - Each operation saves state before execution

## 2. Core Data Structures

### 2.1 Region Data Structure

Each region is stored as a dictionary with the following structure:

```python
{
    'image_coords': [x1, y1, x2, y2],      # Pixel coordinates in original image
    'normalized_coords': [x1, y1, x2, y2], # Normalized 0.0-1.0 coordinates
    'canvas_coords': [x1, y1, x2, y2],     # Display coordinates on canvas
    'color': 'blue',                        # Visual color identifier
    'group': 'lastname'                     # Optional group assignment (None if ungrouped)
}
```

**Requirements:**
- All three coordinate systems must be maintained and kept in sync
- Normalized coordinates are the source of truth for exports
- Canvas coordinates are recalculated on zoom/pan changes
- Image coordinates are derived from normalized when image is loaded

### 2.2 Groups Data Structure

Groups are stored as a dictionary mapping group names to lists of region names:

```python
{
    'lastname': ['Last Name Cell 1', 'Last Name Cell 2', ...],
    'firstname': ['First Name Cell 1', 'First Name Cell 2', ...],
    'studentnumber': ['Student Number Cell 1', ...]
}
```

**Requirements:**
- Groups can contain any number of regions
- Regions can belong to at most one group (or None)
- Deleting a region removes it from its group
- Deleting all regions in a group removes the group

### 2.3 Application State

The application maintains the following state variables:

**Document State:**
- `pdf_path`: Path to loaded PDF file
- `image`: PIL Image object of the first page
- `scale_factor`: Current display scale (accounts for zoom)
- `zoom_level`: Zoom multiplier (1.0 = 100%)
- `image_x`, `image_y`: Image position on canvas

**Selection State:**
- `selected_region`: Primary selected region (for resize handles)
- `selected_regions`: Set of all selected regions (for multi-select)
- `hovered_region`: Currently hovered region (for visual feedback)

**Operation State:**
- `mouse_mode`: "create" or "select"
- `dragging`: Boolean indicating drag operation in progress
- `resizing`: Boolean indicating resize operation in progress
- `selection_box_start`: Starting point for box selection
- `start_x`, `start_y`: Starting coordinates for current operation

**Undo/Redo State:**
- `undo_stack`: List of state snapshots (max 50)
- `redo_stack`: List of undone state snapshots

## 3. Coordinate System Architecture

### 3.1 Three Coordinate Systems

OCR-Orc maintains three coordinate systems that must be kept synchronized:

1. **Normalized Coordinates (0.0-1.0)**
   - Source of truth for exports
   - Resolution-independent
   - Range: 0.0 to 1.0 (percentages)
   - Format: [x1, y1, x2, y2]

2. **Image Coordinates (Pixels)**
   - Pixel positions in the original PDF image
   - Resolution-dependent
   - Calculated from normalized: `pixel = normalized * image_dimension`
   - Format: [x1, y1, x2, y2] (integers)

3. **Canvas Coordinates (Display Pixels)**
   - Screen positions for rendering
   - Accounts for zoom, pan, and image offset
   - Recalculated on every zoom/pan change
   - Format: [x1, y1, x2, y2] (floats)

### 3.2 Coordinate Transformation Rules

**Normalized → Image:**
```
image_x = normalized_x * image_width
image_y = normalized_y * image_height
```

**Image → Canvas:**
```
canvas_x = (image_x * scale_factor) + image_x_offset
canvas_y = (image_y * scale_factor) + image_y_offset
```

**Canvas → Image (for user input):**
```
image_x = (canvas_x - image_x_offset) / scale_factor
image_y = (canvas_y - image_y_offset) / scale_factor
```

**Image → Normalized:**
```
normalized_x = image_x / image_width
normalized_y = image_y / image_height
```

### 3.3 Coordinate Synchronization Requirements

- When a region is created: Calculate all three coordinate systems
- When a region is moved: Update all three coordinate systems
- When a region is resized: Update all three coordinate systems
- When zoom changes: Recalculate canvas coordinates from normalized
- When image loads: Calculate image coordinates from normalized

## 4. UI Component Specifications

### 4.1 Window Layout

**Window Dimensions:**
- Default: 1200x800 pixels
- Minimum: 800x600 pixels
- Resizable: Yes (with debounced redraw)

**Layout Structure:**
```
┌─────────────────────────────────────────────────────┐
│ Toolbar (Mode, Zoom, Groups, Undo/Redo)            │
├─────────────────────────────────────────────────────┤
│ Control Panel (File, Region Name, Group, Color)    │
├──────────────┬──────────────────────────────────────┤
│              │                                      │
│  Canvas      │  Side Panel                         │
│  (PDF View)  │  ├─ Regions List                    │
│              │  ├─ Groups List                     │
│              │  ├─ Info Display                    │
│              │  └─ Help Button (?)                 │
│              │                                      │
└──────────────┴──────────────────────────────────────┘
```

### 4.2 Toolbar Components

**Mode Selection:**
- Two radio buttons: "Create" and "Select/Move"
- Visual indication of current mode
- Keyboard shortcuts: 1 (Create), 2 (Select)

**Zoom Controls:**
- Zoom out button (-)
- Zoom percentage display (100%)
- Zoom in button (+)
- Reset zoom button
- Keyboard shortcuts: +/- (zoom), 0 (reset)

**Group Controls:**
- Group Selected button
- Ungroup Selected button
- Keyboard shortcuts: Cmd+G (group), Cmd+Shift+G (ungroup)

**Undo/Redo Controls:**
- Undo button (enabled when undo stack has items)
- Redo button (enabled when redo stack has items)
- Keyboard shortcuts: Cmd+Z (undo), Cmd+Shift+Z or Cmd+Y (redo)

### 4.3 Canvas Specifications

**Canvas Features:**
- Scrollable (horizontal and vertical scrollbars)
- Light grey background (#d0d0d0)
- Document shadow effect (5px offset, grey)
- Zoom support (0.2x to 5.0x)
- Pan support (via scrollbars)

**Visual Elements:**
- PDF image (centered, scaled)
- Region rectangles (colored borders)
- Region labels (above each region)
- Resize handles (8 handles per selected region)
- Selection box (dashed blue rectangle during drag selection)
- Mask overlay (semi-transparent grey when enabled)

**Region Visual States:**
- Normal: 2px border, no fill
- Hovered: 3px border, 10% opacity fill
- Selected: 4px border, 20% opacity fill, resize handles visible

### 4.4 Side Panel Components

**Regions List:**
- Scrollable listbox
- Alphabetically sorted region names
- Click to select region
- Displays all defined regions

**Groups List:**
- Scrollable listbox
- Format: "Group Name (N regions)"
- Click to view group details
- Alphabetically sorted

**Info Display:**
- Text widget showing region/group details
- Read-only
- Updates on selection changes

**Help Button:**
- "?" button
- Opens scrollable help window
- Contains instructions and keyboard shortcuts

## 5. Event Handling Architecture

### 5.1 Mouse Events

**Click Events:**
- Left Click: Start operation (create, select, drag, resize)
- Right Click: Context menu (edit, color, delete)
- Double Click: Edit region name

**Drag Events:**
- Create Mode: Draw new region rectangle
- Select Mode: Move region(s) or draw selection box

**Release Events:**
- Create Mode: Finalize new region
- Select Mode: Complete move/resize or finalize box selection

**Move Events:**
- Update cursor (crosshair, fleur, resize cursors)
- Update hover state
- Visual feedback for hovered regions

**Wheel Events:**
- Plain scroll: Vertical scrolling
- Shift+scroll: Horizontal scrolling
- Cmd+scroll: Zoom in/out

### 5.2 Keyboard Events

**Mode Switching:**
- 1: Switch to Create mode
- 2: Switch to Select mode

**Zoom:**
- + or =: Zoom in
- -: Zoom out
- 0: Reset zoom

**File Operations:**
- Cmd+O: Load PDF
- Cmd+S: Export coordinates
- Cmd+Shift+S: Load coordinates
- Cmd+M: Export mask

**Region Operations:**
- Delete/Backspace: Delete selected regions
- E: Edit selected region name
- M: Toggle mask overlay
- T: Test OCR extraction

**Selection Operations:**
- Cmd+A: Select all regions
- Cmd+D: Deselect all regions
- Cmd+I: Invert selection
- Cmd+Click: Toggle multi-select

**Group Operations:**
- Cmd+G: Group selected regions
- Cmd+Shift+G: Ungroup selected regions

**Undo/Redo:**
- Cmd+Z: Undo
- Cmd+Shift+Z or Cmd+Y: Redo

## 6. Undo/Redo System Design

### 6.1 State Snapshot Structure

Each state snapshot contains:
```python
{
    'regions': {deep copy of regions dictionary},
    'groups': {deep copy of groups dictionary}
}
```

### 6.2 Operation Triggers

State is saved before:
- Creating a new region
- Moving a region (on release)
- Resizing a region (on release)
- Deleting a region
- Renaming a region
- Changing region color
- Creating a group
- Adding regions to group
- Removing regions from group
- Deleting a group

### 6.3 Undo/Redo Behavior

**Undo:**
- Pops state from undo stack
- Pushes current state to redo stack
- Restores popped state
- Updates UI (lists, canvas, buttons)

**Redo:**
- Pops state from redo stack
- Pushes current state to undo stack
- Restores popped state
- Updates UI

**New Action:**
- Clears redo stack
- Saves current state to undo stack
- Limits undo stack to 50 items

## 7. Export/Import System Design

### 7.1 Export Format Requirements

All exports must:
- Use normalized coordinates (0.0-1.0) only
- Include aspect ratio information
- Include region metadata (name, color, group)
- Include group definitions
- Be human-readable where applicable

### 7.2 Import Format Requirements

Imports must:
- Support normalized coordinates (dict or list format)
- Reconstruct image coordinates if image is loaded
- Reconstruct groups from region data if groups section missing
- Handle missing optional fields gracefully
- Validate coordinate ranges (0.0-1.0)

### 7.3 Mask Export Requirements

Mask images must:
- Match original image dimensions exactly
- Use black background (RGB: 0,0,0)
- Use white rectangles for regions (RGB: 255,255,255)
- Support PNG, JPEG, TIFF formats
- Include coordinate JSON file alongside mask

## 8. Performance Requirements

### 8.1 Responsiveness

- Window resize: Debounced redraw (300ms delay)
- Canvas redraw: < 100ms for typical document (50 regions)
- Zoom operations: Smooth, no lag
- Region manipulation: Real-time visual feedback

### 8.2 Memory Management

- Undo stack limit: 50 states (prevents memory bloat)
- Image caching: Single page only (first page of PDF)
- Canvas elements: Cleaned up on redraw

### 8.3 Scalability

- Support documents up to 10,000x10,000 pixels
- Support up to 500 regions per document
- Handle zoom levels from 0.2x to 5.0x

## 9. Error Handling Requirements

### 9.1 File Operations

- PDF load failures: Show error dialog, maintain current state
- Export failures: Show error dialog, preserve data
- Import failures: Show error dialog, validate format

### 9.2 User Input Validation

- Region name: Required, must be unique
- Group name: Optional, can be empty
- Coordinate validation: Ensure 0.0-1.0 range
- Duplicate region names: Prevent with error message

### 9.3 State Consistency

- Coordinate synchronization: Always maintain consistency
- Group-region relationships: Validate on all operations
- Selection state: Clear invalid selections

## 10. Platform Requirements

### 10.1 Operating Systems

- macOS (primary)
- Windows (secondary)
- Linux (tertiary)

### 10.2 Dependencies

- PDF rendering library
- Image processing library
- GUI framework
- JSON/CSV/XML/YAML serialization

### 10.3 File System

- Support standard file dialogs
- Support relative and absolute paths
- Handle file permissions gracefully

