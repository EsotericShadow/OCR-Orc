# JSON Export - Implementation Steps

## Objective

Export coordinates in JSON format with complete structure and metadata.

## Prerequisites

- DocumentState with regions and groups
- Qt JSON support (QJsonDocument)
- File I/O working

## JSON Structure

```json
{
  "pdf_path": "string",
  "image_size": [width, height],
  "aspect_ratio": 0.6667,
  "regions": {
    "Region Name": {
      "normalized_coords": {"x1": 0.25, "y1": 0.10, "x2": 0.35, "y2": 0.20},
      "color": "blue",
      "group": "lastname"
    }
  },
  "groups": {
    "lastname": ["Region Name 1", "Region Name 2"]
  }
}
```

## Steps

### Step 1: Create JsonExporter Class

1. Create `src/export/JsonExporter.h` and `.cpp`
2. Create class `JsonExporter`
3. Add static method: `static void export(const DocumentState& state, const QString& filePath)`
4. Or standalone function (simpler approach)

### Step 2: Build JSON Structure

1. Create QJsonObject root
2. Add top-level fields:
   ```cpp
   QJsonObject root;
   root["pdf_path"] = state.pdfPath;
   root["image_size"] = QJsonArray{width, height};
   root["aspect_ratio"] = width / height;
   ```
3. Create regions object:
   ```cpp
   QJsonObject regionsObj;
   for (const auto& [name, region] : state.regions) {
       QJsonObject regionObj;
       // Add region data
       regionsObj[name] = regionObj;
   }
   root["regions"] = regionsObj;
   ```
4. Create groups object:
   ```cpp
   QJsonObject groupsObj;
   for (const auto& [name, group] : state.groups) {
       QJsonArray regionArray;
       for (const QString& regionName : group.regionNames) {
           regionArray.append(regionName);
       }
       groupsObj[name] = regionArray;
   }
   root["groups"] = groupsObj;
   ```

### Step 3: Add Region Data to JSON

1. For each region:
   ```cpp
   QJsonObject regionObj;
   
   // Normalized coordinates (dict format)
   QJsonObject coordsObj;
   coordsObj["x1"] = region.normalizedCoords.x1;
   coordsObj["y1"] = region.normalizedCoords.y1;
   coordsObj["x2"] = region.normalizedCoords.x2;
   coordsObj["y2"] = region.normalizedCoords.y2;
   regionObj["normalized_coords"] = coordsObj;
   
   // Color and group
   regionObj["color"] = region.color;
   regionObj["group"] = region.group.isEmpty() ? QJsonValue() : region.group;
   ```
2. Use normalized coordinates only (no image/canvas)

### Step 4: Format JSON Output

1. Create QJsonDocument from root object
2. Convert to JSON string:
   ```cpp
   QJsonDocument doc(root);
   QByteArray json = doc.toJson(QJsonDocument::Indented);
   ```
3. Use indented format (2 spaces) for readability

### Step 5: Write to File

1. Open file for writing
2. Write JSON string
3. Handle file errors
4. Close file

### Step 6: Add Export Function to MainWindow

1. Create method `void exportCoordinates()`
2. Check if regions exist (show warning if none)
3. Show file save dialog:
   ```cpp
   QString filePath = QFileDialog::getSaveFileName(
       this, "Export Coordinates", "",
       "JSON files (*.json);;All files (*.*)");
   ```
4. If file selected:
   - Determine format from extension
   - Call JsonExporter::export()
   - Show success message

### Step 7: Handle Export Errors

1. Check file write permissions
2. Handle disk full errors
3. Handle invalid file path
4. Show error messages
5. Preserve data (don't lose work)

### Step 8: Test JSON Export

1. Create test regions
2. Export to JSON
3. Verify file created
4. Verify JSON structure is correct
5. Verify coordinates are normalized
6. Verify all metadata included
7. Test with empty groups
8. Test with no groups

## Acceptance Criteria

- [ ] JSON file is created correctly
- [ ] JSON structure matches specification
- [ ] Only normalized coordinates are exported
- [ ] All metadata is included
- [ ] File is readable and valid JSON
- [ ] Errors are handled gracefully

## Code Structure

```cpp
class JsonExporter {
public:
    static void export(const DocumentState& state, const QString& filePath);
    
private:
    static QJsonObject buildRegionsObject(const DocumentState& state);
    static QJsonObject buildGroupsObject(const DocumentState& state);
    static QJsonObject regionToJson(const RegionData& region);
};
```

## Notes

- Use Qt's QJsonDocument for JSON handling
- Pretty-print with indentation for readability
- Validate JSON before writing
- Test with various region counts

