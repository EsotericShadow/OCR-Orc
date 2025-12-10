# UI Modularization Execution Plan

## Objectives
- Reduce coupling and file size: 3–4 related functions per file where practical.
- Separate concerns: input handling, rendering, state sync, UI refresh, dialog orchestration, data adapters.
- Preserve behavior; focus on structure and testability.

## Phase 1 — Inventory & Boundaries
- Hotspots to modularize:
  - Canvas/input: `src/ui/canvas/input/*`, `src/ui/canvas/core/*` (render/hit-test/ops).
  - MainWindow wiring: `src/ui/MainWindow.cpp`, `src/ui/mainwindow/**` (setup, handlers, operations).
  - Widgets: `src/ui/components/widgets/*` (Toolbar, SidePanel), dialogs, region editor.
- Dependencies to trace:
  - Shared state: `DocumentState`, `Canvas` selection/hover, coordinate cache.
  - Signals/slots: `Canvas::stateChanged`, `selectionChanged`, undo/redo actions, side-panel edits.
- Risks: hidden dependencies, missed signals. Mitigation: keep a single wiring map; avoid behavioral edits while splitting.

## Phase 2 — Canvas/Input Refinement
- Keep math/ops in `core/` unchanged.
- Maintain small gesture files (press/move/release already split); add a tiny helper only for shared cursor/update boilerplate if duplication appears.
- Keep hover/keyboard/wheel to ≤3–4 functions each; remove stray utilities.
- Risks: gesture regression. Mitigation: no math changes; reuse existing callbacks; rebuild/ctest after splits.

## Phase 3 — Canvas UI Sync Helper
- Add `CanvasUiSync` module to handle selectionChanged/stateChanged:
  - Order: invalidate coordinate cache → update region/group lists → repaint canvas → optionally refresh toolbar buttons.
  - Manage region editor show/hide on selection change.
- Risks: double refresh or stale UI. Mitigation: single entrypoint; document call order.

## Phase 4 — MainWindow Wiring Decomposition
- Split responsibilities into small adapters:
  - Construction/setup
  - Signal wiring (canvas, toolbar, side panel, dialogs)
  - Shortcuts
  - Drag/drop
  - Status/labels updates
  - Undo/redo UI adapter (invalidate cache → update canvas → lists → toolbar buttons)
- Risks: missed connections, circular deps. Mitigation: keep adapters stateless; one wiring hub; run build + integration test.

## Phase 5 — Side Panel / Region Editor Handlers
- Group handlers by concern:
  - Name/color/group
  - Type/fill
  - Coordinates
- Keep SidePanelWidget as view-only; move list population to a data adapter.
- Preserve saveState-before-mutate timing.
- Risks: editor stale or undo drift. Mitigation: mirror current ordering; checklist for saveState timing.

## Phase 6 — Toolbar Adapters
- Create tiny adapter for undo/redo button state and zoom label updates; ToolbarWidget remains a thin view with setters.
- Risks: buttons not reflecting state. Mitigation: ensure adapter is invoked after undo/redo and on stateChanged.

## Phase 7 — Dialogs Controller
- Centralize dialog launches (help/export/preferences/color/name edit) in `DialogsController`; dialogs stay dumb views.
- Pass dependencies explicitly (DocumentState/Canvas where needed).
- Risks: missing deps. Mitigation: thin controller, explicit ctor args.

## Phase 8 — UI Data Adapters
- Add `UiDataAdapters/` for pure transforms:
  - DocumentState → region list model (names, selected, group mapping)
  - DocumentState → group list model (name → size)
  - Selection sets and status strings
- No Qt widgets inside these adapters.
- Risks: shape mismatch. Mitigation: mirror current data structures; add light unit checks if feasible.

## Phase 9 — Undo/Redo Adapter (Command/Memento Alignment)
- Single adapter for undo/redo UI refresh:
  - Call DocumentState undo/redo
  - Then: invalidate cache → update canvas → update lists → update toolbar buttons
  - Selection clear/restoration per current policy.
- Risks: partial refresh. Mitigation: enforce sequence in adapter; reuse existing callbacks.

## Phase 10 — CMake/Tests Update
- Add new modules to build/test lists as splits land.
- Watch for duplicate poppler link warnings; adjust once if needed.
- Run build + ctest after each tranche.

## Phase 11 — Style & Documentation
- Add brief header comments for scope/responsibility.
- Keep consistent naming for adapters/controllers.
- Optional short README in `src/ui/` describing the modular layout and refresh order.

## Execution Notes
- Work in small tranches per module, preserving behavior.
- After each tranche: run build + ctest.
- Keep undo/redo save timing unchanged; avoid math changes in canvas ops.

