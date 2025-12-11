# OCR-First Detection Implementation Plan

## Executive Summary

This plan implements a two-stage OCR-first detection pipeline that dramatically reduces false positives (from ~890 to ~20-50 high-confidence regions) by using OCR as the semantic seed, then refining with computer vision techniques.

## Research Synthesis

Based on industry best practices and research:

1. **Hybrid OCR+CV Approach**: Combining OCR with CV refinement is the industry standard (PyImageSearch, MDPI research)
2. **Multi-Stage Pipelines**: Preprocessing → Segmentation → Classification improves accuracy
3. **Confidence-Weighted Scoring**: Combining OCR confidence + CV detection quality
4. **Hierarchical Clustering**: Spatial grouping of OCR regions before refinement
5. **Pattern Recognition**: Analyzing checkbox placement patterns across multiple lines
6. **Adaptive Thresholding**: Better than fixed thresholds for varying lighting
7. **Line Intersection Refinement**: Using Hough transforms + intersection analysis for grids

## Phase 1: OCR Integration Foundation

### 1.1 Tesseract OCR Integration
**Files**: 
- `Optical Wizard Doctor/src/utils/OcrTextExtractor.h`
- `Optical Wizard Doctor/src/utils/OcrTextExtractor.cpp`

**Key Methods**:
- `extractTextRegions(const QImage& image)` - Main extraction method
- `preprocessImage(const QImage& image)` - Image preprocessing for OCR
- `configureTesseract(tesseract::TessBaseAPI* api)` - Setup PSM, language, etc.
- `filterByConfidence(const QList<OCRTextRegion>& regions, double minConf)` - Filter low-confidence

**Data Structure**:
```cpp
struct OCRTextRegion {
    QString text;              // Extracted text
    cv::Rect boundingBox;     // Text bounding box (pixels)
    NormalizedCoords coords;   // Normalized coordinates
    double confidence;         // OCR confidence (0.0-100.0)
    QString typeHint;          // "number", "letter", "mixed", "unknown"
    int blockId;               // OCR block/paragraph ID
    int lineId;                // OCR line ID within block
};
```

**Implementation Details**:
- Use Tesseract C++ API (`tesseract/baseapi.h`)
- Set PSM mode: `PSM_AUTO` for general forms, `PSM_6` for uniform blocks
- Extract at word level (`RIL_WORD`) for precise bounding boxes
- Apply preprocessing: grayscale, noise reduction, adaptive thresholding
- Filter results: minimum confidence 50%, minimum text length 1 character

### 1.2 Image Preprocessing for OCR
**File**: `Optical Wizard Doctor/src/utils/OcrTextExtractor.cpp`

**Preprocessing Pipeline**:
1. **Grayscale Conversion**: `cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY)`
2. **Noise Reduction**: `cv::GaussianBlur(gray, blurred, cv::Size(3,3), 0)`
3. **Adaptive Thresholding**: `cv::adaptiveThreshold(blurred, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2)`
4. **Deskewing** (optional): Detect and correct rotation
5. **Resize** (if needed): Ensure minimum 300 DPI equivalent

**CMake Integration**:
- Add Tesseract dependency: `find_package(PkgConfig REQUIRED)`
- `pkg_check_modules(TESSERACT REQUIRED tesseract)`
- Link: `target_link_libraries(ocr-orc PRIVATE ${TESSERACT_LIBRARIES})`

## Phase 2: Text Region Refinement

### 2.1 Text Region Refiner
**Files**:
- `Optical Wizard Doctor/src/utils/TextRegionRefiner.h`
- `Optical Wizard Doctor/src/utils/TextRegionRefiner.h`

**Key Methods**:
- `refineRegion(const OCRTextRegion& ocrRegion, const cv::Mat& image)` - Main refinement
- `detectLinesAroundText(const cv::Rect& textBox, const cv::Mat& image)` - Line detection
- `detectRectanglesAroundText(const cv::Rect& textBox, const cv::Mat& image)` - Rectangle detection
- `mergeDetections(const cv::Rect& textBox, const QList<cv::Rect>& lines, const QList<cv::Rect>& rects)` - Merge into final box
- `expandSearchRadius(const cv::Rect& textBox, int radiusPercent)` - Expand search area

**Refinement Algorithm**:
1. **Expand Search Area**: Create ROI around text (expand by 10-30% of text height)
2. **Horizontal Line Detection**:
   - Use morphological operations: `cv::morphologyEx` with horizontal kernel
   - Detect lines above/below text (form field underlines)
   - Detect lines left/right of text (form field borders)
3. **Rectangle Detection**:
   - Find contours in expanded ROI
   - Filter by rectangularity (area ratio > 0.8)
   - Filter by size (reasonable form field dimensions)
4. **Merge Strategy**:
   - Combine text box + detected lines/rectangles
   - Use union of all bounding boxes
   - Ensure minimum size constraints

### 2.2 Line Detection Enhancement
**File**: `Optical Wizard Doctor/src/utils/TextRegionRefiner.cpp`

**Techniques**:
- **Morphological Line Detection**: 
  ```cpp
  cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(25, 1));
  cv::Mat verticalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 25));
  cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel);
  ```
- **Hough Line Transform**: For precise line detection
- **Line Merging**: Merge nearby parallel lines (within 5px tolerance)

## Phase 3: Checkbox Detection & Pattern Recognition

### 3.1 Checkbox Detector
**Files**:
- `Optical Wizard Doctor/src/utils/CheckboxDetector.h`
- `Optical Wizard Doctor/src/utils/CheckboxDetector.cpp`

**Key Methods**:
- `detectCheckbox(const OCRTextRegion& textRegion, const cv::Mat& image)` - Detect checkbox near text
- `isCheckboxShape(const cv::Rect& rect, double aspectRatio)` - Validate square shape
- `determinePlacement(const cv::Rect& checkbox, const cv::Rect& text)` - Left or right?
- `detectCheckboxState(const cv::Mat& checkboxROI)` - Checked or unchecked?

**Detection Algorithm**:
1. **Search Areas**: 
   - Left side: Rectangle to left of text (width: 5-20% of text height, height: text height)
   - Right side: Rectangle to right of text (same dimensions)
2. **Square Detection**:
   - Find contours in search areas
   - Filter by aspect ratio (0.8-1.2 for squares)
   - Filter by size (10-30px typical checkbox size)
   - Filter by rectangularity (area ratio > 0.7)
3. **Proximity Validation**:
   - Checkbox must be within 2x text height distance
   - Aligned vertically with text (Y-center within 30% of text height)

### 3.2 Pattern Analyzer
**Files**:
- `Optical Wizard Doctor/src/utils/PatternAnalyzer.h`
- `Optical Wizard Doctor/src/utils/PatternAnalyzer.cpp`

**Key Methods**:
- `analyzeCheckboxPattern(const QList<OCRTextRegion>& regions)` - Determine left/right pattern
- `groupByHorizontalAlignment(const QList<OCRTextRegion>& regions)` - Group into rows
- `determineRowPattern(const QList<OCRTextRegion>& row)` - Analyze single row
- `applyPattern(const QList<OCRTextRegion>& regions, const QString& pattern)` - Apply to ambiguous cases
- `detectMultiItemLines(const QList<OCRTextRegion>& regions)` - Detect sequences

**Pattern Detection Algorithm**:
```
1. Group OCR regions by horizontal alignment (Y-coordinate clustering, tolerance: 2% of page height)
2. Sort each group by X-coordinate
3. For each horizontal line:
   a. Analyze first 3 items and last 3 items
   b. Count checkboxes on left vs right
   c. If >70% on left → pattern = "checkbox_left"
   d. If >70% on right → pattern = "checkbox_right"
   e. If mixed → use majority voting across all lines
4. Apply pattern to ambiguous cases:
   - If checkbox placement unclear, use detected pattern
   - If no pattern detected, use proximity (closer to text start = left)
```

**Multi-Item Detection**:
- **Horizontal Clustering**: Group items with gaps < 5% of page width
- **Sequence Detection**: Identify patterns like "☐ A ☐ B ☐ C"
- **Group Assignment**: Items in same sequence → same group

## Phase 4: Form Field Detection

### 4.1 Form Field Detector
**Files**:
- `Optical Wizard Doctor/src/utils/FormFieldDetector.h`
- `Optical Wizard Doctor/src/utils/FormFieldDetector.cpp`

**Key Methods**:
- `detectFormField(const OCRTextRegion& textRegion, const cv::Mat& image)` - Detect form input field
- `detectTextInputField(const OCRTextRegion& textRegion, const cv::Mat& image)` - Text input detection
- `detectMultiLineTextArea(const QList<OCRTextRegion>& regions)` - Multi-line areas
- `detectGridCells(const QList<OCRTextRegion>& regions)` - Grid/table cells

**Form Field Types**:
1. **Text Input**: Horizontal line below text (underline) OR rectangle around text
2. **Multi-line Text Area**: Multiple vertically aligned text regions + bounding rectangle
3. **Grid Cell**: Part of detected grid structure
4. **Checkbox Field**: Checkbox + associated text

### 4.2 Grid/Table Detection Enhancement
**File**: `Optical Wizard Doctor/src/utils/GridDetector.h/cpp`

**Enhancement to Existing Grid Detection**:
- Use OCR regions as **seed points** for grid detection
- **Line Intersection Refinement**: 
  - Detect horizontal/vertical lines
  - Find intersections
  - Align OCR regions to nearest intersection
- **Cell Merging**: Merge OCR regions that belong to same grid cell

## Phase 5: Confidence Scoring & Filtering

### 5.1 Multi-Factor Confidence Calculator
**Files**:
- `Optical Wizard Doctor/src/utils/ConfidenceCalculator.h`
- `Optical Wizard Doctor/src/utils/ConfidenceCalculator.cpp`

**Confidence Formula**:
```cpp
double confidence = 
    (ocrConfidence / 100.0 * 0.4) +           // OCR quality (40%)
    (lineDetectionScore * 0.3) +             // CV line detection (30%)
    (rectangularityScore * 0.2) +            // How rectangular (20%)
    (patternMatchScore * 0.1);               // Pattern match (10%)
```

**Scoring Components**:
- **OCR Confidence**: Direct from Tesseract (0-100, normalized to 0-1)
- **Line Detection Score**: 
  - Number of detected lines around text (0-4 lines)
  - Line strength (edge pixel density)
  - Score: `(lineCount / 4.0) * lineStrength`
- **Rectangularity Score**:
  - How close detected box is to perfect rectangle
  - Aspect ratio appropriateness
  - Score: `rectangularity * aspectRatioScore`
- **Pattern Match Score**:
  - Matches detected checkbox pattern? (1.0 if yes, 0.5 if ambiguous)
  - Part of detected sequence? (1.0 if yes, 0.7 if isolated)

### 5.2 Adaptive Filtering
**File**: `Optical Wizard Doctor/src/utils/ConfidenceCalculator.cpp`

**Filtering Strategy**:
- **High Confidence (>= 0.7)**: Always include
- **Medium Confidence (0.5-0.7)**: 
  - Include if matches detected pattern
  - Include if part of sequence/grid
  - Exclude if isolated and low OCR confidence
- **Low Confidence (< 0.5)**: Exclude (unless user explicitly requests)

## Phase 6: Integration with Existing System

### 6.1 Enhanced RegionDetector
**File**: `Optical Wizard Doctor/src/utils/RegionDetector.cpp`

**New Detection Method**:
```cpp
DetectionResult detectRegionsOCRFirst(const QImage& image, const QString& method) {
    // Stage 1: OCR Extraction
    OcrTextExtractor extractor;
    QList<OCRTextRegion> ocrRegions = extractor.extractTextRegions(image);
    
    // Stage 2: Pattern Analysis
    PatternAnalyzer patternAnalyzer;
    QString checkboxPattern = patternAnalyzer.analyzeCheckboxPattern(ocrRegions);
    QList<QList<OCRTextRegion>> horizontalGroups = patternAnalyzer.groupByHorizontalAlignment(ocrRegions);
    
    // Stage 3: CV Refinement
    TextRegionRefiner refiner;
    CheckboxDetector checkboxDetector;
    FormFieldDetector formFieldDetector;
    
    QList<DetectedRegion> refinedRegions;
    for (const OCRTextRegion& ocrRegion : ocrRegions) {
        // Refine text region
        cv::Mat cvImage = ImageConverter::qImageToMat(image);
        NormalizedCoords refinedCoords = refiner.refineRegion(ocrRegion, cvImage);
        
        // Detect checkbox if applicable
        CheckboxDetection checkbox = checkboxDetector.detectCheckbox(ocrRegion, cvImage);
        
        // Detect form field type
        FormFieldType fieldType = formFieldDetector.detectFormField(ocrRegion, cvImage);
        
        // Calculate confidence
        double confidence = ConfidenceCalculator::calculate(
            ocrRegion.confidence,
            refiner.getLineDetectionScore(),
            refiner.getRectangularityScore(),
            patternAnalyzer.getPatternMatchScore(ocrRegion, checkboxPattern)
        );
        
        // Create DetectedRegion
        DetectedRegion region;
        region.coords = refinedCoords;
        region.confidence = confidence;
        region.inferredType = ocrRegion.typeHint;
        // ... assign group, color based on pattern
        
        refinedRegions.append(region);
    }
    
    // Stage 4: Group Inference
    GroupInferencer groupInferencer;
    QList<DetectedGroup> groups = groupInferencer.inferGroupsFromOCR(ocrRegions, refinedRegions);
    
    // Build result
    DetectionResult result;
    result.regions = refinedRegions;
    result.inferredGroups = groups;
    // ... populate other fields
    
    return result;
}
```

### 6.2 UI Integration
**File**: `Optical Wizard Doctor/src/ui/MainWindow.cpp`

**Changes**:
- Add detection mode selector: "OCR-First" vs "CV-Only" (for comparison)
- Update `onMagicDetect()` to use OCR-first method
- Show OCR confidence in preview dialog
- Display detected patterns (checkbox left/right)

## Phase 7: Advanced Features

### 7.1 Multi-Line Text Area Detection
**File**: `Optical Wizard Doctor/src/utils/FormFieldDetector.cpp`

**Algorithm**:
1. Group OCR regions by vertical alignment (X-coordinate clustering)
2. Detect bounding rectangle around grouped regions
3. Look for border lines around rectangle
4. Merge into single multi-line text area region

### 7.2 Sequence Detection
**File**: `Optical Wizard Doctor/src/utils/PatternAnalyzer.cpp`

**Sequence Types**:
- **Name Fields**: Horizontal sequence of letter-type regions
- **Number Sequences**: Horizontal sequence of number-type regions
- **Postal Codes**: 6-region pattern (already implemented, enhance with OCR)
- **Checkbox Groups**: Multiple checkboxes in a row

### 7.3 Adaptive Preprocessing
**File**: `Optical Wizard Doctor/src/utils/OcrTextExtractor.cpp`

**Adaptive Techniques**:
- **Lighting Analysis**: Detect if image is too dark/bright, adjust accordingly
- **Noise Level Detection**: Measure noise, apply appropriate filtering
- **Resolution Check**: Ensure minimum 300 DPI equivalent
- **Skew Detection**: Auto-deskew if rotation > 2 degrees

## Implementation Order

### Sprint 1: OCR Foundation (Week 1)
1. Integrate Tesseract OCR
2. Implement `OcrTextExtractor` with basic extraction
3. Add image preprocessing pipeline
4. Test OCR extraction on sample forms

### Sprint 2: Text Refinement (Week 2)
1. Implement `TextRegionRefiner`
2. Add line detection around text
3. Add rectangle detection
4. Merge detections into refined bounding boxes

### Sprint 3: Checkbox Detection (Week 3)
1. Implement `CheckboxDetector`
2. Add pattern analysis
3. Implement left/right pattern detection
4. Test on forms with checkboxes

### Sprint 4: Form Field Detection (Week 4)
1. Implement `FormFieldDetector`
2. Add multi-line text area detection
3. Enhance grid detection with OCR seeds
4. Integrate with existing detection system

### Sprint 5: Confidence & Integration (Week 5)
1. Implement `ConfidenceCalculator`
2. Add adaptive filtering
3. Integrate OCR-first method into `RegionDetector`
4. Update UI for OCR-first mode
5. Testing and refinement

## Key Design Decisions

1. **OCR as Seed**: OCR provides semantic understanding, CV refines boundaries
2. **Pattern-First Approach**: Analyze patterns across multiple lines before individual detection
3. **Confidence-Weighted**: Combine multiple confidence sources for accuracy
4. **Hierarchical Processing**: Process at multiple levels (character → word → line → field)
5. **Adaptive Thresholds**: Use adaptive methods instead of fixed values
6. **Progressive Refinement**: Each stage refines the previous stage's output

## Testing Strategy

1. **Unit Tests**: Each component (OCR extraction, refinement, checkbox detection)
2. **Integration Tests**: Full pipeline on sample forms
3. **Comparison Tests**: OCR-first vs CV-only on same forms
4. **Accuracy Metrics**: 
   - Precision: % of detected regions that are correct
   - Recall: % of actual regions detected
   - False Positive Rate: Should drop from ~890 to <50

## Expected Outcomes

- **Dramatic reduction in false positives**: 890 → 20-50 regions
- **Higher accuracy**: OCR provides semantic context
- **Better checkbox handling**: Pattern recognition across lines
- **Improved form understanding**: Detects form structure, not just shapes
- **User confidence**: High-confidence regions are actually useful

## Dependencies

- **Tesseract OCR**: `brew install tesseract` (macOS)
- **Tesseract C++ API**: Available via system libraries
- **Leptonica**: Image processing library (Tesseract dependency)
- **OpenCV**: Already integrated

## Risk Mitigation

1. **OCR Accuracy**: Use preprocessing + confidence filtering
2. **Performance**: OCR can be slow → run in background thread (already implemented)
3. **Language Support**: Tesseract supports 100+ languages
4. **Handwritten Text**: OCR-first approach works best for printed text (acceptable for forms)
