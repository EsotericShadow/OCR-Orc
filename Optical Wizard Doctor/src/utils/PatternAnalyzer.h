#ifndef PATTERN_ANALYZER_H
#define PATTERN_ANALYZER_H

#include "OcrTextExtractor.h"
#include "CheckboxDetector.h"
#include <QtCore/QString>
#include <QtCore/QList>

namespace ocr_orc {

/**
 * @brief Pattern analyzer for form field detection
 * 
 * Analyzes checkbox placement patterns across multiple lines to determine
 * form structure and apply patterns to ambiguous cases.
 */
class PatternAnalyzer {
public:
    PatternAnalyzer();
    ~PatternAnalyzer() = default;
    
    /**
     * @brief Analyze checkbox placement pattern across all regions
     * @param regions List of OCR text regions
     * @param checkboxes List of checkbox detections (one per region, may be empty)
     * @return Detected pattern: "checkbox_left", "checkbox_right", "mixed", "unknown"
     */
    QString analyzeCheckboxPattern(const QList<OCRTextRegion>& regions, 
                                   const QList<CheckboxDetection>& checkboxes);
    
    /**
     * @brief Group regions by horizontal alignment
     * @param regions List of OCR regions
     * @param tolerance Tolerance percentage (default: 2% of page height)
     * @param pageHeight Page height in pixels
     * @return List of groups, each group is a list of regions on the same line
     */
    QList<QList<OCRTextRegion>> groupByHorizontalAlignment(const QList<OCRTextRegion>& regions, 
                                                           double tolerance, 
                                                           int pageHeight);
    
    /**
     * @brief Determine pattern for a single row
     * @param row List of regions in the row
     * @param checkboxes List of checkbox detections
     * @return Pattern for this row
     */
    QString determineRowPattern(const QList<OCRTextRegion>& row, 
                               const QList<CheckboxDetection>& checkboxes);
    
    /**
     * @brief Apply detected pattern to ambiguous cases
     * @param regions List of OCR regions
     * @param pattern Detected pattern to apply
     */
    void applyPatternToAmbiguous(QList<OCRTextRegion>& regions, const QString& pattern);
    
    /**
     * @brief Detect multi-item lines (sequences)
     * @param regions List of OCR regions
     * @param pageWidth Page width in pixels
     * @return List of sequence groups
     */
    QList<QList<OCRTextRegion>> detectMultiItemLines(const QList<OCRTextRegion>& regions, 
                                                      int pageWidth);
    
    /**
     * @brief Get pattern match score for a region
     * @param region OCR text region
     * @param pattern Detected pattern
     * @return Score 0.0-1.0
     */
    double getPatternMatchScore(const OCRTextRegion& region, const QString& pattern);
    
    /**
     * @brief Set pattern detection threshold
     * @param threshold Threshold percentage (default: 70%)
     */
    void setPatternThreshold(double threshold) { patternThreshold = threshold; }

private:
    /**
     * @brief Find checkbox detection for a region
     * @param region OCR text region
     * @param checkboxes List of checkbox detections
     * @return Checkbox detection or empty detection
     */
    CheckboxDetection findCheckboxForRegion(const OCRTextRegion& region, 
                                           const QList<CheckboxDetection>& checkboxes);
    
    double patternThreshold;  // Pattern detection threshold (default: 0.7 = 70%)
};

} // namespace ocr_orc

#endif // PATTERN_ANALYZER_H
