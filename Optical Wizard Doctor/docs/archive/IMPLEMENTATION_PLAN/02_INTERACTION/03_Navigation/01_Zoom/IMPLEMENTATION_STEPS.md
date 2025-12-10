# Zoom - Implementation Steps

## Objective

Implement zoom functionality: zoom in, zoom out, and reset zoom.

## Prerequisites

- Canvas widget created
- PDF display working
- Coordinate system implemented
- DocumentState ready

## Zoom Specifications

- **Range**: 0.2x (20%) to 5.0x (500%)
- **Default**: 1.0x (100%)
- **Step**: Multiply/divide by 1.2
- **Methods**: Buttons, keyboard shortcuts, Command+Scroll

## Steps

### Step 1: Add Zoom State to DocumentState

1. Add `double zoomLevel` member (default: 1.0)
2. Add zoom range constants:
   - `constexpr double MIN_ZOOM = 0.2`
   - `constexpr double MAX_ZOOM = 5.0`
   - `constexpr double ZOOM_STEP = 1.2`

### Step 2: Implement Zoom In

1. Create method `void zoomIn()` in Canvas or DocumentState
2. Multiply zoomLevel by ZOOM_STEP (1.2)
3. Clamp to MAX_ZOOM (5.0):
   ```cpp
   zoomLevel = std::min(zoomLevel * 1.2, 5.0);
   ```
4. Recalculate scale factor
5. Recalculate canvas coordinates for all regions
6. Trigger canvas redraw

### Step 3: Implement Zoom Out

1. Create method `void zoomOut()`
2. Divide zoomLevel by ZOOM_STEP (1.2)
3. Clamp to MIN_ZOOM (0.2):
   ```cpp
   zoomLevel = std::max(zoomLevel / 1.2, 0.2);
   ```
4. Recalculate scale factor
5. Recalculate canvas coordinates for all regions
6. Trigger canvas redraw

### Step 4: Implement Reset Zoom

1. Create method `void resetZoom()`
2. Set zoomLevel to 1.0
3. Reset pan offsets to 0
4. Recalculate scale factor
5. Recalculate image position (center on canvas)
6. Recalculate canvas coordinates for all regions
7. Trigger canvas redraw

### Step 5: Recalculate Scale Factor

1. Create method `void recalculateScaleFactor()`
2. Get canvas size
3. Get image size
4. Calculate base scale (fit to canvas):
   ```cpp
   baseScale = min(canvasWidth/imageWidth, canvasHeight/imageHeight, 1.0)
   ```
5. Apply zoom: `scaleFactor = baseScale * zoomLevel`
6. Store in DocumentState

### Step 6: Update Region Canvas Coordinates

1. When zoom changes, update all region canvas coordinates
2. Use CoordinateSystem::normalizedToCanvas()
3. Pass new scaleFactor and image offset
4. Update each region's canvas_coords
5. This keeps regions aligned with image

### Step 7: Update Zoom Display

1. Create method `void updateZoomDisplay()`
2. Calculate percentage: `int percent = zoomLevel * 100`
3. Update zoom label in toolbar: `zoomLabel->setText(QString("%1%").arg(percent))`
4. Call after every zoom change

### Step 8: Add Zoom Buttons

1. In toolbar, add:
   - "−" button (zoom out)
   - Zoom percentage label
   - "+" button (zoom in)
   - "Reset" button
2. Connect buttons to zoom methods
3. Add tooltips

### Step 9: Add Keyboard Shortcuts

1. Connect keyboard events:
   - `+` or `=` key: zoomIn()
   - `-` key: zoomOut()
   - `0` key: resetZoom()
2. Handle in Canvas keyPressEvent() or MainWindow

### Step 10: Implement Command+Scroll Zoom

1. Override `wheelEvent()` in Canvas
2. Check for Command/Ctrl modifier
3. If Command+Scroll:
   - Up/forward: zoomIn()
   - Down/back: zoomOut()
4. Works for both vertical and horizontal scroll

### Step 11: Handle Zoom Center Point

1. Optionally zoom toward mouse position
2. Calculate mouse position in image coordinates
3. After zoom, adjust pan offset to keep point under cursor
4. More intuitive zoom experience

### Step 12: Update Scroll Region

1. After zoom, update QScrollArea scroll region
2. Calculate new content size: `imageSize * scaleFactor`
3. Set scroll region: `scrollArea->setWidgetResizable(true)`
4. Update scrollbar visibility

### Step 13: Test Zoom

1. Test zoom in (multiple times, verify max)
2. Test zoom out (multiple times, verify min)
3. Test reset zoom
4. Test keyboard shortcuts
5. Test Command+Scroll
6. Verify regions stay aligned
7. Verify image quality at different zoom levels
8. Test performance with many regions

## Acceptance Criteria

- [ ] Zoom in works (up to 500%)
- [ ] Zoom out works (down to 20%)
- [ ] Reset zoom works (returns to 100%)
- [ ] Zoom range is enforced
- [ ] Regions stay aligned at all zoom levels
- [ ] Zoom display updates correctly
- [ ] Keyboard shortcuts work
- [ ] Command+Scroll works
- [ ] Performance is acceptable

## Code Structure

```cpp
class Canvas : public QWidget {
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void recalculateScaleFactor();
    void updateZoomDisplay();
    
protected:
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    
private:
    double zoomLevel;  // 1.0 = 100%
    static constexpr double MIN_ZOOM = 0.2;
    static constexpr double MAX_ZOOM = 5.0;
    static constexpr double ZOOM_STEP = 1.2;
};
```

## Notes

- Recalculate coordinates after every zoom change
- Keep scale factor synchronized with zoom level
- Test at extreme zoom levels
- Ensure smooth zoom experience
- Consider zoom animation (optional)

