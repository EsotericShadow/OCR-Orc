#ifndef CANVAS_ZOOM_CONTROLLER_H
#define CANVAS_ZOOM_CONTROLLER_H

#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtGui/QImage>
#include "../../../../models/DocumentState.h"
#include "../coordinate/CanvasCoordinateCache.h"

namespace ocr_orc {

/**
 * @brief Handles zoom and pan operations for the Canvas
 * 
 * Manages:
 * - Zoom level calculations
 * - Layout calculations (scale factor, image offset)
 * - Pan operations
 * - Coordinate cache invalidation
 */
class CanvasZoomController {
public:
    CanvasZoomController();
    ~CanvasZoomController();
    
    /**
     * @brief Calculate image position and scale to fit canvas
     * @param documentImage Document image
     * @param canvasWidth Canvas width
     * @param canvasHeight Canvas height
     * @param documentState Document state (for zoom level and offset)
     * @param scaleFactor Output: calculated scale factor
     * @param imageOffset Output: calculated image offset
     * @param imageRect Output: calculated image rectangle
     */
    void calculateLayout(const QImage& documentImage,
                        int canvasWidth, int canvasHeight,
                        DocumentState* documentState,
                        double& scaleFactor,
                        QPointF& imageOffset,
                        QRectF& imageRect);
    
    /**
     * @brief Zoom in (increase zoom by factor) at center
     * @param documentState Document state
     * @param documentImage Document image
     * @param canvasWidth Canvas width
     * @param canvasHeight Canvas height
     * @param scaleFactor Output: updated scale factor
     * @param imageOffset Output: updated image offset
     * @param coordinateCache Cache to invalidate
     */
    void zoomIn(DocumentState* documentState,
                const QImage& documentImage,
                int canvasWidth, int canvasHeight,
                double& scaleFactor,
                QPointF& imageOffset,
                CanvasCoordinateCache* coordinateCache);
    
    /**
     * @brief Zoom out (decrease zoom by factor) at center
     * @param documentState Document state
     * @param documentImage Document image
     * @param canvasWidth Canvas width
     * @param canvasHeight Canvas height
     * @param scaleFactor Output: updated scale factor
     * @param imageOffset Output: updated image offset
     * @param coordinateCache Cache to invalidate
     */
    void zoomOut(DocumentState* documentState,
                 const QImage& documentImage,
                 int canvasWidth, int canvasHeight,
                 double& scaleFactor,
                 QPointF& imageOffset,
                 CanvasCoordinateCache* coordinateCache);
    
    /**
     * @brief Reset zoom to 1.0 at center
     * @param documentState Document state
     * @param documentImage Document image
     * @param canvasWidth Canvas width
     * @param canvasHeight Canvas height
     * @param scaleFactor Output: updated scale factor
     * @param imageOffset Output: updated image offset
     * @param coordinateCache Cache to invalidate
     */
    void zoomReset(DocumentState* documentState,
                   const QImage& documentImage,
                   int canvasWidth, int canvasHeight,
                   double& scaleFactor,
                   QPointF& imageOffset,
                   CanvasCoordinateCache* coordinateCache);
    
    /**
     * @brief Set zoom level
     * @param zoomLevel New zoom level (1.0 = 100%)
     * @param documentState Document state
     * @param documentImage Document image
     * @param canvasWidth Canvas width
     * @param canvasHeight Canvas height
     * @param scaleFactor Output: updated scale factor
     * @param imageOffset Output: updated image offset
     * @param coordinateCache Cache to invalidate
     */
    void setZoom(double zoomLevel,
                 DocumentState* documentState,
                 const QImage& documentImage,
                 int canvasWidth, int canvasHeight,
                 double& scaleFactor,
                 QPointF& imageOffset,
                 CanvasCoordinateCache* coordinateCache);
    
    /**
     * @brief Set zoom level centered on a point
     * @param zoomLevel New zoom level
     * @param widgetPos Widget position to zoom at
     * @param documentState Document state
     * @param documentImage Document image
     * @param canvasWidth Canvas width
     * @param canvasHeight Canvas height
     * @param scaleFactor Output: updated scale factor
     * @param imageOffset Output: updated image offset
     * @param coordinateCache Cache to invalidate
     */
    void setZoomAtPoint(double zoomLevel,
                       const QPointF& widgetPos,
                       DocumentState* documentState,
                       const QImage& documentImage,
                       int canvasWidth, int canvasHeight,
                       double& scaleFactor,
                       QPointF& imageOffset,
                       CanvasCoordinateCache* coordinateCache);
    
    /**
     * @brief Get current zoom level
     * @param documentState Document state
     * @return Current zoom level, or 1.0 if no document state
     */
    double getZoom(DocumentState* documentState) const;
    
    /**
     * @brief Pan with wheel delta
     * @param delta Wheel delta
     * @param documentState Document state
     * @param imageOffset Output: updated image offset
     */
    void panWithWheel(const QPointF& delta,
                     DocumentState* documentState,
                     QPointF& imageOffset);
};

} // namespace ocr_orc

#endif // CANVAS_ZOOM_CONTROLLER_H

