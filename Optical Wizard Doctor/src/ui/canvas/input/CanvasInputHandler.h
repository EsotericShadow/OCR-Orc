#ifndef CANVAS_INPUT_HANDLER_H
#define CANVAS_INPUT_HANDLER_H

#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QContextMenuEvent>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <functional>
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"
#include "../CanvasMouseMode.h"  // Include before namespace to avoid double nesting

namespace ocr_orc {

// Forward declarations
class Canvas;
class CanvasHitTester;
class CanvasSelectionManager;
class CanvasRegionCreator;
class CanvasZoomController;
class CanvasRegionOperations;
class CanvasMouseHandler;      // Forward declarations for handler classes
class CanvasKeyboardHandler;
class CanvasWheelHandler;
class CanvasHoverManager;

/**
 * @brief Handles all input events for Canvas (mouse, keyboard, wheel)
 * 
 * This class processes all user input and delegates to appropriate handlers.
 * It uses callbacks to interact with Canvas state and emit signals.
 */
class CanvasInputHandler {
public:
    // Callback types for Canvas operations
    using UpdateCallback = std::function<void()>;
    using SetCursorCallback = std::function<void(Qt::CursorShape)>;
    using CalculateLayoutCallback = std::function<void()>;
    using EmitSignalCallback = std::function<void(const QString&)>;
    using EmitRegionsDuplicatedCallback = std::function<void(const QList<QString>&)>;
    using EmitStateChangedCallback = std::function<void()>;
    
    CanvasInputHandler();
    ~CanvasInputHandler();
    
    /**
     * @brief Handle mouse press event
     */
    void handleMousePress(QMouseEvent* event,
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
                         const EmitSignalCallback& emitRegionCreationRequested);
    
    /**
     * @brief Handle mouse move event
     */
    void handleMouseMove(QMouseEvent* event,
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
                        const SetCursorCallback& setCursor);
    
    /**
     * @brief Handle mouse release event
     */
    void handleMouseRelease(QMouseEvent* event,
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
                           const EmitStateChangedCallback& emitStateChanged);
    
    /**
     * @brief Handle mouse double-click event
     */
    void handleMouseDoubleClick(QMouseEvent* event,
                               MouseMode mouseMode,
                               DocumentState* documentState,
                               const QImage& documentImage,
                               const QRectF& imageRect,
                               double scaleFactor,
                               const QPointF& imageOffset,
                               CanvasHitTester* hitTester,
                               const std::function<QPointF(const QPointF&)>& widgetToCanvas,
                               const EmitSignalCallback& emitRegionEditRequested);
    
    /**
     * @brief Handle key press event
     */
    void handleKeyPress(QKeyEvent* event,
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
                       const EmitStateChangedCallback& emitStateChanged);
    
    /**
     * @brief Handle wheel event
     */
    void handleWheel(QWheelEvent* event,
                    DocumentState* documentState,
                    const QImage& documentImage,
                    double& scaleFactor,
                    QPointF& imageOffset,
                    CanvasZoomController* zoomController,
                    const std::function<void()>& calculateLayout,
                    const UpdateCallback& update);
    
    /**
     * @brief Handle context menu event
     */
    void handleContextMenu(QContextMenuEvent* event,
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
                          const EmitRegionsDuplicatedCallback& emitRegionsDuplicated);
    
    /**
     * @brief Update hover state
     * Note: Cursor is set in mouse move handler, not here
     */
    void updateHoverState(const QPointF& canvasPos,
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
                         const UpdateCallback& update);

private:
    // Forward declarations (at namespace level, not nested)
    CanvasMouseHandler* mouseHandler;
    CanvasKeyboardHandler* keyboardHandler;
    CanvasWheelHandler* wheelHandler;
    CanvasHoverManager* hoverManager;
};

} // namespace ocr_orc

#endif // CANVAS_INPUT_HANDLER_H

