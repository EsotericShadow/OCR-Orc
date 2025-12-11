#include "NameFieldPatternDetector.h"
#include "../RegionDetector.h"
#include <algorithm>
#include <cmath>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

NameFieldPatternDetector::NameFieldPatternDetector() {
}

bool NameFieldPatternDetector::isHorizontalSequence(const QList<DetectedRegion>& regions, double maxGap) {
    if (regions.size() < 2) {
        return false;
    }
    
    // Sort by X-coordinate
    QList<DetectedRegion> sorted = regions;
    std::sort(sorted.begin(), sorted.end(), 
              [](const DetectedRegion& a, const DetectedRegion& b) {
                  double centerXA = (a.coords.x1 + a.coords.x2) / 2.0;
                  double centerXB = (b.coords.x1 + b.coords.x2) / 2.0;
                  return centerXA < centerXB;
              });
    
    // Check gaps between consecutive regions
    for (int i = 0; i < sorted.size() - 1; ++i) {
        double centerX1 = (sorted[i].coords.x1 + sorted[i].coords.x2) / 2.0;
        double centerX2 = (sorted[i+1].coords.x1 + sorted[i+1].coords.x2) / 2.0;
        double width1 = sorted[i].coords.x2 - sorted[i].coords.x1;
        double width2 = sorted[i+1].coords.x2 - sorted[i+1].coords.x1;
        
        double gap = centerX2 - centerX1 - width1/2.0 - width2/2.0;
        
        if (gap > maxGap) {
            return false; // Gap too large
        }
    }
    
    return true;
}

bool NameFieldPatternDetector::areSizesConsistent(const QList<DetectedRegion>& regions, double tolerance) {
    if (regions.size() < 2) {
        return true;
    }
    
    // Calculate average size
    double avgWidth = 0.0;
    double avgHeight = 0.0;
    for (const auto& region : regions) {
        avgWidth += (region.coords.x2 - region.coords.x1);
        avgHeight += (region.coords.y2 - region.coords.y1);
    }
    avgWidth /= regions.size();
    avgHeight /= regions.size();
    
    // Check if all regions are within tolerance
    for (const auto& region : regions) {
        double width = region.coords.x2 - region.coords.x1;
        double height = region.coords.y2 - region.coords.y1;
        
        double widthDev = std::abs(width - avgWidth) / avgWidth;
        double heightDev = std::abs(height - avgHeight) / avgHeight;
        
        if (widthDev > tolerance || heightDev > tolerance) {
            return false;
        }
    }
    
    return true;
}

bool NameFieldPatternDetector::areAligned(const QList<DetectedRegion>& regions, double tolerance) {
    if (regions.size() < 2) {
        return true;
    }
    
    // Check Y-coordinate alignment
    double firstY = (regions[0].coords.y1 + regions[0].coords.y2) / 2.0;
    for (int i = 1; i < regions.size(); ++i) {
        double y = (regions[i].coords.y1 + regions[i].coords.y2) / 2.0;
        if (std::abs(y - firstY) > tolerance) {
            return false;
        }
    }
    
    return true;
}

QString NameFieldPatternDetector::suggestGroupName(int fieldIndex) {
    // Alternate between Firstname and Lastname
    // Typically first detected field is Lastname (appears higher on form)
    // Second is Firstname
    if (fieldIndex % 2 == 0) {
        return "Lastname";
    } else {
        return "Firstname";
    }
}

QString NameFieldPatternDetector::suggestColor(int fieldIndex) {
    // Alternate colors: blue for Lastname, red for Firstname
    if (fieldIndex % 2 == 0) {
        return "blue";
    } else {
        return "red";
    }
}

bool NameFieldPatternDetector::validateNameField(const QList<DetectedRegion>& fieldRegions) {
    if (fieldRegions.size() < 3) {
        return false; // Name fields typically have multiple cells
    }
    
    // Check horizontal sequence
    if (!isHorizontalSequence(fieldRegions, 0.02)) {
        return false;
    }
    
    // Check size consistency
    if (!areSizesConsistent(fieldRegions, 0.3)) {
        return false;
    }
    
    // Check alignment
    if (!areAligned(fieldRegions, 0.01)) {
        return false;
    }
    
    return true;
}

QList<QList<DetectedRegion>> NameFieldPatternDetector::detectNameFields(const QList<DetectedRegion>& regions) {
    QList<QList<DetectedRegion>> nameFields;
    
    if (regions.size() < 3) {
        return nameFields; // Need at least 3 regions for a name field
    }
    
    // Group by horizontal alignment (rows)
    // Sort by Y-coordinate
    QList<DetectedRegion> sorted = regions;
    std::sort(sorted.begin(), sorted.end(), 
              [](const DetectedRegion& a, const DetectedRegion& b) {
                  double centerYA = (a.coords.y1 + a.coords.y2) / 2.0;
                  double centerYB = (b.coords.y1 + b.coords.y2) / 2.0;
                  return centerYA < centerYB;
              });
    
    // Cluster by Y-coordinate (rows)
    QList<QList<DetectedRegion>> rows;
    double rowTolerance = 0.01; // 1% of image height
    
    for (const DetectedRegion& region : sorted) {
        bool addedToRow = false;
        for (auto& row : rows) {
            if (!row.isEmpty()) {
                double rowY = (row[0].coords.y1 + row[0].coords.y2) / 2.0;
                double regionY = (region.coords.y1 + region.coords.y2) / 2.0;
                if (std::abs(rowY - regionY) < rowTolerance) {
                    row.append(region);
                    addedToRow = true;
                    break;
                }
            }
        }
        if (!addedToRow) {
            rows.append({region});
        }
    }
    
    // Sort each row by X-coordinate
    for (auto& row : rows) {
        std::sort(row.begin(), row.end(), 
                  [](const DetectedRegion& a, const DetectedRegion& b) {
                      double centerXA = (a.coords.x1 + a.coords.x2) / 2.0;
                      double centerXB = (b.coords.x1 + b.coords.x2) / 2.0;
                      return centerXA < centerXB;
                  });
    }
    
    // Validate each row as potential name field
    int fieldIndex = 0;
    for (const auto& row : rows) {
        if (validateNameField(row)) {
            // Assign group name and color
            QList<DetectedRegion> nameField = row;
            QString groupName = suggestGroupName(fieldIndex);
            QString color = suggestColor(fieldIndex);
            
            for (DetectedRegion& region : nameField) {
                region.inferredType = "letters";
                region.suggestedGroup = groupName;
                region.suggestedColor = color;
            }
            
            nameFields.append(nameField);
            fieldIndex++;
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[NameFieldPatternDetector] Detected" << nameFields.size() << "name fields";
    #endif
    
    return nameFields;
}

} // namespace ocr_orc
