#include "FormStructureAnalyzer.h"
#include "TextRegionRefiner.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace ocr_orc {

FormStructureAnalyzer::FormStructureAnalyzer()
{
}

QList<FormFieldGroup> FormStructureAnalyzer::detectFormStructure(
    const QList<DetectedRegion>& regions,
    const QList<OCRTextRegion>& ocrRegions)
{
    QList<FormFieldGroup> groups;
    
    if (regions.isEmpty()) {
        return groups;
    }
    
    // Detect different types of field groups
    QList<FormFieldGroup> nameGroups = detectNameFieldGroups(regions, ocrRegions);
    QList<FormFieldGroup> addressGroups = detectAddressFieldGroups(regions, ocrRegions);
    QList<FormFieldGroup> dateGroups = detectDateFieldGroups(regions, ocrRegions);
    
    groups.append(nameGroups);
    groups.append(addressGroups);
    groups.append(dateGroups);
    
    return groups;
}

QList<FormFieldGroup> FormStructureAnalyzer::detectNameFieldGroups(
    const QList<DetectedRegion>& regions,
    const QList<OCRTextRegion>& ocrRegions)
{
    QList<FormFieldGroup> groups;
    
    // Find OCR regions that are name-related labels
    TextRegionRefiner refiner;
    QList<cv::Rect> nameFields;
    
    for (const OCRTextRegion& ocr : ocrRegions) {
        TextRegionRefiner::SemanticLabelInfo info = refiner.inferFieldTypeFromLabel(ocr.text);
        if (info.fieldType == "text_input" && 
            (ocr.text.toLower().contains("name") || 
             ocr.text.toLower().contains("first") || 
             ocr.text.toLower().contains("last"))) {
            // Find nearby detected regions
            for (const DetectedRegion& region : regions) {
                cv::Rect fieldRect = region.boundingBox;
                cv::Rect labelRect = ocr.boundingBox;
                
                // Check if field is near label (below or right)
                bool isNearby = false;
                if (fieldRect.y > labelRect.y && 
                    fieldRect.y < labelRect.y + labelRect.height + 50 &&
                    std::abs(fieldRect.x - labelRect.x) < 100) {
                    isNearby = true;
                } else if (fieldRect.x > labelRect.x + labelRect.width &&
                           fieldRect.x < labelRect.x + labelRect.width + 200 &&
                           std::abs(fieldRect.y - labelRect.y) < 20) {
                    isNearby = true;
                }
                
                if (isNearby) {
                    nameFields.append(fieldRect);
                }
            }
        }
    }
    
    if (nameFields.size() >= 2) {
        FormFieldGroup group;
        group.groupType = "name";
        group.fields = nameFields;
        group.groupBoundary = calculateGroupBoundary(nameFields);
        group.layout = determineLayout(nameFields);
        group.confidence = 0.85;
        groups.append(group);
    }
    
    return groups;
}

QList<FormFieldGroup> FormStructureAnalyzer::detectAddressFieldGroups(
    const QList<DetectedRegion>& regions,
    const QList<OCRTextRegion>& ocrRegions)
{
    QList<FormFieldGroup> groups;
    
    TextRegionRefiner refiner;
    QList<cv::Rect> addressFields;
    
    for (const OCRTextRegion& ocr : ocrRegions) {
        TextRegionRefiner::SemanticLabelInfo info = refiner.inferFieldTypeFromLabel(ocr.text);
        if (info.fieldType == "text_input" && 
            (ocr.text.toLower().contains("address") || 
             ocr.text.toLower().contains("street") || 
             ocr.text.toLower().contains("city") ||
             ocr.text.toLower().contains("state") ||
             ocr.text.toLower().contains("zip") ||
             ocr.text.toLower().contains("postal"))) {
            // Find nearby detected regions
            for (const DetectedRegion& region : regions) {
                cv::Rect fieldRect = region.boundingBox;
                cv::Rect labelRect = ocr.boundingBox;
                
                // Check if field is near label
                bool isNearby = false;
                if (fieldRect.y > labelRect.y && 
                    fieldRect.y < labelRect.y + labelRect.height + 50 &&
                    std::abs(fieldRect.x - labelRect.x) < 100) {
                    isNearby = true;
                }
                
                if (isNearby) {
                    addressFields.append(fieldRect);
                }
            }
        }
    }
    
    if (addressFields.size() >= 2) {
        FormFieldGroup group;
        group.groupType = "address";
        group.fields = addressFields;
        group.groupBoundary = calculateGroupBoundary(addressFields);
        group.layout = determineLayout(addressFields);
        group.confidence = 0.80;
        groups.append(group);
    }
    
    return groups;
}

QList<FormFieldGroup> FormStructureAnalyzer::detectDateFieldGroups(
    const QList<DetectedRegion>& regions,
    const QList<OCRTextRegion>& ocrRegions)
{
    QList<FormFieldGroup> groups;
    
    TextRegionRefiner refiner;
    QList<cv::Rect> dateFields;
    
    for (const OCRTextRegion& ocr : ocrRegions) {
        TextRegionRefiner::SemanticLabelInfo info = refiner.inferFieldTypeFromLabel(ocr.text);
        if (info.fieldType == "date") {
            // Find nearby detected regions
            for (const DetectedRegion& region : regions) {
                cv::Rect fieldRect = region.boundingBox;
                cv::Rect labelRect = ocr.boundingBox;
                
                // Check if field is near label
                bool isNearby = false;
                if (fieldRect.y > labelRect.y && 
                    fieldRect.y < labelRect.y + labelRect.height + 50 &&
                    std::abs(fieldRect.x - labelRect.x) < 100) {
                    isNearby = true;
                }
                
                if (isNearby) {
                    dateFields.append(fieldRect);
                }
            }
        }
    }
    
    if (dateFields.size() >= 1) {
        FormFieldGroup group;
        group.groupType = "date";
        group.fields = dateFields;
        group.groupBoundary = calculateGroupBoundary(dateFields);
        group.layout = determineLayout(dateFields);
        group.confidence = 0.75;
        groups.append(group);
    }
    
    return groups;
}

QString FormStructureAnalyzer::determineLayout(const QList<cv::Rect>& fields)
{
    if (fields.size() < 2) {
        return "vertical";  // Default
    }
    
    if (areHorizontallyAligned(fields)) {
        return "horizontal";
    } else if (areVerticallyAligned(fields)) {
        return "vertical";
    } else {
        return "grid";
    }
}

cv::Rect FormStructureAnalyzer::calculateGroupBoundary(const QList<cv::Rect>& fields)
{
    if (fields.isEmpty()) {
        return cv::Rect();
    }
    
    int minX = fields[0].x;
    int minY = fields[0].y;
    int maxX = fields[0].x + fields[0].width;
    int maxY = fields[0].y + fields[0].height;
    
    for (const cv::Rect& field : fields) {
        minX = std::min(minX, field.x);
        minY = std::min(minY, field.y);
        maxX = std::max(maxX, field.x + field.width);
        maxY = std::max(maxY, field.y + field.height);
    }
    
    return cv::Rect(minX, minY, maxX - minX, maxY - minY);
}

bool FormStructureAnalyzer::areHorizontallyAligned(const QList<cv::Rect>& fields)
{
    if (fields.size() < 2) {
        return false;
    }
    
    // Check if Y-coordinates are similar (within 20px)
    int firstY = fields[0].y;
    for (const cv::Rect& field : fields) {
        if (std::abs(field.y - firstY) > 20) {
            return false;
        }
    }
    
    return true;
}

bool FormStructureAnalyzer::areVerticallyAligned(const QList<cv::Rect>& fields)
{
    if (fields.size() < 2) {
        return false;
    }
    
    // Check if X-coordinates are similar (within 20px)
    int firstX = fields[0].x;
    for (const cv::Rect& field : fields) {
        if (std::abs(field.x - firstX) > 20) {
            return false;
        }
    }
    
    return true;
}

FormFieldGroup FormStructureAnalyzer::groupRelatedFields(const QList<DetectedRegion>& regions)
{
    FormFieldGroup group;
    
    if (regions.isEmpty()) {
        return group;
    }
    
    // Simple grouping: all regions in a cluster
    for (const DetectedRegion& region : regions) {
        group.fields.append(region.boundingBox);
    }
    
    group.groupBoundary = calculateGroupBoundary(group.fields);
    group.layout = determineLayout(group.fields);
    group.confidence = 0.70;
    
    return group;
}

QString FormStructureAnalyzer::inferFieldTypeFromContext(const cv::Rect& field,
                                                          const QList<DetectedRegion>& nearbyFields,
                                                          const QList<OCRTextRegion>& ocrRegions)
{
    // Find nearest OCR region
    double minDistance = std::numeric_limits<double>::max();
    QString nearestLabel;
    
    for (const OCRTextRegion& ocr : ocrRegions) {
        cv::Rect labelRect = ocr.boundingBox;
        
        // Calculate distance
        int centerX = field.x + field.width / 2;
        int centerY = field.y + field.height / 2;
        int labelCenterX = labelRect.x + labelRect.width / 2;
        int labelCenterY = labelRect.y + labelRect.height / 2;
        
        double distance = std::sqrt(std::pow(centerX - labelCenterX, 2) + 
                                    std::pow(centerY - labelCenterY, 2));
        
        if (distance < minDistance && distance < 200) {  // Within 200px
            minDistance = distance;
            nearestLabel = ocr.text;
        }
    }
    
    if (!nearestLabel.isEmpty()) {
        TextRegionRefiner refiner;
        TextRegionRefiner::SemanticLabelInfo info = refiner.inferFieldTypeFromLabel(nearestLabel);
        return info.fieldType;
    }
    
    return "unknown";
}

} // namespace ocr_orc
