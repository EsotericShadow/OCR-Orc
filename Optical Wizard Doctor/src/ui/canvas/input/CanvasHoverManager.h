#ifndef CANVAS_HOVER_MANAGER_H
#define CANVAS_HOVER_MANAGER_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <functional>
#include "../../../models/DocumentState.h"
#include "../CanvasMouseMode.h"  // Include before namespace to avoid double nesting

namespace ocr_orc {

class CanvasHitTester;

/**
 * @brief Manages hover state for Canvas regions
 */
class CanvasHoverManager {
public:
    // Callback types
    using UpdateCallback = std::function<void()>;
    using SetCursorCallback = std::function<void(Qt::CursorShape)>;
    
    CanvasHoverManager();
    ~CanvasHoverManager();
    
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
                         const UpdateCallback& update,
                         const SetCursorCallback& setCursor);
};

} // namespace ocr_orc

#endif // CANVAS_HOVER_MANAGER_H

