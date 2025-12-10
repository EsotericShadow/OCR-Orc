#ifndef CANVAS_ZOOM_MANAGER_H
#define CANVAS_ZOOM_MANAGER_H

#include <QtCore/QPointF>
#include <QtGui/QImage>  // Full include for QImage
#include "../../../models/DocumentState.h"

namespace ocr_orc {

class CanvasZoomController;
class CanvasCoordinateCache;

/**
 * @brief Manages zoom operations for Canvas
 */
class CanvasZoomManager {
public:
    CanvasZoomManager();
    ~CanvasZoomManager();
    
    void zoomIn(DocumentState* documentState,
                const QImage& documentImage,
                int canvasWidth, int canvasHeight,
                double& scaleFactor,
                QPointF& imageOffset,
                CanvasZoomController* zoomController,
                CanvasCoordinateCache* coordinateCache);
    
    void zoomOut(DocumentState* documentState,
                 const QImage& documentImage,
                 int canvasWidth, int canvasHeight,
                 double& scaleFactor,
                 QPointF& imageOffset,
                 CanvasZoomController* zoomController,
                 CanvasCoordinateCache* coordinateCache);
    
    void zoomReset(DocumentState* documentState,
                   const QImage& documentImage,
                   int canvasWidth, int canvasHeight,
                   double& scaleFactor,
                   QPointF& imageOffset,
                   CanvasZoomController* zoomController,
                   CanvasCoordinateCache* coordinateCache);
    
    void setZoom(double zoomLevel,
                 DocumentState* documentState,
                 const QImage& documentImage,
                 int canvasWidth, int canvasHeight,
                 double& scaleFactor,
                 QPointF& imageOffset,
                 CanvasZoomController* zoomController,
                 CanvasCoordinateCache* coordinateCache);
    
    void setZoomAtPoint(double zoomLevel,
                       const QPointF& widgetPos,
                       DocumentState* documentState,
                       const QImage& documentImage,
                       int canvasWidth, int canvasHeight,
                       double& scaleFactor,
                       QPointF& imageOffset,
                       CanvasZoomController* zoomController,
                       CanvasCoordinateCache* coordinateCache);
    
    double getZoom(DocumentState* documentState,
                   CanvasZoomController* zoomController) const;
};

} // namespace ocr_orc

#endif // CANVAS_ZOOM_MANAGER_H

