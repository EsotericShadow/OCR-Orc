#ifndef STATE_SNAPSHOT_H
#define STATE_SNAPSHOT_H

#include "RegionData.h"
#include "GroupData.h"
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QSize>

namespace ocr_orc {

/**
 * @brief State snapshot for undo/redo functionality
 * 
 * Captures complete document state at a point in time.
 * Used by undo/redo system to restore previous states.
 * 
 * Qt's implicit sharing (copy-on-write) makes these snapshots
 * memory-efficient - actual deep copy only happens when data is modified.
 */
struct StateSnapshot {
    // Core data (source of truth)
    QMap<QString, RegionData> regions;  // All regions with normalized coordinates
    QMap<QString, GroupData> groups;    // All groups
    
    // Document metadata
    QString pdfPath;     // PDF file path
    QSize imageSize;     // Image dimensions (for validation)
    
    /**
     * @brief Create empty snapshot
     */
    StateSnapshot() : imageSize(0, 0) {}
    
    /**
     * @brief Check if snapshot is empty (no data)
     */
    bool isEmpty() const {
        return regions.isEmpty() && groups.isEmpty() && pdfPath.isEmpty();
    }
    
    /**
     * @brief Equality operator for testing
     */
    bool operator==(const StateSnapshot& other) const {
        return regions == other.regions &&
               groups == other.groups &&
               pdfPath == other.pdfPath &&
               imageSize == other.imageSize;
    }
    
    bool operator!=(const StateSnapshot& other) const {
        return !(*this == other);
    }
};

} // namespace ocr_orc

#endif // STATE_SNAPSHOT_H

