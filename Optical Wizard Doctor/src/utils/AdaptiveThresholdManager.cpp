#include "AdaptiveThresholdManager.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

namespace ocr_orc {

AdaptiveThresholdManager::AdaptiveThresholdManager()
    : currentDocumentType(STANDARD_FORM)
    , hasCustomOverrides(false)
    , customBaseBrightness(0.0)
    , customEdgeDensity(0.0)
    , customHorizontalEdgeDensity(0.0)
    , customVerticalEdgeDensity(0.0)
    , customIoUThreshold(0.0)
    , customOcrConfidence(0.0)
    , customHorizontalOverfit(0.0)
    , customVerticalOverfit(0.0)
    , customBrightnessAdaptiveFactor(0.85)
{
    initializeThresholds();
}

AdaptiveThresholdManager::AdaptiveThresholdManager(DocumentType documentType)
    : currentDocumentType(documentType)
    , hasCustomOverrides(false)
    , customBaseBrightness(0.0)
    , customEdgeDensity(0.0)
    , customHorizontalEdgeDensity(0.0)
    , customVerticalEdgeDensity(0.0)
    , customIoUThreshold(0.0)
    , customOcrConfidence(0.0)
    , customHorizontalOverfit(0.0)
    , customVerticalOverfit(0.0)
    , customBrightnessAdaptiveFactor(0.85)
{
    initializeThresholds();
}

void AdaptiveThresholdManager::setDocumentType(DocumentType type)
{
    currentDocumentType = type;
}

void AdaptiveThresholdManager::initializeThresholds()
{
    // GOVERNMENT_FORM: High contrast, colored sections, structured
    thresholds[GOVERNMENT_FORM] = {
        .baseBrightness = 0.75,          // Higher threshold (colored backgrounds)
        .edgeDensity = 0.10,             // Slightly higher (structured forms)
        .horizontalEdgeDensity = 0.12,   // Text lines more prominent
        .verticalEdgeDensity = 0.08,     // Vertical borders
        .iouThreshold = 0.65,            // Stricter matching (well-structured)
        .ocrConfidence = 65.0,           // Higher OCR confidence needed
        .horizontalOverfit = 35.0,       // Less overfitting (precise forms)
        .verticalOverfit = 55.0          // Moderate vertical overfit
    };
    
    // MEDICAL_FORM: Colored sections, varied layouts
    thresholds[MEDICAL_FORM] = {
        .baseBrightness = 0.70,          // Medium threshold
        .edgeDensity = 0.09,              // Medium edge density
        .horizontalEdgeDensity = 0.11,    // Text lines
        .verticalEdgeDensity = 0.07,      // Borders
        .iouThreshold = 0.60,             // Standard matching
        .ocrConfidence = 60.0,            // Standard OCR confidence
        .horizontalOverfit = 40.0,        // Standard overfitting
        .verticalOverfit = 60.0           // Standard vertical
    };
    
    // TAX_FORM: Dense, small fields, high contrast
    thresholds[TAX_FORM] = {
        .baseBrightness = 0.80,           // Very high (dense forms)
        .edgeDensity = 0.12,              // Higher (many small fields)
        .horizontalEdgeDensity = 0.13,    // More text lines
        .verticalEdgeDensity = 0.09,      // More borders
        .iouThreshold = 0.70,             // Very strict (precise matching needed)
        .ocrConfidence = 70.0,            // Higher OCR confidence (dense text)
        .horizontalOverfit = 30.0,        // Less overfitting (small fields)
        .verticalOverfit = 50.0            // Less vertical (small fields)
    };
    
    // HANDWRITTEN_FORM: Low contrast, cursive, guide lines
    thresholds[HANDWRITTEN_FORM] = {
        .baseBrightness = 0.60,           // Lower threshold (low contrast)
        .edgeDensity = 0.06,              // Lower (smooth handwriting)
        .horizontalEdgeDensity = 0.05,   // Lower (cursive, not sharp lines)
        .verticalEdgeDensity = 0.04,      // Lower
        .iouThreshold = 0.55,             // More lenient (varied handwriting)
        .ocrConfidence = 50.0,            // Lower OCR confidence (handwriting harder)
        .horizontalOverfit = 50.0,        // More overfitting (varied sizes)
        .verticalOverfit = 70.0           // More vertical (handwriting fields taller)
    };
    
    // STANDARD_FORM: Default/unknown type
    thresholds[STANDARD_FORM] = {
        .baseBrightness = 0.70,          // Default threshold
        .edgeDensity = 0.08,             // Default edge density
        .horizontalEdgeDensity = 0.10,   // Default horizontal
        .verticalEdgeDensity = 0.06,      // Default vertical
        .iouThreshold = 0.60,             // Default IoU (expert recommendation)
        .ocrConfidence = 60.0,            // Default OCR confidence
        .horizontalOverfit = 40.0,        // Default overfitting
        .verticalOverfit = 60.0           // Default vertical
    };
}

double AdaptiveThresholdManager::getBrightnessThreshold(DocumentType type, const cv::Rect& region, const cv::Mat& image) const
{
    // Calculate local brightness for adaptive thresholding
    double localBrightness = calculateLocalBrightness(region, image, 50);
    
    // Use custom adaptive factor if set, otherwise default to 85%
    double adaptiveFactor = hasCustomOverrides ? customBrightnessAdaptiveFactor : 0.85;
    double adaptiveThreshold = localBrightness * adaptiveFactor;
    
    // Ensure it's not below the base threshold for this document type
    double baseThreshold = getBaseBrightnessThreshold(type);
    
    return std::max(adaptiveThreshold, baseThreshold * 0.9);  // At least 90% of base
}

double AdaptiveThresholdManager::getBaseBrightnessThreshold(DocumentType type) const
{
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].baseBrightness;
}

double AdaptiveThresholdManager::getEdgeDensityThreshold(DocumentType type) const
{
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].edgeDensity;
}

double AdaptiveThresholdManager::getHorizontalEdgeDensityThreshold(DocumentType type) const
{
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].horizontalEdgeDensity;
}

double AdaptiveThresholdManager::getVerticalEdgeDensityThreshold(DocumentType type) const
{
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].verticalEdgeDensity;
}

double AdaptiveThresholdManager::getIoUThreshold(DocumentType type) const
{
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].iouThreshold;
}

double AdaptiveThresholdManager::getOcrConfidenceThreshold(DocumentType type) const
{
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].ocrConfidence;
}

double AdaptiveThresholdManager::getHorizontalOverfitPercent(DocumentType type) const
{
    if (hasCustomOverrides) {
        return customHorizontalOverfit;
    }
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].horizontalOverfit;
}

double AdaptiveThresholdManager::getVerticalOverfitPercent(DocumentType type) const
{
    if (hasCustomOverrides) {
        return customVerticalOverfit;
    }
    if (type < 0 || type > 4) {
        type = STANDARD_FORM;
    }
    return thresholds[type].verticalOverfit;
}

void AdaptiveThresholdManager::setCustomOverrides(double baseBrightness, double edgeDensity,
                                                  double horizontalEdgeDensity, double verticalEdgeDensity,
                                                  double iouThreshold, double ocrConfidence,
                                                  double horizontalOverfit, double verticalOverfit,
                                                  double brightnessAdaptiveFactor)
{
    hasCustomOverrides = true;
    customBaseBrightness = baseBrightness;
    customEdgeDensity = edgeDensity;
    customHorizontalEdgeDensity = horizontalEdgeDensity;
    customVerticalEdgeDensity = verticalEdgeDensity;
    customIoUThreshold = iouThreshold;
    customOcrConfidence = ocrConfidence;
    customHorizontalOverfit = horizontalOverfit;
    customVerticalOverfit = verticalOverfit;
    customBrightnessAdaptiveFactor = brightnessAdaptiveFactor;
}

double AdaptiveThresholdManager::calculateLocalBrightness(const cv::Rect& region, const cv::Mat& image, int padding) const
{
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return 0.7;  // Default brightness
    }
    
    // Expand region with padding for local context
    cv::Rect expandedRegion(
        std::max(0, region.x - padding),
        std::max(0, region.y - padding),
        std::min(image.cols - std::max(0, region.x - padding), region.width + padding * 2),
        std::min(image.rows - std::max(0, region.y - padding), region.height + padding * 2)
    );
    
    if (expandedRegion.width <= 0 || expandedRegion.height <= 0) {
        return 0.7;
    }
    
    // Extract ROI
    cv::Mat roi = image(expandedRegion);
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (roi.channels() == 3) {
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = roi.clone();
    }
    
    // Calculate mean brightness
    cv::Scalar meanBrightness = cv::mean(gray);
    double brightness = meanBrightness[0] / 255.0;
    
    return brightness;
}

} // namespace ocr_orc
