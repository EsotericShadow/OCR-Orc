#include "CheckboxDetector.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

CheckboxDetector::CheckboxDetector()
    : minCheckboxSize(20)
    , maxCheckboxSize(50)
    , fillThreshold(0.25)  // 25% fill threshold (between 20-30% range)
{
}

CheckboxDetection CheckboxDetector::detectCheckbox(const OCRTextRegion& textRegion, const cv::Mat& image)
{
    CheckboxDetection result;
    
    if (image.empty()) {
        return result;
    }
    
    cv::Rect textBox = textRegion.boundingBox;
    
    // Search left side first
    CheckboxDetection leftCheckbox = searchLeftSide(textBox, image);
    
    // Search right side
    CheckboxDetection rightCheckbox = searchRightSide(textBox, image);
    
    // Choose the best detection (closer to text, higher confidence)
    if (leftCheckbox.detected && rightCheckbox.detected) {
        // Choose the one closer to text center
        int textCenterX = textBox.x + textBox.width / 2;
        int leftDist = std::abs(leftCheckbox.boundingBox.x + leftCheckbox.boundingBox.width / 2 - textCenterX);
        int rightDist = std::abs(rightCheckbox.boundingBox.x + rightCheckbox.boundingBox.width / 2 - textCenterX);
        
        result = (leftDist < rightDist) ? leftCheckbox : rightCheckbox;
    } else if (leftCheckbox.detected) {
        result = leftCheckbox;
    } else if (rightCheckbox.detected) {
        result = rightCheckbox;
    }
    
    return result;
}

CheckboxDetection CheckboxDetector::searchLeftSide(const cv::Rect& textBox, const cv::Mat& image)
{
    CheckboxDetection result;
    
    // Define search area: left of text
    // Width: 15% of text height, Height: 120% of text height
    int searchWidth = static_cast<int>(textBox.height * 0.15);
    int searchHeight = static_cast<int>(textBox.height * 1.2);
    int gap = 10;  // 10px gap from text
    
    cv::Rect searchArea(
        std::max(0, textBox.x - searchWidth - gap),
        std::max(0, textBox.y - (searchHeight - textBox.height) / 2),
        searchWidth,
        std::min(searchHeight, image.rows - std::max(0, textBox.y - (searchHeight - textBox.height) / 2))
    );
    
    if (searchArea.width <= 0 || searchArea.height <= 0) {
        return result;
    }
    
    // Clamp to image bounds
    searchArea.x = std::max(0, std::min(searchArea.x, image.cols - 1));
    searchArea.y = std::max(0, std::min(searchArea.y, image.rows - 1));
    searchArea.width = std::min(searchArea.width, image.cols - searchArea.x);
    searchArea.height = std::min(searchArea.height, image.rows - searchArea.y);
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Apply binary thresholding
    cv::Mat binary;
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY_INV);
    
    // Find contours in search area
    cv::Mat roi = binary(searchArea);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(roi, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Find best checkbox candidate
    double bestScore = 0.0;
    cv::Rect bestRect;
    
    for (const auto& contour : contours) {
        if (contour.size() < 4) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        
        // Adjust coordinates to full image
        rect.x += searchArea.x;
        rect.y += searchArea.y;
        
        // Check size constraints
        if (rect.width < minCheckboxSize || rect.width > maxCheckboxSize ||
            rect.height < minCheckboxSize || rect.height > maxCheckboxSize) {
            continue;
        }
        
        // Check aspect ratio (square: 0.8-1.2)
        double aspectRatio = static_cast<double>(rect.width) / rect.height;
        if (aspectRatio < 0.8 || aspectRatio > 1.2) {
            continue;
        }
        
        if (!isCheckboxShape(rect, aspectRatio)) {
            continue;
        }
        
        // Check proximity: within 2x text height distance
        int textCenterY = textBox.y + textBox.height / 2;
        int checkboxCenterY = rect.y + rect.height / 2;
        int distanceY = std::abs(checkboxCenterY - textCenterY);
        
        if (distanceY > textBox.height * 2) {
            continue;
        }
        
        // Check Y-center alignment: within 30% of text height
        if (distanceY > textBox.height * 0.3) {
            continue;
        }
        
        // Calculate score (closer = better, more square = better)
        double proximityScore = 1.0 - (static_cast<double>(distanceY) / (textBox.height * 2.0));
        double squareScore = 1.0 - std::abs(1.0 - aspectRatio) / 0.2;  // Penalize non-square
        double score = (proximityScore + squareScore) / 2.0;
        
        if (score > bestScore) {
            bestScore = score;
            bestRect = rect;
        }
    }
    
    if (bestScore > 0.0) {
        result.detected = true;
        result.boundingBox = bestRect;
        result.placement = "left";
        result.confidence = bestScore;
        
        // Detect checkbox state
        cv::Mat checkboxROI = gray(bestRect);
        result.isChecked = detectCheckboxState(checkboxROI);
    }
    
    return result;
}

CheckboxDetection CheckboxDetector::searchRightSide(const cv::Rect& textBox, const cv::Mat& image)
{
    CheckboxDetection result;
    
    // Define search area: right of text
    // Width: 15% of text height, Height: 120% of text height
    int searchWidth = static_cast<int>(textBox.height * 0.15);
    int searchHeight = static_cast<int>(textBox.height * 1.2);
    int gap = 10;  // 10px gap from text
    
    cv::Rect searchArea(
        std::min(image.cols - searchWidth, textBox.x + textBox.width + gap),
        std::max(0, textBox.y - (searchHeight - textBox.height) / 2),
        searchWidth,
        std::min(searchHeight, image.rows - std::max(0, textBox.y - (searchHeight - textBox.height) / 2))
    );
    
    if (searchArea.width <= 0 || searchArea.height <= 0) {
        return result;
    }
    
    // Clamp to image bounds
    searchArea.x = std::max(0, std::min(searchArea.x, image.cols - 1));
    searchArea.y = std::max(0, std::min(searchArea.y, image.rows - 1));
    searchArea.width = std::min(searchArea.width, image.cols - searchArea.x);
    searchArea.height = std::min(searchArea.height, image.rows - searchArea.y);
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Apply binary thresholding
    cv::Mat binary;
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY_INV);
    
    // Find contours in search area
    cv::Mat roi = binary(searchArea);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(roi, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Find best checkbox candidate
    double bestScore = 0.0;
    cv::Rect bestRect;
    
    for (const auto& contour : contours) {
        if (contour.size() < 4) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        
        // Adjust coordinates to full image
        rect.x += searchArea.x;
        rect.y += searchArea.y;
        
        // Check size constraints
        if (rect.width < minCheckboxSize || rect.width > maxCheckboxSize ||
            rect.height < minCheckboxSize || rect.height > maxCheckboxSize) {
            continue;
        }
        
        // Check aspect ratio (square: 0.8-1.2)
        double aspectRatio = static_cast<double>(rect.width) / rect.height;
        if (aspectRatio < 0.8 || aspectRatio > 1.2) {
            continue;
        }
        
        if (!isCheckboxShape(rect, aspectRatio)) {
            continue;
        }
        
        // Check proximity: within 2x text height distance
        int textCenterY = textBox.y + textBox.height / 2;
        int checkboxCenterY = rect.y + rect.height / 2;
        int distanceY = std::abs(checkboxCenterY - textCenterY);
        
        if (distanceY > textBox.height * 2) {
            continue;
        }
        
        // Check Y-center alignment: within 30% of text height
        if (distanceY > textBox.height * 0.3) {
            continue;
        }
        
        // Calculate score (closer = better, more square = better)
        double proximityScore = 1.0 - (static_cast<double>(distanceY) / (textBox.height * 2.0));
        double squareScore = 1.0 - std::abs(1.0 - aspectRatio) / 0.2;  // Penalize non-square
        double score = (proximityScore + squareScore) / 2.0;
        
        if (score > bestScore) {
            bestScore = score;
            bestRect = rect;
        }
    }
    
    if (bestScore > 0.0) {
        result.detected = true;
        result.boundingBox = bestRect;
        result.placement = "right";
        result.confidence = bestScore;
        
        // Detect checkbox state
        cv::Mat checkboxROI = gray(bestRect);
        result.isChecked = detectCheckboxState(checkboxROI);
    }
    
    return result;
}

bool CheckboxDetector::isCheckboxShape(const cv::Rect& rect, double aspectRatio)
{
    // Already checked aspect ratio (0.8-1.2) in calling code
    // Additional validation: rectangularity
    // For now, if aspect ratio is good, assume it's checkbox-shaped
    // Can be enhanced with contour analysis if needed
    return true;
}

bool CheckboxDetector::detectCheckboxState(const cv::Mat& checkboxROI)
{
    if (checkboxROI.empty()) {
        return false;
    }
    
    // Apply binary thresholding
    cv::Mat binary;
    cv::threshold(checkboxROI, binary, 127, 255, cv::THRESH_BINARY_INV);
    
    // Count black (non-zero) pixels
    int blackPixels = cv::countNonZero(binary);
    int totalPixels = checkboxROI.rows * checkboxROI.cols;
    
    if (totalPixels == 0) {
        return false;
    }
    
    // Calculate fill percentage
    double fillPercentage = static_cast<double>(blackPixels) / totalPixels;
    
    // If >threshold filled, likely checked
    return fillPercentage > fillThreshold;
}

QString CheckboxDetector::determinePlacement(const cv::Rect& checkbox, const cv::Rect& text)
{
    int textCenterX = text.x + text.width / 2;
    int checkboxCenterX = checkbox.x + checkbox.width / 2;
    
    if (checkboxCenterX < textCenterX) {
        return "left";
    } else if (checkboxCenterX > textCenterX) {
        return "right";
    } else {
        return "unknown";
    }
}

} // namespace ocr_orc
