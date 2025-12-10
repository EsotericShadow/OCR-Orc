# JSON Import - Implementation Steps

## Objective

Import coordinates from JSON files and restore application state.

## Prerequisites

- JSON export working (for format reference)
- DocumentState ready
- Qt JSON support

## Steps

### Step 1: Create JsonImporter Class

1. Create `src/export/JsonImporter.h` and `.cpp`
2. Create class `JsonImporter`
3. Add method: `static void import(DocumentState& state, const QString& filePath)`
4. Or return DocumentState (if creating new state)

### Step 2: Read JSON File

1. Open file for reading
2. Read all content:
   ```cpp
   QFile file(filePath);
   if (!file.open(QIODevice::ReadOnly)) {
       // Handle error
   }
   QByteArray data = file.readAll();
   ```
3. Handle file errors

### Step 3: Parse JSON

1. Create QJsonDocument from data:
   ```cpp
   QJsonParseError error;
   QJsonDocument doc = QJsonDocument::fromJson(data, &error);
   if (error.error != QJsonParseError::NoError) {
       // Handle parse error
   }
   ```
2. Get root object: `QJsonObject root = doc.object()`
3. Validate JSON structure

### Step 4: Validate JSON Structure

1. Check required fields:
   ```cpp
   if (!root.contains("regions")) {
       // Error: missing regions
   }
   ```
2. Validate regions is object
3. Validate groups is object (if present)
4. Show clear error messages

### Step 5: Load Regions

1. Get regions object: `QJsonObject regionsObj = root["regions"].toObject()`
2. Iterate through regions:
   ```cpp
   for (auto it = regionsObj.begin(); it != regionsObj.end(); ++it) {
       QString name = it.key();
       QJsonObject regionObj = it.value().toObject();
       // Parse region data
   }
   ```
3. For each region:
   - Extract normalized coordinates (handle dict/list format)
   - Extract color (default to "blue" if missing)
   - Extract group (can be null/empty)
   - Create RegionData
   - Calculate image/canvas coordinates if image loaded
   - Add to state

### Step 6: Handle Coordinate Formats

1. Check if coordinates are dict or array:
   ```cpp
   QJsonValue coordsValue = regionObj["normalized_coords"];
   if (coordsValue.isObject()) {
       // Dict format: {"x1": 0.25, ...}
       QJsonObject coordsObj = coordsValue.toObject();
       x1 = coordsObj["x1"].toDouble();
       // ...
   } else if (coordsValue.isArray()) {
       // List format: [0.25, 0.10, 0.35, 0.20]
       QJsonArray coordsArray = coordsValue.toArray();
       x1 = coordsArray[0].toDouble();
       // ...
   }
   ```
2. Validate coordinates are 0.0-1.0
3. Convert to NormalizedCoords structure

### Step 7: Calculate Missing Coordinates

1. If image is loaded in state:
   - Calculate image coordinates: `image = normalized * imageDimensions`
   - Calculate canvas coordinates: `canvas = normalizedToCanvas(...)`
2. If image not loaded:
   - Store normalized only
   - Image/canvas coords will be calculated when image loads

### Step 8: Load Groups

1. Check if groups section exists:
   ```cpp
   if (root.contains("groups")) {
       QJsonObject groupsObj = root["groups"].toObject();
       // Load groups
   } else {
       // Reconstruct from region data
   }
   ```
2. If groups exist:
   - Load each group
   - Validate region names exist
   - Add to state
3. If groups missing:
   - Reconstruct from region group fields
   - Create groups based on region.group values

### Step 9: Reconstruct Groups from Region Data

1. Iterate through loaded regions
2. For each region with a group:
   - If group doesn't exist, create it
   - Add region to group
3. This handles backward compatibility

### Step 10: Validate Imported Data

1. Check for duplicate region names
2. Validate all coordinates are 0.0-1.0
3. Validate group-region relationships
4. Check aspect ratio matches (if image loaded)
5. Show warnings for issues

### Step 11: Update Application State

1. Clear existing regions (or merge, based on preference)
2. Set imported regions
3. Set imported groups
4. Update PDF path if provided
5. Recalculate canvas coordinates
6. Update UI (listbox, canvas, info)

### Step 12: Add Import UI

1. "Load Coordinates" button in MainWindow
2. Show file open dialog:
   ```cpp
   QString filePath = QFileDialog::getOpenFileName(
       this, "Load Coordinates", "",
       "JSON files (*.json);;All files (*.*)");
   ```
3. If file selected:
   - Call JsonImporter::import()
   - Update UI
   - Show success message: "Loaded N regions"

### Step 13: Handle Import Errors

1. **File not found**: Show error, maintain current state
2. **Invalid JSON**: Show parse error, maintain state
3. **Missing fields**: Show error, maintain state
4. **Invalid coordinates**: Show warning, skip invalid regions
5. **Duplicate names**: Show warning, rename or skip
6. **Aspect ratio mismatch**: Show warning, allow import anyway

### Step 14: Test Import

1. Export test data
2. Import exported file
3. Verify round-trip (export → import → export, compare)
4. Test with dict coordinate format
5. Test with list coordinate format
6. Test with missing groups (reconstruction)
7. Test with invalid data (error handling)
8. Test with no image loaded
9. Test with image loaded

## Acceptance Criteria

- [ ] JSON files can be imported
- [ ] Both coordinate formats supported
- [ ] Regions are restored correctly
- [ ] Groups are restored correctly
- [ ] Groups reconstructed if missing
- [ ] Coordinates are validated
- [ ] Errors are handled gracefully
- [ ] UI updates correctly
- [ ] Round-trip works (export → import)

## Code Structure

```cpp
class JsonImporter {
public:
    static void import(DocumentState& state, const QString& filePath);
    
private:
    static NormalizedCoords parseCoordinates(const QJsonValue& value);
    static void loadRegions(DocumentState& state, const QJsonObject& regionsObj);
    static void loadGroups(DocumentState& state, const QJsonObject& groupsObj);
    static void reconstructGroups(DocumentState& state);
    static bool validateCoordinates(const NormalizedCoords& coords);
};
```

## Notes

- Support both coordinate formats for backward compatibility
- Reconstruct groups if missing
- Validate all imported data
- Handle errors gracefully
- Test round-trip thoroughly

