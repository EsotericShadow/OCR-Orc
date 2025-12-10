# Groups - Implementation Steps

## Objective

Enable users to organize regions into logical groups for better management.

## Prerequisites

- Region selection working
- DocumentState with group support
- UI layout with Groups tab

## Sub-Projects

1. **Group Management** - Create, edit, delete groups

## Steps

### Step 1: Implement Group Creation

1. Add method `createGroup(QString name, QList<QString> regionNames)`
2. Validate: at least 2 regions required
3. Check if group exists (ask to add or create new)
4. Add regions to group
5. Update region group assignments
6. Save to undo stack
7. Update UI (group listbox)

### Step 2: Implement Add to Group

1. Add method `addToGroup(QString regionName, QString groupName)`
2. Create group if doesn't exist
3. Add region to group
4. Update region group assignment
5. Save to undo stack
6. Update UI

### Step 3: Implement Remove from Group

1. Add method `removeFromGroup(QString regionName)`
2. Remove region from its group
3. Delete group if empty
4. Update region group assignment (set to empty)
5. Save to undo stack
6. Update UI

### Step 4: Implement Delete Group

1. Add method `deleteGroup(QString name)`
2. Remove group from groups map
3. Set all member regions' group to empty
4. Save to undo stack
5. Update UI

### Step 5: Update Group Listbox

1. Format: "Group Name (N regions)"
2. Alphabetically sorted
3. Update on group changes
4. Click to view group details

### Step 6: Connect UI Buttons

1. "Create Group" button → createGroup()
2. "Add to Group" button → addToGroup()
3. "Remove from Group" button → removeFromGroup()
4. "Delete Group" button → deleteGroup()

### Step 7: Add Keyboard Shortcuts

1. Cmd+G → Group selected regions
2. Cmd+Shift+G → Ungroup selected regions

## Acceptance Criteria

- [ ] Users can create groups
- [ ] Users can add regions to groups
- [ ] Users can remove regions from groups
- [ ] Users can delete groups
- [ ] Group listbox updates correctly
- [ ] Keyboard shortcuts work
- [ ] All operations save to undo stack
