# Anchor-Based Resize: The Proven Solution

**The Problem We Were Solving Wrong**

Our previous approach had a fundamental flaw: **we were keeping the opposite corner fixed in UNROTATED space, but the user expects it fixed in VISUAL space.**

When you modify the unrotated rectangle (e.g., `setTopRight`), the center moves. When you then rotate around the NEW center, all corners shift in visual space—including the "opposite" corner that should have stayed fixed.

---

## The Core Insight

**Professional graphics software (Photoshop, Figma, etc.) keeps the opposite corner fixed in VISUAL space.**

This means: after the resize, when the rectangle is rendered (rotated around its center), the anchor corner must appear at exactly the same screen position as before.

This is a different problem than what we were solving.

---

## The Mathematical Solution (For Corner Handles)

### Given:
- **A** = Anchor visual position (opposite corner that should stay fixed in visual space)
- **D** = Dragged position (newPos - where user dragged to)
- **θ** = Rotation angle

### The Solution:

**Step 1: New center is the midpoint of the two visual corners**
```
C = (A + D) / 2
```

This is the key insight! For two opposite corners of a rectangle rotated around its center, the center is always the midpoint of those corners in visual space.

**Step 2: Calculate half-diagonal in visual space**
```
half_diag_visual = (D - A) / 2
```

**Step 3: Transform to unrotated space**
```
half_diag_unrot.x = half_diag_visual.x * cos(θ) + half_diag_visual.y * sin(θ)
half_diag_unrot.y = -half_diag_visual.x * sin(θ) + half_diag_visual.y * cos(θ)
```

**Step 4: Build the unrotated rectangle**
```
width  = 2 * |half_diag_unrot.x|
height = 2 * |half_diag_unrot.y|

x1 = C.x - |half_diag_unrot.x|
y1 = C.y - |half_diag_unrot.y|
x2 = C.x + |half_diag_unrot.x|
y2 = C.y + |half_diag_unrot.y|
```

### Proof of Correctness:

After rendering (rotating around center C by angle θ):
- Anchor visual = C + R(θ) * (anchor_unrot - C) = C + R(θ) * (-half_diag_unrot) = C - R(θ) * R(-θ) * half_diag_visual = C - half_diag_visual = A ✓
- Dragged visual = C + R(θ) * (dragged_unrot - C) = C + R(θ) * half_diag_unrot = C + half_diag_visual = D ✓

**Both corners land exactly where they should!**

---

## Corner Handle Mapping

| Handle | Dragged Corner | Anchor Corner |
|--------|---------------|---------------|
| `nw` | Top-left | Bottom-right |
| `ne` | Top-right | Bottom-left |
| `sw` | Bottom-left | Top-right |
| `se` | Bottom-right | Top-left |

---

## Implementation (Corner Handles)

```cpp
if (rotationAngle != 0.0) {
    double angleRad = rotationAngle * M_PI / 180.0;
    double cosA = std::cos(angleRad);
    double sinA = std::sin(angleRad);
    
    // Step 1: Determine anchor corner (opposite to dragged handle)
    // We need anchor in VISUAL space, so rotate the unrotated anchor
    QPointF unrotatedAnchor;
    if (handle == "nw") {
        unrotatedAnchor = unrotatedRect.bottomRight();
    } else if (handle == "ne") {
        unrotatedAnchor = unrotatedRect.bottomLeft();
    } else if (handle == "sw") {
        unrotatedAnchor = unrotatedRect.topRight();
    } else if (handle == "se") {
        unrotatedAnchor = unrotatedRect.topLeft();
    }
    
    // Rotate anchor to get its visual position
    QPointF oldCenter = unrotatedRect.center();
    QPointF anchorTranslated = unrotatedAnchor - oldCenter;
    QPointF anchorVisual = QPointF(
        anchorTranslated.x() * cosA - anchorTranslated.y() * sinA,
        anchorTranslated.x() * sinA + anchorTranslated.y() * cosA
    ) + oldCenter;
    
    // Step 2: New center = midpoint of anchor and dragged position
    QPointF newCenter = (anchorVisual + newPos) / 2.0;
    
    // Step 3: Half-diagonal in visual space
    QPointF halfDiagVisual = (newPos - anchorVisual) / 2.0;
    
    // Step 4: Transform to unrotated space (inverse rotation)
    QPointF halfDiagUnrot = QPointF(
        halfDiagVisual.x() * cosA + halfDiagVisual.y() * sinA,
        -halfDiagVisual.x() * sinA + halfDiagVisual.y() * cosA
    );
    
    // Step 5: Build new rectangle
    double halfWidth = std::abs(halfDiagUnrot.x());
    double halfHeight = std::abs(halfDiagUnrot.y());
    
    newRect = QRectF(
        newCenter.x() - halfWidth,
        newCenter.y() - halfHeight,
        halfWidth * 2.0,
        halfHeight * 2.0
    );
}
```

---

## Edge Handles (More Complex)

For edge handles (`n`, `s`, `e`, `w`), we want to move one edge while keeping the opposite edge fixed in visual space.

### The Constraint:
- The opposite edge midpoint must stay fixed in visual space
- The center moves perpendicular to the edge (in unrotated space)

### Algorithm for Edge Handles:

**Given:**
- **E** = Anchor edge midpoint in visual space (opposite edge)
- **D** = Where user dragged to (newPos)
- **θ** = Rotation angle

**For `n` and `s` handles:**
- We're moving in the Y direction (in unrotated space)
- The width stays constant
- We need to find the new height and position

**For `e` and `w` handles:**
- We're moving in the X direction (in unrotated space)
- The height stays constant
- We need to find the new width and position

### Implementation Sketch for Edge Handle "s":

```cpp
// Anchor = top edge midpoint visual position
QPointF topMidUnrot = QPointF(unrotatedRect.center().x(), unrotatedRect.top());
QPointF anchorTranslated = topMidUnrot - oldCenter;
QPointF anchorVisual = QPointF(
    anchorTranslated.x() * cosA - anchorTranslated.y() * sinA,
    anchorTranslated.x() * sinA + anchorTranslated.y() * cosA
) + oldCenter;

// Project newPos onto the line perpendicular to the edge
// The perpendicular direction in visual space is (sinA, cosA) for horizontal edges

// Calculate the distance from anchor to newPos along the perpendicular
QPointF delta = newPos - anchorVisual;
double perpDistance = delta.x() * sinA + delta.y() * cosA;  // dot product with perpendicular

// New height = |perpDistance|
// New center = anchor + perpendicular * perpDistance / 2
// (But we also need to account for the original half-height)

// This gets complex - full derivation needed
```

---

## Why Our Previous Approach Failed

Our old approach:
1. Calculated rotated handle position ✓
2. Measured visual delta ✓
3. Inverse-rotated delta ✓
4. Applied delta to unrotated rectangle using `setTopRight()` etc. ✗

The problem with step 4: `setTopRight()` keeps **bottom-left fixed in unrotated space**. But after rotation around the NEW center, bottom-left is NOT fixed in visual space.

**The center movement is the killer.**

When `setTopRight()` moves the top-right corner, the center moves toward the top-right. When we rotate around this new center, the bottom-left (which was "fixed" in unrotated space) moves in visual space.

---

## The Key Equations

For corner handles, the entire solution reduces to:

```
C_new = (A_visual + D) / 2

half_diag = R(-θ) * (D - A_visual) / 2

rect_new = centered at C_new, size = 2*|half_diag|
```

That's it. No bounding boxes. No singularities. No drift.

---

## References

- [Resizing Rotated Elements - shihn.ca](https://shihn.ca/posts/2020/resizing-rotated-elements/)
- The mathematical derivation is based on the constraint that rotation around center preserves the midpoint of opposite corners

---

## Summary

| Approach | Keeps Fixed | Result |
|----------|-------------|--------|
| Old (buggy) | Opposite corner in **unrotated space** | Visual drift, center shift causes all corners to move |
| Bounding box | Nothing (inverse solve) | Fails at 45°, wrong positions |
| **Anchor-based** | Opposite corner in **visual space** | ✓ Correct behavior |

The anchor-based approach is what Photoshop, Figma, and other professional tools use. The opposite corner truly stays fixed on screen.

