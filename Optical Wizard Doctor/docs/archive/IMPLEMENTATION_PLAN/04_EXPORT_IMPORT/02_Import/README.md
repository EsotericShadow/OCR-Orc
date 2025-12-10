# Import - Implementation Steps

## Objective

Enable users to import saved coordinate calibrations from JSON files.

## Prerequisites

- JSON export working
- DocumentState ready
- File dialogs working

## Sub-Projects

1. **JSON Import** - Import coordinates from JSON files

## Steps

### Step 1: Create JSON Import Function

1. Create `src/export/JsonImporter.h` and `.cpp`
2. Create method: `static DocumentState import(const QString& filePath)`
3. Or method: `static void import(DocumentState& state, const QString& filePath)`

### Step 2: Read and Parse JSON

1. Read file into QByteArray
2. Parse JSON using QJsonDocument
3. Validate JSON structure
4. Handle parse errors

### Step 3: Validate JSON Structure

1. Check required fields:
   - "regions" object (required)
   - "groups" object (optional)
2. Validate coordinate format (dict or list)
3. Validate coordinate ranges (0.0-1.0)
4. Show errors for invalid data

### Step 4: Load Regions

1. Iterate through regions object
2. For each region:
   - Extract normalized coordinates
   - Handle both dict and list formats
   - Extract color and group
   - Create RegionData
   - Calculate image coordinates if image loaded
   - Calculate canvas coordinates
   - Add to DocumentState

### Step 5: Load Groups

1. If groups section exists, load it
2. If groups section missing, reconstruct from region data
3. Validate group-region relationships
4. Add groups to DocumentState

### Step 6: Handle Coordinate Formats

1. Support dict format: `{"x1": 0.25, "y1": 0.10, ...}`
2. Support list format: `[0.25, 0.10, 0.35, 0.20]`
3. Convert to consistent format internally
4. Validate all coordinates are 0.0-1.0

### Step 7: Calculate Missing Coordinates

1. If image is loaded:
   - Calculate image coordinates from normalized
   - Calculate canvas coordinates from normalized
2. If image not loaded:
   - Store normalized only
   - Calculate others when image loads

### Step 8: Add Import UI

1. "Load Coordinates" button
2. Show file open dialog (JSON files only)
3. Call import function
4. Update UI (listbox, canvas)
5. Show success message with region count

### Step 9: Handle Import Errors

1. Invalid JSON format
2. Missing required fields
3. Invalid coordinate ranges
4. Duplicate region names
5. Show clear error messages
6. Preserve current state on error

### Step 10: Test Import

1. Export test data to JSON
2. Import JSON file
3. Verify regions are restored
4. Verify groups are restored
5. Verify coordinates are correct
6. Test with both coordinate formats
7. Test error handling

## Acceptance Criteria

- [ ] JSON files can be imported
- [ ] Regions are restored correctly
- [ ] Groups are restored correctly
- [ ] Coordinates are validated
- [ ] Both coordinate formats supported
- [ ] Errors are handled gracefully
- [ ] UI updates correctly
