# Region Selection - Implementation Steps

## Objective

Enable users to select regions (single and multi-select) in Select mode.

## Prerequisites

- Canvas widget created
- Region rendering working
- Mouse event handling set up
- DocumentState ready

## Selection Methods

1. **Single Click**: Click region to select (deselects others)
2. **Command+Click**: Toggle region in multi-selection
3. **Box Selection**: Click empty space and drag to select multiple
4. **Shift+Drag**: Box selection that adds to existing selection

## Steps

### Step 1: Set Up Selection State

1. Add to Canvas class:
   - `QSet<QString> selectedRegions` - All selected regions
   - `QString primarySelectedRegion` - For resize handles
   - `QString hoveredRegion` - For hover feedback
2. Initialize as empty sets

### Step 2: Implement Region Hit Testing

1. Create function `QString getRegionAt(const QPointF& canvasPos)`
2. Iterate through all regions
3. Check if point is inside region rectangle
4. Use canvas coordinates for comparison
5. Return region name or empty string
6. Handle overlapping regions (return first match)

### Step 3: Handle Single Click Selection

1. In `mousePressEvent()`, check if mode is Select
2. Get region at click position using hit testing
3. If region found:
   - Clear current selection
   - Add region to selectedRegions
   - Set as primarySelectedRegion
   - Start drag operation (for movement)
4. If no region found:
   - Clear selection
   - Start box selection

### Step 4: Implement Command+Click Multi-Select

1. Check for modifier key (Command/Ctrl) in mousePressEvent
2. If Command+Click on region:
   - Toggle region in selectedRegions
   - If already selected: remove from set
   - If not selected: add to set
   - Update primarySelectedRegion
3. Redraw canvas to show updated selection

### Step 5: Implement Box Selection

1. On mouse press in empty space, store start position
2. Set flag: `isBoxSelecting = true`
3. On mouse drag:
   - Calculate selection box rectangle
   - Draw dashed blue rectangle
   - Find all regions intersecting box
4. On mouse release:
   - Finalize selection (add intersecting regions)
   - Clear selection box
   - Redraw canvas

### Step 6: Implement Shift+Drag Box Selection

1. Check for Shift modifier in mousePressEvent
2. If Shift+Click in empty space:
   - Start box selection
   - Don't clear existing selection
3. On mouse release:
   - Add intersecting regions to existing selection
   - Don't remove current selection

### Step 7: Implement Selection Box Rendering

1. Create function `void drawSelectionBox(const QRectF& rect)`
2. Draw dashed blue outline (#0066ff)
3. Line width: 2px
4. Dash pattern: 5px dash, 5px gap
5. Draw during mouse drag
6. Clear on mouse release

### Step 8: Find Regions in Selection Box

1. Create function `QSet<QString> findRegionsInBox(const QRectF& box)`
2. For each region:
   - Get canvas coordinates
   - Check if region intersects with box
   - Use QRectF::intersects() or manual check
3. Return set of region names

### Step 9: Update Visual Feedback

1. In `renderRegions()`, check if region is in selectedRegions
2. If selected: use selected state styling (4px border, 20% fill)
3. If hovered: use hovered state styling (3px border, 10% fill)
4. If primary selected: show resize handles
5. Redraw on selection change

### Step 10: Implement Selection from Listbox

1. Connect listbox selection to canvas
2. When region selected in listbox:
   - Clear canvas selection
   - Select region in canvas
   - Scroll canvas to show region (if needed)
   - Update info display

### Step 11: Implement Select All / Deselect All

1. Add keyboard shortcuts: Cmd+A (select all), Cmd+D (deselect all)
2. Select All: Add all region names to selectedRegions
3. Deselect All: Clear selectedRegions
4. Update primarySelectedRegion
5. Redraw canvas

### Step 12: Implement Invert Selection

1. Add keyboard shortcut: Cmd+I
2. Get all region names
3. Calculate: unselected = all - selected
4. Set selectedRegions to unselected
5. Update primarySelectedRegion
6. Redraw canvas

### Step 13: Test Selection

1. Test single click selection
2. Test Command+Click multi-select
3. Test box selection
4. Test Shift+Drag box selection
5. Test select all/deselect all
6. Test invert selection
7. Verify visual feedback is correct

## Acceptance Criteria

- [ ] Single click selects region
- [ ] Command+Click toggles multi-select
- [ ] Box selection works (click and drag)
- [ ] Shift+Drag adds to selection
- [ ] Selected regions show correct visual styling
- [ ] Resize handles appear for single selection
- [ ] Selection from listbox works
- [ ] Keyboard shortcuts work (Cmd+A, Cmd+D, Cmd+I)

## Code Structure

```cpp
class Canvas : public QWidget {
    QSet<QString> selectedRegions;
    QString primarySelectedRegion;
    QString hoveredRegion;
    bool isBoxSelecting;
    QRectF selectionBox;
    
    QString getRegionAt(const QPointF& canvasPos);
    QSet<QString> findRegionsInBox(const QRectF& box);
    void drawSelectionBox(const QRectF& rect);
    void updateSelection(const QSet<QString>& newSelection);
};
```

## Notes

- Use QSet for selectedRegions (fast lookup, no duplicates)
- Update visual feedback immediately on selection change
- Handle edge cases (clicking on resize handle vs region)
- Test with overlapping regions
- Ensure selection state is consistent

