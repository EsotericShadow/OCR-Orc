#include "CanvasMouseHandler.h"
#include "../core/coordinate/CanvasHitTester.h"
#include "../../../core/CoordinateSystem.h"
#include <QtGui/QImage>

namespace ocr_orc {

void CanvasMouseHandler::handleDoubleClick(QMouseEvent* event,
                                          MouseMode mouseMode,
                                          DocumentState* documentState,
                                          const QImage& documentImage,
                                          const QRectF& imageRect,
                                          double scaleFactor,
                                          const QPointF& imageOffset,
                                          CanvasHitTester* hitTester,
                                          const WidgetToCanvasCallback& widgetToCanvas,
                                          const EmitSignalCallback& emitRegionEditRequested) {
    if (!documentState || documentImage.isNull() || mouseMode != Select) {
        return;
    }
    
    QPointF canvasPos = widgetToCanvas(event->pos());
    
    if (hitTester) {
        QString regionName = hitTester->getRegionAt(canvasPos, documentState, documentImage,
                                                    imageRect, scaleFactor, imageOffset);
        if (!regionName.isEmpty()) {
            emitRegionEditRequested(regionName);
        }
    }
    
    event->accept();
}

} // namespace ocr_orc


