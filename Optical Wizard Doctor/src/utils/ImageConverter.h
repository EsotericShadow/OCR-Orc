#ifndef IMAGE_CONVERTER_H
#define IMAGE_CONVERTER_H

#include <QtGui/QImage>
#include <opencv2/opencv.hpp>

namespace ocr_orc {

/**
 * @brief Utility functions for converting between QImage and cv::Mat
 * 
 * Handles format conversion and memory management between Qt and OpenCV image formats.
 * All conversions perform deep copies to ensure memory safety.
 */
class ImageConverter {
public:
    /**
     * @brief Convert QImage to cv::Mat
     * @param qImage Source QImage (any format)
     * @return cv::Mat in BGR format (OpenCV standard)
     * 
     * Performs deep copy. Handles ARGB32, RGB888, and Grayscale formats.
     */
    static cv::Mat qImageToMat(const QImage& qImage);
    
    /**
     * @brief Convert cv::Mat to QImage
     * @param mat Source cv::Mat (any type)
     * @return QImage in appropriate format
     * 
     * Performs deep copy. Handles CV_8UC1 (grayscale), CV_8UC3 (BGR), CV_8UC4 (BGRA).
     * Converts BGR to RGB for Qt display.
     */
    static QImage matToQImage(const cv::Mat& mat);
};

} // namespace ocr_orc

#endif // IMAGE_CONVERTER_H
