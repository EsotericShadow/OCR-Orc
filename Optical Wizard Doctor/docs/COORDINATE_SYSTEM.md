# Coordinate System Specification

## Overview

OCR-Orc uses a **three-tier coordinate system** to ensure coordinates work across different document resolutions while maintaining accurate display and manipulation.

## Coordinate Systems

### 1. Normalized Coordinates (Source of Truth)

**Range:** 0.0 to 1.0 (percentages)  
**Type:** Float  
**Format:** [x1, y1, x2, y2]  
**Purpose:** Resolution-independent coordinate storage

**Characteristics:**
- Always in range 0.0-1.0
- Represents position as percentage of document dimensions
- Used for all exports
- Source of truth for coordinate calculations
- Works with any document resolution if aspect ratio matches

**Example:**
```json
{
  "normalized_coords": [0.25, 0.10, 0.35, 0.20]
}
```
This means the region is:
- X starts at 25% of document width
- Y starts at 10% of document height
- X ends at 35% of document width
- Y ends at 20% of document height

### 2. Image Coordinates (Pixel Positions)

**Range:** 0 to image_width/height (pixels)  
**Type:** Integer  
**Format:** [x1, y1, x2, y2]  
**Purpose:** Exact pixel positions in the original PDF image

**Characteristics:**
- Integer pixel coordinates
- Resolution-dependent
- Calculated from normalized: `pixel = normalized * image_dimension`
- Used for mask generation and OCR extraction
- Recalculated when image loads

**Calculation:**
```python
image_x = normalized_x * image_width
image_y = normalized_y * image_height
```

**Example:**
If image is 2000x3000 pixels:
```python
normalized = [0.25, 0.10, 0.35, 0.20]
image_coords = [
    0.25 * 2000,  # 500
    0.10 * 3000,  # 300
    0.35 * 2000,  # 700
    0.20 * 3000   # 600
]
# Result: [500, 300, 700, 600]
```

### 3. Canvas Coordinates (Display Pixels)

**Range:** 0 to canvas_width/height (display pixels)  
**Type:** Float  
**Format:** [x1, y1, x2, y2]  
**Purpose:** Screen positions for rendering and user interaction

**Characteristics:**
- Float display coordinates
- Accounts for zoom, pan, and image offset
- Recalculated on every zoom/pan change
- Used for drawing regions on canvas
- Used for mouse interaction

**Calculation:**
```python
canvas_x = (image_x * scale_factor) + image_x_offset
canvas_y = (image_y * scale_factor) + image_y_offset
```

Where:
- `scale_factor = base_scale * zoom_level`
- `base_scale = min(canvas_width/image_width, canvas_height/image_height, 1.0)`
- `image_x_offset` and `image_y_offset` are the image's position on canvas

**Example:**
If image is 2000x3000, canvas is 1000x800, zoom is 1.5x:
```python
base_scale = min(1000/2000, 800/3000, 1.0) = 0.2667
scale_factor = 0.2667 * 1.5 = 0.4
image_x_offset = (1000 - 2000*0.4) / 2 = 100

image_coords = [500, 300, 700, 600]
canvas_coords = [
    500 * 0.4 + 100,  # 300
    300 * 0.4 + 50,   # 170
    700 * 0.4 + 100,  # 380
    600 * 0.4 + 50    # 290
]
```

## Coordinate Transformation Rules

### Normalized → Image
```python
image_x = int(normalized_x * image_width)
image_y = int(normalized_y * image_height)
```

### Image → Normalized
```python
normalized_x = image_x / image_width
normalized_y = image_y / image_height
```

### Image → Canvas
```python
canvas_x = (image_x * scale_factor) + image_x_offset
canvas_y = (image_y * scale_factor) + image_y_offset
```

### Canvas → Image (for user input)
```python
image_x = int((canvas_x - image_x_offset) / scale_factor)
image_y = int((canvas_y - image_y_offset) / scale_factor)
```

### Normalized → Canvas (via Image)
```python
# First: Normalized → Image
image_x = normalized_x * image_width
image_y = normalized_y * image_height

# Then: Image → Canvas
canvas_x = (image_x * scale_factor) + image_x_offset
canvas_y = (image_y * scale_factor) + image_y_offset
```

## Coordinate Synchronization

### When Region is Created
1. User draws rectangle on canvas (canvas coordinates)
2. Convert canvas → image coordinates
3. Convert image → normalized coordinates
4. Store all three coordinate systems

### When Region is Moved
1. Calculate movement delta in canvas coordinates
2. Convert delta to image coordinates: `delta_image = delta_canvas / scale_factor`
3. Update image coordinates
4. Convert delta to normalized: `delta_normalized = delta_image / image_dimension`
5. Update normalized coordinates
6. Recalculate canvas coordinates from normalized

### When Region is Resized
1. User drags handle (new canvas coordinates)
2. Convert new canvas coordinates → image coordinates
3. Convert image → normalized coordinates
4. Update all three coordinate systems

### When Zoom Changes
1. Recalculate `scale_factor` based on new zoom level
2. Recalculate `image_x_offset` and `image_y_offset`
3. For each region: Recalculate canvas coordinates from normalized
4. Redraw all regions with new canvas coordinates

### When Image Loads
1. For each region: Calculate image coordinates from normalized
2. Calculate canvas coordinates from normalized
3. Display regions on canvas

## Coordinate Validation

### Normalized Coordinate Validation
- **Range Check:** All values must be between 0.0 and 1.0
- **Order Check:** x1 < x2 and y1 < y2 (after normalization)
- **Type Check:** All values must be floats

### Image Coordinate Validation
- **Range Check:** All values must be within image dimensions (0 to width/height)
- **Type Check:** All values must be integers
- **Order Check:** x1 < x2 and y1 < y2

### Canvas Coordinate Validation
- **Type Check:** All values must be floats
- **Order Check:** x1 < x2 and y1 < y2 (after normalization)

## Export Format

**All exports use normalized coordinates only.**

This ensures:
- Coordinates work with any resolution
- Coordinates are aspect-ratio dependent, not pixel-dependent
- Same coordinates work for 300 DPI and 600 DPI scans (if aspect ratio matches)

**Export Structure:**
```json
{
  "regions": {
    "Region Name": {
      "normalized_coords": {
        "x1": 0.25,
        "y1": 0.10,
        "x2": 0.35,
        "y2": 0.20
      }
    }
  }
}
```

## Usage in OCR Pipeline

1. **Load coordinates** (normalized) from export file
2. **Load document image** (any resolution)
3. **Calculate image coordinates:** `image_coords = normalized * image_dimensions`
4. **Crop regions** from image using image coordinates
5. **Run OCR** on cropped regions

This allows the same coordinate file to work with:
- 300 DPI scans
- 600 DPI scans
- Different scanner models
- Any resolution, as long as aspect ratio matches

## Aspect Ratio Requirement

**Important:** Normalized coordinates are aspect-ratio dependent.

If the aspect ratio of the scanned document doesn't match the calibration document, coordinates will be incorrect.

**Solution:** Always calibrate using a document with the same aspect ratio as the documents to be processed.

**Aspect Ratio Calculation:**
```python
aspect_ratio = image_width / image_height
```

**Aspect Ratio Storage:**
Exports include aspect ratio for validation:
```json
{
  "aspect_ratio": 0.6667,
  "image_size": [2000, 3000]
}
```

