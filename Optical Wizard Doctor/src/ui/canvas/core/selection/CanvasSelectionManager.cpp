#include "CanvasSelectionManager.h"
#include "../../../../models/RegionData.h"
#include <QtGui/QImage>

namespace ocr_orc {

CanvasSelectionManager::CanvasSelectionManager() {
}

CanvasSelectionManager::~CanvasSelectionManager() {
}

void CanvasSelectionManager::clearSelection(QSet<QString>& selectedRegions,
                                            QString& primarySelectedRegion) {
    selectedRegions.clear();
    primarySelectedRegion = QString();
}

void CanvasSelectionManager::selectRegion(const QString& regionName,
                                         QSet<QString>& selectedRegions,
                                         QString& primarySelectedRegion) {
    selectedRegions.clear();
    selectedRegions.insert(regionName);
    primarySelectedRegion = regionName;
    
    // Validate selection consistency
    validateSelection(selectedRegions, primarySelectedRegion);
}

void CanvasSelectionManager::toggleRegionSelection(const QString& regionName,
                                                   QSet<QString>& selectedRegions,
                                                   QString& primarySelectedRegion) {
    if (selectedRegions.contains(regionName)) {
        // Remove from selection
        selectedRegions.remove(regionName);
        if (primarySelectedRegion == regionName) {
            // Update primary selection
            if (selectedRegions.isEmpty()) {
                primarySelectedRegion = QString();
            } else {
                primarySelectedRegion = *selectedRegions.begin();
            }
        }
    } else {
        // Add to selection - this becomes the primary (last selected)
        selectedRegions.insert(regionName);
            primarySelectedRegion = regionName;
    }
    
    // Validate selection consistency
    validateSelection(selectedRegions, primarySelectedRegion);
}

void CanvasSelectionManager::addToSelection(const QSet<QString>& regionNames,
                                           QSet<QString>& selectedRegions,
                                           QString& primarySelectedRegion) {
    selectedRegions.unite(regionNames);
    if (selectedRegions.size() == 1) {
        primarySelectedRegion = *selectedRegions.begin();
    } else if (selectedRegions.size() > 1) {
        // Keep primary if it's still selected, otherwise pick first from new set
        // (The caller should set primary to the last region if needed)
        if (!selectedRegions.contains(primarySelectedRegion)) {
            primarySelectedRegion = *selectedRegions.begin();
        }
    }
    
    // Validate selection consistency
    validateSelection(selectedRegions, primarySelectedRegion);
}

QSet<QString> CanvasSelectionManager::findRegionsInBox(const QRectF& box,
                                                       DocumentState* documentState,
                                                       const QImage& documentImage,
                                                       double scaleFactor,
                                                       const QPointF& imageOffset) const {
    QSet<QString> result;
    
    if (!documentState || documentImage.isNull()) {
        return result;
    }
    
    // Get all region names
    QList<QString> regionNames = documentState->getAllRegionNames();
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    
    for (const QString& regionName : regionNames) {
        RegionData region = documentState->getRegion(regionName);
        
        // Convert normalized coordinates to canvas coordinates
        NormalizedCoords norm = region.normalizedCoords;
        CanvasCoords canvasCoords = CoordinateSystem::normalizedToCanvas(
            norm, imgWidth, imgHeight, scaleFactor, imageOffset
        );
        
        // Create QRectF from canvas coordinates
        QRectF canvasRect(
            canvasCoords.x1,
            canvasCoords.y1,
            canvasCoords.x2 - canvasCoords.x1,
            canvasCoords.y2 - canvasCoords.y1
        );
        
        // Check if region intersects with selection box
        if (canvasRect.intersects(box)) {
            result.insert(regionName);
        }
    }
    
    return result;
}

void CanvasSelectionManager::validateSelection(QSet<QString>& selectedRegions,
                                              QString& primarySelectedRegion) {
    // If primary is not in selected set, clear it
    if (!primarySelectedRegion.isEmpty() && 
        !selectedRegions.contains(primarySelectedRegion)) {
        primarySelectedRegion = QString();
    }
    
    // If no selection but primary exists, clear primary
    if (selectedRegions.isEmpty() && !primarySelectedRegion.isEmpty()) {
        primarySelectedRegion = QString();
    }
    
    // If selection exists but no primary, set first as primary
    if (!selectedRegions.isEmpty() && primarySelectedRegion.isEmpty()) {
        primarySelectedRegion = *selectedRegions.begin();
    }
}

} // namespace ocr_orc

