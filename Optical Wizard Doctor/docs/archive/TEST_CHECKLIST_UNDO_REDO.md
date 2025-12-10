# Undo/Redo Functionality - Test Checklist

## Test Environment Setup
- [ ] Application is running
- [ ] PDF is loaded
- [ ] Undo/Redo buttons are visible in toolbar
- [ ] Buttons are initially disabled (no operations yet)

---

## Test 1: Basic Undo/Redo - Region Creation

### Steps:
1. Create a region (click and drag in Create mode)
2. Observe: Region appears, undo button should be enabled
3. Click Undo button (or press Cmd+Z)
4. Observe: Region should disappear, undo button disabled, redo button enabled
5. Click Redo button (or press Cmd+Shift+Z)
6. Observe: Region should reappear, redo button disabled, undo button enabled

### Expected Results:
- [ ] Undo removes created region
- [ ] Redo restores created region
- [ ] Button states update correctly
- [ ] Tooltips show correct stack depth

---

## Test 2: Multiple Operations - Undo Chain

### Steps:
1. Create Region 1
2. Create Region 2
3. Create Region 3
4. Click Undo (should remove Region 3)
5. Click Undo (should remove Region 2)
6. Click Undo (should remove Region 1)
7. Click Redo (should restore Region 1)
8. Click Redo (should restore Region 2)
9. Click Redo (should restore Region 3)

### Expected Results:
- [ ] Undo removes regions in reverse order (3, 2, 1)
- [ ] Redo restores regions in forward order (1, 2, 3)
- [ ] All regions restore with correct positions and properties
- [ ] Button states update correctly throughout

---

## Test 3: Undo/Redo - Region Movement

### Steps:
1. Create a region
2. Select the region
3. Hold Shift and drag the region to a new position
4. Release mouse
5. Click Undo (or Cmd+Z)
6. Observe: Region should return to original position
7. Click Redo (or Cmd+Shift+Z)
8. Observe: Region should move back to new position

### Expected Results:
- [ ] Undo restores original position
- [ ] Redo restores moved position
- [ ] Coordinates are correct after undo/redo
- [ ] Region is still selected after undo/redo

---

## Test 4: Undo/Redo - Region Resize

### Steps:
1. Create a region
2. Select the region (should show resize handles)
3. Drag a resize handle to make region larger
4. Release mouse
5. Click Undo (or Cmd+Z)
6. Observe: Region should return to original size
7. Click Redo (or Cmd+Shift+Z)
8. Observe: Region should return to resized dimensions

### Expected Results:
- [ ] Undo restores original size
- [ ] Redo restores resized dimensions
- [ ] Coordinates are correct after undo/redo
- [ ] Resize handles appear correctly

---

## Test 5: Undo/Redo - Region Duplication

### Steps:
1. Create a region named "Test Region"
2. Select the region
3. Press Cmd+D (or right-click → Duplicate)
4. Observe: Duplicate "Test Region_1" created
5. Click Undo (or Cmd+Z)
6. Observe: Duplicate should be removed
7. Click Redo (or Cmd+Shift+Z)
8. Observe: Duplicate should be restored

### Expected Results:
- [ ] Undo removes duplicate
- [ ] Redo restores duplicate
- [ ] Original region remains unchanged
- [ ] Region listbox updates correctly

---

## Test 6: Keyboard Shortcuts

### Steps:
1. Create a region
2. Press Cmd+Z (Undo)
3. Press Cmd+Shift+Z (Redo)
4. Press Cmd+Y (if implemented, alternative Redo)

### Expected Results:
- [ ] Cmd+Z triggers undo
- [ ] Cmd+Shift+Z triggers redo
- [ ] Shortcuts work from anywhere in application
- [ ] Buttons reflect keyboard actions

---

## Test 7: Button State Management

### Steps:
1. Observe initial state: Both buttons disabled
2. Create a region: Undo enabled, Redo disabled
3. Click Undo: Undo disabled, Redo enabled
4. Click Redo: Undo enabled, Redo disabled
5. Create another region: Undo enabled, Redo disabled (redo stack cleared)

### Expected Results:
- [ ] Buttons enable/disable correctly
- [ ] Tooltips show stack depth when enabled
- [ ] Tooltips show "no actions" when disabled
- [ ] New operation clears redo stack

---

## Test 8: Stack Limit (50 Levels)

### Steps:
1. Create 60 regions (one at a time)
2. Check undo count (should be 50, not 60)
3. Undo 50 times
4. Verify all regions are removed

### Expected Results:
- [ ] Stack limited to 50 levels
- [ ] Oldest states removed when limit exceeded
- [ ] Undo works correctly with 50-level stack

---

## Test 9: UI Updates After Undo/Redo

### Steps:
1. Create 3 regions with different names
2. Observe region listbox shows all 3
3. Click Undo
4. Observe: Region listbox updates (shows 2 regions)
5. Click Redo
6. Observe: Region listbox updates (shows 3 regions again)

### Expected Results:
- [ ] Region listbox updates immediately
- [ ] Group listbox updates (if groups exist)
- [ ] Canvas redraws correctly
- [ ] Selection cleared after undo/redo
- [ ] Status bar shows appropriate messages

---

## Test 10: Undo During Operation (Should Be Prevented)

### Steps:
1. Start dragging a region (Shift+drag)
2. While dragging, try to press Cmd+Z
3. Release mouse to finish drag
4. Try Cmd+Z again

### Expected Results:
- [ ] Undo should not work during drag (buttons disabled or ignored)
- [ ] Undo works after drag completes
- [ ] No crashes or errors

---

## Test 11: PDF Load Clears Stacks

### Steps:
1. Create some regions
2. Perform some operations (create, move, resize)
3. Verify undo stack has states
4. Load a new PDF
5. Check undo/redo buttons

### Expected Results:
- [ ] Undo/redo buttons disabled after loading new PDF
- [ ] Stacks are cleared
- [ ] No errors or crashes

---

## Test 12: Complex Scenario - Multiple Operations

### Steps:
1. Create Region 1
2. Move Region 1
3. Resize Region 1
4. Create Region 2
5. Duplicate Region 2
6. Undo 5 times (should undo in reverse: duplicate, create, resize, move, create)
7. Redo 5 times (should redo in forward order)

### Expected Results:
- [ ] Undo reverses all operations correctly
- [ ] Redo restores all operations correctly
- [ ] All regions have correct state after undo/redo
- [ ] No data corruption or inconsistencies

---

## Test 13: Edge Cases

### Steps:
1. Click Undo when stack is empty → Should do nothing, button disabled
2. Click Redo when stack is empty → Should do nothing, button disabled
3. Rapidly create and undo regions → Should handle correctly
4. Undo, then create new region → Redo should be disabled

### Expected Results:
- [ ] No crashes on empty stack operations
- [ ] Rapid operations handled correctly
- [ ] New operation clears redo stack
- [ ] Application remains stable

---

## Issues Found

### Critical Issues:
- [ ] (List any critical bugs here)

### Minor Issues:
- [ ] (List any minor issues here)

### Observations:
- [ ] (List any observations or notes here)

---

## Test Results Summary

**Date:** _______________  
**Tester:** _______________  
**Status:** ⬜ Pass  ⬜ Fail  ⬜ Partial

**Overall Assessment:**
- [ ] All tests passed
- [ ] Some tests failed (see issues above)
- [ ] Ready for production
- [ ] Needs fixes before proceeding

