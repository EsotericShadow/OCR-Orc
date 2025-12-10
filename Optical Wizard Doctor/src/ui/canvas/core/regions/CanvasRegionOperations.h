#ifndef CANVAS_REGION_OPERATIONS_H
#define CANVAS_REGION_OPERATIONS_H

#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QList>
#include <QtGui/QImage>
#include "../../../../models/DocumentState.h"
#include "../../../../models/RegionData.h"
#include "../../../../core/CoordinateSystem.h"

namespace ocr_orc {

/**
 * @brief Handles region manipulation operations
 * 
 * Manages:
 * - Moving regions
 * - Resizing regions
 * - Constraining regions to bounds
 * - Validating regions
 */
class CanvasRegionOperations {
public:
    CanvasRegionOperations();
    ~CanvasRegionOperations();
    
    /**
     * @brief Move a region by delta in canvas coordinates
     * @param regionName Name of region to move
     * @param delta Movement delta in canvas coordinates
     * @param documentState Document state
     * @param documentImage Document image (for dimensions)
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     */
    void moveRegion(const QString& regionName,
                   const QPointF& delta,
                   DocumentState* documentState,
                   const QImage& documentImage,
                   double scaleFactor,
                   const QPointF& imageOffset);
    
    /**
     * @brief Move all specified regions by delta
     * @param regionNames List of region names to move
     * @param delta Movement delta in canvas coordinates
     * @param documentState Document state
     * @param documentImage Document image (for dimensions)
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     */
    void moveRegions(const QList<QString>& regionNames,
                    const QPointF& delta,
                    DocumentState* documentState,
                    const QImage& documentImage,
                    double scaleFactor,
                    const QPointF& imageOffset);
    
    /**
     * @brief Resize a region using a handle
     * @param regionName Name of region to resize
     * @param handle Handle being dragged ("nw", "ne", "sw", "se", "n", "s", "e", "w")
     * @param newPos New position of the handle in canvas coordinates
     * @param resizeStartRect Original region rectangle before resize
     * @param documentState Document state
     * @param documentImage Document image (for dimensions)
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     * @param imageRect Image rectangle in canvas coordinates
     * @param rotationAngle Rotation angle in degrees (0.0 = no rotation)
     */
    void resizeRegion(const QString& regionName,
                     const QString& handle,
                     const QPointF& newPos,
                     const QRectF& resizeStartRect,
                     DocumentState* documentState,
                     const QImage& documentImage,
                     double scaleFactor,
                     const QPointF& imageOffset,
                     const QRectF& imageRect,
                     double rotationAngle,
                     const NormalizedCoords& originalNormalizedCoords);
    
    /**
     * @brief Rotate a region around its center
     * @param regionName Name of region to rotate
     * @param rotationAngle Rotation angle in degrees (positive = counterclockwise)
     * @param documentState Document state
     * @param documentImage Document image (for dimensions)
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     */
    void rotateRegion(const QString& regionName,
                     double rotationAngle,
                     DocumentState* documentState,
                     const QImage& documentImage,
                     double scaleFactor,
                     const QPointF& imageOffset);
    
    /**
     * @brief Constrain region coordinates to valid bounds
     * @param region Region to constrain (modified in place)
     */
    void constrainToBounds(RegionData& region);
    
    /**
     * @brief Validate region before creation
     * @param rect Canvas rectangle
     * @param imageRect Image rectangle in canvas coordinates
     * @return true if valid, false otherwise
     */
    bool validateRegion(const QRectF& rect, const QRectF& imageRect) const;
};

} // namespace ocr_orc

#endif // CANVAS_REGION_OPERATIONS_H

