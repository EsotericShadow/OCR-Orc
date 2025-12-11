#include "RegionValidator.h"
#include "RegionDetector.h"
#include "SpatialClusterer.h"
#include <cmath>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

RegionValidator::RegionValidator() 
    : spatialWeight(1.0), sizeWeight(1.0), typeWeight(1.0), 
      patternWeight(1.0), alignmentWeight(1.0) {
}

void RegionValidator::setWeights(double spatialWeight, double sizeWeight, double typeWeight,
                                 double patternWeight, double alignmentWeight) {
    this->spatialWeight = spatialWeight;
    this->sizeWeight = sizeWeight;
    this->typeWeight = typeWeight;
    this->patternWeight = patternWeight;
    this->alignmentWeight = alignmentWeight;
}

QList<DetectedRegion> RegionValidator::findNeighbors(const DetectedRegion& region,
                                                      const QList<DetectedRegion>& allRegions,
                                                      double distanceThreshold) {
    QList<DetectedRegion> neighbors;
    
    double centerX = (region.coords.x1 + region.coords.x2) / 2.0;
    double centerY = (region.coords.y1 + region.coords.y2) / 2.0;
    
    for (const DetectedRegion& other : allRegions) {
        // Skip self
        if (&other == &region) {
            continue;
        }
        
        double otherCenterX = (other.coords.x1 + other.coords.x2) / 2.0;
        double otherCenterY = (other.coords.y1 + other.coords.y2) / 2.0;
        
        double distance = std::sqrt(
            std::pow(centerX - otherCenterX, 2) + 
            std::pow(centerY - otherCenterY, 2)
        );
        
        if (distance <= distanceThreshold) {
            neighbors.append(other);
        }
    }
    
    return neighbors;
}

double RegionValidator::validateSpatial(const DetectedRegion& region, 
                                         const QList<DetectedRegion>& allRegions) {
    QList<DetectedRegion> neighbors = findNeighbors(region, allRegions, 0.1);
    
    if (neighbors.isEmpty()) {
        return 0.5; // Penalty: isolated region
    }
    
    return 1.0; // Has neighbors, good
}

double RegionValidator::validateSize(const DetectedRegion& region, 
                                      const QList<DetectedRegion>& allRegions) {
    QList<DetectedRegion> neighbors = findNeighbors(region, allRegions, 0.1);
    
    if (neighbors.isEmpty()) {
        return 0.7; // No neighbors to compare, moderate confidence
    }
    
    // Calculate average size of neighbors
    double avgWidth = 0.0;
    double avgHeight = 0.0;
    for (const auto& neighbor : neighbors) {
        avgWidth += (neighbor.coords.x2 - neighbor.coords.x1);
        avgHeight += (neighbor.coords.y2 - neighbor.coords.y1);
    }
    avgWidth /= neighbors.size();
    avgHeight /= neighbors.size();
    
    // Calculate size of current region
    double regionWidth = region.coords.x2 - region.coords.x1;
    double regionHeight = region.coords.y2 - region.coords.y1;
    
    // Check deviation
    double widthDev = std::abs(regionWidth - avgWidth) / avgWidth;
    double heightDev = std::abs(regionHeight - avgHeight) / avgHeight;
    double maxDev = std::max(widthDev, heightDev);
    
    // Penalty based on deviation
    if (maxDev > 0.3) {
        return 0.3; // Significant deviation
    } else if (maxDev > 0.15) {
        return 0.7; // Moderate deviation
    } else {
        return 1.0; // Good consistency
    }
}

double RegionValidator::validateType(const DetectedRegion& region, 
                                      const QList<DetectedRegion>& allRegions) {
    if (region.inferredType == "unknown") {
        return 0.8; // Unknown type is acceptable
    }
    
    QList<DetectedRegion> neighbors = findNeighbors(region, allRegions, 0.1);
    
    if (neighbors.isEmpty()) {
        return 0.8; // No neighbors to compare
    }
    
    // Check type consistency with neighbors
    int matchingTypes = 0;
    for (const auto& neighbor : neighbors) {
        if (neighbor.inferredType == region.inferredType || 
            neighbor.inferredType == "unknown") {
            matchingTypes++;
        }
    }
    
    double consistency = (double)matchingTypes / neighbors.size();
    
    // Penalty if types don't match
    if (consistency < 0.5) {
        return 0.5; // Low consistency
    } else if (consistency < 0.8) {
        return 0.8; // Moderate consistency
    } else {
        return 1.0; // Good consistency
    }
}

double RegionValidator::validatePattern(const DetectedRegion& region, 
                                        const QList<DetectedRegion>& allRegions) {
    // If region has a suggested group, it likely fits a pattern
    if (!region.suggestedGroup.isEmpty()) {
        return 1.0; // Fits pattern
    }
    
    // Check if region is part of a detected sequence
    QList<DetectedRegion> neighbors = findNeighbors(region, allRegions, 0.05);
    
    if (neighbors.size() >= 2) {
        // Check if neighbors form a sequence (similar sizes, aligned)
        bool isSequence = true;
        double firstY = (neighbors[0].coords.y1 + neighbors[0].coords.y2) / 2.0;
        for (int i = 1; i < neighbors.size(); ++i) {
            double y = (neighbors[i].coords.y1 + neighbors[i].coords.y2) / 2.0;
            if (std::abs(y - firstY) > 0.02) {
                isSequence = false;
                break;
            }
        }
        
        if (isSequence) {
            return 0.9; // Likely part of a pattern
        }
    }
    
    return 0.6; // Doesn't clearly fit a pattern
}

double RegionValidator::validateAlignment(const DetectedRegion& region, 
                                          const GridStructure& gridStructure) {
    if (gridStructure.rows == 0 || gridStructure.cols == 0) {
        return 1.0; // No grid to align with
    }
    
    // Check if region aligns with grid cells
    double regionCenterX = (region.coords.x1 + region.coords.x2) / 2.0;
    double regionCenterY = (region.coords.y1 + region.coords.y2) / 2.0;
    
    // Simple check: see if region center is close to any grid cell center
    // This is a simplified check - full implementation would check actual grid cell positions
    double tolerance = 0.05; // 5% tolerance
    
    // For now, if grid is detected, give moderate confidence
    // Full implementation would check actual alignment
    return 0.9; // Likely aligned with grid
}

double RegionValidator::validateRegion(const DetectedRegion& region,
                                       const QList<DetectedRegion>& allRegions,
                                       const GridStructure& gridStructure) {
    // Perform all validation layers
    double spatialScore = validateSpatial(region, allRegions);
    double sizeScore = validateSize(region, allRegions);
    double typeScore = validateType(region, allRegions);
    double patternScore = validatePattern(region, allRegions);
    double alignmentScore = validateAlignment(region, gridStructure);
    
    // Weighted average
    double totalWeight = spatialWeight + sizeWeight + typeWeight + patternWeight + alignmentWeight;
    if (totalWeight == 0.0) {
        return 0.5; // Default if all weights are zero
    }
    
    double combinedScore = (
        spatialScore * spatialWeight +
        sizeScore * sizeWeight +
        typeScore * typeWeight +
        patternScore * patternWeight +
        alignmentScore * alignmentWeight
    ) / totalWeight;
    
    return combinedScore;
}

} // namespace ocr_orc
