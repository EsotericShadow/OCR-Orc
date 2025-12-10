# Region Resize - Implementation Steps

## Objective

Enable users to resize regions by dragging resize handles in Select mode.

## Prerequisites

- Region selection working
- Single region selection shows resize handles
- Mouse event handling set up
- Coordinate system implemented

## Resize Handles

8 handles per region:
- **Corners**: nw (top-left), ne (top-right), sw (bottom-left), se (bottom-right)
- **Edges**: n (top), s (bottom), w (left), e (right)

## Steps

### Step 1: Set Up Resize State

1. Add to Canvas class:
   - `bool isResizing` - Resize operation in progress
   - `QString resizingRegion` - Region being resized
   - `QString resizeHandle` - Which handle (nw, ne, sw, se, n, s, w, e)
   - `QPointF resizeStartPos` - Starting position
   - `QRectF originalRect` - Original region bounds
2. Initialize as false

### Step 2: Detect Resize Handle Click

1. Create function `QString getHandleAt(const QPointF& pos, const QRectF& rect)`
2. Check each handle position:
   - Corner handles: at rect corners
   - Edge handles: at rect edge midpoints
3. Handle size: 8x8 pixels
4. Tolerance: 13 pixels (8 + 5) for easier clicking
5. Return handle identifier or empty string

### Step 3: Handle Mouse Press on Handle

1. In `mousePressEvent()`, if mode is Select:
2. Check if single region is selected
3. Get handle at click position
4. If handle found:
   - Set `isResizing = true`
   - Set `resizingRegion = primarySelectedRegion`
   - Set `resizeHandle = handle`
   - Store `resizeStartPos = mousePos`
   - Store `originalRect = regionRect`
   - Change cursor to appropriate resize cursor

### Step 4: Set Resize Cursors

1. Create cursor map based on handle:
   - nw, se: `top_left_corner`, `bottom_right_corner`
   - ne, sw: `top_right_corner`, `bottom_left_corner`
   - n, s: `top_side`, `bottom_side`
   - w, e: `left_side`, `right_side`
2. Update cursor in mouseMoveEvent when over handle
3. Update cursor during resize operation

### Step 5: Handle Mouse Drag During Resize

1. In `mouseMoveEvent()`, if `isResizing` is true:
2. Get current mouse position (canvas coordinates)
3. Calculate new rectangle based on handle:
   - Corner handles: update two coordinates
   - Edge handles: update one coordinate
4. Constrain to minimum size (e.g., 10x10 pixels)
5. Update region coordinates
6. Redraw canvas (region resizes in real-time)

### Step 6: Calculate New Rectangle

1. Create function `QRectF calculateResizedRect(const QRectF& original, const QPointF& newPos, const QString& handle)`
2. For each handle type:
   - **nw**: new top-left = newPos, keep bottom-right
   - **ne**: new top-right = newPos, keep bottom-left
   - **sw**: new bottom-left = newPos, keep top-right
   - **se**: new bottom-right = newPos, keep top-left
   - **n**: new top = newPos.y, keep others
   - **s**: new bottom = newPos.y, keep others
   - **w**: new left = newPos.x, keep others
   - **e**: new right = newPos.x, keep others
3. Ensure x1 < x2, y1 < y2

### Step 7: Update Region Coordinates

1. Create function `void resizeRegion(const QString& name, const QRectF& newCanvasRect)`
2. Update canvas coordinates from new rectangle
3. Convert to image coordinates:
   - Account for image offset and scale factor
   - Use CoordinateSystem::canvasToImage()
4. Convert to normalized coordinates:
   - Use CoordinateSystem::imageToNormalized()
5. Update all three coordinate systems

### Step 8: Enforce Minimum Size

1. Check if new size is below minimum (e.g., 10x10 pixels)
2. If too small:
   - Clamp to minimum size
   - Or reject resize (keep original size)
3. Apply constraint in canvas coordinates
4. Recalculate other coordinate systems

### Step 9: Handle Boundary Constraints

1. Prevent resizing outside document bounds
2. For each handle:
   - Check if new position is within bounds
   - Clamp to bounds if needed
3. Apply to canvas, image, and normalized coordinates

### Step 10: Handle Mouse Release

1. In `mouseReleaseEvent()`, if `isResizing` is true:
2. Finalize resize (last update)
3. Validate final coordinates
4. Save state to undo stack (before resize)
5. Set `isResizing = false`
6. Reset cursor to default
7. Redraw canvas

### Step 11: Visual Feedback During Resize

1. Show region resizing in real-time
2. Keep selection highlighting visible
3. Update resize handles position during resize
4. Show cursor indicating resize direction

### Step 12: Handle Resize Cancellation

1. If Escape key pressed during resize:
   - Cancel resize operation
   - Restore region to originalRect
   - Set `isResizing = false`
2. Restore original coordinates

### Step 13: Test Resize

1. Select single region
2. Test each of 8 handles
3. Test minimum size constraint
4. Test boundary constraints
5. Test coordinate synchronization
6. Test undo after resize
7. Test with different zoom levels

## Acceptance Criteria

- [ ] All 8 resize handles are visible for single selection
- [ ] Users can drag handles to resize region
- [ ] Resize is smooth and responsive
- [ ] Minimum size is enforced
- [ ] Regions cannot resize outside bounds
- [ ] All coordinate systems stay synchronized
- [ ] State is saved to undo stack on release
- [ ] Cursor changes appropriately

## Code Structure

```cpp
class Canvas : public QWidget {
    bool isResizing;
    QString resizingRegion;
    QString resizeHandle;
    QPointF resizeStartPos;
    QRectF originalRect;
    
    QString getHandleAt(const QPointF& pos, const QRectF& rect);
    QRectF calculateResizedRect(const QRectF& original, 
                                const QPointF& newPos, 
                                const QString& handle);
    void resizeRegion(const QString& name, const QRectF& newCanvasRect);
    void enforceMinimumSize(QRectF& rect);
    void constrainToBounds(QRectF& rect);
    
protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};
```

## Notes

- Only show handles for single selection
- Handle positions update during resize
- Calculate new coordinates accurately
- Test with various handle types
- Ensure smooth resize experience

