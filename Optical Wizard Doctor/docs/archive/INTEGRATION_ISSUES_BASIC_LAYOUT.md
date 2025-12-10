# Integration Issues: Basic Layout

## Overview

This document identifies potential integration issues when connecting the Basic Layout (Phase 1.7) with upcoming modules in Phase 2 and beyond.

---

## Phase 2: Interaction Module

### Issue 1: Canvas Widget Integration

**Problem:**
- Current canvas area uses a placeholder `QLabel`
- Phase 2 will need a custom `CanvasWidget` for PDF rendering and region drawing
- Need to replace placeholder without breaking layout

**Solution:**
- Create `CanvasWidget` class in Phase 2
- Replace placeholder in `setupCanvas()` method
- Ensure `CanvasWidget` inherits from `QWidget` and uses same layout constraints
- Test that canvas expands/contracts properly with window resize

**Prevention:**
- Already designed `canvasContainer` as separate widget
- Placeholder is easy to identify and replace
- Layout structure supports widget replacement

---

### Issue 2: Event Handling and Signal/Slot Connections

**Problem:**
- All toolbar/control panel buttons are created but not connected
- No event handlers for user interactions
- Need to connect buttons to functionality in Phase 2

**Solution:**
- Create slot methods in `MainWindow` for each button action
- Connect signals in separate `connectSignals()` method
- Use Qt's signal/slot mechanism for loose coupling
- Example:
  ```cpp
  connect(loadPdfButton, &QPushButton::clicked, this, &MainWindow::onLoadPdf);
  connect(createModeButton, &QRadioButton::toggled, this, &MainWindow::onModeChanged);
  ```

**Prevention:**
- All buttons are member variables, easy to connect
- Naming convention makes it clear what each button does
- Can add connections incrementally as features are implemented

---

### Issue 3: DocumentState Integration

**Problem:**
- `DocumentState` is created but not connected to UI
- UI needs to reflect document state (regions, groups, PDF loaded)
- Need bidirectional updates: UI → State and State → UI

**Solution:**
- Create observer pattern or use Qt signals
- Update UI when `DocumentState` changes (regions added, PDF loaded)
- Update `DocumentState` when UI actions occur (create region, delete region)
- Consider using `QAbstractItemModel` for list widgets (future optimization)

**Prevention:**
- `DocumentState` is already a member variable
- Can add update methods incrementally
- UI components are accessible for updates

---

### Issue 4: Coordinate System Integration

**Problem:**
- Canvas will need to use `CoordinateSystem` for transformations
- Toolbar zoom controls need to update canvas zoom
- Region creation needs coordinate transformations

**Solution:**
- Canvas widget will use `CoordinateSystem` static methods
- Store zoom factor in `DocumentState` or canvas widget
- Update zoom label when zoom changes
- Transform mouse coordinates using `CoordinateSystem`

**Prevention:**
- `CoordinateSystem` is already implemented and tested
- Zoom controls are in place, just need connection
- Coordinate transformations are stateless (easy to integrate)

---

## Phase 3: Organization Module

### Issue 5: Region/Group List Synchronization

**Problem:**
- `regionListBox` and `groupListBox` are placeholders
- Need to sync with `DocumentState` regions and groups
- Need to handle selection changes

**Solution:**
- Create methods to refresh lists from `DocumentState`
- Connect list selection to info display
- Update lists when regions/groups are added/removed
- Consider migrating to Model-View architecture if lists become complex

**Prevention:**
- List widgets are already created and accessible
- Can add refresh methods easily
- Structure supports incremental updates

---

### Issue 6: Info Display Updates

**Problem:**
- `infoText` is static placeholder
- Need to display region/group details when selected
- Need to format information clearly

**Solution:**
- Create `updateInfoDisplay(const RegionData&)` method
- Create `updateInfoDisplay(const GroupData&)` method
- Connect to list selection signals
- Format text with region/group properties

**Prevention:**
- `infoText` is already created and accessible
- Can add update methods easily
- Text widget supports rich text if needed

---

## Phase 4: Export/Import Module

### Issue 7: Export Button Functionality

**Problem:**
- Export buttons exist but don't do anything
- Need file dialogs for save/load
- Need to integrate with export/import classes

**Solution:**
- Use `QFileDialog` for file selection
- Create export/import classes (JSON, CSV, etc.)
- Connect buttons to export/import methods
- Show progress/status in status bar

**Prevention:**
- Buttons are already in place
- Can add functionality incrementally
- Status bar is available for feedback

---

### Issue 8: File Label Updates

**Problem:**
- `fileLabel` shows "No file loaded"
- Need to update when PDF is loaded
- Need to show filename or path

**Solution:**
- Update label in `onLoadPdf()` method
- Store filename in `DocumentState`
- Update label when document changes

**Prevention:**
- Label is already created and accessible
- Easy to update text property

---

## Phase 5: Polish Module

### Issue 9: Keyboard Shortcuts

**Problem:**
- Tooltips mention keyboard shortcuts (1, 2, Cmd+Z, etc.)
- Shortcuts are not implemented
- Need to add `QShortcut` or `QAction` with shortcuts

**Solution:**
- Create `QShortcut` objects for each shortcut
- Connect to same slots as buttons
- Document shortcuts in help system
- Ensure shortcuts don't conflict

**Prevention:**
- Tooltips already document intended shortcuts
- Can add shortcuts incrementally
- Qt's shortcut system is straightforward

---

### Issue 10: Help System Integration

**Problem:**
- Help button exists but doesn't do anything
- Need help dialog or documentation viewer
- Need to integrate with help system

**Solution:**
- Create help dialog in Phase 5
- Connect help button to dialog
- Include keyboard shortcuts, tooltips, usage instructions
- Consider using `QTextBrowser` for formatted help

**Prevention:**
- Help button is already in place
- Can add functionality in Phase 5
- UI structure supports help integration

---

## Cross-Cutting Concerns

### Issue 11: Undo/Redo Integration

**Problem:**
- Undo/Redo buttons are disabled
- Need command pattern or state history
- Need to track all user actions

**Solution:**
- Implement command pattern in Phase 2 or 3
- Store command history in `DocumentState` or separate class
- Enable/disable buttons based on history state
- Connect buttons to undo/redo methods

**Prevention:**
- Buttons are already in place
- Can implement incrementally
- Structure supports command pattern integration

---

### Issue 12: Status Bar Updates

**Problem:**
- Status bar shows "Ready"
- Need to update for various operations (loading, saving, errors)
- Need to show progress for long operations

**Solution:**
- Create `updateStatus(const QString&)` helper method
- Update status bar for all major operations
- Use `QProgressBar` for long operations (if needed)
- Clear status after timeout or next operation

**Prevention:**
- Status bar is already set up
- Can add update methods easily
- Qt's status bar supports progress indicators

---

### Issue 13: Mode Switching

**Problem:**
- Mode buttons (Create/Select) exist but don't affect behavior
- Canvas needs to respond to mode changes
- Different mouse behaviors for each mode

**Solution:**
- Store current mode in `DocumentState` or canvas widget
- Connect mode button signals to mode change handler
- Canvas widget checks mode before handling events
- Update cursor based on mode

**Prevention:**
- Mode buttons are already in place
- Can add mode state easily
- Structure supports mode-based behavior

---

## Testing Considerations

### Issue 14: UI Testing

**Problem:**
- No automated tests for UI components
- Manual testing required for layout
- Need to verify all buttons/widgets are accessible

**Solution:**
- Use Qt Test framework for UI testing
- Test button clicks, mode switching, layout resizing
- Verify widgets are created and visible
- Test on different screen sizes

**Prevention:**
- UI structure is simple and testable
- Can add tests incrementally
- Qt Test integrates well with Qt widgets

---

## Summary

### High Priority (Phase 2)
1. ✅ Canvas widget integration
2. ✅ Event handling connections
3. ✅ DocumentState integration
4. ✅ Coordinate system integration

### Medium Priority (Phase 3)
5. ✅ Region/group list synchronization
6. ✅ Info display updates

### Low Priority (Phase 4-5)
7. ✅ Export button functionality
8. ✅ File label updates
9. ✅ Keyboard shortcuts
10. ✅ Help system integration

### Cross-Cutting
11. ✅ Undo/redo integration
12. ✅ Status bar updates
13. ✅ Mode switching

**All issues are manageable and have clear solutions.** The current layout design supports incremental integration of all features.

