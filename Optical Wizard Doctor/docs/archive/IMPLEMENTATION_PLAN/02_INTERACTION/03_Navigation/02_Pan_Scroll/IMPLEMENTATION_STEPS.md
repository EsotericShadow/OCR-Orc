# Pan/Scroll - Implementation Steps

## Objective

Enable users to pan and scroll the canvas to navigate large documents.

## Prerequisites

- Canvas widget created
- PDF display working
- Zoom functionality working
- QScrollArea set up

## Scroll Methods

1. **Scrollbars**: Click and drag scrollbars
2. **Mouse Wheel**: Scroll vertically
3. **Shift+Scroll**: Scroll horizontally
4. **Pan**: Drag canvas when space is pressed (optional)

## Steps

### Step 1: Set Up Scroll Area

1. Wrap Canvas widget in QScrollArea
2. Set scroll area properties:
   - `setWidgetResizable(true)`
   - `setAlignment(Qt::AlignCenter)`
3. Add to MainWindow layout
4. Store reference to scroll area

### Step 2: Calculate Scroll Region

1. Create method `void updateScrollRegion()`
2. Calculate content size:
   - `contentWidth = imageWidth * scaleFactor`
   - `contentHeight = imageHeight * scaleFactor`
3. Set scroll region:
   ```cpp
   scrollArea->widget()->setMinimumSize(contentWidth, contentHeight);
   ```
4. Call after zoom or image load

### Step 3: Handle Vertical Scrolling

1. Override `wheelEvent()` in Canvas
2. If no modifier keys:
   - Get scroll delta
   - Scroll vertically: `scrollArea->verticalScrollBar()->setValue(...)`
   - Or use: `QApplication::sendEvent(scrollArea, event)`

### Step 4: Handle Horizontal Scrolling

1. In `wheelEvent()`, check for Shift modifier
2. If Shift+Scroll:
   - Scroll horizontally: `scrollArea->horizontalScrollBar()->setValue(...)`
3. Also handle Magic Mouse side-to-side scrolling

### Step 5: Update Scrollbars Visibility

1. Show scrollbars only when content exceeds viewport
2. Check if content size > viewport size
3. Set scrollbar policy:
   - `Qt::ScrollBarAsNeeded` (show when needed)
   - Or `Qt::ScrollBarAlwaysOn` (always show)

### Step 6: Maintain Image Position

1. After zoom, maintain scroll position relative to image
2. Calculate scroll position in image coordinates
3. After zoom, restore scroll position
4. Or center image on canvas

### Step 7: Handle Canvas Resize

1. On canvas resize, update scroll region
2. Maintain current scroll position if possible
3. Recalculate image position
4. Update region canvas coordinates

### Step 8: Implement Pan (Optional)

1. If Space+Drag:
   - Store start position
   - On drag, calculate delta
   - Scroll canvas by delta
   - Update scrollbar positions
2. Change cursor to "closed hand" during pan

### Step 9: Sync Scroll with Zoom

1. When zooming, update scroll region
2. Optionally maintain point under cursor
3. Or center image on canvas
4. Update scrollbar positions

### Step 10: Test Scrolling

1. Test vertical scrolling (mouse wheel)
2. Test horizontal scrolling (Shift+wheel)
3. Test scrollbars
4. Test with different zoom levels
5. Test with large images
6. Verify smooth scrolling

## Acceptance Criteria

- [ ] Canvas is scrollable
- [ ] Mouse wheel scrolls vertically
- [ ] Shift+wheel scrolls horizontally
- [ ] Scrollbars appear when needed
- [ ] Scrolling is smooth
- [ ] Scroll position maintained on zoom
- [ ] Works with large documents

## Code Structure

```cpp
class Canvas : public QWidget {
    QScrollArea* scrollArea;
    
    void updateScrollRegion();
    void centerImage();
    
protected:
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
};
```

## Notes

- Use QScrollArea for built-in scroll support
- Update scroll region on zoom/image change
- Test with various document sizes
- Ensure smooth scrolling performance

