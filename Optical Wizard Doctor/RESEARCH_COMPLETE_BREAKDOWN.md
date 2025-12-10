# Complete Problem Breakdown: Resize Rotated Rectangle

## User Observation
**"When I move away from the rect to make it larger, it actually gets smaller."**

This indicates an **inverted behavior** - the direction of the resize operation is backwards.

## Defining ALL Operands (The Math Problem)

### Input Operands (Given)
1. **unrotatedRect**: `QRectF(x, y, width, height)` - The rectangle in its unrotated state
2. **rotationAngle**: `double` - Rotation angle in degrees (e.g., -51.7°)
3. **handle**: `QString` - Which handle was clicked ("nw", "ne", "sw", "se", "n", "s", "e", "w")
   - **CRITICAL**: This handle name refers to the **unrotated rectangle's corner**, NOT the bounding box corner
4. **newPos**: `QPointF(x, y)` - The new mouse position in **canvas coordinates** (visual/rotated space)
5. **originalNormalizedCoords**: `NormalizedCoords` - The stable reference from resize start

### Intermediate Operands (Calculated)
1. **center**: `QPointF` - Center of unrotatedRect (rotation origin)
2. **angleRad**: `double` - rotationAngle converted to radians
3. **cosAngle, sinAngle**: `double` - Cosine and sine of rotation angle
4. **rotatedCorners[4]**: `QPointF[4]` - The 4 corners of unrotatedRect after rotation
5. **currentBoundingBox**: `QRectF` - Axis-aligned box containing all rotatedCorners
6. **boundingBoxHandlePos**: `QPointF` - Position of handle in bounding box space
7. **boundingBoxDelta**: `QPointF` - Delta in bounding box space (newPos - boundingBoxHandlePos)
8. **newBoundingBox**: `QRectF` - The desired new bounding box after resize
9. **w_prime, h_prime**: `double` - The new unrotated width and height (solved for)
10. **newUnrotatedRect**: `QRectF` - The output rectangle

### Output Operand (What We Need)
1. **newUnrotatedRect**: `QRectF` - The new unrotated rectangle that, when rotated, shows only the dragged corner moving

## The Transformation Chain

### Forward Transformation (Rendering)
```
unrotatedRect (x, y, w, h)
  ↓ [rotate by angle around center]
rotatedCorners[4] (4 points in rotated space)
  ↓ [calculate min/max]
currentBoundingBox (axis-aligned box)
  ↓ [draw handles at bounding box corners]
visual representation
```

### Backward Transformation (What We're Trying to Do)
```
user drags handle → newPos (in visual space)
  ↓ [identify which bounding box corner]
boundingBoxHandlePos
  ↓ [calculate delta]
boundingBoxDelta = newPos - boundingBoxHandlePos
  ↓ [resize bounding box]
newBoundingBox
  ↓ [solve inverse problem]
newUnrotatedRect (w', h')
```

## The Core Problem: Handle Name Mismatch

### What `getHandleAt` Does
1. Transforms click point from canvas space → unrotated space
2. Checks which unrotated rectangle corner is closest
3. Returns handle name (e.g., "ne") referring to **unrotated rectangle's top-right corner**

### What the Visual Handle Represents
1. Handles are drawn at **unrotated rectangle corners**
2. Then rotated with the rectangle
3. So the visual handle position is the **rotated unrotated corner**, NOT the bounding box corner

### The Mismatch
- **Handle name "ne"** = unrotated rectangle's top-right corner
- **Visual handle position** = rotated unrotated rectangle's top-right corner (after rotation)
- **Bounding box top-right** = NOT the same as rotated unrotated rectangle's top-right corner

When rotated 45°, the unrotated rectangle's top-right corner, when rotated, is NOT at the bounding box's top-right corner.

## Current Algorithm Analysis

### Step 1: Calculate Current Bounding Box
```
rotatedCorners[4] = rotate(unrotatedRect.corners, angle, center)
currentBoundingBox = minMax(rotatedCorners)
```
✅ **This is correct**

### Step 2: Get Handle Position in Bounding Box Space
```cpp
if (handle == "ne") {
    boundingBoxHandlePos = currentBoundingBox.topRight();
}
```
❌ **THIS IS WRONG**

**Why it's wrong:**
- `handle == "ne"` means the user clicked the **unrotated rectangle's top-right corner**
- When rotated, this corner is at some position in the bounding box
- But we're using `currentBoundingBox.topRight()`, which is NOT where the rotated corner is
- The bounding box's top-right is actually whichever rotated corner is most top-right

### Step 3: Calculate Delta
```cpp
boundingBoxDelta = newPos - boundingBoxHandlePos;
```
❌ **THIS IS WRONG BECAUSE boundingBoxHandlePos IS WRONG**

### Step 4: Resize Bounding Box
```cpp
newBoundingBox.setTopRight(boundingBoxHandlePos + boundingBoxDelta);
```
❌ **This resizes the bounding box, but we're using the wrong reference point**

### Step 5: Solve for Unrotated Rectangle
```cpp
// Solve: newBoundingBox = boundingBox(rotate(newUnrotatedRect, angle))
// Using: W' = w'*|cos(θ)| + h'*|sin(θ)|
//        H' = w'*|sin(θ)| + h'*|cos(θ)|
```
✅ **The math is correct, but the input (newBoundingBox) is wrong**

## The Root Cause

**We're using the wrong reference point for the delta calculation.**

When the user drags a handle:
1. The handle is visually at the **rotated unrotated corner position**
2. We need to calculate delta from this **rotated corner position**, not the bounding box corner
3. The bounding box corner is NOT where the handle is visually

## The Correct Approach (Simple Terms)

### What Actually Needs to Happen

1. **Identify the unrotated corner** that corresponds to the handle name
   - Handle "ne" = unrotated rectangle's top-right corner

2. **Calculate where this corner is visually** (after rotation)
   - Rotate the unrotated corner: `rotatedCorner = rotate(unrotatedCorner, angle, center)`
   - This is where the handle is visually positioned

3. **Calculate delta in visual space**
   - `delta = newPos - rotatedCorner`
   - This is how far the user dragged the handle

4. **Transform delta back to unrotated space**
   - `unrotatedDelta = inverseRotate(delta, -angle, center)`
   - This tells us how to move the unrotated corner

5. **Apply delta to unrotated corner**
   - `newUnrotatedCorner = originalUnrotatedCorner + unrotatedDelta`
   - Keep the opposite unrotated corner fixed

6. **Calculate new unrotated rectangle**
   - From the new corner position and fixed opposite corner, calculate new width/height
   - Keep center fixed (or adjust based on which corner moved)

## Why Current Approach Fails

The current approach:
1. Uses `boundingBoxHandlePos = currentBoundingBox.topRight()` for handle "ne"
2. But the visual handle is NOT at `currentBoundingBox.topRight()`
3. The visual handle is at `rotatedCorners[1]` (the rotated top-right corner of unrotated rect)
4. So the delta is calculated from the wrong point
5. This causes the inverted behavior

## The Inversion Explanation

When you drag away from the rectangle:
- You're dragging away from the **rotated corner position**
- But we calculate delta from the **bounding box corner** (which is further away)
- So `delta = newPos - boundingBoxCorner` is smaller than it should be
- Or in some cases, the sign is wrong
- This makes the rectangle shrink when it should grow

## The Fix (Conceptual)

Instead of:
```cpp
boundingBoxHandlePos = currentBoundingBox.topRight(); // WRONG
```

We need:
```cpp
// Find which rotated corner corresponds to the handle
QPointF rotatedCorner = rotatedCorners[handleIndex];
// Calculate delta from the actual visual handle position
QPointF delta = newPos - rotatedCorner;
// Transform delta to unrotated space
QPointF unrotatedDelta = inverseRotate(delta, -angle);
// Apply to unrotated corner
QPointF newUnrotatedCorner = originalUnrotatedCorner + unrotatedDelta;
```

## Handle to Corner Index Mapping

- "nw" (north-west) = top-left = corners[0]
- "ne" (north-east) = top-right = corners[1]
- "se" (south-east) = bottom-right = corners[2]
- "sw" (south-west) = bottom-left = corners[3]

For edge handles:
- "n" = top edge = average of corners[0] and corners[1]
- "e" = right edge = average of corners[1] and corners[2]
- "s" = bottom edge = average of corners[2] and corners[3]
- "w" = left edge = average of corners[3] and corners[0]

## Summary

**The Problem**: We're calculating delta from the bounding box corner, but the visual handle is at the rotated unrotated corner position.

**The Solution**: Calculate delta from the actual rotated corner position, transform it to unrotated space, and apply it directly to the unrotated rectangle.

**The Key Insight**: The handle name refers to the unrotated rectangle's corner, and we need to work with that corner's rotated position, not the bounding box corner.

