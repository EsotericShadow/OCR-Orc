#include "CanvasWheelHandler.h"
#include "../core/zoom/CanvasZoomController.h"
#include "../../../core/Constants.h"

namespace ocr_orc {

CanvasWheelHandler::CanvasWheelHandler() {
}

CanvasWheelHandler::~CanvasWheelHandler() {
}

void CanvasWheelHandler::handleWheel(QWheelEvent* event,
                                    DocumentState* documentState,
                                    const QImage& documentImage,
                                    double& scaleFactor,
                                    QPointF& imageOffset,
                                    CanvasZoomController* zoomController,
                                    const CalculateLayoutCallback& calculateLayout,
                                    const UpdateCallback& update) {
    if (!documentState || documentImage.isNull() || !zoomController) {
        return;
    }
    
    bool isModifierPressed = (event->modifiers() & Qt::ControlModifier) ||
                             (event->modifiers() & Qt::MetaModifier);
    
    if (isModifierPressed) {
        // Zoom handling - handled in Canvas::wheelEvent directly
        event->accept();
        return;
    } else {
        QPointF delta = event->angleDelta();
        if (zoomController) {
            zoomController->panWithWheel(delta, documentState, imageOffset);
        }
        calculateLayout();
        update();
    }
    
    event->accept();
}

} // namespace ocr_orc

