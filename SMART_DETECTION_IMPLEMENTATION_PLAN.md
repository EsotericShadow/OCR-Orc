# Smart Detection System - Detailed Implementation Plan

## Overview

Transform the current "too sensitive" detection into a **context-aware, pattern-recognizing, multi-layer smart detection system** that understands:
- Spatial relationships (grids, rows, columns, sequences)
- Type patterns (letters, numbers, mixed)
- Semantic patterns (postal codes, name fields, etc.)
- Grouping logic (related regions form logical units)

---

## Phase 1: Enhanced Spatial Detection with Clustering

### 1.1 Multi-Scale Region Detection
**File**: `src/utils/RegionDetector.cpp`
**Method**: `detectRegions()` - enhance existing

**Changes**:
- Process at 3 scales: 0.5x, 1.0x, 2.0x
- Detect regions at each scale
- Merge results with scale-aware confidence
- Filter duplicates across scales

**Implementation**:
```cpp
DetectionResult detectRegions(const QImage& image, const QString& method) {
    // Detect at multiple scales
    std::vector<DetectionResult> scaleResults;
    for (double scale : {0.5, 1.0, 2.0}) {
        QImage scaledImage = image.scaled(
            image.width() * scale, 
            image.height() * scale,
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation
        );
        DetectionResult result = detectAtScale(scaledImage, method, scale);
        scaleResults.push_back(result);
    }
    return mergeScaleResults(scaleResults, image.size());
}
```

### 1.2 Spatial Clustering
**New File**: `src/utils/SpatialClusterer.h/cpp`

**Purpose**: Group regions by spatial relationships

**Key Methods**:
- `clusterByProximity()` - Group nearby regions
- `clusterByAlignment()` - Group horizontally/vertically aligned regions
- `detectGridStructure()` - Identify grid patterns
- `detectSequentialPattern()` - Identify linear sequences

**Algorithm**:
```cpp
class SpatialClusterer {
    // Group regions by horizontal alignment (same Y-range)
    QList<QList<DetectedRegion>> clusterByHorizontalAlignment(
        const QList<DetectedRegion>& regions,
        double tolerance = 0.01  // 1% of image height
    );
    
    // Group regions by vertical alignment (same X-range)
    QList<QList<DetectedRegion>> clusterByVerticalAlignment(
        const QList<DetectedRegion>& regions,
        double tolerance = 0.01
    );
    
    // Detect grid structure
    GridStructure detectGrid(
        const QList<DetectedRegion>& regions
    );
    
    // Detect sequential pattern (cells in a row/column)
    QList<DetectedRegion> detectSequence(
        const QList<DetectedRegion>& regions,
        double maxGap = 0.02  // Max gap between cells
    );
};
```

### 1.3 Size Consistency Analysis
**File**: `src/utils/RegionDetector.cpp`
**New Method**: `validateSizeConsistency()`

**Purpose**: Filter regions that don't match expected sizes in their context

**Algorithm**:
1. For each detected region, find nearby regions (within distance threshold)
2. Calculate average size of nearby regions
3. If region size deviates significantly (>30%) from average → lower confidence
4. Filter regions with confidence below threshold

---

## Phase 2: Type Inference System

### 2.1 Visual Type Inference
**New File**: `src/utils/TypeInferencer.h/cpp`

**Purpose**: Infer region type (letters/numbers/mixed) from visual characteristics

**Methods**:
- `inferTypeFromVisual()` - Analyze region appearance
- `inferTypeFromOCR()` - Use OCR if text present
- `inferTypeFromPattern()` - Use pattern matching
- `combineTypeInferences()` - Merge multiple signals

**Visual Analysis**:
```cpp
QString inferTypeFromVisual(const cv::Mat& regionImage) {
    // Analyze characteristics:
    // - Aspect ratio (letter fields often wider)
    // - Edge density (number fields might have different edge patterns)
    // - Texture (letter vs number fields might differ)
    // - Size (postal code cells are typically small and square)
    
    // Return: "letters", "numbers", "mixed", "unknown"
}
```

### 2.2 OCR Integration (Optional)
**New File**: `src/utils/OCRTypeDetector.h/cpp`

**Purpose**: Use OCR to validate/infer types when text is present

**Note**: This is optional - can be added later if OCR library is integrated

**Algorithm**:
1. Extract text from detected region
2. Analyze character distribution:
   - All letters → "letters"
   - All numbers → "numbers"
   - Mixed → "mixed" or "alphanumeric"
3. Use for validation/refinement

### 2.3 Pattern-Based Type Inference
**File**: `src/utils/TypeInferencer.cpp`
**Method**: `inferTypeFromPattern()`

**Purpose**: Use known patterns to infer types

**Examples**:
- Canadian postal code pattern → alternating letters/numbers
- Name field pattern → all letters
- Student number pattern → all numbers

---

## Phase 3: Pattern Recognition System

### 3.1 Canadian Postal Code Pattern Detector
**New File**: `src/utils/patterns/PostalCodePatternDetector.h/cpp`

**Purpose**: Detect and validate Canadian postal code patterns (A#A-#A#)

**Key Methods**:
```cpp
class PostalCodePatternDetector {
    // Detect postal code pattern from regions
    PostalCodePattern detectPattern(
        const QList<DetectedRegion>& regions
    );
    
    // Validate pattern matches A#A-#A# format
    bool validatePattern(const PostalCodePattern& pattern);
    
    // Group regions into postal code structure
    QList<DetectedRegion> groupPostalCode(
        const QList<DetectedRegion>& regions
    );
};

struct PostalCodePattern {
    QList<DetectedRegion> group1;  // [L][N][L]
    QList<DetectedRegion> group2;  // [N][L][N]
    DetectedRegion separator;     // Dash or space
    double confidence;
};
```

**Algorithm**:
1. Find 6 regions in close horizontal proximity
2. Detect separator (gap or visual dash)
3. Validate pattern:
   - Group 1: [letter][number][letter]
   - Group 2: [number][letter][number]
4. Assign types to individual cells
5. Create postal code group

### 3.2 Name Field Pattern Detector
**New File**: `src/utils/patterns/NameFieldPatternDetector.h/cpp`

**Purpose**: Detect name field patterns (multiple letter cells in a row)

**Algorithm**:
1. Detect horizontal sequence of regions
2. Validate all are letter type (or inferred as letters)
3. Validate similar sizes
4. Group as name field (Firstname or Lastname)

### 3.3 Number Sequence Pattern Detector
**New File**: `src/utils/patterns/NumberSequencePatternDetector.h/cpp`

**Purpose**: Detect number sequence patterns (e.g., student numbers)

**Algorithm**:
1. Detect horizontal sequence of regions
2. Validate all are number type
3. Validate similar sizes
4. Group as number sequence

### 3.4 Generic Pattern Matcher
**New File**: `src/utils/patterns/PatternMatcher.h/cpp`

**Purpose**: Configurable pattern matching for future patterns

**Design**:
- Pattern definition language (JSON/YAML)
- Support for alternation, repetition, separators
- Extensible for new patterns

---

## Phase 4: Group Inference System

### 4.1 Spatial Grouping
**File**: `src/utils/GroupInferencer.h/cpp`
**Method**: `inferGroupsFromSpatial()`

**Algorithm**:
1. Cluster regions by spatial proximity
2. For each cluster:
   - Check type consistency
   - Check size consistency
   - Check alignment
   - Create group if criteria met

### 4.2 Pattern-Based Grouping
**File**: `src/utils/GroupInferencer.cpp`
**Method**: `inferGroupsFromPatterns()`

**Algorithm**:
1. Run pattern detectors (postal code, name fields, etc.)
2. For each detected pattern:
   - Create group with appropriate name
   - Assign regions to group
   - Suggest color based on pattern type

### 4.3 Color Suggestion
**File**: `src/utils/GroupInferencer.cpp`
**Method**: `suggestColorForGroup()`

**Logic**:
- Name fields → red/blue (alternating for first/last)
- Number sequences → green
- Mixed types (postal codes) → yellow
- Unknown → default color

---

## Phase 5: Context-Aware Filtering

### 5.1 Multi-Layer Validation
**File**: `src/utils/RegionValidator.h/cpp`

**Purpose**: Validate regions against multiple criteria

**Validation Layers**:
1. **Spatial Validation**: Has neighbors? Not isolated?
2. **Size Validation**: Consistent with nearby regions?
3. **Type Validation**: Type makes sense in context?
4. **Pattern Validation**: Fits detected pattern?
5. **Alignment Validation**: Aligned with grid/pattern?

**Algorithm**:
```cpp
double validateRegion(
    const DetectedRegion& region,
    const QList<DetectedRegion>& allRegions,
    const GridStructure& grid,
    const QList<Pattern>& patterns
) {
    double confidence = 1.0;
    
    // Spatial validation
    if (!hasNeighbors(region, allRegions)) {
        confidence *= 0.5;  // Penalize isolated regions
    }
    
    // Size validation
    if (!isSizeConsistent(region, allRegions)) {
        confidence *= 0.7;
    }
    
    // Type validation
    if (!isTypeConsistent(region, allRegions)) {
        confidence *= 0.8;
    }
    
    // Pattern validation
    if (!fitsPattern(region, patterns)) {
        confidence *= 0.6;
    }
    
    // Alignment validation
    if (!isAlignedWithGrid(region, grid)) {
        confidence *= 0.9;
    }
    
    return confidence;
}
```

### 5.2 Confidence Scoring
**File**: `src/utils/RegionDetector.cpp`
**Enhance**: `calculateConfidence()`

**Add Factors**:
- Pattern fit score
- Spatial context score
- Type consistency score
- Size consistency score
- Alignment score

### 5.3 Adaptive Filtering
**File**: `src/utils/RegionDetector.cpp`
**Method**: `filterByConfidence()`

**Algorithm**:
1. Calculate confidence for all regions
2. Set confidence threshold (default: 0.5)
3. Filter regions below threshold
4. Allow user to adjust threshold in UI

---

## Phase 6: Integration & UI

### 6.1 Enhanced Detection Result
**File**: `src/utils/RegionDetector.h`
**Enhance**: `DetectionResult` struct

**Add Fields**:
```cpp
struct DetectionResult {
    // Existing fields...
    
    // New fields:
    QList<DetectedGroup> inferredGroups;  // Groups inferred from patterns
    QMap<QString, QString> regionTypes;    // Type for each region
    QMap<QString, QString> suggestedColors; // Suggested color for each group
    GridStructure detectedGrid;            // Detected grid structure
    QList<Pattern> detectedPatterns;       // Detected patterns
};
```

### 6.2 Preview Dialog Enhancements
**File**: `src/ui/components/dialogs/DetectionPreviewDialog.cpp`

**Add Features**:
- Show inferred groups
- Show suggested colors
- Show detected patterns
- Allow user to accept/reject groups
- Allow user to adjust confidence threshold

### 6.3 Region Creation with Types
**File**: `src/ui/MainWindow.cpp`
**Enhance**: `onRegionsAcceptedFromDetection()`

**Changes**:
- Assign inferred types to regions
- Assign suggested colors
- Create groups automatically
- Allow user to override

---

## Implementation Order

### Sprint 1: Foundation (Week 1)
1. ✅ Multi-scale detection
2. ✅ Spatial clustering (basic)
3. ✅ Size consistency validation

### Sprint 2: Type Inference (Week 2)
1. ✅ Visual type inference
2. ✅ Pattern-based type inference
3. ⏳ OCR integration (optional, later)

### Sprint 3: Pattern Recognition (Week 3)
1. ✅ Canadian postal code detector
2. ✅ Name field detector
3. ✅ Number sequence detector

### Sprint 4: Grouping (Week 4)
1. ✅ Spatial grouping
2. ✅ Pattern-based grouping
3. ✅ Color suggestion

### Sprint 5: Filtering & Polish (Week 5)
1. ✅ Multi-layer validation
2. ✅ Confidence scoring
3. ✅ UI integration
4. ✅ Testing & refinement

---

## Key Design Decisions

### 1. **Layered Architecture**
- Each layer builds on previous
- Allows incremental improvement
- Easy to test and debug

### 2. **Pattern-First Approach**
- Start with known patterns (postal codes)
- Expand to generic patterns
- User can define custom patterns

### 3. **Confidence-Based Filtering**
- Score everything
- Filter by threshold
- User can adjust sensitivity

### 4. **Flexible Grouping**
- Multiple grouping strategies
- User can override
- Suggests but doesn't force

### 5. **Type-Aware Detection**
- Infer types early
- Use types for grouping
- Validate types in context

---

## Testing Strategy

### Unit Tests
- Pattern detectors (postal code, name fields, etc.)
- Type inference
- Spatial clustering
- Validation logic

### Integration Tests
- End-to-end detection on sample forms
- Pattern recognition accuracy
- Group inference accuracy

### User Testing
- Test on various form types
- Collect feedback on accuracy
- Iterate based on real-world usage

---

## Future Enhancements

1. **Machine Learning Integration**
   - Train models on user corrections
   - Learn new patterns automatically
   - Improve type inference accuracy

2. **Multi-Language Support**
   - Pattern detection for other languages
   - International postal code formats
   - Cultural form variations

3. **Custom Pattern Definition**
   - UI for defining new patterns
   - Pattern library/sharing
   - Pattern validation

4. **Advanced OCR Integration**
   - Real-time type validation
   - Content-based grouping
   - Semantic understanding

---

## Success Metrics

1. **Accuracy**: >90% correct region detection on test forms
2. **Precision**: <10% false positives
3. **Recall**: >85% of actual regions detected
4. **User Satisfaction**: Reduced manual correction time by 70%
5. **Pattern Recognition**: >95% accuracy on Canadian postal codes

---

## Conclusion

This plan transforms the detection system from "sensitive rectangle detector" to "intelligent form analyzer" that understands context, patterns, and semantics. The key is **layered analysis** with **pattern recognition** and **context-aware validation**.
