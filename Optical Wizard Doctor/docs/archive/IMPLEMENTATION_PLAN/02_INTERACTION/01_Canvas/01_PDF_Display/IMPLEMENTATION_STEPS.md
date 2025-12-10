# PDF Display - Implementation Steps

## Objective

Display a PDF document (first page) as an image on the canvas.

## Prerequisites

- Poppler installed and working
- Canvas widget created
- DocumentState model ready

## Steps

### Step 1: Create PdfLoader Utility

1. Create `src/utils/PdfLoader.h` and `.cpp`
2. Implement function: `QImage loadPdfFirstPage(const QString& filePath, int dpi = 150)`
3. Use Poppler to:
   - Open PDF file
   - Get first page
   - Render to image at specified DPI
   - Return QImage

### Step 2: Handle PDF Loading Errors

1. Check if file exists
2. Check if file is valid PDF
3. Return empty QImage on error
4. Log error messages

### Step 3: Create Canvas Widget

1. Create `src/ui/Canvas.h` and `.cpp`
2. Inherit from `QWidget`
3. Override `paintEvent()` for custom drawing
4. Add member variable for document image

### Step 4: Implement Image Display

1. In `paintEvent()`:
   - Calculate scale to fit canvas (maintain aspect ratio)
   - Calculate centered position
   - Draw image using QPainter
   - Draw shadow effect (5px offset, grey)

### Step 5: Handle Canvas Resize

1. Override `resizeEvent()`
2. Recalculate image position and scale
3. Trigger repaint
4. Update scroll region if needed

### Step 6: Add Scroll Support

1. Make canvas scrollable (use QScrollArea)
2. Set scroll region based on image size
3. Handle scrollbar visibility

### Step 7: Integrate with MainWindow

1. Add Canvas widget to MainWindow layout
2. Connect file loading to PDF loader
3. Update canvas when PDF loads
4. Test PDF loading

### Step 8: Add Loading Indicator

1. Show loading message while PDF loads
2. Use QProgressDialog or status message
3. Handle loading in background thread (optional, for large PDFs)

### Step 9: Store Document State

1. Update DocumentState when PDF loads
2. Store image, dimensions, aspect ratio
3. Clear regions when new PDF loads

### Step 10: Test PDF Display

1. Test with various PDF files
2. Test with different sizes
3. Test resize behavior
4. Test scroll behavior
5. Verify image quality

## Acceptance Criteria

- [ ] PDF loads and displays correctly
- [ ] Image is centered and scaled properly
- [ ] Aspect ratio is maintained
- [ ] Canvas is scrollable for large images
- [ ] Shadow effect is visible
- [ ] Errors are handled gracefully
- [ ] Loading is responsive

## Code Structure

```cpp
class Canvas : public QWidget {
    Q_OBJECT
    
public:
    explicit Canvas(QWidget *parent = nullptr);
    void loadPdf(const QString& filePath);
    void setImage(const QImage& image);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private:
    QImage documentImage;
    double scaleFactor;
    QPointF imageOffset;
    void calculateLayout();
};
```

## Notes

- Use high-quality image rendering (smooth scaling)
- Consider caching rendered image
- Handle memory for large images
- Test with various PDF formats

