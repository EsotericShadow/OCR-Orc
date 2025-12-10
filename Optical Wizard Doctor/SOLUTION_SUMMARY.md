# Solution Summary: Fix Inverted Resize Behavior

## Problem Statement
**User Observation**: "When I move away from the rect to make it larger, it actually gets smaller."

**Root Cause**: We're calculating the resize delta from the wrong reference point.

## The Exact Problem

### Current (Wrong) Approach
```cpp
// Step 1: Get handle position in bounding box space
if (handle == "ne") {
    boundingBoxHandlePos = currentBoundingBox.topRight(); // ❌ WRONG
}

// Step 2: Calculate delta
boundingBoxDelta = newPos - boundingBoxHandlePos; // ❌ WRONG because handlePos is wrong
```

**Why it's wrong:**
- Handle name "ne" refers to the **unrotated rectangle's top-right corner**
- When rotated, this corner is at position `rotatedCorners[1]`
- But we're using `currentBoundingBox.topRight()`, which is NOT where the handle is visually
- The bounding box's top-right is whichever rotated corner is most top-right, which may not be the rotated top-right corner

### Correct Approach
```cpp
// Step 1: Identify which unrotated corner corresponds to the handle
int cornerIndex;
if (handle == "nw") cornerIndex = 0;      // top-left
else if (handle == "ne") cornerIndex = 1; // top-right
else if (handle == "se") cornerIndex = 2; // bottom-right
else if (handle == "sw") cornerIndex = 3; // bottom-left

// Step 2: Get the rotated position of this corner (where handle is visually)
QPointF rotatedCorner = rotatedCorners[cornerIndex];

// Step 3: Calculate delta in visual/rotated space
QPointF delta = newPos - rotatedCorner; // ✅ CORRECT - delta from actual handle position

// Step 4: Transform delta back to unrotated space
QPointF translated = delta; // delta is already relative to center (rotatedCorner - center)
double invAngleRad = -angleRad;
double invCos = std::cos(invAngleRad);
double invSin = std::sin(invAngleRad);
QPointF unrotatedDelta(
    translated.x() * invCos - translated.y() * invSin,
    translated.x() * invSin + translated.y() * invCos
);

// Step 5: Apply delta to unrotated corner
QPointF originalUnrotatedCorner = corners[cornerIndex];
QPointF newUnrotatedCorner = originalUnrotatedCorner + unrotatedDelta;

// Step 6: Calculate new rectangle keeping opposite corner fixed
// (Implementation depends on which corner moved)
```

## The Key Insight

**The handle is visually at the rotated corner position, NOT the bounding box corner position.**

When you drag a handle:
- You're dragging from `rotatedCorners[cornerIndex]` (the visual handle position)
- NOT from `currentBoundingBox.corner()` (which is wrong)

## Why This Causes Inversion

Example scenario:
- Unrotated rectangle: 100x100 at (0, 0)
- Rotated 45°: The top-right corner, when rotated, is at some position
- Bounding box top-right is further away (it's the most top-right of all 4 rotated corners)
- User drags handle 10 pixels to the right
- Current code: `delta = newPos - boundingBoxTopRight` = might be -5 (wrong direction!)
- Correct code: `delta = newPos - rotatedCorner` = +10 (correct!)

## The Complete Fix Algorithm

1. **Map handle name to corner index**
   - "nw" → 0 (top-left)
   - "ne" → 1 (top-right)
   - "se" → 2 (bottom-right)
   - "sw" → 3 (bottom-left)

2. **Get rotated corner position** (where handle is visually)
   - `rotatedCorner = rotatedCorners[cornerIndex]`

3. **Calculate delta in visual space**
   - `delta = newPos - rotatedCorner`

4. **Transform delta to unrotated space**
   - Inverse rotate the delta vector

5. **Apply to unrotated corner**
   - `newUnrotatedCorner = originalUnrotatedCorner + unrotatedDelta`

6. **Calculate new rectangle**
   - Keep opposite corner fixed
   - Calculate new width/height from the two corners
   - Adjust center if needed

## Edge Handles

For edge handles ("n", "s", "e", "w"):
- Calculate the midpoint of the two adjacent corners
- Rotate that midpoint
- Calculate delta from rotated midpoint
- Transform delta back
- Apply to both corners of that edge

## Summary

**The Fix**: Calculate delta from the actual rotated corner position (where the handle is visually), not from the bounding box corner position.

**The Change**: Replace `boundingBoxHandlePos = currentBoundingBox.corner()` with `rotatedCorner = rotatedCorners[cornerIndex]` and calculate delta from there.

