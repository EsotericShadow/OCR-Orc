#include "CanvasMouseHandler.h"
#include "../core/coordinate/CanvasHitTester.h"
#include "../core/selection/CanvasSelectionManager.h"
#include "../core/regions/CanvasRegionCreator.h"
#include "../core/regions/CanvasRegionOperations.h"
#include "../../../models/RegionData.h"
#include "../../../core/CoordinateSystem.h"
#include "../../../core/Constants.h"
#include <QtGui/QImage>
#include <cmath>

namespace ocr_orc {

void CanvasMouseHandler::handlePress(QMouseEvent* event,
                                    MouseMode mouseMode,
                                    bool isRotateMode,
                                    DocumentState* documentState,
                                    const QImage& documentImage,
                                    const QRectF& imageRect,
                                    double scaleFactor,
                                    const QPointF& imageOffset,
                                    QSet<QString>& selectedRegions,
                                    QString& primarySelectedRegion,
                                    bool& isCreating,
                                    QPointF& creationStartPos,
                                    QRectF& tempRect,
                                    bool& isBoxSelecting,
                                    QPointF& boxSelectionStart,
                                    QRectF& selectionBox,
                                    bool& isDragging,
                                    QPointF& dragStartPos,
                                    QPointF& lastDragPos,
                                    QMap<QString, RegionData>& dragStartState,
                    bool& isResizing,
                    QString& resizingRegion,
                    QString& resizeHandle,
                    QRectF& resizeStartRect,
                    RegionData& resizeStartRegionData,
                    bool& isRotating,
                    QString& rotatingRegion,
                    QPointF& rotationCenter,
                    double& rotationStartAngle,
                    double& rotationAngle,
                    bool& isPanning,
                                    QPointF& panStartPos,
                                    QPointF& panStartOffset,
                                    QPointF& imageOffsetRef,
                                    CanvasHitTester* hitTester,
                                    CanvasSelectionManager* selectionManager,
                                    CanvasRegionCreator* regionCreator,
                                    const WidgetToCanvasCallback& widgetToCanvas,
                                    const UpdateCallback& update,
                                    const SetCursorCallback& setCursor,
                                    const CalculateLayoutCallback& calculateLayout,
                                    const EmitSignalCallback& emitRegionCreationRequested) {
    // Note: imageOffsetRef, calculateLayout, and emitRegionCreationRequested are used in other functions
    // but not in handlePress - they're part of the interface for consistency
    Q_UNUSED(imageOffsetRef);
    Q_UNUSED(calculateLayout);
    Q_UNUSED(emitRegionCreationRequested);
    
    // DISABLED FOR PRODUCTION: Debug logging commented out for performance
    // OCR_ORC_DEBUG("=== mousePressEvent ===");
    // OCR_ORC_DEBUG("  button=" << event->button() << "modifiers=" << event->modifiers());
    // OCR_ORC_DEBUG("  mouseMode=" << (mouseMode == Create ? "Create" : "Select"));
    
    if (!documentState || documentImage.isNull()) {
        // OCR_ORC_DEBUG("  No document state or image - returning");
        resetAllFlags(isCreating, isBoxSelecting, isDragging, isResizing, isRotating, isPanning);
        return;
    }
    
    // Check for pan (middle-click or Alt+left-click)
    bool isMiddleButton = (event->button() == Qt::MiddleButton);
    bool isAltPressed = (event->modifiers() & Qt::AltModifier);
    
    if (isMiddleButton || (event->button() == Qt::LeftButton && isAltPressed)) {
        isPanning = true;
        panStartPos = event->pos();
        panStartOffset = imageOffset;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    
    QPointF canvasPos = widgetToCanvas(event->pos());
    // OCR_ORC_DEBUG("  widgetPos=" << event->pos() << "canvasPos=" << canvasPos);
    
    if (!imageRect.contains(canvasPos)) {
        // OCR_ORC_DEBUG("  Click outside image bounds - returning");
        return;
    }
    
    if (mouseMode == Create) {
        if (regionCreator) {
            if (regionCreator->startRegionCreation(canvasPos, imageRect, isCreating, creationStartPos, tempRect)) {
                update();
            }
        }
    } else if (mouseMode == Select) {
        // Check for rotate icon first (if rotate mode is active)
        if (isRotateMode && !primarySelectedRegion.isEmpty() && selectedRegions.size() == 1 && hitTester) {
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
            
            OCR_ORC_DEBUG("[RESIZE/ROTATE] === Checking rotate icon hit ===");
            OCR_ORC_DEBUG("  region: " << primarySelectedRegion);
            OCR_ORC_DEBUG("  region.rotationAngle: " << region.rotationAngle << "degrees");
            
            if (hitTester->isOnRotateIcon(canvasPos, canvasRect, 24.0, region.rotationAngle)) {
                if (documentState) {
                    documentState->saveState();
                }
                
                // Calculate rotation center (center of region)
                QPointF center = canvasRect.center();
                
                // Calculate initial angle from center to mouse position
                QPointF delta = canvasPos - center;
                double angle = std::atan2(delta.y(), delta.x()) * 180.0 / M_PI;
                
                OCR_ORC_DEBUG("=== Rotation START ===");
                OCR_ORC_DEBUG("  primarySelectedRegion:" << primarySelectedRegion);
                OCR_ORC_DEBUG("  center:" << center.x() << "," << center.y());
                OCR_ORC_DEBUG("  canvasPos:" << canvasPos.x() << "," << canvasPos.y());
                OCR_ORC_DEBUG("  delta:" << delta.x() << "," << delta.y());
                OCR_ORC_DEBUG("  initial angle:" << angle << "degrees");
                
                isRotating = true;
                rotatingRegion = primarySelectedRegion;
                rotationCenter = center;
                rotationStartAngle = angle;
                rotationAngle = 0.0; // Will be updated during drag
                event->accept();
                return;
            }
        }
        
        // Check for resize handle (only if rotate mode is NOT active)
        if (!isRotateMode && !primarySelectedRegion.isEmpty() && selectedRegions.size() == 1 && hitTester) {
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
            
            OCR_ORC_DEBUG("[RESIZE/ROTATE] === Checking resize handle hit ===");
            OCR_ORC_DEBUG("  region: " << primarySelectedRegion);
            OCR_ORC_DEBUG("  region.rotationAngle: " << region.rotationAngle << "degrees");
            OCR_ORC_DEBUG("  canvasPos: (" << canvasPos.x() << "," << canvasPos.y() << ")");
            OCR_ORC_DEBUG("  canvasRect: (" << canvasRect.left() << "," << canvasRect.top() << ") size: " << canvasRect.width() << "x" << canvasRect.height());
            
            QString handle = hitTester->getHandleAt(canvasPos, canvasRect, region.rotationAngle);
            if (!handle.isEmpty()) {
                OCR_ORC_DEBUG("[RESIZE PRESS] === Resize PRESS ===");
                OCR_ORC_DEBUG("[RESIZE PRESS]   Hit resize handle: " << handle);
                OCR_ORC_DEBUG("[RESIZE PRESS]   primarySelectedRegion: " << primarySelectedRegion);
                if (documentState) {
                    // Save state BEFORE resize begins (one snapshot per gesture)
                    documentState->saveState();
                    resizeStartRegionData = documentState->getRegion(primarySelectedRegion);
                    OCR_ORC_DEBUG("[RESIZE PRESS]   resizeStartRegionData:");
                    OCR_ORC_DEBUG("[RESIZE PRESS]     normalizedCoords: x1=" << resizeStartRegionData.normalizedCoords.x1 << " y1=" << resizeStartRegionData.normalizedCoords.y1 << " x2=" << resizeStartRegionData.normalizedCoords.x2 << " y2=" << resizeStartRegionData.normalizedCoords.y2);
                    OCR_ORC_DEBUG("[RESIZE PRESS]     rotationAngle: " << resizeStartRegionData.rotationAngle << "degrees");
                }
                
                isResizing = true;
                resizingRegion = primarySelectedRegion;
                resizeHandle = handle;
                resizeStartRect = canvasRect;
                OCR_ORC_DEBUG("[RESIZE PRESS]   resizeStartRect (canvas): (" << resizeStartRect.left() << "," << resizeStartRect.top() << ") size: " << resizeStartRect.width() << "x" << resizeStartRect.height());
                OCR_ORC_DEBUG("[RESIZE PRESS]   Resize state set: resizingRegion=" << resizingRegion << " handle=" << resizeHandle);
                event->accept();
                return;
            } else {
                OCR_ORC_DEBUG("  ✗ No resize handle hit");
            }
        }
        
        handleSelectModePress(event, canvasPos, documentState, documentImage, imageRect,
                             scaleFactor, imageOffset, selectedRegions, primarySelectedRegion,
                             isBoxSelecting, boxSelectionStart, selectionBox, isDragging, dragStartPos,
                             lastDragPos, dragStartState, hitTester, selectionManager, setCursor, widgetToCanvas);
    }
    
    event->accept();
}

void CanvasMouseHandler::handleSelectModePress(QMouseEvent* event,
                                              const QPointF& canvasPos,
                                              DocumentState* documentState,
                                              const QImage& documentImage,
                                              const QRectF& imageRect,
                                              double scaleFactor,
                                              const QPointF& imageOffset,
                                              QSet<QString>& selectedRegions,
                                              QString& primarySelectedRegion,
                                              bool& isBoxSelecting,
                                              QPointF& boxSelectionStart,
                                              QRectF& selectionBox,
                                              bool& isDragging,
                                              QPointF& dragStartPos,
                                              QPointF& lastDragPos,
                                              QMap<QString, RegionData>& dragStartState,
                                              CanvasHitTester* hitTester,
                                              CanvasSelectionManager* selectionManager,
                                              const SetCursorCallback& setCursor,
                                              const WidgetToCanvasCallback& widgetToCanvas) {
    // Mark unused parameters to suppress warnings
    Q_UNUSED(widgetToCanvas);
    if (!hitTester || !selectionManager) {
        return;
    }
    
    QString regionName = hitTester->getRegionAt(canvasPos, documentState, documentImage,
                                                imageRect, scaleFactor, imageOffset);
    
    bool isModifierPressed = (event->modifiers() & Qt::ControlModifier) || 
                              (event->modifiers() & Qt::MetaModifier);
    bool isShiftPressed = (event->modifiers() & Qt::ShiftModifier);
    
    if (regionName.isEmpty()) {
        if (isShiftPressed) {
            isBoxSelecting = true;
            boxSelectionStart = canvasPos;
            selectionBox = QRectF();
        } else {
            selectionManager->clearSelection(selectedRegions, primarySelectedRegion);
            isBoxSelecting = true;
            boxSelectionStart = canvasPos;
            selectionBox = QRectF();
        }
    } else {
        if (isModifierPressed) {
            selectionManager->toggleRegionSelection(regionName, selectedRegions, primarySelectedRegion);
        } else if (isShiftPressed) {
            if (selectedRegions.contains(regionName)) {
                if (documentState) {
                    documentState->saveState();
                    dragStartState.clear();
                    for (const QString& name : selectedRegions) {
                        dragStartState[name] = documentState->getRegion(name);
                    }
                }
                
                isDragging = true;
                dragStartPos = canvasPos;
                lastDragPos = canvasPos;
                setCursor(Qt::ClosedHandCursor);
                return;
            } else {
                selectionManager->selectRegion(regionName, selectedRegions, primarySelectedRegion);
            }
        } else {
            selectionManager->selectRegion(regionName, selectedRegions, primarySelectedRegion);
        }
    }
}

} // namespace ocr_orc


