#ifndef ADAPTIVE_THRESHOLD_MANAGER_H
#define ADAPTIVE_THRESHOLD_MANAGER_H

#include "DocumentTypeClassifier.h"
#include <opencv2/opencv.hpp>
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Adaptive threshold manager
 * 
 * Provides document-type-aware thresholds for all detection parameters.
 * Centralizes threshold management to enable adaptive behavior based on document characteristics.
 */
class AdaptiveThresholdManager {
public:
    AdaptiveThresholdManager();
    explicit AdaptiveThresholdManager(DocumentType documentType);
    ~AdaptiveThresholdManager() = default;
    
    /**
     * @brief Set document type (updates all thresholds)
     * @param type Document type
     */
    void setDocumentType(DocumentType type);
    
    /**
     * @brief Get adaptive brightness threshold for text filtering
     * @param type Document type
     * @param region Region to check (for local brightness calculation)
     * @param image Source image
     * @return Adaptive brightness threshold (0.0-1.0)
     */
    double getBrightnessThreshold(DocumentType type, const cv::Rect& region, const cv::Mat& image) const;
    
    /**
     * @brief Get base brightness threshold for document type
     * @param type Document type
     * @return Base brightness threshold
     */
    double getBaseBrightnessThreshold(DocumentType type) const;
    
    /**
     * @brief Get edge density threshold for text filtering
     * @param type Document type
     * @return Edge density threshold
     */
    double getEdgeDensityThreshold(DocumentType type) const;
    
    /**
     * @brief Get horizontal edge density threshold
     * @param type Document type
     * @return Horizontal edge density threshold
     */
    double getHorizontalEdgeDensityThreshold(DocumentType type) const;
    
    /**
     * @brief Get vertical edge density threshold
     * @param type Document type
     * @return Vertical edge density threshold
     */
    double getVerticalEdgeDensityThreshold(DocumentType type) const;
    
    /**
     * @brief Get IoU threshold for consensus matching
     * @param type Document type
     * @return IoU threshold (0.0-1.0)
     */
    double getIoUThreshold(DocumentType type) const;
    
    /**
     * @brief Get OCR confidence threshold
     * @param type Document type
     * @return OCR confidence threshold (0.0-100.0)
     */
    double getOcrConfidenceThreshold(DocumentType type) const;
    
    /**
     * @brief Get horizontal overfitting percentage
     * @param type Document type
     * @return Overfitting percentage (0-100)
     */
    double getHorizontalOverfitPercent(DocumentType type) const;
    
    /**
     * @brief Get vertical overfitting percentage
     * @param type Document type
     * @return Overfitting percentage (0-100)
     */
    double getVerticalOverfitPercent(DocumentType type) const;
    
    /**
     * @brief Get current document type
     * @return Document type
     */
    DocumentType getDocumentType() const { return currentDocumentType; }
    
    /**
     * @brief Set custom threshold overrides (from user parameters)
     * @param baseBrightness Base brightness threshold override
     * @param edgeDensity Total edge density threshold override
     * @param horizontalEdgeDensity Horizontal edge density threshold override
     * @param verticalEdgeDensity Vertical edge density threshold override
     * @param iouThreshold IoU threshold override
     * @param ocrConfidence OCR confidence threshold override
     * @param horizontalOverfit Horizontal overfit percentage override
     * @param verticalOverfit Vertical overfit percentage override
     * @param brightnessAdaptiveFactor Brightness adaptive factor override
     */
    void setCustomOverrides(double baseBrightness, double edgeDensity,
                           double horizontalEdgeDensity, double verticalEdgeDensity,
                           double iouThreshold, double ocrConfidence,
                           double horizontalOverfit, double verticalOverfit,
                           double brightnessAdaptiveFactor);
    
    /**
     * @brief Get brightness adaptive factor (for custom overrides)
     * @return Adaptive factor (0.0-1.0)
     */
    double getBrightnessAdaptiveFactor() const { return customBrightnessAdaptiveFactor; }

private:
    DocumentType currentDocumentType;
    
    // Threshold storage per document type
    struct Thresholds {
        double baseBrightness;           // Base brightness threshold
        double edgeDensity;              // Total edge density threshold
        double horizontalEdgeDensity;    // Horizontal edge density threshold
        double verticalEdgeDensity;      // Vertical edge density threshold
        double iouThreshold;             // IoU threshold for consensus
        double ocrConfidence;            // OCR confidence threshold
        double horizontalOverfit;        // Horizontal overfitting percentage
        double verticalOverfit;          // Vertical overfitting percentage
    };
    
    Thresholds thresholds[5];  // One for each DocumentType
    
    // Custom overrides (from user parameters)
    bool hasCustomOverrides;
    double customBaseBrightness;
    double customEdgeDensity;
    double customHorizontalEdgeDensity;
    double customVerticalEdgeDensity;
    double customIoUThreshold;
    double customOcrConfidence;
    double customHorizontalOverfit;
    double customVerticalOverfit;
    double customBrightnessAdaptiveFactor;
    
    /**
     * @brief Initialize default thresholds for all document types
     */
    void initializeThresholds();
    
    /**
     * @brief Calculate local brightness for adaptive thresholding
     * @param region Region to check
     * @param image Source image
     * @param padding Padding around region for local context
     * @return Local brightness (0.0-1.0)
     */
    double calculateLocalBrightness(const cv::Rect& region, const cv::Mat& image, int padding = 50) const;
};

} // namespace ocr_orc

#endif // ADAPTIVE_THRESHOLD_MANAGER_H
