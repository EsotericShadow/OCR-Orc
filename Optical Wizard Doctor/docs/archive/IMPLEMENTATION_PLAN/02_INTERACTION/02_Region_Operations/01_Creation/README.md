# Region Creation - Implementation Steps

## Objective

Enable users to create regions by clicking and dragging on the canvas in Create mode.

## Prerequisites

- Canvas widget with mouse event handling
- Create mode implemented
- DocumentState ready
- Coordinate system working

## Steps

### Step 1: Handle Mouse Press in Create Mode

1. In Canvas::mousePressEvent(), check if mode is Create
2. Store start position (canvas coordinates)
3. Set dragging flag to true
4. Clear any existing selection

### Step 2: Handle Mouse Drag

1. In Canvas::mouseMoveEvent(), if dragging in Create mode:
2. Calculate current rectangle (start to current position)
3. Draw temporary rectangle on canvas
4. Update visual feedback

### Step 3: Handle Mouse Release

1. In Canvas::mouseReleaseEvent(), if dragging in Create mode:
2. Calculate final rectangle coordinates
3. Validate region (minimum size, within bounds)
4. Convert canvas coordinates to normalized
5. Create RegionData with auto-incremented name
6. Add to DocumentState
7. Save to undo stack
8. Redraw canvas

### Step 4: Auto-Increment Region Names

1. Generate name: "Region 1", "Region 2", etc.
2. Check for existing names
3. Find next available number

## Acceptance Criteria

- [ ] Users can create regions by clicking and dragging
- [ ] Temporary rectangle shows during drag
- [ ] Regions are created with valid coordinates
- [ ] Region names auto-increment
- [ ] Minimum size validation works
- [ ] Undo stack is updated

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
