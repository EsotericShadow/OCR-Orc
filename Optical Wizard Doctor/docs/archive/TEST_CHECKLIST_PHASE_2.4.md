# Phase 2.4: Region Selection - Test Checklist

## Application Status
- ✅ Build: Successful
- ✅ Application: Running
- ✅ Logs: Tracking to `/tmp/owd_test.log`

---

## Test Actions (Perform in Order)

### 1. Prerequisites
- [x] Application is running
- [x] PDF is loaded
- [x] At least 3-4 regions are created (for testing multi-select)

### 2. Mode Switching to Select
- [x] Click "Select/Move" mode button
- [x] **OBSERVE:** Cursor changes to arrow
- [x] **OBSERVE:** "Create" mode button is deselected
- [x] **OBSERVE:** "Select/Move" mode button is selected

### 3. Hover State (Visual Feedback)
- [x] Move mouse over a region (without clicking)
- [x] **OBSERVE:** Region border thickens to 3px
- [x] **OBSERVE:** Region shows 10% opacity fill (slight tint)
- [x] **OBSERVE:** Region color matches the region's assigned color
- [x] Move mouse away from region
- [x] **OBSERVE:** Region returns to normal state (2px border, no fill)
- [x] Test with multiple regions - hover should work on all

### 4. Single Click Selection
- [x] Click on a region
- [x] **OBSERVE:** Region border thickens to 4px
- [x] **OBSERVE:** Region shows 20% opacity fill
- [x] **OBSERVE:** Resize handles appear (8 handles: 4 corners, 4 edges)
- [x] **OBSERVE:** Resize handles are white with black outline
- [x] **OBSERVE:** Other regions are not selected (normal state)
- [x] Click on a different region
- [x] **OBSERVE:** Previous region is deselected
- [x] **OBSERVE:** New region is selected
- [x] **OBSERVE:** Resize handles move to new region

### 5. Multi-Select (Cmd+Click)
- [x] Click on a region to select it
- [x] Hold Cmd (or Ctrl on Windows/Linux) and click another region
- [x] **OBSERVE:** Both regions are now selected (4px border, 20% fill)
- [x] **OBSERVE:** Resize handles disappear (only show for single selection)
- [x] Cmd+Click the first region again
- [x] **OBSERVE:** First region is deselected (returns to normal)
- [x] **OBSERVE:** Second region remains selected
- [x] Cmd+Click a third region
- [x] **OBSERVE:** All three regions are selected
- [x] **OBSERVE:** No resize handles (multi-select)

### 6. Clear Selection
- [x] Have multiple regions selected
- [x] Click on empty space (not on any region)
- [x] **OBSERVE:** All regions are deselected
- [x] **OBSERVE:** All regions return to normal state (2px border)
- [x] **OBSERVE:** Resize handles disappear

### 7. Box Selection (Click and Drag)
- [x] Click and hold on empty space (not on any region)
- [x] Drag to create a selection box
- [x] **OBSERVE:** Blue dashed rectangle appears while dragging
- [x] **OBSERVE:** Blue rectangle has semi-transparent fill
- [x] Drag to encompass 2-3 regions
- [x] Release mouse button
- [x] **OBSERVE:** All regions inside box are selected
- [x] **OBSERVE:** Selection box disappears
- [x] **OBSERVE:** Selected regions show 4px border, 20% fill
- [x] **OBSERVE:** No resize handles (multi-select)* it shows the squares to do the resizing, but the cursor doesnt change, and resizing functionality doesnt work yet, its not yet implemented, but i can see that it will. 

### 8. Box Selection - Shift+Drag (Add to Selection)
- [x] Select one region by clicking
- [x] Hold Shift and click on empty space
- [x] Drag to create selection box
- [x] **OBSERVE:** Blue selection box appears
- [x] Drag to encompass other regions
- [x] Release mouse button
- [x] **OBSERVE:** Original region remains selected
- [x] **OBSERVE:** Regions in box are added to selection
- [x] **OBSERVE:** All selected regions show selected state

### 9. Resize Handle Detection
- [x] Select a single region
- [x] **OBSERVE:** 8 resize handles appear
- [x] Move mouse over a resize handle
- [x] **OBSERVE:** Hover state clears (region doesn't show hover when over handle)
- [x] Move mouse away from handle but still over region
- [x] **OBSERVE:** Hover state returns
- [x] **NOTE:** Resize functionality will be in Phase 2.5

### 10. Selection with Overlapping Regions
- [x] Create two overlapping regions (if possible)
- [x] Click on the overlap area
- [x] **OBSERVE:** One region is selected (topmost or first found)
- [x] **OBSERVE:** Selection is consistent (same region selected each time)

### 11. Visual State Priority
- [x] Select a region
- [x] Move mouse over the selected region
- [x] **OBSERVE:** Selected state takes priority (4px border, not 3px hover)
- [x] **OBSERVE:** Still shows 20% fill (selected), not 10% (hovered)
- [x] Move mouse over a different, unselected region
- [x] **OBSERVE:** That region shows hover state (3px, 10% fill)
- [x] **OBSERVE:** Selected region remains selected

### 12. Selection Persistence
- [x] Select a region
- [x] Switch to "Create" mode
- [x] **OBSERVE:** Selection remains (regions still show selected state)
- [x] Switch back to "Select/Move" mode
- [x] **OBSERVE:** Selection is still there
- [x] Create a new region
- [x] **OBSERVE:** Previous selection remains

### 13. Edge Cases
- [x] Try to select when no PDF is loaded
- [x] **OBSERVE:** No selection possible (or appropriate behavior)
- [x] Try to select when no regions exist
- [x] **OBSERVE:** Clicking does nothing (no error)
- [x] Try box selection with no regions in box
- [x] **OBSERVE:** Selection is cleared (empty selection)

### 14. Performance
- [x] Create 10+ regions
- [x] Test hover state with many regions
- [x] **OBSERVE:** Hover feedback is responsive (no lag)
- [x] Test box selection with many regions
- [x] **OBSERVE:** Selection is fast and smooth
- [x] **OBSERVE:** No UI freezing or stuttering

---

## What to Report

After testing, please report:

1. **What worked:**
   - List all features that worked as expected

2. **What didn't work:**
   - List any features that failed or behaved unexpectedly
   - Describe the exact behavior you observed
   - Note which action you were performing

3. **Visual issues:**
   - Any rendering problems
   - Any incorrect visual states
   - Any missing visual feedback
   - Any flickering or glitches

4. **Selection issues:**
   - Any regions not selecting when clicked
   - Any incorrect multi-select behavior
   - Any box selection problems
   - Any selection not clearing properly

5. **Performance issues:**
   - Any lag or slowness
   - Any UI freezing
   - Any delays in visual feedback

6. **Edge cases:**
   - Any unexpected behavior
   - Any crashes or errors

---

## Expected Behaviors Summary

### Hover State
- **Trigger:** Mouse over region (in Select mode)
- **Visual:** 3px border, 10% opacity fill
- **Duration:** While mouse is over region

### Selected State
- **Trigger:** Click on region
- **Visual:** 4px border, 20% opacity fill
- **Duration:** Until deselected or another region selected

### Primary Selected (Single Selection)
- **Trigger:** Single region selected
- **Visual:** 4px border, 20% fill, resize handles visible
- **Duration:** While only one region is selected

### Box Selection
- **Trigger:** Click and drag in empty space
- **Visual:** Blue dashed rectangle with semi-transparent fill
- **Result:** All regions intersecting box are selected

### Multi-Select
- **Trigger:** Cmd+Click on regions
- **Visual:** All selected regions show 4px border, 20% fill
- **Note:** No resize handles in multi-select

---

## Logs Location

Application logs are being tracked to: `/tmp/owd_test.log`

I will analyze the logs after you report your findings.

