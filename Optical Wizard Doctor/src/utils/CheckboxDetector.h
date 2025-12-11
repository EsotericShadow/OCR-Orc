#ifndef CHECKBOX_DETECTOR_H
#define CHECKBOX_DETECTOR_H

#include "OcrTextExtractor.h"
#include <opencv2/opencv.hpp>
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Checkbox detection result
 */
struct CheckboxDetection {
    cv::Rect boundingBox;      // Checkbox bounding box
    bool detected;              // Was checkbox found?
    QString placement;          // "left", "right", "unknown"
    bool isChecked;             // Checked or unchecked?
    double confidence;          // Detection confidence (0.0-1.0)
    
    CheckboxDetection() : detected(false), placement("unknown"), isChecked(false), confidence(0.0) {}
};

/**
 * @brief Checkbox detector for form processing
 * 
 * Detects checkboxes near text regions and determines their state
 * (checked/unchecked) and placement (left/right of text).
 */
class CheckboxDetector {
public:
    CheckboxDetector();
    ~CheckboxDetector() = default;
    
    /**
     * @brief Detect checkbox near text region
     * @param textRegion OCR text region
     * @param image Source image
     * @return Checkbox detection result
     */
    CheckboxDetection detectCheckbox(const OCRTextRegion& textRegion, const cv::Mat& image);
    
    /**
     * @brief Set checkbox size range
     * @param minSize Minimum checkbox size in pixels
     * @param maxSize Maximum checkbox size in pixels
     */
    void setSizeRange(int minSize, int maxSize) {
        minCheckboxSize = minSize;
        maxCheckboxSize = maxSize;
    }
    
    /**
     * @brief Set fill percentage threshold for checked state
     * @param threshold Fill percentage threshold (0.0-1.0)
     */
    void setFillThreshold(double threshold) { fillThreshold = threshold; }

private:
    /**
     * @brief Search for checkbox on left side of text
     * @param textBox Text bounding box
     * @param image Source image
     * @return Detected checkbox or empty detection
     */
    CheckboxDetection searchLeftSide(const cv::Rect& textBox, const cv::Mat& image);
    
    /**
     * @brief Search for checkbox on right side of text
     * @param textBox Text bounding box
     * @param image Source image
     * @return Detected checkbox or empty detection
     */
    CheckboxDetection searchRightSide(const cv::Rect& textBox, const cv::Mat& image);
    
    /**
     * @brief Validate if rectangle is checkbox-shaped
     * @param rect Rectangle to validate
     * @param aspectRatio Aspect ratio of rectangle
     * @return True if valid checkbox shape
     */
    bool isCheckboxShape(const cv::Rect& rect, double aspectRatio);
    
    /**
     * @brief Detect checkbox state (checked/unchecked)
     * @param checkboxROI Checkbox region of interest
     * @return True if checked, false if unchecked
     */
    bool detectCheckboxState(const cv::Mat& checkboxROI);
    
    /**
     * @brief Determine checkbox placement relative to text
     * @param checkbox Checkbox bounding box
     * @param text Text bounding box
     * @return "left", "right", or "unknown"
     */
    QString determinePlacement(const cv::Rect& checkbox, const cv::Rect& text);
    
    int minCheckboxSize;      // Minimum checkbox size (default: 20)
    int maxCheckboxSize;      // Maximum checkbox size (default: 50)
    double fillThreshold;     // Fill percentage threshold for checked (default: 0.2-0.3)
};

} // namespace ocr_orc

#endif // CHECKBOX_DETECTOR_H
