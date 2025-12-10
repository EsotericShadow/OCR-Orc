# Undo/Redo Current State & Gaps

## Snapshot of Operations & saveState Timing
- Rotate (mouse): saveState on press; saveState again on release after applying rotation → two snapshots per gesture; undo may jump to mid-state; angle drift observed.
- Resize (mouse): no save on press; saveState only on release → undo goes to post-change (no-op for user); large move drift plus missing undo.
- Move (mouse drag): no save on press; saveState only on release → undo goes to post-change (no-op).
- Drag-start with Shift on already-selected region: saveState before capturing dragStartState (good pre-snapshot) but not used for normal move; still missing press save in main path.
- Esc cancel (keyboard): restores stored start state via addRegion/dragStartState (no save; acts as revert), bypasses undo stack.
- Rotation icon hit test: saveState on press (good), but duplicate save on release.
- Editor changes:
  - Name: external handlers saveState before rename; rename itself does not save.
  - Color, Group, Type, Percentage, Coordinates: saveState before modification (recent fixes). 
- Creation: saveState before add (good). 
- Deletion: saveState before delete (good). 
- Duplication: saveState before duplicate (good). 
- Group ops (add/remove/delete): saveState before mutation (good). 

## State Restore & Sync Flow
- DocumentState::restoreState: restores regions/groups/pdfPath, reloads image if path valid, then synchronizeCoordinates() if image is valid; else clears coords.
- Undo/Redo buttons: MainWindow onUndo/onRedo call undoAction/redoAction, then update region list, group list, canvas repaint, clear selection.
- Selection/Editor refresh: Canvas emits selectionChanged on move/resize/moveSelected; MainWindow selectionChanged handler updates SidePanelWidget editor fields and hides/shows editor. 
- Coordinate cache invalidation: move/resize invalidate cache; editor coord changes also invalidate cache and call synchronizeCoordinates.

## Symptom Mapping
- Undo does not revert move/resize: missing pre-change saveState; only post-change snapshots exist.
- Rotation undo yields random angle: double save (press+release) causes undo to land on intermediate snapshot; angle tracked from live drag then saved again after mutation.
- Editor-driven changes: covered by pre-save (color/group/type/fill/coords); should be undoable.
- Esc cancel: bypasses undo stack (restores start state directly); acceptable but not tracked as undo history.

## Alignment with Prior Docs
- Rotation/resize geometry: anchor-based rotated resize per `ROTATED_RESIZE_DEFINITIVE_ANALYSIS.md`; no identified geometry regression here.
- Entropy is localized to undo/redo timing and UI refresh expectations, not the anchor-based math.

## Key Gaps to Fix (no code yet)
1) Standardize gesture pattern: saveState once at gesture start (press), not at release.
2) Remove duplicate saves on rotation release; ensure one snapshot per gesture.
3) Ensure move/resize have pre-change snapshots so undo reverts visual change.
4) After undo/redo, ensure selection/editor refresh occurs (currently mainwindow clears selection; acceptable but may need selection restore if desired).
