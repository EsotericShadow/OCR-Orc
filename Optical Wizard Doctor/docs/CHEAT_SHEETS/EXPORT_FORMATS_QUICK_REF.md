# Export Formats Quick Reference

## Format Selection

| Extension | Format | Exporter Class |
|-----------|--------|----------------|
| `.json` | JSON | JsonExporter |
| `.csv` | CSV | CsvExporter |
| `.txt` | TXT | TxtExporter |
| `.xml` | XML | XmlExporter |
| `.yaml` / `.yml` | YAML | YamlExporter |

---

## JSON Structure

```json
{
  "pdf_path": "string",
  "image_size": [width, height],
  "aspect_ratio": 0.6667,
  "regions": {
    "Region Name": {
      "normalized_coords": {
        "x1": 0.25, "y1": 0.10,
        "x2": 0.35, "y2": 0.20
      },
      "color": "blue",
      "group": "lastname"
    }
  },
  "groups": {
    "lastname": ["Region Name 1", "Region Name 2"]
  }
}
```

---

## CSV Structure

```csv
Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)
Last Name Cell 1,lastname,blue,0.250000,0.100000,0.350000,0.200000
```

**Columns**:
1. Region Name
2. Group
3. Color
4. X1 (%)
5. Y1 (%)
6. X2 (%)
7. Y2 (%)

**Precision**: 6 decimal places

---

## TXT Structure

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
  - Last Name Cell 3
```

---

## XML Structure

```xml
<?xml version="1.0" ?>
<coordinate_calibration 
    pdf_path="/path/to/file.pdf"
    image_width="2000"
    image_height="3000"
    aspect_ratio="0.666700">
  <regions>
    <region name="Last Name Cell 1" 
            group="lastname" 
            color="blue">
      <normalized_coords 
          x1="0.250000" y1="0.100000"
          x2="0.350000" y2="0.200000"/>
    </region>
  </regions>
  <groups>
    <group name="lastname">
      <region>Last Name Cell 1</region>
      <region>Last Name Cell 2</region>
    </group>
  </groups>
</coordinate_calibration>
```

---

## YAML Structure

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

---

## Mask Image

### Format
- **Type**: RGB image
- **Dimensions**: Same as original image
- **Background**: Black (0, 0, 0)
- **Regions**: White rectangles (255, 255, 255)

### File Formats
- PNG (recommended)
- JPEG
- TIFF

### Coordinate File
- Automatically saved alongside mask
- Same base name, `.json` extension
- Example: `mask.png` + `mask.json`

---

## Export Requirements

### All Formats Must Include
- Normalized coordinates only (0.0-1.0)
- Region metadata (name, color, group)
- Group definitions
- Aspect ratio information
- Image dimensions (if available)

### Coordinate Precision
- **Normalized**: 6 decimal places minimum
- **Aspect Ratio**: 4-6 decimal places
- **Percentages**: 2 decimal places (for display)

---

## Import Requirements

### Supported Format
- **JSON only** (for now)

### Coordinate Formats Supported
- Dict format: `{"x1": 0.25, "y1": 0.10, ...}`
- List format: `[0.25, 0.10, 0.35, 0.20]`

### Validation
- Coordinates must be 0.0-1.0
- Region names must be unique
- Groups reconstructed from region data if missing

---

*Quick reference for export/import formats*

