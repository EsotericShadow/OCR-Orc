#ifndef FORM_STRUCTURE_ANALYZER_H
#define FORM_STRUCTURE_ANALYZER_H

#include "RegionDetector.h"
#include "OcrTextExtractor.h"
#include <opencv2/opencv.hpp>
#include <QtCore/QList>
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Form field group structure
 */
struct FormFieldGroup {
    QString groupType;      // "name", "address", "date", "contact", "unknown"
    QList<cv::Rect> fields; // Fields in this group
    cv::Rect groupBoundary; // Bounding box of entire group
    QString layout;         // "horizontal", "vertical", "grid"
    double confidence;      // Confidence in group detection (0.0-1.0)
    
    FormFieldGroup() : confidence(0.0) {}
};

/**
 * @brief Form structure analyzer
 * 
 * Understands form semantics - field grouping, relationships, visual hierarchy.
 * Detects field groups (name fields, address fields, date fields) and identifies
 * field relationships (horizontal, vertical, grid layouts).
 */
class FormStructureAnalyzer {
public:
    FormStructureAnalyzer();
    ~FormStructureAnalyzer() = default;
    
    /**
     * @brief Detect form structure from detected regions
     * @param regions Detected regions
     * @param ocrRegions OCR text regions for semantic analysis
     * @return List of form field groups
     */
    QList<FormFieldGroup> detectFormStructure(
        const QList<DetectedRegion>& regions,
        const QList<OCRTextRegion>& ocrRegions);
    
    /**
     * @brief Group related fields together
     * @param regions Detected regions
     * @return Form field group
     */
    FormFieldGroup groupRelatedFields(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Infer field type from context
     * @param field Field rectangle
     * @param nearbyFields Nearby detected fields
     * @param ocrRegions OCR regions for semantic hints
     * @return Inferred field type
     */
    QString inferFieldTypeFromContext(const cv::Rect& field,
                                     const QList<DetectedRegion>& nearbyFields,
                                     const QList<OCRTextRegion>& ocrRegions);

private:
    /**
     * @brief Detect name field groups (First, Middle, Last)
     * @param regions All detected regions
     * @param ocrRegions OCR regions
     * @return List of name field groups
     */
    QList<FormFieldGroup> detectNameFieldGroups(
        const QList<DetectedRegion>& regions,
        const QList<OCRTextRegion>& ocrRegions);
    
    /**
     * @brief Detect address field groups (Street, City, State, Zip)
     * @param regions All detected regions
     * @param ocrRegions OCR regions
     * @return List of address field groups
     */
    QList<FormFieldGroup> detectAddressFieldGroups(
        const QList<DetectedRegion>& regions,
        const QList<OCRTextRegion>& ocrRegions);
    
    /**
     * @brief Detect date field groups
     * @param regions All detected regions
     * @param ocrRegions OCR regions
     * @return List of date field groups
     */
    QList<FormFieldGroup> detectDateFieldGroups(
        const QList<DetectedRegion>& regions,
        const QList<OCRTextRegion>& ocrRegions);
    
    /**
     * @brief Determine layout type (horizontal, vertical, grid)
     * @param fields Fields in group
     * @return Layout type string
     */
    QString determineLayout(const QList<cv::Rect>& fields);
    
    /**
     * @brief Calculate group boundary from fields
     * @param fields Fields in group
     * @return Bounding rectangle
     */
    cv::Rect calculateGroupBoundary(const QList<cv::Rect>& fields);
    
    /**
     * @brief Check if fields are horizontally aligned
     * @param fields Fields to check
     * @return True if horizontally aligned
     */
    bool areHorizontallyAligned(const QList<cv::Rect>& fields);
    
    /**
     * @brief Check if fields are vertically aligned
     * @param fields Fields to check
     * @return True if vertically aligned
     */
    bool areVerticallyAligned(const QList<cv::Rect>& fields);
};

} // namespace ocr_orc

#endif // FORM_STRUCTURE_ANALYZER_H
