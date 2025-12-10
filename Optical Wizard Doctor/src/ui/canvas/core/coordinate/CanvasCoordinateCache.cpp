#include "CanvasCoordinateCache.h"
#include "../../../../models/RegionData.h"

namespace ocr_orc {

CanvasCoordinateCache::CanvasCoordinateCache()
    : cachedZoomLevel(1.0)
    , cachedImageOffset(0.0, 0.0)
    , cachedImageSize(0, 0)
    , cacheValid(false)
{
}

CanvasCoordinateCache::~CanvasCoordinateCache() {
    // QMap automatically cleans up
}

QRectF CanvasCoordinateCache::getCachedCoordinates(const QString& regionName,
                                                    DocumentState* documentState,
                                                    int imgWidth, int imgHeight,
                                                    double scaleFactor,
                                                    const QPointF& imageOffset) {
    if (!documentState || !documentState->hasRegion(regionName)) {
        return QRectF();
    }
    
    // Check if cached
    if (coordinateCache.contains(regionName)) {
        return coordinateCache.value(regionName);
    }
    
    // Cache miss - calculate and cache it
    RegionData region = documentState->getRegion(regionName);
    NormalizedCoords norm = region.normalizedCoords;
    CanvasCoords canvasCoords = CoordinateSystem::normalizedToCanvas(
        norm, imgWidth, imgHeight, scaleFactor, imageOffset
    );
    
    QRectF canvasRect(
        canvasCoords.x1,
        canvasCoords.y1,
        canvasCoords.x2 - canvasCoords.x1,
        canvasCoords.y2 - canvasCoords.y1
    );
    
    // Cache it
    coordinateCache[regionName] = canvasRect;
    
    return canvasRect;
}

void CanvasCoordinateCache::updateCache(DocumentState* documentState,
                                        int imgWidth, int imgHeight,
                                        double scaleFactor,
                                        const QPointF& imageOffset,
                                        double zoomLevel) {
    if (!documentState) {
        return;
    }
    
    coordinateCache.clear();
    
    QList<QString> regionNames = documentState->getAllRegionNames();
    for (const QString& regionName : regionNames) {
        RegionData region = documentState->getRegion(regionName);
        
        // Convert normalized coordinates to canvas coordinates
        NormalizedCoords norm = region.normalizedCoords;
        CanvasCoords canvasCoords = CoordinateSystem::normalizedToCanvas(
            norm, imgWidth, imgHeight, scaleFactor, imageOffset
        );
        
        // Create QRectF from canvas coordinates and cache it
        QRectF canvasRect(
            canvasCoords.x1,
            canvasCoords.y1,
            canvasCoords.x2 - canvasCoords.x1,
            canvasCoords.y2 - canvasCoords.y1
        );
        coordinateCache[regionName] = canvasRect;
    }
    
    // Update cache metadata
    cachedZoomLevel = zoomLevel;
    cachedImageOffset = imageOffset;
    cachedImageSize = QSize(imgWidth, imgHeight);
    cacheValid = true;
}

bool CanvasCoordinateCache::needsUpdate(double zoomLevel,
                                        const QPointF& imageOffset,
                                        const QSize& imageSize) const {
    return !cacheValid ||
           cachedZoomLevel != zoomLevel ||
           cachedImageOffset != imageOffset ||
           cachedImageSize != imageSize;
}

void CanvasCoordinateCache::invalidate() {
    cacheValid = false;
    coordinateCache.clear();
}

} // namespace ocr_orc

