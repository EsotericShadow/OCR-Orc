#ifndef CANVAS_REGION_CREATION_MANAGER_H
#define CANVAS_REGION_CREATION_MANAGER_H

#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <functional>
#include "../../../models/DocumentState.h"

namespace ocr_orc {

class CanvasRegionCreator;

/**
 * @brief Manages region creation operations for Canvas
 */
class CanvasRegionCreationManager {
public:
    CanvasRegionCreationManager();
    ~CanvasRegionCreationManager();
    
    bool startRegionCreation(const QPointF& pos,
                            const QRectF& imageRect,
                            CanvasRegionCreator* regionCreator,
                            bool& isCreating,
                            QPointF& creationStartPos,
                            QRectF& tempRect);
    
    void updateRegionCreation(const QPointF& pos,
                             const QRectF& imageRect,
                             CanvasRegionCreator* regionCreator,
                             bool isCreating,
                             QPointF& creationStartPos,
                             QRectF& tempRect);
    
    bool finishRegionCreation(const QString& regionName,
                             const QString& color,
                             const QString& group,
                             const QString& shapeType,
                             CanvasRegionCreator* regionCreator,
                             DocumentState* documentState,
                             const QImage& documentImage,
                             double scaleFactor,
                             const QPointF& imageOffset,
                             bool& isCreating,
                             QRectF& tempRect,
                             const std::function<bool(const QRectF&)>& validateRegion);
};

} // namespace ocr_orc

#endif // CANVAS_REGION_CREATION_MANAGER_H

