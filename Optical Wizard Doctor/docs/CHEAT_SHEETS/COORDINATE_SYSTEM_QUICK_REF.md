# Coordinate System Quick Reference

## Three Coordinate Systems

### 1. Normalized (Source of Truth)
- **Range**: 0.0 to 1.0
- **Type**: double
- **Purpose**: Resolution-independent storage
- **Format**: `{x1, y1, x2, y2}`

### 2. Image (Pixels)
- **Range**: 0 to image dimensions
- **Type**: int
- **Purpose**: Pixel positions in original PDF
- **Format**: `{x1, y1, x2, y2}`

### 3. Canvas (Display)
- **Range**: 0 to canvas dimensions
- **Type**: double
- **Purpose**: Screen positions for rendering
- **Format**: `{x1, y1, x2, y2}`

---

## Transformation Formulas

### Normalized → Image
```cpp
image_x = normalized_x * image_width
image_y = normalized_y * image_height
```

### Image → Normalized
```cpp
normalized_x = image_x / image_width
normalized_y = image_y / image_height
```

### Image → Canvas
```cpp
canvas_x = (image_x * scale_factor) + image_x_offset
canvas_y = (image_y * scale_factor) + image_y_offset
```

### Canvas → Image
```cpp
image_x = (canvas_x - image_x_offset) / scale_factor
image_y = (canvas_y - image_y_offset) / scale_factor
```

### Normalized → Canvas (via Image)
```cpp
// Step 1: Normalized → Image
image_x = normalized_x * image_width
// Step 2: Image → Canvas
canvas_x = (image_x * scale_factor) + image_x_offset
```

---

## Scale Factor Calculation

```cpp
base_scale = min(canvas_width/image_width, 
                 canvas_height/image_height, 
                 1.0)
scale_factor = base_scale * zoom_level
```

---

## Synchronization Rules

1. **Normalized is source of truth** - Always calculate from normalized
2. **On zoom change**: Recalculate canvas from normalized
3. **On image load**: Calculate image from normalized
4. **On region creation**: Calculate all three from canvas input
5. **On region move**: Update all three with delta
6. **On region resize**: Update all three with new bounds

---

## Validation Rules

### Normalized Coordinates
- All values: 0.0 ≤ value ≤ 1.0
- Order: x1 < x2, y1 < y2

### Image Coordinates
- All values: 0 ≤ value ≤ image_dimension
- Order: x1 < x2, y1 < y2
- Type: integers

### Canvas Coordinates
- Order: x1 < x2, y1 < y2
- Type: doubles (sub-pixel accuracy)

---

## Common Operations

### Create Region
```
Canvas Coords (user input)
    → Image Coords (canvasToImage)
    → Normalized Coords (imageToNormalized)
    → Store all three
```

### Move Region
```
Calculate delta in Canvas Coords
    → Convert delta to Image Coords (delta / scale_factor)
    → Convert delta to Normalized Coords (delta / image_dimensions)
    → Update all three coordinate systems
```

### Zoom Change
```
Update zoom_level
    → Recalculate scale_factor
    → For each region:
        Recalculate Canvas Coords from Normalized
        (normalizedToCanvas)
```

---

*Quick reference for coordinate transformations*

