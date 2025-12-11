#ifndef NUMBER_SEQUENCE_PATTERN_DETECTOR_H
#define NUMBER_SEQUENCE_PATTERN_DETECTOR_H

#include <QtCore/QList>
#include <QtCore/QString>
#include "../RegionDetector.h"

namespace ocr_orc {

/**
 * @brief Detector for number sequence patterns (horizontal sequences of number cells)
 * 
 * Detects number sequence patterns:
 * - Horizontal sequence of regions
 * - All regions are number type (or inferred as numbers)
 * - Similar sizes
 * - Good alignment
 * - Suggests group name: "Studentnumber" or generic "NumberSequence"
 * - Suggests color: "green"
 */
class NumberSequencePatternDetector {
public:
    NumberSequencePatternDetector();
    ~NumberSequencePatternDetector() = default;
    
    /**
     * @brief Detect number sequence pattern
     * @param regions List of detected regions
     * @return List of detected number sequences
     */
    QList<QList<DetectedRegion>> detectNumberSequences(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Validate number sequence pattern
     * @param sequenceRegions List of regions forming a number sequence
     * @return true if valid number sequence
     */
    bool validateNumberSequence(const QList<DetectedRegion>& sequenceRegions);

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
     * @brief Suggest group name
     */
    QString suggestGroupName(int sequenceIndex);
};

} // namespace ocr_orc

#endif // NUMBER_SEQUENCE_PATTERN_DETECTOR_H
