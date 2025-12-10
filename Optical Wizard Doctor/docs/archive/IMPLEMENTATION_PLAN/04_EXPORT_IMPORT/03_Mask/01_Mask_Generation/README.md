# Mask Generation - Implementation Steps

## Objective

Generate black/white mask images for OCR preprocessing.

## Prerequisites

- PDF loaded (image available)
- Regions defined with image coordinates
- Export functionality ready

## Steps

### Step 1: Create Mask Image

1. Get image dimensions from DocumentState
2. Create QImage with same dimensions
3. Fill with black (RGB: 0, 0, 0)

### Step 2: Draw White Rectangles

1. For each region:
2. Get image coordinates (not canvas!)
3. Draw white rectangle (RGB: 255, 255, 255)
4. Use QPainter to draw on mask image

### Step 3: Save Mask Image

1. Save in requested format (PNG, JPEG, TIFF)
2. Use QImage::save()
3. Handle file errors

### Step 4: Save Coordinate JSON

1. Automatically save JSON file alongside mask
2. Same base name, .json extension
3. Use JsonExporter to save coordinates

### Step 5: Add Export UI

1. "Export Mask" button
2. Show file save dialog
3. Call MaskGenerator::generate()
4. Show success message

## Acceptance Criteria

- [ ] Mask image is created correctly
- [ ] Dimensions match original image
- [ ] Background is black
- [ ] Regions are white rectangles
- [ ] Coordinate JSON is saved
- [ ] File formats work

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
