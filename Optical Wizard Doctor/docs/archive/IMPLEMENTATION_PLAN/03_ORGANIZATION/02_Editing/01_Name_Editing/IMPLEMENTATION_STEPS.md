# Name Editing - Implementation Steps

## Objective

Enable users to edit region names with validation and UI updates.

## Prerequisites

- Region selection working
- DocumentState ready
- Edit button in UI

## Steps

### Step 1: Create Edit Name Dialog

1. Create method `void showEditNameDialog(const QString& currentName)`
2. Create QDialog window:
   - Size: 300x100
   - Title: "Edit Region Name"
3. Add QLabel: "New Name:"
4. Add QLineEdit:
   - Pre-fill with current name
   - Select all text (ready to type)
   - Set focus
5. Add QPushButton: "Save"
6. Connect Enter key to save

### Step 2: Implement Name Validation

1. Create method `bool isValidRegionName(const QString& name)`
2. Check:
   - Name not empty
   - Name is unique (not already exists)
   - Name is different from current name
3. Return true if valid, false otherwise
4. Show appropriate error messages

### Step 3: Implement Rename Function

1. Create method in DocumentState: `void renameRegion(const QString& oldName, const QString& newName)`
2. Validate new name
3. Get region data
4. Remove old name from regions map
5. Add with new name to regions map
6. Update groups:
   - Find all groups containing old name
   - Replace old name with new name in group lists
7. Update region's internal name (if stored)
8. Save state to undo stack

### Step 4: Handle Rename in UI

1. Connect "Edit Name" button to edit function
2. Get selected region from listbox or canvas
3. Show edit dialog
4. On save:
   - Validate name
   - Call renameRegion()
   - Update region listbox
   - Update group listbox (if region in group)
   - Update info display
   - Redraw canvas (if name shown on region)

### Step 5: Support Multiple Edit Methods

1. **Double-click region** on canvas:
   - Detect double-click
   - Get region at click position
   - Show edit dialog
2. **Right-click → Edit Name**:
   - Show context menu
   - "Edit Name" option
   - Show edit dialog
3. **Select region → Press E key**:
   - Handle keyPressEvent
   - If E pressed and region selected, show dialog
4. **Select in listbox → Edit button**:
   - Get selection from listbox
   - Show edit dialog

### Step 6: Update Region Labels on Canvas

1. After rename, update region label text
2. Redraw canvas to show new name
3. Ensure label positioning is correct

### Step 7: Handle Rename Errors

1. **Duplicate name**:
   - Show error: "Region '[name]' already exists"
   - Keep dialog open
   - Highlight error
2. **Empty name**:
   - Show warning: "Region name cannot be empty"
   - Keep dialog open
3. **Same name**:
   - Close dialog (no change needed)

### Step 8: Update Group References

1. When region renamed, update all group references
2. Iterate through all groups
3. If group contains old name, replace with new name
4. Maintain group list order

### Step 9: Test Name Editing

1. Test rename via double-click
2. Test rename via right-click menu
3. Test rename via E key
4. Test rename via listbox button
4. Test duplicate name rejection
5. Test empty name rejection
6. Test rename updates groups
7. Test rename updates canvas labels
8. Test undo/redo for rename

## Acceptance Criteria

- [ ] Users can edit region names via multiple methods
- [ ] Name validation works (unique, not empty)
- [ ] Rename updates groups correctly
- [ ] Rename updates UI (listbox, canvas, info)
- [ ] Errors are handled gracefully
- [ ] Undo/redo works for rename

## Code Structure

```cpp
class MainWindow {
    void showEditNameDialog(const QString& currentName);
    void renameRegion(const QString& oldName, const QString& newName);
    bool isValidRegionName(const QString& name) const;
};

class DocumentState {
    void renameRegion(const QString& oldName, const QString& newName);
};
```

## Notes

- Region names are case-sensitive and unique
- Rename must update all references (groups, selections)
- Keep dialog simple and focused
- Provide clear error messages

