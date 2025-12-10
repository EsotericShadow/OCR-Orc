#include "CanvasZoomManager.h"
#include "../core/zoom/CanvasZoomController.h"
#include "../core/coordinate/CanvasCoordinateCache.h"
#include <QtGui/QImage>

namespace ocr_orc {

CanvasZoomManager::CanvasZoomManager() {
}

CanvasZoomManager::~CanvasZoomManager() {
}

void CanvasZoomManager::zoomIn(DocumentState* documentState,
                               const QImage& documentImage,
                               int canvasWidth, int canvasHeight,
                               double& scaleFactor,
                               QPointF& imageOffset,
                               CanvasZoomController* zoomController,
                               CanvasCoordinateCache* coordinateCache) {
    if (!documentState || !zoomController) return;
    zoomController->zoomIn(documentState, documentImage, canvasWidth, canvasHeight,
                          scaleFactor, imageOffset, coordinateCache);
}

void CanvasZoomManager::zoomOut(DocumentState* documentState,
                                const QImage& documentImage,
                                int canvasWidth, int canvasHeight,
                                double& scaleFactor,
                                QPointF& imageOffset,
                                CanvasZoomController* zoomController,
                                CanvasCoordinateCache* coordinateCache) {
    if (!documentState || !zoomController) return;
    zoomController->zoomOut(documentState, documentImage, canvasWidth, canvasHeight,
                           scaleFactor, imageOffset, coordinateCache);
}

void CanvasZoomManager::zoomReset(DocumentState* documentState,
                                  const QImage& documentImage,
                                  int canvasWidth, int canvasHeight,
                                  double& scaleFactor,
                                  QPointF& imageOffset,
                                  CanvasZoomController* zoomController,
                                  CanvasCoordinateCache* coordinateCache) {
    if (!documentState || !zoomController) return;
    zoomController->zoomReset(documentState, documentImage, canvasWidth, canvasHeight,
                             scaleFactor, imageOffset, coordinateCache);
}

void CanvasZoomManager::setZoom(double zoomLevel,
                                DocumentState* documentState,
                                const QImage& documentImage,
                                int canvasWidth, int canvasHeight,
                                double& scaleFactor,
                                QPointF& imageOffset,
                                CanvasZoomController* zoomController,
                                CanvasCoordinateCache* coordinateCache) {
    if (!documentState || !zoomController) return;
    zoomController->setZoom(zoomLevel, documentState, documentImage,
                           canvasWidth, canvasHeight, scaleFactor, imageOffset, coordinateCache);
}

void CanvasZoomManager::setZoomAtPoint(double zoomLevel,
                                       const QPointF& widgetPos,
                                       DocumentState* documentState,
                                       const QImage& documentImage,
                                       int canvasWidth, int canvasHeight,
                                       double& scaleFactor,
                                       QPointF& imageOffset,
                                       CanvasZoomController* zoomController,
                                       CanvasCoordinateCache* coordinateCache) {
    if (!documentState || documentImage.isNull() || !zoomController) return;
    zoomController->setZoomAtPoint(zoomLevel, widgetPos, documentState, documentImage,
                                  canvasWidth, canvasHeight, scaleFactor, imageOffset, coordinateCache);
}

double CanvasZoomManager::getZoom(DocumentState* documentState,
                                 CanvasZoomController* zoomController) const {
    if (!zoomController) {
        return documentState ? documentState->zoomLevel : 1.0;
    }
    return zoomController->getZoom(documentState);
}

} // namespace ocr_orc

