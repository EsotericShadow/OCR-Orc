#ifndef CANVAS_STATE_MANAGER_H
#define CANVAS_STATE_MANAGER_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include "../../../models/DocumentState.h"

namespace ocr_orc {

class CanvasSelectionManager;
class CanvasCoordinateCache;

/**
 * @brief Manages Canvas state operations (selection, cache, etc.)
 */
class CanvasStateManager {
public:
    CanvasStateManager();
    ~CanvasStateManager();
    
    void clearSelection(CanvasSelectionManager* selectionManager,
                       QSet<QString>& selectedRegions,
                       QString& primarySelectedRegion);
    
    void selectRegion(const QString& regionName,
                     CanvasSelectionManager* selectionManager,
                     QSet<QString>& selectedRegions,
                     QString& primarySelectedRegion);
    
    void toggleRegionSelection(const QString& regionName,
                              CanvasSelectionManager* selectionManager,
                              QSet<QString>& selectedRegions,
                              QString& primarySelectedRegion);
    
    void addToSelection(const QSet<QString>& regionNames,
                       CanvasSelectionManager* selectionManager,
                       QSet<QString>& selectedRegions,
                       QString& primarySelectedRegion);
    
    void invalidateCoordinateCache(CanvasCoordinateCache* coordinateCache);
    
    void setHoveredRegion(const QString& regionName,
                         QString& hoveredRegion);
};

} // namespace ocr_orc

#endif // CANVAS_STATE_MANAGER_H

