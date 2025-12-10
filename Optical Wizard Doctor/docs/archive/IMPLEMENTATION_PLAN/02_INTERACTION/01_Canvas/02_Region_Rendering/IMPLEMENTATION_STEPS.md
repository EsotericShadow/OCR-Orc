# Region Rendering - Implementation Steps

## Objective

Render regions on the canvas with proper visual styling (normal, hovered, selected states).

## Prerequisites

- PDF display working
- Canvas widget created
- DocumentState with regions
- Coordinate system implemented

## Visual States

1. **Normal**: 2px border, no fill, colored outline
2. **Hovered**: 3px border, 10% opacity fill, colored outline
3. **Selected**: 4px border, 20% opacity fill, colored outline, resize handles

## Steps

### Step 1: Create Region Rendering Function

1. Add method `void renderRegions(QPainter& painter)` to Canvas class
2. Get all regions from DocumentState
3. Iterate through each region
4. Determine visual state (normal/hovered/selected)

### Step 2: Implement Coordinate Conversion

1. For each region, get normalized coordinates
2. Convert to canvas coordinates using CoordinateSystem
3. Account for zoom level and pan offset
4. Store canvas coordinates for rendering

### Step 3: Draw Normal State Regions

1. Get region color from RegionData
2. Set pen: 2px width, region color
3. Set brush: no fill
4. Draw rectangle using canvas coordinates
5. Draw label above region (8pt font, normal weight)

### Step 4: Draw Hovered State

1. Check if region matches hovered_region
2. Set pen: 3px width, region color
3. Set brush: 10% opacity fill (use QColor with alpha)
4. Draw rectangle
5. Draw label: 10pt font, normal weight

### Step 5: Draw Selected State

1. Check if region is in selected_regions set
2. Set pen: 4px width, region color
3. Set brush: 20% opacity fill
4. Draw rectangle
5. Draw label: 10pt font, bold weight
6. Draw resize handles (if single selection)

### Step 6: Implement Resize Handles

1. Create function `void drawResizeHandles(QPainter& painter, const QRect& rect)`
2. Draw 8 handles (4 corners, 4 edges)
3. Handle size: 8x8 pixels
4. Style: white fill, black outline, 1px border
5. Only draw for single selected region

### Step 7: Implement Region Labels

1. Create function `void drawRegionLabel(QPainter& painter, const QString& name, const QRect& rect, bool isSelected)`
2. Position: above region, centered horizontally
3. Font size: 8pt (normal) or 10pt (selected/hovered)
4. Font weight: normal or bold
5. Color: match region color

### Step 8: Implement Color Mapping

1. Create color map: `QMap<QString, QColor>`
2. Map color names to hex values:
   - "blue": #0066ff
   - "red": #ff0000
   - "green": #00cc00
   - "yellow": #ffcc00
   - "purple": #9900cc
   - "orange": #ff6600
   - "cyan": #00cccc
3. Default to blue if color not found

### Step 9: Optimize Rendering

1. Only render visible regions (viewport culling)
2. Cache region canvas coordinates
3. Update coordinates only on zoom/pan change
4. Use QPainter's high-quality rendering

### Step 10: Handle Mask Overlay

1. If mask enabled, draw semi-transparent overlay
2. Create grey overlay (128, 128, 128, 200 alpha)
3. Draw white rectangles for regions (transparent holes)
4. Use QPainter's composition mode

### Step 11: Test Region Rendering

1. Create multiple regions with different colors
2. Test normal state rendering
3. Test hover state (move mouse over region)
4. Test selected state (click region)
5. Test resize handles appear for single selection
6. Test labels display correctly
7. Test mask overlay

## Acceptance Criteria

- [ ] Regions render with correct colors
- [ ] Visual states work correctly (normal/hovered/selected)
- [ ] Resize handles appear for single selection
- [ ] Labels display above regions
- [ ] Mask overlay works when enabled
- [ ] Rendering is smooth (no flicker)
- [ ] Performance is acceptable with 100+ regions

## Code Structure

```cpp
class Canvas : public QWidget {
    void renderRegions(QPainter& painter);
    void drawResizeHandles(QPainter& painter, const QRect& rect);
    void drawRegionLabel(QPainter& painter, const QString& name, 
                        const QRect& rect, bool isSelected);
    QColor getRegionColor(const QString& colorName) const;
    
    // State
    QString hoveredRegion;
    QSet<QString> selectedRegions;
    bool maskEnabled;
};
```

## Notes

- Use QPainter's antialiasing for smooth rendering
- Cache color map for performance
- Update hover state on mouse move
- Redraw only when necessary (dirty regions)
- Test with many regions to ensure performance

