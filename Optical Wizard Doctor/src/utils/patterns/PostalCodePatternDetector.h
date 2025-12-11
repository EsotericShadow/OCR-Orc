#ifndef POSTAL_CODE_PATTERN_DETECTOR_H
#define POSTAL_CODE_PATTERN_DETECTOR_H

#include <QtCore/QList>
#include <QtCore/QString>
#include "../RegionDetector.h"

namespace ocr_orc {

/**
 * @brief Pattern structure for Canadian postal code
 */
struct PostalCodePattern {
    QList<DetectedRegion> group1;  // [L][N][L] - first 3 cells
    QList<DetectedRegion> group2;  // [N][L][N] - last 3 cells
    int separatorIndex;            // Index where separator occurs (-1 if no separator detected)
    double confidence;             // Confidence in pattern match
    QString patternString;         // Detected pattern string
    
    PostalCodePattern() : separatorIndex(-1), confidence(0.0) {}
};

/**
 * @brief Detector for Canadian postal code patterns (A#A-#A# format)
 * 
 * Detects and validates Canadian postal code patterns:
 * - Format: [Letter][Number][Letter]-[Number][Letter][Number]
 * - Example: K1A-0B1
 * - Handles variations: with/without dash, slight misalignment
 */
class PostalCodePatternDetector {
public:
    PostalCodePatternDetector();
    ~PostalCodePatternDetector() = default;
    
    /**
     * @brief Detect postal code pattern from regions
     * @param regions List of detected regions (should be in postal code area)
     * @return PostalCodePattern if detected, or empty pattern if not found
     */
    PostalCodePattern detectPattern(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Validate pattern matches A#A-#A# format
     * @param pattern Pattern to validate
     * @return true if pattern is valid
     */
    bool validatePattern(const PostalCodePattern& pattern);
    
    /**
     * @brief Group regions into postal code structure
     * @param regions List of regions to group
     * @return List of regions grouped as postal code (6 regions)
     */
    QList<DetectedRegion> groupPostalCode(const QList<DetectedRegion>& regions);

private:
    /**
     * @brief Find 6 regions in close horizontal proximity
     */
    QList<DetectedRegion> findCandidateRegions(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Detect separator (dash or space) between groups
     */
    int detectSeparator(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Validate alternating pattern [L][N][L]-[N][L][N]
     */
    bool validateAlternatingPattern(const QList<DetectedRegion>& regions, int separatorIndex);
    
    /**
     * @brief Calculate distance between two regions (normalized)
     */
    double calculateDistance(const DetectedRegion& r1, const DetectedRegion& r2);
    
    /**
     * @brief Check if regions are horizontally aligned
     */
    bool areHorizontallyAligned(const QList<DetectedRegion>& regions, double tolerance = 0.02);
};

} // namespace ocr_orc

#endif // POSTAL_CODE_PATTERN_DETECTOR_H
