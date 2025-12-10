#include "RegionData.h"
#include <QtCore/QStringList>

namespace ocr_orc {

bool RegionData::isValid() const {
    // Validate normalized coordinates (source of truth)
    if (!CoordinateSystem::isValidNormalized(normalizedCoords)) {
        return false;
    }
    
    // Validate name is not empty
    if (name.isEmpty()) {
        return false;
    }
    
    // Validate color is one of the allowed values
    QStringList validColors = {"blue", "red", "green", "yellow", "purple", "orange", "cyan"};
    if (!validColors.contains(color)) {
        return false;
    }
    
    return true;
}

void RegionData::syncFromNormalized(int imgWidth, int imgHeight,
                                    double scaleFactor,
                                    const QPointF& offset) {
    // Normalized is source of truth - calculate image and canvas from it
    imageCoords = CoordinateSystem::normalizedToImage(normalizedCoords, imgWidth, imgHeight);
    canvasCoords = CoordinateSystem::normalizedToCanvas(normalizedCoords, imgWidth, imgHeight, 
                                                       scaleFactor, offset);
}

void RegionData::syncFromCanvas(const CanvasCoords& canvas,
                                int imgWidth, int imgHeight,
                                double scaleFactor,
                                const QPointF& offset) {
    // User input is in canvas coordinates - convert to normalized (source of truth)
    canvasCoords = canvas;
    
    // Canvas → Image
    imageCoords = CoordinateSystem::canvasToImage(canvas, scaleFactor, offset);
    
    // Image → Normalized (source of truth)
    normalizedCoords = CoordinateSystem::imageToNormalized(imageCoords, imgWidth, imgHeight);
}

void RegionData::updateCanvasCoords(int imgWidth, int imgHeight,
                                     double scaleFactor,
                                     const QPointF& offset) {
    // Recalculate canvas coordinates from normalized (source of truth)
    canvasCoords = CoordinateSystem::normalizedToCanvas(normalizedCoords, imgWidth, imgHeight,
                                                        scaleFactor, offset);
}

void RegionData::updateImageCoords(int imgWidth, int imgHeight) {
    // Recalculate image coordinates from normalized (source of truth)
    imageCoords = CoordinateSystem::normalizedToImage(normalizedCoords, imgWidth, imgHeight);
}

QRectF RegionData::toQRectF() const {
    return QRectF(canvasCoords.x1, canvasCoords.y1,
                  canvasCoords.x2 - canvasCoords.x1,
                  canvasCoords.y2 - canvasCoords.y1);
}

QRect RegionData::toQRect() const {
    return QRect(imageCoords.x1, imageCoords.y1,
                 imageCoords.x2 - imageCoords.x1,
                 imageCoords.y2 - imageCoords.y1);
}

} // namespace ocr_orc

