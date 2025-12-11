#include "TextRegionRefiner.h"
#include "../core/CoordinateSystem.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <QtCore/QRegularExpression>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

TextRegionRefiner::TextRegionRefiner()
    : expansionRadiusPercent(50)  // Increased from 20% to 50% for better form field detection
    , lineDetectionScore(0.0)
    , rectangularityScore(0.0)
{
}

NormalizedCoords TextRegionRefiner::refineRegion(const OCRTextRegion& ocrRegion, const cv::Mat& image)
{
    if (image.empty()) {
        return ocrRegion.coords;
    }
    
    cv::Rect textBox = ocrRegion.boundingBox;
    int imgWidth = image.cols;
    int imgHeight = image.rows;
    cv::Rect formFieldBox;
    
    // Check if this text is likely a LABEL (not input text)
    if (isLikelyLabel(ocrRegion.text)) {
        // This is a label - search for form field NEAR it (not around it)
        formFieldBox = findFormFieldNearLabel(textBox, image);
        
        if (formFieldBox.width > 0 && formFieldBox.height > 0) {
            // Found form field near label - use that location
            // Calculate scores based on what we found
            QList<cv::Rect> dummyLines;
            if (formFieldBox.width > 0) {
                lineDetectionScore = 0.7;  // Found something
            }
            rectangularityScore = calculateRectangularityScore(formFieldBox, image);
        } else {
            // No form field found - might be pre-filled or label-only
            // Fall back to text location but expand it
            formFieldBox = textBox;
            int expandBy = static_cast<int>(textBox.height * 0.3);
            formFieldBox.x = std::max(0, formFieldBox.x - expandBy);
            formFieldBox.y = std::max(0, formFieldBox.y - expandBy);
            formFieldBox.width = std::min(imgWidth - formFieldBox.x, formFieldBox.width + expandBy * 2);
            formFieldBox.height = std::min(imgHeight - formFieldBox.y, formFieldBox.height + expandBy * 2);
            lineDetectionScore = 0.3;
            rectangularityScore = 0.5;
        }
    } else {
        // This might be input text (pre-filled field) - text location might be correct
        // But still try to find the containing form field
        QList<cv::Rect> rects = detectRectanglesAroundText(textBox, image);
        
        if (!rects.isEmpty()) {
            // Find rectangle that contains the text
            for (const cv::Rect& rect : rects) {
                if (rect.x <= textBox.x && rect.y <= textBox.y &&
                    rect.x + rect.width >= textBox.x + textBox.width &&
                    rect.y + rect.height >= textBox.y + textBox.height) {
                    formFieldBox = rect;
                    break;
                }
            }
        }
        
        if (formFieldBox.width == 0) {
            // No containing box found - use text location with slight expansion
            formFieldBox = textBox;
            int expandBy = static_cast<int>(textBox.height * 0.2);
            formFieldBox.x = std::max(0, formFieldBox.x - expandBy);
            formFieldBox.y = std::max(0, formFieldBox.y - expandBy);
            formFieldBox.width = std::min(imgWidth - formFieldBox.x, formFieldBox.width + expandBy * 2);
            formFieldBox.height = std::min(imgHeight - formFieldBox.y, formFieldBox.height + expandBy * 2);
        }
        
        lineDetectionScore = 0.5;
        rectangularityScore = calculateRectangularityScore(formFieldBox, image);
    }
    
    // Clamp to image bounds
    formFieldBox.x = std::max(0, std::min(formFieldBox.x, imgWidth - 1));
    formFieldBox.y = std::max(0, std::min(formFieldBox.y, imgHeight - 1));
    formFieldBox.width = std::min(formFieldBox.width, imgWidth - formFieldBox.x);
    formFieldBox.height = std::min(formFieldBox.height, imgHeight - formFieldBox.y);
    
    // Convert to normalized coordinates
    ImageCoords imgCoords(formFieldBox.x, formFieldBox.y, 
                         formFieldBox.x + formFieldBox.width, 
                         formFieldBox.y + formFieldBox.height);
    return CoordinateSystem::imageToNormalized(imgCoords, imgWidth, imgHeight);
}

cv::Rect TextRegionRefiner::expandSearchRadius(const cv::Rect& textBox, int radiusPercent, 
                                                int imgWidth, int imgHeight)
{
    int expandBy = static_cast<int>(textBox.height * radiusPercent / 100.0);
    
    cv::Rect expanded;
    expanded.x = std::max(0, textBox.x - expandBy);
    expanded.y = std::max(0, textBox.y - expandBy);
    expanded.width = std::min(imgWidth - expanded.x, textBox.width + expandBy * 2);
    expanded.height = std::min(imgHeight - expanded.y, textBox.height + expandBy * 2);
    
    return expanded;
}

QList<cv::Rect> TextRegionRefiner::detectLinesAroundText(const cv::Rect& textBox, const cv::Mat& image)
{
    QList<cv::Rect> lines;
    
    if (image.empty()) {
        return lines;
    }
    
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
    
    // Create morphological kernels for line detection
    cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(25, 1));
    cv::Mat verticalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 25));
    
    // Detect horizontal lines
    cv::Mat horizontalLines;
    cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel, cv::Point(-1, -1), 2);
    
    // Detect vertical lines
    cv::Mat verticalLines;
    cv::morphologyEx(binary, verticalLines, cv::MORPH_OPEN, verticalKernel, cv::Point(-1, -1), 2);
    
    // Expand search area
    int expandBy = static_cast<int>(textBox.height * expansionRadiusPercent / 100.0);
    cv::Rect searchArea = expandSearchRadius(textBox, expansionRadiusPercent, image.cols, image.rows);
    
    // Find contours in search area for horizontal lines
    cv::Mat roiH = horizontalLines(searchArea);
    std::vector<std::vector<cv::Point>> contoursH;
    cv::findContours(roiH, contoursH, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contoursH) {
        if (contour.size() < 2) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        // Adjust coordinates to full image
        rect.x += searchArea.x;
        rect.y += searchArea.y;
        
        // Check if line is near text (above, below, or overlapping)
        bool isNearText = (rect.y + rect.height >= textBox.y - 5 && 
                          rect.y <= textBox.y + textBox.height + 5);
        
        if (isNearText && rect.width > textBox.width * 0.5) {
            lines.append(rect);
        }
    }
    
    // Find contours in search area for vertical lines
    cv::Mat roiV = verticalLines(searchArea);
    std::vector<std::vector<cv::Point>> contoursV;
    cv::findContours(roiV, contoursV, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contoursV) {
        if (contour.size() < 2) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        // Adjust coordinates to full image
        rect.x += searchArea.x;
        rect.y += searchArea.y;
        
        // Check if line is near text (left, right, or overlapping)
        bool isNearText = (rect.x + rect.width >= textBox.x - 5 && 
                          rect.x <= textBox.x + textBox.width + 5);
        
        if (isNearText && rect.height > textBox.height * 0.5) {
            lines.append(rect);
        }
    }
    
    return lines;
}

QList<cv::Rect> TextRegionRefiner::detectRectanglesAroundText(const cv::Rect& textBox, const cv::Mat& image)
{
    QList<cv::Rect> rects;
    
    if (image.empty()) {
        return rects;
    }
    
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
    
    // Expand search area
    cv::Rect searchArea = expandSearchRadius(textBox, expansionRadiusPercent, image.cols, image.rows);
    
    // Find contours in search area
    cv::Mat roi = binary(searchArea);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(roi, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        if (contour.size() < 4) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        
        // Calculate rectangularity (how close to perfect rectangle)
        double contourArea = cv::contourArea(contour);
        double rectArea = rect.width * rect.height;
        double rectangularity = (rectArea > 0) ? contourArea / rectArea : 0.0;
        
        // Filter by rectangularity (>0.8 for good rectangles)
        if (rectangularity > 0.8) {
            // Adjust coordinates to full image
            rect.x += searchArea.x;
            rect.y += searchArea.y;
            
            // Check if rectangle overlaps or is near text
            bool overlapsText = (rect.x < textBox.x + textBox.width + 10 &&
                                 rect.x + rect.width > textBox.x - 10 &&
                                 rect.y < textBox.y + textBox.height + 10 &&
                                 rect.y + rect.height > textBox.y - 10);
            
            if (overlapsText) {
                rects.append(rect);
            }
        }
    }
    
    return rects;
}

bool TextRegionRefiner::isLikelyLabel(const QString& text)
{
    if (text.isEmpty()) {
        return false;
    }
    
    QString trimmed = text.trimmed();
    
    // Labels often end with colon
    if (trimmed.endsWith(':')) {
        return true;
    }
    
    // Labels are typically short (1-3 words)
    QStringList words = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (words.size() <= 3 && trimmed.length() < 30) {
        // Could be a label - check for common label patterns
        QString lower = trimmed.toLower();
        if (lower.contains("name") || lower.contains("address") || 
            lower.contains("phone") || lower.contains("email") ||
            lower.contains("date") || lower.contains("time") ||
            lower.contains("city") || lower.contains("state") ||
            lower.contains("zip") || lower.contains("postal") ||
            lower.contains("country") || lower.contains("age")) {
            return true;
        }
    }
    
    return false;
}

cv::Rect TextRegionRefiner::findFormFieldNearLabel(const cv::Rect& labelBox, const cv::Mat& image)
{
    if (image.empty()) {
        return cv::Rect();
    }
    
    int imgWidth = image.cols;
    int imgHeight = image.rows;
    
    // Strategy: Search in common directions (below, right, above)
    // Priority order: below > right > above
    
    // 1. Search BELOW label (most common - vertical forms)
    cv::Rect searchBelow(
        std::max(0, labelBox.x - 10),  // Start slightly left of label
        labelBox.y + labelBox.height + 5,  // Start 5px below label
        std::min(imgWidth - std::max(0, labelBox.x - 10), labelBox.width + 100),  // Wider than label
        std::min(imgHeight - (labelBox.y + labelBox.height + 5), labelBox.height * 3)  // Search 3x label height down
    );
    
    if (searchBelow.width > 0 && searchBelow.height > 0) {
        cv::Rect fieldBelow = detectFormFieldInArea(searchBelow, image);
        if (fieldBelow.width > 0) {
            return fieldBelow;
        }
    }
    
    // 2. Search to the RIGHT of label (horizontal forms)
    cv::Rect fieldRight = detectFormFieldToRight(labelBox, image);
    if (fieldRight.width > 0) {
        return fieldRight;
    }
    
    // 3. Search ABOVE label (less common, but sometimes used)
    cv::Rect searchAbove(
        std::max(0, labelBox.x - 10),
        std::max(0, labelBox.y - labelBox.height * 2),  // Search 2x label height up
        std::min(imgWidth - std::max(0, labelBox.x - 10), labelBox.width + 100),
        std::min(labelBox.y - std::max(0, labelBox.y - labelBox.height * 2), labelBox.height * 2)
    );
    
    if (searchAbove.width > 0 && searchAbove.height > 0) {
        cv::Rect fieldAbove = detectFormFieldInArea(searchAbove, image);
        if (fieldAbove.width > 0) {
            return fieldAbove;
        }
    }
    
    return cv::Rect();  // No form field found
}

cv::Rect TextRegionRefiner::detectFormFieldToRight(const cv::Rect& labelBox, const cv::Mat& image)
{
    if (image.empty()) {
        return cv::Rect();
    }
    
    int imgWidth = image.cols;
    int imgHeight = image.rows;
    
    // Search area to the right of label
    int searchStartX = labelBox.x + labelBox.width + 10;  // 10px gap
    int searchWidth = std::min(imgWidth - searchStartX, 200);  // Search up to 200px right
    int searchY = std::max(0, labelBox.y - 5);  // Align with label (slight vertical tolerance)
    int searchHeight = std::min(imgHeight - searchY, labelBox.height + 10);
    
    if (searchWidth <= 0 || searchHeight <= 0) {
        return cv::Rect();
    }
    
    cv::Rect searchArea(searchStartX, searchY, searchWidth, searchHeight);
    return detectFormFieldInArea(searchArea, image);
}

cv::Rect TextRegionRefiner::detectFormFieldInArea(const cv::Rect& searchArea, const cv::Mat& image)
{
    if (image.empty() || searchArea.width <= 0 || searchArea.height <= 0) {
        return cv::Rect();
    }
    
    // Clamp search area to image bounds
    cv::Rect clampedArea(
        std::max(0, searchArea.x),
        std::max(0, searchArea.y),
        std::min(image.cols - std::max(0, searchArea.x), searchArea.width),
        std::min(image.rows - std::max(0, searchArea.y), searchArea.height)
    );
    
    if (clampedArea.width <= 0 || clampedArea.height <= 0) {
        return cv::Rect();
    }
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Extract ROI
    cv::Mat roi = gray(clampedArea);
    
    // KEY INSIGHT: Look for EMPTY areas (high brightness/whiteness) with lines/boxes
    // Form fields are typically WHITE/EMPTY with dark borders or underlines
    
    // Apply adaptive threshold to separate text from background
    cv::Mat binary;
    cv::adaptiveThreshold(roi, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY, 11, 2);
    
    // Invert: white areas (form fields) become black, text/lines become white
    cv::bitwise_not(binary, binary);
    
    // Look for horizontal lines (underlines) - these indicate form fields
    cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(40, 1));
    cv::Mat horizontalLines;
    cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel);
    
    // Find contours (underlines)
    std::vector<std::vector<cv::Point>> lineContours;
    cv::findContours(horizontalLines, lineContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    cv::Rect bestField;
    double bestScore = 0.0;
    
    for (const auto& contour : lineContours) {
        if (contour.size() < 2) continue;
        
        cv::Rect lineRect = cv::boundingRect(contour);
        // Adjust coordinates to full image
        lineRect.x += clampedArea.x;
        lineRect.y += clampedArea.y;
        
        // Check if this is a good underline (wide enough, reasonable position)
        if (lineRect.width < 30) continue;  // Too narrow
        
        // Create form field box: above the underline, reasonable height
        int fieldHeight = 20;  // Typical form field height
        cv::Rect fieldRect(
            lineRect.x,
            lineRect.y - fieldHeight,  // Field is above the underline
            lineRect.width,
            fieldHeight
        );
        
        // Clamp to image bounds
        if (fieldRect.y < 0) {
            fieldRect.height += fieldRect.y;
            fieldRect.y = 0;
        }
        
        // Check if this area is mostly EMPTY (white/high brightness)
        cv::Rect checkArea = fieldRect;
        checkArea.x = std::max(0, checkArea.x);
        checkArea.y = std::max(0, checkArea.y);
        checkArea.width = std::min(image.cols - checkArea.x, checkArea.width);
        checkArea.height = std::min(image.rows - checkArea.y, checkArea.height);
        
        if (checkArea.width > 0 && checkArea.height > 0) {
            cv::Mat fieldROI = gray(checkArea);
            
            // Calculate whiteness/emptiness score
            // High brightness = empty form field
            cv::Scalar meanBrightness = cv::mean(fieldROI);
            double brightness = meanBrightness[0] / 255.0;  // Normalize to 0-1
            
            // Prefer wider fields with high brightness (empty)
            double score = lineRect.width * brightness;
            
            if (score > bestScore && brightness > 0.7) {  // At least 70% white/empty
                bestScore = score;
                bestField = fieldRect;
            }
        }
    }
    
    // Also look for empty boxes (rectangles with borders but empty inside)
    cv::Mat edges;
    cv::Canny(roi, edges, 50, 150);
    
    // Find rectangular contours (box borders)
    std::vector<std::vector<cv::Point>> boxContours;
    cv::findContours(edges, boxContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : boxContours) {
        if (contour.size() < 4) continue;
        
        cv::Rect boxRect = cv::boundingRect(contour);
        // Adjust coordinates to full image
        boxRect.x += clampedArea.x;
        boxRect.y += clampedArea.y;
        
        // Check if it's a reasonable form field size
        if (boxRect.width < 30 || boxRect.height < 10 || boxRect.height > 50) continue;
        
        // Calculate rectangularity
        double contourArea = cv::contourArea(contour);
        double rectArea = boxRect.width * boxRect.height;
        double rectangularity = (rectArea > 0) ? contourArea / rectArea : 0.0;
        
        if (rectangularity < 0.6) continue;  // Not rectangular enough
        
        // Check if box interior is EMPTY (white/high brightness)
        cv::Rect interiorRect(
            boxRect.x + 2,  // Slight margin from border
            boxRect.y + 2,
            std::max(1, boxRect.width - 4),
            std::max(1, boxRect.height - 4)
        );
        
        // Clamp to image bounds
        interiorRect.x = std::max(0, interiorRect.x);
        interiorRect.y = std::max(0, interiorRect.y);
        interiorRect.width = std::min(image.cols - interiorRect.x, interiorRect.width);
        interiorRect.height = std::min(image.rows - interiorRect.y, interiorRect.height);
        
        if (interiorRect.width > 0 && interiorRect.height > 0) {
            cv::Mat interiorROI = gray(interiorRect);
            cv::Scalar meanBrightness = cv::mean(interiorROI);
            double brightness = meanBrightness[0] / 255.0;
            
            // Prefer empty boxes (high brightness)
            double score = boxRect.width * boxRect.height * brightness;
            
            if (score > bestScore && brightness > 0.75) {  // At least 75% white/empty
                bestScore = score;
                bestField = boxRect;
            }
        }
    }
    
    return bestField;
}

bool TextRegionRefiner::regionContainsText(const cv::Rect& region, const cv::Mat& image, 
                                          const QList<OCRTextRegion>& ocrRegions)
{
    // Check if region overlaps with any OCR text regions
    for (const OCRTextRegion& ocrRegion : ocrRegions) {
        cv::Rect ocrBox = ocrRegion.boundingBox;
        
        // Calculate overlap
        int overlapX = std::max(0, std::min(region.x + region.width, ocrBox.x + ocrBox.width) - 
                                  std::max(region.x, ocrBox.x));
        int overlapY = std::max(0, std::min(region.y + region.height, ocrBox.y + ocrBox.height) - 
                                  std::max(region.y, ocrBox.y));
        int overlapArea = overlapX * overlapY;
        
        // Calculate region and OCR areas
        int regionArea = region.width * region.height;
        int ocrArea = ocrBox.width * ocrBox.height;
        
        // If significant overlap (>20% of region or OCR area), region contains text
        if (overlapArea > regionArea * 0.2 || overlapArea > ocrArea * 0.2) {
            return true;
        }
    }
    
    // Also check image content: if region has low brightness (dark pixels = text), it contains text
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return false;
    }
    
    // Clamp region to image bounds
    cv::Rect clampedRegion(
        std::max(0, region.x),
        std::max(0, region.y),
        std::min(image.cols - std::max(0, region.x), region.width),
        std::min(image.rows - std::max(0, region.y), region.height)
    );
    
    if (clampedRegion.width <= 0 || clampedRegion.height <= 0) {
        return false;
    }
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Extract ROI
    cv::Mat roi = gray(clampedRegion);
    
    // Calculate mean brightness
    cv::Scalar meanBrightness = cv::mean(roi);
    double brightness = meanBrightness[0] / 255.0;
    
    // If brightness is too low (<0.6), likely contains text (dark pixels)
    if (brightness < 0.6) {
        return true;
    }
    
    // Check for text-like patterns (high variance = edges/text)
    cv::Mat edges;
    cv::Canny(roi, edges, 50, 150);
    int edgePixels = cv::countNonZero(edges);
    double edgeDensity = static_cast<double>(edgePixels) / (roi.rows * roi.cols);
    
    // High edge density (>0.1) suggests text content
    if (edgeDensity > 0.1) {
        return true;
    }
    
    return false;  // Region appears empty
}

QList<cv::Rect> TextRegionRefiner::findEmptyFormFields(const QList<OCRTextRegion>& ocrHints, 
                                                       const cv::Mat& image)
{
    QList<cv::Rect> emptyFields;
    
    if (image.empty()) {
        return emptyFields;
    }
    
    // Pass 1: For each OCR hint, search for empty form fields nearby
    for (const OCRTextRegion& hint : ocrHints) {
        cv::Rect hintBox = hint.boundingBox;
        
        // Skip very small hints (likely noise)
        if (hintBox.width < 10 || hintBox.height < 5) {
            continue;
        }
        
        // Search in multiple directions: below, right, above
        // Priority: below > right > above
        
        // 1. Search BELOW hint (most common - vertical forms)
        cv::Rect searchBelow(
            std::max(0, hintBox.x - 20),
            hintBox.y + hintBox.height + 5,  // Start 5px below
            std::min(image.cols - std::max(0, hintBox.x - 20), hintBox.width + 100),
            std::min(image.rows - (hintBox.y + hintBox.height + 5), hintBox.height * 4)  // Search 4x height down
        );
        
        if (searchBelow.width > 0 && searchBelow.height > 0) {
            cv::Rect field = detectFormFieldInArea(searchBelow, image);
            if (field.width > 0 && field.height > 0) {
                emptyFields.append(field);
            }
        }
        
        // 2. Search to the RIGHT of hint (horizontal forms)
        cv::Rect searchRight(
            hintBox.x + hintBox.width + 10,  // 10px gap
            std::max(0, hintBox.y - 5),
            std::min(image.cols - (hintBox.x + hintBox.width + 10), 200),  // Up to 200px right
            std::min(image.rows - std::max(0, hintBox.y - 5), hintBox.height + 10)
        );
        
        if (searchRight.width > 0 && searchRight.height > 0) {
            cv::Rect field = detectFormFieldInArea(searchRight, image);
            if (field.width > 0 && field.height > 0) {
                emptyFields.append(field);
            }
        }
        
        // 3. Search ABOVE hint (less common)
        cv::Rect searchAbove(
            std::max(0, hintBox.x - 20),
            std::max(0, hintBox.y - hintBox.height * 2),  // Search 2x height up
            std::min(image.cols - std::max(0, hintBox.x - 20), hintBox.width + 100),
            std::min(hintBox.y - std::max(0, hintBox.y - hintBox.height * 2), hintBox.height * 2)
        );
        
        if (searchAbove.width > 0 && searchAbove.height > 0) {
            cv::Rect field = detectFormFieldInArea(searchAbove, image);
            if (field.width > 0 && field.height > 0) {
                emptyFields.append(field);
            }
        }
    }
    
    // Pass 2: Remove duplicates (fields that overlap significantly)
    QList<cv::Rect> deduplicatedFields;
    for (const cv::Rect& field : emptyFields) {
        bool isDuplicate = false;
        
        for (const cv::Rect& existing : deduplicatedFields) {
            // Calculate overlap
            int overlapX = std::max(0, std::min(field.x + field.width, existing.x + existing.width) - 
                                      std::max(field.x, existing.x));
            int overlapY = std::max(0, std::min(field.y + field.height, existing.y + existing.height) - 
                                      std::max(field.y, existing.y));
            int overlapArea = overlapX * overlapY;
            
            int fieldArea = field.width * field.height;
            int existingArea = existing.width * existing.height;
            
            // If >50% overlap, it's a duplicate
            if (overlapArea > fieldArea * 0.5 || overlapArea > existingArea * 0.5) {
                isDuplicate = true;
                break;
            }
        }
        
        if (!isDuplicate) {
            deduplicatedFields.append(field);
        }
    }
    
    return deduplicatedFields;
}

cv::Rect TextRegionRefiner::detectUnderlineBelow(const cv::Rect& textBox, const cv::Mat& image)
{
    if (image.empty()) {
        return cv::Rect();
    }
    
    // Look for horizontal line below text (within 10px)
    int searchStartY = textBox.y + textBox.height;
    int searchEndY = std::min(image.rows, searchStartY + 15);  // Search 15px below
    int searchHeight = searchEndY - searchStartY;
    
    if (searchHeight <= 0) {
        return cv::Rect();
    }
    
    // Expand search horizontally to find full underline width
    int searchX = std::max(0, textBox.x - 20);
    int searchWidth = std::min(image.cols - searchX, textBox.width + 40);
    
    if (searchWidth <= 0) {
        return cv::Rect();
    }
    
    cv::Rect searchArea(searchX, searchStartY, searchWidth, searchHeight);
    
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
    
    // Extract ROI
    cv::Mat roi = binary(searchArea);
    
    // Use horizontal morphology to detect underline
    cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 1));
    cv::Mat horizontalLines;
    cv::morphologyEx(roi, horizontalLines, cv::MORPH_OPEN, horizontalKernel);
    
    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(horizontalLines, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Find the longest horizontal line (likely the underline)
    cv::Rect bestLine;
    int maxWidth = 0;
    
    for (const auto& contour : contours) {
        if (contour.size() < 2) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        // Adjust coordinates to full image
        rect.x += searchArea.x;
        rect.y += searchArea.y;
        
        // Check if line is below text and wide enough
        if (rect.width > textBox.width * 0.5 && rect.width > maxWidth) {
            maxWidth = rect.width;
            bestLine = rect;
        }
    }
    
    return bestLine;
}

cv::Rect TextRegionRefiner::mergeDetections(const cv::Rect& textBox, 
                                            const QList<cv::Rect>& lines, 
                                            const QList<cv::Rect>& rects,
                                            const cv::Rect& underlineRect,
                                            bool hasUnderline)
{
    cv::Rect merged = textBox;
    
    // If we found an underline, expand downward significantly
    if (hasUnderline && underlineRect.width > 0) {
        // Extend box to include underline, and expand horizontally to full underline width
        int x1 = std::min(merged.x, underlineRect.x);
        int y1 = merged.y;
        int x2 = std::max(merged.x + merged.width, underlineRect.x + underlineRect.width);
        int y2 = std::max(merged.y + merged.height, underlineRect.y + underlineRect.height);
        
        // Add some padding below underline for form field
        y2 += 5;  // 5px padding below underline
        
        merged = cv::Rect(x1, y1, x2 - x1, y2 - y1);
    }
    
    // Merge with other lines
    for (const cv::Rect& line : lines) {
        // Extend merged box to include line
        int x1 = std::min(merged.x, line.x);
        int y1 = std::min(merged.y, line.y);
        int x2 = std::max(merged.x + merged.width, line.x + line.width);
        int y2 = std::max(merged.y + merged.height, line.y + line.height);
        
        merged = cv::Rect(x1, y1, x2 - x1, y2 - y1);
    }
    
    // Merge with rectangles (but prefer rectangles that contain text)
    for (const cv::Rect& rect : rects) {
        // If rectangle contains text, use it as base
        if (rect.x <= textBox.x && rect.y <= textBox.y &&
            rect.x + rect.width >= textBox.x + textBox.width &&
            rect.y + rect.height >= textBox.y + textBox.height) {
            // Rectangle contains text, use it
            merged = rect;
        } else {
            // Union with merged box
            int x1 = std::min(merged.x, rect.x);
            int y1 = std::min(merged.y, rect.y);
            int x2 = std::max(merged.x + merged.width, rect.x + rect.width);
            int y2 = std::max(merged.y + merged.height, rect.y + rect.height);
            
            merged = cv::Rect(x1, y1, x2 - x1, y2 - y1);
        }
    }
    
    return merged;
}

double TextRegionRefiner::calculateLineDetectionScore(const QList<cv::Rect>& lines, const cv::Mat& image)
{
    if (lines.isEmpty()) {
        return 0.0;
    }
    
    // Count lines (0-4: above, below, left, right)
    int lineCount = (lines.size() < 4) ? lines.size() : 4;
    
    // Calculate average line strength (edge pixel density)
    double totalStrength = 0.0;
    for (const cv::Rect& line : lines) {
        if (line.width > 0 && line.height > 0) {
            cv::Mat lineROI = image(line);
            cv::Mat edges;
            cv::Canny(lineROI, edges, 50, 150);
            int edgePixels = cv::countNonZero(edges);
            double strength = static_cast<double>(edgePixels) / (line.width * line.height);
            totalStrength += strength;
        }
    }
    
    double avgStrength = lines.size() > 0 ? totalStrength / lines.size() : 0.0;
    
    // Score: (lineCount / 4.0) * averageLineStrength
    return (static_cast<double>(lineCount) / 4.0) * avgStrength;
}

double TextRegionRefiner::calculateRectangularityScore(const cv::Rect& rect, const cv::Mat& image)
{
    if (rect.width <= 0 || rect.height <= 0) {
        return 0.0;
    }
    
    // Calculate aspect ratio score (prefer 0.5-2.0 range)
    double aspectRatio = static_cast<double>(rect.width) / rect.height;
    double aspectScore = 1.0;
    if (aspectRatio < 0.5 || aspectRatio > 2.0) {
        aspectScore = 0.5;  // Penalize extreme aspect ratios
    }
    
    // Calculate rectangularity from contour (if we can extract it)
    // For now, assume good rectangularity if aspect ratio is reasonable
    double rectangularity = 0.8;  // Default assumption
    
    return rectangularity * aspectScore;
}

} // namespace ocr_orc
