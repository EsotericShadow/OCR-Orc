# Bug Investigation Report
Generated: $(date)

## App Status
✅ App is running for testing

## Critical Issues Found

### 1. **State Flag Reset Issues**
**Location**: `CanvasMouseHandler.cpp`, `Canvas.cpp`
**Issue**: State flags (isCreating, isDragging, isResizing, isRotating, isPanning) may not be reset in all error paths
**Risk**: Medium - Could cause stuck states where operations don't work
**Investigation Needed**: 
- Check all early returns in mouse handlers
- Verify flags are reset on exceptions/errors
- Check if window close/resize cancels operations properly

### 2. **Coordinate Synchronization Race Conditions**
**Location**: `DocumentState::synchronizeCoordinates()`, `Canvas::resizeEvent()`
**Issue**: `synchronizeCoordinates()` is called in multiple places - potential race if called during operations
**Risk**: Medium - Could cause coordinate desync
**Investigation Needed**:
- Check if synchronizeCoordinates is called during drag/resize operations
- Verify resizeEvent properly updates scaleFactor before synchronizing
- Check if image dimensions are validated before sync

### 3. **Missing Image Dimension Validation**
**Location**: `DocumentState::synchronizeCoordinates()`, `RegionData::syncFromNormalized()`
**Issue**: No check if imgWidth/imgHeight are > 0 before using in calculations
**Risk**: Low - Division by zero is handled in CoordinateSystem, but should validate earlier
**Investigation Needed**:
- Add validation in synchronizeCoordinates
- Check all places that use image.width()/height()

### 4. **Rotation Calculation Edge Cases**
**Location**: `CanvasRegionOperations::rotateRegion()`
**Issue**: 
- Rotation uses atan2 which can produce NaN for edge cases
- Bounding box calculation after rotation might create invalid coordinates
- No validation that rotation angle is reasonable
**Risk**: Medium - Could produce invalid regions
**Investigation Needed**:
- Check for NaN/infinity in rotation calculations
- Validate rotation angle range
- Test with very small/large regions
- Test with 90/180/270 degree rotations

### 5. **Multiple Selection Assumptions**
**Location**: Various mouse handlers
**Issue**: Some operations check `selectedRegions.size() == 1` but might fail if selection changes during operation
**Risk**: Medium - Operations might fail silently or apply to wrong region
**Investigation Needed**:
- Check if selection can change during drag/resize/rotate
- Verify primarySelectedRegion stays consistent
- Check what happens if user selects different region mid-operation

### 6. **Undo/Redo State Restoration**
**Location**: `DocumentState::restoreState()`
**Issue**: 
- Image is not restored (only pdfPath) - what if PDF was moved/deleted?
- No validation that restored state is valid
- synchronizeCoordinates called after restore - but what if image is null?
**Risk**: Medium - Undo/redo might fail or produce invalid state
**Investigation Needed**:
- Check if image reload is handled properly
- Validate state after restore
- Check if coordinates are valid after restore

### 7. **File Operation Error Handling**
**Location**: `JsonExporter`, `JsonImporter`, `CsvExporter`, `PdfLoader`
**Issue**: 
- Some operations throw exceptions but UI might not handle gracefully
- Atomic file writes might leave temp files on crash
- PDF loading returns empty QImage on error - is this handled everywhere?
**Risk**: Medium - Could cause crashes or data loss
**Investigation Needed**:
- Check all file operation error paths
- Verify temp file cleanup
- Check if empty QImage is handled in all callers

### 8. **Signal/Slot Recursion Risk**
**Location**: `SidePanelWidget`, region editor connections
**Issue**: 
- blockSignals used but might miss some connections
- Coordinate value changes emit signals that might trigger updates
**Risk**: Low - Should be handled but worth verifying
**Investigation Needed**:
- Trace all signal connections from region editor
- Verify no infinite loops
- Check if blockSignals covers all cases

### 9. **Update Batching Timer Issues**
**Location**: `Canvas.cpp` - updateTimer
**Issue**: 
- Single-shot timer might miss rapid updates
- pendingUpdateRect might not accumulate properly
**Risk**: Low - Performance issue, not correctness
**Investigation Needed**:
- Check if rapid updates are batched correctly
- Verify updateTimer doesn't cause missed repaints

### 10. **Region Validation Gaps**
**Location**: `RegionData::isValid()`, `CoordinateSystem::isValidNormalized()`
**Issue**: 
- isValid checks coordinates but not if region is too small
- No check for regions outside image bounds after operations
**Risk**: Low - Should be caught but worth checking
**Investigation Needed**:
- Check minimum region size validation
- Verify bounds checking after all operations

## Suspicious Patterns Found

### Pattern 1: State Flag Management
- Multiple boolean flags for different operations
- Flags set in one handler, cleared in another
- Risk of flags getting out of sync

### Pattern 2: Coordinate System Conversions
- Multiple coordinate systems (normalized, image, canvas)
- Conversions happen in many places
- Risk of using wrong coordinate system

### Pattern 3: Selection State Management
- selectedRegions (QSet) and primarySelectedRegion (QString) must stay in sync
- Operations check both independently
- Risk of inconsistency

### Pattern 4: Update/Repaint Calls
- Many direct update() calls
- Some use updateTimer, some don't
- Risk of excessive repaints or missed updates

## Areas Requiring Deep Investigation

1. **Mouse Event Handling Flow**
   - Verify all state transitions are valid
   - Check for missing state resets
   - Verify operation cancellation works

2. **Coordinate System Consistency**
   - Verify normalized coords are always source of truth
   - Check all sync points
   - Verify no stale coordinate data

3. **Memory Management**
   - Check for QImage/QPixmap leaks
   - Verify smart pointers used correctly
   - Check widget parent/child relationships

4. **Error Recovery**
   - What happens if PDF load fails mid-operation?
   - What happens if region operation fails?
   - Are all error paths tested?

5. **Edge Cases**
   - Very small regions (< 1 pixel)
   - Very large images
   - Extreme zoom levels
   - Rapid mouse movements
   - Multiple rapid operations

## Additional Issues Found (Deep Dive)

### 11. **Image Dimension Validation Missing**
**Location**: `DocumentState::synchronizeCoordinates()`, `CanvasRegionOperations::moveRegion()`, `rotateRegion()`, `resizeRegion()`
**Issue**: No validation that `imgWidth > 0` and `imgHeight > 0` before using in calculations
**Risk**: Medium - Could cause division by zero or invalid coordinates
**Code Locations**:
- `DocumentState::synchronizeCoordinates()` line 323-324: Uses `image.width()` and `image.height()` without checking > 0
- `CanvasRegionOperations::moveRegion()` line 42-46: Checks <= 0 but only returns, doesn't validate before use
- `CanvasRegionOperations::rotateRegion()` line 271-272: No validation before multiplication

### 12. **Rotation NaN/Infinity Risk**
**Location**: `CanvasRegionOperations::rotateRegion()`
**Issue**: 
- `atan2` can theoretically produce NaN if both arguments are zero (though unlikely)
- `cos` and `sin` of very large angles might have precision issues
- Bounding box calculation with `std::min/max` on QPointF might produce NaN if any point is NaN
**Risk**: Medium - Could create invalid regions
**Code Location**: Lines 289-311

### 13. **Coordinate Clamping Order Issue**
**Location**: `CanvasRegionOperations::constrainToBounds()`
**Issue**: 
- Swaps coordinates if x1 > x2 or y1 > y2 (line 214-215)
- Then clamps to 0.0-1.0 (line 218-221)
- But if clamping makes x1 == x2 or y1 == y2, region becomes invalid
- No check after clamping to ensure x1 < x2 and y1 < y2
**Risk**: Medium - Could create invalid regions that pass validation
**Code Location**: Lines 209-225

### 14. **State Flag Reset on Early Return**
**Location**: `CanvasMouseHandler::handlePress()`, `handleMove()`, `handleRelease()`
**Issue**: Multiple early returns that might not reset state flags
**Risk**: Medium - Could leave flags in wrong state
**Code Locations**:
- `handlePress()` line 76: Early return if `!documentState || documentImage.isNull()` - flags not reset
- `handlePress()` line 85: Early return for pan - other flags not checked/reset
- `handleMove()` line 229: Early return - flags not reset
- `handleRelease()` line 377: Early return - flags not reset

### 15. **Selection Consistency Check Missing**
**Location**: `Canvas::resizeEvent()`, various mouse handlers
**Issue**: 
- `primarySelectedRegion` might not be in `selectedRegions` after operations
- No validation that they stay in sync
**Risk**: Low-Medium - Could cause operations to fail silently
**Code Location**: `Canvas.cpp` line 239-240 has a check, but might not cover all cases

### 16. **Undo/Redo Image Restoration**
**Location**: `DocumentState::restoreState()`
**Issue**: 
- Image is not restored, only `pdfPath`
- If PDF was moved/deleted, undo/redo will fail
- `synchronizeCoordinates()` called after restore, but what if image is null?
**Risk**: Medium - Undo/redo might produce invalid state
**Code Location**: Lines 532-551

### 17. **Region Name Validation Gaps**
**Location**: `DocumentState::isValidRegionName()`
**Issue**: 
- Only checks if empty and unique
- No check for maximum length
- No check for invalid characters (newlines, control chars, etc.)
- No check for reserved names
**Risk**: Low - Could cause issues in CSV/JSON export
**Code Location**: Lines 139-151

### 18. **Group Reference Cleanup**
**Location**: `DocumentState::removeRegion()`
**Issue**: 
- Removes region from group (line 61)
- But what if group becomes empty? Group is not deleted
- Orphaned empty groups might accumulate
**Risk**: Low - Memory/UI clutter, not correctness
**Code Location**: Lines 53-66

### 19. **Coordinate Precision Loss**
**Location**: `CoordinateSystem::normalizedToImage()`, `canvasToImage()`
**Issue**: 
- Uses `std::round()` which might lose precision for very small regions
- Integer conversion might truncate important fractional parts
**Risk**: Low - Precision issue, not correctness
**Code Locations**: 
- `normalizedToImage()` line 13-16
- `canvasToImage()` line 71-74

### 20. **Resize Handle Validation**
**Location**: `CanvasRegionOperations::resizeRegion()`
**Issue**: 
- No validation that `handle` string is valid before using `contains()`
- Empty handle string would pass all `contains()` checks
- Invalid handle names not caught
**Risk**: Low-Medium - Could cause resize to fail or behave unexpectedly
**Code Location**: Lines 100-175

### 21. **Rotation Angle Normalization**
**Location**: `CanvasMouseHandler::handleMove()`
**Issue**: 
- Normalizes angle to -180 to 180 range (lines 311-312)
- But what if `currentAngle` or `rotationStartAngle` is NaN or Infinity?
- No check for invalid angles before normalization
**Risk**: Medium - Could produce NaN rotation
**Code Location**: Lines 305-312

### 22. **Bounding Box Calculation Edge Case**
**Location**: `CanvasRegionOperations::rotateRegion()`
**Issue**: 
- Uses `std::min({p1.x(), p2.x(), p3.x(), p4.x()})` which requires at least 2 arguments
- If any point is NaN, result is NaN
- No validation that points are valid before min/max
**Risk**: Medium - Could produce NaN coordinates
**Code Location**: Lines 308-311

### 23. **Minimum Size Enforcement Failure**
**Location**: `CanvasRegionOperations::rotateRegion()`, `constrainToBounds()`
**Issue**: 
- After rotation, minimum size is enforced (lines 327-341)
- But if region is at boundary (0,0) or (1,1), might not be able to enforce minimum
- Could create invalid region with x1 == x2 or y1 == y2
**Risk**: Medium - Could create invalid regions
**Code Location**: Lines 327-341

### 24. **Coordinate Editor Race Condition**
**Location**: `SidePanelWidget::updateCoordinateValue()`
**Issue**: 
- Updates coordinates while region might be moved/resized
- No check if region still exists
- No check if `currentEditingRegion` is still valid
**Risk**: Medium - Could modify wrong region or crash
**Code Location**: Lines 700+

### 25. **JSON Import Duplicate Handling**
**Location**: `JsonImporter::loadRegions()`
**Issue**: 
- If JSON has duplicate region names, second one overwrites first
- No warning or error
- Might lose data silently
**Risk**: Medium - Data loss
**Code Location**: Lines 121-164

### 26. **CSV Export Special Character Handling**
**Location**: `CsvExporter::escapeCsvField()`
**Issue**: 
- Escapes quotes and commas
- But what about newlines, tabs, or other control characters?
- Might break CSV format
**Risk**: Low-Medium - CSV might be invalid
**Code Location**: Lines 70+

### 27. **Mask Generation Coordinate Validation**
**Location**: `MaskGenerator::createMaskImage()`
**Issue**: 
- Uses `region.imageCoords` directly
- What if imageCoords are stale or invalid?
- Should use normalized coords and convert
**Risk**: Medium - Mask might be wrong
**Code Location**: Lines 68-77

### 28. **Update Timer Race Condition**
**Location**: `Canvas::updateTimer`
**Issue**: 
- Single-shot timer might miss rapid updates
- `pendingUpdateRect` might not accumulate properly
- What if update is called while timer is running?
**Risk**: Low - Performance/visual glitch, not correctness
**Code Location**: Lines 69-79

### 29. **Widget Parent-Child Memory**
**Location**: Various widget constructors
**Issue**: 
- Most widgets have parent set, which is good
- But `editorContainer` in `SidePanelWidget` is parented to `editorScrollArea`, not `regionEditorFrame`
- Should verify all widgets have correct parents
**Risk**: Low - Memory management should be fine, but worth checking
**Code Location**: `SidePanelWidget.cpp` line 343

### 30. **Coordinate Cache Invalidation Timing**
**Location**: Various `invalidateCoordinateCache()` calls
**Issue**: 
- Cache invalidated in many places
- But what if cache is accessed between invalidation and update?
- Race condition possible
**Risk**: Low - Might show stale coordinates briefly
**Code Locations**: Multiple in `Canvas.cpp`

### 31. **Zoom Level Bounds Checking**
**Location**: `CanvasZoomController::setZoom()`, `zoomIn()`, `zoomOut()`
**Issue**: 
- Zoom levels clamped to MIN_ZOOM/MAX_ZOOM
- But what if zoom becomes exactly 0.0?
- Division by zero in coordinate calculations
**Risk**: Medium - Could cause crashes
**Code Location**: `CanvasZoomController.cpp`

### 32. **Pan Offset Validation**
**Location**: `CanvasMouseHandler::handleMove()`, `handleRelease()`
**Issue**: 
- `imageOffsetRef` updated during pan (line 288, 443)
- No validation that offset is reasonable
- Could become very large or contain NaN
**Risk**: Low-Medium - Could cause rendering issues
**Code Locations**: Lines 286-296, 442-451

### 33. **Region Creation Validation**
**Location**: `CanvasRegionCreator::finishRegionCreation()`
**Issue**: 
- `validateRegion` callback used, but what if it returns false?
- Region might still be created
- Or creation might be cancelled incorrectly
**Risk**: Medium - Invalid regions might be created
**Code Location**: `CanvasRegionCreator.cpp`

### 34. **Box Selection Edge Cases**
**Location**: `CanvasMouseHandler::handleRelease()`
**Issue**: 
- `findRegionsInBox` might return empty set
- `regionsInBox.isEmpty()` check exists (line 466)
- But what if box is very small or invalid?
**Risk**: Low - Should be handled, but worth verifying
**Code Location**: Lines 454-490

### 35. **Hit Test Coordinate Conversion**
**Location**: `CanvasHitTester::getRegionAt()`
**Issue**: 
- Converts widget coordinates to canvas to image
- Multiple coordinate system conversions
- Precision loss possible
- What if conversion produces invalid coordinates?
**Risk**: Low-Medium - Might miss regions or hit wrong ones
**Code Location**: `CanvasHitTester.cpp`

### 36. **Resize Handle Detection Precision**
**Location**: `CanvasHitTester::getResizeHandle()`
**Issue**: 
- Uses fixed `HANDLE_SIZE` and tolerance
- What if region is very small?
- Handles might overlap or be outside region
**Risk**: Low - Usability issue
**Code Location**: `CanvasHitTester.cpp`

### 37. **Region List Update During Selection**
**Location**: `SidePanelWidget::updateRegionListBox()`
**Issue**: 
- Uses `blockSignals()` to prevent recursion
- But what if selection changes during block?
- Selection might be lost
**Risk**: Low - UI glitch
**Code Location**: Lines 262-291

### 38. **Group List Update During Selection**
**Location**: Similar to region list
**Issue**: Same as above
**Risk**: Low - UI glitch

### 39. **Undo During Operation**
**Location**: Various operation handlers
**Issue**: 
- No check if operation is in progress before undo
- Undo might cancel operation incorrectly
- State might be inconsistent
**Risk**: Medium - Could cause state corruption
**Code Location**: Various

### 40. **PDF Reload During Operation**
**Location**: `Canvas::loadPdf()`
**Issue**: 
- Clears regions and groups (line 103-104)
- But what if user is editing a region?
- What if drag/resize is in progress?
**Risk**: Medium - Could cause crashes or data loss
**Code Location**: Lines 99-124

### 41. **Zoom During Operation**
**Location**: Zoom handlers
**Issue**: 
- Zoom changes `scaleFactor`
- But what if region is being resized?
- Coordinates might become invalid
**Risk**: Medium - Could cause coordinate errors
**Code Location**: Various zoom functions

### 42. **Theme Change During Render**
**Location**: `ThemeManager::applyTheme()`
**Issue**: 
- Changes stylesheets
- But what if widgets are being rendered?
- Might cause visual glitches
**Risk**: Low - Visual issue only

### 43. **Window Minimize During Operation**
**Location**: Window event handlers
**Issue**: 
- No special handling for minimize/hide
- Operations might continue
- State flags might not be reset
**Risk**: Low - State might be stuck

### 44. **Coordinate Editor Value Parsing**
**Location**: `SidePanelWidget::updateCoordinateValue()`
**Issue**: 
- Directly modifies normalized coordinates
- No validation that new value is valid
- No check for NaN or infinity
**Risk**: Medium - Could create invalid regions
**Code Location**: Lines 700+

### 45. **JSON Coordinate Parsing**
**Location**: `JsonImporter::parseCoordinates()`
**Issue**: 
- Uses `toDouble()` which might return 0.0 on error
- No check for NaN or infinity
- No validation that coordinates are in valid range
**Risk**: Medium - Could import invalid regions
**Code Location**: `JsonImporter.cpp`

### 46. **Shape Type Validation**
**Location**: Region rendering, JSON import
**Issue**: 
- `shapeType` used in rendering (line 133-146 in CanvasRenderer.cpp)
- But what if shapeType is invalid?
- Defaults to rectangle, but no validation
**Risk**: Low - Should default correctly

### 47. **Region Type/Percentage Fill Validation**
**Location**: JSON import, region editor
**Issue**: 
- No validation that `regionType` and `percentageFill` are valid values
- Invalid values might be stored
**Risk**: Low - Data integrity issue

### 48. **Canvas Size Zero Check**
**Location**: `CanvasZoomController::calculateLayout()`
**Issue**: 
- Checks `canvasWidth <= 0 || canvasHeight <= 0` and returns early (line 27-29)
- But `scaleFactor` might not be set correctly
- Subsequent operations might use invalid scaleFactor
**Risk**: Medium - Could cause division by zero
**Code Location**: Lines 14-29

### 49. **Rotation Center Calculation**
**Location**: `CanvasRegionOperations::rotateRegion()`
**Issue**: 
- Calculates center as average of x1+x2 and y1+y2 (line 275-276)
- What if coordinates are invalid?
- No validation before calculation
**Risk**: Low-Medium - Could produce invalid center

### 50. **Bounding Box After Rotation**
**Location**: `CanvasRegionOperations::rotateRegion()`
**Issue**: 
- Calculates bounding box of 4 rotated points
- Divides by imgWidth/imgHeight to normalize (lines 315-318)
- But what if imgWidth or imgHeight is 0? (Should be checked earlier, but defense in depth)
**Risk**: Low - Should be caught earlier, but no check here

## Next Steps

1. Create test cases for each suspicious area
2. Add validation/assertions to catch issues early
3. Review error handling paths
4. Test edge cases systematically
5. Add logging to track state transitions
6. Add NaN/Infinity checks for all floating point calculations
7. Add bounds checking for all coordinate operations
8. Add state consistency checks
9. Add validation for all user inputs
10. Test with extreme values (very small/large images, many regions, rapid operations)

