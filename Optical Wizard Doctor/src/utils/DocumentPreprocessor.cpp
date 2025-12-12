#include "DocumentPreprocessor.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <QtCore/QList>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

DocumentPreprocessor::DocumentPreprocessor()
    : enableSkewCorrection(true)
    , enableDenoising(true)
    , enableShadowRemoval(true)
    , enableContrastEnhancement(true)
{
}

cv::Mat DocumentPreprocessor::preprocess(const cv::Mat& image)
{
    if (image.empty()) {
        return image;
    }
    
    cv::Mat processed = image.clone();
    
    // Apply preprocessing steps in order (expert recommendation)
    
    // 1. Denoise first (before other operations)
    if (enableDenoising) {
        processed = denoise(processed);
    }
    
    // 2. Remove shadows
    if (enableShadowRemoval) {
        processed = removeShadows(processed);
    }
    
    // 3. Enhance contrast
    if (enableContrastEnhancement) {
        processed = enhanceContrast(processed);
    }
    
    // 4. Correct skew last (after other enhancements)
    if (enableSkewCorrection) {
        processed = correctSkew(processed);
    }
    
    return processed;
}

cv::Mat DocumentPreprocessor::correctSkew(const cv::Mat& image)
{
    if (image.empty()) {
        return image;
    }
    
    double angle = detectSkewAngle(image);
    
    // Only correct if skew is significant (>0.5 degrees)
    if (std::abs(angle) < 0.5) {
        return image;
    }
    
    return rotateImage(image, -angle);  // Negative to correct
}

double DocumentPreprocessor::detectSkewAngle(const cv::Mat& image)
{
    if (image.empty()) {
        return 0.0;
    }
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Apply edge detection
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);
    
    // Use HoughLines to detect lines
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(edges, lines, 1, CV_PI / 180, 100);
    
    if (lines.empty()) {
        return 0.0;
    }
    
    // Calculate angles of detected lines
    QList<double> angles;
    for (const cv::Vec2f& line : lines) {
        // double rho = line[0];  // Unused
        double theta = line[1];
        
        // Convert to angle in degrees
        double angle = (theta * 180.0 / CV_PI) - 90.0;
        angles.append(angle);
    }
    
    // Find median angle (most common skew angle)
    std::sort(angles.begin(), angles.end());
    double medianAngle = 0.0;
    if (!angles.isEmpty()) {
        int mid = angles.size() / 2;
        if (angles.size() % 2 == 0) {
            medianAngle = (angles[mid - 1] + angles[mid]) / 2.0;
        } else {
            medianAngle = angles[mid];
        }
    }
    
    return medianAngle;
}

cv::Mat DocumentPreprocessor::rotateImage(const cv::Mat& image, double angle)
{
    if (std::abs(angle) < 0.1) {
        return image;
    }
    
    // Get image center
    cv::Point2f center(image.cols / 2.0, image.rows / 2.0);
    
    // Get rotation matrix
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
    
    // Calculate new image size
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), image.size(), angle).boundingRect2f();
    rotationMatrix.at<double>(0, 2) += bbox.width / 2.0 - image.cols / 2.0;
    rotationMatrix.at<double>(1, 2) += bbox.height / 2.0 - image.rows / 2.0;
    
    // Rotate image
    cv::Mat rotated;
    cv::warpAffine(image, rotated, rotationMatrix, bbox.size());
    
    return rotated;
}

cv::Mat DocumentPreprocessor::denoise(const cv::Mat& image)
{
    if (image.empty()) {
        return image;
    }
    
    cv::Mat denoised;
    
    // Use bilateral filter for edge-preserving denoising (expert recommendation)
    if (image.channels() == 3) {
        cv::bilateralFilter(image, denoised, 9, 75, 75);
    } else {
        // For grayscale, use Gaussian blur (bilateral filter also works)
        cv::GaussianBlur(image, denoised, cv::Size(3, 3), 0);
    }
    
    return denoised;
}

cv::Mat DocumentPreprocessor::removeShadows(const cv::Mat& image)
{
    if (image.empty()) {
        return image;
    }
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Use morphological operations to estimate background (shadow areas)
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));
    cv::Mat background;
    cv::morphologyEx(gray, background, cv::MORPH_CLOSE, kernel);
    
    // Normalize by background to remove shadows
    cv::Mat normalized;
    cv::divide(gray, background, normalized, 255.0);
    
    // Convert back to original format
    cv::Mat result;
    if (image.channels() == 3) {
        // Apply to each channel
        std::vector<cv::Mat> channels;
        cv::split(image, channels);
        
        for (cv::Mat& channel : channels) {
            cv::Mat channelGray;
            if (channel.channels() == 1) {
                channelGray = channel;
            } else {
                cv::cvtColor(channel, channelGray, cv::COLOR_BGR2GRAY);
            }
            
            cv::Mat channelBackground;
            cv::morphologyEx(channelGray, channelBackground, cv::MORPH_CLOSE, kernel);
            cv::divide(channelGray, channelBackground, channel, 255.0);
        }
        
        cv::merge(channels, result);
    } else {
        result = normalized;
    }
    
    return result;
}

cv::Mat DocumentPreprocessor::enhanceContrast(const cv::Mat& image)
{
    if (image.empty()) {
        return image;
    }
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Use CLAHE (Contrast Limited Adaptive Histogram Equalization) - expert recommendation
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    cv::Mat enhanced;
    clahe->apply(gray, enhanced);
    
    // Convert back to original format
    cv::Mat result;
    if (image.channels() == 3) {
        // Apply CLAHE to each channel
        std::vector<cv::Mat> channels;
        cv::split(image, channels);
        
        for (cv::Mat& channel : channels) {
            cv::Mat channelGray;
            if (channel.channels() == 1) {
                channelGray = channel;
            } else {
                cv::cvtColor(channel, channelGray, cv::COLOR_BGR2GRAY);
            }
            
            cv::Mat channelEnhanced;
            clahe->apply(channelGray, channelEnhanced);
            channel = channelEnhanced;
        }
        
        cv::merge(channels, result);
    } else {
        result = enhanced;
    }
    
    return result;
}

void DocumentPreprocessor::setPreprocessingOptions(bool enableSkew, bool enableDenoise,
                                                 bool enableShadowRemoval, bool enableContrast)
{
    enableSkewCorrection = enableSkew;
    enableDenoising = enableDenoise;
    this->enableShadowRemoval = enableShadowRemoval;
    enableContrastEnhancement = enableContrast;
}

} // namespace ocr_orc
