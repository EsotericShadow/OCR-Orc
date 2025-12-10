# Canvas Modularization Plan

## Overview
Canvas.cpp is currently 1791 lines and handles too many responsibilities. We need to break it into smaller, focused modules.

## Current Responsibilities
1. **Rendering** (~400 lines): paintEvent, drawDocumentImage, renderRegions, drawRegion, drawResizeHandles, drawRegionLabel, drawTempRectangle, drawSelectionBox
2. **Input Handling** (~600 lines): mousePressEvent, mouseMoveEvent, mouseReleaseEvent, mouseDoubleClickEvent, wheelEvent, keyPressEvent, contextMenuEvent
3. **Region Operations** (~400 lines): moveRegion, resizeRegion, selection management, duplicate, hit testing
4. **Zoom/Pan/Layout** (~300 lines): calculateLayout, zoom operations, pan operations
5. **Coordinate Caching** (~100 lines): coordinate cache management, batched updates

## Proposed Modules

### 1. CanvasRenderer
**Purpose**: Handle all painting and rendering operations

**Responsibilities**:
- paintEvent coordination
- drawDocumentImage
- renderRegions (with coordinate cache integration)
- drawRegion, drawResizeHandles, drawRegionLabel
- drawTempRectangle, drawSelectionBox
- getRegionColor

**Files**:
- `src/ui/CanvasRenderer.h`
- `src/ui/CanvasRenderer.cpp`

**Dependencies**:
- Canvas (for access to state)
- DocumentState
- CoordinateSystem

### 2. CanvasInputHandler
**Purpose**: Handle all user input (mouse, keyboard, wheel)

**Responsibilities**:
- mousePressEvent, mouseMoveEvent, mouseReleaseEvent, mouseDoubleClickEvent
- keyPressEvent, contextMenuEvent
- wheelEvent (delegates zoom to ZoomController, pan to LayoutManager)
- handleSelectModePress, handleSelectModeRelease
- startRegionCreation, updateRegionCreation, finishRegionCreation
- updateHoverState
- getRegionAt, getHandleAt, findRegionsInBox
- Selection management (clearSelection, selectRegion, toggleRegionSelection, addToSelection)

**Files**:
- `src/ui/CanvasInputHandler.h`
- `src/ui/CanvasInputHandler.cpp`

**Dependencies**:
- Canvas (for access to state and to emit signals)
- DocumentState
- CoordinateSystem

### 3. CanvasZoomController
**Purpose**: Handle zoom and pan operations

**Responsibilities**:
- zoomIn, zoomOut, zoomReset, setZoom, setZoomAtPoint, getZoom
- Pan operations (from wheel and mouse drag)
- calculateLayout (layout calculations)
- Coordinate with coordinate cache invalidation

**Files**:
- `src/ui/CanvasZoomController.h`
- `src/ui/CanvasZoomController.cpp`

**Dependencies**:
- Canvas (for access to state)
- DocumentState
- CoordinateSystem

### 4. CanvasCoordinateCache
**Purpose**: Manage coordinate caching for performance

**Responsibilities**:
- Coordinate cache storage and retrieval
- Cache invalidation
- Batched update management
- Integration with renderRegions

**Files**:
- `src/ui/CanvasCoordinateCache.h`
- `src/ui/CanvasCoordinateCache.cpp`

**Dependencies**:
- DocumentState
- CoordinateSystem

### 5. CanvasRegionOperations
**Purpose**: Handle region manipulation operations

**Responsibilities**:
- moveRegion, moveSelectedRegions
- resizeRegion, constrainToBounds
- duplicateSelectedRegions
- validateRegion

**Files**:
- `src/ui/CanvasRegionOperations.h`
- `src/ui/CanvasRegionOperations.cpp`

**Dependencies**:
- Canvas (for access to state)
- DocumentState
- CoordinateSystem

## Refactored Canvas Structure

After modularization, Canvas will:
- Own instances of the modular components
- Delegate operations to appropriate components
- Maintain public API (no breaking changes)
- Coordinate between components
- Emit signals for MainWindow

## Implementation Order

1. Create CanvasCoordinateCache (simplest, least dependencies)
2. Create CanvasRenderer (uses CoordinateCache)
3. Create CanvasZoomController (uses CoordinateCache)
4. Create CanvasRegionOperations
5. Create CanvasInputHandler (uses all other components)
6. Refactor Canvas to use all components

## Benefits

- **Maintainability**: Each module has a single, clear responsibility
- **Testability**: Modules can be tested independently
- **Readability**: Smaller, focused files are easier to understand
- **Reusability**: Components could be reused in other contexts
- **Performance**: Easier to optimize individual components

