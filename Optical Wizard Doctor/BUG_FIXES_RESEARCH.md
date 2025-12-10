# Bug Fixes Research & Best Practices
Generated: $(date)

## Overview
This document compiles researched fixes and best practices for the bugs identified in `BUG_INVESTIGATION.md`. Each section provides specific, actionable solutions based on common patterns from Stack Overflow, Qt documentation, and C++ best practices.

---

## 1. State Flag Reset Issues

### Problem
State flags (isCreating, isDragging, isResizing, isRotating, isPanning) may not be reset in all error paths, causing stuck states.

### Research Findings
- **RAII Pattern**: Use RAII (Resource Acquisition Is Initialization) to ensure cleanup
- **Guard Classes**: Create guard classes that automatically reset flags on destruction
- **Early Return Pattern**: Always reset flags before early returns

### Recommended Fix

**Option 1: Guard Class (Recommended)**
```cpp
class StateFlagGuard {
public:
    StateFlagGuard(bool& flag) : flag_(flag) { flag_ = true; }
    ~StateFlagGuard() { flag_ = false; }
    void release() { flag_ = false; }
private:
    bool& flag_;
};

// Usage in handlePress:
if (!documentState || documentImage.isNull()) {
    // Guard automatically resets flags on scope exit
    return;
}
```

**Option 2: Explicit Reset Function**
```cpp
void CanvasMouseHandler::resetAllFlags() {
    isCreating = false;
    isDragging = false;
    isResizing = false;
    isRotating = false;
    isPanning = false;
    isBoxSelecting = false;
    // ... reset all flags
}

// Call before every early return:
if (!documentState || documentImage.isNull()) {
    resetAllFlags();
    return;
}
```

**Option 3: State Machine Pattern**
Use a state machine enum instead of multiple boolean flags:
```cpp
enum class MouseState {
    Idle,
    Creating,
    Dragging,
    Resizing,
    Rotating,
    Panning,
    BoxSelecting
};
```

### Implementation Priority
- **High**: Add `resetAllFlags()` and call it before all early returns
- **Medium**: Consider guard classes for complex operations
- **Low**: Refactor to state machine (long-term improvement)

---

## 2. NaN/Infinity Checks in Floating Point Calculations

### Problem
Rotation and coordinate calculations can produce NaN or Infinity values, leading to invalid regions.

### Research Findings
- **std::isnan()**: Check for NaN values
- **std::isfinite()**: Check for both NaN and Infinity
- **Early Validation**: Check inputs before calculations
- **Defensive Programming**: Validate all floating point results

### Recommended Fix

**Add Validation Helper:**
```cpp
#include <cmath>

inline bool isValidDouble(double value) {
    return std::isfinite(value) && !std::isnan(value);
}

inline bool isValidPoint(const QPointF& point) {
    return isValidDouble(point.x()) && isValidDouble(point.y());
}

inline bool isValidNormalizedCoords(const NormalizedCoords& coords) {
    return isValidDouble(coords.x1) && isValidDouble(coords.y1) &&
           isValidDouble(coords.x2) && isValidDouble(coords.y2);
}
```

**Apply to Rotation Function:**
```cpp
void CanvasRegionOperations::rotateRegion(...) {
    // ... existing checks ...
    
    // Validate rotation angle
    if (!isValidDouble(rotationAngle) || std::abs(rotationAngle) > 360.0) {
        return; // Invalid angle
    }
    
    // Validate image dimensions
    if (imgWidth <= 0 || imgHeight <= 0) {
        return;
    }
    
    // Validate input coordinates
    if (!isValidNormalizedCoords(region.normalizedCoords)) {
        return;
    }
    
    // ... rotation calculations ...
    
    // Validate rotated points before min/max
    if (!isValidPoint(p1) || !isValidPoint(p2) || 
        !isValidPoint(p3) || !isValidPoint(p4)) {
        return; // Invalid rotation result
    }
    
    // Validate bounding box result
    if (!isValidDouble(minX) || !isValidDouble(maxX) ||
        !isValidDouble(minY) || !isValidDouble(maxY)) {
        return;
    }
    
    // ... rest of function ...
}
```

**Apply to Coordinate Conversions:**
```cpp
NormalizedCoords CoordinateSystem::imageToNormalized(...) {
    if (imgWidth <= 0 || imgHeight <= 0) {
        return NormalizedCoords(0.0, 0.0, 0.0, 0.0);
    }
    
    NormalizedCoords norm;
    norm.x1 = static_cast<double>(img.x1) / imgWidth;
    // ... other conversions ...
    
    // Validate result
    if (!isValidNormalizedCoords(norm)) {
        return NormalizedCoords(0.0, 0.0, 0.0, 0.0);
    }
    
    clampToBounds(norm);
    return norm;
}
```

### Implementation Priority
- **Critical**: Add NaN checks to rotation calculations
- **High**: Add validation to all coordinate conversion functions
- **Medium**: Add validation to all floating point operations

---

## 3. Coordinate Clamping and Ordering

### Problem
After clamping, coordinates might have x1 >= x2 or y1 >= y2, creating invalid regions.

### Research Findings
- **Post-Clamp Validation**: Always verify ordering after clamping
- **Minimum Size Enforcement**: Ensure minimum size after clamping
- **Boundary Handling**: Handle edge cases at boundaries (0.0, 1.0)

### Recommended Fix

**Update constrainToBounds:**
```cpp
void CanvasRegionOperations::constrainToBounds(RegionData& region) {
    NormalizedCoords& norm = region.normalizedCoords;
    
    // Ensure x1 < x2 and y1 < y2 BEFORE clamping
    if (norm.x1 > norm.x2) std::swap(norm.x1, norm.x2);
    if (norm.y1 > norm.y2) std::swap(norm.y1, norm.y2);
    
    // Clamp to bounds
    norm.x1 = std::clamp(norm.x1, 0.0, 1.0);
    norm.y1 = std::clamp(norm.y1, 0.0, 1.0);
    norm.x2 = std::clamp(norm.x2, 0.0, 1.0);
    norm.y2 = std::clamp(norm.y2, 0.0, 1.0);
    
    // CRITICAL: Re-validate ordering after clamping
    // Clamping might have made x1 == x2 or y1 == y2
    if (norm.x1 >= norm.x2) {
        // Region too small or invalid - enforce minimum size
        const double minSize = 0.001;
        if (norm.x1 >= 1.0 - minSize) {
            norm.x1 = 1.0 - minSize;
            norm.x2 = 1.0;
        } else {
            norm.x2 = norm.x1 + minSize;
        }
    }
    
    if (norm.y1 >= norm.y2) {
        const double minSize = 0.001;
        if (norm.y1 >= 1.0 - minSize) {
            norm.y1 = 1.0 - minSize;
            norm.y2 = 1.0;
        } else {
            norm.y2 = norm.y1 + minSize;
        }
    }
    
    // Final validation
    if (norm.x1 >= norm.x2 || norm.y1 >= norm.y2) {
        // Last resort: create minimal valid region
        norm.x1 = 0.0;
        norm.y1 = 0.0;
        norm.x2 = minSize;
        norm.y2 = minSize;
    }
    
    // Ensure minimum size
    const double minSize = 0.001;
    if (norm.x2 - norm.x1 < minSize) {
        norm.x2 = std::min(1.0, norm.x1 + minSize);
        if (norm.x2 > 1.0) {
            norm.x1 = std::max(0.0, norm.x2 - minSize);
        }
    }
    if (norm.y2 - norm.y1 < minSize) {
        norm.y2 = std::min(1.0, norm.y1 + minSize);
        if (norm.y2 > 1.0) {
            norm.y1 = std::max(0.0, norm.y2 - minSize);
        }
    }
}
```

### Implementation Priority
- **Critical**: Fix constrainToBounds to always ensure x1 < x2 and y1 < y2
- **High**: Add validation after all coordinate operations

---

## 4. Image Dimension Validation

### Problem
No validation that imgWidth > 0 and imgHeight > 0 before using in calculations.

### Research Findings
- **Early Validation**: Check dimensions at function entry
- **Defensive Programming**: Validate before every use
- **Consistent Pattern**: Use same validation pattern everywhere

### Recommended Fix

**Add Validation Helper:**
```cpp
inline bool isValidImageDimensions(int width, int height) {
    return width > 0 && height > 0;
}
```

**Apply to All Functions:**
```cpp
void DocumentState::synchronizeCoordinates() {
    if (image.isNull() || !isValidImageDimensions(image.width(), image.height())) {
        return; // Cannot synchronize without valid image
    }
    
    int imgWidth = image.width();
    int imgHeight = image.height();
    // ... rest of function ...
}

void CanvasRegionOperations::rotateRegion(...) {
    // ... existing checks ...
    
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    
    if (!isValidImageDimensions(imgWidth, imgHeight)) {
        return; // Invalid image dimensions
    }
    
    // ... rest of function ...
}
```

### Implementation Priority
- **Critical**: Add validation to synchronizeCoordinates
- **High**: Add validation to all region operations
- **Medium**: Add validation to all coordinate conversion functions

---

## 5. Selection Consistency Validation

### Problem
primarySelectedRegion might not be in selectedRegions after operations.

### Research Findings
- **Invariant Checking**: Maintain selection invariants
- **Validation Function**: Create function to validate selection state
- **Auto-Correction**: Automatically fix inconsistencies

### Recommended Fix

**Add Validation Function:**
```cpp
void CanvasSelectionManager::validateSelection(
    QSet<QString>& selectedRegions,
    QString& primarySelectedRegion) {
    
    // If primary is not in selected set, clear it
    if (!primarySelectedRegion.isEmpty() && 
        !selectedRegions.contains(primarySelectedRegion)) {
        primarySelectedRegion = QString();
    }
    
    // If no selection but primary exists, clear primary
    if (selectedRegions.isEmpty() && !primarySelectedRegion.isEmpty()) {
        primarySelectedRegion = QString();
    }
    
    // If selection exists but no primary, set first as primary
    if (!selectedRegions.isEmpty() && primarySelectedRegion.isEmpty()) {
        primarySelectedRegion = *selectedRegions.begin();
    }
}
```

**Call After Every Selection Change:**
```cpp
void CanvasSelectionManager::toggleRegionSelection(...) {
    // ... existing logic ...
    
    // Validate selection state
    validateSelection(selectedRegions, primarySelectedRegion);
}
```

### Implementation Priority
- **High**: Add validation function and call after all selection operations
- **Medium**: Add assertions in debug builds

---

## 6. Undo/Redo Image Restoration

### Problem
Image is not restored in undo/redo, only pdfPath. If PDF was moved/deleted, undo/redo fails.

### Research Findings
- **Lazy Loading**: Reload image on demand
- **Error Handling**: Handle missing PDF gracefully
- **State Validation**: Validate state after restore

### Recommended Fix

**Update restoreState:**
```cpp
void DocumentState::restoreState(const StateSnapshot& snapshot) {
    // Restore core data
    regions = snapshot.regions;
    groups = snapshot.groups;
    pdfPath = snapshot.pdfPath;
    
    // Attempt to reload image if path exists
    if (!pdfPath.isEmpty()) {
        QFileInfo fileInfo(pdfPath);
        if (fileInfo.exists() && fileInfo.isReadable()) {
            // Reload image from PDF
            QImage reloadedImage = PdfLoader::loadPdfAsImage(pdfPath);
            if (!reloadedImage.isNull()) {
                image = reloadedImage;
            } else {
                // PDF exists but couldn't load - keep current image or clear?
                // Option: Keep current image, or clear it
                // image = QImage(); // Clear if load fails
            }
        } else {
            // PDF file doesn't exist - clear image
            image = QImage();
        }
    } else {
        // No PDF path - clear image
        image = QImage();
    }
    
    // Only synchronize if we have a valid image
    if (!image.isNull() && isValidImageDimensions(image.width(), image.height())) {
        synchronizeCoordinates();
    } else {
        // Clear coordinates if no valid image
        for (auto& region : regions) {
            region.imageCoords = ImageCoords();
            region.canvasCoords = CanvasCoords();
        }
    }
}
```

### Implementation Priority
- **High**: Add image reload logic to restoreState
- **Medium**: Add error handling for missing PDFs

---

## 7. File Operation Error Handling

### Problem
File operations might leave temp files on crash, or fail silently.

### Research Findings
- **RAII for Files**: Use RAII to ensure cleanup
- **Try-Catch**: Wrap file operations in try-catch
- **Atomic Writes**: Use temp files and rename (already implemented, but needs cleanup)

### Recommended Fix

**Add RAII File Helper:**
```cpp
class TempFileGuard {
public:
    TempFileGuard(const QString& path) : path_(path) {}
    ~TempFileGuard() {
        if (QFile::exists(path_)) {
            QFile::remove(path_);
        }
    }
    void release() { path_.clear(); }
private:
    QString path_;
};
```

**Update Atomic Write Functions:**
```cpp
bool JsonExporter::saveToFile(const QString& filePath, ...) {
    QString tempPath = filePath + ".tmp";
    TempFileGuard guard(tempPath);
    
    try {
        // Write to temp file
        QFile file(tempPath);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        
        // ... write data ...
        
        file.close();
        
        // Atomic rename
        if (QFile::exists(filePath)) {
            QFile::remove(filePath);
        }
        if (!QFile::rename(tempPath, filePath)) {
            return false;
        }
        
        guard.release(); // Success - don't delete
        return true;
    } catch (...) {
        // Temp file will be deleted by guard
        return false;
    }
}
```

### Implementation Priority
- **High**: Add temp file cleanup to all file operations
- **Medium**: Add try-catch blocks around file I/O

---

## 8. Signal/Slot Recursion Prevention

### Problem
Signal/slot connections might cause infinite loops despite blockSignals.

### Research Findings
- **Recursion Guards**: Use boolean flags to prevent recursion
- **QSignalBlocker**: Use Qt's QSignalBlocker class
- **Connection Types**: Use Qt::DirectConnection vs Qt::QueuedConnection appropriately

### Recommended Fix

**Use QSignalBlocker:**
```cpp
#include <QtCore/QSignalBlocker>

void SidePanelWidget::updateCoordinateValue(...) {
    // Prevent recursion
    QSignalBlocker blocker(this);
    
    // ... update logic ...
    
    // Signals are automatically unblocked when blocker goes out of scope
}
```

**Add Recursion Guard:**
```cpp
class SidePanelWidget : public QWidget {
private:
    bool updatingCoordinates_ = false;
    
public:
    void updateCoordinateValue(...) {
        if (updatingCoordinates_) {
            return; // Prevent recursion
        }
        
        updatingCoordinates_ = true;
        
        // ... update logic ...
        
        updatingCoordinates_ = false;
    }
};
```

### Implementation Priority
- **High**: Add QSignalBlocker to all coordinate update functions
- **Medium**: Add recursion guards for complex update chains

---

## 9. Region Name Validation

### Problem
Region names might contain invalid characters or be too long.

### Research Findings
- **Character Validation**: Check for control characters, newlines, etc.
- **Length Limits**: Enforce maximum length
- **Reserved Names**: Check for reserved/system names

### Recommended Fix

**Update isValidRegionName:**
```cpp
bool DocumentState::isValidRegionName(const QString& name) const {
    // Check if empty
    if (name.isEmpty()) {
        return false;
    }
    
    // Check maximum length (e.g., 255 characters)
    if (name.length() > 255) {
        return false;
    }
    
    // Check for invalid characters (control chars, newlines, etc.)
    for (QChar ch : name) {
        if (ch.isControl() || ch == '\n' || ch == '\r' || ch == '\t') {
            return false;
        }
    }
    
    // Check for reserved names (if any)
    static const QStringList reservedNames = {"", "null", "undefined"};
    if (reservedNames.contains(name, Qt::CaseInsensitive)) {
        return false;
    }
    
    // Check if unique
    if (regions.contains(name)) {
        return false;
    }
    
    return true;
}
```

### Implementation Priority
- **Medium**: Add character and length validation
- **Low**: Add reserved name checking

---

## 10. Group Cleanup on Region Removal

### Problem
Empty groups are not deleted when last region is removed.

### Research Findings
- **Cleanup on Remove**: Delete empty groups after region removal
- **Periodic Cleanup**: Optionally clean up empty groups periodically

### Recommended Fix

**Update removeRegion:**
```cpp
void DocumentState::removeRegion(const QString& regionName) {
    if (!regions.contains(regionName)) {
        return;
    }
    
    // Get group before removal
    QString groupName = regions[regionName].group;
    
    // Remove region
    regions.remove(regionName);
    
    // Remove from group
    if (!groupName.isEmpty() && groups.contains(groupName)) {
        groups[groupName].remove(regionName);
        
        // Delete group if empty
        if (groups[groupName].isEmpty()) {
            groups.remove(groupName);
        }
    }
}
```

### Implementation Priority
- **Medium**: Add group cleanup to removeRegion
- **Low**: Add periodic cleanup function

---

## 11. Resize Handle Validation

### Problem
No validation that resize handle string is valid before using contains().

### Research Findings
- **Early Validation**: Validate handle string before use
- **Enum Instead of String**: Consider using enum for handles

### Recommended Fix

**Add Validation:**
```cpp
bool isValidResizeHandle(const QString& handle) {
    static const QStringList validHandles = {
        "top-left", "top", "top-right",
        "left", "right",
        "bottom-left", "bottom", "bottom-right"
    };
    return validHandles.contains(handle);
}

void CanvasRegionOperations::resizeRegion(...) {
    if (!isValidResizeHandle(handle)) {
        return; // Invalid handle
    }
    
    // ... rest of function ...
}
```

### Implementation Priority
- **Medium**: Add handle validation
- **Low**: Consider refactoring to enum

---

## 12. JSON Import Duplicate Handling

### Problem
Duplicate region names in JSON silently overwrite existing regions.

### Research Findings
- **Warning System**: Warn user about duplicates
- **Auto-Rename**: Automatically rename duplicates
- **Error Reporting**: Report errors to user

### Recommended Fix

**Update loadRegions:**
```cpp
void JsonImporter::loadRegions(DocumentState& state, const QJsonArray& regionsArray) {
    QStringList duplicateNames;
    
    for (const QJsonValue& value : regionsArray) {
        // ... parse region ...
        
        QString regionName = obj["name"].toString();
        
        // Check for duplicates
        if (state.hasRegion(regionName)) {
            duplicateNames.append(regionName);
            // Option 1: Skip duplicate
            continue;
            // Option 2: Auto-rename
            // regionName = generateUniqueName(regionName, state);
        }
        
        // ... add region ...
    }
    
    // Report duplicates to user
    if (!duplicateNames.isEmpty()) {
        qWarning() << "Duplicate region names found:" << duplicateNames;
        // Could emit signal or show dialog
    }
}
```

### Implementation Priority
- **Medium**: Add duplicate detection and warning
- **Low**: Add auto-rename option

---

## 13. CSV Export Special Character Handling

### Problem
CSV export might not handle all special characters correctly.

### Research Findings
- **RFC 4180 Compliance**: Follow CSV standard
- **Quote Escaping**: Escape quotes by doubling them
- **Field Wrapping**: Wrap fields containing special chars in quotes

### Recommended Fix

**Update escapeCsvField:**
```cpp
QString CsvExporter::escapeCsvField(const QString& field) {
    // Check if field contains special characters
    bool needsQuotes = field.contains(',') || 
                       field.contains('"') || 
                       field.contains('\n') || 
                       field.contains('\r');
    
    if (!needsQuotes) {
        return field;
    }
    
    // Wrap in quotes and escape internal quotes
    QString escaped = field;
    escaped.replace("\"", "\"\""); // Double quotes
    return "\"" + escaped + "\"";
}
```

### Implementation Priority
- **Medium**: Improve CSV escaping
- **Low**: Add CSV validation

---

## 14. Mask Generation Coordinate Validation

### Problem
Uses imageCoords directly, which might be stale.

### Research Findings
- **Source of Truth**: Always use normalized coordinates
- **Re-sync Before Use**: Ensure coordinates are synchronized

### Recommended Fix

**Update createMaskImage:**
```cpp
QImage MaskGenerator::createMaskImage(...) {
    // ... existing code ...
    
    for (const RegionData& region : regions) {
        // Use normalized coordinates (source of truth)
        NormalizedCoords norm = region.normalizedCoords;
        
        // Convert to image coordinates
        ImageCoords img = CoordinateSystem::normalizedToImage(
            norm, imageWidth, imageHeight
        );
        
        // Validate coordinates
        if (!CoordinateSystem::isValidImage(img, imageWidth, imageHeight)) {
            continue; // Skip invalid region
        }
        
        // Use img coordinates for mask
        // ... rest of function ...
    }
}
```

### Implementation Priority
- **High**: Fix mask generation to use normalized coordinates
- **Medium**: Add coordinate validation

---

## 15. Zoom Level Bounds Checking

### Problem
Zoom might become exactly 0.0, causing division by zero.

### Research Findings
- **Minimum Zoom**: Enforce minimum zoom > 0
- **Validation**: Check zoom before use in calculations

### Recommended Fix

**Update setZoom:**
```cpp
void CanvasZoomController::setZoom(double zoom) {
    // Enforce minimum zoom
    const double MIN_ZOOM = 0.01; // Never allow exactly 0.0
    const double MAX_ZOOM = 10.0;
    
    zoom = std::clamp(zoom, MIN_ZOOM, MAX_ZOOM);
    
    // Validate
    if (!isValidDouble(zoom) || zoom <= 0.0) {
        zoom = 1.0; // Default to 1.0 if invalid
    }
    
    scaleFactor = zoom;
    // ... rest of function ...
}
```

### Implementation Priority
- **Critical**: Enforce minimum zoom > 0
- **High**: Add validation to all zoom functions

---

## Implementation Priority Summary

### Critical (Fix Immediately)
1. Zoom level bounds checking (prevent division by zero)
2. Coordinate clamping ordering (ensure x1 < x2, y1 < y2)

### High (Fix Soon)
3. NaN/Infinity checks in rotation calculations
4. Image dimension validation
5. State flag resets in error paths
6. Mask generation coordinate validation

### Medium (Fix When Possible)
7. Selection consistency validation
8. Undo/redo image restoration
9. File operation error handling
10. Signal/slot recursion prevention
11. Region name validation
12. Group cleanup
13. Resize handle validation
14. JSON import duplicate handling
15. CSV export special character handling

### Low (Nice to Have)
16. State machine refactoring
17. Resize handle enum
18. Periodic group cleanup
19. CSV validation

---

## Testing Recommendations

After implementing fixes, test:

1. **Edge Cases**:
   - Very small regions (< 1 pixel)
   - Very large images
   - Extreme zoom levels (0.01, 10.0)
   - Rapid mouse movements
   - Multiple rapid operations

2. **Error Conditions**:
   - Invalid input files
   - Missing PDF files
   - Corrupted JSON/CSV
   - Network interruptions during save

3. **State Transitions**:
   - All mouse operation sequences
   - Undo/redo with missing files
   - Selection changes during operations
   - Window minimize during operations

4. **Coordinate Validation**:
   - NaN/Infinity inputs
   - Zero/negative dimensions
   - Out-of-bounds coordinates
   - Invalid region names

---

## References

- Qt Documentation: https://doc.qt.io/
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/
- Stack Overflow: Common Qt/C++ patterns
- RAII Pattern: Resource management best practices
- Defensive Programming: Input validation patterns

