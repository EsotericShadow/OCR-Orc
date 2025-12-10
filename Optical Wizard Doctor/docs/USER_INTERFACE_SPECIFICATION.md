# User Interface Specification

## 1. Window Layout

### 1.1 Main Window

**Dimensions:**
- Default size: 1200x800 pixels
- Minimum size: 800x600 pixels
- Resizable: Yes (with debounced redraw)

**Layout Structure:**
```
┌─────────────────────────────────────────────────────────────┐
│ Toolbar (Mode, Zoom, Groups, Undo/Redo)                    │
├─────────────────────────────────────────────────────────────┤
│ Control Panel (File, Region Name, Group, Color, Buttons)   │
├──────────────────────┬──────────────────────────────────────┤
│                      │                                      │
│   Canvas Area        │   Side Panel                         │
│   (PDF Display)      │   ├─ Regions List                    │
│                      │   ├─ Groups List                     │
│                      │   ├─ Info Display                    │
│                      │   └─ Help Button (?)                 │
│                      │                                      │
└──────────────────────┴──────────────────────────────────────┘
```

### 1.2 Component Hierarchy

```
Main Window
├── Toolbar Frame
│   ├── Mode Frame (Create/Select buttons)
│   ├── Zoom Frame (Zoom controls)
│   ├── Group Frame (Group/Ungroup buttons)
│   └── Undo/Redo Frame (Undo/Redo buttons)
├── Control Panel Frame
│   ├── File Controls (Load PDF, Export, etc.)
│   ├── Region Input (Name, Group, Color)
│   └── Action Buttons (Edit, Delete, Test, etc.)
├── Main Frame
│   ├── Canvas Frame (Left)
│   │   ├── Status Label
│   │   └── Canvas Container
│   │       ├── Canvas
│   │       ├── Vertical Scrollbar
│   │       └── Horizontal Scrollbar
│   └── Side Panel Frame (Right)
│       ├── Notebook (Tabs: Regions, Groups)
│       ├── Info Text Widget
│       └── Help Button
```

## 2. Toolbar Components

### 2.1 Mode Selection

**Location:** Top-left of toolbar

**Components:**
- Label: "Mode:" (bold, black text)
- Radio Button: "Create" (8 characters wide)
- Radio Button: "Select/Move" (10 characters wide)

**Visual Design:**
- Background: Light grey (#e0e0e0)
- Selected: Light blue highlight (#90caf9)
- Text: Black
- Button style: Flat (no indicator circle)
- Active background: Light grey
- Active foreground: Black

**Behavior:**
- Clicking button switches mode
- Keyboard shortcuts: 1 (Create), 2 (Select)
- Tooltips on hover

### 2.2 Zoom Controls

**Location:** Left of toolbar, after mode selection

**Components:**
- Label: "Zoom:" (bold, black text)
- Button: "−" (zoom out, 3 characters wide)
- Label: "100%" (zoom percentage, 6 characters wide, black text)
- Button: "+" (zoom in, 3 characters wide)
- Button: "Reset" (reset zoom, 6 characters wide)

**Visual Design:**
- Background: Light grey (#e0e0e0)
- Buttons: Standard button style, black text
- Zoom label: Updates dynamically (e.g., "150%", "50%")

**Behavior:**
- Zoom range: 20% to 500%
- Zoom in: Multiply by 1.2
- Zoom out: Divide by 1.2
- Reset: Set to 100% (1.0x)
- Keyboard shortcuts: +/= (in), - (out), 0 (reset)
- Command+Scroll: Zoom in/out

### 2.3 Group Controls

**Location:** Left of toolbar, after zoom controls

**Components:**
- Button: "Group Selected" (12 characters wide)
- Button: "Ungroup Selected" (12 characters wide)

**Visual Design:**
- Background: Light grey (#e0e0e0)
- Buttons: Standard button style, black text

**Behavior:**
- Group: Creates group from selected regions in listbox
- Ungroup: Removes selected regions from their groups
- Keyboard shortcuts: Cmd+G (group), Cmd+Shift+G (ungroup)
- Requires at least 2 regions selected for grouping

### 2.4 Undo/Redo Controls

**Location:** Left of toolbar, after group controls

**Components:**
- Button: "Undo" (8 characters wide)
- Button: "Redo" (8 characters wide)

**Visual Design:**
- Background: Light grey (#e0e0e0)
- Buttons: Standard button style, black text
- Buttons disabled when stack is empty (greyed out)

**Behavior:**
- Undo: Restores previous state
- Redo: Restores undone state
- Keyboard shortcuts: Cmd+Z (undo), Cmd+Shift+Z or Cmd+Y (redo)
- Buttons enable/disable based on stack availability

## 3. Control Panel

### 3.1 File Controls

**Components:**
- Button: "Load PDF"
- Label: "No file loaded" (or filename)
- Button: "Export Coordinates"
- Button: "Export Mask"
- Button: "Load Coordinates"
- Button: "Test Extraction"

**Layout:** Horizontal, left-aligned

**Visual Design:**
- Buttons: Standard style, black text
- File label: Shows current PDF filename or "No file loaded"

**Behavior:**
- Load PDF: Opens file dialog, loads first page
- Export Coordinates: Opens save dialog with format options
- Export Mask: Opens save dialog for image formats
- Load Coordinates: Opens file dialog for JSON files
- Test Extraction: Runs OCR on all regions (if OCR available)

### 3.2 Region Input Controls

**Components:**
- Label: "Region Name:"
- Entry: Text input (20 characters wide, default: "Last Name Cell 1")
- Label: "Group:"
- Entry: Text input (15 characters wide, default: "lastname")
- Label: "Color:"
- Combobox: Color dropdown (10 characters wide, values: blue, red, green, yellow, purple, orange, cyan)

**Layout:** Horizontal, left-aligned

**Visual Design:**
- Labels: Standard label style
- Entries: Standard text entry style
- Combobox: Dropdown style

**Behavior:**
- Region name: Required for creating regions, auto-increments after creation
- Group: Optional, can be empty
- Color: Defaults to blue, affects region border color

### 3.3 Action Controls

**Components:**
- Checkbox: "Mask Outside" (toggles mask overlay)
- Button: "Clear Selection"
- Button: "Edit Selected"
- (Other buttons from file controls)

**Layout:** Horizontal, left-aligned

**Visual Design:**
- Checkbox: Standard checkbox style, black text
- Buttons: Standard button style, black text

**Behavior:**
- Mask Outside: Toggles semi-transparent grey overlay
- Clear Selection: Clears current region selection
- Edit Selected: Opens edit dialog for selected region

## 4. Canvas Area

### 4.1 Canvas Container

**Layout:** Left side of main frame, expandable

**Components:**
- Status Label (top)
- Canvas Container Frame
  - Canvas (scrollable)
  - Vertical Scrollbar (right)
  - Horizontal Scrollbar (bottom)

**Visual Design:**
- Background: Light grey (#d0d0d0)
- Status label: Black text, Arial 10pt
- Canvas: Light grey background (#d0d0d0)
- Scrollbars: Standard scrollbar style

### 4.2 Canvas Visual Elements

**Background:**
- Color: Light grey (#d0d0d0)
- Purpose: Distinguishes from white document

**Document Image:**
- Position: Centered on canvas
- Shadow: 5px offset, grey (#808080)
- Scaling: Maintains aspect ratio, fits to canvas
- Quality: High-quality resampling (LANCZOS)

**Region Rectangles:**
- Normal: 2px border, no fill, colored outline
- Hovered: 3px border, 10% opacity fill, colored outline
- Selected: 4px border, 20% opacity fill, colored outline, resize handles

**Region Labels:**
- Position: Above region (centered horizontally)
- Normal: 8pt font, normal weight, colored text
- Hovered/Selected: 10pt font, bold weight, colored text
- Anchor: Bottom (text above region)

**Resize Handles:**
- Count: 8 handles (4 corners, 4 edges)
- Size: 8x8 pixels
- Appearance: White fill, black outline, 1px border
- Visibility: Only for single selected region

**Selection Box:**
- Appearance: Dashed blue outline (#0066ff), 2px width
- Dash pattern: 5px dash, 5px gap
- Visibility: During drag selection in Select mode

**Mask Overlay:**
- Appearance: Semi-transparent grey (128, 128, 128, 200 alpha)
- Coverage: Entire canvas except regions
- Regions: Appear as transparent "windows"
- Toggle: Checkbox or M key

### 4.3 Canvas Interactions

**Mouse Interactions:**
- Click: Start operation (create, select, drag, resize)
- Drag: Draw rectangle, move region, resize region, draw selection box
- Release: Finalize operation
- Move: Update cursor, hover feedback
- Wheel: Scroll or zoom (with modifiers)
- Double-click: Edit region name
- Right-click: Context menu

**Keyboard Interactions:**
- All keyboard shortcuts work when canvas has focus
- Mode switching, zoom, selection, etc.

## 5. Side Panel

### 5.1 Tabbed Interface

**Tabs:**
- "Regions" tab
- "Groups" tab

**Visual Design:**
- Standard tabbed notebook style
- Tabs at top
- Content area below

### 5.2 Regions Tab

**Components:**
- Scrollable Listbox (shows all region names)
- Vertical Scrollbar

**Layout:**
- Listbox fills available space
- Scrollbar on right side

**Visual Design:**
- Listbox: Standard listbox style
- Scrollbar: Standard scrollbar style
- Selection: Highlighted when clicked

**Behavior:**
- Click region name to select
- Selected region highlighted on canvas
- Region info displayed in info panel
- Alphabetically sorted

### 5.3 Groups Tab

**Components:**
- Scrollable Listbox (shows groups with counts)
- Vertical Scrollbar
- Buttons:
  - "Create Group"
  - "Add to Group"
  - "Remove from Group"
  - "Delete Group" (red background)

**Layout:**
- Buttons at top (horizontal)
- Listbox below (fills remaining space)
- Scrollbar on right side

**Visual Design:**
- Listbox: Standard listbox style
- Format: "Group Name (N regions)"
- Delete button: Red background (#ff6b6b)
- Other buttons: Standard style, black text

**Behavior:**
- Click group name to view members
- Group info displayed in info panel
- Alphabetically sorted

### 5.4 Info Display

**Location:** Below tabbed interface

**Components:**
- Text Widget (read-only, word wrap)

**Layout:**
- Fills available vertical space
- Scrollable if content exceeds space

**Visual Design:**
- Text widget: Standard text widget style
- Read-only: Cannot be edited
- Font: Arial, default size

**Content:**
- Shows region details when region selected
- Shows group details when group selected
- Includes coordinates, size, color, group info

### 5.5 Help Button

**Location:** Bottom of side panel

**Components:**
- Button: "?" (3 characters wide, 1 character tall)

**Visual Design:**
- Button: Raised style, bold font (Arial 12pt)
- Background: Light grey (#e0e0e0)
- Text: Black, bold "?"

**Behavior:**
- Click opens help window
- Tooltip: "Show help and instructions"
- Help window: Scrollable, 600x700 pixels, resizable

## 6. Help Window

### 6.1 Window Specifications

**Dimensions:**
- Default: 600x700 pixels
- Resizable: Yes

**Components:**
- Scrollable Text Widget
- Vertical Scrollbar
- Close Button

**Layout:**
- Text widget: Fills most of window
- Scrollbar: Right side
- Close button: Bottom, centered

### 6.2 Content

**Sections:**
1. Basic Workflow
2. Selection Mode
3. Keyboard Shortcuts
4. Export Formats
5. Mask Export
6. Scroll Behavior

**Formatting:**
- Headers: Bold, underlined
- Sections: Separated by blank lines
- Lists: Bullet points
- Code/Keys: Monospace font

## 7. Dialog Windows

### 7.1 Edit Region Name Dialog

**Dimensions:** 300x100 pixels

**Components:**
- Label: "New Name:"
- Entry: Text input (30 characters wide)
- Button: "Save"

**Layout:**
- Label: Top, centered
- Entry: Middle, centered
- Button: Bottom, centered

**Behavior:**
- Entry pre-filled with current name
- Text selected (ready to type)
- Enter key saves
- Validates uniqueness
- Updates region and groups

### 7.2 Change Color Dialog

**Dimensions:** 250x150 pixels

**Components:**
- Label: "Select Color:"
- Combobox: Color dropdown (15 characters wide)
- Button: "Save"

**Layout:**
- Label: Top, centered
- Combobox: Middle, centered
- Button: Bottom, centered

**Behavior:**
- Combobox pre-filled with current color
- Available colors: blue, red, green, yellow, purple, orange, cyan
- Updates region color on save

### 7.3 Context Menu

**Location:** At mouse cursor position

**Components:**
- Menu Item: "Edit Name"
- Menu Item: "Change Color"
- Menu Item: "Delete"

**Visual Design:**
- Standard context menu style
- Appears on right-click over region

**Behavior:**
- Edit Name: Opens edit dialog
- Change Color: Opens color dialog
- Delete: Deletes region (with undo)

## 8. Visual Feedback

### 8.1 Cursor Changes

**Cursors:**
- Default: Crosshair (Create mode, empty canvas)
- Over region: Fleur (move cursor, Select mode)
- Over resize handle: Resize cursors (8 types)
  - Top-left corner, top-right corner, etc.
  - Top side, bottom side, left side, right side

### 8.2 Tooltips

**Appearance:**
- Yellow background (#ffffe0)
- Black text
- Black border
- Small font (Arial 9pt)
- Padding: 5px horizontal, 2px vertical

**Behavior:**
- Appears on hover
- Follows mouse cursor (offset 10px)
- Disappears when mouse leaves

**Tooltips on:**
- All buttons
- All controls
- Mode buttons
- Zoom controls
- Group controls
- Undo/Redo buttons
- Help button

### 8.3 Status Messages

**Status Label:**
- Location: Above canvas
- Text: Updates based on mode and operation
- Examples:
  - "Mode: Create - Click and drag to create new regions"
  - "Mode: Select/Move - Click to select, Cmd+Click to multi-select, Shift+Drag for box select"
  - "Click and drag to create regions"

## 9. Color Scheme

### 9.1 Background Colors

- **Toolbar:** Light grey (#e0e0e0)
- **Canvas:** Light grey (#d0d0d0)
- **Side Panel:** Default (white/light)
- **Control Panel:** Default (white/light)

### 9.2 Text Colors

- **All text:** Black (#000000)
- **Labels:** Black
- **Buttons:** Black text
- **Tooltips:** Black text
- **Status:** Black text

### 9.3 Region Colors

- **Blue:** #0066ff
- **Red:** #ff0000
- **Green:** #00cc00
- **Yellow:** #ffcc00
- **Purple:** #9900cc
- **Orange:** #ff6600
- **Cyan:** #00cccc

### 9.4 Selection Colors

- **Selection box:** Blue (#0066ff), dashed
- **Selected region fill:** 20% opacity of region color
- **Hovered region fill:** 10% opacity of region color

## 10. Responsive Design

### 10.1 Window Resizing

**Behavior:**
- Window is resizable
- Minimum size enforced (800x600)
- Components scale proportionally
- Canvas redraws with debounce (300ms)
- Regions maintain relative positions

### 10.2 Component Scaling

**Fixed Size:**
- Buttons maintain minimum width
- Labels maintain text size
- Toolbar height fixed

**Flexible Size:**
- Canvas expands/contracts with window
- Side panel expands/contracts with window
- Lists scroll when content exceeds space

### 10.3 Layout Adaptation

**Horizontal:**
- Canvas and side panel share horizontal space
- Side panel has minimum width
- Canvas gets remaining space

**Vertical:**
- Toolbar fixed height
- Control panel fixed height
- Canvas and side panel share remaining vertical space

