# Magic Detect Algorithm - Complete Recipe

## Overview
The Magic Detect algorithm uses a **concurrent multi-pipeline consensus approach** to detect empty form fields in scanned documents. It combines OCR text extraction (for coordinate hints) with computer vision rectangle detection, then matches results for consensus.

---

## Algorithm Flowchart

```
┌─────────────────────────────────────────────────────────────────┐
│                    START: Magic Detect                          │
└───────────────────────┬─────────────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────┐
        │  Stage 1: OCR Text Extraction  │
        │  - Extract all text + coords   │
        │  - Get coordinate hints only   │
        └───────────────┬─────────────────┘
                        │
                        ▼
        ┌───────────────────────────────┐
        │  Stage 2: Pattern Analysis      │
        │  - Detect checkboxes           │
        │  - Analyze checkbox patterns   │
        └───────────────┬─────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 1: Find Empty Form Fields Near OCR Hints    │
        │  - Search below, right, above labels             │
        │  - Detect empty boxes/underlines                 │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 2: Filter Regions Containing Text           │
        │  - Remove any regions with OCR text overlap       │
        │  - Keep only EMPTY form fields                    │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 3: EXTREME Overfitting                      │
        │  - Expand 40% horizontal, 60% vertical          │
        │  - Add minimum pixel expansions                   │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 3.5: Smart Boundary Detection               │
        │  - Find actual edges within overfitted regions   │
        │  - Anchor to hard edges (above, left, right)     │
        │  - Increase vertical height                       │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 4: Detect Cell Groups with Shared Walls     │
        │  - Find vertical walls (grid patterns)           │
        │  - Group horizontally aligned cells               │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 5: Classify and Filter                      │
        │  - Filter out titles/headings                     │
        │  - Classify: TextLine, TextBlock, Cell, etc.     │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 6: CONCURRENT Rectangle Detection           │
        │  ┌─────────────────────────────────────────────┐ │
        │  │  - Multiple Canny thresholds (10/30, 20/60)│ │
        │  │  - Multiple binary thresholds (5 levels)   │ │
        │  │  - Aggressive morphology (5x5 kernel, 3x)   │ │
        │  │  - RETR_TREE for nested rectangles         │ │
        │  │  - Overfit detected rectangles (10%)       │ │
        │  └─────────────────────────────────────────────┘ │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 7: Match and Merge Pipelines                │
        │  - Match OCR regions with rectangles (IoU 0.3)   │
        │  - Filter text-containing regions                │
        │  - Create consensus regions                      │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 8.5: FINAL Text Filter                      │
        │  - Check OCR overlap (>10% = text)              │
        │  - Check brightness (<0.7 = text)               │
        │  - Check edge density (>0.08 = text)            │
        │  - Check horizontal lines (3+ = text)           │
        │  - REJECT any region with text                   │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 9: Enhance and Classify                     │
        │  - Add checkbox detection                        │
        │  - Classify field types                           │
        │  - Calculate final confidence                     │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  Pass 10: Group Inference                         │
        │  - Infer groups from spatial patterns            │
        │  - Infer groups from content patterns             │
        └───────────────┬───────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │  RETURN: DetectionResult                          │
        │  - List of empty form field regions               │
        │  - Confidence scores                              │
        │  - Inferred types and groups                     │
        └───────────────────────────────────────────────────┘
```

---

## Detailed Recipe: Step-by-Step Instructions

### **Stage 1: OCR Text Extraction (Coordinate Hints Only)**

**Ingredients:**
- Source image (QImage)
- Tesseract OCR engine

**Steps:**
1. Convert image to grayscale
2. Apply Gaussian blur (3x3 kernel)
3. Apply adaptive thresholding (Gaussian, block 11, C=2)
4. Initialize Tesseract with PSM_SPARSE_TEXT mode (PSM 11)
5. Extract all text regions at word level
6. For each word, capture:
   - Text content
   - Bounding box coordinates
   - OCR confidence
   - Inferred type (number, letter, mixed)
7. **Important:** Text is ONLY used as coordinate hints - we never detect/highlight text itself

**Output:** List of `OCRTextRegion` objects (text + coordinates)

---

### **Stage 2: Pattern Analysis**

**Ingredients:**
- OCR text regions from Stage 1
- CheckboxDetector
- PatternAnalyzer

**Steps:**
1. For each OCR region, detect nearby checkboxes
2. Analyze checkbox placement patterns (left/right of text)
3. Store checkbox detection results for later use

**Output:** Checkbox detections and pattern information

---

### **Pass 1: Find Empty Form Fields Near OCR Hints**

**Ingredients:**
- OCR text regions (coordinate hints)
- TextRegionRefiner
- Source image (cv::Mat)

**Steps:**
1. For each OCR hint:
   - Check if it's a label (ends with ':', short, common words)
   - If label, search for empty form field nearby:
     - **Priority 1:** Search BELOW label (5px below, 20px left/right expansion)
     - **Priority 2:** Search to the RIGHT of label
     - **Priority 3:** Search ABOVE label
2. For each search area:
   - Apply adaptive thresholding (inverted) to highlight empty (white) areas
   - Use morphological operations to find underlines (horizontal kernel)
   - Find contours of lines and score by width/brightness
   - Also search for rectangular boxes using Canny edges
   - Check for empty interiors (high brightness >0.65)
   - Return best empty field candidate
3. Deduplicate found fields

**Output:** List of candidate empty form fields (cv::Rect)

---

### **Pass 2: Filter Regions Containing Text**

**Ingredients:**
- Candidate fields from Pass 1
- OCR text regions
- Source image

**Steps:**
1. For each candidate field:
   - Check if region overlaps with any OCR text (>10% overlap = contains text)
   - Check brightness (mean brightness <0.7 = contains text)
   - Check edge density (edge density >0.1 = contains text)
   - If ANY check fails, REJECT the region
2. Keep only regions that pass ALL checks (truly empty)

**Output:** Validated empty form fields (no text content)

---

### **Pass 3: Extreme Overfitting**

**Ingredients:**
- Validated empty fields from Pass 2
- FormFieldDetector
- Source image

**Steps:**
1. For each validated field:
   - Calculate expansion amounts:
     - Horizontal: 40% of width (minimum 25px)
     - Vertical: 60% of height (minimum 30px)
   - Expand rectangle in all directions
   - Clamp to image boundaries
2. **Purpose:** Ensure we capture full form field boundaries, even if initial detection was slightly off

**Output:** Overfitted regions (larger than actual fields)

---

### **Pass 3.5: Smart Boundary Detection**

**Ingredients:**
- Overfitted regions from Pass 3
- FormFieldDetector
- Source image

**Steps:**
1. For each overfitted region:
   - Extract ROI (region of interest) from image
   - Apply Canny edge detection (50, 150 thresholds)
   - Apply morphological dilation to connect edges
   - Find contours within the overfitted area
   - Score contours by:
     - Rectangularity (how close to perfect rectangle)
     - Size appropriateness (30-400px wide, 10-60px tall)
     - Emptiness (brightness >0.65)
   - Select best contour as actual form field boundary
2. **Hard Edge Anchoring:**
   - Search 80px ABOVE for horizontal lines → lock top edge
   - Search 80px LEFT for vertical lines → lock left edge
   - Search 80px RIGHT for vertical lines → lock right edge
   - Use HoughLinesP for sensitive line detection
3. **Vertical Expansion:**
   - Extend downward by 80% for more vertical room
   - Extend upward by 50% if no hard edge found
4. **Fallback:** If no strong box found, try horizontal line detection for text lines

**Output:** Refined regions with precise boundaries

---

### **Pass 4: Detect Cell Groups with Shared Walls**

**Ingredients:**
- Refined regions from Pass 3.5
- FormFieldDetector
- Source image

**Steps:**
1. **Detect All Vertical Walls in Image:**
   - Use extremely sensitive Canny (20, 80 thresholds)
   - Apply multiple thresholding techniques (adaptive, binary, regular)
   - Use tall vertical kernels for morphology
   - Use HoughLinesP for very thin vertical lines
   - Criteria: height > width * 1.5, height > 40% of region
   - Sort and deduplicate walls (2px tolerance)
2. **Group Regions by Shared Walls:**
   - For each region, check if it's separated by detected walls
   - Group horizontally aligned regions that:
     - Share vertical walls between them, OR
     - Are very adjacent (<5px gap)
   - Flatten groups back to individual regions (for now)

**Output:** Regions with cell grouping information

---

### **Pass 5: Classify and Filter**

**Ingredients:**
- Regions from Pass 4
- FormFieldDetector
- OCR text regions
- Source image

**Steps:**
1. **Aggressively Filter Titles/Headings:**
   - Check for significant overlap with large OCR text (height >18px or width >80px)
   - Check for very large/wide regions (height >35px or width >75% of image)
   - Check for low brightness (<0.75) and high edge density (>0.15)
   - Check for lack of form field structure (no strong lines/boxes)
   - REJECT if any title/heading indicators found
2. **Classify Remaining Regions:**
   - **TextBlock:** 3+ evenly spaced horizontal lines, height >40px
   - **TextLine:** Single strong underline detected
   - **Cell:** Box borders detected, small size
   - **TextInput:** Default for other valid form fields
   - **CheckboxField:** If checkbox detected nearby

**Output:** Classified form fields (titles/headings removed)

---

### **Pass 6: Concurrent Rectangle Detection (Secondary Pipeline)**

**Ingredients:**
- Source image (cv::Mat)
- RectangleDetector

**Steps:**
1. **Convert to Grayscale**
2. **Multiple Detection Methods (Combined):**
   - **Canny Edge Detection:** 3 levels (10/30, 20/60, 30/90 thresholds)
   - **Adaptive Thresholding:** Gaussian, block 11, C=2
   - **Binary Thresholding:** 5 levels (80, 120, 160, 200, 240 thresholds)
   - Combine all methods with bitwise OR
3. **Aggressive Morphology:**
   - Use 5x5 kernel (larger than normal)
   - Apply MORPH_CLOSE 3 times
   - Apply dilation 2 times
   - Connect broken edges
4. **Find Contours:**
   - Use RETR_TREE (catches nested rectangles)
   - CHAIN_APPROX_SIMPLE
5. **Filter and Score:**
   - Calculate rectangularity (area ratio + angle quality)
   - Accept if rectangularity >0.15 OR area ratio >0.4 (very lenient)
   - Filter by size (15x10 min, 800x300 max)
6. **Overfit Detected Rectangles:**
   - Expand by 10% in all directions (minimum 5px)
   - Clamp to image boundaries
7. **Deduplicate:**
   - Remove rectangles with >70% overlap

**Output:** List of detected rectangles/squares (DetectedRectangle)

---

### **Pass 7: Match and Merge Pipelines (Consensus)**

**Ingredients:**
- Classified fields from Pass 5 (OCR-first pipeline)
- Rectangle detections from Pass 6 (Rectangle pipeline)
- OCR text regions (for text filtering)
- Source image

**Steps:**
1. **For each OCR-first region:**
   - Check if it matches any rectangle detection:
     - Calculate IoU (Intersection over Union)
     - Calculate individual overlaps (OCR overlap, CV overlap)
     - Match if: IoU >0.3 OR OCR overlap >0.4 OR CV overlap >0.4
   - If matched:
     - Use intersection of both regions (or prefer rectangle if confidence >0.8)
     - Mark as "consensus" (detected by both pipelines)
     - Confidence: 0.85
   - If not matched:
     - Still include OCR region (fallback)
     - Mark as "ocr-first"
     - Confidence: 0.70
   - **CRITICAL:** Check if region contains text BEFORE adding:
     - OCR overlap >10%? → REJECT
     - Brightness <0.7? → REJECT
     - Edge density >0.08? → REJECT
     - 3+ horizontal lines? → REJECT
2. **For each rectangle detection (not matched):**
   - If confidence >0.5:
     - Check if already matched (overlap >50% with existing)
     - If not matched:
       - **CRITICAL:** Check if region contains text BEFORE adding
       - If no text, add as "rectangle-detection"
       - Confidence: rectangle confidence

**Output:** Merged regions (consensus + high-confidence single-pipeline)

---

### **Pass 8.5: Final Text Filter (Critical Gate)**

**Ingredients:**
- Merged regions from Pass 7
- OCR text regions
- Source image

**Steps:**
1. For each merged region:
   - **Check 1: OCR Overlap**
     - Calculate overlap with all OCR text regions
     - If overlap >10% of region area OR >10% of OCR area → REJECT
   - **Check 2: Brightness**
     - Extract ROI from image
     - Calculate mean brightness
     - If brightness <0.7 → REJECT (too dark = contains text)
   - **Check 3: Edge Density**
     - Apply Canny edge detection to ROI
     - Calculate edge density (edge pixels / total pixels)
     - If edge density >0.08 → REJECT (too many edges = text)
   - **Check 4: Horizontal Lines**
     - Count horizontal edge runs >5px
     - If 3+ horizontal runs → REJECT (multiple text lines)
2. **Only regions passing ALL 4 checks are kept**

**Output:** Text-filtered regions (ONLY empty form fields)

---

### **Pass 9: Enhance and Classify**

**Ingredients:**
- Text-filtered regions from Pass 8.5
- OCR text regions
- Checkbox detections
- FormFieldDetector
- Source image

**Steps:**
1. For each region:
   - **Checkbox Detection:**
     - Find nearby checkboxes (within 50px)
     - If checkbox found, mark as CheckboxField
   - **Type Classification:**
     - Re-classify using FormFieldDetector
     - Types: TextLine, TextBlock, Cell, TextInput, CheckboxField
   - **Confidence Adjustment:**
     - If consensus (both pipelines): boost confidence (+0.1, max 0.95)
     - If checkbox: confidence = 0.9
     - If text input: confidence = max(current, 0.8)
   - **Assign Metadata:**
     - inferredType: "checkbox", "text_line", "text_block", "cell", "text_input"
     - suggestedGroup: "CheckboxGroup", "TextLineGroup", etc.
     - suggestedColor: "blue", "red", "green", etc.

**Output:** Enhanced DetectedRegion objects with full metadata

---

### **Pass 10: Group Inference**

**Ingredients:**
- Enhanced regions from Pass 9
- GroupInferencer

**Steps:**
1. **Spatial Grouping:**
   - Group regions by proximity and alignment
   - Identify patterns (rows, columns, grids)
2. **Pattern-Based Grouping:**
   - Detect postal code patterns (A#A-#A#)
   - Detect name field patterns
   - Detect number sequences
3. **Merge Groups:**
   - Combine spatial and pattern groups
   - Assign group names and colors

**Output:** DetectedGroup objects with region associations

---

### **Final Assembly**

**Steps:**
1. Count confidence levels:
   - High: >=0.8
   - Medium: 0.5-0.8
   - Low: <0.5
2. Populate region types and color maps
3. Build DetectionResult:
   - regions: List of DetectedRegion
   - inferredGroups: List of DetectedGroup
   - totalDetected: Count
   - confidence counts
   - regionTypes: Map of region names to types
   - suggestedColors: Map of group names to colors

**Output:** Complete DetectionResult ready for UI display

---

## Key Principles

1. **OCR is Hints Only:** Text extraction provides coordinate hints, never direct detection targets
2. **Empty Fields Only:** Multiple layers of text filtering ensure we only detect empty form fields
3. **Consensus is King:** Regions detected by both pipelines get highest confidence
4. **Overfitting is Intentional:** We overfit to capture full boundaries, then refine
5. **Multi-Layer Filtering:** Text filtering happens at 3+ stages for reliability
6. **Fallback Mechanisms:** OCR-only regions included if no rectangle match (better than nothing)

---

## Algorithm Complexity

- **Time Complexity:** O(n*m + k) where:
  - n = number of OCR hints
  - m = average search area size per hint
  - k = number of rectangles detected
- **Space Complexity:** O(n + k) for storing regions
- **Parallelization:** Rectangle detection can run concurrently (future enhancement)

---

## Tuning Parameters

Key parameters that can be adjusted:

- **Overfitting:** 40% horizontal, 60% vertical (can be increased for more coverage)
- **Matching Thresholds:** IoU 0.3, overlap 0.4 (lower = more matches)
- **Text Filtering:** 10% OCR overlap, 0.7 brightness, 0.08 edge density (stricter = fewer false positives)
- **Rectangle Sensitivity:** 0.15 rectangularity threshold (lower = more sensitive)
- **Size Limits:** 15x10 min, 800x300 max (adjust based on document type)
