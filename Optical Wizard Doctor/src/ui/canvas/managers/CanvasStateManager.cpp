#include "CanvasStateManager.h"
#include "../core/selection/CanvasSelectionManager.h"
#include "../core/coordinate/CanvasCoordinateCache.h"

namespace ocr_orc {

CanvasStateManager::CanvasStateManager() {
}

CanvasStateManager::~CanvasStateManager() {
}

void CanvasStateManager::clearSelection(CanvasSelectionManager* selectionManager,
                                        QSet<QString>& selectedRegions,
                                        QString& primarySelectedRegion) {
    if (!selectionManager) return;
    selectionManager->clearSelection(selectedRegions, primarySelectedRegion);
}

void CanvasStateManager::selectRegion(const QString& regionName,
                                     CanvasSelectionManager* selectionManager,
                                     QSet<QString>& selectedRegions,
                                     QString& primarySelectedRegion) {
    if (!selectionManager) return;
    selectionManager->selectRegion(regionName, selectedRegions, primarySelectedRegion);
}

void CanvasStateManager::toggleRegionSelection(const QString& regionName,
                                               CanvasSelectionManager* selectionManager,
                                               QSet<QString>& selectedRegions,
                                               QString& primarySelectedRegion) {
    if (!selectionManager) return;
    selectionManager->toggleRegionSelection(regionName, selectedRegions, primarySelectedRegion);
}

void CanvasStateManager::addToSelection(const QSet<QString>& regionNames,
                                       CanvasSelectionManager* selectionManager,
                                       QSet<QString>& selectedRegions,
                                       QString& primarySelectedRegion) {
    if (!selectionManager) return;
    selectionManager->addToSelection(regionNames, selectedRegions, primarySelectedRegion);
}

void CanvasStateManager::invalidateCoordinateCache(CanvasCoordinateCache* coordinateCache) {
    if (coordinateCache) {
        coordinateCache->invalidate();
    }
}

void CanvasStateManager::setHoveredRegion(const QString& regionName,
                                         QString& hoveredRegion) {
    if (hoveredRegion != regionName) {
        hoveredRegion = regionName;
    }
}

} // namespace ocr_orc

