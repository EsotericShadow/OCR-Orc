# Root Cause Analysis: Resize Rotated Region - All Sides Changing

## Key Findings from Logs

### What's Actually Happening

1. **Handle**: "ne" (top-right corner)
2. **Unrotated Rectangle**: (234, 103.855) size: 61.5709 x 49.7018
3. **Original Handle Position (unrotated)**: (295.571, 103.855) - This is the top-right corner ✓
4. **Mouse Drag Point (rotated canvas space)**: Transformed back to unrotated space
5. **Delta Calculated**: Correctly calculated from original handle position
6. **Resize Applied**: 
   - `newRect.setRight(unrotatedRect.right() + delta.x())` ✓
   - `newRect.setTop(unrotatedRect.top() + delta.y())` ✓

### The Problem

**When resizing from top-right corner:**
- **Expected**: Only top-right corner moves, bottom-left stays fixed
- **Actual in unrotated space**: 
  - Top-left: (234, 109.602) [was: (234, 103.855)] - **Y changed by delta.y = 5.747** ❌
  - Top-right: (323.224, 109.602) - **Correctly moved** ✓
  - Bottom-left: (234, 153.556) [was: (234, 153.556)] - **Correctly fixed** ✓
  - Bottom-right: (323.224, 153.556) [was: (295.571, 153.556)] - **X changed by delta.x = 27.653** ❌

### Root Cause Identified

**The fundamental issue is a conceptual mismatch:**

1. **Normalized coordinates represent the unrotated rectangle** (not the bounding box)
2. **When we resize, we modify the unrotated rectangle** (correctly)
3. **But the visual representation shows the rotated bounding box**
4. **When the unrotated rectangle changes, the bounding box changes in a way that makes ALL corners appear to move visually**

### Why This Happens

When you resize a rotated rectangle:
- The unrotated rectangle is modified (e.g., top edge moves up by delta.y)
- When this unrotated rectangle is rotated, the bounding box changes
- The bounding box is what's visually displayed
- The bounding box corners are calculated from the rotated corners of the unrotated rectangle
- **When the top edge of the unrotated rectangle moves, ALL corners of the bounding box move**

### The Transformation Problem

The current approach:
1. Transforms mouse drag point from rotated space → unrotated space ✓
2. Calculates delta from original handle position ✓
3. Applies delta to unrotated rectangle ✓
4. **BUT**: The handle position in rotated space doesn't correspond to a single corner of the unrotated rectangle

**The real issue**: When you drag a handle in rotated space, you're dragging a corner of the **visual bounding box**, not a corner of the **unrotated rectangle**. These are different things!

### Visual vs. Actual Geometry

- **Visual bounding box** (what user sees): The axis-aligned rectangle that contains the rotated region
- **Unrotated rectangle** (what we store): The actual rectangle before rotation

When rotated:
- The bounding box is larger than the unrotated rectangle
- The bounding box corners don't align with unrotated rectangle corners
- Dragging a bounding box corner doesn't correspond to dragging an unrotated rectangle corner

### The Correct Approach (Conceptual)

To fix this, we need to:

1. **Identify which corner of the unrotated rectangle corresponds to the dragged handle**
   - The handle is at a corner of the visual bounding box
   - We need to find which corner of the unrotated rectangle, when rotated, creates that bounding box corner

2. **Resize the unrotated rectangle such that when rotated, only the dragged handle moves**
   - This requires calculating the inverse: given a desired bounding box corner position, what should the unrotated rectangle be?

3. **Or, resize in rotated space directly**
   - Transform the unrotated rectangle to rotated space
   - Resize in rotated space
   - Transform back to unrotated space

### Current Code Flow (What's Wrong)

```
User drags handle in rotated space
  ↓
Transform mouse position to unrotated space (using center of unrotated rect)
  ↓
Calculate delta from original handle position (in unrotated space)
  ↓
Apply delta to unrotated rectangle edges
  ↓
Convert back to normalized coordinates
  ↓
Render with rotation → bounding box changes → ALL corners appear to move
```

### The Missing Piece

**We're not accounting for the fact that:**
- The handle position in rotated space is a corner of the bounding box
- The handle position in unrotated space is a corner of the unrotated rectangle
- These two positions are related by rotation, but the relationship is complex when resizing

**The transformation center matters:**
- We're using the center of the unrotated rectangle as the rotation center
- But when resizing, the center might shift
- The handle position calculation needs to account for this

### Next Steps to Fix

1. **Calculate the actual handle position in rotated space** (from the visual bounding box)
2. **Transform this to find the corresponding unrotated rectangle corner**
3. **Resize the unrotated rectangle such that this corner moves to the new position**
4. **Ensure the opposite corner stays fixed in the unrotated rectangle**

This requires:
- Calculating the bounding box from the rotated unrotated rectangle
- Finding which unrotated corner corresponds to the dragged bounding box corner
- Resizing the unrotated rectangle appropriately
- Recalculating the bounding box to verify only the dragged corner moved

