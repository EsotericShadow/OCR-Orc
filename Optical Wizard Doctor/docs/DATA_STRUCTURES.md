# Data Structures Specification

## 1. Region Data Structure

### Structure Definition

Each region is stored as a dictionary with the following structure:

```python
{
    'image_coords': [x1, y1, x2, y2],      # List of 4 integers
    'normalized_coords': [x1, y1, x2, y2], # List of 4 floats (0.0-1.0)
    'canvas_coords': [x1, y1, x2, y2],    # List of 4 floats
    'color': 'blue',                        # String color identifier
    'group': 'lastname'                     # String group name or None
}
```

### Field Specifications

#### image_coords
- **Type:** List of 4 integers
- **Format:** [x1, y1, x2, y2]
- **Range:** 0 to image_width/height (pixels)
- **Purpose:** Exact pixel positions in original PDF image
- **Calculation:** `image_coords = normalized_coords * image_dimensions`
- **Usage:** Mask generation, OCR extraction

#### normalized_coords
- **Type:** List of 4 floats
- **Format:** [x1, y1, x2, y2]
- **Range:** 0.0 to 1.0 (percentages)
- **Purpose:** Resolution-independent coordinate storage
- **Calculation:** `normalized_coords = image_coords / image_dimensions`
- **Usage:** Export format, source of truth

#### canvas_coords
- **Type:** List of 4 floats
- **Format:** [x1, y1, x2, y2]
- **Range:** 0 to canvas_width/height (display pixels)
- **Purpose:** Screen positions for rendering
- **Calculation:** `canvas_coords = (image_coords * scale_factor) + image_offset`
- **Usage:** Display, mouse interaction

#### color
- **Type:** String
- **Values:** 'blue', 'red', 'green', 'yellow', 'purple', 'orange', 'cyan'
- **Default:** 'blue'
- **Purpose:** Visual identifier for region
- **Usage:** Display border color, region identification

#### group
- **Type:** String or None
- **Values:** Group name string or None
- **Default:** None
- **Purpose:** Logical grouping of regions
- **Usage:** Organization, batch operations

### Region Storage

Regions are stored in a dictionary keyed by region name:

```python
regions = {
    'Last Name Cell 1': {
        'image_coords': [500, 300, 700, 600],
        'normalized_coords': [0.25, 0.10, 0.35, 0.20],
        'canvas_coords': [300, 170, 380, 290],
        'color': 'blue',
        'group': 'lastname'
    },
    'Last Name Cell 2': {
        ...
    }
}
```

**Requirements:**
- Region names must be unique (dictionary keys)
- All three coordinate systems must be maintained
- Coordinates must be kept in sync

## 2. Groups Data Structure

### Structure Definition

Groups are stored as a dictionary mapping group names to lists of region names:

```python
groups = {
    'lastname': ['Last Name Cell 1', 'Last Name Cell 2', 'Last Name Cell 3'],
    'firstname': ['First Name Cell 1', 'First Name Cell 2'],
    'studentnumber': ['Student Number Cell 1', 'Student Number Cell 2', ...]
}
```

### Field Specifications

#### Group Name (Key)
- **Type:** String
- **Constraints:** Case-sensitive, must be unique
- **Purpose:** Identifier for the group
- **Example:** 'lastname', 'firstname', 'studentnumber'

#### Region Names List (Value)
- **Type:** List of strings
- **Constraints:** Each region name must exist in regions dictionary
- **Purpose:** List of regions belonging to this group
- **Order:** Preserved (order matters for some operations)

### Group-Region Relationship

**Rules:**
- A region can belong to at most one group (or None)
- A region's group is stored in `region['group']`
- The groups dictionary maintains the reverse mapping
- Deleting a region removes it from its group
- Deleting all regions in a group removes the group

**Synchronization:**
- When region is added to group: Update both `region['group']` and `groups[group_name]`
- When region is removed from group: Update both `region['group']` and `groups[group_name]`
- When group is deleted: Set `region['group'] = None` for all members

## 3. Application State Structure

### Document State

```python
{
    'pdf_path': '/path/to/document.pdf',  # String or None
    'image': PIL.Image object,             # Image object or None
    'scale_factor': 1.0,                  # Float (display scale)
    'zoom_level': 1.0,                     # Float (1.0 = 100%)
    'image_x': 0,                          # Integer (image position on canvas)
    'image_y': 0,                          # Integer (image position on canvas)
    'canvas_offset_x': 0,                  # Integer (pan offset)
    'canvas_offset_y': 0                   # Integer (pan offset)
}
```

### Selection State

```python
{
    'selected_region': 'Last Name Cell 1',  # String or None (primary selection)
    'selected_regions': {'Last Name Cell 1', 'Last Name Cell 2'},  # Set of strings
    'hovered_region': 'Last Name Cell 1'    # String or None
}
```

**Selection Rules:**
- `selected_region` is always in `selected_regions` (if not None)
- `selected_regions` can contain multiple regions
- Only `selected_region` shows resize handles
- All `selected_regions` move together

### Operation State

```python
{
    'mouse_mode': 'create',                # String: 'create' or 'select'
    'dragging': False,                     # Boolean
    'resizing': False,                     # Boolean
    'resize_handle': None,                 # String or None: 'nw', 'ne', 'sw', 'se', 'n', 's', 'w', 'e'
    'selection_box_start': None,           # Tuple (x, y) or None
    'start_x': None,                       # Float or None
    'start_y': None,                       # Float or None
    'current_region': None                 # Canvas item ID or None
}
```

### Undo/Redo State

```python
{
    'undo_stack': [                        # List of state snapshots
        {
            'regions': {...},              # Deep copy of regions
            'groups': {...}                # Deep copy of groups
        },
        ...
    ],
    'redo_stack': [                        # List of undone states
        ...
    ],
    'max_undo_history': 50                  # Integer (maximum stack size)
}
```

**State Snapshot Structure:**
```python
{
    'regions': {
        'Region Name': {
            'image_coords': [...],
            'normalized_coords': [...],
            'canvas_coords': [...],
            'color': 'blue',
            'group': 'lastname'
        },
        ...
    },
    'groups': {
        'lastname': ['Region Name', ...],
        ...
    }
}
```

## 4. Export Data Structure

### JSON Export Format

```json
{
    "pdf_path": "/path/to/document.pdf",
    "image_size": [2000, 3000],
    "aspect_ratio": 0.6667,
    "regions": {
        "Last Name Cell 1": {
            "normalized_coords": {
                "x1": 0.25,
                "y1": 0.10,
                "x2": 0.35,
                "y2": 0.20
            },
            "color": "blue",
            "group": "lastname"
        },
        ...
    },
    "groups": {
        "lastname": ["Last Name Cell 1", "Last Name Cell 2", ...],
        ...
    }
}
```

### CSV Export Format

```csv
Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)
Last Name Cell 1,lastname,blue,0.250000,0.100000,0.350000,0.200000
Last Name Cell 2,lastname,blue,0.350000,0.100000,0.450000,0.200000
...
```

### TXT Export Format

```
COORDINATE CALIBRATION DATA
==================================================

PDF Path: /path/to/document.pdf
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

...

GROUPS:
--------------------------------------------------

lastname (3 regions):
  - Last Name Cell 1
  - Last Name Cell 2
  - Last Name Cell 3
...
```

### XML Export Format

```xml
<?xml version="1.0" ?>
<coordinate_calibration pdf_path="/path/to/document.pdf" 
                        image_width="2000" 
                        image_height="3000" 
                        aspect_ratio="0.666700">
  <regions>
    <region name="Last Name Cell 1" group="lastname" color="blue">
      <normalized_coords x1="0.250000" y1="0.100000" x2="0.350000" y2="0.200000"/>
    </region>
    ...
  </regions>
  <groups>
    <group name="lastname">
      <region>Last Name Cell 1</region>
      <region>Last Name Cell 2</region>
      ...
    </group>
    ...
  </groups>
</coordinate_calibration>
```

### YAML Export Format

```yaml
pdf_path: /path/to/document.pdf
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
  ...
groups:
  lastname:
    - Last Name Cell 1
    - Last Name Cell 2
    ...
```

## 5. Color Mapping

### Color Identifiers

```python
color_map = {
    'blue': '#0066ff',
    'red': '#ff0000',
    'green': '#00cc00',
    'yellow': '#ffcc00',
    'purple': '#9900cc',
    'orange': '#ff6600',
    'cyan': '#00cccc'
}
```

**Usage:**
- Color identifiers are stored as strings ('blue', 'red', etc.)
- Hex values are used for rendering
- Default color: 'blue'

## 6. Resize Handle Positions

### Handle Identifiers

```python
handles = {
    'nw': 'top_left_corner',      # Top-left corner
    'ne': 'top_right_corner',     # Top-right corner
    'sw': 'bottom_left_corner',   # Bottom-left corner
    'se': 'bottom_right_corner',  # Bottom-right corner
    'n': 'top_side',              # Top edge
    's': 'bottom_side',            # Bottom edge
    'w': 'left_side',              # Left edge
    'e': 'right_side'              # Right edge
}
```

**Handle Positions:**
- Corner handles: At region corners
- Edge handles: At midpoints of edges
- Handle size: 8x8 pixels
- Tolerance: 13 pixels (8 + 5) for hit detection

## 7. Visual State Indicators

### Region Visual States

```python
visual_states = {
    'normal': {
        'border_width': 2,
        'fill_opacity': 0.0,
        'label_size': 8,
        'label_weight': 'normal'
    },
    'hovered': {
        'border_width': 3,
        'fill_opacity': 0.1,
        'label_size': 10,
        'label_weight': 'normal'
    },
    'selected': {
        'border_width': 4,
        'fill_opacity': 0.2,
        'label_size': 10,
        'label_weight': 'bold',
        'resize_handles': True
    }
}
```

## 8. Mask Image Structure

### Mask Image Format

- **Format:** RGB image
- **Dimensions:** Same as original image
- **Background:** Black (RGB: 0, 0, 0)
- **Regions:** White rectangles (RGB: 255, 255, 255)
- **Purpose:** OCR preprocessing (hide everything except regions)

### Mask Generation Process

1. Create black image (same size as original)
2. For each region:
   - Get image coordinates
   - Draw white rectangle at those coordinates
3. Save mask image (PNG, JPEG, or TIFF)
4. Save coordinate JSON file alongside mask

## 9. Import Format Compatibility

### Supported Import Formats

**Primary Format:** JSON

**Coordinate Format Support:**
- Dict format: `{'x1': 0.25, 'y1': 0.10, 'x2': 0.35, 'y2': 0.20}`
- List format: `[0.25, 0.10, 0.35, 0.20]`

**Backward Compatibility:**
- Handles both old (list) and new (dict) formats
- Reconstructs groups from region data if groups section missing
- Calculates image coordinates from normalized if image is loaded

## 10. Data Validation Rules

### Region Name Validation
- **Required:** Yes
- **Uniqueness:** Must be unique
- **Type:** String
- **Empty:** Not allowed

### Coordinate Validation
- **Normalized:** All values 0.0-1.0, floats
- **Image:** All values 0 to image_dimension, integers
- **Order:** x1 < x2, y1 < y2 (after normalization)

### Group Name Validation
- **Required:** No (can be None)
- **Uniqueness:** Must be unique if provided
- **Type:** String or None
- **Empty String:** Treated as None

### Color Validation
- **Required:** No (defaults to 'blue')
- **Values:** Must be in color_map
- **Type:** String

