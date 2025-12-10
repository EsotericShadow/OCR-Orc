# Unified Undo/Redo & State Sync Plan

## Principles
- **One snapshot per gesture:** saveState once at gesture start (press), not at release; no duplicate saves.
- **Editor edits:** saveState immediately before mutation (single-step).
- **Undo/Redo visibility:** after undo/redo, refresh lists/editor/canvas; clear or restore selection explicitly by policy.
- **Normalized is truth:** all coords derived from normalized after restore and after editor edits.

## Standard Patterns
### Gesture (move/resize/rotate)
1) On press/start: `documentState->saveState()` (pre-change snapshot).
2) Apply live changes during move/drag; no saves.
3) On release: apply final change only (no save).
4) Selection/editor refresh handled via `selectionChanged` or explicit update.

### Editor Mutations (name/color/group/type/fill/coords)
- `saveState()` immediately before mutation.
- After mutation: sync coords if needed, invalidate cache, update lists/editor, update undo/redo buttons.

### Creation / Deletion / Duplication / Group Ops
- Keep existing pattern: `saveState()` before mutation.
- Ensure post-restore UI refresh (lists, canvas) remains.

### Undo/Redo Flow
- `undoAction/redoAction` already push current to opposite stack and call `restoreState` (which syncs coords if image valid).
- Callers (MainWindow viewHandlers) should continue: update lists, canvas repaint, clear or restore selection (current behavior clears selection).

## Rotation/Resize Notes
- Geometry: anchor-based rotated resize per `ROTATED_RESIZE_DEFINITIVE_ANALYSIS.md` (handles at rotated unrotated-corners/midpoints). Keep math; change only snapshot timing.
- Rotation angle drift: avoid double save; rely on press snapshot only.

## Test Matrix (undo/redo focus)
1) Move: drag region → Cmd+Z reverts position → Cmd+Shift+Z reapplies.
2) Resize (unrotated): drag corner/edge → undo/redo toggles size/position.
3) Resize (rotated): rotate region then large/small resize → undo/redo stable center and angle.
4) Rotate: rotate region → undo returns angle to prior; redo re-applies.
5) Editor changes: name, color, group, type, percentage, coordinates → each undoable/redone.
6) Create/Delete/Duplicate: each undoable/redone; selection cleared per current policy.
7) Esc cancel: remains out-of-undo (acts as revert), acceptable; verify no stack corruption.
8) Restore refresh: after undo/redo lists/editor/canvas reflect state; coords synced (normalized→image→canvas).

## Implementation Checklist (when coding later)
- Move/Resize/Rotate: add pre-save at press/start; remove post-save at release; ensure rotation not double-saved.
- Keyboard moves/esc: decide whether to push undo snapshots or keep as transient revert; document policy.
- Ensure selectionChanged emitted appropriately after gesture to refresh editor.
- Keep editor pre-save pattern; no change needed.
