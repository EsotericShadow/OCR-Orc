#ifndef REGION_DATA_H
#define REGION_DATA_H

#include "../core/CoordinateSystem.h"
#include <QtCore/QString>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QPointF>

namespace ocr_orc {

/**
 * @brief Region data structure
 * 
 * Stores all information about a region including coordinates in all three
 * coordinate systems, name, color, and group assignment.
 * 
 * Coordinate synchronization:
 * - Normalized coordinates are the source of truth
 * - Image and canvas coordinates are calculated from normalized
 * - Use syncFromNormalized() to update all coordinates
 */
struct RegionData {
    // Coordinate systems (normalized is source of truth)
    NormalizedCoords normalizedCoords;
    ImageCoords imageCoords;
    CanvasCoords canvasCoords;
    
    // Region metadata
    QString name;
    QString color;  // "blue", "red", "green", "yellow", "purple", "orange", "cyan"
    QString group;  // Group name or empty string
    QString shapeType;  // "rect", "circle", "triangle", "poly" - shape type for rendering
    QString regionType;  // "text", "alphanumeric", "letters", "numbers", "roman", "unicode", "none"
    QString percentageFill;  // "none", "standard" - percentage fill detection option
    double rotationAngle;  // Rotation angle in degrees (0.0 = no rotation)
    
    // Constructors
    RegionData() : shapeType("rect"), regionType("none"), percentageFill("none"), rotationAngle(0.0) {}
    
    RegionData(const QString& name, 
               const NormalizedCoords& norm,
               const QString& color = "blue",
               const QString& group = "",
               const QString& shapeType = "rect",
               const QString& regionType = "none",
               const QString& percentageFill = "none",
               double rotationAngle = 0.0)
        : normalizedCoords(norm)
        , name(name)
        , color(color)
        , group(group)
        , shapeType(shapeType)
        , regionType(regionType)
        , percentageFill(percentageFill)
        , rotationAngle(rotationAngle)
    {
        // Image and canvas coords will be calculated when image/zoom is known
    }
    
    // Equality operator (for testing and comparison)
    bool operator==(const RegionData& other) const {
        return name == other.name &&
               color == other.color &&
               group == other.group &&
               shapeType == other.shapeType &&
               regionType == other.regionType &&
               percentageFill == other.percentageFill &&
               rotationAngle == other.rotationAngle &&
               normalizedCoords.x1 == other.normalizedCoords.x1 &&
               normalizedCoords.y1 == other.normalizedCoords.y1 &&
               normalizedCoords.x2 == other.normalizedCoords.x2 &&
               normalizedCoords.y2 == other.normalizedCoords.y2;
    }
    
    /**
     * @brief Validate all coordinates
     * @return true if all coordinates are valid
     */
    bool isValid() const;
    
    /**
     * @brief Synchronize all coordinates from normalized (source of truth)
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     * @param scaleFactor Display scale factor
     * @param offset Image position offset on canvas
     */
    void syncFromNormalized(int imgWidth, int imgHeight, 
                           double scaleFactor, 
                           const QPointF& offset);
    
    /**
     * @brief Synchronize coordinates from canvas input (user drawing)
     * @param canvas Canvas coordinates from user input
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     * @param scaleFactor Display scale factor
     * @param offset Image position offset on canvas
     */
    void syncFromCanvas(const CanvasCoords& canvas,
                       int imgWidth, int imgHeight,
                       double scaleFactor,
                       const QPointF& offset);
    
    /**
     * @brief Update canvas coordinates only (for zoom/pan changes)
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     * @param scaleFactor Display scale factor
     * @param offset Image position offset on canvas
     */
    void updateCanvasCoords(int imgWidth, int imgHeight,
                           double scaleFactor,
                           const QPointF& offset);
    
    /**
     * @brief Update image coordinates only (when image loads)
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     */
    void updateImageCoords(int imgWidth, int imgHeight);
    
    /**
     * @brief Convert canvas coordinates to QRectF for rendering
     * @return QRectF rectangle for Qt rendering
     */
    QRectF toQRectF() const;
    
    /**
     * @brief Convert image coordinates to QRect for image operations
     * @return QRect rectangle for image operations
     */
    QRect toQRect() const;
};

} // namespace ocr_orc

#endif // REGION_DATA_H

