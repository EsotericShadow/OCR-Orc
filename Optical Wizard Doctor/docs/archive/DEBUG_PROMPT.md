# Debug Prompt: Region Resize & Move Glitches

## Your Task

You are tasked with debugging and fixing glitching issues with **region resizing** and **region moving** in the OCR-Orc application. Users have reported that:

1. **Region resizing** is glitching (resize handles not working correctly, regions jumping/snapping incorrectly)
2. **Region moving** is glitching (dragging regions causes visual glitches, regions not moving smoothly)

## Context Document

**READ THIS FIRST**: `docs/REGION_RESIZE_MOVE_DEBUG_CONTEXT.md`

This comprehensive document (609 lines) contains:
- Complete application architecture
- All key files and methods with line numbers
- Coordinate system explanation
- Known issues and potential bug patterns
- Debugging strategy and checklist
- Code locations to investigate
- Testing scenarios

**You must read and understand this document before starting.**

## Getting Started

### Step 1: Understand the Problem
1. Read `docs/REGION_RESIZE_MOVE_DEBUG_CONTEXT.md` completely
2. Build and run the application to reproduce the issues:
   ```bash
   cd "/Users/main/Desktop/OCR-Orc-OCR_Coordinates/OCR-Orc"
   cmake --build build --target owd
   ./build/bin/owd.app/Contents/MacOS/owd
   ```
3. Test the problematic features:
   - Create a region
   - Switch to Select/Move mode
   - Try resizing a region (click and drag a resize handle)
   - Try moving a region (click and drag the region)
   - Note the exact glitch behavior

### Step 2: Identify Root Cause
Based on the context document, focus on these **high-priority areas**:

1. **`Canvas::setSelectedRegions()`** (Canvas.cpp:222-236)
   - Check if `primarySelectedRegion` is being set correctly
   - This is critical for resize handles to appear

2. **`CanvasMouseHandler::handleMove()`** (CanvasMouseHandler.cpp:154-215)
   - Check coordinate conversion during resize/move
   - Verify `canvasPos` is in the correct coordinate system
   - Check if `resizeStartRect` is in the correct coordinate system

3. **`CanvasRegionOperations::resizeRegion()`** (CanvasRegionOperations.cpp:75-170)
   - Verify coordinate conversion chain: Canvas → Image → Normalized
   - Check if `syncFromNormalized()` is being called
   - Verify `resizeStartRect` coordinate system matches expectations

4. **`CanvasRegionOperations::moveRegion()`** (CanvasRegionOperations.cpp:14-61)
   - Verify delta calculation is correct
   - Check coordinate conversion: Canvas delta → Image delta → Normalized delta
   - Verify `syncFromNormalized()` is being called

### Step 3: Add Debug Logging
Enable debug logging to trace the issue:

1. Check if `OCR-Orc_DEBUG_ENABLED` is defined in `src/core/Constants.h`
2. Add `qDebug()` statements to trace:
   ```cpp
   // In CanvasMouseHandler::handleMove()
   qDebug() << "=== Resize Debug ===";
   qDebug() << "  isResizing:" << isResizing;
   qDebug() << "  resizingRegion:" << resizingRegion;
   qDebug() << "  resizeHandle:" << resizeHandle;
   qDebug() << "  canvasPos:" << canvasPos;
   qDebug() << "  resizeStartRect:" << resizeStartRect;
   
   // In CanvasRegionOperations::resizeRegion()
   qDebug() << "  newRect:" << newRect;
   qDebug() << "  newNormalized:" << newNormalized;
   ```

3. Run the application and observe the debug output during resize/move operations

### Step 4: Fix the Issues
Based on your findings, fix the bugs. Common issues to check:

#### Issue 1: Primary Selection Not Set
**Location**: `Canvas::setSelectedRegions()` (Canvas.cpp:222-236)

**Check**: Does this method correctly set `primarySelectedRegion` when exactly one region is selected?

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
    
    update();
}
```

#### Issue 2: Wrong Coordinate System
**Location**: `CanvasMouseHandler::handlePress()` and `handleMove()`

**Check**: Is `resizeStartRect` stored in canvas coordinates? Is it used correctly in `resizeRegion()`?

**Potential Fix**: Ensure `resizeStartRect` is in canvas coordinates when stored, and verify it's used correctly in `CanvasRegionOperations::resizeRegion()`.

#### Issue 3: Delta Calculation
**Location**: `CanvasMouseHandler::handleMove()` (line 193-196)

**Check**: Are `canvasPos` and `lastDragPos` in the same coordinate system?

**Current Code**:
```cpp
QPointF delta = canvasPos - lastDragPos;
```

**Verify**: Both should be in canvas coordinates.

#### Issue 4: Coordinate Cache Not Invalidated
**Location**: After `moveRegion()` or `resizeRegion()`

**Check**: Is the coordinate cache invalidated after region updates?

**Potential Fix**: Call `invalidateCoordinateCache()` after region operations.

### Step 5: Test Your Fixes
After making changes, test thoroughly:

- [ ] Resize single region from each handle (8 handles: nw, n, ne, e, se, s, sw, w)
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

### Step 6: Run Tests
Run the full test suite to ensure nothing broke:

```bash
cd "/Users/main/Desktop/OCR-Orc-OCR_Coordinates/OCR-Orc"
cd build
ctest --output-on-failure
```

## Key Files to Focus On

1. **`src/ui/canvas/Canvas.cpp`** (line 222-236: `setSelectedRegions()`)
2. **`src/ui/canvas/input/CanvasMouseHandler.cpp`** (mouse event handling)
3. **`src/ui/canvas/core/regions/CanvasRegionOperations.cpp`** (resize/move operations)
4. **`src/ui/canvas/core/coordinate/CanvasHitTester.cpp`** (resize handle detection)
5. **`src/models/RegionData.cpp`** (`syncFromNormalized()` method)

## Debugging Workflow

1. **Reproduce**: Run the app and reproduce the glitch
2. **Trace**: Add debug logging to trace the execution path
3. **Identify**: Find where the bug occurs (coordinate conversion? state management? cache?)
4. **Fix**: Apply the fix
5. **Verify**: Test thoroughly with all scenarios
6. **Validate**: Run test suite

## Success Criteria

Your fix is successful when:
- ✅ Regions resize smoothly without glitches
- ✅ Regions move smoothly without glitches
- ✅ Coordinates update correctly during operations
- ✅ Visual updates match coordinate updates
- ✅ Works at all zoom levels
- ✅ Works with single and multiple selections
- ✅ Undo/Redo works correctly after operations
- ✅ Resize handles appear/disappear correctly
- ✅ Primary selection is set correctly

## Important Notes

1. **Don't break existing functionality**: The application has many working features. Be careful not to break them.

2. **Coordinate Systems**: The app uses 3 coordinate systems (Normalized, Image, Canvas). Always verify you're using the correct one.

3. **Normalized is Source of Truth**: Always update `normalizedCoords` first, then call `syncFromNormalized()`.

4. **Test Incrementally**: Fix one issue at a time, test, then move to the next.

5. **Ask for Help**: If you're stuck, document what you've tried and what you've found.

## Build Commands

```bash
# Build the application
cd "/Users/main/Desktop/OCR-Orc-OCR_Coordinates/OCR-Orc"
cmake --build build --target owd

# Run the application
./build/bin/owd.app/Contents/MacOS/owd

# Run tests
cd build && ctest --output-on-failure

# Clean build (if needed)
rm -rf build && mkdir build && cd build && cmake .. && cmake --build .
```

## Questions to Answer

As you debug, document answers to these questions:

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

4. **Root cause?**
   - Coordinate conversion error?
   - State management issue?
   - Cache invalidation problem?
   - Primary selection not set?

## Next Steps

1. Read `docs/REGION_RESIZE_MOVE_DEBUG_CONTEXT.md`
2. Build and run the application
3. Reproduce the issues
4. Add debug logging
5. Identify root cause
6. Fix the bugs
7. Test thoroughly
8. Report your findings

Good luck! 🚀

