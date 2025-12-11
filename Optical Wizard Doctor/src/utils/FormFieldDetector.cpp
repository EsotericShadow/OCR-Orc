#include "FormFieldDetector.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

namespace ocr_orc {

FormFieldDetector::FormFieldDetector()
{
}

FormFieldType FormFieldDetector::detectFormField(const OCRTextRegion& textRegion, 
                                                  const cv::Mat& image,
                                                  const CheckboxDetection& checkbox)
{
    // If checkbox detected, it's a checkbox field
    if (checkbox.detected) {
        return FormFieldType::CheckboxField;
    }
    
    // Check for text input field (underline or box)
    if (detectTextInputField(textRegion, image)) {
        return FormFieldType::TextInput;
    }
    
    // Default to label (just text, no input field)
    return FormFieldType::Label;
}

bool FormFieldDetector::detectTextInputField(const OCRTextRegion& textRegion, const cv::Mat& image)
{
    cv::Rect textBox = textRegion.boundingBox;
    
    // Check for underline
    if (detectUnderline(textBox, image)) {
        return true;
    }
    
    // Check for bounding rectangle
    if (detectBoundingRectangle(textBox, image)) {
        return true;
    }
    
    return false;
}

bool FormFieldDetector::detectUnderline(const cv::Rect& textBox, const cv::Mat& image)
{
    if (image.empty()) {
        return false;
    }
    
    // Check for horizontal line below text (within 5px)
    int lineY = textBox.y + textBox.height + 5;
    int lineHeight = 3;  // 3px line height
    
    if (lineY + lineHeight > image.rows) {
        return false;
    }
    
    // Extract ROI below text
    cv::Rect roiRect(
        std::max(0, textBox.x - 5),
        lineY,
        std::min(image.cols - std::max(0, textBox.x - 5), textBox.width + 10),
        lineHeight
    );
    
    if (roiRect.width <= 0 || roiRect.height <= 0) {
        return false;
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
    
    // Extract ROI
    cv::Mat roi = binary(roiRect);
    
    // Check if there's a significant horizontal line
    // Count non-zero pixels (should be high for a line)
    int nonZeroPixels = cv::countNonZero(roi);
    double lineDensity = static_cast<double>(nonZeroPixels) / (roi.cols * roi.rows);
    
    // If >50% of pixels are non-zero, likely a line
    return lineDensity > 0.5;
}

bool FormFieldDetector::detectBoundingRectangle(const cv::Rect& textBox, const cv::Mat& image)
{
    if (image.empty()) {
        return false;
    }
    
    // Expand search area around text
    int expandBy = static_cast<int>(textBox.height * 0.2);  // 20% expansion
    
    cv::Rect searchArea(
        std::max(0, textBox.x - expandBy),
        std::max(0, textBox.y - expandBy),
        std::min(image.cols - std::max(0, textBox.x - expandBy), textBox.width + expandBy * 2),
        std::min(image.rows - std::max(0, textBox.y - expandBy), textBox.height + expandBy * 2)
    );
    
    if (searchArea.width <= 0 || searchArea.height <= 0) {
        return false;
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
    
    // Find contours in search area
    cv::Mat roi = binary(searchArea);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(roi, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Look for rectangle that contains or overlaps text box
    for (const auto& contour : contours) {
        if (contour.size() < 4) continue;
        
        cv::Rect rect = cv::boundingRect(contour);
        
        // Adjust coordinates to full image
        rect.x += searchArea.x;
        rect.y += searchArea.y;
        
        // Check rectangularity
        double contourArea = cv::contourArea(contour);
        double rectArea = rect.width * rect.height;
        double rectangularity = (rectArea > 0) ? contourArea / rectArea : 0.0;
        
        if (rectangularity > 0.8) {
            // Check if rectangle contains or overlaps text box
            bool containsText = (rect.x <= textBox.x &&
                                 rect.y <= textBox.y &&
                                 rect.x + rect.width >= textBox.x + textBox.width &&
                                 rect.y + rect.height >= textBox.y + textBox.height);
            
            bool overlapsText = (rect.x < textBox.x + textBox.width &&
                                 rect.x + rect.width > textBox.x &&
                                 rect.y < textBox.y + textBox.height &&
                                 rect.y + rect.height > textBox.y);
            
            if (containsText || overlapsText) {
                return true;
            }
        }
    }
    
    return false;
}

QList<OCRTextRegion> FormFieldDetector::detectMultiLineTextArea(const QList<OCRTextRegion>& regions, 
                                                                int pageWidth)
{
    QList<OCRTextRegion> multiLineAreas;
    
    if (regions.size() < 3) {
        return multiLineAreas;
    }
    
    // Group by vertical alignment (X-coordinate clustering, 2% tolerance)
    double tolerance = pageWidth * 0.02;
    
    // Sort by X-coordinate
    QList<OCRTextRegion> sorted = regions;
    std::sort(sorted.begin(), sorted.end(),
        [](const OCRTextRegion& a, const OCRTextRegion& b) {
            return a.boundingBox.x < b.boundingBox.x;
        });
    
    // Cluster by X-coordinate
    QList<QList<OCRTextRegion>> verticalGroups;
    for (const OCRTextRegion& region : sorted) {
        int regionX = region.boundingBox.x + region.boundingBox.width / 2;
        bool added = false;
        
        for (auto& group : verticalGroups) {
            if (group.isEmpty()) continue;
            
            int groupX = group[0].boundingBox.x + group[0].boundingBox.width / 2;
            if (std::abs(regionX - groupX) < tolerance) {
                group.append(region);
                added = true;
                break;
            }
        }
        
        if (!added) {
            verticalGroups.append({region});
        }
    }
    
    // Find groups with 3+ regions (multi-line areas)
    for (const QList<OCRTextRegion>& group : verticalGroups) {
        if (group.size() >= 3) {
            // Create a merged region for the multi-line area
            // For now, just add all regions (can be enhanced to merge into single region)
            for (const OCRTextRegion& region : group) {
                multiLineAreas.append(region);
            }
        }
    }
    
    return multiLineAreas;
}

QList<OCRTextRegion> FormFieldDetector::detectGridCells(const QList<OCRTextRegion>& regions)
{
    // This would use existing GridDetector with OCR regions as seeds
    // For now, return empty list (can be enhanced later)
    Q_UNUSED(regions);
    return QList<OCRTextRegion>();
}

cv::Rect FormFieldDetector::overfitRegion(const cv::Rect& region, const cv::Mat& image, int expandPercent)
{
    if (image.empty()) {
        return region;
    }
    
    int imgWidth = image.cols;
    int imgHeight = image.rows;
    
    // Calculate expansion amount - MORE DRASTIC
    // Expand both by percentage AND add minimum pixel amounts
    int expandX = static_cast<int>(region.width * expandPercent / 100.0);
    int expandY = static_cast<int>(region.height * expandPercent / 100.0);
    
    // Add minimum expansion (at least 20px each direction for small regions)
    expandX = std::max(expandX, 20);
    expandY = std::max(expandY, 15);
    
    // Expand region DRASTICALLY
    cv::Rect overfitted;
    overfitted.x = std::max(0, region.x - expandX);
    overfitted.y = std::max(0, region.y - expandY);
    overfitted.width = std::min(imgWidth - overfitted.x, region.width + expandX * 2);
    overfitted.height = std::min(imgHeight - overfitted.y, region.height + expandY * 2);
    
    return overfitted;
}

cv::Rect FormFieldDetector::overfitRegionAsymmetric(const cv::Rect& region, const cv::Mat& image, 
                                                     int expandPercentX, int expandPercentY)
{
    if (image.empty()) {
        return region;
    }
    
    int imgWidth = image.cols;
    int imgHeight = image.rows;
    
    // Calculate asymmetric expansion - EXTREME vertical expansion
    int expandX = static_cast<int>(region.width * expandPercentX / 100.0);
    int expandY = static_cast<int>(region.height * expandPercentY / 100.0);
    
    // Add minimum expansion with emphasis on vertical
    // Horizontal: at least 25px
    // Vertical: at least 30px (more aggressive for multi-line fields)
    expandX = std::max(expandX, 25);
    expandY = std::max(expandY, 30);
    
    // For very small regions, add even more vertical room
    if (region.height < 20) {
        expandY = std::max(expandY, 40);  // At least 40px vertical for small regions
    }
    
    // Expand region EXTREMELY DRASTICALLY with asymmetric expansion
    cv::Rect overfitted;
    overfitted.x = std::max(0, region.x - expandX);
    overfitted.y = std::max(0, region.y - expandY);
    overfitted.width = std::min(imgWidth - overfitted.x, region.width + expandX * 2);
    overfitted.height = std::min(imgHeight - overfitted.y, region.height + expandY * 2);
    
    return overfitted;
}

QList<cv::Rect> FormFieldDetector::refineOverfittedRegions(const QList<cv::Rect>& overfittedRegions, 
                                                           const cv::Mat& image)
{
    QList<cv::Rect> refined;
    
    if (image.empty()) {
        return refined;
    }
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    for (const cv::Rect& overfitted : overfittedRegions) {
        // Clamp to image bounds
        cv::Rect searchArea(
            std::max(0, overfitted.x),
            std::max(0, overfitted.y),
            std::min(image.cols - std::max(0, overfitted.x), overfitted.width),
            std::min(image.rows - std::max(0, overfitted.y), overfitted.height)
        );
        
        if (searchArea.width <= 0 || searchArea.height <= 0) {
            continue;
        }
        
        // Extract ROI
        cv::Mat roi = gray(searchArea);
        
        // Use Canny edge detection to find form field boundaries
        cv::Mat edges;
        cv::Canny(roi, edges, 50, 150);
        
        // Use morphological operations to connect edges and find complete boundaries
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::Mat dilated;
        cv::dilate(edges, dilated, kernel, cv::Point(-1, -1), 2);
        
        // Find contours (form field boundaries)
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        cv::Rect bestField;
        double bestScore = 0.0;
        
        for (const auto& contour : contours) {
            if (contour.size() < 4) continue;
            
            cv::Rect rect = cv::boundingRect(contour);
            // Adjust to full image coordinates
            rect.x += searchArea.x;
            rect.y += searchArea.y;
            
            // Calculate score based on:
            // 1. Rectangularity (how close to perfect rectangle)
            double contourArea = cv::contourArea(contour);
            double rectArea = rect.width * rect.height;
            double rectangularity = (rectArea > 0) ? contourArea / rectArea : 0.0;
            
            // 2. Size appropriateness (form fields are typically 30-300px wide, 10-50px tall)
            bool goodSize = (rect.width >= 30 && rect.width <= 400 && 
                            rect.height >= 10 && rect.height <= 60);
            
            // 3. Emptiness (check interior brightness)
            cv::Rect interiorRect(
                std::max(0, rect.x + 2),
                std::max(0, rect.y + 2),
                std::min(image.cols - std::max(0, rect.x + 2), std::max(1, rect.width - 4)),
                std::min(image.rows - std::max(0, rect.y + 2), std::max(1, rect.height - 4))
            );
            
            double brightness = 0.8;  // Default
            if (interiorRect.width > 0 && interiorRect.height > 0) {
                cv::Mat interiorROI = gray(interiorRect);
                cv::Scalar meanBrightness = cv::mean(interiorROI);
                brightness = meanBrightness[0] / 255.0;
            }
            
            // Combined score
            double score = rectangularity * (brightness > 0.7 ? 1.0 : 0.5) * (goodSize ? 1.0 : 0.3);
            
            if (score > bestScore && rectangularity > 0.6 && brightness > 0.65) {
                bestScore = score;
                bestField = rect;
            }
        }
        
        // If we found a good field, enhance it with hard edges (above, left, right) and more vertical height
        if (bestField.width > 0 && bestField.height > 0) {
            // Find hard edges: ABOVE, LEFT, and RIGHT
            int edgeY = findHardEdgeAbove(bestField, image, 80);  // Search 80px above
            int edgeXLeft = findHardEdgeLeft(bestField, image, 80);  // Search 80px left
            int edgeXRight = findHardEdgeRight(bestField, image, 80);  // Search 80px right
            
            cv::Rect enhancedField = bestField;
            
            // Lock to hard edge ABOVE
            if (edgeY >= 0) {
                int currentTop = bestField.y;
                int newTop = edgeY;
                int heightIncrease = currentTop - newTop;
                
                enhancedField.y = newTop;
                enhancedField.height = bestField.height + heightIncrease;
            } else {
                // No edge found above, but still increase vertical height
                int heightIncrease = static_cast<int>(bestField.height * 0.5);
                enhancedField.y = std::max(0, bestField.y - heightIncrease);
                enhancedField.height = bestField.height + heightIncrease;
            }
            
            // Lock to hard edge LEFT
            if (edgeXLeft >= 0) {
                int currentLeft = bestField.x;
                int newLeft = edgeXLeft;
                int widthIncrease = currentLeft - newLeft;
                
                enhancedField.x = newLeft;
                enhancedField.width = bestField.width + widthIncrease;
            } else {
                // Extend left by 20% if no edge found
                int widthIncrease = static_cast<int>(bestField.width * 0.2);
                enhancedField.x = std::max(0, bestField.x - widthIncrease);
                enhancedField.width = bestField.width + widthIncrease;
            }
            
            // Lock to hard edge RIGHT
            if (edgeXRight >= 0) {
                int currentRight = bestField.x + bestField.width;
                int newRight = edgeXRight;
                int widthIncrease = newRight - currentRight;
                
                enhancedField.width = bestField.width + widthIncrease;
            } else {
                // Extend right by 20% if no edge found
                int widthIncrease = static_cast<int>(bestField.width * 0.2);
                enhancedField.width = bestField.width + widthIncrease;
            }
            
            // Also extend downward for more vertical room (especially for multi-line fields)
            int downwardExtension = static_cast<int>(bestField.height * 0.8);  // 80% more height downward
            enhancedField.height += downwardExtension;
            
            // Clamp to image bounds
            enhancedField.x = std::max(0, enhancedField.x);
            enhancedField.y = std::max(0, enhancedField.y);
            enhancedField.width = std::min(image.cols - enhancedField.x, enhancedField.width);
            enhancedField.height = std::min(image.rows - enhancedField.y, enhancedField.height);
            
            refined.append(enhancedField);
        } else {
            // Fallback: try to find horizontal lines (underlines) for text lines
            cv::Mat binary;
            cv::threshold(roi, binary, 127, 255, cv::THRESH_BINARY_INV);
            
            cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(40, 1));
            cv::Mat horizontalLines;
            cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel);
            
            std::vector<std::vector<cv::Point>> lineContours;
            cv::findContours(horizontalLines, lineContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            
            if (!lineContours.empty()) {
                // Find the longest/best horizontal line
                cv::Rect bestLine;
                int maxWidth = 0;
                
                for (const auto& contour : lineContours) {
                    cv::Rect lineRect = cv::boundingRect(contour);
                    lineRect.x += searchArea.x;
                    lineRect.y += searchArea.y;
                    
                    if (lineRect.width > maxWidth && lineRect.width > 30) {
                        maxWidth = lineRect.width;
                        // Create field above the line with MORE vertical height
                        int fieldHeight = 35;  // Increased from 20 to 35px
                        int fieldTop = std::max(0, lineRect.y - fieldHeight);
                        
                        // Find hard edges: above, left, right to anchor to
                        cv::Rect tempField(lineRect.x, fieldTop, lineRect.width, fieldHeight);
                        int edgeY = findHardEdgeAbove(tempField, image, 60);
                        int edgeXLeft = findHardEdgeLeft(tempField, image, 60);
                        int edgeXRight = findHardEdgeRight(tempField, image, 60);
                        
                        if (edgeY >= 0) {
                            fieldTop = edgeY;
                            fieldHeight = lineRect.y - edgeY;
                        }
                        
                        int fieldLeft = lineRect.x;
                        int fieldWidth = lineRect.width;
                        
                        if (edgeXLeft >= 0) {
                            fieldLeft = edgeXLeft;
                            fieldWidth = (lineRect.x + lineRect.width) - edgeXLeft;
                        }
                        
                        if (edgeXRight >= 0) {
                            fieldWidth = edgeXRight - fieldLeft;
                        }
                        
                        bestLine = cv::Rect(
                            fieldLeft,
                            fieldTop,
                            fieldWidth,
                            fieldHeight
                        );
                    }
                }
                
                if (bestLine.width > 0) {
                    // Extend downward for more vertical room
                    int downwardExtension = static_cast<int>(bestLine.height * 0.6);
                    bestLine.height += downwardExtension;
                    bestLine.height = std::min(image.rows - bestLine.y, bestLine.height);
                    
                    refined.append(bestLine);
                } else {
                    // Fallback: use overfitted but increase vertical height
                    cv::Rect enhanced = overfitted;
                    int heightIncrease = static_cast<int>(overfitted.height * 0.5);
                    enhanced.y = std::max(0, enhanced.y - heightIncrease / 2);
                    enhanced.height += heightIncrease;
                    enhanced.height = std::min(image.rows - enhanced.y, enhanced.height);
                    refined.append(enhanced);
                }
            } else {
                // Fallback: use overfitted but increase vertical height
                cv::Rect enhanced = overfitted;
                int heightIncrease = static_cast<int>(overfitted.height * 0.5);
                enhanced.y = std::max(0, enhanced.y - heightIncrease / 2);
                enhanced.height += heightIncrease;
                enhanced.height = std::min(image.rows - enhanced.y, enhanced.height);
                refined.append(enhanced);
            }
        }
    }
    
    return refined;
}

QList<cv::Rect> FormFieldDetector::classifyAndRefineRegions(const QList<cv::Rect>& regions,
                                                           const cv::Mat& image,
                                                           const QList<OCRTextRegion>& ocrRegions)
{
    QList<cv::Rect> validFields;
    
    for (const cv::Rect& region : regions) {
        // First check: Filter out titles/headings aggressively
        if (isTitleOrHeading(region, image, ocrRegions)) {
            continue;  // Skip titles/headings
        }
        
        // Classify the region
        FormFieldType type = classifyRegionType(region, image, ocrRegions);
        
        // Filter out titles/headings (double check)
        if (type == FormFieldType::Title) {
            continue;  // Skip titles/headings
        }
        
        // Only keep actual form fields
        if (type == FormFieldType::TextLine || 
            type == FormFieldType::TextBlock ||
            type == FormFieldType::Cell ||
            type == FormFieldType::CheckboxField ||
            type == FormFieldType::TextInput) {
            validFields.append(region);
        }
    }
    
    return validFields;
}

FormFieldType FormFieldDetector::classifyRegionType(const cv::Rect& region, 
                                                    const cv::Mat& image,
                                                    const QList<OCRTextRegion>& ocrRegions)
{
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return FormFieldType::Unknown;
    }
    
    // Check if it's a title/heading first
    if (isTitleOrHeading(region, image, ocrRegions)) {
        return FormFieldType::Title;
    }
    
    // Distinguish text line from text block
    if (isTextBlock(region, image)) {
        return FormFieldType::TextBlock;
    }
    
    // Check for single underline (text line)
    cv::Rect clampedRegion(
        std::max(0, region.x),
        std::max(0, region.y),
        std::min(image.cols - std::max(0, region.x), region.width),
        std::min(image.rows - std::max(0, region.y), region.height)
    );
    
    if (clampedRegion.width > 0 && clampedRegion.height > 0) {
        cv::Mat roi = image(clampedRegion);
        
        // Convert to grayscale
        cv::Mat gray;
        if (roi.channels() == 3) {
            cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = roi.clone();
        }
        
        // Look for horizontal lines (underlines)
        cv::Mat binary;
        cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY_INV);
        
        cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 1));
        cv::Mat horizontalLines;
        cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel);
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(horizontalLines, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        // If we find a single strong horizontal line, it's a text line
        if (contours.size() == 1) {
            cv::Rect lineRect = cv::boundingRect(contours[0]);
            if (lineRect.width > region.width * 0.7) {  // Line spans most of width
                return FormFieldType::TextLine;
            }
        }
        
        // Check for box borders (cell)
        cv::Mat edges;
        cv::Canny(gray, edges, 50, 150);
        std::vector<std::vector<cv::Point>> boxContours;
        cv::findContours(edges, boxContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        for (const auto& contour : boxContours) {
            if (contour.size() >= 4) {
                cv::Rect boxRect = cv::boundingRect(contour);
                double contourArea = cv::contourArea(contour);
                double rectArea = boxRect.width * boxRect.height;
                double rectangularity = (rectArea > 0) ? contourArea / rectArea : 0.0;
                
                // If we find a good rectangular border, it's a cell
                if (rectangularity > 0.7 && boxRect.width > 20 && boxRect.height > 10) {
                    return FormFieldType::Cell;
                }
            }
        }
    }
    
    // Default to text input
    return FormFieldType::TextInput;
}

bool FormFieldDetector::isTitleOrHeading(const cv::Rect& region, 
                                         const cv::Mat& image,
                                         const QList<OCRTextRegion>& ocrRegions)
{
    if (image.empty()) {
        return false;
    }
    
    // AGGRESSIVE title/heading detection using multiple heuristics
    
    // Heuristic 1: Check for large OCR text overlapping with region
    int largeTextCount = 0;
    int totalOverlapArea = 0;
    int regionArea = region.width * region.height;
    
    for (const OCRTextRegion& ocrRegion : ocrRegions) {
        cv::Rect ocrBox = ocrRegion.boundingBox;
        
        // Check if OCR text overlaps with region
        int overlapX = std::max(0, std::min(region.x + region.width, ocrBox.x + ocrBox.width) - 
                                  std::max(region.x, ocrBox.x));
        int overlapY = std::max(0, std::min(region.y + region.height, ocrBox.y + ocrBox.height) - 
                                  std::max(region.y, ocrBox.y));
        int overlapArea = overlapX * overlapY;
        
        if (overlapArea > 0) {
            totalOverlapArea += overlapArea;
            
            // Large text size suggests title/heading
            if (ocrBox.height > 18 || ocrBox.width > 80) {
                largeTextCount++;
            }
        }
    }
    
    // If >30% of region is covered by large text, it's likely a title
    if (totalOverlapArea > regionArea * 0.3 && largeTextCount > 0) {
        return true;
    }
    
    // Heuristic 2: Check region size and brightness
    bool isVeryLarge = (region.height > 35 || region.width > image.cols * 0.75);
    bool isVeryWide = (region.width > image.cols * 0.6 && region.height < 30);
    
    if (isVeryLarge || isVeryWide) {
        cv::Rect clampedRegion(
            std::max(0, region.x),
            std::max(0, region.y),
            std::min(image.cols - std::max(0, region.x), region.width),
            std::min(image.rows - std::max(0, region.y), region.height)
        );
        
        if (clampedRegion.width > 0 && clampedRegion.height > 0) {
            cv::Mat roi = image(clampedRegion);
            cv::Mat gray;
            if (roi.channels() == 3) {
                cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
            } else {
                gray = roi.clone();
            }
            
            cv::Scalar meanBrightness = cv::mean(gray);
            double brightness = meanBrightness[0] / 255.0;
            
            // Low brightness + large size = title/heading
            if (brightness < 0.75) {
                return true;
            }
        }
    }
    
    // Heuristic 3: Check for text-like patterns (high edge density)
    cv::Rect clampedRegion(
        std::max(0, region.x),
        std::max(0, region.y),
        std::min(image.cols - std::max(0, region.x), region.width),
        std::min(image.rows - std::max(0, region.y), region.height)
    );
    
    if (clampedRegion.width > 0 && clampedRegion.height > 0) {
        cv::Mat roi = image(clampedRegion);
        cv::Mat gray;
        if (roi.channels() == 3) {
            cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = roi.clone();
        }
        
        // Check edge density
        cv::Mat edges;
        cv::Canny(gray, edges, 50, 150);
        int edgePixels = cv::countNonZero(edges);
        double edgeDensity = static_cast<double>(edgePixels) / (roi.rows * roi.cols);
        
        // High edge density (>0.15) suggests text content (title)
        if (edgeDensity > 0.15 && region.height > 25) {
            return true;
        }
        
        // Check for lack of form field structure (no clear underline or box)
        cv::Mat binary;
        cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY_INV);
        
        // Look for horizontal lines (underlines)
        cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 1));
        cv::Mat horizontalLines;
        cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel);
        
        std::vector<std::vector<cv::Point>> lineContours;
        cv::findContours(horizontalLines, lineContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        // If no clear form field structure (no strong lines/boxes) but has text, it's likely a title
        if (lineContours.empty() && edgeDensity > 0.1) {
            return true;
        }
    }
    
    return false;
}

bool FormFieldDetector::isTextBlock(const cv::Rect& region, const cv::Mat& image)
{
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return false;
    }
    
    // Text blocks (multi-line handwriting areas) have:
    // 1. Multiple horizontal lines (underlines)
    // 2. Taller aspect ratio (height > 2x typical line height)
    // 3. Multiple parallel lines
    
    cv::Rect clampedRegion(
        std::max(0, region.x),
        std::max(0, region.y),
        std::min(image.cols - std::max(0, region.x), region.width),
        std::min(image.rows - std::max(0, region.y), region.height)
    );
    
    if (clampedRegion.width <= 0 || clampedRegion.height <= 0) {
        return false;
    }
    
    // Check aspect ratio - text blocks are taller
    double aspectRatio = static_cast<double>(region.height) / region.width;
    if (aspectRatio < 0.3) {
        return false;  // Too wide, likely single line
    }
    
    // Extract ROI
    cv::Mat roi = image(clampedRegion);
    cv::Mat gray;
    if (roi.channels() == 3) {
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = roi.clone();
    }
    
    // Look for multiple horizontal lines
    cv::Mat binary;
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY_INV);
    
    cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 1));
    cv::Mat horizontalLines;
    cv::morphologyEx(binary, horizontalLines, cv::MORPH_OPEN, horizontalKernel);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(horizontalLines, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Text blocks have multiple lines (3+)
    if (contours.size() >= 3) {
        // Check if lines are roughly evenly spaced (characteristic of handwriting blocks)
        QList<int> lineYs;
        for (const auto& contour : contours) {
            cv::Rect lineRect = cv::boundingRect(contour);
            lineYs.append(lineRect.y + lineRect.height / 2);
        }
        
        std::sort(lineYs.begin(), lineYs.end());
        
        // Check spacing consistency
        if (lineYs.size() >= 2) {
            int avgSpacing = 0;
            for (int i = 1; i < lineYs.size(); i++) {
                avgSpacing += (lineYs[i] - lineYs[i-1]);
            }
            avgSpacing /= (lineYs.size() - 1);
            
            // If lines are roughly evenly spaced, it's a text block
            bool consistentSpacing = true;
            for (int i = 1; i < lineYs.size(); i++) {
                int spacing = lineYs[i] - lineYs[i-1];
                if (std::abs(spacing - avgSpacing) > avgSpacing * 0.3) {
                    consistentSpacing = false;
                    break;
                }
            }
            
            if (consistentSpacing && avgSpacing > 10) {
                return true;  // Text block (multi-line handwriting area)
            }
        }
    }
    
    // Also check height - text blocks are typically taller than single lines
    if (region.height > 40) {  // More than typical line height
        return true;
    }
    
    return false;  // Single text line
}

int FormFieldDetector::findHardEdgeAbove(const cv::Rect& region, const cv::Mat& image, int searchHeight)
{
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return -1;
    }
    
    // Search area above the region
    int searchTop = std::max(0, region.y - searchHeight);
    int searchBottom = region.y;
    int searchLeft = region.x;
    int searchRight = std::min(image.cols, region.x + region.width);
    
    if (searchTop >= searchBottom || searchRight <= searchLeft) {
        return -1;
    }
    
    cv::Rect searchArea(searchLeft, searchTop, searchRight - searchLeft, searchBottom - searchTop);
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Extract ROI
    cv::Mat roi = gray(searchArea);
    
    // Use Canny edge detection to find horizontal edges
    cv::Mat edges;
    cv::Canny(roi, edges, 50, 150);
    
    // Use horizontal morphology to find strong horizontal lines
    cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 1));
    cv::Mat horizontalLines;
    cv::morphologyEx(edges, horizontalLines, cv::MORPH_DILATE, horizontalKernel);
    
    // Find contours (horizontal lines)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(horizontalLines, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    int bestEdgeY = -1;
    int maxWidth = 0;
    
    for (const auto& contour : contours) {
        if (contour.size() < 2) continue;
        
        cv::Rect lineRect = cv::boundingRect(contour);
        
        // Check if line spans most of the search width (strong edge)
        if (lineRect.width > (searchArea.width * 0.6)) {
            // Adjust to full image coordinates
            int edgeY = searchArea.y + lineRect.y + lineRect.height / 2;
            
            // Prefer wider, stronger edges
            if (lineRect.width > maxWidth) {
                maxWidth = lineRect.width;
                bestEdgeY = edgeY;
            }
        }
    }
    
    // Also check using binary thresholding for underlines/separators
    if (bestEdgeY < 0) {
        cv::Mat binary;
        cv::threshold(roi, binary, 127, 255, cv::THRESH_BINARY_INV);
        
        cv::Mat horizontalKernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(40, 1));
        cv::Mat horizontalLines2;
        cv::morphologyEx(binary, horizontalLines2, cv::MORPH_OPEN, horizontalKernel2);
        
        std::vector<std::vector<cv::Point>> lineContours2;
        cv::findContours(horizontalLines2, lineContours2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        for (const auto& contour : lineContours2) {
            if (contour.size() < 2) continue;
            
            cv::Rect lineRect = cv::boundingRect(contour);
            
            // Check if line spans most of the width
            if (lineRect.width > (searchArea.width * 0.5)) {
                int edgeY = searchArea.y + lineRect.y + lineRect.height / 2;
                
                // Prefer lines closer to the region (but not too close)
                int distanceFromRegion = region.y - edgeY;
                if (distanceFromRegion > 5 && distanceFromRegion < searchHeight) {
                    if (lineRect.width > maxWidth) {
                        maxWidth = lineRect.width;
                        bestEdgeY = edgeY;
                    }
                }
            }
        }
    }
    
    return bestEdgeY;  // Returns Y coordinate of hard edge, or -1 if not found
}

int FormFieldDetector::findHardEdgeLeft(const cv::Rect& region, const cv::Mat& image, int searchWidth)
{
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return -1;
    }
    
    // Search area to the LEFT of the region
    int searchLeft = std::max(0, region.x - searchWidth);
    int searchRight = region.x;
    int searchTop = region.y;
    int searchBottom = std::min(image.rows, region.y + region.height);
    
    if (searchLeft >= searchRight || searchBottom <= searchTop) {
        return -1;
    }
    
    cv::Rect searchArea(searchLeft, searchTop, searchRight - searchLeft, searchBottom - searchTop);
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Extract ROI
    cv::Mat roi = gray(searchArea);
    
    // Use MORE SENSITIVE Canny edge detection
    cv::Mat edges;
    cv::Canny(roi, edges, 30, 100);  // Lower thresholds for more sensitivity
    
    // Use vertical morphology to find strong vertical lines
    cv::Mat verticalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 25));
    cv::Mat verticalLines;
    cv::morphologyEx(edges, verticalLines, cv::MORPH_DILATE, verticalKernel, cv::Point(-1, -1), 2);
    
    // Also use binary thresholding
    cv::Mat binary;
    cv::threshold(roi, binary, 127, 255, cv::THRESH_BINARY_INV);
    cv::Mat verticalLines2;
    cv::morphologyEx(binary, verticalLines2, cv::MORPH_OPEN, verticalKernel);
    
    // Combine both detections
    cv::Mat combined;
    cv::bitwise_or(verticalLines, verticalLines2, combined);
    
    // Find contours (vertical lines)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combined, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    int bestEdgeX = -1;
    int maxHeight = 0;
    
    for (const auto& contour : contours) {
        if (contour.size() < 2) continue;
        
        cv::Rect lineRect = cv::boundingRect(contour);
        
        // Check if line spans most of the search height (strong vertical edge)
        if (lineRect.height > (searchArea.height * 0.6)) {
            // Adjust to full image coordinates
            int edgeX = searchArea.x + lineRect.x + lineRect.width / 2;
            
            // Prefer taller, stronger edges
            if (lineRect.height > maxHeight) {
                maxHeight = lineRect.height;
                bestEdgeX = edgeX;
            }
        }
    }
    
    // Also check using binary thresholding for vertical separators
    if (bestEdgeX < 0) {
        cv::Mat binary2;
        cv::threshold(roi, binary2, 127, 255, cv::THRESH_BINARY_INV);
        
        cv::Mat verticalKernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 30));
        cv::Mat verticalLines3;
        cv::morphologyEx(binary2, verticalLines3, cv::MORPH_OPEN, verticalKernel2);
        
        std::vector<std::vector<cv::Point>> lineContours2;
        cv::findContours(verticalLines3, lineContours2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        for (const auto& contour : lineContours2) {
            if (contour.size() < 2) continue;
            
            cv::Rect lineRect = cv::boundingRect(contour);
            
            // Check if line spans most of the height
            if (lineRect.height > (searchArea.height * 0.5)) {
                int edgeX = searchArea.x + lineRect.x + lineRect.width / 2;
                
                // Prefer lines closer to the region (but not too close)
                int distanceFromRegion = region.x - edgeX;
                if (distanceFromRegion > 5 && distanceFromRegion < searchWidth) {
                    if (lineRect.height > maxHeight) {
                        maxHeight = lineRect.height;
                        bestEdgeX = edgeX;
                    }
                }
            }
        }
    }
    
    return bestEdgeX;  // Returns X coordinate of hard edge, or -1 if not found
}

int FormFieldDetector::findHardEdgeRight(const cv::Rect& region, const cv::Mat& image, int searchWidth)
{
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return -1;
    }
    
    // Search area to the RIGHT of the region
    int searchLeft = region.x + region.width;
    int searchRight = std::min(image.cols, region.x + region.width + searchWidth);
    int searchTop = region.y;
    int searchBottom = std::min(image.rows, region.y + region.height);
    
    if (searchLeft >= searchRight || searchBottom <= searchTop) {
        return -1;
    }
    
    cv::Rect searchArea(searchLeft, searchTop, searchRight - searchLeft, searchBottom - searchTop);
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Extract ROI
    cv::Mat roi = gray(searchArea);
    
    // Use MORE SENSITIVE Canny edge detection
    cv::Mat edges;
    cv::Canny(roi, edges, 30, 100);  // Lower thresholds for more sensitivity
    
    // Use vertical morphology to find strong vertical lines
    cv::Mat verticalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 25));
    cv::Mat verticalLines;
    cv::morphologyEx(edges, verticalLines, cv::MORPH_DILATE, verticalKernel, cv::Point(-1, -1), 2);
    
    // Also use binary thresholding
    cv::Mat binary;
    cv::threshold(roi, binary, 127, 255, cv::THRESH_BINARY_INV);
    cv::Mat verticalLines2;
    cv::morphologyEx(binary, verticalLines2, cv::MORPH_OPEN, verticalKernel);
    
    // Combine both detections
    cv::Mat combined;
    cv::bitwise_or(verticalLines, verticalLines2, combined);
    
    // Find contours (vertical lines)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combined, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    int bestEdgeX = -1;
    int maxHeight = 0;
    
    for (const auto& contour : contours) {
        if (contour.size() < 2) continue;
        
        cv::Rect lineRect = cv::boundingRect(contour);
        
        // Check if line spans most of the search height (strong vertical edge)
        if (lineRect.height > (searchArea.height * 0.6)) {
            // Adjust to full image coordinates
            int edgeX = searchArea.x + lineRect.x + lineRect.width / 2;
            
            // Prefer taller, stronger edges
            if (lineRect.height > maxHeight) {
                maxHeight = lineRect.height;
                bestEdgeX = edgeX;
            }
        }
    }
    
    // Also check using binary thresholding for vertical separators
    if (bestEdgeX < 0) {
        cv::Mat binary2;
        cv::threshold(roi, binary2, 127, 255, cv::THRESH_BINARY_INV);
        
        cv::Mat verticalKernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 30));
        cv::Mat verticalLines3;
        cv::morphologyEx(binary2, verticalLines3, cv::MORPH_OPEN, verticalKernel2);
        
        std::vector<std::vector<cv::Point>> lineContours2;
        cv::findContours(verticalLines3, lineContours2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        for (const auto& contour : lineContours2) {
            if (contour.size() < 2) continue;
            
            cv::Rect lineRect = cv::boundingRect(contour);
            
            // Check if line spans most of the height
            if (lineRect.height > (searchArea.height * 0.5)) {
                int edgeX = searchArea.x + lineRect.x + lineRect.width / 2;
                
                // Prefer lines closer to the region (but not too close)
                int distanceFromRegion = edgeX - (region.x + region.width);
                if (distanceFromRegion > 5 && distanceFromRegion < searchWidth) {
                    if (lineRect.height > maxHeight) {
                        maxHeight = lineRect.height;
                        bestEdgeX = edgeX;
                    }
                }
            }
        }
    }
    
    return bestEdgeX;  // Returns X coordinate of hard edge, or -1 if not found
}

QList<int> FormFieldDetector::findVerticalWalls(const cv::Rect& region, const cv::Mat& image)
{
    QList<int> wallXCoords;
    
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return wallXCoords;
    }
    
    // Expand search area MORE to catch walls at edges and between cells
    cv::Rect searchArea(
        std::max(0, region.x - 15),  // More expansion
        std::max(0, region.y - 5),
        std::min(image.cols - std::max(0, region.x - 15), region.width + 30),  // More expansion
        std::min(image.rows - std::max(0, region.y - 5), region.height + 10)
    );
    
    if (searchArea.width <= 0 || searchArea.height <= 0) {
        return wallXCoords;
    }
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Extract ROI
    cv::Mat roi = gray(searchArea);
    
    // EXTREMELY SENSITIVE edge detection for thin vertical lines (cell walls)
    cv::Mat edges;
    cv::Canny(roi, edges, 20, 80);  // Even lower thresholds for maximum sensitivity
    
    // Use vertical morphology with longer kernel to catch full-height walls
    cv::Mat verticalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 20));  // Taller kernel
    cv::Mat verticalLines;
    cv::morphologyEx(edges, verticalLines, cv::MORPH_DILATE, verticalKernel, cv::Point(-1, -1), 3);  // More iterations
    
    // Also use binary thresholding with adaptive threshold for better detection
    cv::Mat binary;
    cv::adaptiveThreshold(roi, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY_INV, 11, 2);
    cv::Mat verticalLines2;
    cv::morphologyEx(binary, verticalLines2, cv::MORPH_OPEN, verticalKernel, cv::Point(-1, -1), 2);
    
    // Also try regular thresholding
    cv::Mat binary2;
    cv::threshold(roi, binary2, 140, 255, cv::THRESH_BINARY_INV);  // Slightly higher threshold
    cv::Mat verticalLines3;
    cv::morphologyEx(binary2, verticalLines3, cv::MORPH_OPEN, verticalKernel);
    
    // Combine all three detections
    cv::Mat combined;
    cv::bitwise_or(verticalLines, verticalLines2, combined);
    cv::bitwise_or(combined, verticalLines3, combined);
    
    // Find contours (vertical lines/walls)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combined, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        if (contour.size() < 2) continue;
        
        cv::Rect lineRect = cv::boundingRect(contour);
        
        // MORE LENIENT: Check if it's a vertical line (height > width * 1.5, and height > 40% of region height)
        // This catches thinner walls between cells
        if (lineRect.height > lineRect.width * 1.5 && 
            lineRect.height > region.height * 0.4) {
            
            // Get X coordinate (center of line)
            int wallX = searchArea.x + lineRect.x + lineRect.width / 2;
            
            // Check if wall is within or near the region (wider tolerance)
            if (wallX >= region.x - 15 && wallX <= region.x + region.width + 15) {
                wallXCoords.append(wallX);
            }
        }
    }
    
    // Also check for very thin walls (1-2px wide) that might be missed
    // Use HoughLinesP for line detection
    cv::Mat houghEdges;
    cv::Canny(roi, houghEdges, 20, 80);
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(houghEdges, lines, 1, CV_PI/180, 30, 20, 10);  // Detect short lines too
    
    for (const cv::Vec4i& line : lines) {
        int x1 = line[0];
        int y1 = line[1];
        int x2 = line[2];
        int y2 = line[3];
        
        // Check if it's a vertical line (similar X coordinates, different Y)
        if (std::abs(x1 - x2) < 3 && std::abs(y1 - y2) > region.height * 0.4) {
            int wallX = searchArea.x + (x1 + x2) / 2;
            
            if (wallX >= region.x - 15 && wallX <= region.x + region.width + 15) {
                wallXCoords.append(wallX);
            }
        }
    }
    
    // Sort wall X coordinates
    std::sort(wallXCoords.begin(), wallXCoords.end());
    
    // Remove duplicates (walls within 3px of each other - tighter for accuracy)
    QList<int> uniqueWalls;
    for (int x : wallXCoords) {
        if (uniqueWalls.isEmpty() || std::abs(x - uniqueWalls.last()) > 3) {
            uniqueWalls.append(x);
        }
    }
    
    return uniqueWalls;
}

QList<int> FormFieldDetector::findHorizontalEdges(const cv::Rect& region, const cv::Mat& image)
{
    QList<int> edgeYCoords;
    
    if (image.empty() || region.width <= 0 || region.height <= 0) {
        return edgeYCoords;
    }
    
    // Expand search area slightly
    cv::Rect searchArea(
        std::max(0, region.x),
        std::max(0, region.y - 5),
        std::min(image.cols - std::max(0, region.x), region.width),
        std::min(image.rows - std::max(0, region.y - 5), region.height + 10)
    );
    
    if (searchArea.width <= 0 || searchArea.height <= 0) {
        return edgeYCoords;
    }
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Extract ROI
    cv::Mat roi = gray(searchArea);
    
    // MORE SENSITIVE edge detection
    cv::Mat edges;
    cv::Canny(roi, edges, 30, 100);  // Lower thresholds for more sensitivity
    
    // Use horizontal morphology to find horizontal lines
    cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(25, 1));  // Wider kernel
    cv::Mat horizontalLines;
    cv::morphologyEx(edges, horizontalLines, cv::MORPH_DILATE, horizontalKernel, cv::Point(-1, -1), 2);
    
    // Also use binary thresholding
    cv::Mat binary;
    cv::threshold(roi, binary, 127, 255, cv::THRESH_BINARY_INV);
    cv::Mat horizontalLines2;
    cv::morphologyEx(binary, horizontalLines2, cv::MORPH_OPEN, horizontalKernel);
    
    // Combine both detections
    cv::Mat combined;
    cv::bitwise_or(horizontalLines, horizontalLines2, combined);
    
    // Find contours (horizontal lines)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combined, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        if (contour.size() < 2) continue;
        
        cv::Rect lineRect = cv::boundingRect(contour);
        
        // Check if it's a horizontal line (width > height, and width > 50% of region width)
        if (lineRect.width > lineRect.height * 2 && 
            lineRect.width > region.width * 0.5) {
            
            // Get Y coordinate (center of line)
            int edgeY = searchArea.y + lineRect.y + lineRect.height / 2;
            
            // Check if edge is within or near the region
            if (edgeY >= region.y - 10 && edgeY <= region.y + region.height + 10) {
                edgeYCoords.append(edgeY);
            }
        }
    }
    
    // Sort edge Y coordinates
    std::sort(edgeYCoords.begin(), edgeYCoords.end());
    
    // Remove duplicates (edges within 5px of each other)
    QList<int> uniqueEdges;
    for (int y : edgeYCoords) {
        if (uniqueEdges.isEmpty() || std::abs(y - uniqueEdges.last()) > 5) {
            uniqueEdges.append(y);
        }
    }
    
    return uniqueEdges;
}

QList<QList<cv::Rect>> FormFieldDetector::detectCellGroupsWithSharedWalls(const QList<cv::Rect>& regions, 
                                                                          const cv::Mat& image)
{
    QList<QList<cv::Rect>> cellGroups;
    
    if (image.empty() || regions.isEmpty()) {
        return cellGroups;
    }
    
    // Convert to grayscale
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // EXTREMELY SENSITIVE detection of all vertical walls in the image
    cv::Mat edges;
    cv::Canny(gray, edges, 20, 80);  // Very sensitive for thin walls
    
    // Use vertical morphology with longer kernel
    cv::Mat verticalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 25));
    cv::Mat verticalLines;
    cv::morphologyEx(edges, verticalLines, cv::MORPH_DILATE, verticalKernel, cv::Point(-1, -1), 3);
    
    // Also use adaptive thresholding
    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY_INV, 11, 2);
    cv::Mat verticalLines2;
    cv::morphologyEx(binary, verticalLines2, cv::MORPH_OPEN, verticalKernel, cv::Point(-1, -1), 2);
    
    // Regular thresholding too
    cv::Mat binary2;
    cv::threshold(gray, binary2, 140, 255, cv::THRESH_BINARY_INV);
    cv::Mat verticalLines3;
    cv::morphologyEx(binary2, verticalLines3, cv::MORPH_OPEN, verticalKernel);
    
    // Combine all three
    cv::Mat combined;
    cv::bitwise_or(verticalLines, verticalLines2, combined);
    cv::bitwise_or(combined, verticalLines3, combined);
    
    // Find all vertical wall X coordinates
    std::vector<std::vector<cv::Point>> wallContours;
    cv::findContours(combined, wallContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    QList<int> allWallXCoords;
    for (const auto& contour : wallContours) {
        if (contour.size() < 2) continue;
        
        cv::Rect wallRect = cv::boundingRect(contour);
        
        // MORE LENIENT: Check if it's a vertical wall (height > width * 1.5, and height > 25px)
        // This catches thinner walls between cells
        if (wallRect.height > wallRect.width * 1.5 && wallRect.height > 25) {
            int wallX = wallRect.x + wallRect.width / 2;
            allWallXCoords.append(wallX);
        }
    }
    
    // Also use HoughLinesP for very thin walls
    std::vector<cv::Vec4i> houghLines;
    cv::HoughLinesP(edges, houghLines, 1, CV_PI/180, 25, 15, 8);  // Detect shorter lines too
    
    for (const cv::Vec4i& line : houghLines) {
        int x1 = line[0];
        int y1 = line[1];
        int x2 = line[2];
        int y2 = line[3];
        
        // Check if it's a vertical line
        if (std::abs(x1 - x2) < 3 && std::abs(y1 - y2) > 25) {
            int wallX = (x1 + x2) / 2;
            allWallXCoords.append(wallX);
        }
    }
    
    // Sort and deduplicate walls (tighter tolerance for accuracy)
    std::sort(allWallXCoords.begin(), allWallXCoords.end());
    QList<int> uniqueWalls;
    for (int x : allWallXCoords) {
        if (uniqueWalls.isEmpty() || std::abs(x - uniqueWalls.last()) > 2) {  // 2px tolerance
            uniqueWalls.append(x);
        }
    }
    
    // Group regions that share walls (are separated by vertical lines)
    QList<bool> processed(regions.size(), false);
    
    for (int i = 0; i < regions.size(); i++) {
        if (processed[i]) continue;
        
        QList<cv::Rect> group;
        group.append(regions[i]);
        processed[i] = true;
        
        // Find regions that share walls with this one
        cv::Rect currentRegion = regions[i];
        
        // Find walls within this region
        QList<int> regionWalls = findVerticalWalls(currentRegion, image);
        
        // Look for other regions that align with these walls
        for (int j = i + 1; j < regions.size(); j++) {
            if (processed[j]) continue;
            
            cv::Rect otherRegion = regions[j];
            
            // Check if regions are horizontally aligned (similar Y and height)
            int yDiff = std::abs(currentRegion.y - otherRegion.y);
            int heightDiff = std::abs(currentRegion.height - otherRegion.height);
            
            if (yDiff < 10 && heightDiff < 10) {
                // Check if they share a wall (one region's edge aligns with a wall)
                bool sharesWall = false;
                
                // Check if left edge of other region aligns with right edge of current (or vice versa)
                int leftRightGap = otherRegion.x - (currentRegion.x + currentRegion.width);
                int rightLeftGap = currentRegion.x - (otherRegion.x + otherRegion.width);
                
                // Check if gap is small and contains a wall
                if (leftRightGap >= 0 && leftRightGap < 20) {
                    // Check if there's a wall in the gap
                    for (int wallX : uniqueWalls) {
                        if (wallX >= currentRegion.x + currentRegion.width - 5 &&
                            wallX <= otherRegion.x + 5) {
                            sharesWall = true;
                            break;
                        }
                    }
                } else if (rightLeftGap >= 0 && rightLeftGap < 20) {
                    for (int wallX : uniqueWalls) {
                        if (wallX >= otherRegion.x + otherRegion.width - 5 &&
                            wallX <= currentRegion.x + 5) {
                            sharesWall = true;
                            break;
                        }
                    }
                }
                
                // Also check if regions are directly adjacent (touching or very close)
                if (!sharesWall) {
                    int gap = std::min(leftRightGap, rightLeftGap);
                    if (gap >= 0 && gap < 5) {
                        sharesWall = true;  // Directly adjacent, likely share a wall
                    }
                }
                
                if (sharesWall) {
                    group.append(otherRegion);
                    processed[j] = true;
                    currentRegion = otherRegion;  // Continue from this region
                }
            }
        }
        
        // If we found a group of 2+ cells, add it
        if (group.size() >= 2) {
            cellGroups.append(group);
        }
    }
    
    return cellGroups;
}

} // namespace ocr_orc
