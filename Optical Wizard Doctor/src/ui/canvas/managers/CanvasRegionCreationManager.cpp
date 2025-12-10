#include "CanvasRegionCreationManager.h"
#include "../core/regions/CanvasRegionCreator.h"
#include <QtGui/QImage>

namespace ocr_orc {

CanvasRegionCreationManager::CanvasRegionCreationManager() {
}

CanvasRegionCreationManager::~CanvasRegionCreationManager() {
}

bool CanvasRegionCreationManager::startRegionCreation(const QPointF& pos,
                                                      const QRectF& imageRect,
                                                      CanvasRegionCreator* regionCreator,
                                                      bool& isCreating,
                                                      QPointF& creationStartPos,
                                                      QRectF& tempRect) {
    if (!regionCreator) return false;
    return regionCreator->startRegionCreation(pos, imageRect, isCreating, creationStartPos, tempRect);
}

void CanvasRegionCreationManager::updateRegionCreation(const QPointF& pos,
                                                      const QRectF& imageRect,
                                                      CanvasRegionCreator* regionCreator,
                                                      bool isCreating,
                                                      QPointF& creationStartPos,
                                                      QRectF& tempRect) {
    if (!regionCreator) return;
    regionCreator->updateRegionCreation(pos, isCreating, creationStartPos, imageRect, tempRect);
}

bool CanvasRegionCreationManager::finishRegionCreation(const QString& regionName,
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
                                                      const std::function<bool(const QRectF&)>& validateRegion) {
    if (!regionCreator || !documentState) return false;
    
    auto validateFunc = [validateRegion](const QRectF& rect, const QRectF& /*imgRect*/) -> bool {
        return validateRegion(rect);
    };
    
    return regionCreator->finishRegionCreation(regionName, color, group, shapeType, isCreating, tempRect,
                                              documentState, documentImage, scaleFactor,
                                              imageOffset, validateFunc);
}

} // namespace ocr_orc

