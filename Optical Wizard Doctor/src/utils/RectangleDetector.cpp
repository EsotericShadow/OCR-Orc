#include "RectangleDetector.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

RectangleDetector::RectangleDetector()
    : minWidth_(15)      // Lowered from 20 - catch smaller rectangles
    , minHeight_(10)     // Lowered from 15 - catch smaller rectangles
    , maxWidth_(800)     // Increased from 500 - catch larger form fields
    , maxHeight_(300)    // Increased from 200 - catch taller fields
    , sensitivity_(0.15)  // MUCH more sensitive (lowered from 0.3) - accept more candidates
{
}

QList<DetectedRectangle> RectangleDetector::detectRectangles(const cv::Mat& image)
{
    QList<DetectedRectangle> rectangles;
    
    if (image.empty()) {
        return rectangles;
    }
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // MULTIPLE SENSITIVE DETECTION METHODS
    
    // Method 1: Canny edge detection with VERY low thresholds (extremely sensitive)
    cv::Mat edges1, edges2, edges3;
    cv::Canny(gray, edges1, 10, 30);   // Ultra-sensitive
    cv::Canny(gray, edges2, 20, 60);   // Very sensitive
    cv::Canny(gray, edges3, 30, 90);   // Sensitive
    cv::Mat edges;
    cv::bitwise_or(edges1, edges2, edges);
    cv::bitwise_or(edges, edges3, edges);
    
    // Method 2: Adaptive thresholding (catches varying lighting)
    cv::Mat adaptive;
    cv::adaptiveThreshold(gray, adaptive, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY_INV, 11, 2);
    
    // Method 3: Regular thresholding with MANY thresholds (more aggressive)
    cv::Mat binary1, binary2, binary3, binary4, binary5;
    cv::threshold(gray, binary1, 80, 255, cv::THRESH_BINARY_INV);   // Lower threshold
    cv::threshold(gray, binary2, 120, 255, cv::THRESH_BINARY_INV);
    cv::threshold(gray, binary3, 160, 255, cv::THRESH_BINARY_INV);
    cv::threshold(gray, binary4, 200, 255, cv::THRESH_BINARY_INV);
    cv::threshold(gray, binary5, 240, 255, cv::THRESH_BINARY_INV);  // Higher threshold
    
    // Combine all methods (more aggressive combination)
    cv::Mat combined;
    cv::bitwise_or(edges, adaptive, combined);
    cv::bitwise_or(combined, binary1, combined);
    cv::bitwise_or(combined, binary2, combined);
    cv::bitwise_or(combined, binary3, combined);
    cv::bitwise_or(combined, binary4, combined);
    cv::bitwise_or(combined, binary5, combined);
    
    // Use MORE AGGRESSIVE morphological operations to connect broken edges
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));  // Larger kernel
    cv::Mat closed;
    cv::morphologyEx(combined, closed, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 3);  // More iterations
    
    // Also try dilation to fill gaps
    cv::Mat dilated;
    cv::dilate(closed, dilated, kernel, cv::Point(-1, -1), 2);
    cv::bitwise_or(closed, dilated, closed);
    
    // Find contours - use BOTH external and internal (RETR_TREE) to catch nested rectangles
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(closed, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        if (contour.size() < 4) continue;  // Need at least 4 points for rectangle
        
        // Calculate rectangularity
        double rectScore = calculateRectangularity(contour);
        
        // MUCH more lenient filtering - accept even imperfect rectangles
        // Also accept based on bounding rect area ratio (even if not perfect rectangle)
        cv::Rect testRect = cv::boundingRect(contour);
        double contourArea = cv::contourArea(contour);
        double rectArea = testRect.width * testRect.height;
        double areaRatio = rectArea > 0 ? contourArea / rectArea : 0.0;
        
        // Accept if: good rectangularity OR decent area ratio (overfitted acceptance)
        if (rectScore < sensitivity_ && areaRatio < 0.4) continue;  // Very lenient
        
        cv::Rect rect = cv::boundingRect(contour);
        
        // Filter by size
        if (rect.width < minWidth_ || rect.height < minHeight_ ||
            rect.width > maxWidth_ || rect.height > maxHeight_) {
            continue;
        }
        
        // Check if it's a square
        bool square = isSquare(rect);
        
        DetectedRectangle detected;
        // OVERFIT: Expand rectangle slightly to ensure we capture full form field
        int expandX = std::max(5, rect.width / 10);   // 10% expansion, min 5px
        int expandY = std::max(5, rect.height / 10);   // 10% expansion, min 5px
        cv::Rect overfittedRect(
            std::max(0, rect.x - expandX),
            std::max(0, rect.y - expandY),
            std::min(image.cols - std::max(0, rect.x - expandX), rect.width + expandX * 2),
            std::min(image.rows - std::max(0, rect.y - expandY), rect.height + expandY * 2)
        );
        
        detected.boundingBox = overfittedRect;  // Use overfitted rectangle
        detected.rectangularity = rectScore;
        detected.isSquare = square;
        // Boost confidence if area ratio is good (even if rectangularity is lower)
        detected.confidence = std::max(rectScore, areaRatio * 0.8);  // Use best of both
        
        // Classify type
        if (square) {
            detected.type = "square";
        } else if (rect.width < 100 && rect.height < 50) {
            detected.type = "cell";
        } else {
            detected.type = "form_field";
        }
        
        rectangles.append(detected);
    }
    
    // Filter and deduplicate
    return filterRectangles(rectangles);
}

double RectangleDetector::calculateRectangularity(const std::vector<cv::Point>& contour)
{
    if (contour.size() < 4) {
        return 0.0;
    }
    
    // Approximate contour to polygon
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, cv::arcLength(contour, true) * 0.02, true);
    
    // Check if it's a quadrilateral
    if (approx.size() != 4) {
        return 0.0;
    }
    
    // Calculate area ratio (contour area vs bounding rect area)
    double contourArea = cv::contourArea(contour);
    cv::Rect boundingRect = cv::boundingRect(contour);
    double rectArea = boundingRect.width * boundingRect.height;
    
    if (rectArea == 0) {
        return 0.0;
    }
    
    double areaRatio = contourArea / rectArea;
    
    // Calculate angle variance (rectangles have 90-degree angles)
    double angleVariance = 0.0;
    if (approx.size() == 4) {
        QList<double> angles;
        for (int i = 0; i < 4; i++) {
            cv::Point p1 = approx[i];
            cv::Point p2 = approx[(i + 1) % 4];
            cv::Point p3 = approx[(i + 2) % 4];
            
            // Calculate angle at p2
            double dx1 = p1.x - p2.x;
            double dy1 = p1.y - p2.y;
            double dx2 = p3.x - p2.x;
            double dy2 = p3.y - p2.y;
            
            double dot = dx1 * dx2 + dy1 * dy2;
            double mag1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
            double mag2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
            
            if (mag1 > 0 && mag2 > 0) {
                double cosAngle = dot / (mag1 * mag2);
                cosAngle = std::max(-1.0, std::min(1.0, cosAngle));  // Clamp
                double angle = std::acos(cosAngle) * 180.0 / CV_PI;
                angles.append(angle);
            }
        }
        
        // Calculate variance from 90 degrees
        double meanAngle = 0.0;
        for (double angle : angles) {
            meanAngle += angle;
        }
        meanAngle /= angles.size();
        
        for (double angle : angles) {
            double diff = std::abs(angle - 90.0);
            angleVariance += diff * diff;
        }
        angleVariance /= angles.size();
    }
    
    // Combined score: area ratio (weighted 70%) + angle quality (weighted 30%)
    double angleScore = 1.0 - std::min(1.0, angleVariance / 1000.0);  // Normalize
    double score = areaRatio * 0.7 + angleScore * 0.3;
    
    return score;
}

bool RectangleDetector::isSquare(const cv::Rect& rect)
{
    if (rect.width == 0 || rect.height == 0) {
        return false;
    }
    
    double aspectRatio = static_cast<double>(rect.width) / rect.height;
    // Square if aspect ratio is between 0.8 and 1.2
    return (aspectRatio >= 0.8 && aspectRatio <= 1.2);
}

QList<DetectedRectangle> RectangleDetector::filterRectangles(const QList<DetectedRectangle>& rects)
{
    QList<DetectedRectangle> filtered;
    
    // Sort by confidence (highest first)
    QList<DetectedRectangle> sorted = rects;
    std::sort(sorted.begin(), sorted.end(),
        [](const DetectedRectangle& a, const DetectedRectangle& b) {
            return a.confidence > b.confidence;
        });
    
    // Remove duplicates (overlapping rectangles)
    for (const DetectedRectangle& rect : sorted) {
        bool isDuplicate = false;
        
        for (const DetectedRectangle& existing : filtered) {
            // Calculate overlap
            int overlapX = std::max(0, std::min(rect.boundingBox.x + rect.boundingBox.width, 
                                                 existing.boundingBox.x + existing.boundingBox.width) - 
                                      std::max(rect.boundingBox.x, existing.boundingBox.x));
            int overlapY = std::max(0, std::min(rect.boundingBox.y + rect.boundingBox.height, 
                                                 existing.boundingBox.y + existing.boundingBox.height) - 
                                      std::max(rect.boundingBox.y, existing.boundingBox.y));
            int overlapArea = overlapX * overlapY;
            
            int rectArea = rect.boundingBox.width * rect.boundingBox.height;
            int existingArea = existing.boundingBox.width * existing.boundingBox.height;
            
            // If >70% overlap, it's a duplicate (more lenient - allow some overlap)
            if (overlapArea > rectArea * 0.7 || overlapArea > existingArea * 0.7) {
                isDuplicate = true;
                break;
            }
        }
        
        if (!isDuplicate) {
            filtered.append(rect);
        }
    }
    
    return filtered;
}

} // namespace ocr_orc
