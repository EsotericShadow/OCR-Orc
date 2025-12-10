#ifndef CANVAS_RENDERER_H
#define CANVAS_RENDERER_H

#include <QtGui/QPainter>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtGui/QImage>
#include "../../../../models/DocumentState.h"
#include "../coordinate/CanvasCoordinateCache.h"

namespace ocr_orc {

/**
 * @brief Handles all rendering operations for the Canvas
 * 
 * Responsible for painting:
 * - Document image with shadow
 * - Regions with labels and handles
 * - Temporary rectangles during creation
 * - Selection boxes
 */
class CanvasRenderer {
public:
    CanvasRenderer();
    ~CanvasRenderer();
    
    /**
     * @brief Render the document image with shadow
     * @param painter QPainter to use
     * @param documentImage Image to draw
     * @param imageRect Rectangle where image should be drawn
     */
    void drawDocumentImage(QPainter& painter, const QImage& documentImage, const QRectF& imageRect);
    
    /**
     * @brief Render all regions
     * @param painter QPainter to use
     * @param documentState Document state containing regions
     * @param coordinateCache Coordinate cache for region coordinates
     * @param documentImage Document image (for dimensions)
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     * @param viewportRect Viewport rectangle for culling
     * @param hoveredRegion Currently hovered region name
     * @param selectedRegions Set of selected region names
     * @param primarySelectedRegion Primary selected region (for resize handles)
     */
    void renderRegions(QPainter& painter,
                       DocumentState* documentState,
                       CanvasCoordinateCache* coordinateCache,
                       const QImage& documentImage,
                       double scaleFactor,
                       const QPointF& imageOffset,
                       const QRectF& viewportRect,
                       const QString& hoveredRegion,
                       const QSet<QString>& selectedRegions,
                       const QString& primarySelectedRegion,
                       bool isRotateMode = false,
                       const QString& rotatingRegion = QString(),
                       double rotationAngle = 0.0);
    
    /**
     * @brief Draw a single region
     * @param painter QPainter to use
     * @param region Region data
     * @param canvasRect Canvas coordinates for the region
     * @param isHovered Whether region is hovered
     * @param isSelected Whether region is selected
     * @param isPrimary Whether region is primary selected (for resize handles)
     */
    void drawRegion(QPainter& painter,
                    const RegionData& region,
                    const QRectF& canvasRect,
                    bool isHovered,
                    bool isSelected,
                    bool isPrimary = false,
                    bool isRotateMode = false,
                    bool isRotating = false,
                    double rotationAngle = 0.0);
    
    /**
     * @brief Draw resize handles for selected region
     * @param painter QPainter to use
     * @param rect Canvas rectangle for the region
     * @param isRotateMode If true, draw rotate icon instead of resize handles
     * @param rotationAngle Rotation angle in degrees (0.0 = no rotation)
     */
    void drawResizeHandles(QPainter& painter, const QRectF& rect, bool isRotateMode = false, double rotationAngle = 0.0);
    
    /**
     * @brief Draw region label above the region
     * @param painter QPainter to use
     * @param name Region name
     * @param rect Canvas rectangle for the region
     * @param color Region color
     * @param isSelected Whether region is selected
     */
    void drawRegionLabel(QPainter& painter,
                         const QString& name,
                         const QRectF& rect,
                         const QColor& color,
                         bool isSelected);
    
    /**
     * @brief Draw temporary rectangle during region creation
     * @param painter QPainter to use
     * @param rect Rectangle to draw
     */
    void drawTempRectangle(QPainter& painter, const QRectF& rect);
    
    /**
     * @brief Draw temporary shape during region creation
     * @param painter QPainter to use
     * @param rect Bounding rectangle for the shape
     * @param shapeType Shape type: "rect", "circle", "triangle", "poly"
     */
    void drawTempShape(QPainter& painter, const QRectF& rect, const QString& shapeType);
    
    /**
     * @brief Draw selection box during box selection
     * @param painter QPainter to use
     * @param rect Selection box rectangle
     */
    void drawSelectionBox(QPainter& painter, const QRectF& rect);
    
    /**
     * @brief Get QColor from color name string
     * @param colorName Color name (e.g., "blue", "red")
     * @return QColor, defaults to blue if name not found
     */
    QColor getRegionColor(const QString& colorName) const;

private:
    static constexpr double SHADOW_OFFSET = 5.0;
    static constexpr int SHADOW_ALPHA = 100;
};

} // namespace ocr_orc

#endif // CANVAS_RENDERER_H

