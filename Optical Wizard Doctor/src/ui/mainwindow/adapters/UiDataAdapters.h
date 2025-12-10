#ifndef UI_DATA_ADAPTERS_H
#define UI_DATA_ADAPTERS_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QMap>

namespace ocr_orc {

// Forward declarations
class DocumentState;
struct RegionData;
struct GroupData;

/**
 * @brief Pure data adapters for transforming DocumentState into UI models
 * 
 * Provides pure transformations (no Qt widgets):
 * - DocumentState → region list model (names, selected, group mapping)
 * - DocumentState → group list model (name → size)
 * - Selection sets and status strings
 * 
 * These adapters are stateless and contain no widget dependencies.
 */
class UiDataAdapters {
public:
    /**
     * @brief Region list model structure
     */
    struct RegionListModel {
        QList<QString> regionNames;        // All region names
        QSet<QString> selectedRegions;     // Currently selected regions
        QMap<QString, QString> regionGroups; // Region name → group name mapping
    };
    
    /**
     * @brief Group list model structure
     */
    struct GroupListModel {
        QList<QString> groupNames;         // All group names
        QMap<QString, int> groupSizes;     // Group name → number of regions
    };
    
    /**
     * @brief Transform DocumentState to region list model
     * @param documentState Document state
     * @param selectedRegions Currently selected regions
     * @return Region list model
     */
    static RegionListModel toRegionListModel(DocumentState* documentState,
                                            const QSet<QString>& selectedRegions);
    
    /**
     * @brief Transform DocumentState to group list model
     * @param documentState Document state
     * @return Group list model
     */
    static GroupListModel toGroupListModel(DocumentState* documentState);
    
    /**
     * @brief Get selection status string
     * @param selectedCount Number of selected regions
     * @param totalCount Total number of regions
     * @return Status string (e.g., "2 of 10 selected")
     */
    static QString getSelectionStatusString(int selectedCount, int totalCount);
    
    /**
     * @brief Get region count status string
     * @param regionCount Number of regions
     * @return Status string (e.g., "10 regions")
     */
    static QString getRegionCountStatusString(int regionCount);
    
    /**
     * @brief Get group count status string
     * @param groupCount Number of groups
     * @return Status string (e.g., "3 groups")
     */
    static QString getGroupCountStatusString(int groupCount);
};

} // namespace ocr_orc

#endif // UI_DATA_ADAPTERS_H

