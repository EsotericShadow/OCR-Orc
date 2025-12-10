# Navigation - Implementation Steps

## Objective

Enable users to zoom and pan/scroll the document view for better navigation.

## Prerequisites

- Canvas widget created
- PDF display working
- Region rendering working

## Sub-Projects

1. **Zoom** - Zoom in/out/reset functionality
2. **Pan/Scroll** - Pan and scroll the canvas

## Order of Implementation

Complete these in order:
1. Zoom (affects pan calculations)
2. Pan/Scroll (builds on zoom)

## Steps

### Step 1: Set Up Zoom State

1. Add to Canvas or DocumentState:
   - `double zoomLevel` - Current zoom (1.0 = 100%)
   - Zoom range: 0.2x (20%) to 5.0x (500%)
   - Default: 1.0x (100%)

### Step 2: Implement Zoom In

1. Add method `void zoomIn()`
2. Multiply zoomLevel by 1.2
3. Clamp to maximum 5.0x
4. Recalculate scale factor
5. Recalculate canvas coordinates for all regions
6. Redraw canvas

### Step 3: Implement Zoom Out

1. Add method `void zoomOut()`
2. Divide zoomLevel by 1.2
3. Clamp to minimum 0.2x
4. Recalculate scale factor
5. Recalculate canvas coordinates for all regions
6. Redraw canvas

### Step 4: Implement Reset Zoom

1. Add method `void resetZoom()`
2. Set zoomLevel to 1.0
3. Reset pan offsets to 0
4. Recalculate scale factor
5. Recalculate canvas coordinates
6. Center image on canvas
7. Redraw canvas

### Step 5: Update Zoom Display

1. Update zoom percentage label in toolbar
2. Format as integer: "150%" for 1.5x
3. Update on every zoom change

### Step 6: Implement Pan/Scroll

1. Use QScrollArea for canvas
2. Set scroll region based on image size and zoom
3. Handle scrollbar visibility
4. Update scroll position on zoom

### Step 7: Add Keyboard Shortcuts

1. + or =: Zoom in
2. -: Zoom out
3. 0: Reset zoom
4. Command+Scroll: Zoom in/out

### Step 8: Test Navigation

1. Test zoom in/out
2. Test reset zoom
3. Test scroll behavior
4. Test with different zoom levels
5. Verify regions stay aligned

## Acceptance Criteria

- [ ] Users can zoom in/out
- [ ] Users can reset zoom
- [ ] Zoom range is enforced (20%-500%)
- [ ] Canvas is scrollable
- [ ] Regions stay aligned at all zoom levels
- [ ] Keyboard shortcuts work
- [ ] Performance is acceptable

## Next Steps

After completing Navigation, proceed to Phase 3: Organization.
