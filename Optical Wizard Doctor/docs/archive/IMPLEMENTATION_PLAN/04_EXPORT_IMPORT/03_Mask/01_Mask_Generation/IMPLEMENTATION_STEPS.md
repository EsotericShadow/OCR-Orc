# Mask Generation - Implementation Steps

## Objective

Generate black/white mask images for OCR preprocessing.

## Prerequisites

- PDF loaded (image available)
- Regions defined with image coordinates
- Qt QImage and QPainter support

## Mask Specification

- **Type**: RGB image
- **Dimensions**: Exact match to original image
- **Background**: Black (RGB: 0, 0, 0)
- **Regions**: White rectangles (RGB: 255, 255, 255)
- **Purpose**: Hide everything except labeled cells for OCR

## Steps

### Step 1: Create MaskGenerator Class

1. Create `src/export/MaskGenerator.h` and `.cpp`
2. Create class `MaskGenerator`
3. Add static method: `static void generate(const DocumentState& state, const QString& filePath)`
4. Validate inputs (image exists, regions exist)

### Step 2: Get Image Dimensions

1. Get image from DocumentState:
   ```cpp
   QImage image = state.image;
   int width = image.width();
   int height = image.height();
   ```
2. Use original image dimensions (not scaled/display size)
3. Validate dimensions are valid (> 0)

### Step 3: Create Black Mask Image

1. Create new QImage:
   ```cpp
   QImage mask(width, height, QImage::Format_RGB32);
   ```
2. Fill with black:
   ```cpp
   mask.fill(QColor(0, 0, 0));
   ```
3. This creates the base (everything hidden)

### Step 4: Draw White Rectangles for Regions

1. Create QPainter for mask:
   ```cpp
   QPainter painter(&mask);
   painter.setRenderHint(QPainter::Antialiasing, false);  // Sharp edges
   ```
2. For each region in DocumentState:
   - Get image coordinates (not canvas!)
   - Ensure coordinates are in order:
     ```cpp
     int x1 = std::min(region.imageCoords.x1, region.imageCoords.x2);
     int y1 = std::min(region.imageCoords.y1, region.imageCoords.y2);
     int x2 = std::max(region.imageCoords.x1, region.imageCoords.x2);
     int y2 = std::max(region.imageCoords.y1, region.imageCoords.y2);
     ```
   - Draw white rectangle:
     ```cpp
     painter.fillRect(x1, y1, x2-x1, y2-y1, QColor(255, 255, 255));
     ```
3. White rectangles = visible areas for OCR

### Step 5: Validate Coordinates

1. Check coordinates are within image bounds:
   ```cpp
   if (x1 < 0 || y1 < 0 || x2 > width || y2 > height) {
       // Clamp to bounds or skip region
   }
   ```
2. Clamp coordinates to image dimensions if needed
3. Ensure rectangle has positive size

### Step 6: Save Mask Image

1. Determine format from file extension:
   - `.png` → PNG format
   - `.jpg` / `.jpeg` → JPEG format
   - `.tiff` → TIFF format
2. Save image:
   ```cpp
   if (!mask.save(filePath)) {
       // Handle error
   }
   ```
3. Handle file write errors

### Step 7: Save Coordinate JSON Alongside

1. Get base file path (without extension)
2. Create JSON path: `basePath + ".json"`
3. Use JsonExporter to save coordinates:
   ```cpp
   QString jsonPath = QFileInfo(filePath).baseName() + ".json";
   JsonExporter::export(state, jsonPath);
   ```
4. Ensures mask and coordinates stay together

### Step 8: Add Mask Export UI

1. "Export Mask" button in MainWindow
2. Show file save dialog:
   ```cpp
   QString filePath = QFileDialog::getSaveFileName(
       this, "Export Mask Image", "",
       "PNG files (*.png);;"
       "JPEG files (*.jpg);;"
       "TIFF files (*.tiff);;"
       "All files (*.*)");
   ```
3. If file selected:
   - Call MaskGenerator::generate()
   - Show success message with both file paths

### Step 9: Handle Errors

1. **No image loaded**: Show error "Load PDF first"
2. **No regions**: Show error "Define regions first"
3. **File write error**: Show error, preserve data
4. **Invalid dimensions**: Validate before creating mask

### Step 10: Optimize Mask Generation

1. For many regions, batch rectangle drawing
2. Use efficient QPainter operations
3. Consider memory usage for large images

### Step 11: Test Mask Generation

1. Create test regions
2. Export mask image
3. Verify:
   - Image is black background
   - White rectangles at correct positions
   - Dimensions match original
   - Coordinate JSON is saved
4. Open mask in image viewer
5. Test mask with OCR software (if available)
6. Verify mask improves OCR accuracy

### Step 12: Add Mask Preview (Optional)

1. Show mask preview in canvas when mask overlay enabled
2. Or show mask in separate window
3. Helps users verify mask before export

## Acceptance Criteria

- [ ] Mask image is created correctly
- [ ] Dimensions exactly match original image
- [ ] Background is pure black
- [ ] Regions are pure white rectangles
- [ ] Coordinate JSON is saved alongside
- [ ] All image formats work (PNG, JPEG, TIFF)
- [ ] Errors are handled gracefully
- [ ] Performance is acceptable for large images

## Code Structure

```cpp
class MaskGenerator {
public:
    static void generate(const DocumentState& state, const QString& filePath);
    
private:
    static QImage createMaskImage(const DocumentState& state);
    static void drawRegionRectangles(QPainter& painter, 
                                     const QMap<QString, RegionData>& regions,
                                     int imageWidth, int imageHeight);
    static void validateAndClampCoordinates(int& x1, int& y1, 
                                            int& x2, int& y2,
                                            int width, int height);
};
```

## Notes

- **Critical**: Use image coordinates, not canvas coordinates
- Mask dimensions must match original exactly
- White = visible, Black = hidden
- Save coordinate JSON automatically
- Test with actual OCR to verify effectiveness

