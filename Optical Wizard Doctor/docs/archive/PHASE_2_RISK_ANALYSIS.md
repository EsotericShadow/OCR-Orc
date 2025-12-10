# Phase 2: Interaction - Comprehensive Risk Analysis

## Executive Summary

This document identifies **all potential problems, obstacles, difficulties, and risks** before implementing Phase 2. The goal is to be **preemptive** with problem-solving and avoid coding into corners.

**Status:** Pre-Implementation Analysis  
**Date:** December 7, 2024  
**Purpose:** Identify and mitigate risks before development begins

---

## Critical Dependencies & Setup Challenges

### Risk 1: Poppler Installation & Integration

**Problem:**
- Poppler is **not yet installed** (only in vcpkg.json, not actually installed)
- Poppler has multiple variants: `poppler`, `poppler-qt6`, `poppler-qt5`
- CMake may not find Poppler correctly
- Poppler may require additional system dependencies (fontconfig, freetype, etc.)

**Potential Issues:**
1. **Wrong Poppler Package**: Installing `poppler` instead of `poppler-qt6` means no Qt integration
2. **Missing System Dependencies**: Poppler requires fontconfig, freetype, cairo, etc. on Linux/macOS
3. **CMake Find Issues**: `find_package(Poppler)` may not work if Poppler wasn't installed via vcpkg
4. **Version Mismatches**: Poppler version may be incompatible with Qt6 version
5. **Cross-Platform Differences**: Poppler installation differs significantly between macOS/Windows/Linux

**Solutions:**
1. **Verify Installation First**: Before coding, test Poppler installation:
   ```bash
   # Check if Poppler is installed
   vcpkg list poppler
   # Or check system installation
   pkg-config --modversion poppler
   ```
2. **Use Correct Package**: Install `poppler-qt6` specifically (not just `poppler`)
3. **Test Poppler Early**: Create minimal test program to verify Poppler works before integrating
4. **Handle Missing Poppler Gracefully**: Add CMake checks and user-friendly error messages
5. **Document Platform-Specific Steps**: Create platform-specific installation guides

**Prevention Checklist:**
- [ ] Install Poppler via vcpkg: `vcpkg install poppler-qt6`
- [ ] Verify Poppler is found by CMake
- [ ] Create minimal Poppler test program
- [ ] Test PDF loading before Canvas implementation
- [ ] Document installation process for all platforms

---

### Risk 2: Poppler API Complexity & Learning Curve

**Problem:**
- Poppler API is **not trivial** - requires understanding of:
  - `poppler::Document` class
  - `poppler::Page` class
  - Rendering to QImage
  - DPI and resolution handling
  - Error handling and memory management

**Potential Issues:**
1. **Memory Leaks**: Poppler objects must be properly managed (RAII or smart pointers)
2. **Thread Safety**: Poppler may not be thread-safe (affects background loading)
3. **Error Handling**: PDF files can be corrupted, password-protected, or invalid
4. **DPI Selection**: Wrong DPI leads to poor image quality or huge memory usage
5. **Page Rendering**: First page extraction may fail for some PDFs

**Solutions:**
1. **Create Wrapper Class**: Abstract Poppler complexity behind `PdfLoader` utility class
2. **Use Smart Pointers**: Use `std::unique_ptr` or `QScopedPointer` for Poppler objects
3. **Comprehensive Error Handling**: Check all Poppler operations, return error codes
4. **Test with Various PDFs**: Corrupted, password-protected, multi-page, large files
5. **Document DPI Strategy**: Choose appropriate DPI (150-300) based on use case

**Prevention Checklist:**
- [ ] Study Poppler API documentation
- [ ] Create `PdfLoader` wrapper class first
- [ ] Test with various PDF types (corrupted, password-protected, etc.)
- [ ] Implement proper error handling
- [ ] Use smart pointers for memory management

---

### Risk 3: Canvas Widget Architecture

**Problem:**
- Canvas needs to handle **multiple responsibilities**:
  - PDF image rendering
  - Region rendering
  - Mouse event handling
  - Coordinate transformations
  - Zoom/pan state
  - Selection state
  - This can lead to a **bloated, hard-to-maintain class**

**Potential Issues:**
1. **God Object Anti-Pattern**: Canvas becomes too large and complex
2. **Tight Coupling**: Canvas directly depends on DocumentState, CoordinateSystem, etc.
3. **Hard to Test**: Large class with many responsibilities is difficult to unit test
4. **Performance Issues**: Recalculating everything on every paint event
5. **State Management**: Multiple state variables (zoom, pan, selection, hover) can get out of sync

**Solutions:**
1. **Separate Concerns**: Split into:
   - `CanvasWidget` (rendering only)
   - `CanvasController` (event handling, state management)
   - `RegionRenderer` (region drawing logic)
2. **Use Composition**: Canvas contains renderer objects, not all logic
3. **Cache Calculations**: Cache coordinate transformations, only recalculate on zoom/pan
4. **State Object**: Create `CanvasState` struct to manage all state in one place
5. **Incremental Development**: Build Canvas incrementally, test each feature

**Prevention Checklist:**
- [ ] Design Canvas architecture before coding
- [ ] Separate rendering from event handling
- [ ] Create state management strategy
- [ ] Plan for caching and optimization
- [ ] Design for testability

---

## Coordinate System Integration Risks

### Risk 4: Coordinate Transformation Bugs

**Problem:**
- Canvas needs to convert between **three coordinate systems**:
  - Normalized (0.0-1.0)
  - Image (pixels)
  - Canvas (display pixels with zoom/pan)
- **One bug in transformation = regions appear in wrong place**

**Potential Issues:**
1. **Off-by-One Errors**: Pixel vs. coordinate confusion
2. **Zoom/Pan Not Applied**: Forgetting to account for zoom/pan in transformations
3. **Aspect Ratio Issues**: Image aspect ratio vs. canvas aspect ratio
4. **Precision Loss**: Floating-point precision issues at extreme zoom levels
5. **Coordinate Invalidation**: Canvas coords become stale after zoom/pan

**Solutions:**
1. **Use CoordinateSystem Class**: Don't reimplement transformations, use existing class
2. **Comprehensive Tests**: Test all transformation paths with known values
3. **Visual Debugging**: Draw coordinate grid to verify transformations
4. **Validation**: Validate coordinates after every transformation
5. **Recalculate on Zoom/Pan**: Always recalculate canvas coords when zoom/pan changes

**Prevention Checklist:**
- [ ] Test CoordinateSystem transformations thoroughly
- [ ] Create visual coordinate grid for debugging
- [ ] Test at extreme zoom levels (20%, 500%)
- [ ] Test with different image aspect ratios
- [ ] Verify transformations with known test cases

---

### Risk 5: Mouse Coordinate Conversion

**Problem:**
- Mouse events give **screen coordinates** (widget-relative)
- Need to convert to **canvas coordinates** (image-relative)
- Must account for:
  - Image offset (centering)
  - Image scale factor
  - Zoom level
  - Pan offset
  - Scroll position (if using QScrollArea)

**Potential Issues:**
1. **Wrong Coordinate Space**: Using screen coords instead of canvas coords
2. **Missing Transformations**: Forgetting to account for offset/scale/zoom
3. **Scroll Position**: QScrollArea adds another coordinate transformation
4. **Edge Cases**: Clicking outside image, clicking on scrollbars
5. **Precision Issues**: Integer vs. floating-point coordinate handling

**Solutions:**
1. **Centralized Conversion Function**: Create `screenToCanvas(const QPoint& screen)` function
2. **Test Conversions**: Test with known screen positions, verify canvas coords
3. **Handle Edge Cases**: Check if click is within image bounds
4. **Use QPointF**: Use floating-point for precision
5. **Document Coordinate Spaces**: Clearly document which coordinate space each function uses

**Prevention Checklist:**
- [ ] Create screen-to-canvas conversion function
- [ ] Test coordinate conversions with known values
- [ ] Handle edge cases (clicks outside image)
- [ ] Document coordinate space conventions
- [ ] Test with QScrollArea if used

---

## Performance & Memory Risks

### Risk 6: Large PDF Memory Usage

**Problem:**
- PDFs can be **very large** (100+ MB)
- Rendering to QImage at high DPI creates **huge images**:
  - A4 page at 300 DPI = 2480x3508 pixels = ~25 MB per image
  - Multiple pages or high-res images = **hundreds of MB**

**Potential Issues:**
1. **Out of Memory**: Application crashes on large PDFs
2. **Slow Loading**: PDF loading blocks UI thread
3. **Slow Rendering**: Large images cause paintEvent() to be slow
4. **Memory Leaks**: Not releasing old images when loading new PDF
5. **Zoom Performance**: Re-rendering large images at different scales is slow

**Solutions:**
1. **Reasonable DPI**: Use 150 DPI for initial display (not 300 DPI)
2. **Background Loading**: Load PDF in background thread (QThread or QtConcurrent)
3. **Image Caching**: Cache rendered image, only re-render on zoom change
4. **Progressive Loading**: Load low-res first, then high-res
5. **Memory Management**: Use `QImage::swap()` or move semantics to avoid copies
6. **Limit Image Size**: Cap maximum image dimensions (e.g., 4000x4000 pixels)

**Prevention Checklist:**
- [ ] Test with large PDFs (100+ MB)
- [ ] Monitor memory usage
- [ ] Implement background loading
- [ ] Use reasonable DPI (150, not 300)
- [ ] Add memory limits and error handling

---

### Risk 7: Region Rendering Performance

**Problem:**
- With **100+ regions**, rendering all regions in `paintEvent()` can be slow
- Recalculating coordinates for all regions on every paint is expensive
- Selection/hover state checks for all regions on every mouse move

**Potential Issues:**
1. **UI Lag**: Slow paintEvent() causes laggy UI
2. **Flickering**: Frequent repaints cause flickering
3. **Battery Drain**: Constant repainting drains battery on laptops
4. **Scalability**: Performance degrades with number of regions

**Solutions:**
1. **Viewport Culling**: Only render regions visible in viewport
2. **Coordinate Caching**: Cache canvas coordinates, only recalculate on zoom/pan
3. **Dirty Regions**: Only repaint changed regions, not entire canvas
4. **Optimize Hit Testing**: Use spatial data structure (quadtree) for hit testing
5. **Debounce Updates**: Debounce mouse move events to reduce repaints
6. **Use QGraphicsView**: Consider QGraphicsView for better performance (future optimization)

**Prevention Checklist:**
- [ ] Test with 100+ regions
- [ ] Profile paintEvent() performance
- [ ] Implement viewport culling
- [ ] Cache coordinate transformations
- [ ] Optimize hit testing

---

## Event Handling & State Management Risks

### Risk 8: Event Handling Complexity

**Problem:**
- Canvas needs to handle **many mouse events**:
  - Click (single, double)
  - Drag (region creation, movement, resize)
  - Move (hover detection)
  - Wheel (zoom, scroll)
- Plus **keyboard events**:
  - Shortcuts (zoom, select all, etc.)
  - Modifiers (Command, Shift, etc.)
- **Complex state machine** with many edge cases

**Potential Issues:**
1. **Event Conflicts**: Mouse events interfere with each other
2. **State Confusion**: Not knowing which operation is active (creating, moving, resizing)
3. **Modifier Key Handling**: Command vs. Ctrl, Shift vs. Alt differences
4. **Event Propagation**: Events bubbling to parent widgets
5. **Missing Events**: Mouse release not received (window loses focus)

**Solutions:**
1. **State Machine**: Use enum for operation state (Idle, Creating, Moving, Resizing)
2. **Clear State Transitions**: Document all state transitions
3. **Event Filtering**: Filter events early, handle only relevant ones
4. **Platform-Agnostic Modifiers**: Use `QApplication::keyboardModifiers()` for cross-platform
5. **Handle Focus Loss**: Save state on focus loss, cancel operations
6. **Unit Tests**: Test event handling with mock events

**Prevention Checklist:**
- [ ] Design state machine for operations
- [ ] Document all event handling paths
- [ ] Test modifier key combinations
- [ ] Handle focus loss scenarios
- [ ] Test edge cases (rapid clicking, drag outside window)

---

### Risk 9: Selection State Synchronization

**Problem:**
- Selection state exists in **multiple places**:
  - Canvas widget (for rendering)
  - DocumentState (for data)
  - Side panel listbox (for UI)
- **State can get out of sync** if not carefully managed

**Potential Issues:**
1. **Stale Selection**: Selection in canvas doesn't match listbox
2. **Duplicate Selection**: Same region selected in multiple places
3. **Selection Lost**: Selection cleared when it shouldn't be
4. **Performance**: Updating all three places on every selection change

**Solutions:**
1. **Single Source of Truth**: Use DocumentState as source of truth
2. **Observer Pattern**: Canvas and listbox observe DocumentState changes
3. **Signals/Slots**: Use Qt signals to notify all observers
4. **Batch Updates**: Batch selection changes, update all at once
5. **Validation**: Validate selection state after every change

**Prevention Checklist:**
- [ ] Design selection state management
- [ ] Use DocumentState as source of truth
- [ ] Implement observer pattern or signals/slots
- [ ] Test selection synchronization
- [ ] Add validation for selection state

---

## Integration & Architecture Risks

### Risk 10: MainWindow Integration

**Problem:**
- Canvas needs to integrate with **MainWindow**:
  - Replace placeholder in layout
  - Connect to toolbar buttons (zoom, mode)
  - Connect to control panel (region name, color)
  - Update status bar
  - Handle file loading
- **Tight coupling** can make testing and maintenance difficult

**Potential Issues:**
1. **Circular Dependencies**: MainWindow depends on Canvas, Canvas depends on MainWindow
2. **Signal/Slot Spaghetti**: Too many signal/slot connections
3. **Hard to Test**: Can't test Canvas without MainWindow
4. **Refactoring Difficulty**: Changes to MainWindow break Canvas

**Solutions:**
1. **Interface/Abstract Class**: Define Canvas interface, MainWindow uses interface
2. **Dependency Injection**: Pass dependencies to Canvas constructor
3. **Minimal Coupling**: Canvas only depends on what it needs (DocumentState, not MainWindow)
4. **Event Bus**: Use event bus or mediator pattern for communication
5. **Incremental Integration**: Integrate one feature at a time, test each

**Prevention Checklist:**
- [ ] Design Canvas-MainWindow interface
- [ ] Minimize dependencies
- [ ] Use dependency injection
- [ ] Test Canvas independently
- [ ] Document integration points

---

### Risk 11: Undo/Redo Integration

**Problem:**
- Undo/Redo buttons exist but **not yet implemented**
- Need to save state before every operation
- **State snapshots can be large** (all regions, all groups)
- **When to implement?** Phase 2 or Phase 3?

**Potential Issues:**
1. **Memory Usage**: Storing full state snapshots uses lots of memory
2. **Performance**: Deep copying DocumentState is slow
3. **State Management**: When to save state? Before every operation?
4. **Integration Complexity**: Need to integrate with all operations (create, move, resize, delete)

**Solutions:**
1. **Command Pattern**: Use command pattern instead of state snapshots
2. **Lazy Implementation**: Implement undo/redo in Phase 3 (not Phase 2)
3. **Incremental State**: Only store changed regions, not full state
4. **State Limits**: Limit undo stack size (e.g., 50 operations)
5. **Design for Undo**: Design operations to be reversible from the start

**Prevention Checklist:**
- [ ] Decide: Phase 2 or Phase 3 for undo/redo?
- [ ] Design undo/redo architecture
- [ ] Make operations reversible
- [ ] Plan for memory usage
- [ ] Document undo/redo strategy

---

## Testing & Quality Risks

### Risk 12: Testing Complexity

**Problem:**
- Canvas is **hard to test**:
  - Requires Qt application context
  - Requires rendering (visual testing)
  - Requires mouse/keyboard events
  - Requires PDF files
- **Integration tests** are complex and slow

**Potential Issues:**
1. **No Unit Tests**: Can't easily unit test rendering logic
2. **Manual Testing Only**: Relying on manual testing is error-prone
3. **Test Data**: Need test PDFs of various types
4. **Visual Regression**: Hard to detect visual regressions
5. **Cross-Platform Testing**: Tests may behave differently on different platforms

**Solutions:**
1. **Separate Logic from Rendering**: Extract logic to testable functions
2. **Mock Dependencies**: Mock DocumentState, CoordinateSystem for unit tests
3. **Qt Test Framework**: Use Qt Test for widget testing
4. **Test PDFs**: Create collection of test PDFs (various sizes, types)
5. **Automated Visual Tests**: Use screenshot comparison for visual regression
6. **Incremental Testing**: Test each feature as it's implemented

**Prevention Checklist:**
- [ ] Design testable architecture
- [ ] Create test PDF collection
- [ ] Set up Qt Test framework
- [ ] Plan visual regression testing
- [ ] Document testing strategy

---

## Platform-Specific Risks

### Risk 13: Cross-Platform Differences

**Problem:**
- Application must work on **macOS, Windows, Linux**
- Each platform has different:
  - Qt behavior
  - Poppler installation
  - Mouse/keyboard event handling
  - File system paths
  - Font rendering

**Potential Issues:**
1. **Poppler Installation**: Different on each platform
2. **Modifier Keys**: Command (macOS) vs. Ctrl (Windows/Linux)
3. **Mouse Behavior**: Different mouse button handling
4. **Font Rendering**: Text may render differently
5. **Path Separators**: File paths differ (`/` vs `\`)

**Solutions:**
1. **Platform-Agnostic Code**: Use Qt's platform abstraction
2. **Test on All Platforms**: Test on macOS, Windows, Linux
3. **Platform-Specific Builds**: Document platform-specific build steps
4. **Use Qt Types**: Use `QString`, `QFileInfo` for paths, not `std::string`
5. **Modifier Key Abstraction**: Use `QApplication::keyboardModifiers()`

**Prevention Checklist:**
- [ ] Test on all target platforms
- [ ] Document platform-specific issues
- [ ] Use Qt's platform abstraction
- [ ] Handle platform differences in code
- [ ] Create platform-specific test cases

---

## Build & Deployment Risks

### Risk 14: CMake Configuration Complexity

**Problem:**
- Adding Poppler to CMake may be **complex**:
  - Poppler may not have proper CMake config
  - May need to find libraries manually
  - May need to handle optional dependencies
  - Cross-platform CMake is tricky

**Potential Issues:**
1. **CMake Can't Find Poppler**: `find_package(Poppler)` fails
2. **Wrong Libraries Linked**: Linking wrong Poppler libraries
3. **Missing Dependencies**: Poppler dependencies not found
4. **vcpkg Integration**: vcpkg Poppler may not work with CMake
5. **Cross-Platform CMake**: CMake config works on one platform but not others

**Solutions:**
1. **Test CMake Early**: Test Poppler CMake integration before coding
2. **Fallback Options**: Provide manual library paths if auto-detection fails
3. **Clear Error Messages**: Give user-friendly error messages if Poppler not found
4. **Document CMake Setup**: Document CMake configuration for all platforms
5. **Use vcpkg Toolchain**: Ensure vcpkg toolchain is properly configured

**Prevention Checklist:**
- [ ] Test CMake Poppler integration
- [ ] Create fallback CMake config
- [ ] Document CMake setup
- [ ] Test on all platforms
- [ ] Provide clear error messages

---

## Summary: Critical Path & Priorities

### Must Solve Before Starting
1. ✅ **Poppler Installation** - Verify Poppler works before coding
2. ✅ **Canvas Architecture** - Design architecture before implementation
3. ✅ **Coordinate System Integration** - Test transformations thoroughly
4. ✅ **Mouse Coordinate Conversion** - Get this right from the start

### High Priority (Solve Early)
5. ✅ **Large PDF Memory** - Handle memory issues early
6. ✅ **Event Handling** - Design state machine before coding
7. ✅ **Selection Synchronization** - Design state management

### Medium Priority (Solve During Development)
8. ✅ **Performance Optimization** - Optimize as needed
9. ✅ **Testing Strategy** - Set up testing framework
10. ✅ **MainWindow Integration** - Integrate incrementally

### Lower Priority (Can Defer)
11. ✅ **Undo/Redo** - Can implement in Phase 3
12. ✅ **Cross-Platform** - Test on all platforms before release
13. ✅ **CMake Complexity** - Handle as needed

---

## Action Plan

### Before Starting Phase 2

1. **Install and Test Poppler**
   - [ ] Install Poppler via vcpkg or system package manager
   - [ ] Verify CMake can find Poppler
   - [ ] Create minimal Poppler test program
   - [ ] Test PDF loading with various PDFs

2. **Design Canvas Architecture**
   - [ ] Design Canvas class structure
   - [ ] Design state management
   - [ ] Design event handling
   - [ ] Document architecture decisions

3. **Test Coordinate System**
   - [ ] Test all transformation functions
   - [ ] Test at extreme zoom levels
   - [ ] Create visual coordinate grid for debugging

4. **Set Up Testing Framework**
   - [ ] Set up Qt Test framework
   - [ ] Create test PDF collection
   - [ ] Plan testing strategy

### During Phase 2 Development

1. **Incremental Development**
   - Build one feature at a time
   - Test each feature before moving on
   - Don't skip steps

2. **Continuous Testing**
   - Test with various PDFs
   - Test with many regions
   - Test edge cases
   - Monitor performance

3. **Documentation**
   - Document design decisions
   - Document known issues
   - Document workarounds

---

## Conclusion

Phase 2 has **significant complexity** but all identified risks have **clear solutions**. The key is to:

1. **Solve setup issues first** (Poppler installation, CMake)
2. **Design architecture before coding** (Canvas structure, state management)
3. **Test thoroughly** (coordinate transformations, event handling)
4. **Develop incrementally** (one feature at a time)
5. **Monitor performance** (memory usage, rendering speed)

**With proper planning and incremental development, Phase 2 is achievable.**

---

**Next Steps:**
1. Install and test Poppler
2. Design Canvas architecture
3. Create minimal Canvas implementation
4. Test PDF loading
5. Proceed with incremental feature development

