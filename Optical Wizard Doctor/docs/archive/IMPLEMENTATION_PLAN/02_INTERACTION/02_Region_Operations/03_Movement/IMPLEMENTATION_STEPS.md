# Region Movement - Implementation Steps

## Objective

Enable users to move selected regions by dragging them in Select mode.

## Prerequisites

- Region selection working
- Mouse event handling set up
- Coordinate system implemented
- DocumentState ready

## Steps

### Step 1: Set Up Drag State

1. Add to Canvas class:
   - `bool isDragging` - Drag operation in progress
   - `QPointF dragStartPos` - Starting position of drag
   - `QPointF lastDragPos` - Last position (for delta calculation)
2. Initialize as false

### Step 2: Detect Drag Start

1. In `mousePressEvent()`, if mode is Select:
2. Check if clicked on selected region (not resize handle)
3. If yes:
   - Set `isDragging = true`
   - Store `dragStartPos = mousePos`
   - Store `lastDragPos = mousePos`
   - Change cursor to "fleur" (move cursor)
4. Don't start drag if clicking on resize handle

### Step 3: Handle Mouse Drag

1. In `mouseMoveEvent()`, if `isDragging` is true:
2. Get current mouse position (canvas coordinates)
3. Calculate delta: `delta = currentPos - lastDragPos`
4. For each region in selectedRegions:
   - Move region by delta
   - Update all three coordinate systems
5. Update `lastDragPos = currentPos`
6. Redraw canvas (regions move in real-time)

### Step 4: Implement Region Movement

1. Create function `void moveRegion(const QString& name, const QPointF& delta)`
2. Get region from DocumentState
3. Update canvas coordinates: `canvasCoords += delta`
4. Convert delta to image coordinates:
   - `imageDelta = delta / scaleFactor`
5. Update image coordinates: `imageCoords += imageDelta`
6. Convert delta to normalized:
   - `normalizedDelta = imageDelta / imageDimensions`
7. Update normalized coordinates: `normalizedCoords += normalizedDelta`
8. Validate coordinates (clamp to bounds if needed)

### Step 5: Move Multiple Regions

1. Create function `void moveSelectedRegions(const QPointF& delta)`
2. Iterate through selectedRegions
3. Call moveRegion() for each
4. All regions move together as a group
5. Maintain relative positions

### Step 6: Handle Boundary Constraints

1. Check if movement would place region outside bounds
2. For canvas coordinates: clamp to canvas size
3. For image coordinates: clamp to image dimensions
4. For normalized: clamp to 0.0-1.0
5. Prevent regions from moving outside document

### Step 7: Update Coordinate Systems

1. After moving, ensure all three coordinate systems are synchronized
2. Recalculate from normalized (source of truth)
3. Or update all three directly (faster, but must stay in sync)
4. Validate coordinates after update

### Step 8: Handle Mouse Release

1. In `mouseReleaseEvent()`, if `isDragging` is true:
2. Finalize movement (last delta update)
3. Save state to undo stack (before movement)
4. Set `isDragging = false`
5. Reset cursor to default
6. Redraw canvas

### Step 9: Visual Feedback During Drag

1. Show regions moving in real-time
2. Keep selection highlighting visible
3. Update cursor to show dragging state
4. Don't show resize handles during drag (distracting)

### Step 10: Handle Drag Cancellation

1. If Escape key pressed during drag:
   - Cancel drag operation
   - Restore regions to original positions
   - Set `isDragging = false`
2. If mouse leaves canvas during drag:
   - Continue drag (mouse can return)
   - Or cancel drag (user preference)

### Step 11: Optimize Performance

1. Only redraw affected regions during drag
2. Use dirty region updates if possible
3. Limit redraw frequency (throttle if needed)
4. Cache coordinate calculations

### Step 12: Test Movement

1. Select single region, drag to move
2. Select multiple regions, drag to move together
3. Test boundary constraints (try to move outside)
4. Test coordinate synchronization
5. Test undo after movement
6. Test performance with many regions

## Acceptance Criteria

- [ ] Users can drag selected regions to move them
- [ ] Multiple regions move together as a group
- [ ] Movement is smooth and responsive
- [ ] All coordinate systems stay synchronized
- [ ] Regions cannot move outside document bounds
- [ ] State is saved to undo stack on release
- [ ] Visual feedback is immediate

## Code Structure

```cpp
class Canvas : public QWidget {
    bool isDragging;
    QPointF dragStartPos;
    QPointF lastDragPos;
    
    void moveRegion(const QString& name, const QPointF& delta);
    void moveSelectedRegions(const QPointF& delta);
    void constrainToBounds(RegionData& region);
    
protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;  // For Escape
};
```

## Notes

- Calculate delta in canvas coordinates, convert to other systems
- Keep all three coordinate systems synchronized
- Validate coordinates after movement
- Save state to undo stack on release (not during drag)
- Test with various zoom levels

