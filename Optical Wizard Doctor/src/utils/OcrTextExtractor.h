#ifndef OCR_TEXT_EXTRACTOR_H
#define OCR_TEXT_EXTRACTOR_H

#include <QtGui/QImage>
#include <QtCore/QString>
#include <QtCore/QList>
#include <opencv2/opencv.hpp>
#include "../core/CoordinateSystem.h"

namespace ocr_orc {

/**
 * @brief OCR text region extracted from image
 */
struct OCRTextRegion {
    QString text;              // Extracted text content
    cv::Rect boundingBox;     // Bounding box in pixels
    NormalizedCoords coords;   // Normalized coordinates (0.0-1.0)
    double confidence;         // OCR confidence (0.0-100.0)
    QString typeHint;          // "number", "letter", "mixed", "unknown"
    int blockId;               // OCR block/paragraph ID
    int lineId;                // OCR line ID within block
    int wordId;                // Word ID within line
    
    OCRTextRegion() : confidence(0.0), typeHint("unknown"), blockId(0), lineId(0), wordId(0) {}
};

/**
 * @brief Tesseract OCR text extractor
 * 
 * Extracts text regions from images using Tesseract OCR with bounding boxes
 * and confidence scores. Includes preprocessing pipeline for optimal accuracy.
 */
class OcrTextExtractor {
public:
    OcrTextExtractor();
    ~OcrTextExtractor();
    
    /**
     * @brief Extract text regions from image
     * @param image Source image to process
     * @return List of OCR text regions with bounding boxes and confidence
     */
    QList<OCRTextRegion> extractTextRegions(const QImage& image);
    
    /**
     * @brief Filter regions by minimum confidence
     * @param regions List of OCR regions
     * @param minConf Minimum confidence threshold (0.0-100.0)
     * @return Filtered list of regions
     */
    QList<OCRTextRegion> filterByConfidence(const QList<OCRTextRegion>& regions, double minConf = 50.0);
    
    /**
     * @brief Set minimum confidence threshold
     * @param minConf Minimum confidence (0.0-100.0)
     */
    void setMinConfidence(double minConf) { minConfidence = minConf; }
    
    /**
     * @brief Get current minimum confidence threshold
     * @return Minimum confidence threshold
     */
    double getMinConfidence() const { return minConfidence; }

private:
    /**
     * @brief Preprocess image for OCR
     * @param image Source image
     * @return Preprocessed cv::Mat (grayscale, denoised, thresholded)
     */
    cv::Mat preprocessImage(const QImage& image);
    
    /**
     * @brief Configure Tesseract API settings
     * @param api Tesseract API instance
     */
    void configureTesseract(void* api);  // Using void* to avoid including tesseract headers in header
    
    /**
     * @brief Infer text type from extracted text
     * @param text Extracted text string
     * @return Type hint: "number", "letter", "mixed", "unknown"
     */
    QString inferTypeFromText(const QString& text);
    
    double minConfidence;  // Minimum confidence threshold (default: 50.0)
};

} // namespace ocr_orc

#endif // OCR_TEXT_EXTRACTOR_H
