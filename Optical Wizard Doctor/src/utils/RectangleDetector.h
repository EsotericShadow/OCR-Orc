#ifndef RECTANGLE_DETECTOR_H
#define RECTANGLE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <QtCore/QList>
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Detected rectangle/square from CV detection
 */
struct DetectedRectangle {
    cv::Rect boundingBox;      // Rectangle bounding box
    double confidence;          // Detection confidence (0.0-1.0)
    double rectangularity;      // How rectangular (0.0-1.0)
    bool isSquare;              // True if approximately square
    QString type;               // "square", "rectangle", "cell", "form_field"
    
    DetectedRectangle() : confidence(0.0), rectangularity(0.0), isSquare(false), type("unknown") {}
};

/**
 * @brief Sensitive rectangle/square detector
 * 
 * Detects squares and rectangles in images using sensitive CV techniques.
 * Runs as a secondary pipeline to complement OCR-first detection.
 */
class RectangleDetector {
public:
    RectangleDetector();
    ~RectangleDetector() = default;
    
    /**
     * @brief Detect all rectangles and squares in image
     * @param image Source image
     * @return List of detected rectangles/squares
     */
    QList<DetectedRectangle> detectRectangles(const cv::Mat& image);
    
    /**
     * @brief Set minimum rectangle size
     * @param minWidth Minimum width
     * @param minHeight Minimum height
     */
    void setMinSize(int minWidth, int minHeight) {
        minWidth_ = minWidth;
        minHeight_ = minHeight;
    }
    
    /**
     * @brief Set maximum rectangle size
     * @param maxWidth Maximum width
     * @param maxHeight Maximum height
     */
    void setMaxSize(int maxWidth, int maxHeight) {
        maxWidth_ = maxWidth;
        maxHeight_ = maxHeight;
    }
    
    /**
     * @brief Set sensitivity (lower = more sensitive)
     * @param sensitivity Sensitivity level (0.0-1.0, default: 0.3)
     */
    void setSensitivity(double sensitivity) { sensitivity_ = sensitivity; }

private:
    /**
     * @brief Find rectangular contours using sensitive detection
     * @param image Source image
     * @return List of rectangular contours
     */
    QList<cv::Rect> findRectangularContours(const cv::Mat& image);
    
    /**
     * @brief Calculate rectangularity score
     * @param contour Contour to analyze
     * @return Score 0.0-1.0
     */
    double calculateRectangularity(const std::vector<cv::Point>& contour);
    
    /**
     * @brief Check if rectangle is approximately square
     * @param rect Rectangle to check
     * @return True if square (aspect ratio 0.8-1.2)
     */
    bool isSquare(const cv::Rect& rect);
    
    /**
     * @brief Filter rectangles by size and quality
     * @param rects List of rectangles
     * @return Filtered list
     */
    QList<DetectedRectangle> filterRectangles(const QList<DetectedRectangle>& rects);
    
    int minWidth_;      // Minimum width (default: 20)
    int minHeight_;     // Minimum height (default: 15)
    int maxWidth_;      // Maximum width (default: 500)
    int maxHeight_;     // Maximum height (default: 200)
    double sensitivity_; // Detection sensitivity (default: 0.3)
};

} // namespace ocr_orc

#endif // RECTANGLE_DETECTOR_H
