# Icon Inventory - Current Usage

This document lists all icons currently used in the application and which buttons they're assigned to.

## Icon Usage by Component

### Toolbar Widget

| Button | Icon Name | Current Icon | Qt Standard Icon Used |
|--------|-----------|--------------|----------------------|
| Zoom Out | `zoom_out` | Minus symbol (−) | Text fallback |
| Zoom In | `zoom_in` | Plus symbol (+) | Text fallback |
| Zoom Reset | `zoom_reset` | FileDialogBack | `SP_FileDialogBack` |
| Group | `group` | Folder icon | `SP_DirIcon` |
| Ungroup | `ungroup` | Open folder icon | `SP_DirClosedIcon` |
| Undo | `undo` | Back arrow | `SP_ArrowBack` |
| Redo | `redo` | Forward arrow | `SP_ArrowForward` |

### Control Panel Widget

| Button | Icon Name | Current Icon | Qt Standard Icon Used |
|--------|-----------|--------------|----------------------|
| Load PDF | `open` | Folder icon | `SP_DirOpenIcon` |
| Export Coordinates | `export` | Save icon | `SP_DialogSaveButton` |
| Export Mask | `mask` | File icon | `SP_FileIcon` |
| Load Coordinates | `import` | FileDialogStart | `SP_FileDialogStart` |
| Clear Selection | `clear` | Reset button | `SP_DialogResetButton` |
| Edit Selected | `edit` | Info view | `SP_FileDialogInfoView` |
| Test Extraction | `edit` | Info view (reused) | `SP_FileDialogInfoView` |

### Side Panel Widget

| Button | Icon Name | Current Icon | Qt Standard Icon Used |
|--------|-----------|--------------|----------------------|
| Create Group | `group` | Folder icon | `SP_DirIcon` |
| Add to Group | `add` | New folder | `SP_FileDialogNewFolder` |
| Remove from Group | `remove` | Trash icon | `SP_TrashIcon` |
| Delete Group | `delete` | Trash icon | `SP_TrashIcon` |
| Help | `help` | Question mark | `SP_MessageBoxQuestion` |

## Icon Name Constants

All icon names are defined in `IconManager.h`:

- `ICON_OPEN` = "open"
- `ICON_SAVE` = "save"
- `ICON_EXPORT` = "export"
- `ICON_IMPORT` = "import"
- `ICON_UNDO` = "undo"
- `ICON_REDO` = "redo"
- `ICON_ZOOM_IN` = "zoom_in"
- `ICON_ZOOM_OUT` = "zoom_out"
- `ICON_ZOOM_RESET` = "zoom_reset"
- `ICON_CREATE` = "create"
- `ICON_SELECT` = "select"
- `ICON_GROUP` = "group"
- `ICON_UNGROUP` = "ungroup"
- `ICON_DELETE` = "delete"
- `ICON_EDIT` = "edit"
- `ICON_CLEAR` = "clear"
- `ICON_HELP` = "help"
- `ICON_MASK` = "mask"
- `ICON_COLOR` = "color"
- `ICON_ADD` = "add"
- `ICON_REMOVE` = "remove"

## Issues Identified

1. **Group vs Ungroup**: Both use folder icons (`SP_DirIcon` and `SP_DirClosedIcon`) - need better distinction
2. **Remove vs Delete**: Both use trash icon (`SP_TrashIcon`) - may need distinction
3. **Edit vs Test Extraction**: Both use info view icon - Test Extraction needs unique icon
4. **Zoom icons**: Currently using text symbols (+, −) - could use better icon graphics

## Recommended Icon Replacements

### High Priority (Confusing/Unclear)

1. **Group** (`group`): 
   - Current: Folder icon
   - Suggested: Link/chain icon, or grouped objects icon
   - Purpose: Combine regions into a group

2. **Ungroup** (`ungroup`):
   - Current: Open folder icon
   - Suggested: Unlink/break chain icon, or separated objects icon
   - Purpose: Remove regions from groups

3. **Test Extraction** (`edit` reused):
   - Current: Info view icon
   - Suggested: Play/run icon, or test/checkmark icon
   - Purpose: Test OCR extraction

### Medium Priority (Could be clearer)

4. **Zoom Reset** (`zoom_reset`):
   - Current: FileDialogBack
   - Suggested: Home/center icon, or reset/refresh icon
   - Purpose: Reset zoom to 100%

5. **Export Mask** (`mask`):
   - Current: File icon
   - Suggested: Image/mask icon, or layer icon
   - Purpose: Export mask image

6. **Load Coordinates** (`import`):
   - Current: FileDialogStart
   - Suggested: Import/upload icon, or file with arrow
   - Purpose: Import coordinates from file

### Low Priority (Acceptable but could improve)

7. **Zoom In/Out** (`zoom_in`, `zoom_out`):
   - Current: Text symbols (+, −)
   - Suggested: Magnifying glass with +/−, or zoom icons
   - Purpose: Zoom in/out

8. **Clear Selection** (`clear`):
   - Current: Reset button
   - Suggested: X/close icon, or clear/eraser icon
   - Purpose: Clear selection

## Icon Sources

Icons are currently sourced from:
- Qt Standard Icons (`QStyle::StandardPixmap`)
- Text fallbacks (Unicode symbols)
- Custom text icons (via `createTextIcon`)

## Next Steps

1. Find better icon resources (SVG icons, icon fonts, or custom icons)
2. Update `IconManager.cpp` to use new icons
3. Ensure icons are theme-aware (light/dark mode compatible)
4. Test icon visibility and clarity at different sizes

