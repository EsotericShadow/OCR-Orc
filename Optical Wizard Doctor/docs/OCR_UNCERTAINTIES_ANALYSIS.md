# OCR-First Detection: Research Findings vs FAFO Items

## ✅ RESEARCHED & ANSWERED

### 1. Tesseract CMake Integration ✅
**Answer**: Both methods work on macOS:
- **Method 1**: `find_package(Tesseract REQUIRED)` - if TesseractConfig.cmake exists
- **Method 2**: `pkg_check_modules(TESSERACT REQUIRED tesseract)` - more reliable
- **Recommendation**: Use `pkg-config` method (already in plan) - it's more reliable
- **Installation**: `brew install tesseract` (includes Leptonica automatically)

### 2. Tesseract Performance ✅
**Answer**: 
- **Typical**: ~300ms per page for standard documents
- **Range**: 0.3-17 seconds depending on:
  - Document complexity
  - Language (English: ~2s, Arabic: ~17s)
  - Hardware (CPU, RAM)
- **Our case**: Forms likely 1-3 seconds per page
- **Conclusion**: Background threading is essential (already implemented ✅)

### 3. OCR Confidence Thresholds ✅
**Answer**:
- **High-sensitivity apps**: 90%+ threshold (financial documents)
- **General use**: 50-70% threshold
- **Best practice**: Dynamic thresholding based on document quality
- **Our plan**: 50% minimum is reasonable, but should be configurable
- **Recommendation**: Start with 50%, make adjustable

### 4. Checkbox Size Parameters ✅
**Answer**:
- **Minimum size**: 24x24 CSS pixels (accessibility standard)
- **Physical size**: 10-30px is reasonable for scanned forms
- **Aspect ratio**: Must be square (1:1)
- **Our plan**: 10-30px with 0.8-1.2 aspect ratio is good ✅
- **Recommendation**: Keep as-is, but allow configuration

### 5. Pattern Detection Threshold ✅
**Answer**:
- **Majority voting**: 66%+ is common (2 out of 3 classifiers)
- **Our plan**: 70% threshold is reasonable ✅
- **Research**: Lower threshold = more sensitive, higher = more specific
- **Recommendation**: Start with 70%, make configurable

### 6. Text Expansion Radius ✅
**Answer**:
- **Research**: Small expansion factor helps merge adjacent characters
- **No specific percentage found**, but:
  - Geometric property-based expansion is common
  - Reinforcement learning adjusts boxes dynamically
- **Our plan**: 10-30% of text height is reasonable
- **Recommendation**: Start with 20%, test and adjust

### 7. Horizontal Grouping Tolerance ✅
**Answer**:
- **Transkribus platform**:
  - Low: 0.5% of image width
  - Medium: 1% of image width
  - High: 5% of image width
- **Our plan**: 2% of page height is reasonable (between medium/high)
- **Recommendation**: Start with 2%, allow user adjustment

### 8. Tesseract PSM Mode ✅
**Answer**:
- **PSM_AUTO (3)**: Good for complex layouts, but may reorder text
- **PSM_SINGLE_BLOCK (6)**: Better for structured forms, preserves order
- **Best practice**: Test both, use PSM_6 for forms
- **Our plan**: Start with PSM_AUTO, allow switching
- **Recommendation**: Default to PSM_6 for forms, PSM_AUTO for general

### 9. Hybrid Confidence Scoring ✅
**Answer**:
- **Research**: Weighted average of multiple factors is standard
- **Factors**: OCR confidence, image quality, positional weighting
- **Our plan**: 40% OCR, 30% line, 20% rectangularity, 10% pattern
- **Recommendation**: Start with these weights, make configurable
- **Note**: No "optimal" formula exists - depends on use case

### 10. Checkbox State Detection ✅
**Answer**:
- **Research**: No standard fill percentage threshold found
- **Common approach**: Binary (checked/unchecked) or indeterminate
- **Our plan**: 30% fill threshold is reasonable
- **Recommendation**: Test on real checkboxes, adjust threshold
- **Note**: This is more FAFO than research-based

## ❓ FAFO (Fuck Around and Find Out)

### 1. **Your Specific Form Types** ❓
- What types of forms are you processing?
- What's the typical layout complexity?
- How many fields per form?
- **Action**: Test on real forms, adjust parameters

### 2. **Your Image Quality** ❓
- What's the typical scan quality?
- What DPI are your PDFs?
- How much noise/artifacts?
- **Action**: Test preprocessing pipeline, adjust thresholds

### 3. **Your Checkbox Styles** ❓
- What size are checkboxes in your forms?
- Are they always square?
- How close to text?
- **Action**: Test checkbox detection, adjust search areas

### 4. **Your Form Layouts** ❓
- How consistent are form layouts?
- Are checkboxes always left or right?
- How many items per line?
- **Action**: Test pattern detection, adjust thresholds

### 5. **Confidence Formula Weights** ❓
- Do our weights (40/30/20/10) work for your forms?
- Which factor is most important?
- **Action**: Test, measure precision/recall, adjust weights

### 6. **Filtering Thresholds** ❓
- Is 0.5/0.7 the right split?
- Too aggressive or too lenient?
- **Action**: Test on real forms, measure false positive rate

### 7. **Text Expansion Radius** ❓
- Is 20% the right amount?
- Too much = includes noise, too little = misses lines
- **Action**: Test on various forms, find sweet spot

### 8. **Horizontal Grouping Tolerance** ❓
- Is 2% right for your forms?
- Group unrelated lines or miss related ones?
- **Action**: Test, adjust based on results

### 9. **PSM Mode Selection** ❓
- Which PSM works best for your forms?
- PSM_6 vs PSM_AUTO?
- **Action**: Test both, measure accuracy

### 10. **Checkbox State Threshold** ❓
- Is 30% fill right?
- What about partially filled or damaged checkboxes?
- **Action**: Test on real checkboxes, adjust threshold

### 11. **Multi-Language Support** ❓
- Do you need non-English OCR?
- Which languages?
- **Action**: Test if needed, add language packs

### 12. **Handwritten Text** ❓
- How much handwritten text in your forms?
- Can we ignore it or need special handling?
- **Action**: Test OCR on handwritten portions, filter if needed

### 13. **Memory Usage** ❓
- How large are your forms?
- Memory footprint acceptable?
- **Action**: Monitor memory, optimize if needed

### 14. **Integration with Existing Code** ❓
- Does OCR-first work with existing detection methods?
- Any conflicts or refactoring needed?
- **Action**: Test integration, fix issues as they arise

### 15. **Performance on Your Hardware** ❓
- Actual processing time on your machine?
- CPU/RAM usage?
- **Action**: Benchmark, optimize if needed

## 📊 SUMMARY

### ✅ Can Research (Answered):
1. CMake integration method
2. Performance characteristics (~1-3s per form)
3. Confidence threshold ranges (50-90%)
4. Checkbox size standards (24x24px min)
5. Pattern detection thresholds (66-70%)
6. Text expansion approach (small percentage)
7. Horizontal grouping tolerance (0.5-5%)
8. PSM mode selection (PSM_6 for forms)
9. Hybrid scoring approach (weighted average)
10. General best practices

### ❓ Must FAFO (Test):
1. **Your specific forms** - layout, complexity, field count
2. **Your image quality** - DPI, noise, artifacts
3. **Your checkbox styles** - size, placement, proximity
4. **Your form layouts** - consistency, patterns
5. **Confidence weights** - optimal for your use case
6. **Filtering thresholds** - balance precision/recall
7. **Text expansion radius** - sweet spot for your forms
8. **Grouping tolerance** - right balance
9. **PSM mode** - which works best
10. **Checkbox state threshold** - accurate detection
11. **Language needs** - if any
12. **Handwritten text** - how much, how to handle
13. **Memory usage** - acceptable?
14. **Code integration** - any issues?
15. **Performance** - actual benchmarks

## 🎯 RECOMMENDATIONS

### Phase 1: Implement with Research-Based Defaults
- Use researched values as starting points
- Make all parameters configurable
- Add logging/metrics to measure performance

### Phase 2: Test on Real Forms
- Test on 5-10 representative forms
- Measure precision, recall, false positive rate
- Adjust parameters based on results

### Phase 3: Iterate
- Fine-tune thresholds based on test results
- Document what works for your specific use case
- Create presets for different form types if needed

### Configuration Strategy
Make these configurable:
- OCR confidence threshold (default: 50%)
- Confidence formula weights (default: 40/30/20/10)
- Filtering thresholds (default: 0.5/0.7)
- Text expansion radius (default: 20%)
- Horizontal grouping tolerance (default: 2%)
- Checkbox size range (default: 10-30px)
- Checkbox state threshold (default: 30%)
- PSM mode (default: PSM_6 for forms)

### Testing Plan
1. **Unit tests**: Each component with known inputs
2. **Integration tests**: Full pipeline on sample forms
3. **Real-world tests**: 10+ actual forms
4. **Metrics**: Precision, recall, false positive rate, processing time
5. **Iteration**: Adjust parameters, retest, document
