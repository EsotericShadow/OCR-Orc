# MainWindowRegionOperations Module Plan

## Overview
Extract all region-related operations from `MainWindow.cpp` into a dedicated module to reduce complexity and improve maintainability.

## Current State Analysis

### Methods to Extract (~271 lines)
1. **Region Creation Workflow** (~59 lines)
   - `onRegionCreationRequested()` (41 lines) - Lines 1098-1138
   - `onRegionCreated()` (10 lines) - Lines 1140-1149
   - `onRegionsDuplicated()` (14 lines) - Lines 1151-1164

2. **Region Name Management** (~48 lines)
   - `generateRegionName()` (13 lines) - Lines 1166-1178
   - `autoIncrementRegionName()` (35 lines) - Lines 1180-1214

3. **Region Editing** (~145 lines)
   - `onEditSelected()` (91 lines) - Lines 951-1041
   - `showEditNameDialog()` (50 lines) - Lines ~900-949 (need to verify)
   - `showChangeColorDialog()` (54 lines) - Lines 1043-1096
   - Lambda handler for `regionEditRequested` signal (34 lines) - Lines 183-216
   - Lambda handler for `regionColorChangeRequested` signal (23 lines) - Lines 226-248

4. **Region Deletion** (~46 lines)
   - `onDeleteSelected()` (46 lines) - Lines 1402-1447

## Module Structure

### Header File: `MainWindowRegionOperations.h`
```cpp
namespace ocr_orc {
class MainWindowRegionOperations {
public:
    // Callback types for UI updates
    using UpdateRegionListBoxCallback = std::function<void()>;
    using UpdateGroupListBoxCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void()>;
    using ShowStatusMessageCallback = std::function<void(const QString&, int)>;
    using ShowWarningCallback = std::function<void(const QString&, const QString&)>;
    using ShowInformationCallback = std::function<void(const QString&, const QString&)>;
    using ShowQuestionCallback = std::function<int(const QString&, const QString&)>;
    using ClearSelectionCallback = std::function<void()>;
    using SelectRegionCallback = std::function<void(const QString&)>;
    using UpdateCanvasCallback = std::function<void()>;
    using SetRegionNameEditCallback = std::function<void(const QString&)>;
    using GetRegionNameCallback = std::function<QString()>;
    using GetSelectedColorCallback = std::function<QString()>;
    using GetGroupNameCallback = std::function<QString()>;
    using ClearGroupNameCallback = std::function<void()>;
    using FinishRegionCreationCallback = std::function<bool(const QString&, const QString&, const QString&)>;
    using HasRegionCallback = std::function<bool(const QString&)>;
    using SaveStateCallback = std::function<void()>;
    using RenameRegionCallback = std::function<bool(const QString&, const QString&)>;
    using ChangeRegionColorCallback = std::function<bool(const QString&, const QString&)>;
    using RemoveRegionCallback = std::function<void(const QString&)>;
    using GetSelectedRegionsCallback = std::function<QSet<QString>()>;
    using GetRegionCallback = std::function<RegionData(const QString&)>;
    using IsValidRegionNameCallback = std::function<bool(const QString&, const QString&)>;
    using SynchronizeCoordinatesCallback = std::function<void()>;
    
    // Region Creation
    void handleRegionCreationRequested(...);
    void handleRegionCreated(...);
    void handleRegionsDuplicated(...);
    
    // Name Management
    QString generateRegionName(DocumentState* documentState) const;
    void autoIncrementRegionName(...);
    
    // Region Editing
    void editSelected(...);
    QString showEditNameDialog(QWidget* parent, const QString& currentName) const;
    QString showChangeColorDialog(QWidget* parent, DocumentState* documentState, const QString& regionName) const;
    void handleRegionEditRequested(...);  // For signal handler
    void handleRegionColorChangeRequested(...);  // For signal handler
    
    // Region Deletion
    void deleteSelected(...);
};
}
```

## Implementation Strategy

### Phase 1: Dialog Methods (Low Risk)
**Priority: HIGH - Start here**
- Extract `showEditNameDialog()` - Pure UI, no dependencies
- Extract `showChangeColorDialog()` - Pure UI, minimal dependencies
- **Risk: LOW** - These are self-contained dialog builders
- **Estimated LOC: ~104 lines**

### Phase 2: Name Management (Low Risk)
**Priority: HIGH**
- Extract `generateRegionName()` - Simple logic
- Extract `autoIncrementRegionName()` - Uses regex, but isolated
- **Risk: LOW** - Well-defined inputs/outputs
- **Estimated LOC: ~48 lines**

### Phase 3: Region Creation (Medium Risk)
**Priority: MEDIUM**
- Extract `handleRegionCreationRequested()` - Complex workflow
- Extract `handleRegionCreated()` - Simple callback wrapper
- Extract `handleRegionsDuplicated()` - Simple callback wrapper
- **Risk: MEDIUM** - Multiple callbacks, state management
- **Estimated LOC: ~59 lines**

### Phase 4: Region Editing (High Risk)
**Priority: MEDIUM**
- Extract `editSelected()` - Complex validation and state updates
- Extract signal handlers for `regionEditRequested` and `regionColorChangeRequested`
- **Risk: HIGH** - Many dependencies, complex validation
- **Estimated LOC: ~145 lines**

### Phase 5: Region Deletion (Medium Risk)
**Priority: LOW**
- Extract `onDeleteSelected()` - Confirmation dialog + deletion
- **Risk: MEDIUM** - State management, multiple UI updates
- **Estimated LOC: ~46 lines**

## Dependencies

### External Dependencies
- `DocumentState` - Core data model
- `Canvas` - Canvas widget for region operations
- `ControlPanelWidget` - UI widget for region name/color input
- `RegionData` - Region data structure
- Qt Widgets: `QDialog`, `QLineEdit`, `QComboBox`, `QPushButton`, `QLabel`, `QVBoxLayout`, `QHBoxLayout`
- Qt Core: `QRegularExpression`, `QString`, `QSet`, `QList`

### Callback Dependencies (from MainWindow)
- UI Update callbacks (updateRegionListBox, updateGroupListBox, etc.)
- Canvas operations (clearSelection, selectRegion, update)
- DocumentState operations (saveState, renameRegion, etc.)
- Status bar updates
- Message box dialogs

## Risk Assessment

### High Risk Areas
1. **Region Creation Workflow** - Complex state management, multiple callbacks
2. **Region Editing** - Validation logic, undo/redo integration
3. **Signal Handlers** - Lambda functions that need to be converted to methods

### Mitigation Strategies
1. **Incremental Extraction** - Extract one method at a time, test after each
2. **Callback Pattern** - Use same pattern as `MainWindowFileOperations` (proven to work)
3. **Preserve Functionality** - Keep exact same logic, just move it
4. **Test After Each Phase** - Build and verify after each extraction phase

## Expected Outcomes

### Line Count Reduction
- **Current MainWindow.cpp**: 1783 lines
- **Code to Extract**: ~271 lines
- **Expected New MainWindow.cpp**: ~1512 lines
- **New Module**: ~271 lines (well under 500 LOC limit)

### Benefits
1. **Reduced Complexity** - MainWindow focuses on orchestration, not implementation
2. **Better Testability** - Region operations can be tested independently
3. **Improved Maintainability** - Region logic is isolated and easier to modify
4. **Consistent Pattern** - Follows same modularization approach as Canvas and FileOperations

## Implementation Order (Recommended)

1. ✅ **Phase 1: Dialog Methods** - Start with lowest risk
2. ✅ **Phase 2: Name Management** - Simple, isolated logic
3. ✅ **Phase 3: Region Creation** - Medium complexity
4. ✅ **Phase 4: Region Editing** - Most complex, do last
5. ✅ **Phase 5: Region Deletion** - Medium complexity

## Testing Checklist

After each phase:
- [ ] Build succeeds without errors
- [ ] No linter errors
- [ ] Region creation still works
- [ ] Region editing still works
- [ ] Region deletion still works
- [ ] Name generation works
- [ ] Auto-increment works
- [ ] Signal handlers work (double-click edit, color change)
- [ ] Undo/redo still works after region operations

## Notes

- Signal handlers (lambdas) will need to be converted to methods that MainWindow can call
- The `regionEditRequested` and `regionColorChangeRequested` signal connections in `setupCanvas()` will need to be updated to call module methods
- All callbacks must be passed from MainWindow to maintain UI update functionality
- Preserve all validation logic exactly as-is to avoid breaking existing behavior

