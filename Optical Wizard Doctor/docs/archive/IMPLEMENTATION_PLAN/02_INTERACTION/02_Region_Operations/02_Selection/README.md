# Region Selection - Implementation Steps

## Objective

Enable users to select regions (single and multi-select) for editing operations.

## Prerequisites

- Canvas with mouse event handling
- Select mode implemented
- Region rendering working
- DocumentState ready

## Steps

### Step 1: Implement Single Click Selection

1. In Canvas::mousePressEvent(), in Select mode:
2. Find region at click position (hit testing)
3. If region found:
   - Deselect all other regions
   - Select clicked region
   - Update selected_regions set
4. Redraw canvas to show selection

### Step 2: Implement Multi-Select (Ctrl/Cmd+Click)

1. Check if Ctrl (Windows/Linux) or Cmd (macOS) is pressed
2. If modifier pressed:
   - Toggle selection of clicked region
   - Keep other selections
3. If no modifier:
   - Single selection (deselect others)

### Step 3: Implement Box Selection

1. On Shift+Drag in Select mode:
2. Draw selection box rectangle
3. On mouse release:
   - Find all regions intersecting selection box
   - Add to selected_regions
4. Visual feedback during drag

### Step 4: Implement Hit Testing

1. Create method `RegionData* findRegionAt(QPoint pos)`
2. Check each region's canvas coordinates
3. Return region if point is inside rectangle
4. Handle overlapping regions (select topmost)

### Step 5: Update Visual Feedback

1. Selected regions show thicker border
2. Selected regions show resize handles
3. Update region listbox selection
4. Update info display

## Acceptance Criteria

- [ ] Single click selects region
- [ ] Ctrl/Cmd+Click toggles selection
- [ ] Box selection works (Shift+Drag)
- [ ] Multiple regions can be selected
- [ ] Visual feedback is clear
- [ ] Hit testing is accurate

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
