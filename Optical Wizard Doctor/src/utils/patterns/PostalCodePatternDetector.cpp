#include "PostalCodePatternDetector.h"
#include "../RegionDetector.h"
#include <algorithm>
#include <cmath>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

PostalCodePatternDetector::PostalCodePatternDetector() {
}

double PostalCodePatternDetector::calculateDistance(const DetectedRegion& r1, const DetectedRegion& r2) {
    double centerX1 = (r1.coords.x1 + r1.coords.x2) / 2.0;
    double centerY1 = (r1.coords.y1 + r1.coords.y2) / 2.0;
    double centerX2 = (r2.coords.x1 + r2.coords.x2) / 2.0;
    double centerY2 = (r2.coords.y1 + r2.coords.y2) / 2.0;
    
    return std::sqrt(
        std::pow(centerX1 - centerX2, 2) + 
        std::pow(centerY1 - centerY2, 2)
    );
}

bool PostalCodePatternDetector::areHorizontallyAligned(const QList<DetectedRegion>& regions, double tolerance) {
    if (regions.size() < 2) {
        return true;
    }
    
    // Check if all regions have similar Y-coordinates
    double firstY = (regions[0].coords.y1 + regions[0].coords.y2) / 2.0;
    for (int i = 1; i < regions.size(); ++i) {
        double y = (regions[i].coords.y1 + regions[i].coords.y2) / 2.0;
        if (std::abs(y - firstY) > tolerance) {
            return false;
        }
    }
    
    return true;
}

QList<DetectedRegion> PostalCodePatternDetector::findCandidateRegions(const QList<DetectedRegion>& regions) {
    if (regions.size() < 6) {
        return QList<DetectedRegion>();
    }
    
    // Sort regions by X-coordinate
    QList<DetectedRegion> sorted = regions;
    std::sort(sorted.begin(), sorted.end(), 
              [](const DetectedRegion& a, const DetectedRegion& b) {
                  double centerXA = (a.coords.x1 + a.coords.x2) / 2.0;
                  double centerXB = (b.coords.x1 + b.coords.x2) / 2.0;
                  return centerXA < centerXB;
              });
    
    // Find groups of 6 regions in close proximity
    QList<DetectedRegion> candidates;
    double maxGap = 0.05; // 5% of image width max gap between cells
    
    for (int i = 0; i <= sorted.size() - 6; ++i) {
        QList<DetectedRegion> group;
        group.append(sorted[i]);
        
        for (int j = i + 1; j < sorted.size() && group.size() < 6; ++j) {
            double gap = calculateDistance(group.last(), sorted[j]);
            if (gap <= maxGap) {
                group.append(sorted[j]);
            } else {
                break; // Gap too large
            }
        }
        
        if (group.size() == 6) {
            // Check horizontal alignment
            if (areHorizontallyAligned(group, 0.02)) {
                candidates = group;
                break; // Found a good candidate
            }
        }
    }
    
    return candidates;
}

int PostalCodePatternDetector::detectSeparator(const QList<DetectedRegion>& regions) {
    if (regions.size() < 6) {
        return -1;
    }
    
    // Look for larger gap between regions (likely separator)
    // Typically between index 2 and 3 (after first 3 cells)
    double maxGap = 0.0;
    int separatorIndex = -1;
    
    for (int i = 2; i < 5 && i < regions.size() - 1; ++i) {
        double gap = calculateDistance(regions[i], regions[i + 1]);
        if (gap > maxGap) {
            maxGap = gap;
            separatorIndex = i;
        }
    }
    
    // Separator should have gap > 2x average gap
    if (separatorIndex >= 0) {
        double avgGap = 0.0;
        int gapCount = 0;
        for (int i = 0; i < regions.size() - 1; ++i) {
            if (i != separatorIndex) {
                avgGap += calculateDistance(regions[i], regions[i + 1]);
                gapCount++;
            }
        }
        if (gapCount > 0) {
            avgGap /= gapCount;
            if (maxGap > 2.0 * avgGap) {
                return separatorIndex;
            }
        }
    }
    
    return -1; // No clear separator found
}

bool PostalCodePatternDetector::validateAlternatingPattern(
    const QList<DetectedRegion>& regions, 
    int separatorIndex) {
    
    if (regions.size() != 6) {
        return false;
    }
    
    // Expected pattern: [L][N][L]-[N][L][N]
    // Indices: 0=L, 1=N, 2=L, 3=N, 4=L, 5=N
    
    // For now, we'll validate based on inferred types if available
    // If types are not yet inferred, we'll accept the pattern structure
    // and let type inference happen later
    
    // Check that separator is at index 2 (between first 3 and last 3)
    if (separatorIndex >= 0 && separatorIndex != 2) {
        // Separator should be between groups, typically at index 2
        // But allow some flexibility
        if (separatorIndex < 2 || separatorIndex > 3) {
            return false;
        }
    }
    
    return true;
}

PostalCodePattern PostalCodePatternDetector::detectPattern(const QList<DetectedRegion>& regions) {
    PostalCodePattern pattern;
    
    if (regions.size() < 6) {
        return pattern; // Need at least 6 regions
    }
    
    // Find candidate regions (6 in close proximity, horizontally aligned)
    QList<DetectedRegion> candidates = findCandidateRegions(regions);
    
    if (candidates.size() != 6) {
        return pattern; // No valid candidate found
    }
    
    // Detect separator
    int separatorIndex = detectSeparator(candidates);
    
    // Validate pattern structure
    if (!validateAlternatingPattern(candidates, separatorIndex)) {
        return pattern; // Pattern doesn't match
    }
    
    // Build pattern structure
    pattern.group1 = candidates.mid(0, 3); // First 3 cells
    pattern.group2 = candidates.mid(3, 3); // Last 3 cells
    pattern.separatorIndex = separatorIndex;
    
    // Calculate confidence
    // Higher confidence if:
    // - Regions are well-aligned
    // - Separator is clearly detected
    // - Sizes are consistent
    double alignmentScore = areHorizontallyAligned(candidates, 0.02) ? 1.0 : 0.7;
    double separatorScore = separatorIndex >= 0 ? 1.0 : 0.8; // Slightly lower if no separator
    double sizeConsistency = 1.0;
    
    // Check size consistency
    if (candidates.size() >= 2) {
        double avgWidth = 0.0;
        double avgHeight = 0.0;
        for (const auto& region : candidates) {
            avgWidth += (region.coords.x2 - region.coords.x1);
            avgHeight += (region.coords.y2 - region.coords.y1);
        }
        avgWidth /= candidates.size();
        avgHeight /= candidates.size();
        
        // Check deviation
        int consistentCount = 0;
        for (const auto& region : candidates) {
            double width = region.coords.x2 - region.coords.x1;
            double height = region.coords.y2 - region.coords.y1;
            if (std::abs(width - avgWidth) / avgWidth < 0.3 &&
                std::abs(height - avgHeight) / avgHeight < 0.3) {
                consistentCount++;
            }
        }
        sizeConsistency = (double)consistentCount / candidates.size();
    }
    
    pattern.confidence = (alignmentScore + separatorScore + sizeConsistency) / 3.0;
    pattern.patternString = "A#A-#A#";
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[PostalCodePatternDetector] Pattern detected: confidence" 
             << pattern.confidence << "separator at index" << separatorIndex;
    #endif
    
    return pattern;
}

bool PostalCodePatternDetector::validatePattern(const PostalCodePattern& pattern) {
    if (pattern.group1.size() != 3 || pattern.group2.size() != 3) {
        return false;
    }
    
    if (pattern.confidence < 0.5) {
        return false; // Low confidence
    }
    
    return true;
}

QList<DetectedRegion> PostalCodePatternDetector::groupPostalCode(const QList<DetectedRegion>& regions) {
    PostalCodePattern pattern = detectPattern(regions);
    
    if (!validatePattern(pattern)) {
        return QList<DetectedRegion>(); // Invalid pattern
    }
    
    // Combine groups into single list
    QList<DetectedRegion> postalCodeRegions;
    postalCodeRegions.append(pattern.group1);
    postalCodeRegions.append(pattern.group2);
    
    // Assign types based on pattern: [L][N][L]-[N][L][N]
    for (int i = 0; i < postalCodeRegions.size(); ++i) {
        if (i == 0 || i == 2 || i == 4) {
            postalCodeRegions[i].inferredType = "letters";
        } else {
            postalCodeRegions[i].inferredType = "numbers";
        }
        postalCodeRegions[i].suggestedGroup = "Postalcode";
        postalCodeRegions[i].suggestedColor = "yellow";
    }
    
    return postalCodeRegions;
}

} // namespace ocr_orc
