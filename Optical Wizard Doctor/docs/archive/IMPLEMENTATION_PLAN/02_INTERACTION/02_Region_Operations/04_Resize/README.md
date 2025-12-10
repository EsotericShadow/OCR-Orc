# Region Resize - Implementation Steps

## Objective

Enable users to resize regions by dragging resize handles.

## Prerequisites

- Region selection working
- Resize handles rendering working
- Mouse event handling ready
- Coordinate system transformations working

## Steps

### Step 1: Detect Resize Handle Click

1. In Canvas::mousePressEvent(), in Select mode:
2. Check if click is on a resize handle
3. Identify which handle (8 handles: corners + midpoints)
4. Store initial mouse position
5. Store initial region coordinates
6. Set resizing flag and handle type

### Step 2: Calculate Handle Positions

1. For each selected region, calculate 8 handle positions:
   - Top-left, top-mid, top-right
   - Mid-left, mid-right
   - Bottom-left, bottom-mid, bottom-right
2. Handle size: 8x8 pixels
3. Check if click is within handle bounds

### Step 3: Handle Mouse Drag

1. In Canvas::mouseMoveEvent(), if resizing:
2. Calculate new size based on handle and mouse position
3. Update region coordinates:
   - For corner handles: update two coordinates
   - For edge handles: update one coordinate
4. Maintain minimum size (e.g., 10x10 pixels)
5. Update all three coordinate systems
6. Redraw canvas

### Step 4: Handle Mouse Release

1. In Canvas::mouseReleaseEvent(), if resizing:
2. Finalize new size
3. Update DocumentState
4. Save state to undo stack
5. Clear resizing flag
6. Redraw canvas

### Step 5: Update Coordinate Systems

1. When resizing, update:
   - Canvas coordinates (for display)
   - Image coordinates (recalculate from canvas)
   - Normalized coordinates (recalculate from image)
2. Ensure all systems stay synchronized

## Acceptance Criteria

- [ ] Users can resize regions using handles
- [ ] All 8 handles work correctly
- [ ] Minimum size is enforced
- [ ] All coordinate systems stay synchronized
- [ ] Undo stack is updated
- [ ] Visual feedback is immediate

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
