# Workflow Documentation

## Overview

This document describes the typical user workflows for OCR-Orc, from initial setup through coordinate export and OCR integration.

## 1. Initial Setup Workflow

### 1.1 Starting the Application

**Steps:**
1. Launch OCR-Orc
2. Application opens with empty canvas
3. Toolbar shows default mode (Create)
4. Side panel shows empty lists

**Initial State:**
- No PDF loaded
- No regions defined
- Create mode active
- Zoom at 100%

### 1.2 Loading a PDF Document

**Steps:**
1. Click "Load PDF" button or press Cmd+O
2. File dialog appears
3. Select PDF file to calibrate
4. PDF first page is converted to image (150 DPI)
5. Image displays on canvas (centered, scaled to fit)
6. File label updates with PDF filename
7. Existing regions are cleared (if any)

**Result:**
- PDF image visible on canvas
- Ready to define regions
- Canvas scrollable if image is large

## 2. Region Creation Workflow

### 2.1 Creating a Single Region

**Steps:**
1. Ensure in Create mode (button or press 1)
2. Enter region name in "Region Name" field (e.g., "Last Name Cell 1")
3. Optionally select color from dropdown
4. Optionally enter group name (e.g., "lastname")
5. Click and drag on canvas to draw rectangle
6. Release mouse to create region
7. Region appears with colored border and label
8. Region name auto-increments (e.g., "Last Name Cell 2")

**Visual Feedback:**
- Red rectangle during drag
- Colored border and label after creation
- Region appears in regions list

**Result:**
- Region created with all three coordinate systems
- Region added to group (if specified)
- Ready to create next region

### 2.2 Creating Multiple Regions (Character Cells)

**Workflow for Character Cell Forms:**

**Example: Last Name (3 characters)**

1. **First Cell:**
   - Name: "Last Name Cell 1"
   - Group: "lastname"
   - Color: blue
   - Draw rectangle around first character cell

2. **Second Cell:**
   - Name auto-increments to "Last Name Cell 2"
   - Group: "lastname" (same)
   - Draw rectangle around second character cell

3. **Third Cell:**
   - Name auto-increments to "Last Name Cell 3"
   - Group: "lastname" (same)
   - Draw rectangle around third character cell

**Repeat for:**
- First Name cells
- Student Number cells
- Any other fields

**Result:**
- Multiple regions created
- Regions organized into groups
- Ready for export

### 2.3 Batch Region Creation Tips

**Efficiency Tips:**
- Use consistent naming patterns (enables auto-increment)
- Set group name once, reuse for related cells
- Use same color for related regions
- Zoom in for precise placement
- Use mask overlay to verify coverage

## 3. Region Editing Workflow

### 3.1 Adjusting Region Position

**Steps:**
1. Switch to Select mode (button or press 2)
2. Click on region to select it
3. Drag region to new position
4. Release to finalize
5. Position updates in all coordinate systems

**Visual Feedback:**
- Region highlights when selected
- Region moves in real-time during drag
- Resize handles appear (for single selection)

### 3.2 Adjusting Region Size

**Steps:**
1. Switch to Select mode
2. Click region to select (single selection only)
3. Resize handles appear (8 handles)
4. Click and drag a handle
5. Release to finalize
6. Size updates in all coordinate systems

**Handle Types:**
- Corner handles: Resize width and height
- Edge handles: Resize one dimension

**Visual Feedback:**
- Cursor changes based on handle
- Region resizes in real-time
- Coordinates update continuously

### 3.3 Renaming a Region

**Methods:**
1. **Double-click region** on canvas
2. **Right-click region** → "Edit Name"
3. **Select region** → Press E key
4. **Select in listbox** → Click "Edit Selected"

**Steps:**
1. Edit dialog appears
2. Current name is pre-filled and selected
3. Type new name
4. Press Enter or click Save
5. Region name updates
6. Updates in groups if applicable

**Validation:**
- Name must be unique
- Name cannot be empty
- Error shown if duplicate

### 3.4 Changing Region Color

**Methods:**
1. **Right-click region** → "Change Color"
2. **Select region in listbox** → Use Change Color button

**Steps:**
1. Color dialog appears
2. Current color is pre-selected
3. Choose new color from dropdown
4. Click Save
5. Region border color updates

**Use Cases:**
- Visual organization
- Distinguishing field types
- Color-coding for different data categories

## 4. Selection Workflow

### 4.1 Single Selection

**Steps:**
1. Switch to Select mode
2. Click on region
3. Region is selected (highlighted)
4. Resize handles appear
5. Region info shown in side panel

**Use Cases:**
- Moving single region
- Resizing region
- Editing region properties

### 4.2 Multi-Selection

**Methods:**

**Method 1: Command+Click**
1. Click first region
2. Hold Command and click additional regions
3. Each click toggles selection
4. All selected regions highlighted

**Method 2: Box Selection**
1. Click empty space on canvas
2. Drag to draw selection box
3. All regions intersecting box are selected
4. Release to finalize

**Method 3: Shift+Drag (Add to Selection)**
1. Have some regions selected
2. Hold Shift and drag selection box
3. New regions added to existing selection
4. Release to finalize

**Use Cases:**
- Moving multiple regions together
- Grouping multiple regions
- Bulk operations

### 4.3 Selection Operations

**Select All:**
- Press Cmd+A
- All regions selected

**Deselect All:**
- Press Cmd+D
- Selection cleared

**Invert Selection:**
- Press Cmd+I
- Selected becomes unselected, vice versa

## 5. Group Management Workflow

### 5.1 Creating a Group

**Method 1: During Region Creation**
1. Enter group name in "Group" field
2. Create regions
3. Regions automatically added to group

**Method 2: From Existing Regions**
1. Select multiple regions in listbox
2. Press Cmd+G or click "Group Selected"
3. Enter group name
4. Regions added to group

**Result:**
- Group appears in Groups tab
- Format: "Group Name (N regions)"
- Regions show group in info panel

### 5.2 Adding Regions to Group

**Steps:**
1. Select regions in listbox
2. Click "Add to Group" button
3. Enter group name (existing or new)
4. Regions added to group

**Behavior:**
- Creates group if doesn't exist
- Adds to existing group if exists
- Updates region group assignments

### 5.3 Removing Regions from Group

**Steps:**
1. Select regions in listbox
2. Click "Remove from Group" button
3. Regions removed from their groups
4. Groups deleted if empty

**Result:**
- Regions become ungrouped
- Group removed if no members remain

### 5.4 Deleting a Group

**Steps:**
1. Switch to Groups tab
2. Select group in listbox
3. Click "Delete Group" button
4. Group is deleted
5. All member regions become ungrouped

**Result:**
- Group removed
- Regions still exist but are ungrouped

## 6. Navigation Workflow

### 6.1 Zooming

**Methods:**
1. **Buttons:** Click + or - buttons
2. **Keyboard:** Press +/= (in) or - (out)
3. **Mouse:** Hold Command and scroll
4. **Reset:** Press 0 or click Reset button

**Workflow:**
1. Zoom in for precise region placement
2. Zoom out to see full document
3. Reset to fit canvas

**Zoom Range:**
- Minimum: 20% (0.2x)
- Maximum: 500% (5.0x)
- Default: 100% (1.0x)

### 6.2 Scrolling

**Methods:**
1. **Scrollbars:** Click and drag scrollbars
2. **Mouse Wheel:** Scroll vertically
3. **Shift+Scroll:** Scroll horizontally
4. **Magic Mouse:** Side-to-side scrolling

**Use Cases:**
- Navigating large documents
- Accessing regions outside viewport
- Panning while zoomed in

## 7. Export Workflow

### 7.1 Exporting Coordinates

**Steps:**
1. Define all regions
2. Organize into groups (optional)
3. Click "Export Coordinates" or press Cmd+S
4. File dialog appears
5. Select location and filename
6. Choose format (via file extension):
   - `.json` for JSON
   - `.csv` for CSV
   - `.txt` for TXT
   - `.xml` for XML
   - `.yaml` for YAML
7. Click Save
8. Success message appears

**Export Contents:**
- Normalized coordinates (0.0-1.0) only
- Region metadata (name, color, group)
- Group definitions
- Document metadata (aspect ratio, image size)

**Result:**
- Coordinate file saved
- Ready for use in OCR pipeline
- Resolution-independent coordinates

### 7.2 Exporting Mask Image

**Steps:**
1. Define all regions
2. Click "Export Mask" or press Cmd+M
3. File dialog appears (image formats)
4. Select location and filename
5. Choose format (PNG, JPEG, TIFF)
6. Click Save
7. Mask image saved
8. Coordinate JSON file saved alongside
9. Success message shows both paths

**Mask Usage:**
- Overlay on original document during OCR
- Black areas hide unnecessary information
- White rectangles show only labeled cells
- Improves OCR accuracy

**Result:**
- Mask image file
- Coordinate JSON file
- Ready for OCR preprocessing

## 8. Import Workflow

### 8.1 Loading Saved Coordinates

**Steps:**
1. Load PDF document (same aspect ratio as calibration)
2. Click "Load Coordinates" or press Cmd+Shift+S
3. File dialog appears (JSON files)
4. Select coordinate file
5. Coordinates load and display
6. Regions appear on canvas
7. Groups are reconstructed
8. Success message shows region count

**Requirements:**
- PDF must be loaded first
- Coordinate file must be valid JSON
- Supports both dict and list coordinate formats

**Result:**
- All regions restored
- Groups reconstructed
- Ready to edit or use

## 9. Quality Assurance Workflow

### 9.1 Verifying Region Placement

**Steps:**
1. Zoom in on regions
2. Check region boundaries align with character cells
3. Use mask overlay (M key) to visualize OCR extraction areas
4. Adjust regions as needed
5. Test extraction (T key) to verify OCR accuracy

**Visual Checks:**
- Regions align with form cells
- No overlap between regions
- Regions cover entire character cells
- Mask shows correct extraction areas

### 9.2 Testing OCR Extraction

**Steps:**
1. Define all regions
2. Press T key or click "Test Extraction"
3. OCR runs on all regions
4. Results window appears
5. Review extracted text and confidence scores
6. Adjust regions if accuracy is low

**Results Display:**
- Region name
- Extracted text
- Average confidence score

**Use Cases:**
- Validate region placement
- Test OCR accuracy
- Identify problematic regions
- Fine-tune coordinates

## 10. Complete Calibration Workflow

### 10.1 End-to-End Process

**Step 1: Preparation**
1. Launch OCR-Orc
2. Load calibration PDF (blank form or sample)
3. Verify document displays correctly

**Step 2: Region Definition**
1. Switch to Create mode
2. For each field type (e.g., Last Name):
   - Set group name (e.g., "lastname")
   - Set color (e.g., blue)
   - Create region for each character cell
   - Name: "Last Name Cell 1", "Last Name Cell 2", etc.
3. Repeat for all fields

**Step 3: Verification**
1. Switch to Select mode
2. Review all regions
3. Adjust positions/sizes as needed
4. Enable mask overlay to verify
5. Test extraction if OCR available

**Step 4: Organization**
1. Verify groups are correct
2. Adjust group assignments if needed
3. Verify region names are descriptive

**Step 5: Export**
1. Export coordinates (Cmd+S)
   - Choose format (typically JSON)
   - Save to project directory
2. Export mask (Cmd+M) (optional)
   - Save mask image
   - Coordinate JSON saved alongside

**Step 6: Integration**
1. Use exported coordinates in OCR pipeline
2. Load coordinates for new documents
3. Apply mask for preprocessing (optional)

## 11. Common Workflow Patterns

### 11.1 Character Cell Form Pattern

**Typical Form Structure:**
- Last Name: 10-15 character cells
- First Name: 10-15 character cells
- Student Number: 8-10 character cells
- Other fields as needed

**Workflow:**
1. Create group "lastname"
2. Create regions "Last Name Cell 1" through "Last Name Cell N"
3. Repeat for each field type
4. Export coordinates
5. Use in automated processing

### 11.2 Form Field Pattern

**Typical Form Structure:**
- Single regions for each field
- No character cells
- Larger regions for text fields

**Workflow:**
1. Create one region per field
2. Name regions descriptively (e.g., "Address Line 1")
3. Group related fields (optional)
4. Export coordinates

### 11.3 Batch Calibration Pattern

**Workflow:**
1. Calibrate one form type
2. Export coordinates
3. Load coordinates for new documents
4. Verify regions align (same aspect ratio)
5. Adjust if needed
6. Process documents

## 12. Error Recovery Workflow

### 12.1 Using Undo/Redo

**Common Scenarios:**
- Accidentally deleted region → Undo (Cmd+Z)
- Moved region incorrectly → Undo
- Wrong region name → Undo, redo with correct name
- Grouped incorrectly → Undo

**Workflow:**
1. Perform operation
2. Realize mistake
3. Press Cmd+Z to undo
4. Correct operation
5. Continue

**Limitations:**
- Maximum 50 undo levels
- Redo cleared on new action

### 12.2 Recovering from Errors

**File Load Error:**
- Error message shown
- Current state maintained
- Try different PDF file

**Export Error:**
- Error message shown
- Data preserved
- Check file permissions
- Try different location

**Import Error:**
- Error message shown
- Validate JSON format
- Check coordinate ranges
- Verify file structure

## 13. Best Practices

### 13.1 Region Naming

**Conventions:**
- Use descriptive names
- Include field type and position
- Use consistent patterns
- Enable auto-increment

**Examples:**
- "Last Name Cell 1"
- "First Name Cell 1"
- "Student Number Digit 1"

### 13.2 Group Organization

**Strategies:**
- Group by field type (lastname, firstname)
- Group by form section
- Use consistent group names
- Keep groups logical

### 13.3 Color Coding

**Strategies:**
- Same color for related regions
- Different colors for different field types
- Use colors for visual organization

### 13.4 Coordinate Accuracy

**Tips:**
- Zoom in for precise placement
- Use mask overlay to verify
- Test extraction to validate
- Adjust regions iteratively

### 13.5 Workflow Efficiency

**Tips:**
- Use keyboard shortcuts
- Leverage auto-increment
- Batch create related regions
- Use groups for organization
- Save frequently (export coordinates)

## 14. Integration Workflow

### 14.1 OCR Pipeline Integration

**Steps:**
1. Export coordinates from OCR-Orc
2. In OCR pipeline:
   - Load coordinate file
   - Load document image
   - Calculate image coordinates: `image_coords = normalized * image_dimensions`
   - Crop regions from image
   - Run OCR on each region
   - Extract text

**Coordinate Usage:**
```python
# Pseudo-code
coordinates = load_json("calibration.json")
image = load_image("document.pdf")

for region_name, region_data in coordinates['regions'].items():
    norm_coords = region_data['normalized_coords']
    # Calculate image coordinates
    img_x1 = norm_coords['x1'] * image.width
    img_y1 = norm_coords['y1'] * image.height
    img_x2 = norm_coords['x2'] * image.width
    img_y2 = norm_coords['y2'] * image.height
    # Crop and OCR
    cropped = image.crop((img_x1, img_y1, img_x2, img_y2))
    text = ocr.extract(cropped)
```

### 14.2 Mask Integration

**Steps:**
1. Export mask image from OCR-Orc
2. In OCR pipeline:
   - Load original document image
   - Load mask image
   - Overlay mask on original (black areas hide content)
   - Run OCR on masked image
   - Improved accuracy (less noise)

**Mask Usage:**
- Black areas: Hidden from OCR
- White rectangles: Visible to OCR
- Reduces false positives
- Improves character recognition

## 15. Maintenance Workflow

### 15.1 Updating Coordinates

**When Needed:**
- Form design changes
- Scanner settings change (if aspect ratio changes)
- OCR accuracy issues

**Workflow:**
1. Load existing coordinate file
2. Load new calibration PDF
3. Adjust regions as needed
4. Export updated coordinates
5. Test with sample documents

### 15.2 Version Control

**Best Practices:**
- Save coordinate files with version numbers
- Document calibration date
- Keep calibration PDFs for reference
- Track coordinate file versions

**File Naming:**
- `student_form_v1_coordinates.json`
- `student_form_v2_coordinates.json`
- Include date in filename if helpful

