#ifndef CANVAS_MOUSE_HANDLER_H
#define CANVAS_MOUSE_HANDLER_H

#include <QtGui/QMouseEvent>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QMap>
#include <functional>
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"
#include "../CanvasMouseMode.h"  // Include before namespace to avoid double nesting

namespace ocr_orc {

// Forward declarations
class CanvasHitTester;
class CanvasSelectionManager;
class CanvasRegionCreator;
class CanvasRegionOperations;

/**
 * @brief Handles all mouse event processing for Canvas
 */
class CanvasMouseHandler {
public:
    // Callback types
    using UpdateCallback = std::function<void()>;
    using SetCursorCallback = std::function<void(Qt::CursorShape)>;
    using CalculateLayoutCallback = std::function<void()>;
    using EmitSignalCallback = std::function<void(const QString&)>;
    using EmitStateChangedCallback = std::function<void()>;
    using WidgetToCanvasCallback = std::function<QPointF(const QPointF&)>;
    using MoveRegionCallback = std::function<void(const QString&, const QPointF&)>;
    using MoveSelectedRegionsCallback = std::function<void(const QPointF&)>;
    using ResizeRegionCallback = std::function<void(const QString&, const QString&, const QPointF&)>;
    using ValidateRegionCallback = std::function<bool(const QRectF&)>;
    using FindRegionsInBoxCallback = std::function<QSet<QString>(const QRectF&)>;
    
    CanvasMouseHandler();
    ~CanvasMouseHandler();
    
    void handlePress(QMouseEvent* event,
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
                    const EmitSignalCallback& emitRegionCreationRequested);
    
    void handleMove(QMouseEvent* event,
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
                   const SetCursorCallback& setCursor);
    
    void handleRelease(QMouseEvent* event,
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
                      const EmitStateChangedCallback& emitStateChanged);
    
    void handleDoubleClick(QMouseEvent* event,
                          MouseMode mouseMode,
                          DocumentState* documentState,
                          const QImage& documentImage,
                          const QRectF& imageRect,
                          double scaleFactor,
                          const QPointF& imageOffset,
                          CanvasHitTester* hitTester,
                          const WidgetToCanvasCallback& widgetToCanvas,
                          const EmitSignalCallback& emitRegionEditRequested);

private:
    /**
     * @brief Reset all state flags to false
     * Called before early returns to prevent stuck states
     */
    void resetAllFlags(bool& isCreating,
                      bool& isBoxSelecting,
                      bool& isDragging,
                      bool& isResizing,
                      bool& isRotating,
                      bool& isPanning);
    
    void handleSelectModePress(QMouseEvent* event,
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
                              const WidgetToCanvasCallback& widgetToCanvas);
};

} // namespace ocr_orc

#endif // CANVAS_MOUSE_HANDLER_H

