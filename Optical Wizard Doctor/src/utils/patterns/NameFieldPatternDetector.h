#ifndef NAME_FIELD_PATTERN_DETECTOR_H
#define NAME_FIELD_PATTERN_DETECTOR_H

#include <QtCore/QList>
#include <QtCore/QString>
#include "../RegionDetector.h"

namespace ocr_orc {

/**
 * @brief Detector for name field patterns (horizontal sequences of letter cells)
 * 
 * Detects name field patterns:
 * - Horizontal sequence of regions
 * - All regions are letter type (or inferred as letters)
 * - Similar sizes
 * - Good alignment
 * - Suggests group names: "Firstname" or "Lastname"
 * - Suggests colors: red/blue (alternating for distinction)
 */
class NameFieldPatternDetector {
public:
    NameFieldPatternDetector();
    ~NameFieldPatternDetector() = default;
    
    /**
     * @brief Detect name field pattern
     * @param regions List of detected regions
     * @return List of detected name field groups
     */
    QList<QList<DetectedRegion>> detectNameFields(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Validate name field pattern
     * @param fieldRegions List of regions forming a name field
     * @return true if valid name field
     */
    bool validateNameField(const QList<DetectedRegion>& fieldRegions);

private:
    /**
     * @brief Check if regions form a horizontal sequence
     */
    bool isHorizontalSequence(const QList<DetectedRegion>& regions, double maxGap = 0.02);
    
    /**
     * @brief Check size consistency
     */
    bool areSizesConsistent(const QList<DetectedRegion>& regions, double tolerance = 0.3);
    
    /**
     * @brief Check alignment
     */
    bool areAligned(const QList<DetectedRegion>& regions, double tolerance = 0.01);
    
    /**
     * @brief Suggest group name (Firstname or Lastname)
     */
    QString suggestGroupName(int fieldIndex);
    
    /**
     * @brief Suggest color (red or blue, alternating)
     */
    QString suggestColor(int fieldIndex);
};

} // namespace ocr_orc

#endif // NAME_FIELD_PATTERN_DETECTOR_H
