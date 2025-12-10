#ifndef GROUP_DATA_H
#define GROUP_DATA_H

#include <QtCore/QString>
#include <QtCore/QList>

namespace ocr_orc {

/**
 * @brief Group data structure
 * 
 * Stores information about a group of regions.
 * Groups are collections of related regions (e.g., "lastname", "firstname").
 */
struct GroupData {
    QString name;
    QList<QString> regionNames;  // List of region names in this group (preserves order)
    
    // Constructors
    GroupData() = default;
    
    GroupData(const QString& name)
        : name(name)
    {}
    
    GroupData(const QString& name, const QList<QString>& regionNames)
        : name(name)
        , regionNames(regionNames)
    {}
    
    // Equality operator (for testing)
    bool operator==(const GroupData& other) const {
        return name == other.name && regionNames == other.regionNames;
    }
    
    /**
     * @brief Add a region to this group
     * @param regionName Name of region to add
     */
    void addRegion(const QString& regionName);
    
    /**
     * @brief Remove a region from this group
     * @param regionName Name of region to remove
     */
    void removeRegion(const QString& regionName);
    
    /**
     * @brief Check if group contains a region
     * @param regionName Name of region to check
     * @return true if region is in group
     */
    bool contains(const QString& regionName) const;
    
    /**
     * @brief Get number of regions in group
     * @return Count of regions
     */
    int count() const;
    
    /**
     * @brief Check if group is empty
     * @return true if no regions in group
     */
    bool isEmpty() const;
};

} // namespace ocr_orc

#endif // GROUP_DATA_H

