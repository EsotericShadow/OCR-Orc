#include "CanvasInputHandler.h"
// Include full definitions for handler classes (needed for new/delete and method calls)
#include "CanvasMouseHandler.h"
#include "CanvasKeyboardHandler.h"
#include "CanvasWheelHandler.h"
#include "CanvasHoverManager.h"
#include "../core/coordinate/CanvasHitTester.h"
#include "../core/selection/CanvasSelectionManager.h"
#include "../core/regions/CanvasRegionCreator.h"
#include "../core/zoom/CanvasZoomController.h"
#include "../core/regions/CanvasRegionOperations.h"
#include "../../../models/RegionData.h"
#include "../../../models/DocumentState.h"
#include <QtGui/QImage>

namespace ocr_orc {

CanvasInputHandler::CanvasInputHandler()
    : mouseHandler(new CanvasMouseHandler())
    , keyboardHandler(new CanvasKeyboardHandler())
    , wheelHandler(new CanvasWheelHandler())
    , hoverManager(new CanvasHoverManager())
{
}

CanvasInputHandler::~CanvasInputHandler() {
    delete mouseHandler;
    delete keyboardHandler;
    delete wheelHandler;
    delete hoverManager;
}

void CanvasInputHandler::handleMousePress(QMouseEvent* event,
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
                                         CanvasRegionOperations* regionOperations,
                                         const std::function<QPointF(const QPointF&)>& widgetToCanvas,
                                         const UpdateCallback& update,
                                         const SetCursorCallback& setCursor,
                                         const CalculateLayoutCallback& calculateLayout,
                                         const EmitSignalCallback& emitRegionCreationRequested) {
    if (!mouseHandler) return;
    
    mouseHandler->handlePress(event, mouseMode, isRotateMode, documentState, documentImage, imageRect,
                              scaleFactor, imageOffset, selectedRegions, primarySelectedRegion,
                              isCreating, creationStartPos, tempRect, isBoxSelecting, boxSelectionStart,
                              selectionBox, isDragging, dragStartPos, lastDragPos, dragStartState,
                              isResizing, resizingRegion, resizeHandle, resizeStartRect,
                              resizeStartRegionData, isRotating, rotatingRegion, rotationCenter,
                              rotationStartAngle, rotationAngle, isPanning, panStartPos, panStartOffset,
                              imageOffsetRef, hitTester, selectionManager, regionCreator,
                              widgetToCanvas, update, setCursor, calculateLayout, emitRegionCreationRequested);
}

void CanvasInputHandler::handleMouseMove(QMouseEvent* event,
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
                                        const std::function<QPointF(const QPointF&)>& widgetToCanvas,
                                        const std::function<void(const QString&, const QPointF&)>& moveRegion,
                                        const std::function<void(const QPointF&)>& moveSelectedRegions,
                                        const std::function<void(const QString&, const QString&, const QPointF&)>& resizeRegion,
                                        const UpdateCallback& update,
                                        const CalculateLayoutCallback& calculateLayout,
                                        const SetCursorCallback& setCursor) {
    if (!mouseHandler) return;
    
    mouseHandler->handleMove(event, mouseMode, isRotateMode, documentState, documentImage, imageRect,
                            scaleFactor, imageOffset, selectedRegions, primarySelectedRegion,
                            hoveredRegion, isCreating, creationStartPos, tempRect, isBoxSelecting,
                            boxSelectionStart, selectionBox, isDragging, lastDragPos, isResizing,
                            resizingRegion, resizeHandle, resizeStartRect, isRotating, rotatingRegion,
                            rotationCenter, rotationStartAngle, rotationAngle, isPanning, panStartPos, panStartOffset,
                            imageOffsetRef, hitTester, selectionManager, regionCreator,
                            regionOperations, widgetToCanvas, moveRegion, moveSelectedRegions,
                            resizeRegion, update, calculateLayout, setCursor);
    
    // Update hover state if not dragging/resizing
    if (!isDragging && !isResizing && hoverManager) {
        QPointF canvasPos = widgetToCanvas(event->pos());
        bool isShiftPressed = (event->modifiers() & Qt::ShiftModifier);
        hoverManager->updateHoverState(canvasPos, mouseMode, isRotateMode, documentState, documentImage,
                                      imageRect, scaleFactor, imageOffset, selectedRegions,
                                      primarySelectedRegion, hoveredRegion, hitTester, update,
                                      [setCursor, isShiftPressed](Qt::CursorShape shape) {
                                          // If shift is pressed and hovering over region, show closed hand
                                          if (isShiftPressed && shape == Qt::OpenHandCursor) {
                                              setCursor(Qt::ClosedHandCursor);
                                          } else {
                                              setCursor(shape);
                                          }
                                      });
    }
}

void CanvasInputHandler::handleMouseRelease(QMouseEvent* event,
                                           MouseMode mouseMode,
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
                                           QPointF& lastDragPos,
                                           QMap<QString, RegionData>& dragStartState,
                                           bool& isResizing,
                                           QString& resizingRegion,
                                           QString& resizeHandle,
                           RegionData& resizeStartRegionData,
                           bool& isRotating,
                           QString& rotatingRegion,
                           double& rotationStartAngle,
                           double& rotationAngle,
                           bool& isPanning,
                                           QPointF& panStartPos,
                                           QPointF& panStartOffset,
                                           QPointF& imageOffsetRef,
                                           CanvasSelectionManager* selectionManager,
                                           CanvasRegionCreator* regionCreator,
                                           CanvasRegionOperations* regionOperations,
                                           const std::function<QPointF(const QPointF&)>& widgetToCanvas,
                                           const std::function<bool(const QRectF&)>& validateRegion,
                                           const std::function<QSet<QString>(const QRectF&)>& findRegionsInBox,
                                           const std::function<void(const QString&, const QPointF&)>& moveRegion,
                                           const std::function<void(const QPointF&)>& moveSelectedRegions,
                                           const std::function<void(const QString&, const QString&, const QPointF&)>& resizeRegion,
                                           const UpdateCallback& update,
                                           const CalculateLayoutCallback& calculateLayout,
                                           const SetCursorCallback& setCursor,
                                           const EmitSignalCallback& emitRegionCreationRequested,
                                           const EmitStateChangedCallback& emitStateChanged) {
    if (!mouseHandler) return;
    
    mouseHandler->handleRelease(event, mouseMode, documentState, documentImage, imageRect,
                               scaleFactor, imageOffset, selectedRegions, primarySelectedRegion,
                               isCreating, creationStartPos, tempRect, isBoxSelecting, boxSelectionStart,
                               selectionBox, isDragging, lastDragPos, dragStartState, isResizing,
                               resizingRegion, resizeHandle, resizeStartRegionData, isRotating, rotatingRegion, rotationStartAngle, rotationAngle, isPanning,
                               panStartPos, panStartOffset, imageOffsetRef, selectionManager,
                               regionCreator, regionOperations, widgetToCanvas, validateRegion, findRegionsInBox,
                               moveSelectedRegions, resizeRegion, update, calculateLayout,
                               setCursor, emitRegionCreationRequested, emitStateChanged);
}

void CanvasInputHandler::handleMouseDoubleClick(QMouseEvent* event,
                                               MouseMode mouseMode,
                                               DocumentState* documentState,
                                               const QImage& documentImage,
                                               const QRectF& imageRect,
                                               double scaleFactor,
                                               const QPointF& imageOffset,
                                               CanvasHitTester* hitTester,
                                               const std::function<QPointF(const QPointF&)>& widgetToCanvas,
                                               const EmitSignalCallback& emitRegionEditRequested) {
    if (!mouseHandler) return;
    
    mouseHandler->handleDoubleClick(event, mouseMode, documentState, documentImage, imageRect,
                                   scaleFactor, imageOffset, hitTester, widgetToCanvas, emitRegionEditRequested);
}

void CanvasInputHandler::handleKeyPress(QKeyEvent* event,
                                       DocumentState* documentState,
                                       QSet<QString>& selectedRegions,
                                       bool& isDragging,
                                       QMap<QString, RegionData>& dragStartState,
                                       bool& isResizing,
                                       QString& resizingRegion,
                                       QString& resizeHandle,
                                       RegionData& resizeStartRegionData,
                                       MouseMode& mouseMode,
                                       const std::function<void()>& duplicateSelectedRegions,
                                       const std::function<void()>& zoomIn,
                                       const std::function<void()>& zoomOut,
                                       const std::function<void()>& zoomReset,
                                       const std::function<void(MouseMode)>& setMode,
                                       const UpdateCallback& update,
                                       const SetCursorCallback& setCursor,
                                       const EmitSignalCallback& emitUndoRequested,
                                       const EmitSignalCallback& emitRedoRequested,
                                       const EmitStateChangedCallback& emitStateChanged) {
    if (!keyboardHandler) return;
    
    keyboardHandler->handleKeyPress(event, documentState, selectedRegions, isDragging,
                                   dragStartState, isResizing, resizingRegion, resizeHandle,
                                   resizeStartRegionData, mouseMode, duplicateSelectedRegions,
                                   zoomIn, zoomOut, zoomReset, setMode, update, setCursor,
                                   emitUndoRequested, emitRedoRequested, emitStateChanged);
}

void CanvasInputHandler::handleWheel(QWheelEvent* event,
                                    DocumentState* documentState,
                                    const QImage& documentImage,
                                    double& scaleFactor,
                                    QPointF& imageOffset,
                                    CanvasZoomController* zoomController,
                                    const std::function<void()>& calculateLayout,
                                    const UpdateCallback& update) {
    if (!wheelHandler) return;
    
    wheelHandler->handleWheel(event, documentState, documentImage, scaleFactor, imageOffset,
                             zoomController, calculateLayout, update);
}

void CanvasInputHandler::handleContextMenu(QContextMenuEvent* event,
                                          DocumentState* documentState,
                                          const QImage& documentImage,
                                          const QRectF& imageRect,
                                          double scaleFactor,
                                          const QPointF& imageOffset,
                                          QSet<QString>& selectedRegions,
                                          CanvasHitTester* hitTester,
                                          const std::function<QPointF(const QPointF&)>& widgetToCanvas,
                                          const std::function<void(const QString&)>& selectRegion,
                                          const std::function<QList<QString>()>& duplicateSelectedRegions,
                                          const EmitSignalCallback& emitRegionEditRequested,
                                          const EmitSignalCallback& emitRegionColorChangeRequested,
                                          const EmitRegionsDuplicatedCallback& emitRegionsDuplicated) {
    // Context menu is handled in Canvas::contextMenuEvent
    // This method is kept for compatibility but does nothing
}

void CanvasInputHandler::updateHoverState(const QPointF& canvasPos,
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
                                         const UpdateCallback& update) {
    if (!hoverManager) return;
    
    // Note: setCursor callback needs to be provided by Canvas, but this method
    // is called from Canvas which doesn't have access to setCursor here.
    // The cursor is set in the mouse move handler instead.
    hoverManager->updateHoverState(canvasPos, mouseMode, isRotateMode, documentState, documentImage,
                                  imageRect, scaleFactor, imageOffset, selectedRegions,
                                  primarySelectedRegion, hoveredRegion, hitTester, update,
                                  [](Qt::CursorShape) { /* Cursor set elsewhere */ });
}

} // namespace ocr_orc
