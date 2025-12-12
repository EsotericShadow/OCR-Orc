#include "DocumentTypeClassifier.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

DocumentTypeClassifier::DocumentTypeClassifier()
    : classificationConfidence(0.0)
{
}

DocumentType DocumentTypeClassifier::classifyDocument(const cv::Mat& image)
{
    if (image.empty()) {
        return STANDARD_FORM;
    }
    
    // Convert to grayscale for analysis
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Analyze various characteristics
    double contrast = analyzeContrast(gray);
    double colorfulness = analyzeColorDistribution(image);
    double fieldDensity = analyzeFieldDensity(gray);
    double textChar = analyzeTextCharacteristics(gray);
    bool hasColored = hasColoredSections(image);
    bool handwritten = isHandwritten(gray);
    
    // Classification logic based on expert recommendations
    
    // Handwritten forms: Low contrast, smooth edges, guide lines
    if (handwritten && contrast < 0.4) {
        classificationConfidence = 0.75;
        return HANDWRITTEN_FORM;
    }
    
    // Government forms: High contrast, often colored sections, structured
    if (hasColored && contrast > 0.6 && fieldDensity > 0.3) {
        classificationConfidence = 0.80;
        return GOVERNMENT_FORM;
    }
    
    // Medical forms: Colored sections, varied layouts, medium contrast
    if (hasColored && contrast > 0.4 && contrast < 0.7 && colorfulness > 0.3) {
        classificationConfidence = 0.75;
        return MEDICAL_FORM;
    }
    
    // Tax forms: Very dense fields, high contrast, small fields
    if (fieldDensity > 0.5 && contrast > 0.65 && textChar > 0.4) {
        classificationConfidence = 0.70;
        return TAX_FORM;
    }
    
    // Default to standard form
    classificationConfidence = 0.60;
    return STANDARD_FORM;
}

QString DocumentTypeClassifier::documentTypeToString(DocumentType type)
{
    switch (type) {
        case GOVERNMENT_FORM: return "government_form";
        case MEDICAL_FORM: return "medical_form";
        case TAX_FORM: return "tax_form";
        case HANDWRITTEN_FORM: return "handwritten_form";
        case STANDARD_FORM: return "standard_form";
        default: return "unknown";
    }
}

double DocumentTypeClassifier::analyzeContrast(const cv::Mat& image)
{
    if (image.empty()) {
        return 0.0;
    }
    
    // Calculate standard deviation (higher = more contrast)
    cv::Scalar mean, stddev;
    cv::meanStdDev(image, mean, stddev);
    
    // Normalize to 0-1 range (assuming 8-bit image, max stddev ~128)
    double contrast = std::min(1.0, stddev[0] / 128.0);
    
    return contrast;
}

double DocumentTypeClassifier::analyzeColorDistribution(const cv::Mat& image)
{
    if (image.channels() != 3 || image.empty()) {
        return 0.0;
    }
    
    // Convert to HSV for color analysis
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    
    // Calculate saturation (colorfulness)
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::Mat saturation = channels[1];
    
    cv::Scalar meanSat = cv::mean(saturation);
    
    // Normalize to 0-1 (saturation is 0-255)
    return meanSat[0] / 255.0;
}

double DocumentTypeClassifier::analyzeFieldDensity(const cv::Mat& image)
{
    if (image.empty()) {
        return 0.0;
    }
    
    // Detect edges to estimate field density
    cv::Mat edges;
    cv::Canny(image, edges, 50, 150);
    
    // Find contours (potential form fields)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Count rectangular contours (potential form fields)
    int rectangularCount = 0;
    for (const auto& contour : contours) {
        if (contour.size() < 4) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        double area = cv::contourArea(contour);
        double rectArea = rect.width * rect.height;
        
        // Check if roughly rectangular and reasonable size (form field sized)
        if (rectArea > 0 && area / rectArea > 0.7 && 
            rect.width > 20 && rect.height > 10 && 
            rect.width < 500 && rect.height < 200) {
            rectangularCount++;
        }
    }
    
    // Normalize by image area (density = fields per 10000 pixels)
    double imageArea = image.rows * image.cols;
    double density = (rectangularCount * 10000.0) / imageArea;
    
    // Normalize to 0-1 (assuming max density of 10 fields per 10000 pixels)
    return std::min(1.0, density / 10.0);
}

double DocumentTypeClassifier::analyzeTextCharacteristics(const cv::Mat& image)
{
    if (image.empty()) {
        return 0.0;
    }
    
    // Use adaptive thresholding to detect text-like patterns
    cv::Mat adaptive;
    cv::adaptiveThreshold(image, adaptive, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY_INV, 11, 2);
    
    // Find horizontal lines (text lines)
    cv::Mat horizontal = adaptive.clone();
    int horizontalSize = horizontal.cols / 30;
    cv::Mat horizontalStructure = cv::getStructuringElement(cv::MORPH_RECT, 
                                                             cv::Size(horizontalSize, 1));
    cv::erode(horizontal, horizontal, horizontalStructure);
    cv::dilate(horizontal, horizontal, horizontalStructure);
    
    // Count horizontal lines
    int horizontalLines = 0;
    for (int y = 0; y < horizontal.rows; y++) {
        int lineLength = 0;
        for (int x = 0; x < horizontal.cols; x++) {
            if (horizontal.at<uchar>(y, x) > 0) {
                lineLength++;
            } else {
                if (lineLength > horizontal.cols * 0.1) {  // Line > 10% of width
                    horizontalLines++;
                }
                lineLength = 0;
            }
        }
        if (lineLength > horizontal.cols * 0.1) {
            horizontalLines++;
        }
    }
    
    // Normalize by image height (lines per 100 pixels)
    double textDensity = (horizontalLines * 100.0) / image.rows;
    
    // Normalize to 0-1 (assuming max of 5 lines per 100 pixels)
    return std::min(1.0, textDensity / 5.0);
}

bool DocumentTypeClassifier::hasColoredSections(const cv::Mat& image)
{
    if (image.channels() != 3 || image.empty()) {
        return false;
    }
    
    // Convert to HSV
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    
    // Split channels
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::Mat saturation = channels[1];
    cv::Mat value = channels[2];
    
    // Check for colored regions (high saturation, not too dark)
    cv::Mat colored;
    cv::threshold(saturation, colored, 50, 255, cv::THRESH_BINARY);
    cv::Mat notDark;
    cv::threshold(value, notDark, 100, 255, cv::THRESH_BINARY);
    
    cv::Mat coloredRegions;
    cv::bitwise_and(colored, notDark, coloredRegions);
    
    // Count colored pixels
    int coloredPixels = cv::countNonZero(coloredRegions);
    double coloredRatio = static_cast<double>(coloredPixels) / (image.rows * image.cols);
    
    // If >5% of image is colored, likely has colored sections
    return coloredRatio > 0.05;
}

bool DocumentTypeClassifier::isHandwritten(const cv::Mat& image)
{
    if (image.empty()) {
        return false;
    }
    
    // Handwritten text characteristics:
    // - Lower contrast (smoother strokes)
    // - More curved edges (less sharp)
    // - Less uniform spacing
    
    // Analyze edge characteristics
    cv::Mat edges;
    cv::Canny(image, edges, 30, 90);  // Lower thresholds for handwriting
    
    // Count edge pixels
    int edgePixels = cv::countNonZero(edges);
    double edgeDensity = static_cast<double>(edgePixels) / (image.rows * image.cols);
    
    // Handwriting has lower edge density (smoother) than printed text
    // Also check for guide lines (horizontal lines)
    cv::Mat horizontal;
    int horizontalSize = image.cols / 40;
    cv::Mat horizontalStructure = cv::getStructuringElement(cv::MORPH_RECT, 
                                                             cv::Size(horizontalSize, 1));
    cv::erode(edges, horizontal, horizontalStructure);
    cv::dilate(horizontal, horizontal, horizontalStructure);
    
    int guideLines = 0;
    for (int y = 0; y < horizontal.rows; y++) {
        int lineLength = 0;
        for (int x = 0; x < horizontal.cols; x++) {
            if (horizontal.at<uchar>(y, x) > 0) {
                lineLength++;
            } else {
                if (lineLength > image.cols * 0.3) {  // Long horizontal line
                    guideLines++;
                }
                lineLength = 0;
            }
        }
        if (lineLength > image.cols * 0.3) {
            guideLines++;
        }
    }
    
    // Handwritten if: low edge density AND has guide lines
    return edgeDensity < 0.08 && guideLines >= 2;
}

} // namespace ocr_orc
