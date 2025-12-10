# Function Reference

## Overview

This document provides detailed specifications for all functions in the OCR-Orc application. Functions are organized by category.

## 1. Initialization and Setup

### `__init__(root)`
**Purpose:** Initialize the application and set up all state variables.

**Parameters:**
- `root`: Tkinter root window

**State Initialization:**
- Document state (PDF path, image, scale factors)
- Region storage (empty dictionary)
- Group storage (empty dictionary)
- Selection state (None, empty set)
- Operation state (create mode, no active operations)
- Zoom state (1.0x, no offset)
- Undo/Redo stacks (empty, max 50)

**Calls:**
- `setup_ui()`: Creates all UI components

**Requirements:**
- Window title: "PDF Coordinate Calibrator"
- Window size: 1200x800
- Minimum size: 800x600
- Resizable: Yes

### `setup_ui()`
**Purpose:** Create and arrange all UI components.

**Components Created:**
- Toolbar (mode, zoom, groups, undo/redo)
- Control panel (file, region input, actions)
- Main frame (canvas and side panel)
- Side panel (regions list, groups list, info, help)

**Layout:**
- Toolbar: Top, full width
- Control panel: Below toolbar, full width
- Main frame: Below control panel, fills remaining space
- Canvas: Left side of main frame, expandable
- Side panel: Right side of main frame, fixed width

**Calls:**
- `setup_hotkeys()`: Registers keyboard shortcuts
- `create_tooltip()`: Adds tooltips to buttons

## 2. UI Helper Functions

### `create_tooltip(widget, text)`
**Purpose:** Create hover tooltips for UI elements.

**Parameters:**
- `widget`: Tkinter widget to attach tooltip to
- `text`: Tooltip text to display

**Behavior:**
- Shows tooltip on mouse enter
- Hides tooltip on mouse leave
- Tooltip follows mouse cursor (10px offset)
- Yellow background, black text, black border

**Visual:**
- Background: #ffffe0
- Text: Black, Arial 9pt
- Border: 1px solid black
- Padding: 5px horizontal, 2px vertical

### `show_help()`
**Purpose:** Display help window with instructions and shortcuts.

**Window:**
- Size: 600x700 pixels
- Resizable: Yes
- Scrollable: Yes

**Content:**
- Basic workflow
- Selection mode instructions
- Keyboard shortcuts
- Export formats
- Mask export info
- Scroll behavior

**Format:**
- Scrollable text widget
- Read-only
- Formatted with headers and sections

## 3. Mode Management

### `set_mode_create()`
**Purpose:** Switch to Create mode.

**Actions:**
- Set `mouse_mode` to "create"
- Set cursor to "crosshair"
- Update status label
- Clear selection
- Redraw regions

**Status Message:**
"Mode: Create - Click and drag to create new regions"

### `set_mode_select()`
**Purpose:** Switch to Select/Move mode.

**Actions:**
- Set `mouse_mode` to "select"
- Set cursor to "arrow"
- Update status label

**Status Message:**
"Mode: Select/Move - Click to select, Cmd+Click to multi-select, Shift+Drag for box select"

## 4. Zoom Functions

### `zoom_in()`
**Purpose:** Increase zoom level.

**Behavior:**
- Multiply zoom_level by 1.2
- Clamp to maximum 5.0x (500%)
- Update zoom display
- Redraw image and regions

**Keyboard:** + or = key

### `zoom_out()`
**Purpose:** Decrease zoom level.

**Behavior:**
- Divide zoom_level by 1.2
- Clamp to minimum 0.2x (20%)
- Update zoom display
- Redraw image and regions

**Keyboard:** - key

### `reset_zoom()`
**Purpose:** Reset zoom to fit canvas.

**Behavior:**
- Set zoom_level to 1.0 (100%)
- Reset pan offsets to 0
- Update zoom display
- Redraw image and regions

**Keyboard:** 0 key

### `update_zoom_display()`
**Purpose:** Update zoom percentage label.

**Behavior:**
- Calculate percentage: `zoom_level * 100`
- Update label text (e.g., "150%")
- Format as integer percentage

## 5. Mouse Event Handlers

### `on_click(event)`
**Purpose:** Handle mouse click to start operations.

**Parameters:**
- `event`: Mouse click event

**Behavior by Mode:**

**Select Mode:**
1. Check for resize handle click → Start resize
2. Check for region click → Start drag
3. Click empty space → Start selection box

**Create Mode:**
1. Start new region creation
2. Store start coordinates
3. Clear any temporary rectangle

**Actions:**
- Convert event coordinates to canvas coordinates
- Clear hover state
- Set operation flags
- Store start position

### `on_drag(event)`
**Purpose:** Handle mouse drag to update operations.

**Parameters:**
- `event`: Mouse drag event

**Behavior by Mode:**

**Select Mode:**
- If resizing: Update region size
- If dragging: Move selected regions
- If selection box: Update selection box rectangle

**Create Mode:**
- Draw/update temporary rectangle

**Actions:**
- Convert event coordinates to canvas coordinates
- Update region positions/sizes in real-time
- Redraw affected elements

### `on_release(event)`
**Purpose:** Finalize operations on mouse release.

**Parameters:**
- `event`: Mouse release event

**Behavior:**
- Finalize selection box (if active)
- Finalize move/resize (save state to undo)
- Finalize region creation (save state to undo)
- Reset operation flags

**State Saving:**
- Saves state to undo stack before finalizing
- Clears redo stack
- Updates UI

### `on_mouse_move(event)`
**Purpose:** Update cursor and hover feedback.

**Parameters:**
- `event`: Mouse move event

**Behavior:**
- Check for resize handle → Set resize cursor
- Check for region hover → Set move cursor, highlight region
- Update hover state
- Redraw if hover state changed

**Cursors:**
- Resize handles: 8 different resize cursors
- Over region: Fleur (move cursor)
- Default: Crosshair

### `on_mousewheel(event)`
**Purpose:** Handle vertical scrolling.

**Parameters:**
- `event`: Mouse wheel event

**Behavior:**
- Scroll canvas vertically
- Up: Scroll up
- Down: Scroll down

**Modifiers:** None (plain scroll)

### `on_mousewheel_shift(event)`
**Purpose:** Handle horizontal scrolling.

**Parameters:**
- `event`: Shift+Mouse wheel event

**Behavior:**
- Scroll canvas horizontally
- Up/Left: Scroll left
- Down/Right: Scroll right

**Modifiers:** Shift key

### `on_mousewheel_cmd(event)`
**Purpose:** Handle zoom with mouse wheel.

**Parameters:**
- `event`: Command+Mouse wheel event

**Behavior:**
- Zoom in/out
- Works for both horizontal and vertical scroll
- Up: Zoom in
- Down: Zoom out

**Modifiers:** Command key

### `on_cmd_click(event)`
**Purpose:** Handle Command+click for multi-select.

**Parameters:**
- `event`: Command+Click event

**Behavior:**
- Toggle region in multi-selection
- If already selected: Deselect
- If not selected: Add to selection
- Update primary selection
- Redraw regions

**Modifiers:** Command key

### `on_shift_click(event)`
**Purpose:** Start box selection with Shift+click.

**Parameters:**
- `event`: Shift+Click event

**Behavior:**
- Store selection box start position
- Initialize box selection state

**Modifiers:** Shift key

### `on_shift_drag(event)`
**Purpose:** Update selection box during Shift+drag.

**Parameters:**
- `event`: Shift+Drag event

**Behavior:**
- Delete old selection box
- Draw new selection box rectangle
- Dashed blue outline

**Modifiers:** Shift key

### `on_shift_release(event)`
**Purpose:** Finalize box selection.

**Parameters:**
- `event`: Shift+Release event

**Behavior:**
- Find all regions intersecting selection box
- Add to existing selection
- Clean up selection box
- Redraw regions

**Modifiers:** Shift key

### `on_double_click(event)`
**Purpose:** Edit region name on double-click.

**Parameters:**
- `event`: Double-click event

**Behavior:**
- Find region at click position
- Open edit name dialog
- Pre-fill with current name

### `on_right_click(event)`
**Purpose:** Show context menu on right-click.

**Parameters:**
- `event`: Right-click event

**Behavior:**
- Find region at click position
- Select region
- Show context menu (Edit, Color, Delete)
- Menu appears at cursor position

### `on_canvas_leave(event)`
**Purpose:** Clear hover state when mouse leaves canvas.

**Parameters:**
- `event`: Leave event

**Behavior:**
- Clear hovered_region
- Redraw regions (remove hover highlight)

## 6. Region Management Functions

### `get_region_at(x, y)`
**Purpose:** Find region at given canvas coordinates.

**Parameters:**
- `x`: Canvas X coordinate
- `y`: Canvas Y coordinate

**Returns:**
- Region name (string) or None

**Behavior:**
- Check canvas items at coordinates
- Look for region tags
- Return first matching region name

**Tolerance:** 2 pixels

### `get_region_for_handle(x, y)`
**Purpose:** Get region that owns resize handle.

**Parameters:**
- `x`: Canvas X coordinate
- `y`: Canvas Y coordinate

**Returns:**
- Region name (string) or None

**Behavior:**
- Returns currently selected region
- Used during resize operations

### `get_handle_position(x, y)`
**Purpose:** Determine which resize handle is at coordinates.

**Parameters:**
- `x`: Canvas X coordinate
- `y`: Canvas Y coordinate

**Returns:**
- Handle identifier ('nw', 'ne', 'sw', 'se', 'n', 's', 'w', 'e') or None

**Handle Positions:**
- Corners: nw, ne, sw, se
- Edges: n, s, w, e

**Tolerance:** 13 pixels (8px handle + 5px)

### `move_region(region_name, dx, dy)`
**Purpose:** Move a region by delta pixels.

**Parameters:**
- `region_name`: Name of region to move
- `dx`: X delta in canvas pixels
- `dy`: Y delta in canvas pixels

**Behavior:**
- Update canvas coordinates
- Convert delta to image coordinates
- Update image coordinates
- Convert delta to normalized coordinates
- Update normalized coordinates
- Redraw regions

**Coordinate Updates:**
- All three coordinate systems updated
- Maintains synchronization

### `resize_region(region_name, handle, new_x, new_y)`
**Purpose:** Resize a region using specified handle.

**Parameters:**
- `region_name`: Name of region to resize
- `handle`: Handle identifier ('nw', 'ne', etc.)
- `new_x`: New X coordinate for handle
- `new_y`: New Y coordinate for handle

**Behavior:**
- Update canvas coordinates based on handle
- Convert to image coordinates
- Convert to normalized coordinates
- Update all three coordinate systems
- Redraw regions

**Handle Behavior:**
- Corner handles: Update two coordinates
- Edge handles: Update one coordinate

### `auto_increment_name(current_name)`
**Purpose:** Auto-increment region name after creation.

**Parameters:**
- `current_name`: Name of just-created region

**Behavior:**
- Extract trailing number and increment
- Or add " 2" if name contains "Cell"
- Update region name entry field

**Patterns:**
- "Cell 1" → "Cell 2"
- "Last Name 5" → "Last Name 6"
- "Cell" → "Cell 2"

### `update_region_canvas_coords()`
**Purpose:** Recalculate canvas coordinates for all regions.

**Behavior:**
- For each region:
  - Get normalized coordinates
  - Calculate canvas coordinates from normalized
  - Update canvas_coords in region data

**Called:**
- On zoom change
- On pan change
- On image load
- Before redraw

### `redraw_regions()`
**Purpose:** Redraw all regions on canvas.

**Behavior:**
1. Update canvas coordinates from normalized
2. Clear existing region rectangles
3. Draw mask overlay (if enabled)
4. For each region:
   - Determine visual state (normal/hovered/selected)
   - Draw rectangle with appropriate style
   - Draw resize handles (if selected)
   - Draw label

**Visual States:**
- Normal: 2px border, no fill
- Hovered: 3px border, 10% fill
- Selected: 4px border, 20% fill, handles

### `hex_to_rgba(hex_color, alpha)`
**Purpose:** Convert hex color to lighter shade for fill.

**Parameters:**
- `hex_color`: Hex color string (e.g., "#0066ff")
- `alpha`: Opacity (0.0-1.0)

**Returns:**
- Lighter hex color string

**Behavior:**
- Lightens color based on alpha
- Used for semi-transparent fills
- Workaround for Tkinter's lack of RGBA support

### `draw_resize_handles(x1, y1, x2, y2)`
**Purpose:** Draw resize handles on selected region.

**Parameters:**
- `x1, y1, x2, y2`: Region canvas coordinates

**Returns:**
- List of canvas item IDs

**Behavior:**
- Draw 8 handles (4 corners, 4 edges)
- Handle size: 8x8 pixels
- White fill, black outline
- Return handle IDs for cleanup

### `draw_mask_overlay()`
**Purpose:** Draw semi-transparent mask over canvas.

**Behavior:**
- Create RGBA image (canvas size)
- Fill with grey (128, 128, 128, 200)
- Draw white rectangles for regions (transparent holes)
- Display as canvas image

**Visual:**
- Grey overlay everywhere
- Transparent regions (white rectangles)
- Helps visualize OCR extraction areas

## 7. Selection Functions

### `select_all()`
**Purpose:** Select all regions.

**Behavior:**
- Add all region names to selected_regions set
- Set primary selection to first region
- Redraw regions

**Keyboard:** Cmd+A

### `deselect_all()`
**Purpose:** Clear all selections.

**Behavior:**
- Clear selected_regions set
- Clear selected_region
- Redraw regions

**Keyboard:** Cmd+D

### `invert_selection()`
**Purpose:** Invert current selection.

**Behavior:**
- Calculate: unselected = all_regions - selected_regions
- Set selected_regions to unselected
- Update primary selection
- Redraw regions

**Keyboard:** Cmd+I

### `delete_selected_region()`
**Purpose:** Delete all selected regions.

**Behavior:**
- Delete all regions in selected_regions
- Clear selection
- Update lists
- Redraw regions

**Keyboard:** Delete or Backspace

## 8. File Operations

### `load_pdf()`
**Purpose:** Load and display PDF file.

**Behavior:**
1. Show file dialog (PDF files)
2. Convert PDF first page to image (150 DPI)
3. Store PDF path
4. Store image
5. Clear existing regions
6. Display image
7. Update file label

**Error Handling:**
- Show error dialog on failure
- Maintain current state

**Keyboard:** Cmd+O

### `display_image()`
**Purpose:** Display PDF image on canvas with proper scaling.

**Behavior:**
1. Get canvas dimensions
2. Calculate base scale (fit to canvas)
3. Apply zoom level
4. Resize image with high-quality resampling
5. Calculate centered position
6. Draw shadow (5px offset)
7. Draw image
8. Update scroll region
9. Redraw regions

**Visual:**
- Shadow: 5px offset, grey
- Image: Centered, scaled
- Background: Light grey

### `on_canvas_configure(event)`
**Purpose:** Handle canvas resize with debouncing.

**Parameters:**
- `event`: Configure event

**Behavior:**
- Cancel any pending resize
- Schedule delayed redraw (300ms)
- Prevents excessive redraws during window resize

**Debounce:**
- 300ms delay
- Cancels previous timer if new resize occurs

### `_delayed_resize()`
**Purpose:** Perform actual resize after debounce delay.

**Behavior:**
- Clear resize timer
- Call display_image() if image exists
- Redraws canvas with new dimensions

## 9. Region List Management

### `update_region_list()`
**Purpose:** Update the regions listbox.

**Behavior:**
- Clear listbox
- Add all region names (alphabetically sorted)
- Updates when regions are created/deleted/renamed

### `on_region_select(event)`
**Purpose:** Handle region selection in listbox.

**Parameters:**
- `event`: Listbox selection event

**Behavior:**
- Get selected region name
- Set as primary selection
- Update info display with region details
- Redraw regions

**Info Displayed:**
- Region name
- Image coordinates
- Normalized coordinates
- Color
- Group
- Size (pixels and normalized)

### `delete_region()`
**Purpose:** Delete region selected in listbox.

**Behavior:**
- Get selected region from listbox
- Delete region
- Update lists
- Clear info display
- Redraw regions

## 10. Region Editing Functions

### `edit_region_name_dialog(region_name)`
**Purpose:** Open dialog to edit region name.

**Parameters:**
- `region_name`: Current region name

**Dialog:**
- Size: 300x100
- Entry field pre-filled with current name
- Save button
- Enter key saves

**Validation:**
- Name must be unique
- Name cannot be empty
- Updates region and groups
- Saves state to undo stack

### `edit_region_name()`
**Purpose:** Edit name of region selected in listbox.

**Behavior:**
- Get selected region from listbox
- Call edit_region_name_dialog()
- Shows warning if no region selected

**Keyboard:** E key

### `edit_selected_region()`
**Purpose:** Edit currently selected region.

**Behavior:**
- Check if region is selected
- Call edit_region_name_dialog()
- Shows warning if no selection

### `change_region_color_dialog(region_name)`
**Purpose:** Open dialog to change region color.

**Parameters:**
- `region_name`: Region to change color for

**Dialog:**
- Size: 250x150
- Color dropdown (7 colors)
- Pre-filled with current color
- Save button

**Behavior:**
- Updates region color
- Saves state to undo stack
- Redraws regions

### `change_region_color()`
**Purpose:** Change color of region selected in listbox.

**Behavior:**
- Get selected region from listbox
- Call change_region_color_dialog()
- Shows warning if no region selected

### `delete_region_by_name(region_name)`
**Purpose:** Delete a specific region.

**Parameters:**
- `region_name`: Name of region to delete

**Behavior:**
- Remove from groups
- Delete empty groups
- Delete region
- Clear selection if deleted
- Update lists
- Clear info display
- Saves state to undo stack before deletion

## 11. Group Management Functions

### `create_group()`
**Purpose:** Create group from selected regions in listbox.

**Behavior:**
1. Check at least 2 regions selected
2. Prompt for group name
3. Check if group exists (ask to add)
4. Save state to undo stack
5. Add regions to group
6. Update region group assignments
7. Update group list
8. Show success message

**Keyboard:** Cmd+G

### `add_to_group()`
**Purpose:** Add selected regions to existing group.

**Behavior:**
1. Check regions selected
2. Prompt for group name
3. Create group if doesn't exist
4. Save state to undo stack
5. Add regions to group
6. Update region group assignments
7. Update group list

### `remove_from_group()`
**Purpose:** Remove selected regions from their groups.

**Behavior:**
1. Check regions selected
2. Save state to undo stack
3. For each region:
   - Remove from group
   - Set region['group'] = None
   - Delete group if empty
4. Update group list

### `delete_group()`
**Purpose:** Delete selected group.

**Behavior:**
1. Get selected group from listbox
2. Save state to undo stack
3. For each region in group:
   - Set region['group'] = None
4. Delete group
5. Update group list
6. Clear info display

### `update_group_list()`
**Purpose:** Update the groups listbox.

**Behavior:**
- Clear listbox
- For each group:
  - Format: "Group Name (N regions)"
  - Add to listbox
- Alphabetically sorted

### `on_group_select(event)`
**Purpose:** Handle group selection in listbox.

**Parameters:**
- `event`: Listbox selection event

**Behavior:**
- Get selected group name
- Display group info (name, member count, member list)
- Update info display

### `quick_group()`
**Purpose:** Quick group operation (from listbox selection).

**Behavior:**
- Same as create_group() but uses listbox selection
- Shows warning if < 2 regions selected

### `quick_ungroup()`
**Purpose:** Quick ungroup operation (from listbox selection).

**Behavior:**
- Same as remove_from_group() but uses listbox selection
- Shows success message with count

## 12. Export Functions

### `export_coordinates()`
**Purpose:** Export coordinates in multiple formats.

**Behavior:**
1. Check regions exist
2. Show save dialog with format options
3. Determine format from extension
4. Call appropriate export function
5. Show success message

**Formats:**
- .json → _export_json()
- .csv → _export_csv()
- .txt → _export_txt()
- .xml → _export_xml()
- .yaml/.yml → _export_yaml()
- Default → _export_json()

**Keyboard:** Cmd+S

### `_export_json(file_path)`
**Purpose:** Export coordinates as JSON.

**Parameters:**
- `file_path`: Output file path

**Format:**
- Structured JSON with regions and groups
- Normalized coordinates only
- Includes metadata (PDF path, image size, aspect ratio)
- Pretty-printed with 2-space indent

### `_export_csv(file_path)`
**Purpose:** Export coordinates as CSV.

**Parameters:**
- `file_path`: Output file path

**Format:**
- Header row with column names
- One row per region
- Coordinates as percentages (6 decimal places)
- Alphabetically sorted

### `_export_txt(file_path)`
**Purpose:** Export coordinates as plain text.

**Parameters:**
- `file_path`: Output file path

**Format:**
- Human-readable format
- Section headers
- Coordinates in both decimal and percentage
- Group listings

### `_export_xml(file_path)`
**Purpose:** Export coordinates as XML.

**Parameters:**
- `file_path`: Output file path

**Format:**
- XML structure with regions and groups
- Attributes for metadata
- Pretty-printed with 2-space indent
- XML declaration included

### `_export_yaml(file_path)`
**Purpose:** Export coordinates as YAML.

**Parameters:**
- `file_path`: Output file path

**Format:**
- YAML block style
- 2-space indent
- No flow style
- Preserves insertion order

**Dependencies:** Requires PyYAML library

### `export_mask(file_path)`
**Purpose:** Export mask image for OCR preprocessing.

**Behavior:**
1. Check image and regions exist
2. Show save dialog (PNG, JPEG, TIFF)
3. Create black image (same size as original)
4. Draw white rectangles for each region
5. Save mask image
6. Save coordinate JSON file alongside
7. Show success message with both paths

**Mask Format:**
- Black background (RGB: 0,0,0)
- White rectangles for regions (RGB: 255,255,255)
- Same dimensions as original image

**Keyboard:** Cmd+M

## 13. Import Functions

### `load_coordinates(file_path)`
**Purpose:** Load coordinates from JSON file.

**Behavior:**
1. Show file dialog (JSON files)
2. Parse JSON file
3. For each region:
   - Handle both dict and list coordinate formats
   - Calculate image coordinates if image loaded
   - Calculate canvas coordinates
   - Store region data
4. Load or reconstruct groups
5. Update lists
6. Redraw regions
7. Show success message

**Format Support:**
- Dict format: `{'x1': 0.25, ...}`
- List format: `[0.25, 0.10, 0.35, 0.20]`

**Keyboard:** Cmd+Shift+S

## 14. Undo/Redo Functions

### `save_state()`
**Purpose:** Save current state to undo stack.

**Behavior:**
- Create deep copy of regions and groups
- Add to undo stack
- Limit stack to max_undo_history (50)
- Clear redo stack
- Update undo/redo button states

**Called Before:**
- Creating region
- Moving region (on release)
- Resizing region (on release)
- Deleting region
- Renaming region
- Changing color
- Group operations

### `restore_state(state)`
**Purpose:** Restore application state from snapshot.

**Parameters:**
- `state`: State snapshot (regions and groups)

**Behavior:**
- Deep copy state data
- Restore regions dictionary
- Restore groups dictionary
- Recalculate canvas coordinates
- Update lists
- Redraw regions
- Clear selection

### `undo_action()`
**Purpose:** Undo last operation.

**Behavior:**
- Pop state from undo stack
- Save current state to redo stack
- Restore popped state
- Update button states

**Keyboard:** Cmd+Z

### `redo_action()`
**Purpose:** Redo last undone operation.

**Behavior:**
- Pop state from redo stack
- Save current state to undo stack
- Restore popped state
- Update button states

**Keyboard:** Cmd+Shift+Z or Cmd+Y

### `update_undo_redo_buttons()`
**Purpose:** Update undo/redo button enabled states.

**Behavior:**
- Enable undo if undo_stack has items
- Enable redo if redo_stack has items
- Disable if stack is empty

## 15. Utility Functions

### `clear_selection()`
**Purpose:** Clear current region creation selection.

**Behavior:**
- Delete temporary rectangle
- Clear start coordinates
- Used to cancel region creation

### `toggle_mask()`
**Purpose:** Toggle mask overlay on/off.

**Behavior:**
- Get mask state from checkbox
- Set mask_enabled flag
- Redraw regions (mask drawn if enabled)

**Keyboard:** M key

### `toggle_mask_keyboard()`
**Purpose:** Toggle mask via keyboard.

**Behavior:**
- Toggle checkbox state
- Call toggle_mask()

### `setup_hotkeys()`
**Purpose:** Register all keyboard shortcuts.

**Shortcuts Registered:**
- Mode: 1, 2
- Zoom: +, =, -, 0
- File: Cmd+O, Cmd+S, Cmd+Shift+S, Cmd+M
- Region: Delete, Backspace, E
- Group: Cmd+G, Cmd+Shift+G
- Mask: M
- Test: T
- Selection: Cmd+A, Cmd+D, Cmd+I
- Undo/Redo: Cmd+Z, Cmd+Shift+Z, Cmd+Y

## 16. Test Functions

### `test_extraction()`
**Purpose:** Test OCR extraction on all regions.

**Behavior:**
1. Check PDF and regions exist
2. Initialize OCR reader (EasyOCR)
3. Convert PDF to image
4. For each region:
   - Crop region from image
   - Run OCR
   - Extract text and confidence
5. Display results in new window

**Results Window:**
- Size: 600x400
- Scrollable text widget
- Shows region name, extracted text, average confidence

**Dependencies:** Requires EasyOCR library

**Keyboard:** T key

## 17. Main Function

### `main()`
**Purpose:** Application entry point.

**Behavior:**
- Create Tkinter root window
- Create CoordinateCalibrator instance
- Start main event loop

**Entry Point:** Called when script is run directly

