#ifndef CANVAS_HIT_TESTER_H
#define CANVAS_HIT_TESTER_H

#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtGui/QImage>
#include "../../../../models/DocumentState.h"
#include "../../../../core/CoordinateSystem.h"

namespace ocr_orc {

/**
 * @brief Handles hit testing operations (finding regions/handles at positions)
 */
class CanvasHitTester {
public:
    CanvasHitTester();
    ~CanvasHitTester();
    
    /**
     * @brief Find region at the given canvas position (hit testing)
     * @param canvasPos Position in canvas coordinates
     * @param documentState Document state
     * @param documentImage Document image (for dimensions)
     * @param imageRect Image rectangle in canvas coordinates
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     * @return Region name if found, empty string otherwise
     */
    QString getRegionAt(const QPointF& canvasPos,
                       DocumentState* documentState,
                       const QImage& documentImage,
                       const QRectF& imageRect,
                       double scaleFactor,
                       const QPointF& imageOffset) const;
    
    /**
     * @brief Check if point is on a resize handle
     * @param canvasPos Position in canvas coordinates
     * @param regionRect Region rectangle in canvas coordinates
     * @param rotationAngle Rotation angle in degrees (0.0 = no rotation)
     * @return Handle position string ("nw", "ne", "sw", "se", "n", "s", "e", "w") or empty
     */
    QString getHandleAt(const QPointF& canvasPos, const QRectF& regionRect, double rotationAngle = 0.0) const;
    
    /**
     * @brief Check if point is on the rotate icon (bottom-right corner)
     * @param canvasPos Canvas coordinates
     * @param regionRect Region rectangle in canvas coordinates
     * @param iconSize Size of the rotate icon
     * @param rotationAngle Rotation angle in degrees (0.0 = no rotation)
     * @return true if point is on rotate icon
     */
    bool isOnRotateIcon(const QPointF& canvasPos, const QRectF& regionRect, double iconSize = 24.0, double rotationAngle = 0.0) const;
};

} // namespace ocr_orc

#endif // CANVAS_HIT_TESTER_H

