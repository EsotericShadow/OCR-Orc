# Keyboard Shortcuts Reference

## Mode Switching

| Key | Action | Description |
|-----|--------|-------------|
| `1` | Switch to Create Mode | Enter region creation mode |
| `2` | Switch to Select Mode | Enter region selection/movement mode |

## File Operations

| Shortcut | Action | Description |
|----------|--------|-------------|
| `Cmd+O` | Load PDF | Open file dialog to load PDF document |
| `Cmd+S` | Export Coordinates | Export coordinates in selected format |
| `Cmd+Shift+S` | Load Coordinates | Import coordinates from JSON file |
| `Cmd+M` | Export Mask | Export mask image for OCR preprocessing |

## Zoom Operations

| Key | Action | Description |
|-----|--------|-------------|
| `+` or `=` | Zoom In | Increase zoom level by 20% (up to 1000%) |
| `-` | Zoom Out | Decrease zoom level by 20% (down to 10%) |
| `0` | Reset Zoom | Reset zoom to 100% (fit to canvas) |
| `Cmd+Scroll` / `Ctrl+Scroll` | Zoom to Cursor | Zoom in/out centered on cursor position (10% per step) |
| `Escape` | Cancel Zoom | (Future: Cancel ongoing zoom operation) |

## Region Operations

| Key | Action | Description |
|-----|--------|-------------|
| `Delete` | Delete Selected | Delete all selected regions |
| `Backspace` | Delete Selected | Delete all selected regions |
| `E` | Edit Region | Edit name of selected region |
| `M` | Toggle Mask | Toggle mask overlay on/off |
| `T` | Test Extraction | Test OCR extraction on all regions |
| `Escape` | Cancel Operation | Cancel ongoing drag, resize, or creation operation |

## Selection Operations

| Shortcut | Action | Description |
|----------|--------|-------------|
| `Cmd+A` | Select All | Select all regions |
| `Cmd+D` | Duplicate Selected | Duplicate all selected regions with auto-incremented names |
| `Cmd+I` | Invert Selection | Select all unselected regions |
| `Cmd+Click` | Toggle Multi-Select | Add/remove region from selection |

## Group Operations

| Shortcut | Action | Description |
|----------|--------|-------------|
| `Cmd+G` | Group Selected | Create group from selected regions |
| `Cmd+Shift+G` | Ungroup Selected | Remove selected regions from their groups |

## Undo/Redo

| Shortcut | Action | Description |
|----------|--------|-------------|
| `Cmd+Z` | Undo | Undo last operation |
| `Cmd+Shift+Z` | Redo | Redo last undone operation |
| `Cmd+Y` | Redo | Alternative redo shortcut |

## Mouse Interactions

### Mouse Wheel

| Action | Behavior | Description |
|--------|----------|-------------|
| Scroll | Pan/Scroll | Scroll canvas when zoomed in |
| `Cmd+Scroll` / `Ctrl+Scroll` | Zoom to Cursor | Zoom in/out centered on cursor position |
| `Alt+Scroll` | (Reserved) | Future feature |

### Mouse Click Modifiers

| Action | Behavior | Description |
|--------|----------|-------------|
| Click | Select | Click region to select it |
| `Cmd+Click` / `Ctrl+Click` | Multi-Select | Toggle region in multi-selection |
| `Shift+Click` (on region) | Start Move | Click selected region with Shift to start moving it |
| `Shift+Drag` (empty space) | Box Select | Draw selection box to select multiple regions |
| `Shift+Drag` (on selected region) | Move Regions | Drag selected regions to move them |
| Middle-Click + Drag | Pan Canvas | Pan the canvas view |
| `Alt+Left-Click + Drag` | Pan Canvas | Alternative pan method |
| Double-Click | Edit Name | Double-click region to edit its name |
| Right-Click | Context Menu | Show context menu (Duplicate Selected, Select Region) |

### Region Resize

| Action | Behavior | Description |
|--------|----------|-------------|
| Drag Resize Handle | Resize Region | Drag any of the 8 resize handles (corners/edges) to resize a selected region |
| `Escape` | Cancel Resize | Cancel ongoing resize operation |

## Navigation & View

| Shortcut | Action | Description |
|----------|--------|-------------|
| `Middle-Click + Drag` | Pan Canvas | Pan the canvas view |
| `Alt+Left-Click + Drag` | Pan Canvas | Alternative pan method |
| `Mouse Wheel` | Scroll | Scroll canvas when zoomed in (without modifier) |
| `Cmd+Wheel` / `Ctrl+Wheel` | Zoom to Cursor | Zoom in/out at cursor position |

## Notes

- **Cmd** refers to the Command key on macOS (⌘)
- On Windows/Linux, use **Ctrl** instead of **Cmd**
- All shortcuts work when canvas has focus
- Some shortcuts require regions to be selected first
- Undo/Redo buttons are enabled/disabled based on available history
- **Shift+Drag** is required to move selected regions (prevents accidental movement)
- **Zoom-to-cursor**: When zooming with Cmd/Ctrl+Wheel, the point under your cursor stays fixed

