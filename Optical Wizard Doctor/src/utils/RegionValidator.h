#ifndef REGION_VALIDATOR_H
#define REGION_VALIDATOR_H

#include <QtCore/QList>
#include "RegionDetector.h"
#include "SpatialClusterer.h"

namespace ocr_orc {

/**
 * @brief Multi-layer validation for detected regions
 * 
 * Validates regions using multiple criteria:
 * - Spatial: Has neighbors? (penalty if isolated)
 * - Size: Consistent with neighbors? (penalty if deviates)
 * - Type: Consistent in context? (penalty if mismatches)
 * - Pattern: Fits detected pattern? (penalty if doesn't fit)
 * - Alignment: Aligned with grid? (penalty if misaligned)
 */
class RegionValidator {
public:
    RegionValidator();
    ~RegionValidator() = default;
    
    /**
     * @brief Validate a region using multi-layer validation
     * @param region Region to validate
     * @param allRegions All detected regions (for context)
     * @param gridStructure Detected grid structure (if any)
     * @return Combined confidence score (0.0-1.0)
     */
    double validateRegion(
        const DetectedRegion& region,
        const QList<DetectedRegion>& allRegions,
        const GridStructure& gridStructure = GridStructure()
    );
    
    /**
     * @brief Set validation weights (default: all 1.0)
     */
    void setWeights(double spatialWeight, double sizeWeight, double typeWeight, 
                    double patternWeight, double alignmentWeight);

private:
    double spatialWeight;
    double sizeWeight;
    double typeWeight;
    double patternWeight;
    double alignmentWeight;
    
    SpatialClusterer clusterer;
    
    /**
     * @brief Spatial validation: Check if region has neighbors
     */
    double validateSpatial(const DetectedRegion& region, const QList<DetectedRegion>& allRegions);
    
    /**
     * @brief Size validation: Check consistency with neighbors
     */
    double validateSize(const DetectedRegion& region, const QList<DetectedRegion>& allRegions);
    
    /**
     * @brief Type validation: Check type consistency in context
     */
    double validateType(const DetectedRegion& region, const QList<DetectedRegion>& allRegions);
    
    /**
     * @brief Pattern validation: Check if region fits detected pattern
     */
    double validatePattern(const DetectedRegion& region, const QList<DetectedRegion>& allRegions);
    
    /**
     * @brief Alignment validation: Check alignment with grid
     */
    double validateAlignment(const DetectedRegion& region, const GridStructure& gridStructure);
    
    /**
     * @brief Find neighbors within distance threshold
     */
    QList<DetectedRegion> findNeighbors(const DetectedRegion& region, 
                                        const QList<DetectedRegion>& allRegions,
                                        double distanceThreshold = 0.1);
};

} // namespace ocr_orc

#endif // REGION_VALIDATOR_H
