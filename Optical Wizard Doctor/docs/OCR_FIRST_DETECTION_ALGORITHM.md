# OCR-First Detection Algorithm Design

## Problem Statement

Current detection produces ~890 low-confidence regions, which is too many and not useful. The pure computer vision approach is too sensitive and lacks semantic understanding.

## Proposed Solution: OCR-First Two-Stage Pipeline

### Core Concept
1. **Stage 1: OCR Text Extraction** - Extract all text and their bounding boxes
2. **Stage 2: CV Refinement** - Use OpenCV to find bounding boxes around text regions

This approach is superior because:
- OCR provides semantic understanding (knows what's text vs noise)
- Text regions are the actual targets we care about
- CV refinement is more accurate when we know where to look
- Reduces false positives dramatically

## Stage 1: OCR Text Extraction

### Technology Choice
- **Tesseract OCR** (industry standard, well-maintained, good accuracy)
- Alternative: **PaddleOCR** (better for some languages, but Tesseract is simpler)

### Process
1. Run OCR on the entire page
2. Extract:
   - Text content
   - Bounding box coordinates (x, y, width, height)
   - Confidence scores
   - Text type hints (if available)

### Output
```cpp
struct OCRTextRegion {
    QString text;              // Extracted text
    cv::Rect boundingBox;      // Text bounding box
    double confidence;         // OCR confidence
    QString typeHint;          // "number", "letter", "mixed", "checkbox_label"
};
```

## Stage 2: CV Refinement Pipeline

### 2.1 Text Box Detection
For each OCR text region:
1. **Expand search radius** around text (e.g., 5-20% of text height)
2. **Detect horizontal lines** near text:
   - Look for lines above/below text (form field underlines)
   - Look for lines left/right of text (form field borders)
3. **Detect rectangles/squares**:
   - Form field boxes
   - Text input areas
4. **Merge overlapping detections**

### 2.2 Checkbox Detection & Pattern Recognition

#### Checkbox Detection
- **Square detection** near text (left or right side)
- **Size validation**: Checkboxes are typically 10-20px square
- **Proximity check**: Must be within reasonable distance of text

#### Pattern Recognition for Checkbox Placement
```
Algorithm:
1. Group OCR regions by horizontal alignment (same Y-range, tolerance: 2% of page height)
2. For each horizontal line:
   a. Sort regions by X-coordinate
   b. Analyze first and last items:
      - If first item has checkbox on LEFT → pattern = "checkbox_left"
      - If last item has checkbox on RIGHT → pattern = "checkbox_right"
      - If mixed → analyze majority
   c. Apply pattern to all items in line
3. For ambiguous cases:
   - Check if checkbox is closer to text start or end
   - Use majority voting across multiple lines
```

#### Multiple Items Per Line
- **Horizontal clustering**: Group items with small gaps (< 5% of page width)
- **Sequence detection**: Identify patterns like "☐ Option 1 ☐ Option 2 ☐ Option 3"
- **Group assignment**: Items in same sequence → same group

### 2.3 Form Field Detection

#### Text Input Fields
1. **Horizontal line detection** below text (underline)
2. **Rectangle detection** around text
3. **Combine**: Text box + detected lines/rectangles = form field region

#### Multi-line Text Areas
- **Vertical expansion**: If multiple text regions are vertically aligned
- **Rectangle detection**: Find bounding rectangle around all text
- **Line detection**: Look for borders around the area

### 2.4 Grid/Table Detection
1. **OCR regions** provide initial grid structure
2. **Line detection** to find grid lines
3. **Intersection detection** to refine grid cells
4. **Validation**: Ensure OCR regions align with grid cells

## Algorithm Flow

```
1. OCR Extraction
   └─> Extract all text regions with coordinates
   
2. Horizontal Line Grouping
   └─> Group OCR regions by Y-coordinate (rows)
   
3. For each row:
   a. Checkbox Pattern Detection
      └─> Determine if checkboxes are left/right
   b. Item Clustering
      └─> Group items that belong together
   c. CV Refinement
      └─> For each text region:
          - Expand search radius
          - Detect lines/rectangles
          - Merge into final bounding box
   d. Type Inference
      └─> Checkbox vs Text Input vs Label
   
4. Vertical Grouping
   └─> Group rows into form sections
   
5. Confidence Scoring
   └─> Combine OCR confidence + CV detection quality
```

## Implementation Structure

### New Classes

#### `OCRTextExtractor`
- Wraps Tesseract OCR
- Extracts text regions with coordinates
- Filters low-confidence results

#### `TextRegionRefiner`
- Takes OCR text region
- Expands search area
- Detects lines/rectangles around text
- Returns refined bounding box

#### `CheckboxDetector`
- Detects square shapes near text
- Determines left/right placement
- Validates checkbox size/position

#### `FormFieldDetector`
- Detects form input fields
- Combines text + lines/rectangles
- Handles multi-line text areas

#### `PatternAnalyzer`
- Analyzes checkbox placement patterns
- Determines left/right/mixed patterns
- Applies patterns to ambiguous cases

## Confidence Scoring

### Multi-factor Confidence
```cpp
double confidence = 
    (ocrConfidence * 0.4) +           // OCR quality
    (lineDetectionScore * 0.3) +     // CV line detection
    (rectangularityScore * 0.2) +    // How rectangular is the box
    (patternMatchScore * 0.1);       // Matches detected pattern
```

### Filtering
- **High confidence**: >= 0.7 → Always include
- **Medium confidence**: 0.5-0.7 → Include if matches pattern
- **Low confidence**: < 0.5 → Exclude unless user explicitly requests

## Benefits

1. **Dramatically fewer false positives** - Only regions with actual text
2. **Better accuracy** - OCR knows what's text vs noise
3. **Semantic understanding** - Knows text content, can infer types
4. **Pattern recognition** - Understands form structure
5. **Checkbox handling** - Properly detects and positions checkboxes
6. **Multi-item lines** - Handles complex form layouts

## Migration Path

1. **Phase 1**: Add OCR extraction alongside current CV detection
2. **Phase 2**: Use OCR regions to seed CV refinement
3. **Phase 3**: Replace current detection with OCR-first approach
4. **Phase 4**: Add pattern recognition and checkbox detection

## Dependencies

- **Tesseract OCR**: `brew install tesseract` (macOS)
- **Tesseract C++ API**: Already available via system libraries
- **OpenCV**: Already integrated

## Example Output

Instead of 890 low-confidence regions, we'd get:
- ~20-50 high-confidence regions (actual form fields)
- Each region has:
  - Extracted text content
  - Accurate bounding box
  - Type (checkbox, text input, label)
  - Pattern information (checkbox left/right)
  - Group assignments
