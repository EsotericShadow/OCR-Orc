# Functional Requirements

## 1. Document Loading and Display

### 1.1 PDF Loading
**Requirement FR-001**: The application MUST support loading PDF files.

**Details:**
- User can select a PDF file via file dialog
- Only the first page of the PDF is loaded and displayed
- PDF is converted to an image at 150 DPI resolution
- If PDF loading fails, show error message and maintain current state
- File path is stored for reference in exports
- Loading a new PDF clears all existing regions

**User Interface:**
- "Load PDF" button in control panel
- Keyboard shortcut: Cmd+O
- File dialog filters: PDF files (*.pdf), All files (*.*)

### 1.2 Image Display
**Requirement FR-002**: The application MUST display the PDF image on a scrollable canvas.

**Details:**
- Image is centered on canvas
- Image scales to fit canvas while maintaining aspect ratio
- Canvas has light grey background (#d0d0d0)
- Image has shadow effect (5px offset, grey color)
- Canvas supports horizontal and vertical scrolling
- Image position updates when canvas is resized (debounced)

**Visual Requirements:**
- Shadow: 5 pixels offset, grey color (#808080)
- Background: Light grey (#d0d0d0)
- Image quality: High-quality resampling (LANCZOS)

## 2. Region Creation and Management

### 2.1 Create Mode
**Requirement FR-003**: The application MUST support creating new regions by drawing rectangles.

**Details:**
- User switches to "Create" mode via button or keyboard (1)
- In Create mode, clicking and dragging draws a rectangle
- Rectangle is displayed in red outline during creation
- On release, region is created with name from input field
- Region name is required (show warning if empty)
- Region name must be unique (show error if duplicate)
- After creation, region name auto-increments if pattern detected

**Region Creation Process:**
1. User enters region name in input field
2. User selects color from dropdown (optional, defaults to blue)
3. User enters group name (optional)
4. User clicks and drags on canvas to define region
5. On mouse release, region is created
6. Region appears with colored border and label
7. Region name field auto-increments (e.g., "Cell 1" → "Cell 2")

**Auto-increment Rules:**
- If name ends with number, increment that number
- If name contains "Cell" or "cell", add " 2" if no number exists
- Otherwise, no auto-increment

### 2.2 Region Data Storage
**Requirement FR-004**: Each region MUST store three coordinate systems.

**Coordinate Systems:**
1. **Normalized Coordinates** (0.0-1.0): Source of truth, resolution-independent
2. **Image Coordinates** (pixels): Pixel positions in original image
3. **Canvas Coordinates** (display pixels): Screen positions for rendering

**Region Data Structure:**
- `image_coords`: [x1, y1, x2, y2] - Integer pixel coordinates
- `normalized_coords`: [x1, y1, x2, y2] - Float 0.0-1.0 coordinates
- `canvas_coords`: [x1, y1, x2, y2] - Float display coordinates
- `color`: String identifier (blue, red, green, yellow, purple, orange, cyan)
- `group`: String group name or None

**Coordinate Synchronization:**
- All three coordinate systems must be kept in sync
- Normalized coordinates are calculated from image coordinates
- Canvas coordinates are recalculated from normalized on zoom/pan
- Image coordinates are calculated from normalized when image loads

### 2.3 Region Selection
**Requirement FR-005**: The application MUST support single and multi-region selection.

**Selection Methods:**
1. **Single Click**: Click region to select (deselects others)
2. **Command+Click**: Toggle region in multi-selection
3. **Box Selection**: Click empty space and drag to select multiple regions
4. **Shift+Drag**: Box selection that adds to existing selection
5. **Listbox Selection**: Click region name in list to select

**Selection State:**
- `selected_region`: Primary selected region (for resize handles)
- `selected_regions`: Set of all selected regions
- Visual feedback: Selected regions have thicker border and fill

**Selection Visual Feedback:**
- Normal: 2px border, no fill
- Hovered: 3px border, 10% opacity fill
- Selected: 4px border, 20% opacity fill, resize handles visible

### 2.4 Region Movement
**Requirement FR-006**: The application MUST support moving regions by dragging.

**Details:**
- In Select mode, click and drag a region to move it
- All selected regions move together
- Movement updates all three coordinate systems
- Visual feedback during drag (real-time position update)
- State is saved to undo stack on release

**Movement Behavior:**
- Click on region (not resize handle) to start drag
- Drag updates region position in real-time
- Release finalizes position and saves state
- Multiple selected regions move as a group

### 2.5 Region Resizing
**Requirement FR-007**: The application MUST support resizing regions via handles.

**Details:**
- Selected region shows 8 resize handles (4 corners, 4 edges)
- Click and drag handle to resize
- Handles only visible for single selected region
- Resize updates all three coordinate systems
- State is saved to undo stack on release

**Resize Handles:**
- Corner handles: nw, ne, sw, se (resize width and height)
- Edge handles: n, s, w, e (resize one dimension)
- Handle size: 8x8 pixels
- Handle appearance: White fill, black outline
- Cursor changes based on handle position

### 2.6 Region Editing
**Requirement FR-008**: The application MUST support editing region properties.

**Editable Properties:**
- Name: Double-click region or use Edit button
- Color: Right-click menu or Change Color button
- Group: Assign via group input field or group operations

**Name Editing:**
- Double-click region to open edit dialog
- Or select region and press E key
- New name must be unique
- Updates region in all groups
- State saved to undo stack

**Color Editing:**
- Right-click region → "Change Color"
- Or select region and use Change Color button
- Available colors: blue, red, green, yellow, purple, orange, cyan
- State saved to undo stack

### 2.7 Region Deletion
**Requirement FR-009**: The application MUST support deleting regions.

**Deletion Methods:**
- Select region(s) and press Delete/Backspace
- Right-click region → "Delete"
- Select region in listbox and use Delete button

**Deletion Behavior:**
- Deletes all selected regions
- Removes region from its group
- Removes group if it becomes empty
- Clears selection after deletion
- State saved to undo stack before deletion

## 3. Group Management

### 3.1 Group Creation
**Requirement FR-010**: The application MUST support organizing regions into groups.

**Group Creation Methods:**
1. Enter group name when creating region
2. Select regions and use "Group Selected" button
3. Select regions and press Cmd+G

**Group Behavior:**
- Groups can contain any number of regions
- Regions can belong to at most one group (or None)
- Group names are case-sensitive
- Creating group with existing name adds to that group (with confirmation)

### 3.2 Group Operations
**Requirement FR-011**: The application MUST support group management operations.

**Operations:**
- **Create Group**: Create new group from selected regions
- **Add to Group**: Add selected regions to existing group
- **Remove from Group**: Remove selected regions from their group
- **Delete Group**: Delete group (regions become ungrouped)

**Group Management:**
- Groups displayed in side panel listbox
- Format: "Group Name (N regions)"
- Clicking group shows member regions in info panel
- Deleting group ungroups all member regions
- State saved to undo stack for all group operations

## 4. Zoom and Navigation

### 4.1 Zoom Functionality
**Requirement FR-012**: The application MUST support zooming the document view.

**Zoom Methods:**
- Zoom In button (+)
- Zoom Out button (-)
- Reset Zoom button
- Keyboard: +/= (zoom in), - (zoom out), 0 (reset)
- Command+Scroll: Zoom in/out

**Zoom Behavior:**
- Zoom range: 0.2x (20%) to 5.0x (500%)
- Zoom level displayed as percentage (100% = 1.0x)
- Zoom multiplies base scale factor
- Regions scale proportionally with zoom
- Canvas coordinates recalculated on zoom change

### 4.2 Scrolling
**Requirement FR-013**: The application MUST support scrolling the canvas.

**Scroll Methods:**
- Scrollbars (horizontal and vertical)
- Mouse wheel: Vertical scrolling
- Shift+Mouse wheel: Horizontal scrolling
- Magic Mouse side-to-side: Horizontal scrolling

**Scroll Behavior:**
- Scrollbars appear when content exceeds canvas size
- Smooth scrolling
- Scroll position maintained during zoom
- Image and regions scroll together

## 5. Visual Feedback and UI

### 5.1 Hover Feedback
**Requirement FR-014**: The application MUST provide visual feedback on hover.

**Hover Behavior:**
- In Select mode, hovering over region highlights it
- Hovered region: 3px border, 10% opacity fill
- Cursor changes to "fleur" (move cursor) over regions
- Cursor changes to resize cursors over handles
- Hover state clears when mouse leaves canvas

### 5.2 Selection Feedback
**Requirement FR-015**: The application MUST clearly indicate selected regions.

**Selection Indicators:**
- Thicker border (4px vs 2px)
- Semi-transparent fill (20% opacity)
- Resize handles (8 handles for single selection)
- Larger, bold label text
- Highlighted in region listbox

### 5.3 Mask Overlay
**Requirement FR-016**: The application MUST support masking areas outside regions.

**Mask Behavior:**
- Toggle via checkbox or M key
- When enabled, areas outside regions are greyed out
- Regions appear as "windows" in the mask
- Mask is semi-transparent (200 alpha on grey)
- Helps visualize what OCR will see

## 6. Export and Import

### 6.1 Coordinate Export
**Requirement FR-017**: The application MUST export coordinates in multiple formats.

**Supported Formats:**
- JSON (default, structured data)
- CSV (spreadsheet-friendly)
- TXT (human-readable)
- XML (structured XML)
- YAML (YAML format, requires library)

**Export Requirements:**
- Only normalized coordinates (0.0-1.0) are exported
- Includes aspect ratio information
- Includes region metadata (name, color, group)
- Includes group definitions
- File format determined by extension

**Export Process:**
1. User clicks "Export Coordinates" or presses Cmd+S
2. File dialog appears with format options
3. User selects location and format
4. File is saved with appropriate format
5. Success message displayed

### 6.2 Coordinate Import
**Requirement FR-018**: The application MUST import coordinates from JSON files.

**Import Requirements:**
- Loads normalized coordinates
- Reconstructs image coordinates if image is loaded
- Reconstructs groups from region data if groups section missing
- Handles both dict and list format for normalized coordinates
- Validates coordinate ranges (0.0-1.0)
- Shows success message with region count

**Import Process:**
1. User clicks "Load Coordinates" or presses Cmd+Shift+S
2. File dialog appears (JSON files only)
3. User selects coordinate file
4. Regions are loaded and displayed
5. Groups are reconstructed
6. Success message displayed

### 6.3 Mask Export
**Requirement FR-019**: The application MUST export mask images for OCR preprocessing.

**Mask Export Requirements:**
- Creates black image (same size as original)
- Draws white rectangles for each region
- Supports PNG, JPEG, TIFF formats
- Also saves JSON coordinate file alongside mask
- Shows success message with file paths

**Mask Usage:**
- Mask can be overlaid on original image during OCR
- Black areas hide unnecessary information
- White rectangles show only labeled cells
- Improves OCR accuracy by reducing noise

## 7. Undo/Redo System

### 7.1 Undo Functionality
**Requirement FR-020**: The application MUST support undoing operations.

**Undo Behavior:**
- Undo button (enabled when undo stack has items)
- Keyboard shortcut: Cmd+Z
- Restores previous state (regions and groups)
- Pushes current state to redo stack
- Updates all UI elements (lists, canvas, buttons)

**Undoable Operations:**
- Create region
- Move region
- Resize region
- Delete region
- Rename region
- Change region color
- Create group
- Add to group
- Remove from group
- Delete group

### 7.2 Redo Functionality
**Requirement FR-021**: The application MUST support redoing undone operations.

**Redo Behavior:**
- Redo button (enabled when redo stack has items)
- Keyboard shortcuts: Cmd+Shift+Z or Cmd+Y
- Restores next state from redo stack
- Pushes current state to undo stack
- Updates all UI elements

**Redo Limitations:**
- Redo stack is cleared when new action is performed
- Maximum undo history: 50 states

## 8. Keyboard Shortcuts

### 8.1 Mode Switching
**Requirement FR-022**: The application MUST support keyboard mode switching.

**Shortcuts:**
- 1: Switch to Create mode
- 2: Switch to Select mode

### 8.2 File Operations
**Requirement FR-023**: The application MUST support keyboard file operations.

**Shortcuts:**
- Cmd+O: Load PDF
- Cmd+S: Export coordinates
- Cmd+Shift+S: Load coordinates
- Cmd+M: Export mask

### 8.3 Region Operations
**Requirement FR-024**: The application MUST support keyboard region operations.

**Shortcuts:**
- Delete/Backspace: Delete selected regions
- E: Edit selected region name
- M: Toggle mask overlay
- T: Test OCR extraction

### 8.4 Selection Operations
**Requirement FR-025**: The application MUST support keyboard selection operations.

**Shortcuts:**
- Cmd+A: Select all regions
- Cmd+D: Deselect all regions
- Cmd+I: Invert selection
- Cmd+Click: Toggle multi-select

### 8.5 Group Operations
**Requirement FR-026**: The application MUST support keyboard group operations.

**Shortcuts:**
- Cmd+G: Group selected regions
- Cmd+Shift+G: Ungroup selected regions

### 8.6 Zoom Operations
**Requirement FR-027**: The application MUST support keyboard zoom operations.

**Shortcuts:**
- + or =: Zoom in
- -: Zoom out
- 0: Reset zoom

## 9. Error Handling

### 9.1 File Operation Errors
**Requirement FR-028**: The application MUST handle file operation errors gracefully.

**Error Scenarios:**
- PDF load failure: Show error dialog, maintain current state
- Export failure: Show error dialog, preserve data
- Import failure: Show error dialog, validate format

**Error Messages:**
- Clear, user-friendly error messages
- Include error details when helpful
- Do not crash application

### 9.2 Input Validation
**Requirement FR-029**: The application MUST validate user input.

**Validation Rules:**
- Region name: Required, must be unique
- Group name: Optional, can be empty
- Coordinate ranges: 0.0-1.0 for normalized coordinates
- Duplicate region names: Prevent with error message

**Validation Feedback:**
- Show warnings for missing required fields
- Show errors for invalid input
- Prevent invalid operations

## 10. Performance Requirements

### 10.1 Responsiveness
**Requirement FR-030**: The application MUST remain responsive during operations.

**Performance Targets:**
- Window resize: Debounced redraw (300ms delay)
- Canvas redraw: < 100ms for typical document (50 regions)
- Zoom operations: Smooth, no lag
- Region manipulation: Real-time visual feedback

### 10.2 Memory Management
**Requirement FR-031**: The application MUST manage memory efficiently.

**Memory Limits:**
- Undo stack limit: 50 states
- Image caching: Single page only
- Canvas elements: Cleaned up on redraw

### 10.3 Scalability
**Requirement FR-032**: The application MUST handle large documents and many regions.

**Scalability Targets:**
- Support documents up to 10,000x10,000 pixels
- Support up to 500 regions per document
- Handle zoom levels from 0.2x to 5.0x

