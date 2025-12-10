# UI Module Architecture

This directory contains the modularized UI components for the OCR/ORC application. The architecture follows a layered approach with clear separation of concerns.

## Directory Structure

### `/canvas/` - Canvas Widget and Related Components
- **`Canvas.h/cpp`** - Main canvas widget, orchestrates rendering, input, and region management
- **`core/`** - Core canvas functionality (coordinate systems, rendering, region operations, selection, zoom)
- **`input/`** - Input handling (mouse, keyboard, hover, wheel)
- **`managers/`** - State and operation managers (region creation, zoom, state)
- **`ui/`** - UI-related utilities (context menu, UI synchronization)

### `/components/` - Reusable UI Components
- **`dialogs/`** - Dialog widgets (Export, Help, Preferences)
- **`icons/`** - Icon management
- **`widgets/`** - Reusable widgets (ControlPanel, ModeToggle, SidePanel, Toolbar)

### `/mainwindow/` - Main Window Components
- **`adapters/`** - Data and operation adapters
  - `UiDataAdapters` - Pure data transformations (DocumentState → UI models)
  - `MainWindowUndoRedoAdapter` - Unified undo/redo with UI refresh
- **`dialogs/`** - Dialog orchestration (`MainWindowDialogsController`)
- **`handlers/`** - Event and action handlers
  - `actions/` - Action handlers (export, load, etc.)
  - `regions/` - Region-related handlers (editor, group operations)
  - `view/` - View handlers (select all, invert selection, undo/redo)
- **`operations/`** - Business logic operations
  - `file/` - File operations
  - `group/` - Group operations
  - `region/` - Region operations
  - `selection/` - Selection operations
  - `undo/` - Undo/redo operations (legacy, being replaced by adapter)
- **`setup/`** - Setup and wiring
  - `MainWindowCanvasWiring` - Canvas signal/slot connections
  - `MainWindowUISetup` - UI widget creation
  - `MainWindowWidgetConnections` - Widget signal/slot connections
- **`shortcuts/`** - Keyboard shortcuts
- **`ui/`** - UI update utilities
  - `MainWindowToolbarAdapter` - Toolbar state updates
  - `MainWindowUIUpdates` - Region/group list updates

## UI Refresh Order

When the document state changes (e.g., after undo/redo, region operations), the following refresh sequence is enforced:

1. **Invalidate Coordinate Cache** - Clear canvas coordinate cache
2. **Update Region List** - Refresh region list box
3. **Update Group List** - Refresh group list box
4. **Repaint Canvas** - Trigger canvas repaint
5. **Update Toolbar Buttons** - Update undo/redo button states

This sequence is enforced by `CanvasUiSync::refreshAll()` and `MainWindowUndoRedoAdapter`.

## Key Design Patterns

### Adapters
- **Purpose**: Transform data or orchestrate operations with UI refresh
- **Examples**: `UiDataAdapters`, `MainWindowUndoRedoAdapter`, `MainWindowToolbarAdapter`
- **Naming**: `*Adapter` suffix

### Controllers
- **Purpose**: Orchestrate dialog launches and complex UI flows
- **Examples**: `MainWindowDialogsController`
- **Naming**: `*Controller` suffix

### Handlers
- **Purpose**: Handle specific events or actions
- **Examples**: `MainWindowActionHandlers`, `MainWindowRegionEditorHandlers`
- **Naming**: `*Handlers` suffix

### Operations
- **Purpose**: Encapsulate business logic operations
- **Examples**: `MainWindowFileOperations`, `MainWindowRegionOperations`
- **Naming**: `*Operations` suffix

### Wiring/Setup
- **Purpose**: Connect signals/slots and initialize UI components
- **Examples**: `MainWindowCanvasWiring`, `MainWindowWidgetConnections`
- **Naming**: `*Wiring`, `*Setup`, `*Connections` suffix

## Undo/Redo Pattern

All undo/redo operations should use `MainWindowUndoRedoAdapter::performUndo()` or `performRedo()`:

1. Check if operation is available
2. Call DocumentState undo/redo action
3. Clear selection (per current policy)
4. Refresh all UI components via `CanvasUiSync::refreshAll()`
5. Show status message

This ensures consistent behavior and prevents partial refresh issues.

## Data Flow

```
DocumentState (source of truth)
    ↓
UiDataAdapters (transform to UI models)
    ↓
MainWindowUIUpdates (update UI widgets)
    ↓
UI Widgets (display)
```

## State Synchronization

- **Canvas → DocumentState**: Canvas operations (create, move, resize, rotate) update DocumentState
- **DocumentState → UI**: State changes trigger UI refresh via `CanvasUiSync`
- **UI → DocumentState**: Editor changes update DocumentState (with `saveState()` before modification)

## Notes

- All new modules should follow the naming conventions above
- Keep responsibilities clear: one module, one concern
- Use callbacks for dependencies to reduce coupling
- Prefer static methods for stateless utilities (e.g., `CanvasUiSync`, `UiDataAdapters`)

