#ifndef CANVAS_COORDINATE_CACHE_H
#define CANVAS_COORDINATE_CACHE_H

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include "../../../../models/DocumentState.h"
#include "../../../../core/CoordinateSystem.h"

namespace ocr_orc {

/**
 * @brief Manages coordinate caching for canvas regions
 * 
 * Caches canvas coordinates for regions to avoid recalculating
 * on every paint event. Cache is invalidated when zoom, pan, or
 * image size changes.
 */
class CanvasCoordinateCache {
public:
    CanvasCoordinateCache();
    ~CanvasCoordinateCache();
    
    /**
     * @brief Get cached canvas coordinates for a region
     * @param regionName Name of the region
     * @param documentState Document state containing region data
     * @param imgWidth Image width
     * @param imgHeight Image height
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     * @return Cached canvas rectangle, or calculates if not cached
     */
    QRectF getCachedCoordinates(const QString& regionName,
                                DocumentState* documentState,
                                int imgWidth, int imgHeight,
                                double scaleFactor,
                                const QPointF& imageOffset);
    
    /**
     * @brief Update cache for all regions
     * @param documentState Document state containing all regions
     * @param imgWidth Image width
     * @param imgHeight Image height
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     * @param zoomLevel Current zoom level
     */
    void updateCache(DocumentState* documentState,
                     int imgWidth, int imgHeight,
                     double scaleFactor,
                     const QPointF& imageOffset,
                     double zoomLevel);
    
    /**
     * @brief Check if cache needs updating
     * @param zoomLevel Current zoom level
     * @param imageOffset Current image offset
     * @param imageSize Current image size
     * @return true if cache needs update
     */
    bool needsUpdate(double zoomLevel,
                     const QPointF& imageOffset,
                     const QSize& imageSize) const;
    
    /**
     * @brief Invalidate the cache
     */
    void invalidate();
    
    /**
     * @brief Check if cache is valid
     * @return true if cache is valid
     */
    bool isValid() const { return cacheValid; }
    
    /**
     * @brief Get all cached region coordinates
     * @return Map of region names to canvas rectangles
     */
    QMap<QString, QRectF> getAllCachedCoordinates() const { return coordinateCache; }

private:
    QMap<QString, QRectF> coordinateCache;  // Cached canvas coordinates per region
    double cachedZoomLevel;                 // Zoom level when cache was created
    QPointF cachedImageOffset;              // Image offset when cache was created
    QSize cachedImageSize;                  // Image size when cache was created
    bool cacheValid;                        // Whether cache is valid
};

} // namespace ocr_orc

#endif // CANVAS_COORDINATE_CACHE_H

