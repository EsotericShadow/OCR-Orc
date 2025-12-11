#include "ImageConverter.h"
#include <QtGui/QImage>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

cv::Mat ImageConverter::qImageToMat(const QImage& qImage) {
    if (qImage.isNull()) {
        return cv::Mat();
    }
    
    // Convert QImage to RGB888 format if needed
    QImage rgbImage = qImage;
    if (qImage.format() != QImage::Format_RGB888) {
        rgbImage = qImage.convertToFormat(QImage::Format_RGB888);
    }
    
    // Get image dimensions
    int width = rgbImage.width();
    int height = rgbImage.height();
    int bytesPerLine = rgbImage.bytesPerLine();
    
    // Create cv::Mat header pointing to QImage data (shallow copy first)
    cv::Mat mat(height, width, CV_8UC3, (void*)rgbImage.constBits(), bytesPerLine);
    
    // Convert RGB to BGR (OpenCV uses BGR)
    cv::Mat bgrMat;
    cv::cvtColor(mat, bgrMat, cv::COLOR_RGB2BGR);
    
    // Return deep copy to ensure memory safety
    return bgrMat.clone();
}

QImage ImageConverter::matToQImage(const cv::Mat& mat) {
    if (mat.empty()) {
        return QImage();
    }
    
    // Handle different cv::Mat types
    cv::Mat rgbMat;
    
    switch (mat.type()) {
        case CV_8UC1: // Grayscale
        {
            // Convert grayscale to RGB
            cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
            break;
        }
        case CV_8UC3: // BGR
        {
            // Convert BGR to RGB
            cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
            break;
        }
        case CV_8UC4: // BGRA
        {
            // Convert BGRA to RGB
            cv::cvtColor(mat, rgbMat, cv::COLOR_BGRA2RGB);
            break;
        }
        default:
        {
            #if OCR_ORC_DEBUG_ENABLED
            qDebug() << "[ImageConverter] Unsupported cv::Mat type:" << mat.type();
            #endif
            return QImage();
        }
    }
    
    // Create QImage from cv::Mat data
    QImage qImage(rgbMat.data, rgbMat.cols, rgbMat.rows, 
                  rgbMat.step, QImage::Format_RGB888);
    
    // Return deep copy to ensure memory safety
    return qImage.copy();
}

} // namespace ocr_orc
