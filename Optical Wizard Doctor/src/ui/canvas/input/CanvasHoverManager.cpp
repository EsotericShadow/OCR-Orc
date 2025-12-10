#include "CanvasHoverManager.h"
#include "../core/coordinate/CanvasHitTester.h"
#include "../../../models/RegionData.h"
#include "../../../core/CoordinateSystem.h"
#include <QtGui/QCursor>

namespace ocr_orc {

CanvasHoverManager::CanvasHoverManager() {
}

CanvasHoverManager::~CanvasHoverManager() {
}

void CanvasHoverManager::updateHoverState(const QPointF& canvasPos,
                                         MouseMode mouseMode,
                                         bool isRotateMode,
                                         DocumentState* documentState,
                                         const QImage& documentImage,
                                         const QRectF& imageRect,
                                         double scaleFactor,
                                         const QPointF& imageOffset,
                                         QSet<QString>& selectedRegions,
                                         QString& primarySelectedRegion,
                                         QString& hoveredRegion,
                                         CanvasHitTester* hitTester,
                                         const UpdateCallback& update,
                                         const SetCursorCallback& setCursor) {
    if (mouseMode != Select) {
        if (!hoveredRegion.isEmpty()) {
            hoveredRegion = QString();
            update();
        }
        return;
    }
    
    // Check if over a resize handle first
    if (!primarySelectedRegion.isEmpty() && selectedRegions.size() == 1 && hitTester) {
        RegionData region = documentState->getRegion(primarySelectedRegion);
        int imgWidth = documentImage.width();
        int imgHeight = documentImage.height();
        NormalizedCoords norm = region.normalizedCoords;
        CanvasCoords canvasCoords = CoordinateSystem::normalizedToCanvas(
            norm, imgWidth, imgHeight, scaleFactor, imageOffset
        );
        QRectF canvasRect(
            canvasCoords.x1, canvasCoords.y1,
            canvasCoords.x2 - canvasCoords.x1,
            canvasCoords.y2 - canvasCoords.y1
        );
        
        QString handle = hitTester->getHandleAt(canvasPos, canvasRect);
        if (!handle.isEmpty()) {
            if (!hoveredRegion.isEmpty()) {
                hoveredRegion = QString();
                update();
            }
            // Set cursor based on handle type
            if (handle == "nw" || handle == "se") {
                setCursor(Qt::SizeFDiagCursor);  // Diagonal cursor for NW/SE
            } else if (handle == "ne" || handle == "sw") {
                setCursor(Qt::SizeBDiagCursor);  // Diagonal cursor for NE/SW
            } else if (handle == "n" || handle == "s") {
                setCursor(Qt::SizeVerCursor);    // Vertical cursor for N/S
            } else if (handle == "e" || handle == "w") {
                setCursor(Qt::SizeHorCursor);    // Horizontal cursor for E/W
            }
            return;
        }
    }
    
    // Find region under cursor
    QString newHovered = hitTester->getRegionAt(canvasPos, documentState, documentImage,
                                                imageRect, scaleFactor, imageOffset);
    
    if (newHovered != hoveredRegion) {
        hoveredRegion = newHovered;
        update();
    }
    
    // Set cursor based on hover state
    if (!newHovered.isEmpty()) {
        setCursor(Qt::OpenHandCursor);  // Open hand when hovering over region
    } else {
        setCursor(Qt::ArrowCursor);     // Arrow cursor otherwise
    }
}

} // namespace ocr_orc

