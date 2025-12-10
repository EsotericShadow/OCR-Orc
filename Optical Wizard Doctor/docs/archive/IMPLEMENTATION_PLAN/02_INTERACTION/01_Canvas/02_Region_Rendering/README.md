# Region Rendering - Implementation Steps

## Objective

Render regions on the canvas with proper styling, labels, and visual feedback for different states.

## Prerequisites

- PDF display working
- Canvas widget created
- DocumentState with regions
- Coordinate system transformations ready

## Steps

### Step 1: Implement Region Rectangle Drawing

1. In Canvas::paintEvent(), iterate through all regions
2. Convert normalized coordinates to canvas coordinates
3. Draw rectangle using QPainter:
   ```cpp
   painter.setPen(QPen(regionColor, 2));
   painter.setBrush(Qt::NoBrush);
   painter.drawRect(x1, y1, x2-x1, y2-y1);
   ```
4. Use region's color from DocumentState

### Step 2: Draw Region Labels

1. For each region, draw name label
2. Position label at top-left of region
3. Use readable font and size
4. Ensure label is visible (contrast with background)

### Step 3: Implement Visual States

1. **Normal State**: Default region appearance
2. **Hovered State**: Highlighted border (brighter color)
3. **Selected State**: Thicker border, different color
4. Track hovered_region and selected_regions

### Step 4: Draw Resize Handles

1. For selected regions, draw 8 resize handles
2. Handles at corners and midpoints
3. Small squares (8x8 pixels)
4. Visible only when region is selected

### Step 5: Optimize Rendering

1. Only render visible regions (viewport culling)
2. Cache rendered elements if possible
3. Redraw only when necessary

## Acceptance Criteria

- [ ] Regions render with correct colors
- [ ] Labels are visible and readable
- [ ] Visual states work (normal, hovered, selected)
- [ ] Resize handles appear for selected regions
- [ ] Rendering is performant

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
