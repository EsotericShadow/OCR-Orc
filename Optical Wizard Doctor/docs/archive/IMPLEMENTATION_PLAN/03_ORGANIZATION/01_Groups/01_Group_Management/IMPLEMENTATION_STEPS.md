# Group Management - Implementation Steps

## Objective

Implement complete group management functionality: create, add to, remove from, and delete groups.

## Prerequisites

- Region selection working
- DocumentState with group support
- Groups tab in UI
- Undo/redo system ready

## Steps

### Step 1: Implement Create Group Function

1. Create method in DocumentState: `void createGroup(const QString& name, const QList<QString>& regionNames)`
2. Validate input:
   - Group name not empty
   - At least 2 regions required
   - All region names exist
3. Check if group already exists:
   - If exists, ask user: "Add to existing group or create new?"
   - If new, create group
4. Add regions to group:
   - Add to groups map: `groups[name] = regionNames`
   - Update each region's group field: `region['group'] = name`
5. Save state to undo stack
6. Update group listbox
7. Show success message

### Step 2: Implement Add to Group Function

1. Create method: `void addToGroup(const QString& regionName, const QString& groupName)`
2. Validate:
   - Region exists
   - Group name not empty
3. Create group if doesn't exist:
   ```cpp
   if (!groups.contains(groupName)) {
       groups[groupName] = QList<QString>();
   }
   ```
4. Add region to group (if not already in group):
   - Add to groups list
   - Update region's group field
5. Save state to undo stack
6. Update group listbox

### Step 3: Implement Remove from Group Function

1. Create method: `void removeFromGroup(const QString& regionName)`
2. Get region's current group
3. If region has a group:
   - Remove from groups list
   - Set region's group field to empty (null or empty string)
   - If group becomes empty, delete it
4. Save state to undo stack
5. Update group listbox

### Step 4: Implement Delete Group Function

1. Create method: `void deleteGroup(const QString& name)`
2. Validate group exists
3. For each region in group:
   - Set region's group field to empty
4. Remove group from groups map
5. Save state to undo stack
6. Update group listbox
7. Clear info display if deleted group was selected

### Step 5: Add Group Creation UI

1. In Groups tab, add "Create Group" button
2. On click:
   - Get selected regions from region listbox
   - Check at least 2 regions selected
   - Show input dialog for group name
   - Call createGroup()
3. Handle errors (show warnings)

### Step 6: Add Add to Group UI

1. Add "Add to Group" button
2. On click:
   - Get selected regions from region listbox
   - Show input dialog for group name
   - For each selected region, call addToGroup()
3. Show success message with count

### Step 7: Add Remove from Group UI

1. Add "Remove from Group" button
2. On click:
   - Get selected regions from region listbox
   - For each region, call removeFromGroup()
3. Show success message with count

### Step 8: Add Delete Group UI

1. Add "Delete Group" button (red background)
2. On click:
   - Get selected group from group listbox
   - Show confirmation dialog
   - If confirmed, call deleteGroup()
3. Show success message

### Step 9: Update Group Listbox

1. Create method `void updateGroupList()`
2. Clear listbox
3. For each group:
   - Format: "Group Name (N regions)"
   - Add to listbox
4. Sort alphabetically
5. Call after any group operation

### Step 10: Implement Group Selection

1. Connect group listbox selection to info display
2. On selection:
   - Get group name
   - Get group data
   - Display group info:
     - Group name
     - Number of regions
     - List of region names
3. Update info text widget

### Step 11: Add Keyboard Shortcuts

1. Cmd+G (or Ctrl+G):
   - Get selected regions from listbox
   - Show group name dialog
   - Call createGroup()
2. Cmd+Shift+G (or Ctrl+Shift+G):
   - Get selected regions from listbox
   - For each, call removeFromGroup()
3. Register shortcuts in MainWindow

### Step 12: Handle Group-Region Synchronization

1. When region is deleted:
   - Remove from its group
   - Delete group if empty
2. When region name is changed:
   - Update region name in group's region list
3. When group is deleted:
   - Set all member regions' group to empty

### Step 13: Test Group Management

1. Test create group with 2+ regions
2. Test add region to existing group
3. Test add region to new group
4. Test remove region from group
5. Test delete group
6. Test group listbox updates
7. Test keyboard shortcuts
8. Test undo/redo for group operations
9. Test edge cases (empty groups, duplicate names)

## Acceptance Criteria

- [ ] Users can create groups from selected regions
- [ ] Users can add regions to groups
- [ ] Users can remove regions from groups
- [ ] Users can delete groups
- [ ] Group listbox displays correctly
- [ ] Group info displays when selected
- [ ] Keyboard shortcuts work
- [ ] All operations save to undo stack
- [ ] Group-region synchronization works
- [ ] Edge cases handled gracefully

## Code Structure

```cpp
class DocumentState {
    void createGroup(const QString& name, const QList<QString>& regionNames);
    void addToGroup(const QString& regionName, const QString& groupName);
    void removeFromGroup(const QString& regionName);
    void deleteGroup(const QString& name);
    QList<QString> getGroupNames() const;
    GroupData getGroup(const QString& name) const;
};
```

## Notes

- Groups are case-sensitive
- Group names must be unique
- Regions can belong to at most one group
- Empty groups are automatically deleted
- All operations must save to undo stack

