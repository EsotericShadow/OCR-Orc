#include "CanvasMouseHandler.h"
#include "../core/coordinate/CanvasHitTester.h"
#include "../core/selection/CanvasSelectionManager.h"
#include "../core/regions/CanvasRegionCreator.h"
#include "../core/regions/CanvasRegionOperations.h"
#include "../../../core/CoordinateSystem.h"
#include "../../../core/Constants.h"
#include <QtGui/QImage>
#include <cmath>

namespace ocr_orc {

void CanvasMouseHandler::handleMove(QMouseEvent* event,
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
                                   bool isCreating,
                                   QPointF& creationStartPos,
                                   QRectF& tempRect,
                                   bool& isBoxSelecting,
                                   QPointF& boxSelectionStart,
                                   QRectF& selectionBox,
                                   bool isDragging,
                                   QPointF& lastDragPos,
                                   bool isResizing,
                                   QString& resizingRegion,
                                   QString& resizeHandle,
                                   QRectF& resizeStartRect,
                                   bool& isRotating,
                                   QString& rotatingRegion,
                                   QPointF& rotationCenter,
                                   double& rotationStartAngle,
                                   double& rotationAngle,
                                   bool isPanning,
                                   QPointF& panStartPos,
                                   QPointF& panStartOffset,
                                   QPointF& imageOffsetRef,
                                   CanvasHitTester* hitTester,
                                   CanvasSelectionManager* selectionManager,
                                   CanvasRegionCreator* regionCreator,
                                   CanvasRegionOperations* regionOperations,
                                   const WidgetToCanvasCallback& widgetToCanvas,
                                   const MoveRegionCallback& moveRegion,
                                   const MoveSelectedRegionsCallback& moveSelectedRegions,
                                   const ResizeRegionCallback& resizeRegion,
                                   const UpdateCallback& update,
                                   const CalculateLayoutCallback& calculateLayout,
                                   const SetCursorCallback& setCursor) {
    if (!documentState || documentImage.isNull()) {
        resetAllFlags(isCreating, isBoxSelecting, isDragging, isResizing, isRotating, isPanning);
        return;
    }
    
    QPointF canvasPos = widgetToCanvas(event->pos());
    
    // Update cursor based on operation state
    if (isResizing) {
        // Set appropriate resize cursor based on handle
        if (resizeHandle.contains("top") || resizeHandle.contains("bottom")) {
            setCursor(Qt::SizeVerCursor);
        } else if (resizeHandle.contains("left") || resizeHandle.contains("right")) {
            setCursor(Qt::SizeHorCursor);
        } else if (resizeHandle.contains("top-left") || resizeHandle.contains("bottom-right")) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (resizeHandle.contains("top-right") || resizeHandle.contains("bottom-left")) {
            setCursor(Qt::SizeBDiagCursor);
        }
    } else if (isRotating) {
        setCursor(Qt::ClosedHandCursor);
    } else if (isPanning) {
        setCursor(Qt::ClosedHandCursor);
    } else if (isDragging) {
        setCursor(Qt::ClosedHandCursor);
    }
    
    // Note: The following parameters are part of the interface but not used in handleMove:
    // - isRotateMode: Mode is checked via isRotating flag instead
    // - scaleFactor, imageOffset: Coordinate conversion handled by widgetToCanvas callback
    // - selectedRegions, primarySelectedRegion: Selection state managed elsewhere
    // - hoveredRegion: Hover state updated separately in CanvasInputHandler after handleMove
    // - resizeStartRect: Set in handlePress but resize logic uses resizeRegion callback
    // - hitTester, selectionManager, regionOperations: Used in other handlers, not needed here
    // - moveRegion: Single region moves handled via moveSelectedRegions callback
    Q_UNUSED(isRotateMode);
    Q_UNUSED(scaleFactor);
    Q_UNUSED(imageOffset);
    Q_UNUSED(selectedRegions);
    Q_UNUSED(primarySelectedRegion);
    Q_UNUSED(hoveredRegion);
    Q_UNUSED(resizeStartRect);
    Q_UNUSED(hitTester);
    Q_UNUSED(selectionManager);
    Q_UNUSED(regionOperations);
    Q_UNUSED(moveRegion);
    
    if (mouseMode == Create && isCreating) {
        if (regionCreator) {
            regionCreator->updateRegionCreation(canvasPos, isCreating, creationStartPos, imageRect, tempRect);
            update();
        }
    } else if (mouseMode == Select) {
        // Update box selection if active
        if (isBoxSelecting) {
            QRectF newBox(
                std::min(boxSelectionStart.x(), canvasPos.x()),
                std::min(boxSelectionStart.y(), canvasPos.y()),
                std::abs(canvasPos.x() - boxSelectionStart.x()),
                std::abs(canvasPos.y() - boxSelectionStart.y())
            );
            selectionBox = newBox;
            update();
        } else if (isDragging) {
            QPointF delta = canvasPos - lastDragPos;
            // DISABLED FOR PRODUCTION: Verbose drag logging
            // OCR_ORC_DEBUG("=== Move Drag ===");
            // OCR_ORC_DEBUG("  canvasPos:" << canvasPos);
            // OCR_ORC_DEBUG("  lastDragPos:" << lastDragPos);
            // OCR_ORC_DEBUG("  delta:" << delta);
            if (delta.manhattanLength() > 0.1) {
                moveSelectedRegions(delta);
                lastDragPos = canvasPos;
            }
        } else if (isRotating && !rotatingRegion.isEmpty()) {
            // Calculate current angle from center to mouse position
            QPointF delta = canvasPos - rotationCenter;
            double currentAngle = std::atan2(delta.y(), delta.x()) * 180.0 / M_PI;
            
            // Calculate rotation angle (difference from start)
            double rawAngle = currentAngle - rotationStartAngle;
            OCR_ORC_DEBUG("=== Rotation MOVE ===");
            OCR_ORC_DEBUG("  rotatingRegion:" << rotatingRegion);
            OCR_ORC_DEBUG("  canvasPos:" << canvasPos.x() << "," << canvasPos.y());
            OCR_ORC_DEBUG("  rotationCenter:" << rotationCenter.x() << "," << rotationCenter.y());
            OCR_ORC_DEBUG("  delta:" << delta.x() << "," << delta.y());
            OCR_ORC_DEBUG("  currentAngle:" << currentAngle << "degrees");
            OCR_ORC_DEBUG("  rotationStartAngle:" << rotationStartAngle << "degrees");
            OCR_ORC_DEBUG("  rawAngle (before normalize):" << rawAngle << "degrees");
            
            rotationAngle = rawAngle;
            
            // Normalize to -180 to 180 range
            while (rotationAngle > 180.0) rotationAngle -= 360.0;
            while (rotationAngle < -180.0) rotationAngle += 360.0;
            
            OCR_ORC_DEBUG("  rotationAngle (after normalize):" << rotationAngle << "degrees");
            
            update();
        } else if (isResizing) {
            OCR_ORC_DEBUG("[RESIZE MOVE] === Resize MOVE ===");
            OCR_ORC_DEBUG("[RESIZE MOVE]   resizingRegion: " << resizingRegion);
            OCR_ORC_DEBUG("[RESIZE MOVE]   resizeHandle: " << resizeHandle);
            OCR_ORC_DEBUG("[RESIZE MOVE]   canvasPos: (" << canvasPos.x() << "," << canvasPos.y() << ")");
            OCR_ORC_DEBUG("[RESIZE MOVE]   resizeStartRect: (" << resizeStartRect.left() << "," << resizeStartRect.top() << ") size: " << resizeStartRect.width() << "x" << resizeStartRect.height());
            resizeRegion(resizingRegion, resizeHandle, canvasPos);
        } else if (isPanning) {
            QPointF delta = event->pos() - panStartPos;
            QPointF newOffset = panStartOffset - delta;
            
            // Validate offset values (check for NaN/Infinity and reasonable bounds)
            if (CoordinateSystem::isValidPoint(newOffset) &&
                std::abs(newOffset.x()) < 100000.0 && std::abs(newOffset.y()) < 100000.0) {
                imageOffsetRef = newOffset;
                
                if (documentState) {
                    documentState->imageOffset = imageOffsetRef;
                    documentState->synchronizeCoordinates();
                }
                
                calculateLayout();
                update();
            }
        }
    }
    
    event->accept();
}

} // namespace ocr_orc


