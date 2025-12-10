#ifndef CANVAS_WHEEL_HANDLER_H
#define CANVAS_WHEEL_HANDLER_H

#include <QtGui/QWheelEvent>
#include <QtCore/QPointF>
#include <functional>
#include "../../../models/DocumentState.h"

namespace ocr_orc {

class CanvasZoomController;

/**
 * @brief Handles wheel event processing for Canvas (zoom and pan)
 */
class CanvasWheelHandler {
public:
    // Callback types
    using UpdateCallback = std::function<void()>;
    using CalculateLayoutCallback = std::function<void()>;
    
    CanvasWheelHandler();
    ~CanvasWheelHandler();
    
    void handleWheel(QWheelEvent* event,
                    DocumentState* documentState,
                    const QImage& documentImage,
                    double& scaleFactor,
                    QPointF& imageOffset,
                    CanvasZoomController* zoomController,
                    const CalculateLayoutCallback& calculateLayout,
                    const UpdateCallback& update);
};

} // namespace ocr_orc

#endif // CANVAS_WHEEL_HANDLER_H

