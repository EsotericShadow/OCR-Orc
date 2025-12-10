# Mask - Implementation Steps

## Objective

Generate mask images that black out everything except labeled regions for OCR preprocessing.

## Prerequisites

- PDF loaded and displayed
- Regions defined
- Image coordinates available
- Qt QImage support

## Sub-Projects

1. **Mask Generation** - Create black/white mask images

## Mask Specification

- **Format**: RGB image
- **Dimensions**: Same as original PDF image
- **Background**: Black (0, 0, 0)
- **Regions**: White rectangles (255, 255, 255)

## Steps

### Step 1: Create MaskGenerator Class

1. Create `src/export/MaskGenerator.h` and `.cpp`
2. Create class `MaskGenerator`
3. Add method: `static void generate(const DocumentState& state, const QString& filePath)`

### Step 2: Get Image Dimensions

1. Get image size from DocumentState
2. Use original image dimensions (not canvas size)
3. Store width and height

### Step 3: Create Black Image

1. Create QImage:
   ```cpp
   QImage mask(imageWidth, imageHeight, QImage::Format_RGB32);
   mask.fill(QColor(0, 0, 0));  // Black
   ```
2. Same dimensions as original image

### Step 4: Draw White Rectangles

1. Create QPainter for mask image
2. For each region:
   - Get image coordinates (not canvas)
   - Draw white rectangle:
     ```cpp
     painter.fillRect(x1, y1, x2-x1, y2-y1, QColor(255, 255, 255));
     ```
3. Ensure coordinates are in order (x1 < x2, y1 < y2)

### Step 5: Save Mask Image

1. Save in requested format (PNG, JPEG, TIFF)
2. Use QImage::save()
3. Handle file errors

### Step 6: Save Coordinate JSON

1. Automatically save JSON file alongside mask
2. Same base name, .json extension
3. Use JsonExporter to save coordinates
4. Ensures mask and coordinates stay together

### Step 7: Add Mask Export UI

1. "Export Mask" button
2. Show file save dialog (image formats):
   ```cpp
   "PNG files (*.png);;JPEG files (*.jpg);;TIFF files (*.tiff);;All files (*.*)"
   ```
3. Call MaskGenerator::generate()
4. Show success message with both file paths

### Step 8: Handle Mask Errors

1. Check image is loaded
2. Check regions exist
3. Handle file write errors
4. Show clear error messages

### Step 9: Test Mask Generation

1. Create test regions
2. Export mask image
3. Verify image is black with white rectangles
4. Verify dimensions match original
5. Verify coordinates JSON is saved
6. Test with various region counts
7. Test mask overlay in OCR (if possible)

## Acceptance Criteria

- [ ] Mask image is created correctly
- [ ] Dimensions match original image
- [ ] Background is black
- [ ] Regions are white rectangles
- [ ] Coordinate JSON is saved alongside
- [ ] File formats work (PNG, JPEG, TIFF)
- [ ] Errors are handled gracefully

## Code Structure

```cpp
class MaskGenerator {
public:
    static void generate(const DocumentState& state, const QString& filePath);
    
private:
    static QImage createMaskImage(const DocumentState& state);
    static void drawRegionRectangles(QPainter& painter, const DocumentState& state);
};
```

## Notes

- Use image coordinates, not canvas coordinates
- Mask dimensions must match original exactly
- Save coordinate JSON automatically
- Test mask with OCR software if available
