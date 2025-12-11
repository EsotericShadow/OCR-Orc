# OCR-First Detection System - Detailed Implementation Plan

## Problem Analysis

Current detection produces ~890 low-confidence regions, making it unusable. The pure computer vision approach lacks semantic understanding and is too sensitive to noise.

**Root Cause**: CV-only detection treats all shapes equally, without understanding what's actually text vs noise.

**Solution**: OCR-first approach where:
1. OCR extracts text (semantic understanding)
2. CV refines boundaries around detected text
3. Pattern analysis understands form structure
4. Confidence scoring filters false positives

## Research-Backed Approach

Based on industry research (PyImageSearch, MDPI, academic papers):

1. **Hybrid OCR+CV**: Industry standard for form processing
2. **Multi-stage pipelines**: Preprocessing → OCR → Refinement → Classification
3. **Confidence-weighted ensemble**: Combine multiple confidence sources
4. **Hierarchical clustering**: Group before refining
5. **Pattern recognition**: Analyze across multiple lines for consistency
6. **Adaptive thresholds**: Better than fixed values
7. **Morphological operations**: Effective for line detection

## Phase 1: OCR Integration Foundation

### 1.1 Tesseract OCR Integration
**Objective**: Extract all text from form with bounding boxes and confidence scores

**Files to Create**:
- `Optical Wizard Doctor/src/utils/OcrTextExtractor.h`
- `Optical Wizard Doctor/src/utils/OcrTextExtractor.cpp`

**Data Structure**:
```cpp
struct OCRTextRegion {
    QString text;              // Extracted text content
    cv::Rect boundingBox;     // Bounding box in pixels
    NormalizedCoords coords;   // Normalized coordinates (0.0-1.0)
    double confidence;         // OCR confidence (0.0-100.0)
    QString typeHint;          // "number", "letter", "mixed", "unknown"
    int blockId;               // OCR block/paragraph ID
    int lineId;                // OCR line ID within block
    int wordId;                // Word ID within line
};
```

**Implementation Steps**:

1. **Add Tesseract to CMakeLists.txt**:
   ```cmake
   # Find Tesseract
   find_package(PkgConfig REQUIRED)
   pkg_check_modules(TESSERACT REQUIRED tesseract)
   
   # Link Tesseract
   target_link_libraries(ocr-orc PRIVATE ${TESSERACT_LIBRARIES})
   target_include_directories(ocr-orc PRIVATE ${TESSERACT_INCLUDE_DIRS})
   ```

2. **Implement OcrTextExtractor class**:
   - `extractTextRegions(const QImage& image)` - Main extraction
   - `preprocessImage(const QImage& image)` - Preprocessing pipeline
   - `configureTesseract(tesseract::TessBaseAPI* api)` - Setup
   - `filterByConfidence(const QList<OCRTextRegion>& regions, double minConf)` - Filter
   - `inferTypeFromText(const QString& text)` - Type inference

3. **Tesseract Integration Code**:
   ```cpp
   // Initialize
   tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
   if (api->Init(NULL, "eng")) {
       // Error handling
   }
   
   // Set image
   Pix* image = pixRead(imagePath);
   api->SetImage(image);
   
   // Configure
   api->SetPageSegMode(tesseract::PSM_AUTO);
   
   // Recognize
   api->Recognize(0);
   
   // Extract words with bounding boxes
   tesseract::ResultIterator* ri = api->GetIterator();
   tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
   
   while (ri->Next(level)) {
       const char* word = ri->GetUTF8Text(level);
       float conf = ri->Confidence(level);
       int x1, y1, x2, y2;
       ri->BoundingBox(level, &x1, &y1, &x2, &y2);
       // Create OCRTextRegion
   }
   ```

4. **Preprocessing Pipeline**:
   - Convert to grayscale
   - Apply Gaussian blur (3x3 kernel) for noise reduction
   - Apply adaptive thresholding (block size 11, C=2)
   - Optional: Deskew if rotation > 2 degrees
   - Ensure minimum 300 DPI equivalent

5. **Filtering**:
   - Minimum confidence: 50%
   - Minimum text length: 1 character
   - Filter empty/whitespace-only results

### 1.2 Type Inference from OCR Text
**File**: `Optical Wizard Doctor/src/utils/OcrTextExtractor.cpp`

**Algorithm**:
- **Numbers**: Contains only digits (0-9) → "numbers"
- **Letters**: Contains only letters (A-Z, a-z) → "letters"
- **Mixed**: Contains both → "mixed"
- **Unknown**: Empty or special characters only → "unknown"

## Phase 2: Text Region Refinement

### 2.1 Text Region Refiner
**Objective**: Expand OCR text bounding boxes to include form field boundaries

**Files to Create**:
- `Optical Wizard Doctor/src/utils/TextRegionRefiner.h`
- `Optical Wizard Doctor/src/utils/TextRegionRefiner.cpp`

**Key Methods**:
- `refineRegion(const OCRTextRegion& ocrRegion, const cv::Mat& image)` - Main refinement
- `expandSearchRadius(const cv::Rect& textBox, int radiusPercent)` - Expand search area
- `detectLinesAroundText(const cv::Rect& textBox, const cv::Mat& image)` - Line detection
- `detectRectanglesAroundText(const cv::Rect& textBox, const cv::Mat& image)` - Rectangle detection
- `mergeDetections(const cv::Rect& textBox, const QList<cv::Rect>& lines, const QList<cv::Rect>& rects)` - Merge
- `getLineDetectionScore()` - Return line detection quality (0.0-1.0)
- `getRectangularityScore()` - Return rectangularity (0.0-1.0)

**Refinement Algorithm**:

1. **Expand Search Area**:
   ```cpp
   int expandBy = textBox.height * 0.2; // 20% of text height
   cv::Rect searchArea(
       textBox.x - expandBy,
       textBox.y - expandBy,
       textBox.width + expandBy * 2,
       textBox.height + expandBy * 2
   );
   // Clamp to image bounds
   ```

2. **Horizontal Line Detection** (Morphological):
   ```cpp
   // Create horizontal kernel
   cv::Mat horizontalKernel = cv::getStructuringElement(
       cv::MORPH_RECT, 
       cv::Size(25, 1)  // 25px wide, 1px tall
   );
   
   // Detect horizontal lines
   cv::Mat horizontalLines;
   cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel, iterations=2);
   
   // Find lines above text (within search area)
   // Find lines below text (within search area)
   ```

3. **Vertical Line Detection**:
   ```cpp
   cv::Mat verticalKernel = cv::getStructuringElement(
       cv::MORPH_RECT,
       cv::Size(1, 25)  // 1px wide, 25px tall
   );
   cv::morphologyEx(binary, verticalLines, cv::MORPH_OPEN, verticalKernel, iterations=2);
   ```

4. **Rectangle Detection**:
   ```cpp
   // Find contours in search area
   std::vector<std::vector<cv::Point>> contours;
   cv::findContours(roi, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
   
   for (const auto& contour : contours) {
       cv::Rect rect = cv::boundingRect(contour);
       
       // Filter by rectangularity
       double area = cv::contourArea(contour);
       double rectArea = rect.width * rect.height;
       double rectangularity = area / rectArea;
       
       if (rectangularity > 0.8) {  // Close to perfect rectangle
           rectangles.append(rect);
       }
   }
   ```

5. **Merge Strategy**:
   - Start with text bounding box
   - Expand to include detected lines (extend to line endpoints)
   - Expand to include detected rectangles (union with rectangle)
   - Ensure minimum size (at least text size)
   - Clamp to image bounds

### 2.2 Line Detection Scoring
**File**: `Optical Wizard Doctor/src/utils/TextRegionRefiner.cpp`

**Scoring Method**:
- Count detected lines (0-4: above, below, left, right)
- Measure line strength (edge pixel density)
- Score: `(lineCount / 4.0) * averageLineStrength`

## Phase 3: Checkbox Detection & Pattern Recognition

### 3.1 Checkbox Detector
**Objective**: Detect checkboxes near text regions and determine left/right placement

**Files to Create**:
- `Optical Wizard Doctor/src/utils/CheckboxDetector.h`
- `Optical Wizard Doctor/src/utils/CheckboxDetector.cpp`

**Data Structure**:
```cpp
struct CheckboxDetection {
    cv::Rect boundingBox;      // Checkbox bounding box
    bool detected;              // Was checkbox found?
    QString placement;          // "left", "right", "unknown"
    bool isChecked;             // Checked or unchecked?
    double confidence;          // Detection confidence
};
```

**Key Methods**:
- `detectCheckbox(const OCRTextRegion& textRegion, const cv::Mat& image)` - Main detection
- `searchLeftSide(const cv::Rect& textBox, const cv::Mat& image)` - Search left
- `searchRightSide(const cv::Rect& textBox, const cv::Mat& image)` - Search right
- `isCheckboxShape(const cv::Rect& rect, double aspectRatio)` - Validate square
- `detectCheckboxState(const cv::Mat& checkboxROI)` - Checked/unchecked
- `determinePlacement(const cv::Rect& checkbox, const cv::Rect& text)` - Left or right

**Detection Algorithm**:

1. **Define Search Areas**:
   ```cpp
   // Left side search area
   int searchWidth = textBox.height * 0.15;  // 15% of text height
   int searchHeight = textBox.height * 1.2;  // 120% of text height
   
   cv::Rect leftSearchArea(
       textBox.x - searchWidth - 10,  // 10px gap
       textBox.y - (searchHeight - textBox.height) / 2,
       searchWidth,
       searchHeight
   );
   
   // Right side search area (similar)
   cv::Rect rightSearchArea(
       textBox.x + textBox.width + 10,
       textBox.y - (searchHeight - textBox.height) / 2,
       searchWidth,
       searchHeight
   );
   ```

2. **Square Detection in Search Areas**:
   ```cpp
   // Find contours in search area
   std::vector<std::vector<cv::Point>> contours;
   cv::findContours(roi, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
   
   for (const auto& contour : contours) {
       cv::Rect rect = cv::boundingRect(contour);
       
       // Check aspect ratio (square: 0.8-1.2)
       double aspectRatio = (double)rect.width / rect.height;
       if (aspectRatio < 0.8 || aspectRatio > 1.2) continue;
       
       // Check size (typical checkbox: 10-30px)
       if (rect.width < 10 || rect.width > 30) continue;
       if (rect.height < 10 || rect.height > 30) continue;
       
       // Check rectangularity
       double area = cv::contourArea(contour);
       double rectArea = rect.width * rect.height;
       if (area / rectArea < 0.7) continue;  // Not rectangular enough
       
       // Valid checkbox found
   }
   ```

3. **Proximity Validation**:
   - Checkbox center must be within 2x text height distance from text
   - Y-center alignment: within 30% of text height
   - Prefer closer checkbox if multiple found

4. **State Detection** (Checked/Unchecked):
   ```cpp
   // Extract checkbox ROI
   cv::Mat checkboxROI = image(checkboxRect);
   
   // Calculate fill percentage (black pixels)
   cv::Mat binary;
   cv::threshold(checkboxROI, binary, 127, 255, cv::THRESH_BINARY_INV);
   int blackPixels = cv::countNonZero(binary);
   int totalPixels = checkboxROI.rows * checkboxROI.cols;
   double fillPercentage = (double)blackPixels / totalPixels;
   
   // If >30% filled, likely checked
   bool isChecked = fillPercentage > 0.3;
   ```

### 3.2 Pattern Analyzer
**Objective**: Analyze checkbox placement patterns across multiple lines to determine form structure

**Files to Create**:
- `Optical Wizard Doctor/src/utils/PatternAnalyzer.h`
- `Optical Wizard Doctor/src/utils/PatternAnalyzer.cpp`

**Key Methods**:
- `analyzeCheckboxPattern(const QList<OCRTextRegion>& regions, const QList<CheckboxDetection>& checkboxes)` - Main analysis
- `groupByHorizontalAlignment(const QList<OCRTextRegion>& regions, double tolerance)` - Group into rows
- `determineRowPattern(const QList<OCRTextRegion>& row, const QList<CheckboxDetection>& checkboxes)` - Analyze row
- `applyPatternToAmbiguous(const QList<OCRTextRegion>& regions, const QString& pattern)` - Apply pattern
- `detectMultiItemLines(const QList<OCRTextRegion>& regions)` - Detect sequences
- `getPatternMatchScore(const OCRTextRegion& region, const QString& pattern)` - Scoring

**Pattern Detection Algorithm**:

1. **Horizontal Grouping**:
   ```cpp
   // Sort by Y-coordinate
   QList<OCRTextRegion> sorted = regions;
   std::sort(sorted.begin(), sorted.end(), 
       [](const OCRTextRegion& a, const OCRTextRegion& b) {
           return (a.boundingBox.y + a.boundingBox.height/2) < 
                  (b.boundingBox.y + b.boundingBox.height/2);
       });
   
   // Cluster by Y-coordinate (tolerance: 2% of image height)
   double tolerance = imageHeight * 0.02;
   QList<QList<OCRTextRegion>> rows;
   
   for (const OCRTextRegion& region : sorted) {
       double regionY = region.boundingBox.y + region.boundingBox.height/2;
       bool added = false;
       
       for (auto& row : rows) {
           double rowY = row[0].boundingBox.y + row[0].boundingBox.height/2;
           if (std::abs(regionY - rowY) < tolerance) {
               row.append(region);
               added = true;
               break;
           }
       }
       
       if (!added) {
           rows.append({region});
       }
   }
   ```

2. **Pattern Analysis Per Row**:
   ```cpp
   for (auto& row : rows) {
       // Sort by X-coordinate
       std::sort(row.begin(), row.end(),
           [](const OCRTextRegion& a, const OCRTextRegion& b) {
               return a.boundingBox.x < b.boundingBox.x;
           });
       
       // Analyze first 3 and last 3 items
       int leftCount = 0, rightCount = 0;
       
       for (int i = 0; i < std::min(3, row.size()); ++i) {
           CheckboxDetection cb = findCheckboxForRegion(row[i], checkboxes);
           if (cb.detected && cb.placement == "left") leftCount++;
           if (cb.detected && cb.placement == "right") rightCount++;
       }
       
       for (int i = std::max(0, (int)row.size()-3); i < row.size(); ++i) {
           CheckboxDetection cb = findCheckboxForRegion(row[i], checkboxes);
           if (cb.detected && cb.placement == "left") leftCount++;
           if (cb.detected && cb.placement == "right") rightCount++;
       }
       
       // Determine pattern (>70% threshold)
       if (leftCount > rightCount * 2.33) {  // >70% left
           rowPattern = "checkbox_left";
       } else if (rightCount > leftCount * 2.33) {  // >70% right
           rowPattern = "checkbox_right";
       } else {
           rowPattern = "mixed";
       }
   }
   ```

3. **Majority Voting Across Rows**:
   ```cpp
   // Count patterns across all rows
   QMap<QString, int> patternCounts;
   for (const QString& pattern : rowPatterns) {
       patternCounts[pattern]++;
   }
   
   // Find majority pattern
   QString majorityPattern = "unknown";
   int maxCount = 0;
   for (auto it = patternCounts.begin(); it != patternCounts.end(); ++it) {
       if (it.value() > maxCount) {
           maxCount = it.value();
           majorityPattern = it.key();
       }
   }
   
   // Apply to ambiguous cases
   if (majorityPattern != "unknown" && majorityPattern != "mixed") {
       applyPatternToAmbiguous(regions, majorityPattern);
   }
   ```

4. **Multi-Item Line Detection**:
   ```cpp
   // For each row, detect sequences
   for (auto& row : rows) {
       // Calculate gaps between items
       QList<double> gaps;
       for (int i = 0; i < row.size() - 1; ++i) {
           double gap = row[i+1].boundingBox.x - 
                       (row[i].boundingBox.x + row[i].boundingBox.width);
           gaps.append(gap);
       }
       
       // If gaps are small (< 5% of page width), it's a sequence
       double maxGap = imageWidth * 0.05;
       bool isSequence = true;
       for (double gap : gaps) {
           if (gap > maxGap) {
               isSequence = false;
               break;
           }
       }
       
       if (isSequence) {
           // Assign same group to all items in sequence
       }
   }
   ```

## Phase 4: Form Field Detection

### 4.1 Form Field Detector
**Objective**: Classify detected regions into form field types

**Files to Create**:
- `Optical Wizard Doctor/src/utils/FormFieldDetector.h`
- `Optical Wizard Doctor/src/utils/FormFieldDetector.cpp`

**Form Field Types**:
```cpp
enum class FormFieldType {
    TextInput,      // Single-line text input (underline or box)
    MultiLineText,  // Multi-line text area
    CheckboxField,  // Checkbox + label
    GridCell,       // Part of grid/table
    Label,          // Just a label (no input field)
    Unknown
};
```

**Key Methods**:
- `detectFormField(const OCRTextRegion& textRegion, const cv::Mat& image)` - Main detection
- `detectTextInputField(const OCRTextRegion& textRegion, const cv::Mat& image)` - Text input
- `detectMultiLineTextArea(const QList<OCRTextRegion>& regions)` - Multi-line areas
- `detectGridCells(const QList<OCRTextRegion>& regions)` - Grid cells

**Detection Logic**:

1. **Text Input Field**:
   - Check for horizontal line below text (within 5px)
   - Check for rectangle around text
   - If either found → TextInput

2. **Multi-Line Text Area**:
   - Group vertically aligned OCR regions (same X-range)
   - If 3+ regions vertically aligned → MultiLineText
   - Detect bounding rectangle around group
   - Look for border lines

3. **Checkbox Field**:
   - If checkbox detected nearby → CheckboxField

4. **Grid Cell**:
   - If part of detected grid structure → GridCell

### 4.2 Grid Detection Enhancement
**File**: `Optical Wizard Doctor/src/utils/GridDetector.h/cpp` (new or enhance existing)

**Enhancement Strategy**:
- Use OCR regions as **seed points** for grid detection
- Detect lines between OCR regions
- Find line intersections
- Align OCR regions to nearest grid intersection
- Merge OCR regions that belong to same grid cell

## Phase 5: Confidence Scoring & Filtering

### 5.1 Multi-Factor Confidence Calculator
**Files to Create**:
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

1. **OCR Confidence** (0-1.0):
   - Direct from Tesseract (0-100) normalized to 0-1.0
   - Higher OCR confidence = higher overall confidence

2. **Line Detection Score** (0-1.0):
   - Count detected lines (0-4: above, below, left, right)
   - Measure line strength (edge pixel density)
   - Score: `(lineCount / 4.0) * averageLineStrength`

3. **Rectangularity Score** (0-1.0):
   - How close detected box is to perfect rectangle
   - Aspect ratio appropriateness (prefer 0.5-2.0)
   - Score: `rectangularity * aspectRatioScore`

4. **Pattern Match Score** (0-1.0):
   - Matches detected checkbox pattern? (1.0 if yes, 0.5 if ambiguous)
   - Part of detected sequence? (1.0 if yes, 0.7 if isolated)
   - Part of grid? (1.0 if yes, 0.8 if near grid)

### 5.2 Adaptive Filtering
**File**: `Optical Wizard Doctor/src/utils/ConfidenceCalculator.cpp`

**Filtering Strategy**:
- **High Confidence (>= 0.7)**: Always include
- **Medium Confidence (0.5-0.7)**: 
  - Include if matches detected pattern
  - Include if part of sequence/grid
  - Include if OCR confidence > 60%
  - Exclude if isolated and low OCR confidence
- **Low Confidence (< 0.5)**: Exclude (unless user explicitly requests)

## Phase 6: Integration with Existing System

### 6.1 Enhanced RegionDetector
**File**: `Optical Wizard Doctor/src/utils/RegionDetector.cpp`

**New Detection Method**:
```cpp
DetectionResult RegionDetector::detectRegionsOCRFirst(const QImage& image, const QString& method) {
    DetectionResult result;
    result.methodUsed = "ocr-first";
    
    // Stage 1: OCR Extraction
    OcrTextExtractor extractor;
    QList<OCRTextRegion> ocrRegions = extractor.extractTextRegions(image);
    
    if (ocrRegions.isEmpty()) {
        // Fallback to CV-only if OCR fails
        return detectRegions(image, "hybrid");
    }
    
    // Stage 2: Pattern Analysis
    PatternAnalyzer patternAnalyzer;
    QList<CheckboxDetection> checkboxes;
    for (const OCRTextRegion& ocrRegion : ocrRegions) {
        CheckboxDetection cb = checkboxDetector.detectCheckbox(ocrRegion, cvImage);
        if (cb.detected) {
            checkboxes.append(cb);
        }
    }
    
    QString checkboxPattern = patternAnalyzer.analyzeCheckboxPattern(ocrRegions, checkboxes);
    QList<QList<OCRTextRegion>> horizontalGroups = patternAnalyzer.groupByHorizontalAlignment(ocrRegions);
    
    // Stage 3: CV Refinement
    cv::Mat cvImage = ImageConverter::qImageToMat(image);
    TextRegionRefiner refiner;
    FormFieldDetector formFieldDetector;
    
    QList<DetectedRegion> refinedRegions;
    for (const OCRTextRegion& ocrRegion : ocrRegions) {
        // Refine text region
        NormalizedCoords refinedCoords = refiner.refineRegion(ocrRegion, cvImage);
        
        // Find associated checkbox
        CheckboxDetection checkbox = findCheckboxForRegion(ocrRegion, checkboxes);
        
        // Detect form field type
        FormFieldType fieldType = formFieldDetector.detectFormField(ocrRegion, cvImage);
        
        // Calculate confidence
        double confidence = ConfidenceCalculator::calculate(
            ocrRegion.confidence,
            refiner.getLineDetectionScore(),
            refiner.getRectangularityScore(),
            patternAnalyzer.getPatternMatchScore(ocrRegion, checkboxPattern)
        );
        
        // Apply filtering
        if (confidence < 0.5) continue;  // Filter low confidence
        
        // Create DetectedRegion
        DetectedRegion region;
        region.coords = refinedCoords;
        region.confidence = confidence;
        region.inferredType = ocrRegion.typeHint;
        region.method = "ocr-first";
        
        // Assign group and color based on pattern and type
        if (fieldType == FormFieldType::CheckboxField) {
            region.suggestedGroup = "CheckboxGroup";
            region.suggestedColor = "blue";
        } else if (ocrRegion.typeHint == "letters") {
            region.suggestedGroup = patternAnalyzer.inferGroupName(ocrRegion, horizontalGroups);
            region.suggestedColor = "red";  // or "blue" based on pattern
        } else if (ocrRegion.typeHint == "numbers") {
            region.suggestedGroup = "NumberSequence";
            region.suggestedColor = "green";
        }
        
        refinedRegions.append(region);
    }
    
    // Stage 4: Group Inference
    GroupInferencer groupInferencer;
    QList<DetectedGroup> groups = groupInferencer.inferGroupsFromOCR(ocrRegions, refinedRegions);
    
    // Build result
    result.regions = refinedRegions;
    result.totalDetected = refinedRegions.size();
    result.inferredGroups = groups;
    
    // Count confidence levels
    for (const auto& region : refinedRegions) {
        if (region.confidence >= 0.8) {
            result.highConfidence++;
        } else if (region.confidence >= 0.5) {
            result.mediumConfidence++;
        } else {
            result.lowConfidence++;
        }
    }
    
    return result;
}
```

### 6.2 Update RegionDetector Interface
**File**: `Optical Wizard Doctor/src/utils/RegionDetector.h`

**Changes**:
- Add `detectRegionsOCRFirst()` method
- Update `detectRegions()` to support "ocr-first" method
- Add detection mode parameter

### 6.3 UI Integration
**File**: `Optical Wizard Doctor/src/ui/MainWindow.cpp`

**Changes**:
- Add detection mode selector in UI (optional, can default to OCR-first)
- Update `onMagicDetect()` to use OCR-first method
- Show OCR confidence in preview dialog
- Display detected patterns (checkbox left/right) in preview

## Phase 7: Advanced Features

### 7.1 Multi-Line Text Area Detection
**File**: `Optical Wizard Doctor/src/utils/FormFieldDetector.cpp`

**Algorithm**:
1. Group OCR regions by vertical alignment (X-coordinate clustering, tolerance: 2% of page width)
2. If 3+ regions in same vertical group → potential multi-line area
3. Detect bounding rectangle around grouped regions
4. Look for border lines around rectangle
5. Merge into single multi-line text area region

### 7.2 Sequence Detection Enhancement
**File**: `Optical Wizard Doctor/src/utils/PatternAnalyzer.cpp`

**Enhancement**:
- Use OCR text content to validate sequences
- "Firstname" cells → group together
- "Lastname" cells → group together
- Postal code pattern: Validate A#A-#A# format using OCR text

### 7.3 Adaptive Preprocessing
**File**: `Optical Wizard Doctor/src/utils/OcrTextExtractor.cpp`

**Adaptive Techniques**:
- **Lighting Analysis**: 
  ```cpp
  cv::Scalar mean, stddev;
  cv::meanStdDev(gray, mean, stddev);
  if (mean[0] < 100) {  // Too dark
      // Apply brightness adjustment
  } else if (mean[0] > 200) {  // Too bright
      // Apply contrast adjustment
  }
  ```
- **Noise Level Detection**: Measure standard deviation, apply appropriate filtering
- **Resolution Check**: Ensure minimum 300 DPI, upscale if needed
- **Skew Detection**: Use Hough lines to detect rotation, deskew if > 2 degrees

## Implementation Order

### Sprint 1: OCR Foundation
**Tasks**:
1. Add Tesseract dependency to CMakeLists.txt
2. Create `OcrTextExtractor` class
3. Implement basic OCR extraction
4. Add image preprocessing pipeline
5. Test on sample forms
6. Filter by confidence

**Deliverable**: Working OCR extraction with bounding boxes

### Sprint 2: Text Refinement
**Tasks**:
1. Create `TextRegionRefiner` class
2. Implement search area expansion
3. Add morphological line detection
4. Add rectangle detection
5. Implement merge strategy
6. Add scoring methods

**Deliverable**: Refined bounding boxes around OCR text

### Sprint 3: Checkbox Detection
**Tasks**:
1. Create `CheckboxDetector` class
2. Implement left/right search areas
3. Add square detection and validation
4. Implement state detection (checked/unchecked)
5. Test on forms with checkboxes

**Deliverable**: Checkbox detection with placement determination

### Sprint 4: Pattern Analysis
**Tasks**:
1. Create `PatternAnalyzer` class
2. Implement horizontal grouping
3. Add pattern detection algorithm
4. Implement majority voting
5. Add multi-item line detection
6. Test pattern recognition

**Deliverable**: Pattern-aware detection system

### Sprint 5: Form Field Detection
**Tasks**:
1. Create `FormFieldDetector` class
2. Implement text input detection
3. Add multi-line text area detection
4. Enhance grid detection with OCR seeds
5. Integrate field type classification

**Deliverable**: Form field type classification

### Sprint 6: Confidence & Integration
**Tasks**:
1. Create `ConfidenceCalculator` class
2. Implement multi-factor confidence formula
3. Add adaptive filtering
4. Integrate OCR-first method into RegionDetector
5. Update UI for OCR-first mode
6. Comprehensive testing

**Deliverable**: Complete OCR-first detection system

## Testing Strategy

### Unit Tests
- `OcrTextExtractor`: Test OCR extraction on known images
- `TextRegionRefiner`: Test refinement on sample text regions
- `CheckboxDetector`: Test checkbox detection on forms with checkboxes
- `PatternAnalyzer`: Test pattern detection on various layouts
- `ConfidenceCalculator`: Test confidence scoring

### Integration Tests
- Full pipeline on sample forms
- Compare OCR-first vs CV-only results
- Test on various form types (registration, survey, etc.)

### Accuracy Metrics
- **Precision**: % of detected regions that are correct (target: >90%)
- **Recall**: % of actual regions detected (target: >85%)
- **False Positive Rate**: Should drop from ~890 to <50 regions
- **Confidence Distribution**: Most regions should be high-confidence

## Expected Outcomes

- **Dramatic reduction in false positives**: 890 → 20-50 regions
- **Higher accuracy**: OCR provides semantic context
- **Better checkbox handling**: Pattern recognition across lines
- **Improved form understanding**: Detects form structure, not just shapes
- **User confidence**: High-confidence regions are actually useful

## Dependencies

- **Tesseract OCR**: `brew install tesseract` (macOS)
- **Tesseract C++ API**: Available via system libraries
- **Leptonica**: Image processing library (Tesseract dependency, usually auto-installed)
- **OpenCV**: Already integrated

## Risk Mitigation

1. **OCR Accuracy**: 
   - Use preprocessing to improve image quality
   - Filter low-confidence OCR results
   - Fallback to CV-only if OCR fails completely

2. **Performance**: 
   - OCR can be slow → already running in background thread
   - Cache OCR results if same image processed multiple times
   - Consider OCR only on first detection, CV refinement on subsequent

3. **Language Support**: 
   - Tesseract supports 100+ languages
   - Default to English, allow user to specify language

4. **Handwritten Text**: 
   - OCR-first approach works best for printed text
   - Acceptable limitation for form processing (forms are typically printed)

5. **Complex Layouts**: 
   - Pattern analysis handles most common cases
   - User can manually adjust if needed

## Success Criteria

- **False Positives**: < 50 regions (down from ~890)
- **High Confidence Regions**: > 80% of detected regions
- **Accuracy**: > 90% precision, > 85% recall
- **Checkbox Detection**: > 95% accuracy for checkbox placement
- **Performance**: < 5 seconds for typical form (already in background thread)
