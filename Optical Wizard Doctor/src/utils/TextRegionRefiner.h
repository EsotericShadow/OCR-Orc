#ifndef TEXT_REGION_REFINER_H
#define TEXT_REGION_REFINER_H

#include "OcrTextExtractor.h"
#include <opencv2/opencv.hpp>
#include <QtCore/QList>

namespace ocr_orc {

/**
 * @brief Text region refiner using computer vision
 * 
 * Refines OCR text bounding boxes by detecting lines and rectangles
 * around text regions, expanding boxes to include form field boundaries.
 */
class TextRegionRefiner {
public:
    TextRegionRefiner();
    ~TextRegionRefiner() = default;
    
    /**
     * @brief Refine OCR text region bounding box
     * @param ocrRegion OCR text region to refine
     * @param image Source image for CV processing
     * @return Refined normalized coordinates
     */
    NormalizedCoords refineRegion(const OCRTextRegion& ocrRegion, const cv::Mat& image);
    
    /**
     * @brief Get line detection score (0.0-1.0)
     * @return Score based on number of detected lines and their strength
     */
    double getLineDetectionScore() const { return lineDetectionScore; }
    
    /**
     * @brief Get rectangularity score (0.0-1.0)
     * @return Score based on how rectangular the detected region is
     */
    double getRectangularityScore() const { return rectangularityScore; }
    
    /**
     * @brief Set text expansion radius percentage
     * @param percent Percentage of text height to expand (default: 20)
     */
    void setExpansionRadius(int percent) { expansionRadiusPercent = percent; }
    
    /**
     * @brief Get current expansion radius percentage
     * @return Expansion radius percentage
     */
    int getExpansionRadius() const { return expansionRadiusPercent; }
    
    /**
     * @brief Check if text is likely a label (for external use)
     * @param text Text to check
     * @return True if text appears to be a label
     */
    bool isLikelyLabel(const QString& text);
    
    /**
     * @brief Semantic label information
     */
    struct SemanticLabelInfo {
        QString fieldType;        // "text_input", "date", "address", "name", "number", "unknown"
        QString expectedLocation; // "below", "right", "above", "unknown"
        double confidence;       // Confidence in semantic inference (0.0-1.0)
        
        SemanticLabelInfo() : fieldType("unknown"), expectedLocation("unknown"), confidence(0.0) {}
    };
    
    /**
     * @brief Infer field type and expected location from label text
     * @param labelText Label text to analyze
     * @return SemanticLabelInfo with inferred field type and location
     */
    SemanticLabelInfo inferFieldTypeFromLabel(const QString& labelText);
    
    /**
     * @brief Check if a region contains text (should be filtered out)
     * @param region Region to check
     * @param image Source image
     * @param ocrRegions All OCR text regions (to check for overlap)
     * @param thresholdManager Optional adaptive threshold manager (for adaptive brightness)
     * @param ocrOverlapThreshold Custom OCR overlap threshold (0.0-1.0, default 0.10)
     * @param minHorizontalLines Custom minimum horizontal lines threshold (default 2)
     * @return True if region contains text, false if empty
     */
    bool regionContainsText(const cv::Rect& region, const cv::Mat& image, 
                           const QList<OCRTextRegion>& ocrRegions,
                           class AdaptiveThresholdManager* thresholdManager = nullptr,
                           double ocrOverlapThreshold = 0.10,
                           int minHorizontalLines = 2);
    
    /**
     * @brief Multi-pass refinement: find empty form fields near OCR hints
     * @param ocrHints OCR text regions (used as coordinate hints only)
     * @param image Source image
     * @return List of empty form field rectangles (no text content)
     */
    QList<cv::Rect> findEmptyFormFields(const QList<OCRTextRegion>& ocrHints, const cv::Mat& image);
    
    /**
     * @brief Set detection cache for performance optimization
     * @param cache Detection cache instance (can be nullptr to disable caching)
     */
    void setDetectionCache(class DetectionCache* cache);

private:
    /**
     * @brief Expand search area around text box
     * @param textBox Original text bounding box
     * @param radiusPercent Percentage of text height to expand
     * @param imgWidth Image width
     * @param imgHeight Image height
     * @return Expanded search area rectangle
     */
    cv::Rect expandSearchRadius(const cv::Rect& textBox, int radiusPercent, 
                                int imgWidth, int imgHeight);
    
    /**
     * @brief Detect horizontal and vertical lines around text
     * @param textBox Text bounding box
     * @param image Source image
     * @return List of detected line rectangles
     */
    QList<cv::Rect> detectLinesAroundText(const cv::Rect& textBox, const cv::Mat& image);
    
    /**
     * @brief Detect rectangles around text (form field boundaries)
     * @param textBox Text bounding box
     * @param image Source image
     * @return List of detected rectangle bounding boxes
     */
    QList<cv::Rect> detectRectanglesAroundText(const cv::Rect& textBox, const cv::Mat& image);
    
    /**
     * @brief Search for form field near label (below, right, or above)
     * @param labelBox Label bounding box
     * @param image Source image
     * @return Form field rectangle, or empty rect if not found
     */
    cv::Rect findFormFieldNearLabel(const cv::Rect& labelBox, const cv::Mat& image);
    
    /**
     * @brief Detect underline below text (common form field pattern)
     * @param textBox Text bounding box
     * @param image Source image
     * @return Underline rectangle, or empty rect if not found
     */
    cv::Rect detectUnderlineBelow(const cv::Rect& textBox, const cv::Mat& image);
    
    /**
     * @brief Detect form field to the right of label (horizontal forms)
     * @param labelBox Label bounding box
     * @param image Source image
     * @return Form field rectangle, or empty rect if not found
     */
    cv::Rect detectFormFieldToRight(const cv::Rect& labelBox, const cv::Mat& image);
    
    /**
     * @brief Detect empty box or underline in search area
     * @param searchArea Area to search in
     * @param image Source image
     * @return Detected form field rectangle, or empty rect if not found
     */
    cv::Rect detectFormFieldInArea(const cv::Rect& searchArea, const cv::Mat& image);
    
    /**
     * @brief Merge text box with detected lines and rectangles
     * @param textBox Original text box
     * @param lines Detected line rectangles
     * @param rects Detected rectangle bounding boxes
     * @param underlineRect Detected underline rectangle
     * @param hasUnderline Whether underline was detected
     * @return Merged bounding box (union of all)
     */
    cv::Rect mergeDetections(const cv::Rect& textBox, 
                             const QList<cv::Rect>& lines, 
                             const QList<cv::Rect>& rects,
                             const cv::Rect& underlineRect,
                             bool hasUnderline);
    
    /**
     * @brief Calculate line detection score
     * @param lines Detected lines
     * @param image Source image
     * @return Score 0.0-1.0
     */
    double calculateLineDetectionScore(const QList<cv::Rect>& lines, const cv::Mat& image);
    
    /**
     * @brief Calculate rectangularity score
     * @param rect Bounding rectangle
     * @param image Source image (for contour analysis)
     * @return Score 0.0-1.0
     */
    double calculateRectangularityScore(const cv::Rect& rect, const cv::Mat& image);
    
    /**
     * @brief Calculate horizontal edge density
     * @param roi Region of interest
     * @return Horizontal edge density (0.0-1.0)
     */
    double calculateHorizontalEdgeDensity(const cv::Mat& roi);
    
    /**
     * @brief Calculate vertical edge density
     * @param roi Region of interest
     * @return Vertical edge density (0.0-1.0)
     */
    double calculateVerticalEdgeDensity(const cv::Mat& roi);
    
    /**
     * @brief Count horizontal lines using Hough transform
     * @param edges Edge image
     * @param angleTolerance Angle tolerance in degrees (default: 5.0)
     * @return Number of horizontal lines detected
     */
    int countHorizontalLinesWithHough(const cv::Mat& edges, double angleTolerance = 5.0);
    
    int expansionRadiusPercent;  // Expansion radius as % of text height (default: 20)
    double lineDetectionScore;   // Cached line detection score
    double rectangularityScore;   // Cached rectangularity score
    
    // Detection cache for performance optimization (expert recommendation)
    class DetectionCache* detectionCache;  // Optional cache for expensive calculations
};

} // namespace ocr_orc

#endif // TEXT_REGION_REFINER_H
