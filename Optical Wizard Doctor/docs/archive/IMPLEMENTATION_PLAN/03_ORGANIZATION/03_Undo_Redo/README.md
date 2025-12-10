# Undo/Redo - Implementation Steps

## Objective

Implement undo/redo system with 50-level history for all operations.

## Prerequisites

- All region operations working
- DocumentState ready
- UI with undo/redo buttons

## Sub-Projects

1. **State Management** - Save and restore application state

## Steps

### Step 1: Define State Snapshot Structure

1. Create `struct StateSnapshot`:
   ```cpp
   struct StateSnapshot {
       QMap<QString, RegionData> regions;
       QMap<QString, GroupData> groups;
   };
   ```
2. Contains deep copy of all state data
3. Serializable for potential persistence

### Step 2: Implement Save State Function

1. Create method `void saveState()`
2. Create deep copy of DocumentState:
   - Copy all regions (deep copy)
   - Copy all groups (deep copy)
3. Add to undo stack
4. Limit stack size to MAX_UNDO_LEVELS (50)
5. Clear redo stack (new action clears redo)
6. Update undo/redo button states

### Step 3: Implement Restore State Function

1. Create method `void restoreState(const StateSnapshot& state)`
2. Deep copy state data
3. Restore regions map
4. Restore groups map
5. Recalculate canvas coordinates for all regions
6. Update UI (listbox, canvas, info)
7. Clear selection

### Step 4: Implement Undo Function

1. Create method `void undoAction()`
2. Check if undo stack has items
3. Pop state from undo stack
4. Save current state to redo stack
5. Restore popped state
6. Update undo/redo button states
7. Redraw canvas

### Step 5: Implement Redo Function

1. Create method `void redoAction()`
2. Check if redo stack has items
3. Pop state from redo stack
4. Save current state to undo stack
5. Restore popped state
6. Update undo/redo button states
7. Redraw canvas

### Step 6: Add Undo/Redo Buttons

1. In toolbar, add Undo and Redo buttons
2. Connect to undoAction() and redoAction()
3. Set enabled/disabled based on stack state
4. Add tooltips

### Step 7: Update Button States

1. Create method `void updateUndoRedoButtons()`
2. Enable undo if undo stack has items
3. Enable redo if redo stack has items
4. Disable if stack is empty
5. Call after every state change

### Step 8: Save State Before Operations

1. Before region creation: saveState()
2. Before region movement (on release): saveState()
3. Before region resize (on release): saveState()
4. Before region deletion: saveState()
5. Before region rename: saveState()
6. Before color change: saveState()
7. Before group operations: saveState()

### Step 9: Add Keyboard Shortcuts

1. Cmd+Z (or Ctrl+Z): undoAction()
2. Cmd+Shift+Z (or Ctrl+Shift+Z): redoAction()
3. Cmd+Y (or Ctrl+Y): redoAction() (alternative)
4. Register in MainWindow

### Step 10: Optimize State Storage

1. Use efficient deep copy (Qt's implicit sharing helps)
2. Consider compression for large states (optional)
3. Limit stack size to prevent memory issues
4. Monitor memory usage

### Step 11: Test Undo/Redo

1. Test undo after region creation
2. Test undo after region movement
3. Test undo after region resize
4. Test undo after region deletion
5. Test undo after rename
6. Test undo after color change
7. Test undo after group operations
8. Test redo after undo
9. Test multiple undo/redo
10. Test stack limit (50 levels)

## Acceptance Criteria

- [ ] Undo works for all operations
- [ ] Redo works after undo
- [ ] Stack limit is enforced (50 levels)
- [ ] Button states update correctly
- [ ] Keyboard shortcuts work
- [ ] Memory usage is reasonable
- [ ] State restoration is accurate

## Code Structure

```cpp
class DocumentState {
    void saveState();
    void restoreState(const StateSnapshot& state);
    void undoAction();
    void redoAction();
    void updateUndoRedoButtons();
    
private:
    QList<StateSnapshot> undoStack;
    QList<StateSnapshot> redoStack;
    static constexpr int MAX_UNDO_LEVELS = 50;
};
```

## Notes

- Save state before operations, not during
- Deep copy is essential (Qt containers help)
- Clear redo stack on new action
- Test with many operations to ensure performance
