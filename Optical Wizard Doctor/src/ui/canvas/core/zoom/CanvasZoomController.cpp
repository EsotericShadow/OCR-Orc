#include "CanvasZoomController.h"
#include "../../../../core/Constants.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ocr_orc {

/**
 * @brief Validate zoom level (check for NaN, Infinity, and bounds)
 * @param zoom Zoom level to validate
 * @return true if zoom is valid, false otherwise
 */
static bool isValidZoom(double zoom) {
    return std::isfinite(zoom) && !std::isnan(zoom) && 
           zoom >= CanvasConstants::MIN_ZOOM && zoom <= CanvasConstants::MAX_ZOOM;
}

CanvasZoomController::CanvasZoomController() {
}

CanvasZoomController::~CanvasZoomController() {
}

void CanvasZoomController::calculateLayout(const QImage& documentImage,
                                          int canvasWidth, int canvasHeight,
                                          DocumentState* documentState,
                                          double& scaleFactor,
                                          QPointF& imageOffset,
                                          QRectF& imageRect) {
    if (documentImage.isNull()) {
        scaleFactor = 1.0;
        imageOffset = QPointF(0.0, 0.0);
        imageRect = QRectF();
        return;
    }
    
    if (canvasWidth <= 0 || canvasHeight <= 0) {
        // Canvas not yet sized - set safe defaults
        scaleFactor = 1.0;
        imageOffset = QPointF(0.0, 0.0);
        imageRect = QRectF();
        return;
    }
    
    // Get image size
    int imageWidth = documentImage.width();
    int imageHeight = documentImage.height();
    
    // Validate image dimensions
    if (imageWidth <= 0 || imageHeight <= 0) {
        scaleFactor = 1.0;
        imageOffset = QPointF(0.0, 0.0);
        imageRect = QRectF();
        return;
    }
    
    // Get zoom level from DocumentState if available
    double zoomLevel = 1.0;
    if (documentState) {
        zoomLevel = documentState->zoomLevel;
        // Validate and clamp zoom level
        if (!isValidZoom(zoomLevel)) {
            zoomLevel = CanvasConstants::DEFAULT_ZOOM;
            documentState->zoomLevel = zoomLevel;
        }
    }
    
    // Calculate base scale to fit canvas while maintaining aspect ratio
    double scaleX = static_cast<double>(canvasWidth) / static_cast<double>(imageWidth);
    double scaleY = static_cast<double>(canvasHeight) / static_cast<double>(imageHeight);
    double baseScale = std::min({scaleX, scaleY, 1.0}); // Don't scale up beyond 1.0
    
    // Apply zoom level
    scaleFactor = baseScale * zoomLevel;
    
    // Calculate scaled image dimensions
    int scaledWidth = static_cast<int>(imageWidth * scaleFactor);
    int scaledHeight = static_cast<int>(imageHeight * scaleFactor);
    
    // Get offset from DocumentState if available, otherwise center
    if (documentState) {
        imageOffset = documentState->imageOffset;
        // If offset is (0,0) or very close to it, center the image (first load)
        if (qAbs(imageOffset.x()) < 1.0 && qAbs(imageOffset.y()) < 1.0) {
            // Center image on first load
            imageOffset = QPointF(
                (canvasWidth - scaledWidth) / 2.0,
                (canvasHeight - scaledHeight) / 2.0
            );
            // Update DocumentState with centered offset
            documentState->imageOffset = imageOffset;
        }
    } else {
        // Center image
        imageOffset = QPointF(
            (canvasWidth - scaledWidth) / 2.0,
            (canvasHeight - scaledHeight) / 2.0
        );
    }
    
    // Update DocumentState if available
    if (documentState) {
        documentState->scaleFactor = scaleFactor;
        documentState->imageOffset = imageOffset;
        documentState->synchronizeCoordinates();
    }
    
    // Calculate image rectangle
    imageRect = QRectF(
        imageOffset.x(),
        imageOffset.y(),
        scaledWidth,
        scaledHeight
    );
}

void CanvasZoomController::zoomIn(DocumentState* documentState,
                                  const QImage& documentImage,
                                  int canvasWidth, int canvasHeight,
                                  double& scaleFactor,
                                  QPointF& imageOffset,
                                  CanvasCoordinateCache* coordinateCache) {
    if (!documentState) return;
    
    // Get center of canvas for zoom
    QPointF centerPos(canvasWidth / 2.0, canvasHeight / 2.0);
    double currentZoom = documentState->zoomLevel;
    double newZoom = currentZoom * CanvasConstants::ZOOM_FACTOR; // 20% increase
    setZoomAtPoint(newZoom, centerPos, documentState, documentImage,
                   canvasWidth, canvasHeight, scaleFactor, imageOffset, coordinateCache);
}

void CanvasZoomController::zoomOut(DocumentState* documentState,
                                   const QImage& documentImage,
                                   int canvasWidth, int canvasHeight,
                                   double& scaleFactor,
                                   QPointF& imageOffset,
                                   CanvasCoordinateCache* coordinateCache) {
    if (!documentState) return;
    
    // Get center of canvas for zoom
    QPointF centerPos(canvasWidth / 2.0, canvasHeight / 2.0);
    double currentZoom = documentState->zoomLevel;
    double newZoom = currentZoom / CanvasConstants::ZOOM_FACTOR; // 20% decrease
    setZoomAtPoint(newZoom, centerPos, documentState, documentImage,
                   canvasWidth, canvasHeight, scaleFactor, imageOffset, coordinateCache);
}

void CanvasZoomController::zoomReset(DocumentState* documentState,
                                     const QImage& documentImage,
                                     int canvasWidth, int canvasHeight,
                                     double& scaleFactor,
                                     QPointF& imageOffset,
                                     CanvasCoordinateCache* coordinateCache) {
    if (!documentState) return;
    
    // Reset zoom to center
    QPointF centerPos(canvasWidth / 2.0, canvasHeight / 2.0);
    setZoomAtPoint(1.0, centerPos, documentState, documentImage,
                   canvasWidth, canvasHeight, scaleFactor, imageOffset, coordinateCache);
}

void CanvasZoomController::setZoom(double zoomLevel,
                                  DocumentState* documentState,
                                  const QImage& documentImage,
                                  int canvasWidth, int canvasHeight,
                                  double& scaleFactor,
                                  QPointF& imageOffset,
                                  CanvasCoordinateCache* coordinateCache) {
    if (!documentState) return;
    
    // Validate zoom level (check for NaN/Infinity)
    if (!isValidZoom(zoomLevel)) {
        zoomLevel = CanvasConstants::DEFAULT_ZOOM;
    }
    
    // Clamp zoom level to bounds
    const double minZoom = CanvasConstants::MIN_ZOOM;
    const double maxZoom = CanvasConstants::MAX_ZOOM;
    zoomLevel = std::max(minZoom, std::min(maxZoom, zoomLevel));
    
    // Ensure zoom is never exactly 0.0
    if (zoomLevel <= 0.0) {
        zoomLevel = CanvasConstants::MIN_ZOOM;
    }
    
    // Update zoom level in DocumentState
    documentState->zoomLevel = zoomLevel;
    
    // Recalculate layout first (this updates scaleFactor and imageOffset)
    QRectF imageRect;
    calculateLayout(documentImage, canvasWidth, canvasHeight, documentState,
                   scaleFactor, imageOffset, imageRect);
    
    // Invalidate coordinate cache AFTER layout is recalculated
    if (coordinateCache) {
        coordinateCache->invalidate();
    }
}

void CanvasZoomController::setZoomAtPoint(double zoomLevel,
                                        const QPointF& widgetPos,
                                        DocumentState* documentState,
                                        const QImage& documentImage,
                                        int canvasWidth, int canvasHeight,
                                        double& scaleFactor,
                                        QPointF& imageOffset,
                                        CanvasCoordinateCache* coordinateCache) {
    if (!documentState || documentImage.isNull()) return;
    
    // Validate zoom level (check for NaN/Infinity)
    if (!isValidZoom(zoomLevel)) {
        zoomLevel = CanvasConstants::DEFAULT_ZOOM;
    }
    
    // Clamp zoom level to bounds
    const double minZoom = CanvasConstants::MIN_ZOOM;
    const double maxZoom = CanvasConstants::MAX_ZOOM;
    zoomLevel = std::max(minZoom, std::min(maxZoom, zoomLevel));
    
    // Ensure zoom is never exactly 0.0
    if (zoomLevel <= 0.0) {
        zoomLevel = CanvasConstants::MIN_ZOOM;
    }
    
    // Get current zoom and image dimensions
    double oldZoom = documentState->zoomLevel;
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    
    // Calculate base scale to fit
    double scaleX = static_cast<double>(canvasWidth) / static_cast<double>(imgWidth);
    double scaleY = static_cast<double>(canvasHeight) / static_cast<double>(imgHeight);
    double baseScale = std::min({scaleX, scaleY, 1.0});
    
    // Calculate old and new scale factors
    double oldScaleFactor = baseScale * oldZoom;
    double newScaleFactor = baseScale * zoomLevel;
    
    // Convert widget position to canvas coordinates (relative to current image position)
    QPointF canvasPos = widgetPos - imageOffset;
    
    // Convert canvas position to image coordinates (before zoom)
    QPointF imagePos(canvasPos.x() / oldScaleFactor, canvasPos.y() / oldScaleFactor);
    
    // Update zoom level
    documentState->zoomLevel = zoomLevel;
    
    // Recalculate layout with new zoom
    QRectF imageRect;
    calculateLayout(documentImage, canvasWidth, canvasHeight, documentState,
                   scaleFactor, imageOffset, imageRect);
    
    // Calculate where the image point should be in canvas coordinates after zoom
    QPointF newCanvasPos(imagePos.x() * newScaleFactor, imagePos.y() * newScaleFactor);
    
    // Adjust offset so the point under cursor stays in the same widget position
    QPointF newOffset = widgetPos - newCanvasPos;
    imageOffset = newOffset;
    
    // Update DocumentState
    documentState->imageOffset = imageOffset;
    documentState->scaleFactor = newScaleFactor;
    documentState->synchronizeCoordinates();
    
    // Recalculate layout again with new offset
    calculateLayout(documentImage, canvasWidth, canvasHeight, documentState,
                   scaleFactor, imageOffset, imageRect);
    
    // Invalidate coordinate cache AFTER layout is recalculated
    if (coordinateCache) {
        coordinateCache->invalidate();
    }
}

double CanvasZoomController::getZoom(DocumentState* documentState) const {
    return documentState ? documentState->zoomLevel : 1.0;
}

void CanvasZoomController::panWithWheel(const QPointF& delta,
                                       DocumentState* documentState,
                                       QPointF& imageOffset) {
    // Scroll/pan with wheel - inverted: scroll up moves down, scroll right moves left
    imageOffset += QPointF(delta.x() * CanvasConstants::SCROLL_SPEED,
                          delta.y() * CanvasConstants::SCROLL_SPEED);
    
    // Update DocumentState
    if (documentState) {
        documentState->imageOffset = imageOffset;
        documentState->synchronizeCoordinates();
    }
}

} // namespace ocr_orc

