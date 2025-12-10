#include "CanvasMouseHandler.h"
#include "../core/selection/CanvasSelectionManager.h"
#include "../core/regions/CanvasRegionCreator.h"
#include "../core/regions/CanvasRegionOperations.h"
#include "../../../models/RegionData.h"
#include "../../../core/CoordinateSystem.h"
#include "../../../core/Constants.h"
#include <QtGui/QImage>
#include <cmath>

namespace ocr_orc {

void CanvasMouseHandler::handleRelease(QMouseEvent* event,
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
                                      const WidgetToCanvasCallback& widgetToCanvas,
                                      const ValidateRegionCallback& validateRegion,
                                      const FindRegionsInBoxCallback& findRegionsInBox,
                                      const MoveSelectedRegionsCallback& moveSelectedRegions,
                                      const ResizeRegionCallback& resizeRegion,
                                      const UpdateCallback& update,
                                      const CalculateLayoutCallback& calculateLayout,
                                      const SetCursorCallback& setCursor,
                                      const EmitSignalCallback& emitRegionCreationRequested,
                                      const EmitStateChangedCallback& emitStateChanged) {
    if (!documentState || documentImage.isNull()) {
        resetAllFlags(isCreating, isBoxSelecting, isDragging, isResizing, isRotating, isPanning);
        return;
    }
    
    Q_UNUSED(calculateLayout);
    
    if (mouseMode == Create && isCreating) {
        QPointF canvasPos = widgetToCanvas(event->pos());
        if (regionCreator) {
            regionCreator->updateRegionCreation(canvasPos, isCreating, creationStartPos, imageRect, tempRect);
        }
        
        if (validateRegion(tempRect)) {
            emitRegionCreationRequested(QString());
        } else {
            isCreating = false;
            tempRect = QRectF();
            update();
        }
    } else if (mouseMode == Select) {
        QPointF canvasPos = widgetToCanvas(event->pos());
        
        if (isDragging) {
            QPointF delta = canvasPos - lastDragPos;
            if (delta.manhattanLength() > 0.1) {
                moveSelectedRegions(delta);
            }
            isDragging = false;
            setCursor(Qt::ArrowCursor);
            update();
            
            if (documentState) {
                dragStartState.clear();
                emitStateChanged();
            }
        } else if (isRotating && !rotatingRegion.isEmpty()) {
            OCR_ORC_DEBUG("=== Rotation RELEASE ===");
            OCR_ORC_DEBUG("  rotatingRegion:" << rotatingRegion);
            OCR_ORC_DEBUG("  selectedRegions.size():" << selectedRegions.size());
            OCR_ORC_DEBUG("  selectedRegions.contains(rotatingRegion):" << selectedRegions.contains(rotatingRegion));
            OCR_ORC_DEBUG("  regionOperations:" << (regionOperations ? "valid" : "null"));
            OCR_ORC_DEBUG("  Final rotationAngle:" << rotationAngle << "degrees");
            
            // Rotation is complete - apply the rotation to the region
            // Only rotate if this is the single selected region (safety check)
            if (selectedRegions.size() == 1 && selectedRegions.contains(rotatingRegion) && regionOperations) {
                OCR_ORC_DEBUG("  Calling rotateRegion...");
                regionOperations->rotateRegion(rotatingRegion, rotationAngle,
                                              documentState, documentImage,
                                              scaleFactor, imageOffset);
                
                if (documentState) {
                    emitStateChanged();
                }
                OCR_ORC_DEBUG("  Rotation applied successfully");
            } else {
                OCR_ORC_WARNING("  Rotation NOT applied - conditions not met:");
                OCR_ORC_WARNING("    selectedRegions.size() == 1:" << (selectedRegions.size() == 1));
                OCR_ORC_WARNING("    selectedRegions.contains(rotatingRegion):" << selectedRegions.contains(rotatingRegion));
                OCR_ORC_WARNING("    regionOperations valid:" << (regionOperations != nullptr));
            }
            
            // Clear the rotation state
            isRotating = false;
            rotatingRegion = QString();
            rotationAngle = 0.0;
            rotationStartAngle = 0.0;
            OCR_ORC_DEBUG("  Rotation state cleared");
        } else if (isResizing) {
            OCR_ORC_DEBUG("[RESIZE] === Resize RELEASE ===");
            OCR_ORC_DEBUG("  resizingRegion: " << resizingRegion);
            OCR_ORC_DEBUG("  resizeHandle: " << resizeHandle);
            OCR_ORC_DEBUG("  canvasPos: (" << canvasPos.x() << "," << canvasPos.y() << ")");
            resizeRegion(resizingRegion, resizeHandle, canvasPos);
            isResizing = false;
            resizingRegion = QString();
            resizeHandle = QString();
            setCursor(Qt::ArrowCursor);
            
            if (documentState) {
                resizeStartRegionData = RegionData();
                emitStateChanged();
            }
            OCR_ORC_DEBUG("  Resize complete, state cleared");
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
            }
            
            isPanning = false;
            setCursor(Qt::ArrowCursor);
        } else {
            if (isBoxSelecting && selectionManager) {
                QRectF finalBox(
                    std::min(boxSelectionStart.x(), canvasPos.x()),
                    std::min(boxSelectionStart.y(), canvasPos.y()),
                    std::abs(canvasPos.x() - boxSelectionStart.x()),
                    std::abs(canvasPos.y() - boxSelectionStart.y())
                );
                
                QSet<QString> regionsInBox = findRegionsInBox(finalBox);
                
                // Find the last region in the box (by iterating through document order)
                QString lastRegionInBox;
                if (!regionsInBox.isEmpty() && documentState) {
                    QList<QString> allRegionNames = documentState->getAllRegionNames();
                    for (const QString& regionName : allRegionNames) {
                        if (regionsInBox.contains(regionName)) {
                            lastRegionInBox = regionName;
                        }
                    }
                }
                
                bool isShiftPressed = (event->modifiers() & Qt::ShiftModifier);
                if (isShiftPressed) {
                    // Add to selection and set primary to last region added
                    selectionManager->addToSelection(regionsInBox, selectedRegions, primarySelectedRegion);
                    if (!lastRegionInBox.isEmpty()) {
                        primarySelectedRegion = lastRegionInBox;
                    }
                } else {
                    selectedRegions = regionsInBox;
                    // Set primary to last region in box (or first if only one)
                    if (regionsInBox.size() == 1) {
                        primarySelectedRegion = *regionsInBox.begin();
                    } else if (!lastRegionInBox.isEmpty()) {
                        primarySelectedRegion = lastRegionInBox;
                    } else {
                        primarySelectedRegion = QString();
                    }
                }
                
                isBoxSelecting = false;
                selectionBox = QRectF();
                update();
            }
        }
    }
    
    event->accept();
}

} // namespace ocr_orc


