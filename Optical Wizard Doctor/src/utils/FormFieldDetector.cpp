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
        
        // If we found a good field, use it; otherwise use original overfitted region
        if (bestField.width > 0 && bestField.height > 0) {
            refined.append(bestField);
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
                        // Create field above the line
                        bestLine = cv::Rect(
                            lineRect.x,
                            std::max(0, lineRect.y - 20),  // 20px above line
                            lineRect.width,
                            20  // 20px height
                        );
                    }
                }
                
                if (bestLine.width > 0) {
                    refined.append(bestLine);
                } else {
                    refined.append(overfitted);  // Fallback to overfitted
                }
            } else {
                refined.append(overfitted);  // Fallback to overfitted
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

} // namespace ocr_orc
