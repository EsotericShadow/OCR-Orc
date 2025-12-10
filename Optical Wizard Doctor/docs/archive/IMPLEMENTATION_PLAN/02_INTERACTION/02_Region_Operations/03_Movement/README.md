# Region Movement - Implementation Steps

## Objective

Enable users to move selected regions by dragging them on the canvas.

## Prerequisites

- Region selection working
- Mouse event handling ready
- Coordinate system transformations working
- DocumentState ready

## Steps

### Step 1: Detect Drag Start

1. In Canvas::mousePressEvent(), in Select mode:
2. Check if click is on a selected region
3. Store initial mouse position
4. Store initial region positions
5. Set dragging flag to true

### Step 2: Handle Mouse Drag

1. In Canvas::mouseMoveEvent(), if dragging:
2. Calculate delta: current position - start position
3. For each selected region:
   - Calculate new position: initial + delta
   - Update all three coordinate systems:
     - Canvas coordinates (immediate)
     - Image coordinates (recalculate)
     - Normalized coordinates (recalculate)
4. Redraw canvas with new positions

### Step 3: Handle Mouse Release

1. In Canvas::mouseReleaseEvent(), if dragging:
2. Finalize new positions
3. Update DocumentState
4. Save state to undo stack
5. Clear dragging flag
6. Redraw canvas

### Step 4: Constrain Movement (Optional)

1. Keep regions within canvas bounds
2. Or allow movement with visual feedback
3. Snap to grid (optional feature)

### Step 5: Update All Coordinate Systems

1. When moving, update:
   - Canvas coordinates (for display)
   - Image coordinates (recalculate from canvas)
   - Normalized coordinates (recalculate from image)
2. Ensure all systems stay synchronized

## Acceptance Criteria

- [ ] Users can drag selected regions
- [ ] All selected regions move together
- [ ] Movement is smooth (real-time)
- [ ] All coordinate systems stay synchronized
- [ ] Undo stack is updated
- [ ] Visual feedback is immediate

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
