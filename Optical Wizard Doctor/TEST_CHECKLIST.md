# Region Feature Test Checklist

## Core Region Operations

### 1. Region Creation
- [ ] Create region by dragging on canvas
- [ ] Create region with minimum size validation
- [ ] Create region that respects image boundaries
- [ ] Create region with different mouse modes

### 2. Region Selection
- [ ] Single click to select region
- [ ] Multiple region selection (Ctrl/Cmd + click)
- [ ] Box selection (drag to select multiple)
- [ ] Deselect all regions
- [ ] Selection visual feedback (highlight, handles)

### 3. Region Movement
- [ ] Drag region to move it
- [ ] Move region with keyboard arrows
- [ ] Move region respects image boundaries
- [ ] Move multiple selected regions together
- [ ] **Undo/Redo works for move operations**
- [ ] **Region editor coordinates update during move**

### 4. Region Resize (Unrotated)
- [ ] Resize by dragging corner handles
- [ ] Resize by dragging edge handles (n, s, e, w)
- [ ] Resize respects minimum size
- [ ] Resize respects image boundaries
- [ ] Opposite corner/edge stays fixed during resize
- [ ] **Undo/Redo works for resize operations**
- [ ] **Region editor coordinates update during resize**

### 5. Region Resize (Rotated) ⭐ CRITICAL
- [ ] Rotate region to various angles (0°, 30°, 45°, 90°, 135°, etc.)
- [ ] Resize rotated region by dragging corner handles
  - [ ] Small resize moves work correctly
  - [ ] Large resize moves work correctly
  - [ ] Opposite corner stays visually fixed
  - [ ] Center doesn't drift on large moves
- [ ] Resize rotated region by dragging edge handles
  - [ ] North/South edges (n, s handles)
  - [ ] East/West edges (e, w handles)
  - [ ] Opposite edge stays visually fixed
- [ ] Resize at extreme angles (near 45°, 90°, etc.)
- [ ] **Undo/Redo works for rotated resize operations**
- [ ] **Region editor coordinates update during rotated resize**

### 6. Region Rotation
- [ ] Rotate region using rotation icon
- [ ] Rotate region with keyboard shortcuts
- [ ] Rotation angle is preserved during resize
- [ ] Rotation angle is preserved during move
- [ ] **Undo/Redo works for rotation operations**
- [ ] **Region editor shows correct rotation angle**

### 7. Region Editor (Side Panel)
- [ ] Editor appears when region is selected
- [ ] Editor hides when no region selected
- [ ] Editor shows correct region name
- [ ] Editor shows correct coordinates (x1, y1, x2, y2)
- [ ] **Editor coordinates update when canvas changes region**
- [ ] **Canvas updates when editor coordinates change**
- [ ] Editor shows correct color
- [ ] Editor shows correct group
- [ ] Editor shows correct region type
- [ ] Editor shows correct percentage fill
- [ ] Editor shows correct rotation angle
- [ ] Changing name in editor updates region
- [ ] Changing color in editor updates region
- [ ] Changing group in editor updates region
- [ ] Changing type in editor updates region
- [ ] Changing percentage fill in editor updates region
- [ ] Changing coordinates in editor updates canvas
- [ ] Coordinate changes in editor respect boundaries

### 8. Undo/Redo System
- [ ] Undo works for region creation
- [ ] Undo works for region deletion
- [ ] **Undo works for region move** ✅
- [ ] **Undo works for region resize (unrotated)** ⚠️ FIXED
- [ ] **Undo works for region resize (rotated)** ⚠️ FIXED
- [ ] **Undo works for region rotation** ✅
- [ ] Redo works after undo
- [ ] Undo/Redo buttons update correctly
- [ ] Keyboard shortcuts (Cmd+Z, Cmd+Shift+Z) work

### 9. Coordinate System Synchronization
- [ ] Normalized coordinates are source of truth
- [ ] Image coordinates sync from normalized
- [ ] Canvas coordinates sync from normalized
- [ ] Zoom changes update canvas coordinates correctly
- [ ] Pan changes update canvas coordinates correctly
- [ ] **Canvas changes update normalized coordinates**
- [ ] **Editor changes update normalized coordinates**
- [ ] **Editor and canvas stay in sync**

### 10. Edge Cases & Boundary Conditions
- [ ] Resize region to minimum size
- [ ] Resize region at image boundaries
- [ ] Move region to image boundaries
- [ ] Rotate region at various angles
- [ ] Resize very small regions
- [ ] Resize very large regions
- [ ] Multiple rapid operations (stress test)
- [ ] Operations with multiple regions selected

### 11. Visual Feedback
- [ ] Resize handles appear on selected region
- [ ] Rotation icon appears on selected region
- [ ] Hover feedback on handles
- [ ] Cursor changes appropriately (resize, rotate, move)
- [ ] Selection highlight is visible
- [ ] Handles are visible on rotated regions

### 12. Performance
- [ ] Smooth dragging during resize
- [ ] Smooth dragging during move
- [ ] Smooth dragging during rotation
- [ ] No lag with many regions
- [ ] Editor updates don't cause lag

## Known Issues Fixed
- ✅ Undo/Redo for resize operations (was broken, now fixed)
- ✅ Region editor coordinate syncing (was broken, now fixed)
- ✅ Rotated resize center drift on large moves (was broken, now fixed)

## Testing Priority
1. **HIGH**: Rotated resize operations (corner and edge handles)
2. **HIGH**: Undo/Redo for resize operations
3. **HIGH**: Region editor coordinate syncing
4. **MEDIUM**: All other resize operations
5. **MEDIUM**: Move and rotation operations
6. **LOW**: Edge cases and performance


