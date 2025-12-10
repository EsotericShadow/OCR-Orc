#include "CanvasRegionCreator.h"
#include "../../../../models/RegionData.h"
#include <algorithm>

namespace ocr_orc {

CanvasRegionCreator::CanvasRegionCreator() {
}

CanvasRegionCreator::~CanvasRegionCreator() {
}

bool CanvasRegionCreator::startRegionCreation(const QPointF& pos,
                                              const QRectF& imageRect,
                                              bool& isCreating,
                                              QPointF& creationStartPos,
                                              QRectF& tempRect) {
    // Only allow creation within image bounds
    if (!imageRect.contains(pos)) {
        return false;
    }
    
    isCreating = true;
    creationStartPos = pos;
    tempRect = QRectF();
    return true;
}

void CanvasRegionCreator::updateRegionCreation(const QPointF& pos,
                                               bool isCreating,
                                               const QPointF& creationStartPos,
                                               const QRectF& imageRect,
                                               QRectF& tempRect) {
    if (!isCreating) {
        return;
    }
    
    // Calculate rectangle from start to current position
    QRectF newRect(
        std::min(creationStartPos.x(), pos.x()),
        std::min(creationStartPos.y(), pos.y()),
        std::abs(pos.x() - creationStartPos.x()),
        std::abs(pos.y() - creationStartPos.y())
    );
    
    // Clamp to image bounds (simple intersection)
    newRect = newRect.intersected(imageRect);
    
    tempRect = newRect;
}

bool CanvasRegionCreator::finishRegionCreation(const QString& regionName,
                                               const QString& color,
                                               const QString& group,
                                               const QString& shapeType,
                                               bool& isCreating,
                                               QRectF& tempRect,
                                               DocumentState* documentState,
                                               const QImage& documentImage,
                                               double scaleFactor,
                                               const QPointF& imageOffset,
                                               std::function<bool(const QRectF&, const QRectF&)> validateRegion) {
    if (!isCreating || !documentState || tempRect.isEmpty()) {
        return false;
    }
    
    // Use the temporary rectangle that was calculated during drag
    QRectF finalRect = tempRect;
    
    // Validate region (need imageRect for validation)
    QRectF imageRect(0, 0, documentImage.width() * scaleFactor, documentImage.height() * scaleFactor);
    imageRect.translate(imageOffset);
    
    if (!validateRegion(finalRect, imageRect)) {
        isCreating = false;
        tempRect = QRectF();
        return false;
    }
    
    // Validate name
    if (regionName.isEmpty() || documentState->hasRegion(regionName)) {
        isCreating = false;
        tempRect = QRectF();
        return false;
    }
    
    // Convert canvas coordinates to image coordinates
    CanvasCoords canvasCoords(
        finalRect.left(),
        finalRect.top(),
        finalRect.right(),
        finalRect.bottom()
    );
    
    ImageCoords imageCoords = CoordinateSystem::canvasToImage(
        canvasCoords, scaleFactor, imageOffset
    );
    
    // Convert image coordinates to normalized
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    NormalizedCoords normalizedCoords = CoordinateSystem::imageToNormalized(
        imageCoords, imgWidth, imgHeight
    );
    
    // Save state BEFORE adding region (for undo)
    documentState->saveState();
    
    // Create RegionData with shape type
    RegionData region(regionName, normalizedCoords, color, group, shapeType);
    region.imageCoords = imageCoords;
    region.canvasCoords = canvasCoords;
    
    // Add to DocumentState
    documentState->addRegion(regionName, region);
    
    // Clear creation state
    isCreating = false;
    tempRect = QRectF();
    
    return true;
}

} // namespace ocr_orc

