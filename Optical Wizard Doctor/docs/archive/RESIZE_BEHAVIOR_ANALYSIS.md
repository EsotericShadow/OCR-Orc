# Resize Behavior Analysis - Rotated Regions

## Problem Description
When resizing a rotated region, all 4 sides appear to change visually, even though only one corner/edge should move.

## Log Analysis

### Test Case: Resize from top-right corner (handle "ne") of rotated region

**Handle**: "ne" (northeast/top-right)
**Expected Behavior**: Only top-right corner moves, bottom-left corner stays fixed
**Actual Behavior**: All corners appear to move visually

### Log Data

From the logs, when resizing from top-right corner:
- **x1 change**: 0 (left edge correctly fixed)
- **y1 change**: +0.00969697 (top-left Y is changing - **PROBLEM**)
- **x2 change**: +0.0854902 (right edge correctly moving)
- **y2 change**: 0 (bottom edge correctly fixed)

### Root Cause Analysis

1. **Normalized Coordinates Represent Unrotated Rectangle**
   - `normalizedCoords` stores the actual unrotated rectangle coordinates
   - When a region is rotated, the normalized coords still represent the unrotated rectangle
   - The visual bounding box is calculated during rendering by rotating the corners

2. **Resize Operation Logic**
   - We correctly transform the mouse drag point back to unrotated space
   - We correctly calculate the delta from the original handle position
   - We correctly apply the delta to only the dragged corner/edge
   - **BUT**: When we resize from top-right, we call `setTop()` which moves the entire top edge
   - This is correct for an unrotated rectangle (top edge is horizontal)
   - However, after rotation, the visual result makes it appear like all sides are moving

3. **The Real Issue**
   - The resize logic is working correctly in unrotated space
   - The problem is that when we resize a rotated rectangle, we're modifying the unrotated rectangle
   - After rotation, the visual bounding box changes, and it appears like all corners are moving
   - This is because the bounding box of a rotated rectangle is larger than the unrotated rectangle
   - When we resize the unrotated rectangle, the bounding box changes in a way that makes all corners appear to move

### Why This Happens

When you resize a rotated rectangle:
1. The unrotated rectangle changes (correctly)
2. The bounding box of the rotated rectangle changes
3. The visual representation (which shows the bounding box) makes it look like all corners moved
4. But in reality, only the dragged corner of the unrotated rectangle moved

### The Solution

We need to understand that:
- **Normalized coordinates** = unrotated rectangle (source of truth)
- **Visual bounding box** = calculated from rotated corners
- When resizing, we're modifying the unrotated rectangle
- The visual bounding box will change, but this is expected behavior

However, the user expects the visual behavior to match what they see - when they drag a corner handle, only that corner should move visually.

**The fix requires a different approach:**
- Instead of resizing the unrotated rectangle and then rotating it
- We need to resize the rotated rectangle directly
- Or, we need to calculate the new unrotated rectangle such that when rotated, only the dragged corner moves visually

This is a complex transformation problem that requires:
1. Transforming the drag point to the rotated coordinate system
2. Resizing in rotated space
3. Transforming back to unrotated space

Or alternatively:
1. Calculate which corner of the unrotated rectangle corresponds to the dragged handle
2. Resize the unrotated rectangle such that when rotated, only that corner moves visually
3. This requires inverse transformation of the visual bounding box to the unrotated rectangle

