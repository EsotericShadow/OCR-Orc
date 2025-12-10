#ifndef CANVAS_REGION_CREATOR_H
#define CANVAS_REGION_CREATOR_H

#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtGui/QImage>
#include <functional>
#include "../../../../models/DocumentState.h"
#include "../../../../core/CoordinateSystem.h"

namespace ocr_orc {

/**
 * @brief Handles region creation logic
 */
class CanvasRegionCreator {
public:
    CanvasRegionCreator();
    ~CanvasRegionCreator();
    
    /**
     * @brief Start region creation at the given position
     * @param pos Canvas coordinates where creation starts
     * @param imageRect Image rectangle in canvas coordinates
     * @param isCreating Output: set to true if creation started
     * @param creationStartPos Output: set to start position
     * @param tempRect Output: cleared temporary rectangle
     * @return true if creation started, false if position invalid
     */
    bool startRegionCreation(const QPointF& pos,
                            const QRectF& imageRect,
                            bool& isCreating,
                            QPointF& creationStartPos,
                            QRectF& tempRect);
    
    /**
     * @brief Update region creation during drag
     * @param pos Current canvas coordinates
     * @param isCreating Whether creation is active
     * @param creationStartPos Start position
     * @param imageRect Image rectangle in canvas coordinates
     * @param tempRect Output: updated temporary rectangle
     */
    void updateRegionCreation(const QPointF& pos,
                             bool isCreating,
                             const QPointF& creationStartPos,
                             const QRectF& imageRect,
                             QRectF& tempRect);
    
    /**
     * @brief Finish region creation and add to DocumentState
     * @param regionName Name for the new region
     * @param color Color for the new region
     * @param group Group name (optional)
     * @param isCreating Whether creation is active
     * @param tempRect Temporary rectangle
     * @param documentState Document state
     * @param documentImage Document image (for dimensions)
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     * @param validateRegion Function to validate region rectangle
     * @return true if region was created successfully, false otherwise
     */
    bool finishRegionCreation(const QString& regionName,
                             const QString& color,
                             const QString& group,
                             const QString& shapeType,
                             bool& isCreating,
                             QRectF& tempRect,
                             DocumentState* documentState,
                             const QImage& documentImage,
                             double scaleFactor,
                             const QPointF& imageOffset,
                             std::function<bool(const QRectF&, const QRectF&)> validateRegion);
};

} // namespace ocr_orc

#endif // CANVAS_REGION_CREATOR_H

