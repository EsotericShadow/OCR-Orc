# Coordinate System - Implementation Steps

## Objective

Implement the three-coordinate system (normalized, image, canvas) with all transformation functions.

## Prerequisites

- Directory structure created
- Basic C++ project compiles

## Coordinate Systems

1. **Normalized** (0.0-1.0): Source of truth, resolution-independent
2. **Image** (pixels): Pixel positions in original PDF image
3. **Canvas** (display pixels): Screen positions for rendering

## Steps

### Step 1: Create CoordinateSystem Class

1. Create `src/core/CoordinateSystem.h`
2. Define class `CoordinateSystem`
3. Add private members for image dimensions and scale factors
4. Add public methods for transformations

### Step 2: Define Coordinate Types

1. Create `struct NormalizedCoords { double x1, y1, x2, y2; }`
2. Create `struct ImageCoords { int x1, y1, x2, y2; }`
3. Create `struct CanvasCoords { double x1, y1, x2, y2; }`
4. Add validation functions for each type

### Step 3: Implement Normalized → Image Transformation

1. Function: `ImageCoords normalizedToImage(NormalizedCoords norm, int imgWidth, int imgHeight)`
2. Formula: `image_x = normalized_x * image_width`
3. Round to integers
4. Validate bounds (0 to image dimensions)

### Step 4: Implement Image → Normalized Transformation

1. Function: `NormalizedCoords imageToNormalized(ImageCoords img, int imgWidth, int imgHeight)`
2. Formula: `normalized_x = image_x / image_width`
3. Validate range (0.0 to 1.0)
4. Handle edge cases (division by zero)

### Step 5: Implement Image → Canvas Transformation

1. Function: `CanvasCoords imageToCanvas(ImageCoords img, double scaleFactor, double offsetX, double offsetY)`
2. Formula: `canvas_x = (image_x * scale_factor) + offset_x`
3. Account for image position on canvas
4. Handle zoom level

### Step 6: Implement Canvas → Image Transformation

1. Function: `ImageCoords canvasToImage(CanvasCoords canvas, double scaleFactor, double offsetX, double offsetY)`
2. Formula: `image_x = (canvas_x - offset_x) / scale_factor`
3. Round to integers
4. Validate bounds

### Step 7: Implement Normalized → Canvas (via Image)

1. Function: `CanvasCoords normalizedToCanvas(NormalizedCoords norm, int imgWidth, int imgHeight, double scaleFactor, double offsetX, double offsetY)`
2. First convert to image, then to canvas
3. Reuse existing functions

### Step 8: Add Coordinate Validation

1. Function: `bool isValidNormalized(NormalizedCoords coords)`
   - Check range: 0.0 <= x,y <= 1.0
   - Check order: x1 < x2, y1 < y2
2. Function: `bool isValidImage(ImageCoords coords, int imgWidth, int imgHeight)`
   - Check bounds: 0 <= x,y <= dimensions
   - Check order: x1 < x2, y1 < y2
3. Function: `bool isValidCanvas(CanvasCoords coords)`
   - Check order: x1 < x2, y1 < y2

### Step 9: Add Helper Functions

1. `void normalizeCoords(NormalizedCoords& coords)` - Ensure x1 < x2, y1 < y2
2. `void clampToBounds(NormalizedCoords& coords)` - Clamp to 0.0-1.0
3. `double calculateAspectRatio(int width, int height)` - Calculate aspect ratio

### Step 10: Write Unit Tests

1. Create `tests/test_coordinate_system.cpp`
2. Test all transformation functions
3. Test edge cases (zero dimensions, negative values, out of bounds)
4. Test coordinate validation
5. Verify transformations are reversible (within rounding error)

### Step 11: Integrate with Main Application

1. Include CoordinateSystem in main application
2. Test with sample coordinates
3. Verify no memory leaks

## Acceptance Criteria

- [ ] All transformation functions implemented
- [ ] Coordinate validation works correctly
- [ ] Unit tests pass (100% coverage of transformation functions)
- [ ] No memory leaks
- [ ] Handles edge cases gracefully
- [ ] Code is well-documented

## Test Cases

1. Normalized (0.25, 0.10, 0.35, 0.20) → Image (500, 300, 700, 600) for 2000x3000 image
2. Image → Normalized (reverse of above)
3. Image → Canvas with scale factor 0.4 and offset (100, 50)
4. Canvas → Image (reverse of above)
5. Invalid coordinates are rejected
6. Out-of-bounds coordinates are clamped

## Notes

- Use `double` for floating-point calculations (precision matters)
- Round to integers only for image coordinates
- Keep canvas coordinates as doubles (sub-pixel accuracy)
- Document all formulas in code comments

