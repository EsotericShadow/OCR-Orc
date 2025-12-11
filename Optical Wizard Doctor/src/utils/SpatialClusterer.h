#ifndef SPATIAL_CLUSTERER_H
#define SPATIAL_CLUSTERER_H

#include <QtCore/QList>
#include <QtCore/QString>

namespace ocr_orc {

// Forward declarations
struct DetectedRegion;
struct GridStructure;

/**
 * @brief Spatial clustering and pattern detection for regions
 * 
 * Groups regions by spatial relationships:
 * - Horizontal alignment (same Y-range)
 * - Vertical alignment (same X-range)
 * - Grid structures
 * - Sequential patterns
 */
class SpatialClusterer {
public:
    SpatialClusterer();
    ~SpatialClusterer() = default;
    
    /**
     * @brief Group regions by horizontal alignment (same Y-range)
     * @param regions List of detected regions
     * @param tolerance Tolerance for Y-coordinate matching (normalized, default: 0.01 = 1% of image height)
     * @return List of groups, each group contains horizontally aligned regions
     */
    QList<QList<DetectedRegion>> clusterByHorizontalAlignment(
        const QList<DetectedRegion>& regions,
        double tolerance = 0.01
    );
    
    /**
     * @brief Group regions by vertical alignment (same X-range)
     * @param regions List of detected regions
     * @param tolerance Tolerance for X-coordinate matching (normalized, default: 0.01 = 1% of image width)
     * @return List of groups, each group contains vertically aligned regions
     */
    QList<QList<DetectedRegion>> clusterByVerticalAlignment(
        const QList<DetectedRegion>& regions,
        double tolerance = 0.01
    );
    
    /**
     * @brief Detect grid structure from regions
     * @param regions List of detected regions
     * @return GridStructure with detected grid information
     */
    struct GridStructure detectGridStructure(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Detect sequential pattern (cells in a row/column)
     * @param regions List of detected regions
     * @param maxGap Maximum gap between cells (normalized, default: 0.02 = 2% of image width/height)
     * @return List of detected sequences
     */
    QList<QList<DetectedRegion>> detectSequentialPattern(
        const QList<DetectedRegion>& regions,
        double maxGap = 0.02
    );
    
private:
    /**
     * @brief Calculate average Y-coordinate of a region
     */
    double getCenterY(const DetectedRegion& region);
    
    /**
     * @brief Calculate average X-coordinate of a region
     */
    double getCenterX(const DetectedRegion& region);
    
    /**
     * @brief Check if two regions are horizontally aligned (within tolerance)
     */
    bool isHorizontallyAligned(const DetectedRegion& r1, const DetectedRegion& r2, double tolerance);
    
    /**
     * @brief Check if two regions are vertically aligned (within tolerance)
     */
    bool isVerticallyAligned(const DetectedRegion& r1, const DetectedRegion& r2, double tolerance);
    
    /**
     * @brief Sort regions by X-coordinate
     */
    void sortByX(QList<DetectedRegion>& regions);
    
    /**
     * @brief Sort regions by Y-coordinate
     */
    void sortByY(QList<DetectedRegion>& regions);
};

} // namespace ocr_orc

#endif // SPATIAL_CLUSTERER_H
