# Resize Rotated Rectangle: Complete Technical Analysis & Solution

## Executive Summary

When resizing a rotated region, all 4 sides appear to change visually instead of just the dragged corner/edge. This document provides a complete technical analysis of the bug and the correct solution.

**Root Cause**: The resize code incorrectly assumes handle positions correspond to bounding box corners, when they actually correspond to rotated positions of the unrotated rectangle corners.

**Solution**: Replace the bounding box-based resize logic with a simpler, geometrically correct approach that works in rotated space and transforms back to unrotated space.

---

## Table of Contents

1. [System Architecture Overview](#1-system-architecture-overview)
2. [Data Flow Analysis](#2-data-flow-analysis)
3. [The Bug: Detailed Analysis](#3-the-bug-detailed-analysis)
4. [Visual Proof](#4-visual-proof)
5. [The Correct Solution](#5-the-correct-solution)
6. [Implementation Guide](#6-implementation-guide)
7. [Edge Cases & Considerations](#7-edge-cases--considerations)
8. [Testing Strategy](#8-testing-strategy)

---

## 1. System Architecture Overview

### Coordinate Systems

The application uses three coordinate systems:

| System | Range | Purpose |
|--------|-------|---------|
| **Normalized** | 0.0 - 1.0 | Source of truth, resolution-independent storage |
| **Image** | 0 - width/height pixels | Pixel positions in original PDF |
| **Canvas** | Display pixels | Screen rendering positions |

### Key Data Structures

```cpp
// RegionData stores the unrotated rectangle + rotation angle separately
struct RegionData {
    NormalizedCoords normalizedCoords;  // Unrotated rectangle (source of truth)
    double rotationAngle;               // Rotation in degrees
    // ... other fields
};
```

**Critical Insight**: `normalizedCoords` always represents the **unrotated** rectangle. Rotation is applied only during rendering, not stored in the coordinates.

### Relevant Files

| File | Purpose |
|------|---------|
| `CanvasRenderer.cpp` | Draws regions with rotation transformation |
| `CanvasHitTester.cpp` | Detects which handle was clicked |
| `CanvasRegionOperations.cpp` | Performs resize operation (contains the bug) |
| `CanvasMouseHandler.cpp` | Handles mouse events, calls resize |
| `Canvas.cpp` | Coordinates the resize callback |

---

## 2. Data Flow Analysis

### Handle Detection Flow (Working Correctly)

```
User clicks on canvas
    ↓
CanvasMouseHandler::handlePress()
    ↓
CanvasHitTester::getHandleAt(canvasPos, regionRect, rotationAngle)
    ↓
Transform canvasPos to unrotated space (using -rotationAngle)
    ↓
Check distance to each unrotated corner
    ↓
Return handle name (e.g., "ne" = top-right of UNROTATED rect)
```

**Key Point**: The handle name refers to the **unrotated rectangle corner**, not the bounding box corner.

### Handle Rendering Flow (Working Correctly)

```
CanvasRenderer::drawResizeHandles(painter, rect, isRotateMode, rotationAngle)
    ↓
Apply rotation transformation to painter:
    painter.translate(center)
    painter.rotate(rotationAngle)
    painter.translate(-center)
    ↓
Draw handles at UNROTATED corner positions
    ↓
Painter transformation causes visual rotation
```

**Key Point**: Handles are drawn at unrotated positions, but the painter rotation makes them appear at the rotated positions visually.

### Resize Flow (Contains the Bug)

```
User drags handle
    ↓
CanvasMouseHandler::handleMove() calls resizeRegion callback
    ↓
Canvas::resizeRegion() calls regionOperations->resizeRegion()
    ↓
CanvasRegionOperations::resizeRegion()
    ↓
❌ BUG: Gets handle position from BOUNDING BOX
    ↓
❌ BUG: Calculates delta from bounding box position
    ↓
❌ BUG: Tries to solve inverse bounding box problem
    ↓
Wrong result: All corners appear to move
```

---

## 3. The Bug: Detailed Analysis

### Location

`CanvasRegionOperations.cpp`, lines 170-380 (the rotated resize section)

### The Incorrect Code

```cpp
// Lines 210-228: Getting handle position from BOUNDING BOX
QPointF boundingBoxHandlePos;
if (handle == "nw") {
    boundingBoxHandlePos = currentBoundingBox.topLeft();   // ❌ WRONG
} else if (handle == "ne") {
    boundingBoxHandlePos = currentBoundingBox.topRight();  // ❌ WRONG
} else if (handle == "sw") {
    boundingBoxHandlePos = currentBoundingBox.bottomLeft(); // ❌ WRONG
} else if (handle == "se") {
    boundingBoxHandlePos = currentBoundingBox.bottomRight(); // ❌ WRONG
}
// ... edge handles also wrong
```

### Why This Is Wrong

1. **Handle "ne"** means the user clicked on the **rotated position of the unrotated top-right corner**
2. This is **NOT** the same as `boundingBox.topRight()`
3. The bounding box is an axis-aligned rectangle that **contains** the rotated rectangle
4. Bounding box corners are at the extrema of the rotated corners, not at the rotated corners themselves

### Mathematical Proof

For a rectangle with corners at (0,0), (w,0), (w,h), (0,h) rotated by angle θ around its center:

**Rotated top-right corner position:**
```
rotated_TR = center + rotate(TR - center, θ)
           = center + ((w/2)·cos(θ) - (-h/2)·sin(θ), (w/2)·sin(θ) + (-h/2)·cos(θ))
```

**Bounding box top-right position:**
```
bbox_TR = (max(all_rotated_x), min(all_rotated_y))
```

These are different values except when θ = 0.

---

## 4. Visual Proof

### Example: Square Rotated 45°

```
                    ◆ ← This is where handle "n" actually is
                   /|\    (rotated top-center)
                  / | \
                 /  |  \
    Handle →   ◆   |   ◆  ← Handle "ne" (rotated top-right corner)
    "nw"        \  |  /
                 \ | /
                  \|/
                   ◆ ← Handle "s" (rotated bottom-center)
                   
    ┌─────────────────────┐
    │                     │ ← BOUNDING BOX
    │  bbox.topLeft()     │   The bug uses these corners!
    │         ↓           │
    │    × ─ ─ ─ ─ ─ ×    │   But handles are at ◆ positions,
    │                     │   NOT at × positions!
    └─────────────────────┘
```

### The Mismatch

| Handle Name | Where Handle Actually Is | Where Bug Thinks It Is |
|-------------|-------------------------|------------------------|
| "nw" | Left diamond point (◆) | Bounding box top-left (×) |
| "ne" | Top diamond point (◆) | Bounding box top-right (×) |
| "se" | Right diamond point (◆) | Bounding box bottom-right (×) |
| "sw" | Bottom diamond point (◆) | Bounding box bottom-left (×) |

This mismatch causes the delta calculation to be completely wrong.

---

## 5. The Correct Solution

### Algorithm Overview

1. **Get the unrotated corner** corresponding to the handle name
2. **Rotate this corner** to find its actual visual position (where the handle really is)
3. **Calculate delta** in visual space: `newPos - rotatedCornerPosition`
4. **Transform delta back** to unrotated space (inverse rotation)
5. **Apply delta** to the unrotated rectangle (same logic as non-rotated case)

### Pseudocode

```cpp
void resizeRotatedRegion(handle, newPos, unrotatedRect, rotationAngle) {
    QPointF center = unrotatedRect.center();
    double angleRad = rotationAngle * M_PI / 180.0;
    double cosA = cos(angleRad);
    double sinA = sin(angleRad);
    
    // Step 1: Get unrotated corner for this handle
    QPointF unrotatedCorner;
    if (handle == "nw") unrotatedCorner = unrotatedRect.topLeft();
    else if (handle == "ne") unrotatedCorner = unrotatedRect.topRight();
    else if (handle == "sw") unrotatedCorner = unrotatedRect.bottomLeft();
    else if (handle == "se") unrotatedCorner = unrotatedRect.bottomRight();
    else if (handle == "n") unrotatedCorner = QPointF(unrotatedRect.center().x(), unrotatedRect.top());
    else if (handle == "s") unrotatedCorner = QPointF(unrotatedRect.center().x(), unrotatedRect.bottom());
    else if (handle == "e") unrotatedCorner = QPointF(unrotatedRect.right(), unrotatedRect.center().y());
    else if (handle == "w") unrotatedCorner = QPointF(unrotatedRect.left(), unrotatedRect.center().y());
    
    // Step 2: Rotate corner to find visual position
    QPointF translated = unrotatedCorner - center;
    QPointF rotatedCorner = QPointF(
        translated.x() * cosA - translated.y() * sinA,
        translated.x() * sinA + translated.y() * cosA
    ) + center;
    
    // Step 3: Calculate delta in visual/rotated space
    QPointF visualDelta = newPos - rotatedCorner;
    
    // Step 4: Transform delta to unrotated space (inverse rotation)
    double invCosA = cos(-angleRad);
    double invSinA = sin(-angleRad);
    QPointF unrotatedDelta = QPointF(
        visualDelta.x() * invCosA - visualDelta.y() * invSinA,
        visualDelta.x() * invSinA + visualDelta.y() * invCosA
    );
    
    // Step 5: Apply delta to unrotated rectangle
    QRectF newRect = unrotatedRect;
    if (handle == "nw") {
        newRect.setTopLeft(unrotatedRect.topLeft() + unrotatedDelta);
    } else if (handle == "ne") {
        newRect.setTopRight(unrotatedRect.topRight() + unrotatedDelta);
    } else if (handle == "sw") {
        newRect.setBottomLeft(unrotatedRect.bottomLeft() + unrotatedDelta);
    } else if (handle == "se") {
        newRect.setBottomRight(unrotatedRect.bottomRight() + unrotatedDelta);
    } else if (handle == "n") {
        newRect.setTop(unrotatedRect.top() + unrotatedDelta.y());
    } else if (handle == "s") {
        newRect.setBottom(unrotatedRect.bottom() + unrotatedDelta.y());
    } else if (handle == "e") {
        newRect.setRight(unrotatedRect.right() + unrotatedDelta.x());
    } else if (handle == "w") {
        newRect.setLeft(unrotatedRect.left() + unrotatedDelta.x());
    }
    
    return newRect;
}
```

### Why This Works

1. **We calculate delta from the actual handle position** - The handle is visually at the rotated corner position, so we calculate how far the user dragged from there.

2. **We transform to the coordinate system we modify** - Since `normalizedCoords` stores the unrotated rectangle, we transform the delta to unrotated space before applying it.

3. **The resize logic is identical to non-rotated** - Once we have the delta in unrotated space, we just apply it to the unrotated rectangle exactly like the non-rotated case.

4. **No inverse problem to solve** - We don't need to solve "what unrotated rectangle produces this bounding box" because we're working directly with the unrotated rectangle.

---

## 6. Implementation Guide

### Changes Required

**File**: `CanvasRegionOperations.cpp`

**Lines to Replace**: 170-380 (the entire `if (rotationAngle != 0.0)` block)

### Before (Current Broken Code)

```cpp
if (rotationAngle != 0.0) {
    // ~210 lines of bounding box logic
    // ...
}
```

### After (Fixed Code)

```cpp
if (rotationAngle != 0.0) {
    OCR_ORC_DEBUG("[RESIZE] ROTATED RESIZE - Working in rotated corner space:");
    OCR_ORC_DEBUG("[RESIZE]   Rotation angle: " << rotationAngle << " degrees");
    
    QPointF center = unrotatedRect.center();
    double angleRad = rotationAngle * M_PI / 180.0;
    double cosA = std::cos(angleRad);
    double sinA = std::sin(angleRad);
    
    // Step 1: Get the unrotated corner corresponding to the handle
    QPointF unrotatedCorner;
    if (handle == "nw") {
        unrotatedCorner = unrotatedRect.topLeft();
    } else if (handle == "ne") {
        unrotatedCorner = unrotatedRect.topRight();
    } else if (handle == "sw") {
        unrotatedCorner = unrotatedRect.bottomLeft();
    } else if (handle == "se") {
        unrotatedCorner = unrotatedRect.bottomRight();
    } else if (handle == "n") {
        unrotatedCorner = QPointF(unrotatedRect.center().x(), unrotatedRect.top());
    } else if (handle == "s") {
        unrotatedCorner = QPointF(unrotatedRect.center().x(), unrotatedRect.bottom());
    } else if (handle == "e") {
        unrotatedCorner = QPointF(unrotatedRect.right(), unrotatedRect.center().y());
    } else if (handle == "w") {
        unrotatedCorner = QPointF(unrotatedRect.left(), unrotatedRect.center().y());
    }
    OCR_ORC_DEBUG("[RESIZE]   Unrotated corner: (" << unrotatedCorner.x() << "," << unrotatedCorner.y() << ")");
    
    // Step 2: Rotate this corner to find its visual position
    QPointF translated = unrotatedCorner - center;
    QPointF rotatedCorner = QPointF(
        translated.x() * cosA - translated.y() * sinA,
        translated.x() * sinA + translated.y() * cosA
    ) + center;
    OCR_ORC_DEBUG("[RESIZE]   Rotated corner (visual position): (" << rotatedCorner.x() << "," << rotatedCorner.y() << ")");
    
    // Step 3: Calculate delta in visual/rotated space
    QPointF visualDelta = newPos - rotatedCorner;
    OCR_ORC_DEBUG("[RESIZE]   Visual delta: (" << visualDelta.x() << "," << visualDelta.y() << ")");
    
    // Step 4: Transform delta to unrotated space (inverse rotation)
    double invAngleRad = -angleRad;
    double invCosA = std::cos(invAngleRad);
    double invSinA = std::sin(invAngleRad);
    QPointF unrotatedDelta = QPointF(
        visualDelta.x() * invCosA - visualDelta.y() * invSinA,
        visualDelta.x() * invSinA + visualDelta.y() * invCosA
    );
    OCR_ORC_DEBUG("[RESIZE]   Unrotated delta: (" << unrotatedDelta.x() << "," << unrotatedDelta.y() << ")");
    
    // Step 5: Apply delta to the unrotated rectangle (same as non-rotated case)
    newRect = unrotatedRect;
    if (handle == "nw") {
        newRect.setTopLeft(unrotatedRect.topLeft() + unrotatedDelta);
    } else if (handle == "ne") {
        newRect.setTopRight(unrotatedRect.topRight() + unrotatedDelta);
    } else if (handle == "sw") {
        newRect.setBottomLeft(unrotatedRect.bottomLeft() + unrotatedDelta);
    } else if (handle == "se") {
        newRect.setBottomRight(unrotatedRect.bottomRight() + unrotatedDelta);
    } else if (handle == "n") {
        newRect.setTop(unrotatedRect.top() + unrotatedDelta.y());
    } else if (handle == "s") {
        newRect.setBottom(unrotatedRect.bottom() + unrotatedDelta.y());
    } else if (handle == "e") {
        newRect.setRight(unrotatedRect.right() + unrotatedDelta.x());
    } else if (handle == "w") {
        newRect.setLeft(unrotatedRect.left() + unrotatedDelta.x());
    }
    OCR_ORC_DEBUG("[RESIZE]   New rect: (" << newRect.left() << "," << newRect.top() << ") size: " << newRect.width() << "x" << newRect.height());
}
```

### Code Size Comparison

| Version | Lines of Code | Complexity |
|---------|---------------|------------|
| Current (broken) | ~210 lines | High (inverse problem solving) |
| Fixed | ~60 lines | Low (direct transformation) |

---

## 7. Edge Cases & Considerations

### Center Movement During Resize

When resizing an unrotated rectangle, the center moves. For example:
- Dragging the top-right corner moves TR and shifts the center
- The bottom-left corner stays fixed (in unrotated space)

For rotated rectangles:
- We modify the unrotated rectangle, so BL stays fixed in unrotated space
- When rendered, the rotation is around the NEW center
- This means BL might shift slightly in visual space

**This is expected behavior** and matches how most graphics software handles rotated object resizing.

### Edge Handles (n, s, e, w)

For edge handles, we apply only one component of the unrotated delta:
- "n" and "s": Apply only `unrotatedDelta.y()` to top/bottom edge
- "e" and "w": Apply only `unrotatedDelta.x()` to right/left edge

This means diagonal mouse movement on an edge handle will only move the edge perpendicular to itself (in unrotated space), which may feel slightly different from non-rotated edge dragging but is geometrically consistent.

### Rotation Angles Near 45°, 135°, etc.

The current broken code has special handling for 45° rotations because its inverse calculation fails (determinant approaches zero). The fixed code has no such issues because it doesn't solve an inverse problem.

### Minimum Size Enforcement

The existing minimum size and bounds clamping code (lines 435-479) can remain unchanged. It operates on the final `newRect` regardless of how it was calculated.

### Preserving Existing Validation

Keep all existing validation at the start of the function:
- Region name validation
- Handle string validation
- Document state validation
- Image dimension validation

---

## 8. Testing Strategy

### Manual Test Cases

| Test Case | Steps | Expected Result |
|-----------|-------|-----------------|
| Corner resize 0° | Create region, drag SE corner | Only SE corner moves |
| Corner resize 45° | Rotate 45°, drag SE handle | Only SE corner moves visually |
| Corner resize 90° | Rotate 90°, drag NE handle | Only the handle moves visually |
| Edge resize 30° | Rotate 30°, drag N edge | Top edge moves parallel to itself |
| Small rotation | Rotate 5°, drag any corner | Smooth resize, no jitter |
| Large rotation | Rotate 170°, drag any corner | Correct behavior, no flip |
| Negative rotation | Rotate -45°, drag handle | Same as positive 45° but mirrored |

### Automated Test Suggestions

Add tests to `test_canvas_region_operations.cpp`:

```cpp
TEST(CanvasRegionOperations, ResizeRotated45DegreesCorner) {
    // Create region at known position
    // Rotate 45 degrees
    // Simulate drag on "se" handle
    // Verify only SE corner moved in visual space
    // Verify opposite corner stayed fixed in visual space
}

TEST(CanvasRegionOperations, ResizeRotated45DegreesEdge) {
    // Create region at known position
    // Rotate 45 degrees
    // Simulate drag on "n" handle
    // Verify top edge moved perpendicular to itself
}

TEST(CanvasRegionOperations, ResizeRotated90Degrees) {
    // Special case: 90° rotation
    // Verify handles map correctly (NE becomes SE visually)
}
```

### Regression Tests

Ensure existing non-rotated resize tests still pass:
- `test_canvas_region_operations.cpp` existing tests
- No changes should be needed for non-rotated code path

---

## Appendix A: Why the Bounding Box Approach Failed

The bounding box approach tried to solve:
> "Given a desired bounding box, what unrotated rectangle produces it when rotated?"

This is mathematically:
```
W = w·|cos(θ)| + h·|sin(θ)|
H = w·|sin(θ)| + h·|cos(θ)|
```

Solving for w, h:
```
det = cos²(θ) - sin²(θ) = cos(2θ)
w = (W·|cos(θ)| - H·|sin(θ)|) / det
h = (H·|cos(θ)| - W·|sin(θ)|) / det
```

Problems:
1. **det = 0 at 45°, 135°, etc.** - Division by zero
2. **Not unique** - Different (w,h) pairs can produce same bounding box
3. **Center calculation is wrong** - Resizing bounding box changes center differently than resizing unrotated rect

The correct approach avoids all these problems by not solving an inverse.

---

## Appendix B: File References

| File | Lines | Description |
|------|-------|-------------|
| `CanvasRegionOperations.cpp` | 91-534 | `resizeRegion()` function |
| `CanvasRegionOperations.cpp` | 170-380 | Broken rotated resize logic (to replace) |
| `CanvasHitTester.cpp` | 63-116 | `getHandleAt()` - correct, uses unrotated space |
| `CanvasRenderer.cpp` | 170-249 | `drawResizeHandles()` - correct, uses painter rotation |
| `CanvasMouseHandler.cpp` | 159-203 | Resize initiation on mouse press |
| `Canvas.cpp` | 608-648 | `resizeRegion()` callback |

---

## Document History

| Date | Author | Description |
|------|--------|-------------|
| 2025-12-09 | Claude (Opus 4.5) | Initial deep analysis and solution design |

