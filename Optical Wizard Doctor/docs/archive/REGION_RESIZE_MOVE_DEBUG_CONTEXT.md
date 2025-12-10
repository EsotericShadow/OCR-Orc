# Region Resize & Move Debug Context

## Overview

This document provides comprehensive context for debugging region resizing and moving issues in the OCR-Orc application. The user has reported glitching issues with:
- **Region Resizing**: Resize handles not working correctly
- **Region Moving**: Dragging/moving regions causing glitches

## Application Architecture

### Canvas System
The application uses a modular canvas system located in `src/ui/canvas/`:

```
canvas/
├── Canvas.h/cpp                    # Main canvas widget
├── core/
│   ├── coordinate/                 # Coordinate transformation
│   │   ├── CanvasCoordinateCache.h/cpp
│   │   └── CanvasHitTester.h/cpp
│   ├── regions/                    # Region operations
│   │   ├── CanvasRegionCreator.h/cpp
│   │   └── CanvasRegionOperations.h/cpp
│   ├── rendering/                 # Rendering logic
│   │   └── CanvasRenderer.h/cpp
│   ├── selection/                 # Selection management
│   │   └── CanvasSelectionManager.h/cpp
│   └── zoom/                      # Zoom controls
│       └── CanvasZoomController.h/cpp
├── input/                         # Input event handlers
│   ├── CanvasInputHandler.h/cpp
│   ├── CanvasMouseHandler.h/cpp
│   ├── CanvasKeyboardHandler.h/cpp
│   ├── CanvasWheelHandler.h/cpp
│   └── CanvasHoverManager.h/cpp
├── managers/                      # State management
│   ├── CanvasRegionManager.h/cpp
│   ├── CanvasRegionCreationManager.h/cpp
│   ├── CanvasZoomManager.h/cpp
│   └── CanvasStateManager.h/cpp
└── ui/                            # UI components
    └── CanvasContextMenuBuilder.h/cpp
```

### Key Components for Region Interaction

1. **CanvasMouseHandler** (`src/ui/canvas/input/CanvasMouseHandler.h/cpp`)
   - Handles mouse press, move, release events
   - Manages region selection, dragging, and resizing

2. **CanvasHitTester** (`src/ui/canvas/core/coordinate/CanvasHitTester.h/cpp`)
   - Tests if mouse clicks hit regions or resize handles
   - Determines which region/handle is being interacted with

3. **CanvasRegionOperations** (`src/ui/canvas/core/regions/CanvasRegionOperations.h/cpp`)
   - Contains logic for moving and resizing regions
   - Updates region coordinates

4. **CanvasSelectionManager** (`src/ui/canvas/core/selection/CanvasSelectionManager.h/cpp`)
   - Manages selected regions
   - Tracks primary selected region (for resize handles)

5. **CanvasCoordinateCache** (`src/ui/canvas/core/coordinate/CanvasCoordinateCache.h/cpp`)
   - Caches coordinate transformations
   - Converts between normalized, image, and canvas coordinates

## Coordinate Systems

The application uses three coordinate systems:

1. **Normalized Coordinates** (0.0 - 1.0): Source of truth, resolution-independent
2. **Image Coordinates**: Pixel coordinates on the source image
3. **Canvas Coordinates**: Pixel coordinates on the canvas widget (includes zoom/pan)

Transformations are handled by `CoordinateSystem` (`src/core/CoordinateSystem.h/cpp`).

## Current Issues

### Reported Problems

1. **Region Resizing Glitches**
   - Resize handles not responding correctly
   - Regions jumping or snapping incorrectly during resize
   - Resize not updating coordinates properly

2. **Region Moving Glitches**
   - Dragging regions causes visual glitches
   - Regions not moving smoothly
   - Coordinates not updating correctly during drag

### Potential Root Causes

1. **Coordinate Transformation Issues**
   - Incorrect conversion between coordinate systems during resize/move
   - Zoom/pan not accounted for properly
   - Cache invalidation issues

2. **Event Handling Issues**
   - Mouse events not being processed correctly
   - State management problems (drag state, resize state)
   - Multiple handlers interfering with each other

3. **Selection State Issues**
   - Primary selected region not set correctly
   - Selection state not updating during operations
   - Multiple regions selected causing conflicts

4. **Rendering Issues**
   - Visual updates not syncing with coordinate updates
   - Dirty region updates not working correctly
   - Repaint issues during drag/resize

## Key Files to Investigate

### Primary Files
1. `src/ui/canvas/input/CanvasMouseHandler.h/cpp`
   - Mouse event handling
   - Drag and resize logic
   - **Key Methods**: `mousePressEvent()`, `mouseMoveEvent()`, `mouseReleaseEvent()`
   - **State**: Tracks drag state, resize handle, initial positions

2. `src/ui/canvas/core/coordinate/CanvasHitTester.h/cpp`
   - Hit testing for regions and handles
   - Resize handle detection
   - **Key Methods**: `hitTestRegion()`, `hitTestResizeHandle()`
   - **Returns**: Which region/handle was clicked

3. `src/ui/canvas/core/regions/CanvasRegionOperations.h/cpp`
   - Region move/resize operations
   - Coordinate updates
   - **Key Methods**: `moveRegion()`, `resizeRegion()`
   - **Updates**: Normalized coordinates, syncs all coordinate systems

4. `src/ui/canvas/Canvas.h/cpp`
   - Main canvas widget
   - Event routing and coordination
   - **Key Methods**: `paintEvent()`, `mousePressEvent()`, `mouseMoveEvent()`
   - **Coordinates**: Routes events to handlers

### Supporting Files
5. `src/ui/canvas/core/coordinate/CanvasCoordinateCache.cpp`
   - Coordinate caching and transformation

6. `src/ui/canvas/core/selection/CanvasSelectionManager.cpp`
   - Selection state management

7. `src/core/CoordinateSystem.cpp`
   - Coordinate transformation functions

## Mouse Modes

The application has two mouse modes (set via toolbar):

1. **Create Mode**: Click and drag to create new regions
2. **Select/Move Mode**: Click to select, drag to move/resize regions

Mode is tracked in `Canvas` and affects mouse event handling.

## Resize Handle System

Resize handles are displayed for the **primary selected region** (when exactly one region is selected). Handles are typically at:
- 8 positions: corners (4) + edges (4)
- Or 4 positions: corners only

Handle positions are calculated in canvas coordinates and need to be hit-tested correctly.

## Region Data Structure

Regions are stored in `DocumentState` (`src/models/DocumentState.h/cpp`) as `RegionData` objects (`src/models/RegionData.h/cpp`).

Each `RegionData` contains:
- `name`: Region identifier
- `normalizedCoords`: Normalized coordinates (source of truth)
- `imageCoords`: Image pixel coordinates
- `canvasCoords`: Canvas pixel coordinates
- `color`: Display color

Coordinates should be kept in sync via `syncFromNormalized()`.

## Debugging Strategy

### Step 1: Identify the Issue
1. Test region resizing in Select/Move mode
2. Test region moving in Select/Move mode
3. Note specific glitch behaviors:
   - Does it happen on mouse press, move, or release?
   - Does it happen with single or multiple regions?
   - Does zoom level affect it?
   - Are coordinates updating correctly?

### Step 2: Trace Event Flow
1. Check `CanvasMouseHandler::mousePressEvent()`
   - Is hit testing working?
   - Is drag/resize state initialized correctly?

2. Check `CanvasMouseHandler::mouseMoveEvent()`
   - Are coordinates being updated correctly?
   - Is the visual update happening?

3. Check `CanvasMouseHandler::mouseReleaseEvent()`
   - Are final coordinates saved correctly?
   - Is state cleaned up properly?

### Step 3: Check Coordinate Transformations
1. Verify coordinate conversions during resize/move
2. Check if zoom/pan are accounted for
3. Verify cache invalidation when coordinates change

### Step 4: Check Selection State
1. Verify `primarySelectedRegion` is set correctly
2. Check if selection state interferes with operations
3. Verify `setSelectedRegions()` updates `primarySelectedRegion`

## Known Code Locations

### Resize Handle Detection
- `CanvasHitTester::getHandleAt()` (line 63-91 in CanvasHitTester.cpp)
  - Detects which handle is clicked (8 handles: nw, n, ne, e, se, s, sw, w)
  - Uses distance calculation with tolerance
  - Returns handle name string or empty
  
- `CanvasRenderer::drawResizeHandles()` - Renders resize handles
- `CanvasMouseHandler::handlePress()` (line 94-121) - Checks for resize handle on press

### Region Moving
- `CanvasRegionOperations::moveRegion()` (line 14-61 in CanvasRegionOperations.cpp)
  - Converts delta from canvas → image → normalized coordinates
  - Updates normalized coordinates (source of truth)
  - Calls `syncFromNormalized()` to update all coordinate systems
  
- `CanvasMouseHandler::handleMove()` (line 81-116 in CanvasMouseHandler.h)
  - Handles drag during move
  - Calls `moveSelectedRegions()` callback

### Region Resizing
- `CanvasRegionOperations::resizeRegion()` (line 75-220 in CanvasRegionOperations.cpp)
  - Takes handle name and new position
  - Calculates new rectangle based on handle
  - Enforces minimum size constraints
  - Clamps to image bounds
  - Converts back to normalized coordinates
  
- `CanvasMouseHandler::handleMove()` (line 195-199) - Calls `resizeRegion()` callback during drag
- `CanvasMouseHandler::handlePress()` (line 118-120) - Sets resize state on handle click

### Selection Management
- `CanvasSelectionManager::setPrimarySelectedRegion()` - Sets primary selection
- `Canvas::setSelectedRegions()` (line 90 in Canvas.h) - Updates selection
  - **CRITICAL**: Should set `primarySelectedRegion` if exactly one region selected
  - Currently may not be updating `primarySelectedRegion` correctly

### State Variables (in Canvas class)
- `isResizing` (bool) - Whether currently resizing
- `resizingRegion` (QString) - Name of region being resized
- `resizeHandle` (QString) - Which handle ("nw", "ne", etc.)
- `resizeStartRect` (QRectF) - Original rectangle before resize
- `resizeStartRegionData` (RegionData) - Original region data (for cancel)
- `isDragging` (bool) - Whether currently dragging
- `dragStartPos` (QPointF) - Initial drag position
- `lastDragPos` (QPointF) - Last drag position
- `primarySelectedRegion` (QString) - Primary selected region (for resize handles)

## Testing Checklist

When debugging, test these scenarios:

- [ ] Resize single region from each handle (8 handles)
- [ ] Move single region
- [ ] Resize with multiple regions selected
- [ ] Move with multiple regions selected
- [ ] Resize at different zoom levels (50%, 100%, 200%)
- [ ] Move at different zoom levels
- [ ] Resize after panning
- [ ] Move after panning
- [ ] Switch between Create and Select/Move modes
- [ ] Resize immediately after creating a region
- [ ] Undo/Redo after resize/move operations

## Recent Changes

The application has undergone significant modularization:
- Canvas code split into multiple modules
- Input handling separated into specialized handlers
- Coordinate caching added for performance
- Selection management extracted to separate class

These changes may have introduced bugs or broken existing functionality.

## Build Information

- **Build System**: CMake
- **Qt Version**: Qt6
- **C++ Standard**: C++23
- **Build Directory**: `build/`
- **Binary Location**: `build/bin/owd.app/Contents/MacOS/owd`

## Quick Start Commands

```bash
# Build the application
cd "/Users/main/Desktop/OCR-Orc-OCR_Coordinates/OCR-Orc"
cmake --build build --target owd

# Run the application
./build/bin/owd.app/Contents/MacOS/owd

# Run tests
cd build && ctest --output-on-failure
```

## Related Documentation

- `docs/UI_UX_DESIGN.md` - UI design documentation
- `docs/ICON_INVENTORY.md` - Icon usage documentation
- `src/ui/canvas/Canvas.h` - Main canvas class documentation

## Next Steps for Debugging

1. **Reproduce the Issue**
   - Run the application
   - Create a region
   - Switch to Select/Move mode
   - Try resizing and moving
   - Document exact glitch behavior

2. **Add Debug Logging**
   - Add `qDebug()` statements in mouse event handlers
   - Log coordinate transformations
   - Log selection state changes
   - Log resize handle detection

3. **Check State Management**
   - Verify drag state is managed correctly
   - Check if resize state persists correctly
   - Ensure state cleanup on mouse release

4. **Verify Coordinate Updates**
   - Check if normalized coordinates update correctly
   - Verify image/canvas coordinates sync
   - Check cache invalidation

5. **Test Edge Cases**
   - Very small regions
   - Very large regions
   - Regions at canvas edges
   - Multiple rapid operations

## Code Patterns to Look For

### Common Issues

1. **Coordinate System Confusion**
   ```cpp
   // BAD: Mixing coordinate systems
   QPoint canvasPos = event->pos();
   region->normalizedCoords.x = canvasPos.x(); // WRONG!
   
   // GOOD: Convert properly
   QPoint canvasPos = event->pos();
   NormalizedCoords norm = coordinateSystem->canvasToNormalized(canvasPos, ...);
   region->normalizedCoords = norm;
   ```

2. **State Not Cleared**
   ```cpp
   // BAD: State persists
   void mouseReleaseEvent() {
       // Forgot to clear dragState
   }
   
   // GOOD: Clear state
   void mouseReleaseEvent() {
       dragState = DragState::None;
       resizeHandle = ResizeHandle::None;
   }
   ```

3. **Cache Not Invalidated**
   ```cpp
   // BAD: Cache not updated
   region->normalizedCoords.x = newX;
   // Cache still has old coordinates!
   
   // GOOD: Invalidate cache
   region->normalizedCoords.x = newX;
   coordinateCache->invalidateRegion(region->name);
   ```

## Critical Code Sections

### Canvas::setSelectedRegions() - POTENTIAL BUG
This method should set `primarySelectedRegion` when exactly one region is selected. Check if this is implemented correctly.

**Location**: `src/ui/canvas/Canvas.cpp` (around line 380-450)

**Expected Behavior**:
```cpp
void Canvas::setSelectedRegions(const QSet<QString>& regionNames) {
    selectedRegions = regionNames;
    
    // CRITICAL: Set primary selected region for resize handles
    if (selectedRegions.size() == 1) {
        primarySelectedRegion = *selectedRegions.begin();
    } else {
        primarySelectedRegion = QString(); // Clear if multiple or none
    }
    
    update(); // Trigger repaint
}
```

### CanvasMouseHandler::handleMove() - Resize Logic
**Location**: `src/ui/canvas/input/CanvasMouseHandler.cpp` (around line 195-199)

**Current Code Pattern**:
```cpp
if (isResizing) {
    resizeRegion(resizingRegion, resizeHandle, canvasPos);
}
```

**Potential Issues**:
- `canvasPos` might not be converted correctly
- `resizeStartRect` might be in wrong coordinate system
- Coordinate conversion chain might be broken

### CanvasRegionOperations::resizeRegion() - Coordinate Conversion
**Location**: `src/ui/canvas/core/regions/CanvasRegionOperations.cpp` (line 75-220)

**Key Steps**:
1. Calculate new rectangle from handle and new position
2. Enforce minimum size
3. Clamp to image bounds
4. Convert canvas rect → normalized coords
5. Call `syncFromNormalized()` to update all coordinate systems

**Potential Issues**:
- `resizeStartRect` might be in wrong coordinate system
- Conversion from canvas to normalized might be incorrect
- `syncFromNormalized()` might not be called or might fail

## Questions to Answer

When debugging, answer these questions:

1. **When does the glitch occur?**
   - On mouse press?
   - During mouse move?
   - On mouse release?
   - After operation completes?

2. **What type of glitch?**
   - Visual (region jumps on screen)?
   - Functional (coordinates wrong)?
   - Both?

3. **What triggers it?**
   - Specific resize handle?
   - Specific zoom level?
   - Multiple regions selected?
   - After specific operations?

4. **Is it consistent?**
   - Happens every time?
   - Intermittent?
   - Only in specific scenarios?

5. **Coordinate System Issues?**
   - Are coordinates being converted correctly?
   - Is `resizeStartRect` in the right coordinate system?
   - Are normalized coordinates being updated?
   - Is `syncFromNormalized()` being called?

## Coordinate Conversion Chain

### During Resize
1. **Mouse Position**: Widget coordinates (from `QMouseEvent::pos()`)
2. **Convert to Canvas**: `widgetToCanvas(widgetPos)` → Canvas coordinates
3. **Calculate New Rect**: Based on `resizeStartRect` (canvas coords) + handle + new position
4. **Convert to Normalized**: Canvas rect → Image coords → Normalized coords
5. **Update Region**: Set normalized coords, call `syncFromNormalized()`

### During Move
1. **Mouse Position**: Widget coordinates
2. **Convert to Canvas**: `widgetToCanvas(widgetPos)`
3. **Calculate Delta**: `currentPos - dragStartPos` (canvas coords)
4. **Convert Delta**: Canvas delta → Image delta → Normalized delta
5. **Update Region**: Add delta to normalized coords, call `syncFromNormalized()`

### Key Conversion Functions
- `CoordinateSystem::normalizedToCanvas()` - Normalized → Canvas
- `CoordinateSystem::canvasToNormalized()` - Canvas → Normalized
- `CoordinateSystem::normalizedToImage()` - Normalized → Image
- `CoordinateSystem::imageToNormalized()` - Image → Normalized
- `RegionData::syncFromNormalized()` - Updates all coordinate systems from normalized

## Potential Bug Patterns

### Pattern 1: Wrong Coordinate System for resizeStartRect
```cpp
// BUG: resizeStartRect might be in wrong coordinate system
QRectF resizeStartRect = canvasRect; // Is this canvas or image coords?
// Later in resizeRegion():
QRectF newRect = resizeStartRect; // Assumes resizeStartRect is canvas coords
newRect.setTopLeft(newPos); // newPos is canvas coords
```

### Pattern 2: Delta Calculation Issues
```cpp
// BUG: Delta might be calculated incorrectly
QPointF delta = currentPos - dragStartPos; // Both should be same coord system
// But dragStartPos might be widget coords and currentPos might be canvas coords
```

### Pattern 3: Primary Selection Not Set
```cpp
// BUG: primarySelectedRegion not updated
void Canvas::setSelectedRegions(const QSet<QString>& names) {
    selectedRegions = names;
    // Missing: primarySelectedRegion update
    update();
}
```

### Pattern 4: Cache Not Invalidated
```cpp
// BUG: Coordinate cache not invalidated after resize/move
region.normalizedCoords.x1 = newX;
region.syncFromNormalized(...);
// Missing: coordinateCache->invalidateRegion(regionName);
```

## Debugging Tools Available

### Debug Macros
The code uses `OCR-Orc_DEBUG` macros (when `OCR-Orc_DEBUG_ENABLED` is defined):
```cpp
OCR-Orc_DEBUG("=== mousePressEvent ===");
OCR-Orc_DEBUG("  canvasPos=" << canvasPos);
```

### Enable Debug Logging
In `src/core/Constants.h`, ensure `OCR-Orc_DEBUG_ENABLED` is defined for debug builds.

### Add Temporary Logging
Add `qDebug()` statements to trace:
- Mouse event coordinates
- Coordinate conversions
- State transitions
- Region updates

## Success Criteria

The fix is successful when:
- ✅ Regions resize smoothly without glitches
- ✅ Regions move smoothly without glitches
- ✅ Coordinates update correctly during operations
- ✅ Visual updates match coordinate updates
- ✅ Works at all zoom levels
- ✅ Works with single and multiple selections
- ✅ Undo/Redo works correctly after operations
- ✅ Resize handles appear/disappear correctly
- ✅ Primary selection is set correctly

## File Structure Summary

```
src/ui/canvas/
├── Canvas.h/cpp                    # Main widget, state management
├── input/
│   └── CanvasMouseHandler.h/cpp   # Mouse event handling (PRIMARY DEBUG TARGET)
├── core/
│   ├── coordinate/
│   │   ├── CanvasHitTester.h/cpp   # Hit testing (resize handle detection)
│   │   └── CanvasCoordinateCache.h/cpp
│   ├── regions/
│   │   └── CanvasRegionOperations.h/cpp  # Move/resize operations (PRIMARY DEBUG TARGET)
│   └── selection/
│       └── CanvasSelectionManager.h/cpp  # Selection state
└── managers/
    └── CanvasRegionManager.h/cpp

src/models/
├── RegionData.h/cpp                # Region data structure, syncFromNormalized()
└── DocumentState.h/cpp             # Region storage

src/core/
└── CoordinateSystem.h/cpp          # Coordinate conversion functions
```

## Quick Reference: Key Methods

| Method | File | Purpose |
|--------|------|---------|
| `CanvasMouseHandler::handlePress()` | CanvasMouseHandler.cpp:21 | Mouse press - detect resize handle |
| `CanvasMouseHandler::handleMove()` | CanvasMouseHandler.cpp:154 | Mouse move - update resize/move |
| `CanvasMouseHandler::handleRelease()` | CanvasMouseHandler.cpp:237 | Mouse release - finalize operation |
| `CanvasHitTester::getHandleAt()` | CanvasHitTester.cpp:63 | Detect which handle clicked |
| `CanvasRegionOperations::resizeRegion()` | CanvasRegionOperations.cpp:75 | Perform resize operation |
| `CanvasRegionOperations::moveRegion()` | CanvasRegionOperations.cpp:14 | Perform move operation |
| `RegionData::syncFromNormalized()` | RegionData.cpp | Sync all coordinate systems |
| `Canvas::setSelectedRegions()` | Canvas.cpp:380+ | Update selection (check primary) |

---

**Last Updated**: 2024-12-08
**Context Created For**: Region resize and move debugging session
**Status**: Ready for new agent to begin debugging

