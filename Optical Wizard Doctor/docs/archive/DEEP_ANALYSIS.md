# Deep Analysis: Resize Rotated Region Issue

## The Core Problem

When resizing a rotated region, all 4 sides appear to change instead of just the dragged corner/edge.

## Key Findings

### 1. Handle Detection vs Handle Drawing
- **Handle Detection (`getHandleAt`)**: Transforms click point back to unrotated space, checks against unrotated rectangle corners
- **Handle Drawing (`drawResizeHandles`)**: Draws handles at unrotated rectangle corners, then rotates them with the rectangle
- **Result**: Handles are correctly positioned and detected

### 2. The Resize Operation
- **Current Approach**: 
  1. Transform mouse drag point to unrotated space
  2. Calculate delta from original handle position (unrotated corner)
  3. Apply delta to unrotated rectangle edges
  4. Convert back to normalized coordinates
  
- **Problem**: When we modify the unrotated rectangle (e.g., `setTop()`), we're moving an entire edge. When this rotated rectangle is displayed, the bounding box changes in a way that makes ALL corners appear to move.

### 3. The Fundamental Issue

**The unrotated rectangle and the visual bounding box are different things:**
- Unrotated rectangle: The actual rectangle stored in normalized coordinates
- Visual bounding box: The axis-aligned rectangle that contains the rotated rectangle

When you resize:
- You're modifying the unrotated rectangle
- The visual bounding box is recalculated from the rotated unrotated rectangle
- The bounding box corners don't correspond 1:1 with unrotated rectangle corners
- So when you move one unrotated corner, multiple bounding box corners move

### 4. Why This Happens

Example: Rotate a rectangle 45 degrees
- The top-right corner of the unrotated rectangle, when rotated, is NOT at the top-right of the bounding box
- The top-right of the bounding box is actually the rotated top-right OR bottom-right corner of the unrotated rectangle (whichever is higher/more right)
- When you resize by moving the unrotated top-right corner, the bounding box changes, and it looks like all corners moved

## The Solution

We need to resize in **rotated space**, not unrotated space:

1. Transform the unrotated rectangle to rotated space (get the 4 rotated corners)
2. Calculate the bounding box of these rotated corners
3. Resize the bounding box (keeping opposite corner fixed)
4. Find the new unrotated rectangle that, when rotated, produces this new bounding box
5. This is a complex inverse problem

**OR** a simpler approach:
1. Work entirely in rotated coordinate system
2. Resize the rotated rectangle directly
3. Transform back to unrotated space
4. But this requires storing the rotated rectangle, not the unrotated one

## Current Code Flow

```
User drags handle (visual bounding box corner)
  ↓
getHandleAt: Transforms click to unrotated space, finds handle name (e.g., "ne")
  ↓
resizeRegion: 
  - Gets unrotated rectangle from normalized coords
  - Transforms mouse drag point to unrotated space
  - Calculates delta from unrotated corner
  - Applies delta to unrotated rectangle edges (setTop, setRight)
  - Converts back to normalized
  ↓
Rendering: Rotates unrotated rectangle, calculates bounding box
  ↓
Visual result: Bounding box changes, all corners appear to move
```

## What Needs to Change

The resize operation needs to:
1. Calculate which corner of the unrotated rectangle, when rotated, creates the dragged bounding box corner
2. Resize the unrotated rectangle such that when rotated, only that bounding box corner moves
3. This requires solving: Given a desired bounding box corner position, what should the unrotated rectangle be?

This is a complex geometric problem that requires:
- Finding the inverse transformation
- Solving for the unrotated rectangle that produces the desired bounding box
- Ensuring only the dragged corner moves visually

