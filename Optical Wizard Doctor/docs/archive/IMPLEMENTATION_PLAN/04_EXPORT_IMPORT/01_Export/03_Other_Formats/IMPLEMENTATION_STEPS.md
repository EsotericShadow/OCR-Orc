# Other Export Formats - Implementation Steps

## Objective

Implement TXT, XML, and YAML export formats.

## Prerequisites

- JSON export working (for structure reference)
- DocumentState ready
- Qt XML support (built-in)
- yaml-cpp library (for YAML, optional)

## Formats

1. **TXT** - Human-readable plain text
2. **XML** - Structured XML format
3. **YAML** - YAML format (requires library)

## Steps

### Step 1: Implement TXT Export

1. Create `src/export/TxtExporter.h` and `.cpp`
2. Create class `TxtExporter`
3. Format as human-readable text:
   ```
   COORDINATE CALIBRATION DATA
   ==================================================
   
   PDF Path: /path/to/file.pdf
   Image Size: 2000 x 3000 pixels
   Aspect Ratio: 0.6667
   Total Regions: 15
   
   REGIONS (Normalized Coordinates - Percentages):
   --------------------------------------------------
   
   Region: Last Name Cell 1
     Group: lastname
     Color: blue
     Coordinates (normalized 0.0-1.0):
       X1: 0.250000 (25.00%)
       Y1: 0.100000 (10.00%)
       X2: 0.350000 (35.00%)
       Y2: 0.200000 (20.00%)
   
   GROUPS:
   --------------------------------------------------
   lastname (3 regions):
     - Last Name Cell 1
     - Last Name Cell 2
   ```
4. Include both decimal and percentage formats
5. Sort regions and groups alphabetically

### Step 2: Implement XML Export

1. Create `src/export/XmlExporter.h` and `.cpp`
2. Use Qt's QXmlStreamWriter
3. Create XML structure:
   ```xml
   <?xml version="1.0" ?>
   <coordinate_calibration 
       pdf_path="..." 
       image_width="2000" 
       image_height="3000" 
       aspect_ratio="0.666700">
     <regions>
       <region name="..." group="..." color="...">
         <normalized_coords x1="..." y1="..." x2="..." y2="..."/>
       </region>
     </regions>
     <groups>
       <group name="...">
         <region>...</region>
       </group>
     </groups>
   </coordinate_calibration>
   ```
4. Use QXmlStreamWriter for proper XML formatting
5. Include XML declaration
6. Pretty-print with indentation

### Step 3: Implement YAML Export

1. Create `src/export/YamlExporter.h` and `.cpp`
2. Check if yaml-cpp is available (optional dependency)
3. Use yaml-cpp library for YAML generation
4. Create YAML structure:
   ```yaml
   pdf_path: /path/to/file.pdf
   image_size: [2000, 3000]
   aspect_ratio: 0.6667
   regions:
     Last Name Cell 1:
       normalized_coords:
         x1: 0.25
         y1: 0.10
         x2: 0.35
         y2: 0.20
       color: blue
       group: lastname
   groups:
     lastname:
       - Last Name Cell 1
       - Last Name Cell 2
   ```
5. Use block style (not flow style)
6. 2-space indentation

### Step 4: Handle Optional YAML

1. Make YAML export conditional:
   ```cpp
   #ifdef HAVE_YAML_CPP
       YamlExporter::export(state, filePath);
   #else
       showError("YAML export requires yaml-cpp library");
   #endif
   ```
2. Or check at runtime if library available
3. Show helpful error message if not available

### Step 5: Add to Export Dialog

1. Update file dialog to include all formats:
   ```cpp
   "JSON files (*.json);;"
   "CSV files (*.csv);;"
   "TXT files (*.txt);;"
   "XML files (*.xml);;"
   "YAML files (*.yaml *.yml);;"
   "All files (*.*)"
   ```
2. Detect format from extension
3. Call appropriate exporter

### Step 6: Test All Formats

1. Test TXT export (human-readable)
2. Test XML export (valid XML)
3. Test YAML export (if available)
4. Verify all formats contain same data
5. Verify coordinates are normalized in all formats
6. Test file opening in appropriate applications

## Acceptance Criteria

- [ ] TXT export works and is human-readable
- [ ] XML export works and is valid XML
- [ ] YAML export works (if library available)
- [ ] All formats contain same data
- [ ] All formats use normalized coordinates only
- [ ] Files are properly formatted

## Code Structure

```cpp
class TxtExporter {
public:
    static void export(const DocumentState& state, const QString& filePath);
};

class XmlExporter {
public:
    static void export(const DocumentState& state, const QString& filePath);
};

class YamlExporter {
public:
    static void export(const DocumentState& state, const QString& filePath);
    static bool isAvailable();  // Check if library available
};
```

## Notes

- TXT is simple string formatting
- XML uses Qt's QXmlStreamWriter
- YAML requires external library (optional)
- All formats must use normalized coordinates only

