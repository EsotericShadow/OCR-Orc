# State Management - Implementation Steps

## Objective

Implement the core state management system for undo/redo functionality.

## Prerequisites

- DocumentState implemented
- Deep copy capability
- Qt containers (QMap, QList)

## Steps

### Step 1: Define State Snapshot Structure

1. Create `src/models/StateSnapshot.h`
2. Define structure:
   ```cpp
   struct StateSnapshot {
       QMap<QString, RegionData> regions;
       QMap<QString, GroupData> groups;
       
       // Optional: metadata
       QString pdfPath;
       QSize imageSize;
   };
   ```
3. Add equality operator for testing
4. Add serialization support (optional)

### Step 2: Implement Deep Copy

1. Qt containers (QMap, QList) use implicit sharing
2. For true deep copy:
   ```cpp
   StateSnapshot snapshot;
   snapshot.regions = regions;  // Qt does deep copy
   snapshot.groups = groups;    // Qt does deep copy
   ```
3. Qt automatically deep copies when needed
4. Test that modifications don't affect original

### Step 3: Create Undo/Redo Stacks

1. Add to DocumentState:
   ```cpp
   QList<StateSnapshot> undoStack;
   QList<StateSnapshot> redoStack;
   static constexpr int MAX_UNDO_LEVELS = 50;
   ```
2. Initialize as empty lists
3. Use QList for stack (push_back, pop_back)

### Step 4: Implement Save State

1. Create method `void saveState()`
2. Create snapshot:
   ```cpp
   StateSnapshot snapshot;
   snapshot.regions = regions;
   snapshot.groups = groups;
   snapshot.pdfPath = pdfPath;
   snapshot.imageSize = image.size();
   ```
3. Add to undo stack: `undoStack.append(snapshot)`
4. Limit stack size:
   ```cpp
   if (undoStack.size() > MAX_UNDO_LEVELS) {
       undoStack.removeFirst();  // Remove oldest
   }
   ```
5. Clear redo stack: `redoStack.clear()`
6. Update button states

### Step 5: Implement Restore State

1. Create method `void restoreState(const StateSnapshot& snapshot)`
2. Restore regions: `regions = snapshot.regions`
3. Restore groups: `groups = snapshot.groups`
4. Restore PDF path: `pdfPath = snapshot.pdfPath`
5. Recalculate canvas coordinates for all regions
6. Update UI:
   - Update region listbox
   - Update group listbox
   - Redraw canvas
   - Clear selection
   - Update info display

### Step 6: Implement Undo

1. Create method `void undoAction()`
2. Check if undo stack has items:
   ```cpp
   if (undoStack.isEmpty()) return;
   ```
3. Save current state to redo stack:
   ```cpp
   StateSnapshot current = createCurrentSnapshot();
   redoStack.append(current);
   ```
4. Pop from undo stack:
   ```cpp
   StateSnapshot previous = undoStack.takeLast();
   ```
5. Restore previous state: `restoreState(previous)`
6. Update button states

### Step 7: Implement Redo

1. Create method `void redoAction()`
2. Check if redo stack has items:
   ```cpp
   if (redoStack.isEmpty()) return;
   ```
3. Save current state to undo stack:
   ```cpp
   StateSnapshot current = createCurrentSnapshot();
   undoStack.append(current);
   ```
4. Pop from redo stack:
   ```cpp
   StateSnapshot next = redoStack.takeLast();
   ```
5. Restore next state: `restoreState(next)`
6. Update button states

### Step 8: Create Current Snapshot Helper

1. Create method `StateSnapshot createCurrentSnapshot() const`
2. Create snapshot from current state
3. Return snapshot
4. Used by undo/redo to save current state

### Step 9: Add State Saving to Operations

1. Before region creation: `saveState()`
2. Before region move (on mouse release): `saveState()`
3. Before region resize (on mouse release): `saveState()`
4. Before region delete: `saveState()`
5. Before region rename: `saveState()`
6. Before color change: `saveState()`
7. Before group create: `saveState()`
8. Before group delete: `saveState()`
9. Before add to group: `saveState()`
10. Before remove from group: `saveState()`

### Step 10: Update Button States

1. Create method `void updateUndoRedoButtons()`
2. Enable undo button if `!undoStack.isEmpty()`
3. Enable redo button if `!redoStack.isEmpty()`
4. Disable if stack is empty
5. Call after every state change

### Step 11: Handle Stack Overflow

1. Monitor stack size
2. Remove oldest state when limit reached
3. Log warning if stack gets too large
4. Consider memory usage

### Step 12: Test State Management

1. Test save state creates correct snapshot
2. Test restore state restores correctly
3. Test undo restores previous state
4. Test redo restores next state
5. Test stack limit (50 levels)
6. Test memory usage with many states
7. Test state isolation (modifications don't affect stack)

## Acceptance Criteria

- [ ] State snapshots are created correctly
- [ ] Deep copy works (modifications don't affect stack)
- [ ] Undo restores previous state accurately
- [ ] Redo restores next state accurately
- [ ] Stack limit is enforced
- [ ] Button states update correctly
- [ ] Memory usage is reasonable

## Code Structure

```cpp
struct StateSnapshot {
    QMap<QString, RegionData> regions;
    QMap<QString, GroupData> groups;
    QString pdfPath;
    QSize imageSize;
    
    bool operator==(const StateSnapshot& other) const;
};

class DocumentState {
    void saveState();
    void restoreState(const StateSnapshot& snapshot);
    StateSnapshot createCurrentSnapshot() const;
    
private:
    QList<StateSnapshot> undoStack;
    QList<StateSnapshot> redoStack;
};
```

## Notes

- Qt's implicit sharing makes deep copy efficient
- Save state before operations, not during
- Test with many operations to ensure performance
- Consider memory usage with large states

