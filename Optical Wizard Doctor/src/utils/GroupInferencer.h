#ifndef GROUP_INFERENCER_H
#define GROUP_INFERENCER_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include "RegionDetector.h"
#include "SpatialClusterer.h"
#include "patterns/PostalCodePatternDetector.h"
#include "patterns/NameFieldPatternDetector.h"
#include "patterns/NumberSequencePatternDetector.h"

namespace ocr_orc {

/**
 * @brief Group inference system
 * 
 * Infers groups from spatial relationships and patterns:
 * - Spatial clustering (proximity, alignment)
 * - Pattern-based grouping (postal codes, name fields, number sequences)
 * - Color suggestion based on pattern type
 */
class GroupInferencer {
public:
    GroupInferencer();
    ~GroupInferencer() = default;
    
    /**
     * @brief Infer groups from spatial relationships
     * @param regions List of detected regions
     * @return List of inferred groups
     */
    QList<DetectedGroup> inferGroupsFromSpatial(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Infer groups from detected patterns
     * @param regions List of detected regions
     * @return List of inferred groups from patterns
     */
    QList<DetectedGroup> inferGroupsFromPatterns(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Suggest color for a group based on pattern type
     * @param groupName Group name
     * @param patternType Pattern type (e.g., "postal_code", "name_field", "number_sequence")
     * @return Suggested color name
     */
    QString suggestColorForGroup(const QString& groupName, const QString& patternType = "");

private:
    SpatialClusterer clusterer;
    PostalCodePatternDetector postalCodeDetector;
    NameFieldPatternDetector nameFieldDetector;
    NumberSequencePatternDetector numberSequenceDetector;
    
    /**
     * @brief Check type consistency in a group
     */
    bool areTypesConsistent(const QList<DetectedRegion>& regions);
    
    /**
     * @brief Check size consistency in a group
     */
    bool areSizesConsistent(const QList<DetectedRegion>& regions, double tolerance = 0.3);
};

} // namespace ocr_orc

#endif // GROUP_INFERENCER_H
