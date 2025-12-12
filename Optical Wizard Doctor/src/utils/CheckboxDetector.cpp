#include "CheckboxDetector.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <QtCore/QList>

namespace ocr_orc {

CheckboxDetector::CheckboxDetector()
    : minCheckboxSize(10)   // Lowered from 20 - catch smaller checkboxes
    , maxCheckboxSize(60)   // Increased from 50 - catch larger checkboxes
    , fillThreshold(0.25)  // 25% fill threshold (between 20-30% range)
    , minAspectRatio(0.6)   // Minimum aspect ratio for standalone detection
    , maxAspectRatio(1.6)   // Maximum aspect ratio for standalone detection
    , minRectangularity(0.5) // Minimum rectangularity (50%)
{
    fprintf(stderr, "[CheckboxDetector::CheckboxDetector] Constructor - Default params: size=%d-%dpx, aspect=%.1f-%.1f, rect=%.2f\n",
            minCheckboxSize, maxCheckboxSize, minAspectRatio, maxAspectRatio, minRectangularity);
    fflush(stderr);
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

bool CheckboxDetector::hasFourCorners(const std::vector<cv::Point>& contour, double angleTolerance)
{
    if (contour.size() < 4) {
        return false;
    }
    
    // Approximate contour to polygon to reduce noise and get main vertices
    std::vector<cv::Point> approx;
    double epsilon = cv::arcLength(contour, true) * 0.02;  // 2% of perimeter
    cv::approxPolyDP(contour, approx, epsilon, true);
    
    // Must have exactly 4 vertices for a rectangle/checkbox
    if (approx.size() != 4) {
        fprintf(stderr, "[CheckboxDetector::hasFourCorners] Rejected: has %zu vertices (expected 4)\n", approx.size());
        fflush(stderr);
        return false;
    }
    
    // Calculate angles at each of the 4 corners
    QList<double> angles;
    for (int i = 0; i < 4; i++) {
        cv::Point p1 = approx[i];
        cv::Point p2 = approx[(i + 1) % 4];  // Current corner
        cv::Point p3 = approx[(i + 2) % 4];
        
        // Calculate vectors from corner to adjacent points
        double dx1 = p1.x - p2.x;
        double dy1 = p1.y - p2.y;
        double dx2 = p3.x - p2.x;
        double dy2 = p3.y - p2.y;
        
        // Calculate angle using dot product
        double dot = dx1 * dx2 + dy1 * dy2;
        double mag1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
        double mag2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
        
        if (mag1 > 0 && mag2 > 0) {
            double cosAngle = dot / (mag1 * mag2);
            cosAngle = std::max(-1.0, std::min(1.0, cosAngle));  // Clamp to valid range
            double angle = std::acos(cosAngle) * 180.0 / CV_PI;
            angles.append(angle);
        }
    }
    
    if (angles.size() != 4) {
        fprintf(stderr, "[CheckboxDetector::hasFourCorners] Failed to calculate all 4 angles\n");
        fflush(stderr);
        return false;
    }
    
    // Check if all angles are approximately 90 degrees (within tolerance)
    int validCorners = 0;
    for (double angle : angles) {
        double deviation = std::abs(angle - 90.0);
        if (deviation <= angleTolerance) {
            validCorners++;
        } else {
            fprintf(stderr, "[CheckboxDetector::hasFourCorners] Corner angle %.1f° deviates %.1f° from 90° (tolerance: %.1f°)\n",
                    angle, deviation, angleTolerance);
            fflush(stderr);
        }
    }
    
    // Require at least 3 out of 4 corners to be approximately 90 degrees
    // (allows for slight distortion or one imperfect corner)
    bool isValid = validCorners >= 3;
    
    if (!isValid) {
        fprintf(stderr, "[CheckboxDetector::hasFourCorners] Rejected: only %d/4 corners are ~90° (angles: %.1f°, %.1f°, %.1f°, %.1f°)\n",
                validCorners, angles[0], angles[1], angles[2], angles[3]);
        fflush(stderr);
    } else {
        fprintf(stderr, "[CheckboxDetector::hasFourCorners] ✓ Valid checkbox: %d/4 corners are ~90° (angles: %.1f°, %.1f°, %.1f°, %.1f°)\n",
                validCorners, angles[0], angles[1], angles[2], angles[3]);
        fflush(stderr);
    }
    
    return isValid;
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

QList<CheckboxDetection> CheckboxDetector::detectAllCheckboxes(const cv::Mat& image)
{
    QList<CheckboxDetection> results;
    
    if (image.empty()) {
        fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Image is empty, returning empty list\n");
        fflush(stderr);
        return results;
    }
    
    fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Starting standalone checkbox detection (image size: %dx%d)\n", 
            image.cols, image.rows);
    fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Using parameters - size: %d-%dpx, aspect: %.1f-%.1f, rectangularity: %.2f\n",
            minCheckboxSize, maxCheckboxSize, minAspectRatio, maxAspectRatio, minRectangularity);
    fflush(stderr);
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Use multiple thresholding methods to catch checkboxes with different contrast
    std::vector<cv::Mat> binaryImages;
    
    // Method 1: Standard binary threshold
    cv::Mat binary1;
    cv::threshold(gray, binary1, 127, 255, cv::THRESH_BINARY_INV);
    binaryImages.push_back(binary1);
    
    // Method 2: Adaptive threshold (catches varying lighting)
    cv::Mat binary2;
    cv::adaptiveThreshold(gray, binary2, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY_INV, 11, 2);
    binaryImages.push_back(binary2);
    
    // Method 3: Lower threshold (catches faint borders)
    cv::Mat binary3;
    cv::threshold(gray, binary3, 100, 255, cv::THRESH_BINARY_INV);
    binaryImages.push_back(binary3);
    
    // Method 4: Even lower threshold (catches very faint checkboxes)
    cv::Mat binary4;
    cv::threshold(gray, binary4, 80, 255, cv::THRESH_BINARY_INV);
    binaryImages.push_back(binary4);
    
    // Method 5: Otsu's threshold (automatic threshold selection)
    cv::Mat binary5;
    cv::threshold(gray, binary5, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    binaryImages.push_back(binary5);
    
    // Method 6: Canny edges (catches edge-based checkboxes) - use multiple thresholds
    cv::Mat edges1;
    cv::Canny(gray, edges1, 50, 150);
    binaryImages.push_back(edges1);
    
    cv::Mat edges2;
    cv::Canny(gray, edges2, 30, 100);  // Lower threshold for faint edges
    binaryImages.push_back(edges2);
    
    // Combine all binary images
    cv::Mat combined;
    binaryImages[0].copyTo(combined);
    for (size_t i = 1; i < binaryImages.size(); i++) {
        cv::bitwise_or(combined, binaryImages[i], combined);
    }
    
    // Find contours - use RETR_TREE to get all contours (including nested ones)
    // This helps find checkboxes that might be inside other shapes
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(combined, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    
    fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Found %zu contours to analyze\n", contours.size());
    if (contours.size() < 10) {
        fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] WARNING: Very few contours found (%zu). This might indicate thresholding issues.\n", contours.size());
        fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Consider adjusting thresholding methods or checking image quality.\n");
    }
    fflush(stderr);
    
    // Track detected checkboxes to avoid duplicates
    QList<cv::Rect> detectedRects;
    
    int validCheckboxes = 0;
    int rejectedSize = 0, rejectedAspect = 0, rejectedRect = 0, rejectedCorners = 0, rejectedTooSmall = 0;
    
    // Filter out very small contours first (likely noise)
    std::vector<std::vector<cv::Point>> filteredContours;
    for (const auto& contour : contours) {
        if (contour.size() < 4) {
            rejectedTooSmall++;
            continue;
        }
        
        cv::Rect rect = cv::boundingRect(contour);
        // Filter out extremely small contours (likely noise)
        if (rect.width < 3 || rect.height < 3) {
            rejectedTooSmall++;
            continue;
        }
        
        filteredContours.push_back(contour);
    }
    
    fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Filtered %zu contours (rejected %d too small)\n", 
            filteredContours.size(), rejectedTooSmall);
    fflush(stderr);
    
    for (const auto& contour : filteredContours) {
        
        cv::Rect rect = cv::boundingRect(contour);
        
        // Check size constraints (using configured parameters)
        // Check if the smaller dimension is within range (more lenient - allows slight variations)
        int minDim = std::min(rect.width, rect.height);
        int maxDim = std::max(rect.width, rect.height);
        
        // Reject if smaller dimension is too small OR larger dimension is way too large
        // (allows some flexibility for slightly non-square checkboxes)
        if (minDim < minCheckboxSize) {
            rejectedSize++;
            fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Rejected contour: too small - size %dx%d (minDim: %d < min: %d)\n",
                    rect.width, rect.height, minDim, minCheckboxSize);
            fflush(stderr);
            continue;
        }
        
        // Reject if larger dimension is way too large (likely not a checkbox)
        if (maxDim > maxCheckboxSize * 2) {
            rejectedSize++;
            fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Rejected contour: too large - size %dx%d (maxDim: %d > max*2: %d)\n",
                    rect.width, rect.height, maxDim, maxCheckboxSize * 2);
            fflush(stderr);
            continue;
        }
        
        // Also reject if both dimensions are way too large (safety check)
        if (rect.width > maxCheckboxSize * 3 || rect.height > maxCheckboxSize * 3) {
            rejectedSize++;
            fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Rejected contour: extremely large - size %dx%d (max: %d)\n",
                    rect.width, rect.height, maxCheckboxSize);
            fflush(stderr);
            continue;
        }
        
        // Check aspect ratio (using configured parameters)
        double aspectRatio = static_cast<double>(rect.width) / rect.height;
        if (aspectRatio < minAspectRatio || aspectRatio > maxAspectRatio) {
            rejectedAspect++;
            fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Rejected contour: aspect ratio %.2f (range: %.1f-%.1f, rect: %dx%d)\n",
                    aspectRatio, minAspectRatio, maxAspectRatio, rect.width, rect.height);
            fflush(stderr);
            continue;
        }
        
        // Check rectangularity (using configured threshold)
        double contourArea = cv::contourArea(contour);
        double rectArea = rect.width * rect.height;
        double rectangularity = rectArea > 0 ? contourArea / rectArea : 0.0;
        if (rectangularity < minRectangularity) {
            rejectedRect++;
            fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Rejected contour: rectangularity %.2f (min: %.2f, rect: %dx%d, area: %.0f/%.0f)\n",
                    rectangularity, minRectangularity, rect.width, rect.height, contourArea, rectArea);
            fflush(stderr);
            continue;
        }
        
        // CRITICAL: Validate that contour has 4 corners at approximately 90 degrees
        // This filters out text regions which won't have this structure
        if (!hasFourCorners(contour, 20.0)) {  // 20 degree tolerance for 90-degree angles
            rejectedCorners++;
            fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Rejected contour: does not have 4 corners at ~90 degrees (rect: %dx%d)\n",
                    rect.width, rect.height);
            fflush(stderr);
            continue;
        }
        
        // Check if this checkbox overlaps significantly with already detected ones
        bool isDuplicate = false;
        for (const cv::Rect& existing : detectedRects) {
            // Calculate IoU (Intersection over Union)
            int x1 = std::max(rect.x, existing.x);
            int y1 = std::max(rect.y, existing.y);
            int x2 = std::min(rect.x + rect.width, existing.x + existing.width);
            int y2 = std::min(rect.y + rect.height, existing.y + existing.height);
            
            if (x2 > x1 && y2 > y1) {
                int intersection = (x2 - x1) * (y2 - y1);
                int unionArea = rectArea + (existing.width * existing.height) - intersection;
                double iou = unionArea > 0 ? static_cast<double>(intersection) / unionArea : 0.0;
                
                if (iou > 0.5) {  // More than 50% overlap = duplicate
                    isDuplicate = true;
                    break;
                }
            }
        }
        
        if (isDuplicate) {
            continue;
        }
        
        // Valid checkbox found!
        CheckboxDetection detection;
        detection.detected = true;
        detection.boundingBox = rect;
        detection.placement = "standalone";
        detection.confidence = rectangularity * (1.0 - std::abs(1.0 - aspectRatio) / 0.4);  // Higher for more square
        
        // Detect checkbox state
        cv::Mat checkboxROI = gray(rect);
        detection.isChecked = detectCheckboxState(checkboxROI);
        
        results.append(detection);
        detectedRects.append(rect);
        validCheckboxes++;
    }
    
    fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] ✓ Detected %d standalone checkboxes (from %zu contours)\n", 
            validCheckboxes, filteredContours.size());
    fprintf(stderr, "[CheckboxDetector::detectAllCheckboxes] Rejection stats: tooSmall=%d, size=%d, aspect=%d, rectangularity=%d, corners=%d\n",
            rejectedTooSmall, rejectedSize, rejectedAspect, rejectedRect, rejectedCorners);
    fflush(stderr);
    
    return results;
}

} // namespace ocr_orc
