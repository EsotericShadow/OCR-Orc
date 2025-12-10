#include "CanvasRegionManager.h"
#include "../core/regions/CanvasRegionOperations.h"
#include <QtGui/QImage>

namespace ocr_orc {

CanvasRegionManager::CanvasRegionManager() {
}

CanvasRegionManager::~CanvasRegionManager() {
}

QList<QString> CanvasRegionManager::duplicateSelectedRegions(DocumentState* documentState,
                                                             const QSet<QString>& selectedRegions,
                                                             QSet<QString>& updatedSelectedRegions,
                                                             QString& primarySelectedRegion) {
    if (!documentState || selectedRegions.isEmpty()) {
        return QList<QString>();
    }
    
    // Save state BEFORE duplicating (for undo)
    documentState->saveState();
    
    // Convert QSet to QList for DocumentState
    QList<QString> regionNames = selectedRegions.values();
    
    // Duplicate regions
    QList<QString> duplicatedNames = documentState->duplicateRegions(regionNames);
    
    if (!duplicatedNames.isEmpty()) {
        // Select the newly duplicated regions
        updatedSelectedRegions.clear();
        for (const QString& name : duplicatedNames) {
            updatedSelectedRegions.insert(name);
        }
        primarySelectedRegion = duplicatedNames.first();
    }
    
    return duplicatedNames;
}

void CanvasRegionManager::moveRegion(const QString& regionName,
                                     const QPointF& delta,
                                     DocumentState* documentState,
                                     const QImage& documentImage,
                                     double scaleFactor,
                                     const QPointF& imageOffset,
                                     CanvasRegionOperations* regionOperations) {
    if (!documentState || !regionOperations) {
        return;
    }
    regionOperations->moveRegion(regionName, delta, documentState, documentImage,
                                scaleFactor, imageOffset);
}

void CanvasRegionManager::moveSelectedRegions(const QPointF& delta,
                                              DocumentState* documentState,
                                              const QImage& documentImage,
                                              double scaleFactor,
                                              const QPointF& imageOffset,
                                              const QSet<QString>& selectedRegions,
                                              CanvasRegionOperations* regionOperations) {
    if (!documentState || !regionOperations) {
        return;
    }
    QList<QString> regionNames = selectedRegions.values();
    regionOperations->moveRegions(regionNames, delta, documentState, documentImage,
                                scaleFactor, imageOffset);
}

} // namespace ocr_orc

