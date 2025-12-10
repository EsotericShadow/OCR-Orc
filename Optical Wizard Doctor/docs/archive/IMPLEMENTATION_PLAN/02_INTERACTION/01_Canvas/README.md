# Canvas - Implementation Steps

## Objective

Implement the canvas widget that displays the PDF document and renders regions.

## Prerequisites

- PDF loading functionality working
- DocumentState model ready
- Basic UI layout complete

## Sub-Projects

1. **PDF Display** - Load and display PDF first page as image
2. **Region Rendering** - Draw regions on canvas with proper styling

## Order of Implementation

Complete these in order:
1. PDF Display (foundation)
2. Region Rendering (builds on PDF display)

## Steps

### Step 1: Create Canvas Widget

1. Create `src/ui/Canvas.h` and `Canvas.cpp`
2. Inherit from `QWidget`
3. Override `paintEvent()` for custom drawing
4. Add member variables for document image and regions

### Step 2: Implement PDF Display

1. Integrate PdfLoader utility
2. Load PDF first page when requested
3. Convert to QImage
4. Store in canvas widget
5. Display in paintEvent()

### Step 3: Implement Region Rendering

1. Iterate through all regions in DocumentState
2. Convert normalized coordinates to canvas coordinates
3. Draw region rectangles with appropriate styling
4. Draw region labels
5. Handle selection and hover states

### Step 4: Add Scroll Support

1. Wrap canvas in QScrollArea
2. Set scroll region based on image size
3. Handle scrollbar visibility
4. Update scroll position on zoom/pan

### Step 5: Test Canvas

1. Load a test PDF
2. Verify image displays correctly
3. Create test regions
4. Verify regions render correctly
5. Test scrolling behavior

## Acceptance Criteria

- [ ] Canvas widget created
- [ ] PDF displays correctly
- [ ] Regions render with correct styling
- [ ] Canvas is scrollable
- [ ] Performance is acceptable

## Next Steps

After completing Canvas, proceed to Region Operations to enable user interaction.
