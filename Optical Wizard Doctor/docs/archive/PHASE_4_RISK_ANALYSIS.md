# Phase 4: Export/Import - Comprehensive Risk Analysis

## Executive Summary

This document identifies **all potential problems, obstacles, difficulties, and risks** before implementing Phase 4. The goal is to be **preemptive** with problem-solving and avoid coding into corners.

**Status:** Pre-Implementation Analysis  
**Date:** December 2024  
**Purpose:** Identify and mitigate risks before development begins

---

## Critical Dependencies & Integration Challenges

### Risk 1: JSON Export/Import Round-Trip Data Loss

**Problem:**
- Exporting and then importing the same data must result in **identical state**
- Coordinate precision can be lost during JSON serialization (double → JSON → double)
- Floating-point rounding errors can accumulate
- Missing or extra fields in JSON can cause import failures
- Version compatibility: future JSON format changes must be handled

**Potential Issues:**
1. **Precision Loss**: JSON stores doubles as strings, precision may be lost
2. **Field Mismatch**: Export includes fields that import doesn't expect (or vice versa)
3. **Coordinate System Confusion**: Exporting wrong coordinate system (canvas/image instead of normalized)
4. **Group Reconstruction**: Groups might be reconstructed differently on import
5. **Empty Values**: Null vs empty string vs missing field handling
6. **Unicode/Encoding**: Special characters in region names might not survive round-trip
7. **Ordering**: Region/group order might change (affects UI but not functionality)

**Solutions:**
1. **Use Normalized Coordinates Only**: Always export normalized (0.0-1.0), never image/canvas
2. **High Precision Formatting**: Use 6+ decimal places for coordinates in JSON
3. **Strict Schema Validation**: Validate JSON structure on import, reject invalid files
4. **Version Field**: Add version field to JSON for future compatibility
5. **Round-Trip Testing**: Comprehensive tests: export → import → export, compare
6. **UTF-8 Encoding**: Ensure all file I/O uses UTF-8 encoding
7. **Field Mapping**: Document all fields, handle missing fields gracefully
8. **Coordinate Validation**: Validate all coordinates are 0.0-1.0 on import

**Prevention Checklist:**
- [ ] Document exact JSON schema
- [ ] Test coordinate precision (6 decimal places minimum)
- [ ] Test round-trip with various data sets
- [ ] Test with special characters in names
- [ ] Test with empty groups, no groups, all edge cases
- [ ] Add version field to JSON
- [ ] Validate JSON structure on import
- [ ] Handle missing/extra fields gracefully

---

### Risk 2: CSV Export Format Compatibility

**Problem:**
- CSV must be readable by Excel, Google Sheets, LibreOffice, etc.
- Different applications handle CSV escaping differently
- Regional settings (decimal separator, comma vs semicolon)
- Special characters in region names (commas, quotes, newlines)
- Large files might be slow to open in spreadsheets

**Potential Issues:**
1. **Escaping Conflicts**: Excel vs Google Sheets vs LibreOffice handle quotes differently
2. **Decimal Separator**: Some locales use comma for decimals (conflicts with CSV delimiter)
3. **Encoding Issues**: UTF-8 BOM might be needed for Excel
4. **Large Files**: 1000+ regions might be slow to open
5. **Column Width**: Long region names might not display well
6. **Formula Injection**: Region names starting with "=" might be interpreted as formulas
7. **Line Endings**: Windows (CRLF) vs Unix (LF) line endings

**Solutions:**
1. **Standard CSV Escaping**: Use RFC 4180 standard (quotes around fields with commas/quotes)
2. **UTF-8 BOM**: Add BOM for Excel compatibility (optional, make configurable)
3. **Comma Delimiter**: Use comma (standard), document that decimal separator is period
4. **Quote Escaping**: Double quotes for embedded quotes: `"Name with ""quotes"""`
5. **Sanitize Names**: Warn or escape region names starting with "=", "+", "-", "@"
6. **Line Endings**: Use platform-native line endings (Qt handles this)
7. **Test with Multiple Apps**: Verify with Excel, Google Sheets, LibreOffice
8. **Performance**: For large files, consider streaming write (already done with QFile)

**Prevention Checklist:**
- [ ] Test CSV with Excel (Windows/Mac)
- [ ] Test CSV with Google Sheets
- [ ] Test CSV with LibreOffice
- [ ] Test with special characters (commas, quotes, newlines)
- [ ] Test with formula-like names ("=SUM", "+123", etc.)
- [ ] Test with large files (1000+ regions)
- [ ] Verify UTF-8 encoding
- [ ] Document CSV format specification

---

### Risk 3: JSON Import State Restoration

**Problem:**
- Importing JSON must restore **complete application state**
- Must handle missing PDF/image (coordinates only)
- Must handle PDF loaded but aspect ratio mismatch
- Must handle invalid/out-of-bounds coordinates
- Must handle duplicate region names
- Must handle orphaned groups (groups referencing non-existent regions)
- Must preserve undo/redo stack (or clear it?)

**Potential Issues:**
1. **State Inconsistency**: Imported state doesn't match current UI state
2. **Coordinate System Mismatch**: Imported coordinates don't match current image size
3. **Missing PDF**: User imports coordinates but PDF not loaded - how to handle?
4. **Aspect Ratio Mismatch**: Imported coordinates from different PDF size
5. **Invalid Coordinates**: Coordinates outside 0.0-1.0 range
6. **Duplicate Names**: Import creates duplicate region names
7. **Orphaned Groups**: Groups reference regions that don't exist
8. **Undo Stack**: Should import clear undo stack or preserve it?
9. **Selection State**: Should imported regions be selected?
10. **Zoom/Pan State**: Should zoom/pan be reset on import?

**Solutions:**
1. **Clear State Before Import**: Clear existing regions/groups, then import (or merge mode?)
2. **Coordinate Validation**: Validate all coordinates are 0.0-1.0, clamp or reject invalid
3. **Aspect Ratio Warning**: Warn if aspect ratio doesn't match, allow import anyway
4. **Missing PDF Handling**: Allow import without PDF, coordinates stored but not displayed until PDF loaded
5. **Duplicate Name Handling**: Auto-rename duplicates ("Region 1_1", "Region 1_2")
6. **Orphaned Group Cleanup**: Remove groups with no valid regions, or create empty groups
7. **Clear Undo Stack**: Import should clear undo/redo stacks (fresh start)
8. **Reset View**: Reset zoom/pan to fit on import
9. **Coordinate Recalculation**: After import, recalculate image/canvas coordinates if image loaded
10. **Validation Report**: Show summary of imported data (N regions, M groups, warnings)

**Prevention Checklist:**
- [ ] Design import mode (replace vs merge)
- [ ] Implement coordinate validation
- [ ] Handle missing PDF scenario
- [ ] Handle aspect ratio mismatch
- [ ] Implement duplicate name resolution
- [ ] Clean up orphaned groups
- [ ] Clear undo/redo on import
- [ ] Reset zoom/pan on import
- [ ] Test all edge cases
- [ ] Create validation report UI

---

### Risk 4: Mask Generation Coordinate System Confusion

**Problem:**
- Mask must use **image coordinates** (pixels), not canvas or normalized
- Must match original image dimensions exactly
- Coordinate order matters (x1 < x2, y1 < y2)
- Regions might be outside image bounds
- Large images might cause memory issues
- Multiple formats (PNG, JPEG, TIFF) have different requirements

**Potential Issues:**
1. **Wrong Coordinate System**: Using canvas or normalized instead of image coordinates
2. **Dimension Mismatch**: Mask size doesn't match original image
3. **Coordinate Order**: x1 > x2 or y1 > y2 (must be normalized)
4. **Out of Bounds**: Regions extend beyond image boundaries
5. **Memory Usage**: Large images (4K+) might use significant memory
6. **Format Limitations**: JPEG doesn't support transparency, TIFF might not be supported
7. **Color Space**: RGB vs grayscale, color depth
8. **Performance**: Drawing many regions might be slow

**Solutions:**
1. **Explicit Image Coordinates**: Always use `region.imageCoords`, never canvas/normalized
2. **Dimension Validation**: Verify mask dimensions match `state.image.size()`
3. **Coordinate Normalization**: Ensure x1 < x2, y1 < y2 before drawing
4. **Bounds Clamping**: Clamp coordinates to image bounds (0 to width/height)
5. **Memory Management**: Use QImage efficiently, consider streaming for very large images
6. **Format Selection**: PNG for lossless, JPEG for smaller files, document limitations
7. **RGB32 Format**: Use `QImage::Format_RGB32` for consistent color handling
8. **Batch Drawing**: Draw all rectangles in single QPainter session
9. **Performance Testing**: Test with 100+ regions, large images

**Prevention Checklist:**
- [ ] Verify using image coordinates (not canvas/normalized)
- [ ] Test dimension matching
- [ ] Test coordinate normalization
- [ ] Test bounds clamping
- [ ] Test with large images (4K+)
- [ ] Test with many regions (100+)
- [ ] Test all image formats (PNG, JPEG, TIFF)
- [ ] Verify memory usage is acceptable

---

### Risk 5: File I/O Error Handling

**Problem:**
- File operations can fail for many reasons
- Errors must not corrupt existing data
- User must be informed of errors clearly
- Partial writes must be handled (disk full, permission denied)
- File locking on Windows
- Network drives might have different behavior

**Potential Issues:**
1. **Permission Denied**: User doesn't have write permission
2. **Disk Full**: Write fails partway through
3. **File Locked**: File is open in another application
4. **Invalid Path**: Path contains invalid characters
5. **Network Issues**: Network drive disconnects during write
6. **Partial Write**: File is partially written, then error occurs
7. **Atomicity**: Need to ensure file is either completely written or not at all
8. **Backup**: Should we backup existing file before overwriting?

**Solutions:**
1. **Check Permissions First**: Verify write permission before starting
2. **Atomic Writes**: Write to temp file, then rename (ensures atomicity)
3. **Error Messages**: Clear, actionable error messages
4. **Preserve Data**: Never modify existing data until write succeeds
5. **File Locking**: Handle file locked errors gracefully
6. **Path Validation**: Validate file path before attempting write
7. **Disk Space Check**: Check available disk space (optional, might be slow)
8. **Backup Option**: Offer to backup existing file before overwriting
9. **Retry Logic**: For transient errors (network), offer retry option

**Prevention Checklist:**
- [ ] Test permission denied scenario
- [ ] Test disk full scenario
- [ ] Test file locked scenario
- [ ] Test invalid path scenario
- [ ] Implement atomic writes (temp file + rename)
- [ ] Test on network drives
- [ ] Create clear error messages
- [ ] Test partial write recovery

---

### Risk 6: Multiple Export Format Maintenance

**Problem:**
- Supporting 5+ export formats (JSON, CSV, TXT, XML, YAML)
- Each format must stay in sync with data model changes
- Format-specific bugs might only appear in one format
- Testing burden increases with each format
- Some formats might be rarely used

**Potential Issues:**
1. **Format Drift**: Formats get out of sync with data model
2. **Format-Specific Bugs**: Bug in one format doesn't affect others
3. **Testing Overhead**: Must test all formats for each change
4. **Maintenance Burden**: More code to maintain
5. **User Confusion**: Too many format options might confuse users
6. **Format Limitations**: Some formats can't represent all data (CSV vs JSON)

**Solutions:**
1. **Shared Core Logic**: Extract common export logic, format-specific only for serialization
2. **Format Abstraction**: Create exporter interface, implement per format
3. **Comprehensive Testing**: Test all formats in CI/CD
4. **Format Priority**: Focus on JSON (primary), CSV (common), others as needed
5. **Documentation**: Document which formats support which features
6. **Versioning**: Add version field to all formats for compatibility
7. **Deprecation Path**: Plan for removing rarely-used formats if needed

**Prevention Checklist:**
- [ ] Design exporter interface/abstraction
- [ ] Extract shared export logic
- [ ] Create tests for all formats
- [ ] Document format capabilities
- [ ] Prioritize formats (JSON > CSV > others)
- [ ] Plan versioning strategy

---

### Risk 7: Mask Image Coordinate Synchronization

**Problem:**
- Mask generation requires image coordinates to be up-to-date
- If coordinates aren't synchronized, mask will be wrong
- Mask must be regenerated if regions change
- Coordinate changes during mask generation (race condition?)

**Potential Issues:**
1. **Stale Coordinates**: Image coordinates not synchronized before mask generation
2. **Coordinate Changes**: User modifies regions while mask is being generated
3. **Missing Image**: No image loaded, can't generate mask
4. **Coordinate System Mismatch**: Using wrong coordinate system for mask
5. **Performance**: Generating mask for large images with many regions is slow

**Solutions:**
1. **Force Synchronization**: Call `synchronizeCoordinates()` before mask generation
2. **Lock State**: Prevent modifications during mask generation (or copy state)
3. **Validation**: Verify image exists and coordinates are valid before generating
4. **Progress Indicator**: Show progress for large mask generation
5. **Error Handling**: Clear error if image not loaded or coordinates invalid
6. **State Snapshot**: Create snapshot of state for mask generation (prevents race conditions)

**Prevention Checklist:**
- [ ] Force coordinate synchronization before mask generation
- [ ] Validate image exists
- [ ] Validate coordinates are valid
- [ ] Test with unsynchronized coordinates
- [ ] Test performance with large images
- [ ] Add progress indicator for long operations

---

### Risk 8: Import/Export UI Integration

**Problem:**
- Export/import must integrate seamlessly with existing UI
- File dialogs must be intuitive
- Error messages must be clear
- Progress indicators needed for large operations
- Must not block UI during file operations

**Potential Issues:**
1. **UI Blocking**: Large file operations block UI thread
2. **Unclear File Dialogs**: Users don't know which format to choose
3. **Error Message Placement**: Errors might be missed
4. **Progress Feedback**: No feedback during long operations
5. **Cancel Operation**: Can't cancel long-running export/import
6. **Multiple Dialogs**: Too many dialogs confuse users

**Solutions:**
1. **Async Operations**: Use QThread or QtConcurrent for large operations
2. **Progress Dialogs**: Show QProgressDialog for long operations
3. **Clear Labels**: Use descriptive file dialog labels and filters
4. **Status Bar**: Show operation status in status bar
5. **Error Dialogs**: Use QMessageBox for errors, ensure they're modal
6. **Cancel Support**: Allow canceling long operations
7. **Unified Export Dialog**: Single dialog with format selection

**Prevention Checklist:**
- [ ] Design export/import UI flow
- [ ] Add progress indicators
- [ ] Test with large files
- [ ] Test error handling UI
- [ ] Test cancel functionality
- [ ] Ensure UI doesn't block
- [ ] Create clear file dialog labels

---

## Medium Priority Risks

### Risk 9: CSV Escaping Edge Cases

**Severity:** Medium  
**Problem:** CSV escaping has many edge cases (quotes, commas, newlines, special characters)  
**Solution:** Use Qt's CSV handling if available, or implement robust escaping following RFC 4180  
**Testing:** Test with all special character combinations

---

### Risk 10: JSON Import Backward Compatibility

**Severity:** Medium  
**Problem:** Future JSON format changes might break old imports  
**Solution:** Add version field, support multiple versions, migrate old formats  
**Testing:** Test with old JSON files after format changes

---

### Risk 11: Mask Image Format Support

**Severity:** Medium  
**Problem:** Different image formats have different capabilities (transparency, color depth)  
**Solution:** Document format limitations, recommend PNG for masks  
**Testing:** Test all supported formats, verify OCR compatibility

---

### Risk 12: Large File Performance

**Severity:** Medium  
**Problem:** Exporting/importing large files (1000+ regions) might be slow  
**Solution:** Optimize serialization, use streaming for large files, add progress indicators  
**Testing:** Test with 1000+ regions, measure performance

---

## Low Priority Risks

### Risk 13: Export Format Selection UX

**Severity:** Low  
**Problem:** Users might not know which format to choose  
**Solution:** Add format descriptions in file dialog, default to JSON  
**Testing:** User testing for format selection clarity

---

### Risk 14: Coordinate Precision Display

**Severity:** Low  
**Problem:** CSV/TXT might show too many or too few decimal places  
**Solution:** Use consistent 6 decimal places, document precision  
**Testing:** Verify precision in all formats

---

## Prevention Strategy Summary

1. **Start with JSON Export**: Most critical, establishes pattern for others
2. **Comprehensive Testing**: Test round-trip, edge cases, error handling
3. **Coordinate System Discipline**: Always use normalized for export, image for mask
4. **Error Handling First**: Implement robust error handling before features
5. **Incremental Implementation**: One format at a time, test thoroughly
6. **Documentation**: Document all formats, limitations, and usage

---

## Success Criteria

- [ ] All export formats produce valid, readable files
- [ ] JSON import/export round-trip works perfectly
- [ ] CSV opens correctly in Excel, Google Sheets, LibreOffice
- [ ] Mask images match original dimensions exactly
- [ ] All error cases handled gracefully
- [ ] Performance acceptable for large files (1000+ regions)
- [ ] No data loss in round-trip operations
- [ ] Clear error messages for all failure cases

---

**Next Steps:**
1. Review proven methods document
2. Research Stack Overflow for specific implementation patterns
3. Create detailed implementation plan with risk mitigations
4. Begin implementation with highest-risk items first

