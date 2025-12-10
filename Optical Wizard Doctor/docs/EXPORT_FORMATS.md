# Export Formats Specification

## Overview

OCR-Orc exports coordinate data in multiple formats to support different use cases and integration requirements. All exports use **normalized coordinates (0.0-1.0)** to ensure resolution independence.

## Export Format Selection

The export format is determined by the file extension:

- `.json` → JSON format
- `.csv` → CSV format
- `.txt` → TXT format
- `.xml` → XML format
- `.yaml` or `.yml` → YAML format
- Default (no extension or unknown) → JSON format

## 1. JSON Format

### File Extension
`.json`

### Structure

```json
{
  "pdf_path": "/path/to/calibration/document.pdf",
  "image_size": [2000, 3000],
  "aspect_ratio": 0.6667,
  "regions": {
    "Region Name": {
      "normalized_coords": {
        "x1": 0.25,
        "y1": 0.10,
        "x2": 0.35,
        "y2": 0.20
      },
      "color": "blue",
      "group": "lastname"
    }
  },
  "groups": {
    "lastname": ["Region Name 1", "Region Name 2"],
    "firstname": ["Region Name 3", "Region Name 4"]
  }
}
```

### Field Specifications

#### Top-Level Fields

- **pdf_path** (string or null): Path to the PDF used for calibration
- **image_size** (array of 2 integers or null): [width, height] in pixels
- **aspect_ratio** (float or null): width / height ratio
- **regions** (object): Dictionary of region data
- **groups** (object): Dictionary of group definitions

#### Region Object

- **normalized_coords** (object): Coordinate object with x1, y1, x2, y2
- **color** (string): Color identifier ('blue', 'red', etc.)
- **group** (string or null): Group name or null

#### Normalized Coordinates Object

- **x1** (float): Left edge (0.0-1.0)
- **y1** (float): Top edge (0.0-1.0)
- **x2** (float): Right edge (0.0-1.0)
- **y2** (float): Bottom edge (0.0-1.0)

### Usage

- **Primary format** for programmatic access
- Easy to parse in any language
- Supports nested structures
- Human-readable with indentation

## 2. CSV Format

### File Extension
`.csv`

### Structure

```csv
Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)
Last Name Cell 1,lastname,blue,0.250000,0.100000,0.350000,0.200000
Last Name Cell 2,lastname,blue,0.350000,0.100000,0.450000,0.200000
First Name Cell 1,firstname,red,0.250000,0.200000,0.350000,0.300000
```

### Column Specifications

1. **Region Name** (string): Unique region identifier
2. **Group** (string): Group name or empty string
3. **Color** (string): Color identifier
4. **X1 (%)** (float): Left edge as percentage (0.0-1.0)
5. **Y1 (%)** (float): Top edge as percentage (0.0-1.0)
6. **X2 (%)** (float): Right edge as percentage (0.0-1.0)
7. **Y2 (%)** (float): Bottom edge as percentage (0.0-1.0)

### Formatting

- **Header row:** Always present
- **Coordinate precision:** 6 decimal places
- **Sorting:** Regions sorted alphabetically by name
- **Empty groups:** Shown as empty string (not "None")

### Usage

- **Spreadsheet import** (Excel, Google Sheets)
- **Database import**
- **Simple data analysis**
- **Tabular data processing**

## 3. TXT Format

### File Extension
`.txt`

### Structure

```
COORDINATE CALIBRATION DATA
==================================================

PDF Path: /path/to/calibration/document.pdf
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

Region: Last Name Cell 2
  Group: lastname
  Color: blue
  Coordinates (normalized 0.0-1.0):
    X1: 0.350000 (35.00%)
    Y1: 0.100000 (10.00%)
    X2: 0.450000 (45.00%)
    Y2: 0.200000 (20.00%)

...

GROUPS:
--------------------------------------------------

lastname (3 regions):
  - Last Name Cell 1
  - Last Name Cell 2
  - Last Name Cell 3

firstname (2 regions):
  - First Name Cell 1
  - First Name Cell 2
```

### Formatting

- **Human-readable** format
- **Section headers** with separators
- **Coordinate display:** Both decimal (0.0-1.0) and percentage (0-100%)
- **Group listings:** Show region count and member list
- **Sorted:** Regions and groups sorted alphabetically

### Usage

- **Documentation**
- **Human review**
- **Printing**
- **Quick reference**

## 4. XML Format

### File Extension
`.xml`

### Structure

```xml
<?xml version="1.0" ?>
<coordinate_calibration 
    pdf_path="/path/to/calibration/document.pdf" 
    image_width="2000" 
    image_height="3000" 
    aspect_ratio="0.666700">
  <regions>
    <region name="Last Name Cell 1" group="lastname" color="blue">
      <normalized_coords 
          x1="0.250000" 
          y1="0.100000" 
          x2="0.350000" 
          y2="0.200000"/>
    </region>
    <region name="Last Name Cell 2" group="lastname" color="blue">
      <normalized_coords 
          x1="0.350000" 
          y1="0.100000" 
          x2="0.450000" 
          y2="0.200000"/>
    </region>
  </regions>
  <groups>
    <group name="lastname">
      <region>Last Name Cell 1</region>
      <region>Last Name Cell 2</region>
      <region>Last Name Cell 3</region>
    </group>
    <group name="firstname">
      <region>First Name Cell 1</region>
      <region>First Name Cell 2</region>
    </group>
  </groups>
</coordinate_calibration>
```

### Element Specifications

#### Root Element: `<coordinate_calibration>`

Attributes:
- **pdf_path** (string): PDF file path
- **image_width** (integer): Image width in pixels
- **image_height** (integer): Image height in pixels
- **aspect_ratio** (float): Width/height ratio

#### `<regions>` Element

Container for all region elements.

#### `<region>` Element

Attributes:
- **name** (string): Region name
- **group** (string): Group name or empty string
- **color** (string): Color identifier

Child:
- **<normalized_coords>** element

#### `<normalized_coords>` Element

Attributes:
- **x1** (float): Left edge
- **y1** (float): Top edge
- **x2** (float): Right edge
- **y2** (float): Bottom edge

#### `<groups>` Element

Container for all group elements.

#### `<group>` Element

Attributes:
- **name** (string): Group name

Children:
- **<region>** elements (text content is region name)

### Formatting

- **Pretty-printed** with indentation
- **XML declaration** included
- **Sorted:** Regions and groups sorted alphabetically
- **Precision:** 6 decimal places for coordinates

### Usage

- **XML-based systems**
- **Configuration files**
- **Data exchange standards**
- **XSLT processing**

## 5. YAML Format

### File Extension
`.yaml` or `.yml`

### Structure

```yaml
pdf_path: /path/to/calibration/document.pdf
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
  Last Name Cell 2:
    normalized_coords:
      x1: 0.35
      y1: 0.10
      x2: 0.45
      y2: 0.20
    color: blue
    group: lastname
groups:
  lastname:
    - Last Name Cell 1
    - Last Name Cell 2
    - Last Name Cell 3
  firstname:
    - First Name Cell 1
    - First Name Cell 2
```

### Formatting

- **Block style** (not flow style)
- **Indentation:** 2 spaces
- **No sorting** (preserves insertion order)
- **Precision:** Natural float representation

### Usage

- **Configuration files**
- **Human-readable data**
- **YAML-based systems**
- **Documentation**

## 6. Mask Image Export

### File Formats
- PNG (`.png`) - Recommended
- JPEG (`.jpg`)
- TIFF (`.tiff`)

### Image Specifications

- **Color Mode:** RGB
- **Dimensions:** Same as original image (exact match required)
- **Background:** Black (RGB: 0, 0, 0)
- **Regions:** White rectangles (RGB: 255, 255, 255)
- **Purpose:** OCR preprocessing mask

### Mask Structure

```
┌─────────────────────────────────┐
│  Black Background               │
│  ┌───┐ ┌───┐ ┌───┐             │
│  │ W │ │ W │ │ W │  White       │
│  └───┘ └───┘ └───┘  Rectangles  │
│                                 │
│  ┌───┐ ┌───┐                    │
│  │ W │ │ W │                    │
│  └───┘ └───┘                    │
└─────────────────────────────────┘
```

### Coordinate JSON File

When exporting a mask, a coordinate JSON file is automatically saved alongside the mask with the same base name:

- Mask: `calibration_mask.png`
- Coordinates: `calibration_mask.json`

This ensures the mask and coordinates stay together.

### Usage

1. **Load mask image** in OCR pipeline
2. **Overlay mask** on original document image
3. **Black areas** hide unnecessary information
4. **White rectangles** show only labeled cells
5. **Run OCR** on masked image for improved accuracy

## 7. Export Requirements

### Common Requirements (All Formats)

1. **Normalized Coordinates Only**
   - All exports use 0.0-1.0 coordinates
   - No pixel coordinates in exports
   - Resolution-independent

2. **Aspect Ratio Information**
   - Include aspect ratio for validation
   - Include image dimensions if available
   - Helps verify coordinate compatibility

3. **Region Metadata**
   - Region name (required)
   - Color identifier
   - Group assignment

4. **Group Definitions**
   - Complete group structure
   - List of regions per group
   - Preserves organization

5. **Sorting**
   - Regions sorted alphabetically (where applicable)
   - Groups sorted alphabetically (where applicable)
   - Consistent ordering for reproducibility

### Precision Requirements

- **Normalized Coordinates:** 6 decimal places minimum
- **Aspect Ratio:** 4-6 decimal places
- **Percentages:** 2 decimal places (for display)

## 8. Import Compatibility

### Supported Import Format

**Primary:** JSON only

### Import Requirements

1. **Normalized Coordinates**
   - Must be in range 0.0-1.0
   - Supports both dict and list formats
   - Validates coordinate ranges

2. **Group Reconstruction**
   - If groups section missing, reconstruct from region data
   - Maintains group-region relationships

3. **Coordinate Calculation**
   - Calculates image coordinates if image is loaded
   - Calculates canvas coordinates for display
   - Maintains all three coordinate systems

4. **Error Handling**
   - Validates file format
   - Handles missing optional fields
   - Shows error messages for invalid data

## 9. Export Workflow

### Standard Export Process

1. User clicks "Export Coordinates" or presses Cmd+S
2. File dialog appears with format options
3. User selects:
   - File location
   - File name
   - File format (via extension)
4. Application:
   - Validates regions exist
   - Determines format from extension
   - Generates export data
   - Writes file
   - Shows success message

### Mask Export Process

1. User clicks "Export Mask" or presses Cmd+M
2. File dialog appears (image formats)
3. User selects:
   - File location
   - File name
   - Image format (PNG, JPEG, TIFF)
4. Application:
   - Validates image and regions exist
   - Creates black mask image
   - Draws white rectangles for regions
   - Saves mask image
   - Saves coordinate JSON file
   - Shows success message with both file paths

## 10. Format Selection Guidelines

### Use JSON When:
- Building automated pipelines
- Programmatic access required
- Need nested data structures
- Integration with APIs

### Use CSV When:
- Importing into spreadsheets
- Database operations
- Simple tabular analysis
- Excel/Google Sheets compatibility

### Use TXT When:
- Human review needed
- Documentation purposes
- Printing reference
- Quick visual inspection

### Use XML When:
- XML-based systems
- Configuration standards
- XSLT processing
- Enterprise integration

### Use YAML When:
- Configuration files
- Human-readable data
- YAML-based tools
- Documentation

### Use Mask When:
- OCR preprocessing
- Visual validation
- Testing extraction accuracy
- Quality assurance

