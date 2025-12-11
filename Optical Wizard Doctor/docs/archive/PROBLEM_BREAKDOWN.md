# Problem Breakdown: Resize Rotated Rectangle

## Defining All Operands

### Inputs (What We Have)
1. **unrotatedRect**: The rectangle before rotation (x, y, width, height)
2. **rotationAngle**: Angle in degrees (e.g., -51.7°)
3. **handle**: Which handle is being dragged ("nw", "ne", "sw", "se", "n", "s", "e", "w")
4. **newPos**: New mouse position in canvas coordinates (where user dragged to)
5. **originalNormalizedCoords**: The normalized coordinates at resize start (stable reference)

### Intermediate Values (What We Calculate)
1. **center**: Center point of unrotatedRect
2. **rotatedCorners[4]**: The 4 corners of unrotatedRect after rotation
3. **currentBoundingBox**: The axis-aligned rectangle that contains all rotatedCorners
4. **handlePosition**: The position of the handle in bounding box space
5. **delta**: How far the handle moved (newPos - handlePosition)

### Output (What We Need)
1. **newUnrotatedRect**: The new unrotated rectangle that, when rotated, shows only the dragged corner moving

## The Transformation Chain

### Forward (What Happens During Rendering)
```
unrotatedRect → [rotate by angle] → rotatedCorners → [calculate bbox] → boundingBox → [draw] → visual representation
```

### Backward (What We Need to Do)
```
user drags handle → newPos → [resize boundingBox] → newBoundingBox → [inverse transform] → newUnrotatedRect
```

## The Core Problem

**Question**: Given a new bounding box position, what unrotated rectangle produces it when rotated?

**The Issue**: The bounding box is NOT the rotated rectangle. The bounding box is the axis-aligned container of the rotated rectangle.

## What Actually Needs to Happen (Simple Terms)

1. **User drags a handle** → This handle is at a corner of the VISUAL bounding box
2. **We need to resize** → But we're storing an unrotated rectangle
3. **The solution**: 
   - Calculate which corner of the unrotated rectangle corresponds to the dragged bounding box corner
   - Resize the unrotated rectangle such that when rotated, only that visual corner moves
   - Keep the opposite corner of the unrotated rectangle fixed

## The Key Insight

When you rotate a rectangle, the bounding box corners are NOT the same as the rotated rectangle corners.

Example: Rotate a rectangle 45°
- The bounding box is larger than the rectangle
- The top-left of the bounding box is NOT the top-left of the rotated rectangle
- The top-left of the bounding box is actually the rotated top-left OR rotated top-right corner (whichever is more top-left)

## What We're Currently Doing Wrong

1. We're resizing the bounding box
2. We're trying to find the unrotated rectangle that produces this bounding box
3. But this is the WRONG approach because:
   - The bounding box doesn't directly correspond to the rectangle
   - Resizing the bounding box changes the rectangle in unexpected ways

## What We Should Be Doing

1. **Identify which unrotated corner** corresponds to the dragged handle
   - The handle name (e.g., "ne") refers to the unrotated rectangle corner
   - When rotated, this corner is at some position in the bounding box
   - But it's NOT necessarily at the bounding box corner with the same name

2. **Resize the unrotated rectangle directly**
   - Move the identified unrotated corner
   - Keep the opposite unrotated corner fixed
   - This is simple: just modify the unrotated rectangle

3. **The visual result** will show the correct behavior because:
   - We're modifying the actual rectangle, not the bounding box
   - The bounding box will automatically adjust to contain the new rotated rectangle
   - Only the dragged corner will move visually

## The Real Problem

We're trying to work backwards from the bounding box, but we should be working forwards from the unrotated rectangle.

The handle detection (`getHandleAt`) already transforms the click back to unrotated space and identifies which unrotated corner was clicked. So the handle name (e.g., "ne") refers to the unrotated rectangle's top-right corner.

When we resize, we should:
1. Transform the mouse drag point to unrotated space (we're doing this)
2. Calculate delta from the original unrotated corner position (we're doing this)
3. Apply delta to move ONLY that unrotated corner (we're doing this)
4. Keep the opposite unrotated corner fixed (we're doing this)

**So why isn't it working?**

The issue must be in step 1 or 2 - the transformation or the delta calculation.

## The Actual Issue

When we transform `newPos` (the mouse drag point) back to unrotated space, we're using the center of the unrotated rectangle as the rotation center. But the mouse is dragging a handle that's visually at a bounding box corner, not an unrotated rectangle corner.

The transformation is correct, but the **reference point** is wrong. We're comparing:
- `transformedNewPos` (mouse position transformed to unrotated space)
- `originalHandlePos` (unrotated rectangle corner position)

These should match if the transformation is correct, but they might not because:
- The visual handle is at the bounding box corner
- The unrotated corner, when rotated, is NOT at the bounding box corner
- So the delta is calculated incorrectly

## The Solution (Simple Terms)

1. **Don't transform the mouse position** - work directly with it
2. **Calculate which unrotated corner** the handle refers to
3. **Rotate that unrotated corner** to find where it is visually (in bounding box space)
4. **Calculate delta** in visual/rotated space: `newPos - rotatedCornerPosition`
5. **Transform the delta back** to unrotated space
6. **Apply delta** to the unrotated corner
7. **Keep opposite corner fixed**

This way, we're working in the space where the user is actually dragging (visual/rotated space), calculating the delta there, then transforming it back.

