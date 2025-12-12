#ifndef DOCUMENT_TYPE_CLASSIFIER_H
#define DOCUMENT_TYPE_CLASSIFIER_H

#include <opencv2/opencv.hpp>
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Document type enumeration
 */
enum DocumentType {
    GOVERNMENT_FORM,    // Structured, high contrast, often colored sections
    MEDICAL_FORM,       // Colored sections, varied layouts
    TAX_FORM,           // Dense, small fields, high information density
    HANDWRITTEN_FORM,   // Low contrast, cursive, guide lines
    STANDARD_FORM       // Default/unknown type
};

/**
 * @brief Document type classifier
 * 
 * Classifies document types to enable adaptive thresholds and processing strategies.
 * Uses heuristics based on contrast, color distribution, field density, and text characteristics.
 */
class DocumentTypeClassifier {
public:
    DocumentTypeClassifier();
    ~DocumentTypeClassifier() = default;
    
    /**
     * @brief Classify document type from image
     * @param image Source image
     * @return DocumentType with classification result
     */
    DocumentType classifyDocument(const cv::Mat& image);
    
    /**
     * @brief Get classification confidence
     * @return Confidence score (0.0-1.0)
     */
    double getConfidence() const { return classificationConfidence; }
    
    /**
     * @brief Get document type as string
     * @param type Document type
     * @return String representation
     */
    static QString documentTypeToString(DocumentType type);

private:
    /**
     * @brief Analyze contrast characteristics
     * @param image Source image
     * @return Contrast score (0.0-1.0)
     */
    double analyzeContrast(const cv::Mat& image);
    
    /**
     * @brief Analyze color distribution
     * @param image Source image
     * @return Colorfulness score (0.0-1.0)
     */
    double analyzeColorDistribution(const cv::Mat& image);
    
    /**
     * @brief Analyze field density (detected rectangles/contours)
     * @param image Source image
     * @return Field density score (0.0-1.0)
     */
    double analyzeFieldDensity(const cv::Mat& image);
    
    /**
     * @brief Analyze text characteristics
     * @param image Source image
     * @return Text characteristic score
     */
    double analyzeTextCharacteristics(const cv::Mat& image);
    
    /**
     * @brief Detect if document has colored sections
     * @param image Source image
     * @return True if colored sections detected
     */
    bool hasColoredSections(const cv::Mat& image);
    
    /**
     * @brief Detect if document appears to be handwritten
     * @param image Source image
     * @return True if handwritten characteristics detected
     */
    bool isHandwritten(const cv::Mat& image);
    
    double classificationConfidence;
};

} // namespace ocr_orc

#endif // DOCUMENT_TYPE_CLASSIFIER_H
