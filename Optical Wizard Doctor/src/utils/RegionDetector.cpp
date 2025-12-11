#include "RegionDetector.h"
#include "ImageConverter.h"
#include "TypeInferencer.h"
#include "GroupInferencer.h"
#include "SpatialClusterer.h"
#include "patterns/PostalCodePatternDetector.h"
#include "patterns/NameFieldPatternDetector.h"
#include "patterns/NumberSequencePatternDetector.h"
#include "OcrTextExtractor.h"
#include "TextRegionRefiner.h"
#include "CheckboxDetector.h"
#include "PatternAnalyzer.h"
#include "FormFieldDetector.h"
#include "ConfidenceCalculator.h"
#include "RectangleDetector.h"
#include "../core/CoordinateSystem.h"
#include <QtGui/QImage>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <cmath>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

RegionDetector::RegionDetector()
    : minCellWidth(20)
    , minCellHeight(20)
    , maxCellWidth(200)
    , maxCellHeight(200)
    , lineThreshold(100)
    , contourMinArea(400)
{
}

void RegionDetector::setMinCellSize(int width, int height) {
    minCellWidth = width;
    minCellHeight = height;
}

void RegionDetector::setMaxCellSize(int width, int height) {
    maxCellWidth = width;
    maxCellHeight = height;
}

void RegionDetector::setLineDetectionThreshold(int threshold) {
    lineThreshold = threshold;
}

void RegionDetector::setContourMinArea(int area) {
    contourMinArea = area;
}

NormalizedCoords RegionDetector::convertToNormalized(const cv::Rect& rect, int imgWidth, int imgHeight) {
    // Convert cv::Rect (x, y, width, height) to ImageCoords
    ImageCoords imgCoords;
    imgCoords.x1 = rect.x;
    imgCoords.y1 = rect.y;
    imgCoords.x2 = rect.x + rect.width;
    imgCoords.y2 = rect.y + rect.height;
    
    // Convert to normalized coordinates
    return CoordinateSystem::imageToNormalized(imgCoords, imgWidth, imgHeight);
}

DetectionResult RegionDetector::detectRegions(const QImage& image, const QString& method) {
    if (image.isNull()) {
        DetectionResult emptyResult;
        emptyResult.methodUsed = "none";
        return emptyResult;
    }
    
    // Check for OCR-first method
    if (method == "ocr-first") {
        return detectRegionsOCRFirst(image, method);
    }
    
    // Multi-scale detection: process at 3 scales and merge results
    QList<DetectionResult> scaleResults;
    QList<double> scales = {0.5, 1.0, 2.0};
    
    for (double scale : scales) {
        DetectionResult result = detectAtScale(image, method, scale);
        scaleResults.append(result);
    }
    
    // Merge results from all scales
    DetectionResult mergedResult = mergeScaleResults(scaleResults, image.size());
    mergedResult.methodUsed = method == "auto" ? "hybrid" : method;
    
    // Apply smart detection enhancements
    if (!mergedResult.regions.isEmpty()) {
        // Convert image to cv::Mat for type inference
        cv::Mat cvImage = ImageConverter::qImageToMat(image);
        
        // Step 1: Type inference for each region
        TypeInferencer typeInferencer;
        for (DetectedRegion& region : mergedResult.regions) {
            // Extract ROI for type inference
            int x1 = (int)(region.coords.x1 * image.width());
            int y1 = (int)(region.coords.y1 * image.height());
            int x2 = (int)(region.coords.x2 * image.width());
            int y2 = (int)(region.coords.y2 * image.height());
            
            // Ensure bounds
            x1 = std::max(0, std::min(x1, image.width() - 1));
            y1 = std::max(0, std::min(y1, image.height() - 1));
            x2 = std::max(x1 + 1, std::min(x2, image.width()));
            y2 = std::max(y1 + 1, std::min(y2, image.height()));
            
            if (x2 > x1 && y2 > y1 && !cvImage.empty()) {
                cv::Rect roiRect(x1, y1, x2 - x1, y2 - y1);
                cv::Rect validRect = roiRect & cv::Rect(0, 0, cvImage.cols, cvImage.rows);
                if (validRect.area() > 0) {
                    cv::Mat roi = cvImage(validRect);
                    double aspectRatio = (double)(x2 - x1) / (double)(y2 - y1);
                    region.inferredType = typeInferencer.inferTypeFromVisual(roi, aspectRatio);
                }
            }
        }
        
        // Step 2: Pattern detection
        PostalCodePatternDetector postalDetector;
        NameFieldPatternDetector nameDetector;
        NumberSequencePatternDetector numberDetector;
        
        // Detect postal code patterns
        QList<DetectedRegion> postalRegions = postalDetector.groupPostalCode(mergedResult.regions);
        if (!postalRegions.isEmpty()) {
            // Update regions with postal code pattern info
            for (DetectedRegion& region : mergedResult.regions) {
                for (const DetectedRegion& postalRegion : postalRegions) {
                    // Check if this region matches a postal code region
                    double overlap = calculateOverlap(region.coords, postalRegion.coords);
                    if (overlap > 0.8) {
                        region.inferredType = postalRegion.inferredType;
                        region.suggestedGroup = postalRegion.suggestedGroup;
                        region.suggestedColor = postalRegion.suggestedColor;
                        break;
                    }
                }
            }
        }
        
        // Detect name field patterns
        QList<QList<DetectedRegion>> nameFields = nameDetector.detectNameFields(mergedResult.regions);
        for (const auto& nameField : nameFields) {
            for (const DetectedRegion& nameRegion : nameField) {
                // Update matching regions
                for (DetectedRegion& region : mergedResult.regions) {
                    double overlap = calculateOverlap(region.coords, nameRegion.coords);
                    if (overlap > 0.8) {
                        if (region.suggestedGroup.isEmpty()) {
                            region.inferredType = nameRegion.inferredType;
                            region.suggestedGroup = nameRegion.suggestedGroup;
                            region.suggestedColor = nameRegion.suggestedColor;
                        }
                        break;
                    }
                }
            }
        }
        
        // Detect number sequence patterns
        QList<QList<DetectedRegion>> numberSequences = numberDetector.detectNumberSequences(mergedResult.regions);
        for (const auto& numberSequence : numberSequences) {
            for (const DetectedRegion& numberRegion : numberSequence) {
                // Update matching regions
                for (DetectedRegion& region : mergedResult.regions) {
                    double overlap = calculateOverlap(region.coords, numberRegion.coords);
                    if (overlap > 0.8) {
                        if (region.suggestedGroup.isEmpty()) {
                            region.inferredType = numberRegion.inferredType;
                            region.suggestedGroup = numberRegion.suggestedGroup;
                            region.suggestedColor = numberRegion.suggestedColor;
                        }
                        break;
                    }
                }
            }
        }
        
        // Step 3: Spatial clustering and grid detection
        SpatialClusterer clusterer;
        mergedResult.detectedGrid = clusterer.detectGridStructure(mergedResult.regions);
        
        // Step 4: Group inference
        GroupInferencer groupInferencer;
        QList<DetectedGroup> spatialGroups = groupInferencer.inferGroupsFromSpatial(mergedResult.regions);
        QList<DetectedGroup> patternGroups = groupInferencer.inferGroupsFromPatterns(mergedResult.regions);
        
        // Combine groups (prefer pattern-based groups)
        QMap<QString, DetectedGroup> combinedGroups;
        for (const DetectedGroup& group : spatialGroups) {
            combinedGroups[group.name] = group;
        }
        for (const DetectedGroup& group : patternGroups) {
            // Pattern groups override spatial groups
            combinedGroups[group.name] = group;
        }
        
        // Convert map values to list
        mergedResult.inferredGroups.clear();
        for (auto it = combinedGroups.begin(); it != combinedGroups.end(); ++it) {
            mergedResult.inferredGroups.append(it.value());
        }
        
        // Step 5: Build region type and color maps
        for (int i = 0; i < mergedResult.regions.size(); ++i) {
            const DetectedRegion& region = mergedResult.regions[i];
            if (!region.inferredType.isEmpty() && region.inferredType != "unknown") {
                // Generate a temporary name for mapping (will be replaced with actual names later)
                QString tempName = QString("region_%1").arg(i);
                mergedResult.regionTypes[tempName] = region.inferredType;
            }
        }
        
        for (const DetectedGroup& group : mergedResult.inferredGroups) {
            mergedResult.suggestedColors[group.name] = group.suggestedColor;
        }
        
        #if OCR_ORC_DEBUG_ENABLED
        qDebug() << "[RegionDetector] Smart detection complete:"
                 << mergedResult.regions.size() << "regions,"
                 << mergedResult.inferredGroups.size() << "groups,"
                 << "grid:" << mergedResult.detectedGrid.rows << "x" << mergedResult.detectedGrid.cols;
        #endif
    }
    
    return mergedResult;
}

DetectionResult RegionDetector::detectAtScale(const QImage& image, const QString& method, double scale) {
    if (image.isNull() || scale <= 0.0) {
        DetectionResult emptyResult;
        emptyResult.methodUsed = "none";
        return emptyResult;
    }
    
    // Scale the image
    QImage scaledImage = image.scaled(
        (int)(image.width() * scale),
        (int)(image.height() * scale),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    
    // Detect regions at this scale
    DetectionResult result;
    if (method == "grid") {
        result = detectGrid(scaledImage);
    } else if (method == "contour") {
        result = detectContours(scaledImage);
    } else if (method == "hybrid") {
        result = detectHybrid(scaledImage);
    } else { // "auto"
        result = detectHybrid(scaledImage);
    }
    
    // Adjust coordinates back to original image scale
    for (DetectedRegion& region : result.regions) {
        // Scale coordinates back to original image size
        region.coords.x1 /= scale;
        region.coords.x2 /= scale;
        region.coords.y1 /= scale;
        region.coords.y2 /= scale;
        
        // Ensure coordinates are within [0, 1] range
        region.coords.x1 = std::max(0.0, std::min(1.0, region.coords.x1));
        region.coords.x2 = std::max(0.0, std::min(1.0, region.coords.x2));
        region.coords.y1 = std::max(0.0, std::min(1.0, region.coords.y1));
        region.coords.y2 = std::max(0.0, std::min(1.0, region.coords.y2));
        
        // Adjust confidence based on scale (full scale gets highest confidence)
        if (scale != 1.0) {
            // Slightly lower confidence for scaled detections
            region.confidence *= (scale == 1.0 ? 1.0 : 0.9);
        }
    }
    
    return result;
}

DetectionResult RegionDetector::mergeScaleResults(const QList<DetectionResult>& scaleResults, const QSize& originalSize) {
    DetectionResult merged;
    merged.methodUsed = "multi-scale";
    
    // Collect all regions from all scales
    QList<DetectedRegion> allRegions;
    for (const DetectionResult& result : scaleResults) {
        allRegions.append(result.regions);
    }
    
    if (allRegions.isEmpty()) {
        return merged;
    }
    
    // Remove duplicates based on overlap
    // If two regions overlap >80%, keep the one with higher confidence
    QList<DetectedRegion> uniqueRegions;
    for (int i = 0; i < allRegions.size(); ++i) {
        bool isDuplicate = false;
        for (int j = i + 1; j < allRegions.size(); ++j) {
            double overlap = calculateOverlap(allRegions[i].coords, allRegions[j].coords);
            if (overlap > 0.8) { // 80% overlap threshold
                // Keep region with higher confidence
                if (allRegions[i].confidence < allRegions[j].confidence) {
                    isDuplicate = true;
                    break;
                }
            }
        }
        if (!isDuplicate) {
            uniqueRegions.append(allRegions[i]);
        }
    }
    
    merged.regions = uniqueRegions;
    merged.totalDetected = uniqueRegions.size();
    
    // Count confidence levels
    for (const auto& region : uniqueRegions) {
        if (region.confidence >= 0.8) {
            merged.highConfidence++;
        } else if (region.confidence >= 0.5) {
            merged.mediumConfidence++;
        } else {
            merged.lowConfidence++;
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[RegionDetector] Multi-scale detection: Merged" 
             << scaleResults.size() << "scales into" 
             << merged.totalDetected << "unique regions";
    #endif
    
    return merged;
}

DetectionResult RegionDetector::detectGrid(const QImage& image) {
    DetectionResult result;
    result.methodUsed = "grid";
    
    if (image.isNull()) {
        return result;
    }
    
    // Step 1.3.1: Image Preprocessing
    cv::Mat cvImage = ImageConverter::qImageToMat(image);
    if (cvImage.empty()) {
        return result;
    }
    
    int imgWidth = image.width();
    int imgHeight = image.height();
    
    // Convert to grayscale
    cv::Mat gray;
    if (cvImage.channels() == 3) {
        cv::cvtColor(cvImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = cvImage.clone();
    }
    
    // Apply Gaussian blur to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    
    // Apply adaptive threshold to create binary image
    cv::Mat binary;
    cv::adaptiveThreshold(blurred, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                         cv::THRESH_BINARY_INV, 11, 2);
    
    // Step 1.3.2: Hough Line Detection
    // Detect edges using Canny
    cv::Mat edges;
    cv::Canny(binary, edges, 50, 150);
    
    // Detect lines using HoughLinesP
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI/180, lineThreshold, 50, 10);
    
    // Separate horizontal and vertical lines
    std::vector<cv::Vec4i> horizontalLines, verticalLines;
    for (const auto& line : lines) {
        int x1 = line[0], y1 = line[1], x2 = line[2], y2 = line[3];
        double angle = std::atan2((double)(y2 - y1), (double)(x2 - x1)) * 180.0 / CV_PI;
        double absAngle = std::abs(angle);
        
        // Horizontal lines: angle close to 0 or 180 degrees
        if (absAngle < 10 || absAngle > 170) {
            horizontalLines.push_back(line);
        }
        // Vertical lines: angle close to 90 or -90 degrees
        else if (std::abs(absAngle - 90) < 10) {
            verticalLines.push_back(line);
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[RegionDetector] Grid detection: Found" << horizontalLines.size() 
             << "horizontal and" << verticalLines.size() << "vertical lines";
    #endif
    
    // Step 1.3.3: Line Intersection Detection
    std::vector<cv::Point2f> intersections;
    cv::Size imageSize(cvImage.cols, cvImage.rows);
    
    for (const auto& hLine : horizontalLines) {
        for (const auto& vLine : verticalLines) {
            cv::Point2f intersection = findLineIntersection(hLine, vLine);
            if (isValidIntersection(intersection, imageSize)) {
                intersections.push_back(intersection);
            }
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[RegionDetector] Found" << intersections.size() << "line intersections";
    #endif
    
    // Step 1.3.4: Grid Cell Extraction
    if (intersections.size() < 4) {
        // Not enough intersections to form a grid
        return result;
    }
    
    // Sort intersections by Y coordinate (rows)
    std::sort(intersections.begin(), intersections.end(), 
              [](const cv::Point2f& a, const cv::Point2f& b) {
                  return a.y < b.y;
              });
    
    // Group into rows (tolerance: 5 pixels)
    std::vector<std::vector<cv::Point2f>> rows;
    double rowTolerance = 5.0;
    
    for (const auto& point : intersections) {
        bool addedToRow = false;
        for (auto& row : rows) {
            if (!row.empty() && std::abs(row[0].y - point.y) < rowTolerance) {
                row.push_back(point);
                addedToRow = true;
                break;
            }
        }
        if (!addedToRow) {
            rows.push_back({point});
        }
    }
    
    // Sort each row by X coordinate
    for (auto& row : rows) {
        std::sort(row.begin(), row.end(), 
                  [](const cv::Point2f& a, const cv::Point2f& b) {
                      return a.x < b.x;
                  });
    }
    
    // Form grid cells from intersections
    QList<DetectedRegion> detectedRegions;
    
    for (size_t i = 0; i < rows.size() - 1; ++i) {
        for (size_t j = 0; j < rows[i].size() - 1; ++j) {
            cv::Point2f topLeft = rows[i][j];
            cv::Point2f topRight = rows[i][j + 1];
            cv::Point2f bottomLeft = rows[i + 1][j];
            cv::Point2f bottomRight = rows[i + 1][j + 1];
            
            // Create bounding rectangle
            float x1 = std::min({topLeft.x, bottomLeft.x});
            float y1 = std::min({topLeft.y, topRight.y});
            float x2 = std::max({topRight.x, bottomRight.x});
            float y2 = std::max({bottomLeft.y, bottomRight.y});
            
            cv::Rect cellRect((int)x1, (int)y1, (int)(x2 - x1), (int)(y2 - y1));
            
            // Filter by size
            if (cellRect.width < minCellWidth || cellRect.height < minCellHeight) continue;
            if (cellRect.width > maxCellWidth || cellRect.height > maxCellHeight) continue;
            
            // Calculate confidence
            double confidence = calculateConfidence(cellRect, cvImage, "grid");
            
            // Apply size consistency validation
            QList<DetectedRegion> currentRegions;
            for (const auto& r : detectedRegions) {
                currentRegions.append(r);
            }
            double sizeConsistency = validateSizeConsistency(cellRect, currentRegions, imgWidth, imgHeight, 0.1);
            confidence *= sizeConsistency;
            
            // Convert to normalized coordinates
            NormalizedCoords normCoords = convertToNormalized(cellRect, imgWidth, imgHeight);
            
            // Create detected region
            DetectedRegion region(normCoords, confidence, "grid", cellRect);
            detectedRegions.append(region);
        }
    }
    
    // Remove duplicates (overlapping cells, keep highest confidence)
    QList<DetectedRegion> uniqueRegions;
    for (int i = 0; i < detectedRegions.size(); ++i) {
        bool isDuplicate = false;
        for (int j = i + 1; j < detectedRegions.size(); ++j) {
            double overlap = calculateOverlap(detectedRegions[i].coords, detectedRegions[j].coords);
            if (overlap > 0.8) { // 80% overlap threshold
                // Keep region with higher confidence
                if (detectedRegions[i].confidence < detectedRegions[j].confidence) {
                    isDuplicate = true;
                    break;
                }
            }
        }
        if (!isDuplicate) {
            uniqueRegions.append(detectedRegions[i]);
        }
    }
    
    // Step 1.3.5: Grid Detection Result Assembly
    result.regions = uniqueRegions;
    result.totalDetected = uniqueRegions.size();
    
    // Count confidence levels
    for (const auto& region : uniqueRegions) {
        if (region.confidence >= 0.8) {
            result.highConfidence++;
        } else if (region.confidence >= 0.5) {
            result.mediumConfidence++;
        } else {
            result.lowConfidence++;
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[RegionDetector] Grid detection complete:"
             << result.totalDetected << "regions ("
             << result.highConfidence << "high," << result.mediumConfidence 
             << "medium," << result.lowConfidence << "low confidence)";
    #endif
    
    return result;
}

DetectionResult RegionDetector::detectContours(const QImage& image) {
    DetectionResult result;
    result.methodUsed = "contour";
    
    if (image.isNull()) {
        return result;
    }
    
    // Step 2.1.1: Image Preprocessing
    cv::Mat cvImage = ImageConverter::qImageToMat(image);
    if (cvImage.empty()) {
        return result;
    }
    
    int imgWidth = image.width();
    int imgHeight = image.height();
    
    // Convert to grayscale
    cv::Mat gray;
    if (cvImage.channels() == 3) {
        cv::cvtColor(cvImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = cvImage.clone();
    }
    
    // Apply adaptive threshold to create binary image
    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                         cv::THRESH_BINARY_INV, 11, 2);
    
    // Optional: Apply morphological operations to clean up
    cv::Mat cleaned;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(binary, cleaned, cv::MORPH_CLOSE, kernel);
    
    // Step 2.1.2: Contour Finding
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(cleaned, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[RegionDetector] Contour detection: Found" << contours.size() << "contours";
    #endif
    
    // Step 2.1.3: Contour Filtering
    QList<DetectedRegion> detectedRegions;
    
    for (const auto& contour : contours) {
        // Get bounding rectangle
        cv::Rect rect = cv::boundingRect(contour);
        
        // Filter by size
        if (rect.width < minCellWidth || rect.height < minCellHeight) continue;
        if (rect.width > maxCellWidth || rect.height > maxCellHeight) continue;
        
        // Filter by area
        double area = cv::contourArea(contour);
        if (area < contourMinArea) continue;
        
        // Calculate aspect ratio
        double aspectRatio = (double)rect.width / rect.height;
        if (aspectRatio < 0.3 || aspectRatio > 3.0) continue;
        
        // Calculate rectangularity (how close to rectangle)
        double rectArea = rect.width * rect.height;
        double rectangularity = area / rectArea;
        if (rectangularity < 0.7) continue; // Not rectangular enough
        
        // Step 2.1.4: Grid Alignment (snap to common positions)
        // Find common X and Y positions by clustering
        // For now, skip alignment - can be added later if needed
        
        // Calculate confidence
        double confidence = calculateConfidence(rect, cvImage, "contour");
        
        // Apply size consistency validation
        QList<DetectedRegion> currentRegions;
        for (const auto& r : detectedRegions) {
            currentRegions.append(r);
        }
        double sizeConsistency = validateSizeConsistency(rect, currentRegions, imgWidth, imgHeight, 0.1);
        confidence *= sizeConsistency;
        
        // Convert to normalized coordinates
        NormalizedCoords normCoords = convertToNormalized(rect, imgWidth, imgHeight);
        
        // Create detected region
        DetectedRegion region(normCoords, confidence, "contour", rect);
        detectedRegions.append(region);
    }
    
    // Remove duplicates (overlapping regions, keep highest confidence)
    QList<DetectedRegion> uniqueRegions;
    for (int i = 0; i < detectedRegions.size(); ++i) {
        bool isDuplicate = false;
        for (int j = i + 1; j < detectedRegions.size(); ++j) {
            double overlap = calculateOverlap(detectedRegions[i].coords, detectedRegions[j].coords);
            if (overlap > 0.8) { // 80% overlap threshold
                // Keep region with higher confidence
                if (detectedRegions[i].confidence < detectedRegions[j].confidence) {
                    isDuplicate = true;
                    break;
                }
            }
        }
        if (!isDuplicate) {
            uniqueRegions.append(detectedRegions[i]);
        }
    }
    
    // Step 2.1.5: Contour Detection Result Assembly
    result.regions = uniqueRegions;
    result.totalDetected = uniqueRegions.size();
    
    // Count confidence levels
    for (const auto& region : uniqueRegions) {
        if (region.confidence >= 0.8) {
            result.highConfidence++;
        } else if (region.confidence >= 0.5) {
            result.mediumConfidence++;
        } else {
            result.lowConfidence++;
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[RegionDetector] Contour detection complete:"
             << result.totalDetected << "regions ("
             << result.highConfidence << "high," << result.mediumConfidence 
             << "medium," << result.lowConfidence << "low confidence)";
    #endif
    
    return result;
}

DetectionResult RegionDetector::detectHybrid(const QImage& image) {
    DetectionResult result;
    result.methodUsed = "hybrid";
    
    if (image.isNull()) {
        return result;
    }
    
    // Step 2.2.1: Try grid detection first
    DetectionResult gridResult = detectGrid(image);
    
    // If grid detection found good results (>= 10 regions with high confidence)
    if (gridResult.totalDetected >= 10 && gridResult.highConfidence >= 5) {
        // Also try contour detection
        DetectionResult contourResult = detectContours(image);
        
        // Merge results
        result = mergeDetectionResults(gridResult, contourResult);
        result.methodUsed = "hybrid";
        
        #if OCR_ORC_DEBUG_ENABLED
        qDebug() << "[RegionDetector] Hybrid detection: Merged" 
                 << gridResult.totalDetected << "grid +" 
                 << contourResult.totalDetected << "contour ="
                 << result.totalDetected << "total regions";
        #endif
    }
    // If grid detection found some regions but not many
    else if (gridResult.totalDetected > 0) {
        // Use grid result only
        result = gridResult;
        result.methodUsed = "grid";
        
        #if OCR_ORC_DEBUG_ENABLED
        qDebug() << "[RegionDetector] Hybrid detection: Using grid only ("
                 << result.totalDetected << "regions)";
        #endif
    }
    // If grid detection found nothing, try contour
    else {
        DetectionResult contourResult = detectContours(image);
        result = contourResult;
        result.methodUsed = "contour";
        
        #if OCR_ORC_DEBUG_ENABLED
        qDebug() << "[RegionDetector] Hybrid detection: Grid failed, using contour ("
                 << result.totalDetected << "regions)";
        #endif
    }
    
    return result;
}

QList<DetectedRegion> RegionDetector::filterRegions(const QList<DetectedRegion>& regions) {
    QList<DetectedRegion> filtered;
    for (const auto& region : regions) {
        cv::Rect rect = region.boundingBox;
        // Filter by size
        if (rect.width < minCellWidth || rect.height < minCellHeight) continue;
        if (rect.width > maxCellWidth || rect.height > maxCellHeight) continue;
        filtered.append(region);
    }
    return filtered;
}

double RegionDetector::validateSizeConsistency(const cv::Rect& rect, const QList<DetectedRegion>& allRegions, int imgWidth, int imgHeight, double distanceThreshold) {
    if (allRegions.isEmpty()) {
        return 1.0; // No neighbors to compare, assume consistent
    }
    
    // Convert rect to normalized coords for comparison
    NormalizedCoords rectCoords;
    rectCoords.x1 = (double)rect.x / imgWidth;
    rectCoords.y1 = (double)rect.y / imgHeight;
    rectCoords.x2 = (double)(rect.x + rect.width) / imgWidth;
    rectCoords.y2 = (double)(rect.y + rect.height) / imgHeight;
    
    // Find nearby regions
    QList<DetectedRegion> neighbors;
    for (const DetectedRegion& region : allRegions) {
        // Calculate distance between centers
        double centerX1 = (rectCoords.x1 + rectCoords.x2) / 2.0;
        double centerY1 = (rectCoords.y1 + rectCoords.y2) / 2.0;
        double centerX2 = (region.coords.x1 + region.coords.x2) / 2.0;
        double centerY2 = (region.coords.y1 + region.coords.y2) / 2.0;
        
        double distance = std::sqrt(
            std::pow(centerX1 - centerX2, 2) + 
            std::pow(centerY1 - centerY2, 2)
        );
        
        if (distance <= distanceThreshold && distance > 0.001) { // Exclude self
            neighbors.append(region);
        }
    }
    
    if (neighbors.isEmpty()) {
        return 0.5; // No neighbors found, lower confidence
    }
    
    // Calculate average size of neighbors
    double avgWidth = 0.0;
    double avgHeight = 0.0;
    for (const DetectedRegion& neighbor : neighbors) {
        avgWidth += (neighbor.coords.x2 - neighbor.coords.x1);
        avgHeight += (neighbor.coords.y2 - neighbor.coords.y1);
    }
    avgWidth /= neighbors.size();
    avgHeight /= neighbors.size();
    
    // Calculate size of current region
    double rectWidth = rectCoords.x2 - rectCoords.x1;
    double rectHeight = rectCoords.y2 - rectCoords.y1;
    
    // Check deviation
    double widthDeviation = std::abs(rectWidth - avgWidth) / avgWidth;
    double heightDeviation = std::abs(rectHeight - avgHeight) / avgHeight;
    double maxDeviation = std::max(widthDeviation, heightDeviation);
    
    // If deviation > 30%, lower confidence
    if (maxDeviation > 0.3) {
        return 0.3; // Significant deviation
    } else if (maxDeviation > 0.15) {
        return 0.7; // Moderate deviation
    } else {
        return 1.0; // Good consistency
    }
}

double RegionDetector::calculateConfidence(const cv::Rect& rect, const cv::Mat& image, const QString& method) {
    if (image.empty() || rect.width <= 0 || rect.height <= 0) {
        return 0.0;
    }
    
    // Ensure rect is within image bounds
    cv::Rect validRect = rect & cv::Rect(0, 0, image.cols, image.rows);
    if (validRect.area() == 0) {
        return 0.0;
    }
    
    double lineStrength = 0.5; // Default for non-grid methods
    double rectangularity = 0.5; // Default for non-contour methods
    double sizeScore = 0.5;
    double aspectScore = 0.5;
    
    // Method-specific confidence calculations
    if (method == "grid" || method == "hybrid") {
        // Line strength: Analyze edge pixels in bounding box
        cv::Mat roi = image(validRect);
        cv::Mat edges;
        cv::Canny(roi, edges, 50, 150);
        
        // Count edge pixels along borders (should be high for grid cells)
        int borderPixels = 0;
        int totalBorderPixels = (validRect.width * 2 + validRect.height * 2);
        
        // Top and bottom borders
        for (int x = 0; x < validRect.width; ++x) {
            if (edges.at<uchar>(0, x) > 0) borderPixels++;
            if (validRect.height > 1 && edges.at<uchar>(validRect.height - 1, x) > 0) borderPixels++;
        }
        // Left and right borders
        for (int y = 0; y < validRect.height; ++y) {
            if (edges.at<uchar>(y, 0) > 0) borderPixels++;
            if (validRect.width > 1 && edges.at<uchar>(y, validRect.width - 1) > 0) borderPixels++;
        }
        
        lineStrength = totalBorderPixels > 0 ? (double)borderPixels / totalBorderPixels : 0.0;
    }
    
    if (method == "contour" || method == "hybrid") {
        // Rectangularity: For contour detection, this would be calculated from contour
        // For now, assume good rectangularity if aspect ratio is reasonable
        double aspectRatio = (double)validRect.width / validRect.height;
        if (aspectRatio >= 0.3 && aspectRatio <= 3.0) {
            rectangularity = 0.8; // Good rectangularity
        } else {
            rectangularity = 0.4; // Poor rectangularity
        }
    }
    
    // Size appropriateness score
    int expectedWidth = (minCellWidth + maxCellWidth) / 2;
    int expectedHeight = (minCellHeight + maxCellHeight) / 2;
    double widthDiff = std::abs(validRect.width - expectedWidth) / (double)expectedWidth;
    double heightDiff = std::abs(validRect.height - expectedHeight) / (double)expectedHeight;
    double avgDiff = (widthDiff + heightDiff) / 2.0;
    sizeScore = 1.0 - std::min(avgDiff, 1.0);
    
    // Aspect ratio score (prefer square-ish cells, but allow reasonable rectangles)
    double aspectRatio = (double)validRect.width / validRect.height;
    if (aspectRatio >= 0.5 && aspectRatio <= 2.0) {
        aspectScore = 1.0; // Ideal range
    } else if (aspectRatio >= 0.3 && aspectRatio <= 3.0) {
        aspectScore = 0.7; // Acceptable range
    } else {
        aspectScore = 0.3; // Poor aspect ratio
    }
    
    // Combine scores based on method
    double confidence;
    if (method == "grid") {
        confidence = lineStrength * 0.4 + sizeScore * 0.3 + aspectScore * 0.2 + rectangularity * 0.1;
    } else if (method == "contour") {
        confidence = rectangularity * 0.4 + sizeScore * 0.3 + aspectScore * 0.2 + lineStrength * 0.1;
    } else { // hybrid
        confidence = (lineStrength + rectangularity) * 0.3 + sizeScore * 0.25 + aspectScore * 0.15;
    }
    
    return std::max(0.0, std::min(1.0, confidence));
}

double RegionDetector::calculateOverlap(const NormalizedCoords& coords1, const NormalizedCoords& coords2) {
    // Calculate intersection
    double x1 = std::max(coords1.x1, coords2.x1);
    double y1 = std::max(coords1.y1, coords2.y1);
    double x2 = std::min(coords1.x2, coords2.x2);
    double y2 = std::min(coords1.y2, coords2.y2);
    
    if (x2 <= x1 || y2 <= y1) {
        return 0.0; // No intersection
    }
    
    double intersectionArea = (x2 - x1) * (y2 - y1);
    
    // Calculate union
    double area1 = (coords1.x2 - coords1.x1) * (coords1.y2 - coords1.y1);
    double area2 = (coords2.x2 - coords2.x1) * (coords2.y2 - coords2.y1);
    double unionArea = area1 + area2 - intersectionArea;
    
    if (unionArea <= 0.0) {
        return 0.0;
    }
    
    return intersectionArea / unionArea; // IoU (Intersection over Union)
}

DetectionResult RegionDetector::mergeDetectionResults(const DetectionResult& result1, const DetectionResult& result2) {
    DetectionResult merged;
    merged.methodUsed = "hybrid";
    
    // Combine region lists
    QList<DetectedRegion> allRegions = result1.regions;
    allRegions.append(result2.regions);
    
    // Remove duplicates (overlapping regions)
    QList<DetectedRegion> uniqueRegions;
    for (int i = 0; i < allRegions.size(); ++i) {
        bool isDuplicate = false;
        for (int j = i + 1; j < allRegions.size(); ++j) {
            double overlap = calculateOverlap(allRegions[i].coords, allRegions[j].coords);
            if (overlap > 0.8) { // 80% overlap threshold
                // Keep region with higher confidence
                if (allRegions[i].confidence < allRegions[j].confidence) {
                    isDuplicate = true;
                    break;
                }
            }
        }
        if (!isDuplicate) {
            uniqueRegions.append(allRegions[i]);
        }
    }
    
    merged.regions = uniqueRegions;
    merged.totalDetected = uniqueRegions.size();
    
    // Count confidence levels
    for (const auto& region : uniqueRegions) {
        if (region.confidence >= 0.8) {
            merged.highConfidence++;
        } else if (region.confidence >= 0.5) {
            merged.mediumConfidence++;
        } else {
            merged.lowConfidence++;
        }
    }
    
    return merged;
}

cv::Point2f RegionDetector::findLineIntersection(const cv::Vec4i& line1, const cv::Vec4i& line2) {
    // Line 1: (x1, y1) to (x2, y2)
    int x1 = line1[0], y1 = line1[1], x2 = line1[2], y2 = line1[3];
    // Line 2: (x3, y3) to (x4, y4)
    int x3 = line2[0], y3 = line2[1], x4 = line2[2], y4 = line2[3];
    
    // Calculate line equations: ax + by + c = 0
    int a1 = y2 - y1;
    int b1 = x1 - x2;
    int c1 = x2 * y1 - x1 * y2;
    
    int a2 = y4 - y3;
    int b2 = x3 - x4;
    int c2 = x4 * y3 - x3 * y4;
    
    // Calculate determinant
    int det = a1 * b2 - a2 * b1;
    
    if (det == 0) {
        // Lines are parallel
        return cv::Point2f(-1, -1); // Invalid point
    }
    
    // Calculate intersection point
    float x = (float)(b1 * c2 - b2 * c1) / det;
    float y = (float)(a2 * c1 - a1 * c2) / det;
    
    return cv::Point2f(x, y);
}

bool RegionDetector::isValidIntersection(const cv::Point2f& point, const cv::Size& imageSize) {
    return point.x >= 0 && point.x < imageSize.width &&
           point.y >= 0 && point.y < imageSize.height;
}

std::vector<cv::Rect> RegionDetector::formGridCells(const std::vector<cv::Point2f>& intersections) {
    // This method is not used - grid cell formation is done directly in detectGrid()
    // For now, return empty vector
    (void)intersections; // Suppress unused parameter warning
    return std::vector<cv::Rect>();
}

DetectionResult RegionDetector::detectRegionsOCRFirst(const QImage& image, const QString& method) {
    DetectionResult result;
    result.methodUsed = method;
    
    if (image.isNull()) {
        return result;
    }
    
    // Stage 1: OCR Extraction
    OcrTextExtractor extractor;
    QList<OCRTextRegion> ocrRegions = extractor.extractTextRegions(image);
    
    if (ocrRegions.isEmpty()) {
        // Fallback to CV-only if OCR fails
        return detectRegions(image, "hybrid");
    }
    
    // Convert image to cv::Mat for CV processing
    cv::Mat cvImage = ImageConverter::qImageToMat(image);
    int imgWidth = image.width();
    int imgHeight = image.height();
    
    // Stage 2: Pattern Analysis (before individual refinement)
    PatternAnalyzer patternAnalyzer;
    CheckboxDetector checkboxDetector;
    
    // Detect checkboxes for all regions
    QList<CheckboxDetection> checkboxes;
    for (const OCRTextRegion& ocrRegion : ocrRegions) {
        CheckboxDetection cb = checkboxDetector.detectCheckbox(ocrRegion, cvImage);
        checkboxes.append(cb);
    }
    
    // Analyze checkbox pattern
    QString checkboxPattern = patternAnalyzer.analyzeCheckboxPattern(ocrRegions, checkboxes);
    
    // Stage 3: Multi-Pass Refinement - Find EMPTY Form Fields Only
    // OCR text is ONLY used as coordinate hints - we never detect/highlight text itself
    TextRegionRefiner refiner;
    FormFieldDetector formFieldDetector;
    
    // Pass 1: Use OCR hints to find empty form fields nearby
    QList<cv::Rect> emptyFormFields = refiner.findEmptyFormFields(ocrRegions, cvImage);
    
    // Pass 2: Filter out any regions that contain text
    QList<cv::Rect> validatedFields;
    for (const cv::Rect& field : emptyFormFields) {
        // Strict check: region must NOT contain any OCR text
        if (!refiner.regionContainsText(field, cvImage, ocrRegions)) {
            validatedFields.append(field);
        }
    }
    
    // Pass 3: EXTREMELY DRASTIC overfitting, especially vertical
    QList<cv::Rect> overfittedFields;
    for (const cv::Rect& field : validatedFields) {
        // EXTREME overfit with asymmetric expansion:
        // - Horizontal: 40% (more room for wide fields)
        // - Vertical: 60% (MUCH more vertical room for multi-line fields)
        cv::Rect overfitted = formFieldDetector.overfitRegionAsymmetric(
            field, cvImage, 40, 60);  // 40% horizontal, 60% vertical
        overfittedFields.append(overfitted);
    }
    
    // Pass 3.5: Use smart boundary detection to find actual form field edges within overfitted regions
    QList<cv::Rect> refinedOverfitted = formFieldDetector.refineOverfittedRegions(
        overfittedFields, cvImage);
    
    // Pass 4: Detect cell groups with shared walls (grid patterns)
    QList<QList<cv::Rect>> cellGroups = formFieldDetector.detectCellGroupsWithSharedWalls(
        refinedOverfitted, cvImage);
    
    // Flatten cell groups back to individual regions (for now - can enhance later to keep groups)
    QList<cv::Rect> flattenedRegions = refinedOverfitted;
    for (const QList<cv::Rect>& group : cellGroups) {
        // Add any new cells found in groups
        for (const cv::Rect& cell : group) {
            // Check if not already in flattenedRegions
            bool found = false;
            for (const cv::Rect& existing : flattenedRegions) {
                int overlapX = std::max(0, std::min(cell.x + cell.width, existing.x + existing.width) - 
                                          std::max(cell.x, existing.x));
                int overlapY = std::max(0, std::min(cell.y + cell.height, existing.y + existing.height) - 
                                          std::max(cell.y, existing.y));
                if (overlapX > cell.width * 0.5 && overlapY > cell.height * 0.5) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                flattenedRegions.append(cell);
            }
        }
    }
    
    // Pass 5: Classify regions and filter out titles/headings
    QList<cv::Rect> classifiedFields = formFieldDetector.classifyAndRefineRegions(
        flattenedRegions, cvImage, ocrRegions);
    
    // Pass 6: SECONDARY PIPELINE - Concurrent rectangle/square detection
    // Run rectangle detection (can be made truly concurrent later with QThread if needed)
    RectangleDetector rectangleDetector;
    rectangleDetector.setSensitivity(0.15);  // MUCH higher sensitivity (lower = more sensitive)
    rectangleDetector.setMinSize(15, 10);    // Smaller minimums
    rectangleDetector.setMaxSize(800, 300);  // Larger maximums
    
    // Run rectangle detection (for now, synchronously - can make concurrent with QThread later)
    QList<DetectedRectangle> rectangleResults = rectangleDetector.detectRectangles(cvImage);
    
    // Pass 7: MATCH and MERGE regions from both pipelines (consensus-based detection)
    // Match OCR-first results with rectangle detection results
    // Pass ocrRegions for text filtering
    DetectionResult mergedResult = matchAndMergePipelines(classifiedFields, rectangleResults, image, ocrRegions);
    
    // Pass 8: Use merged consensus regions directly (already processed in matchAndMergePipelines)
    QList<DetectedRegion> refinedRegions = mergedResult.regions;
    
    // Pass 8.5: CRITICAL FINAL FILTER - Remove ANY regions that contain text
    // This is the absolute gate: if text is detected inside, it's NOT an empty form field
    // Empty form fields should be bright, have low edge density, and NO OCR text overlap
    QList<DetectedRegion> textFilteredRegions;
    for (const DetectedRegion& region : refinedRegions) {
        cv::Rect fieldRect = region.boundingBox;
        
        // STRICT check: region must NOT contain any text
        // This checks: OCR overlap, brightness, edge density, horizontal text lines
        if (!refiner.regionContainsText(fieldRect, cvImage, ocrRegions)) {
            textFilteredRegions.append(region);
        } else {
            // Region contains text - REJECT IT (not an empty form field)
            // This is the critical filter to ensure we only detect empty input cells/lines
            #if OCR_ORC_DEBUG_ENABLED
            qDebug() << "REJECTED region with text (not empty form field):" 
                     << fieldRect.x << fieldRect.y << fieldRect.width << fieldRect.height;
            #endif
        }
    }
    refinedRegions = textFilteredRegions;
    
    // If we filtered out everything, that's okay - better to have no results than wrong results
    
    // Pass 9: Enhance regions with additional classification and checkbox detection
    for (DetectedRegion& region : refinedRegions) {
        cv::Rect fieldRect = region.boundingBox;
        
        // Check if this field has a nearby checkbox
        CheckboxDetection nearbyCheckbox;
        for (int j = 0; j < ocrRegions.size(); j++) {
            cv::Rect ocrBox = ocrRegions[j].boundingBox;
            // Check if OCR hint is near this field (within 50px)
            int distanceX = std::abs((ocrBox.x + ocrBox.width/2) - (fieldRect.x + fieldRect.width/2));
            int distanceY = std::abs((ocrBox.y + ocrBox.height/2) - (fieldRect.y + fieldRect.height/2));
            if (distanceX < 50 && distanceY < 50) {
                nearbyCheckbox = checkboxes[j];
                break;
            }
        }
        
        // Classify form field type (with proper TextLine vs TextBlock distinction)
        FormFieldType fieldType = formFieldDetector.classifyRegionType(fieldRect, cvImage, ocrRegions);
        
        // Override with checkbox if detected
        if (nearbyCheckbox.detected) {
            fieldType = FormFieldType::CheckboxField;
        }
        
        // Update confidence based on consensus and field characteristics
        if (region.method == "consensus") {
            region.confidence = std::min(0.95, region.confidence + 0.1);  // Boost consensus confidence
        }
        
        if (fieldType == FormFieldType::CheckboxField) {
            region.confidence = 0.9;  // Checkboxes are high confidence
        } else if (fieldType == FormFieldType::TextInput) {
            region.confidence = std::max(region.confidence, 0.8);  // Text inputs with clear boundaries
        }
        
        // Assign type and group based on classification
        if (fieldType == FormFieldType::CheckboxField) {
            region.inferredType = "checkbox";
            region.suggestedGroup = "CheckboxGroup";
            region.suggestedColor = "blue";
        } else if (fieldType == FormFieldType::TextLine) {
            region.inferredType = "text_line";
            region.suggestedGroup = "TextLineGroup";
            region.suggestedColor = "red";
        } else if (fieldType == FormFieldType::TextBlock) {
            region.inferredType = "text_block";
            region.suggestedGroup = "TextBlockGroup";
            region.suggestedColor = "orange";
        } else if (fieldType == FormFieldType::Cell) {
            region.inferredType = "cell";
            region.suggestedGroup = "CellGroup";
            region.suggestedColor = "green";
        } else if (fieldType == FormFieldType::TextInput) {
            region.inferredType = "text_input";
            region.suggestedGroup = "TextInputGroup";
            region.suggestedColor = "red";
        } else if (region.inferredType.isEmpty()) {
            region.inferredType = "form_field";
            region.suggestedGroup = "FormFieldGroup";
            region.suggestedColor = "gray";
        }
    }
    
    // Stage 4: Group Inference
    GroupInferencer groupInferencer;
    
    // Convert OCR regions to DetectedRegions for group inference
    // (GroupInferencer expects DetectedRegion list)
    QList<DetectedGroup> groups = groupInferencer.inferGroupsFromSpatial(refinedRegions);
    
    // Also infer from patterns
    QList<DetectedGroup> patternGroups = groupInferencer.inferGroupsFromPatterns(refinedRegions);
    
    // Merge groups
    QMap<QString, DetectedGroup> combinedGroups;
    for (const DetectedGroup& group : groups) {
        combinedGroups[group.name] = group;
    }
    for (const DetectedGroup& group : patternGroups) {
        if (combinedGroups.contains(group.name)) {
            // Merge region names
            DetectedGroup& existing = combinedGroups[group.name];
            for (const QString& name : group.regionNames) {
                if (!existing.regionNames.contains(name)) {
                    existing.regionNames.append(name);
                }
            }
        } else {
            combinedGroups[group.name] = group;
        }
    }
    
    // Convert to list
    for (auto it = combinedGroups.begin(); it != combinedGroups.end(); ++it) {
        result.inferredGroups.append(it.value());
    }
    
    // Build result
    result.regions = refinedRegions;
    result.totalDetected = refinedRegions.size();
    
    // Count confidence levels and populate maps
    for (int i = 0; i < refinedRegions.size(); i++) {
        const DetectedRegion& region = refinedRegions[i];
        
        if (region.confidence >= 0.8) {
            result.highConfidence++;
        } else if (region.confidence >= 0.5) {
            result.mediumConfidence++;
        } else {
            result.lowConfidence++;
        }
        
        // Populate region types and colors maps
        QString regionName = QString("Region_%1").arg(i + 1);
        result.regionTypes[regionName] = region.inferredType;
        if (!region.suggestedGroup.isEmpty()) {
            result.suggestedColors[region.suggestedGroup] = region.suggestedColor;
        }
    }
    
    return result;
}

DetectionResult RegionDetector::matchAndMergePipelines(const QList<cv::Rect>& ocrRegions,
                                                       const QList<DetectedRectangle>& rectangleRegions,
                                                       const QImage& image,
                                                       const QList<OCRTextRegion>& ocrTextRegions)
{
    DetectionResult result;
    result.methodUsed = "ocr-first+rectangle-consensus";
    
    if (image.isNull()) {
        return result;
    }
    
    // Convert image to cv::Mat for text checking
    cv::Mat cvImage = ImageConverter::qImageToMat(image);
    
    // Create TextRegionRefiner for text filtering
    TextRegionRefiner refiner;
    
    QList<DetectedRegion> matchedRegions;
    
    // Match OCR-first regions with rectangle detection regions
    // Strategy: Only keep regions detected by BOTH pipelines (consensus)
    // OR regions with very high confidence from a single pipeline
    
    for (const cv::Rect& ocrRect : ocrRegions) {
        bool foundMatch = false;
        double bestMatchScore = 0.0;
        cv::Rect bestMatchedRect = ocrRect;
        
        // Check if this OCR region matches any rectangle detection
        for (const DetectedRectangle& rectDet : rectangleRegions) {
            const cv::Rect& cvRect = rectDet.boundingBox;
            
            // Calculate overlap
            int overlapX = std::max(0, std::min(ocrRect.x + ocrRect.width, cvRect.x + cvRect.width) - 
                                      std::max(ocrRect.x, cvRect.x));
            int overlapY = std::max(0, std::min(ocrRect.y + ocrRect.height, cvRect.y + cvRect.height) - 
                                      std::max(ocrRect.y, cvRect.y));
            int overlapArea = overlapX * overlapY;
            
            int ocrArea = ocrRect.width * ocrRect.height;
            int cvArea = cvRect.width * cvRect.height;
            
            // Calculate overlap percentage (IoU - Intersection over Union)
            int unionArea = ocrArea + cvArea - overlapArea;
            double iou = unionArea > 0 ? static_cast<double>(overlapArea) / unionArea : 0.0;
            
            // Also calculate individual overlaps
            double ocrOverlap = ocrArea > 0 ? static_cast<double>(overlapArea) / ocrArea : 0.0;
            double cvOverlap = cvArea > 0 ? static_cast<double>(overlapArea) / cvArea : 0.0;
            
            // MUCH more lenient matching: IoU > 0.3 OR either overlap > 0.4
            // This allows partial matches and catches more regions
            if (iou > 0.3 || ocrOverlap > 0.4 || cvOverlap > 0.4) {
                foundMatch = true;
                
                // Calculate match score (combine IoU and rectangle confidence)
                double matchScore = iou * 0.6 + rectDet.confidence * 0.4;
                
                if (matchScore > bestMatchScore) {
                    bestMatchScore = matchScore;
                    // Use the intersection (more precise) or prefer rectangle if it has high confidence
                    if (rectDet.confidence > 0.8) {
                        bestMatchedRect = cvRect;  // Use rectangle detection if very confident
                    } else {
                        // Use intersection of both
                        int x1 = std::max(ocrRect.x, cvRect.x);
                        int y1 = std::max(ocrRect.y, cvRect.y);
                        int x2 = std::min(ocrRect.x + ocrRect.width, cvRect.x + cvRect.width);
                        int y2 = std::min(ocrRect.y + ocrRect.height, cvRect.y + cvRect.height);
                        bestMatchedRect = cv::Rect(x1, y1, x2 - x1, y2 - y1);
                    }
                }
            }
        }
        
        // Create DetectedRegion from matched OR any OCR region (fallback to OCR-first)
        // Always include OCR regions, but boost confidence if matched
        if (foundMatch || true) {  // Always include OCR regions as fallback
            // Convert to normalized coordinates
            ImageCoords imgCoords(bestMatchedRect.x, bestMatchedRect.y, 
                                 bestMatchedRect.x + bestMatchedRect.width, 
                                 bestMatchedRect.y + bestMatchedRect.height);
            NormalizedCoords normCoords = CoordinateSystem::imageToNormalized(
                imgCoords, image.width(), image.height());
            
            // CRITICAL: Check if region contains text BEFORE adding it
            if (refiner.regionContainsText(bestMatchedRect, cvImage, ocrTextRegions)) {
                // Region contains text - skip it (not an empty form field)
                continue;
            }
            
            DetectedRegion region;
            region.coords = normCoords;
            region.boundingBox = bestMatchedRect;
            region.method = foundMatch ? "consensus" : "ocr-first";
            // Confidence: consensus gets boost, OCR-only gets base confidence
            region.confidence = foundMatch ? 0.85 : 0.70;  // Higher confidence for consensus, but still good for OCR-only
            region.inferredType = "form_field";
            region.suggestedGroup = "FormFieldGroup";
            region.suggestedColor = "green";
            
            matchedRegions.append(region);
        }
    }
    
    // Also check for rectangle detections that weren't matched (lower threshold)
    // (might be form fields OCR missed - be more inclusive)
    for (const DetectedRectangle& rectDet : rectangleRegions) {
        if (rectDet.confidence < 0.5) continue;  // Lowered from 0.7 - include more rectangles
        
        bool alreadyMatched = false;
        for (const DetectedRegion& existing : matchedRegions) {
            cv::Rect existingRect = existing.boundingBox;
            const cv::Rect& cvRect = rectDet.boundingBox;
            
            // Check overlap
            int overlapX = std::max(0, std::min(existingRect.x + existingRect.width, cvRect.x + cvRect.width) - 
                                      std::max(existingRect.x, cvRect.x));
            int overlapY = std::max(0, std::min(existingRect.y + existingRect.height, cvRect.y + cvRect.height) - 
                                      std::max(existingRect.y, cvRect.y));
            int overlapArea = overlapX * overlapY;
            int existingArea = existingRect.width * existingRect.height;
            
            if (overlapArea > existingArea * 0.5) {
                alreadyMatched = true;
                break;
            }
        }
        
        if (!alreadyMatched) {
            // CRITICAL: Check if rectangle region contains text BEFORE adding it
            if (refiner.regionContainsText(rectDet.boundingBox, cvImage, ocrTextRegions)) {
                // Region contains text - skip it (not an empty form field)
                continue;
            }
            
            // High-confidence rectangle not matched - add it
            ImageCoords imgCoords(rectDet.boundingBox.x, rectDet.boundingBox.y, 
                                 rectDet.boundingBox.x + rectDet.boundingBox.width, 
                                 rectDet.boundingBox.y + rectDet.boundingBox.height);
            NormalizedCoords normCoords = CoordinateSystem::imageToNormalized(
                imgCoords, image.width(), image.height());
            
            DetectedRegion region;
            region.coords = normCoords;
            region.boundingBox = rectDet.boundingBox;
            region.method = "rectangle-detection";
            region.confidence = rectDet.confidence;
            region.inferredType = rectDet.isSquare ? "square" : "form_field";
            region.suggestedGroup = "FormFieldGroup";
            region.suggestedColor = "blue";
            
            matchedRegions.append(region);
        }
    }
    
    // Build result
    result.regions = matchedRegions;
    result.totalDetected = matchedRegions.size();
    
    // Count confidence levels and populate maps
    for (int i = 0; i < matchedRegions.size(); i++) {
        const DetectedRegion& region = matchedRegions[i];
        
        if (region.confidence >= 0.8) {
            result.highConfidence++;
        } else if (region.confidence >= 0.5) {
            result.mediumConfidence++;
        } else {
            result.lowConfidence++;
        }
        
        // Populate region types and colors maps
        QString regionName = QString("Region_%1").arg(i + 1);
        result.regionTypes[regionName] = region.inferredType;
        if (!region.suggestedGroup.isEmpty()) {
            result.suggestedColors[region.suggestedGroup] = region.suggestedColor;
        }
    }
    
    return result;
}

} // namespace ocr_orc
