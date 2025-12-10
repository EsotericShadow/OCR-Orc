# Zoom - Implementation Steps

## Objective

Implement zoom in, zoom out, and reset zoom functionality.

## Prerequisites

- Canvas widget created
- PDF display working
- Coordinate system ready

## Steps

### Step 1: Add Zoom State

1. Add `double zoomLevel` to Canvas or DocumentState
2. Range: 0.2x (20%) to 5.0x (500%)
3. Default: 1.0x (100%)
4. Update scale_factor based on zoomLevel

### Step 2: Implement Zoom In

1. Create method `void zoomIn()`
2. Multiply zoomLevel by 1.2
3. Clamp to maximum 5.0x
4. Recalculate scale_factor
5. Recalculate canvas coordinates for all regions
6. Redraw canvas
7. Update zoom percentage display

### Step 3: Implement Zoom Out

1. Create method `void zoomOut()`
2. Divide zoomLevel by 1.2
3. Clamp to minimum 0.2x
4. Recalculate scale_factor
5. Recalculate canvas coordinates for all regions
6. Redraw canvas
7. Update zoom percentage display

### Step 4: Implement Reset Zoom

1. Create method `void resetZoom()`
2. Set zoomLevel to 1.0
3. Reset pan offsets to 0
4. Recalculate scale_factor
5. Recalculate canvas coordinates
6. Center image on canvas
7. Redraw canvas
8. Update zoom percentage display

### Step 5: Add Mouse Wheel Zoom

1. In Canvas::wheelEvent():
2. Check if Ctrl/Cmd is pressed
3. If pressed: zoom in/out based on wheel delta
4. Zoom toward mouse cursor position
5. Update zoom level and redraw

### Step 6: Update Zoom Display

1. Update zoom percentage label in toolbar
2. Format as integer: "150%" for 1.5x
3. Update on every zoom change

## Acceptance Criteria

- [ ] Zoom in works (up to 500%)
- [ ] Zoom out works (down to 20%)
- [ ] Reset zoom works
- [ ] Mouse wheel zoom works
- [ ] Zoom range is enforced
- [ ] Regions stay aligned at all zoom levels
- [ ] Zoom display updates correctly

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
