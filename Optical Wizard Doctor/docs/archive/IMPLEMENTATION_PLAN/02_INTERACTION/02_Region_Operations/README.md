# Region Operations - Implementation Steps

## Objective

Enable users to create, select, move, and resize regions on the canvas.

## Prerequisites

- Canvas widget created
- Region rendering working
- Mouse event handling set up
- DocumentState ready

## Sub-Projects

1. **Creation** - Create regions by clicking and dragging
2. **Selection** - Select regions (single and multi-select)
3. **Movement** - Move selected regions by dragging
4. **Resize** - Resize regions using handles

## Order of Implementation

Complete these in order:
1. Creation (foundation)
2. Selection (needed for move/resize)
3. Movement (builds on selection)
4. Resize (most complex)

## Steps

### Step 1: Set Up Mouse Event Handlers

1. Override mouse event handlers in Canvas:
   - `mousePressEvent()`
   - `mouseMoveEvent()`
   - `mouseReleaseEvent()`
2. Track mouse state (pressed, dragging, etc.)
3. Convert screen coordinates to canvas coordinates

### Step 2: Implement Mode Switching

1. Add `mouseMode` enum: Create, Select
2. Add mode switching UI (toolbar buttons)
3. Update cursor based on mode
4. Handle mode-specific mouse events

### Step 3: Implement Region Creation

1. In Create mode, on mouse press: store start position
2. On mouse drag: draw temporary rectangle
3. On mouse release: create region with coordinates
4. Validate region (minimum size, within bounds)
5. Add to DocumentState

### Step 4: Implement Region Selection

1. In Select mode, on mouse press: find region at click position
2. Single click: select region (deselect others)
3. Command+Click: toggle multi-select
4. Box selection: drag to select multiple regions
5. Update selected_regions set

### Step 5: Implement Region Movement

1. On mouse press over selected region: start drag
2. On mouse move: calculate delta, move all selected regions
3. Update all three coordinate systems
4. On mouse release: finalize position, save to undo stack

### Step 6: Implement Region Resize

1. Detect resize handle click (8 handles per region)
2. On mouse drag: update region size based on handle
3. Update all three coordinate systems
4. Maintain aspect ratio (optional, based on handle)
5. On mouse release: finalize size

## Acceptance Criteria

- [ ] Users can create regions by clicking and dragging
- [ ] Users can select regions (single and multi)
- [ ] Users can move selected regions
- [ ] Users can resize regions using handles
- [ ] All coordinate systems stay synchronized
- [ ] Visual feedback is immediate

## Next Steps

After completing Region Operations, proceed to Navigation for zoom and pan functionality.
