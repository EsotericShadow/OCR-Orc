# Region Creation - Implementation Steps

## Objective

Enable users to create new regions by clicking and dragging on the canvas in Create mode.

## Prerequisites

- Canvas widget created
- Mouse event handling set up
- DocumentState ready
- Coordinate system implemented
- Mode switching working

## Steps

### Step 1: Set Up Create Mode

1. Add `mouseMode` member variable to Canvas (enum: Create, Select)
2. Add method `setMode(MouseMode mode)`
3. Update cursor to crosshair in Create mode
4. Connect mode button in toolbar to setMode()

### Step 2: Handle Mouse Press in Create Mode

1. Override `mousePressEvent()` in Canvas
2. Check if mode is Create
3. Convert screen coordinates to canvas coordinates
4. Store start position: `startPos = canvasCoords`
5. Clear any temporary rectangle
6. Set flag: `isCreating = true`

### Step 3: Handle Mouse Drag in Create Mode

1. Override `mouseMoveEvent()` in Canvas
2. Check if `isCreating` is true
3. Get current mouse position (canvas coordinates)
4. Delete old temporary rectangle if exists
5. Draw new temporary rectangle:
   - Outline: red, 2px width
   - From startPos to currentPos
   - Use dashed line style
6. Update display

### Step 4: Handle Mouse Release in Create Mode

1. Override `mouseReleaseEvent()` in Canvas
2. Check if `isCreating` is true
3. Get end position (canvas coordinates)
4. Calculate rectangle bounds (ensure x1 < x2, y1 < y2)
5. Validate region:
   - Minimum size: 10x10 pixels
   - Within canvas bounds
6. Get region name from input field
7. Validate name: not empty, unique

### Step 5: Convert Coordinates

1. Convert canvas coordinates to image coordinates:
   - Account for image offset and scale factor
   - Use CoordinateSystem::canvasToImage()
2. Convert image coordinates to normalized:
   - Use CoordinateSystem::imageToNormalized()
3. Store all three coordinate systems

### Step 6: Create Region Data

1. Get color from color dropdown
2. Get group name from group input (optional)
3. Create RegionData structure:
   - normalizedCoords
   - imageCoords
   - canvasCoords
   - name
   - color
   - group
4. Add to DocumentState

### Step 7: Update UI

1. Add region to region listbox
2. Update group list if group specified
3. Redraw canvas (regions will appear)
4. Clear temporary rectangle
5. Auto-increment region name if pattern detected

### Step 8: Implement Auto-Increment

1. Check if region name ends with number
2. If yes, increment number: "Cell 1" → "Cell 2"
3. If name contains "Cell" but no number, add " 2"
4. Update region name input field
5. Keep group and color the same

### Step 9: Handle Errors

1. If name is empty: show warning, don't create region
2. If name is duplicate: show error, don't create region
3. If region too small: show warning, don't create region
4. If region outside bounds: clamp to bounds or reject

### Step 10: Save to Undo Stack

1. Before creating region, save current state
2. Add to undo stack
3. Clear redo stack
4. Update undo/redo button states

### Step 11: Test Region Creation

1. Switch to Create mode
2. Click and drag to create region
3. Verify region appears with correct color
4. Verify region name is correct
5. Verify coordinates are correct (check info panel)
6. Test auto-increment
7. Test error handling (duplicate name, empty name)

## Acceptance Criteria

- [ ] Users can create regions by clicking and dragging
- [ ] Temporary rectangle shows during drag (red outline)
- [ ] Region is created on mouse release
- [ ] All three coordinate systems are populated correctly
- [ ] Region name auto-increments if pattern detected
- [ ] Errors are handled gracefully
- [ ] State is saved to undo stack

## Code Structure

```cpp
class Canvas : public QWidget {
    enum MouseMode { Create, Select };
    
    MouseMode mouseMode;
    bool isCreating;
    QPointF startPos;
    QPointF currentPos;
    QGraphicsRectItem* tempRect;
    
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    
private:
    void startRegionCreation(const QPointF& pos);
    void updateRegionCreation(const QPointF& pos);
    void finishRegionCreation(const QPointF& pos);
    void drawTempRectangle(const QRectF& rect);
};
```

## Notes

- Use canvas coordinates for mouse events
- Convert to image/normalized only when finalizing
- Show visual feedback during drag (temporary rectangle)
- Validate input before creating region
- Keep UI responsive during creation

