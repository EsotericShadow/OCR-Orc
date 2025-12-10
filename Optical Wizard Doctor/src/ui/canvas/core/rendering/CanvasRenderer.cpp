#include "../rendering/CanvasRenderer.h"
#include "../../../../models/RegionData.h"
#include "../../../../core/CoordinateSystem.h"
#include "../../../../core/Constants.h"
#include <QtCore/QMap>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QPolygonF>

namespace ocr_orc {

CanvasRenderer::CanvasRenderer() {
}

CanvasRenderer::~CanvasRenderer() {
}

void CanvasRenderer::drawDocumentImage(QPainter& painter, const QImage& documentImage, const QRectF& imageRect) {
    // Draw shadow first (offset by SHADOW_OFFSET)
    QRectF shadowRect = imageRect.translated(SHADOW_OFFSET, SHADOW_OFFSET);
    painter.fillRect(shadowRect, QColor(0, 0, 0, SHADOW_ALPHA));
    
    // Draw image
    painter.drawImage(imageRect, documentImage);
}

void CanvasRenderer::renderRegions(QPainter& painter,
                                   DocumentState* documentState,
                                   CanvasCoordinateCache* coordinateCache,
                                   const QImage& documentImage,
                                   double scaleFactor,
                                   const QPointF& imageOffset,
                                   const QRectF& viewportRect,
                                   const QString& hoveredRegion,
                                   const QSet<QString>& selectedRegions,
                                   const QString& primarySelectedRegion,
                                   bool isRotateMode,
                                   const QString& rotatingRegion,
                                   double rotationAngle) {
    if (!documentState || documentImage.isNull() || !coordinateCache) {
        return;
    }
    
    // Get all region names
    QList<QString> regionNames = documentState->getAllRegionNames();
    
    // Get image dimensions
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    
    // Check if coordinate cache needs updating
    double currentZoom = documentState->zoomLevel;
    if (coordinateCache->needsUpdate(currentZoom, imageOffset, documentImage.size())) {
        // Update cache for all regions
        coordinateCache->updateCache(documentState, imgWidth, imgHeight,
                                     scaleFactor, imageOffset, currentZoom);
    }
    
    // Render each region (using cached coordinates)
    for (const QString& regionName : regionNames) {
        // Get cached canvas coordinates (calculates if not cached)
        QRectF canvasRect = coordinateCache->getCachedCoordinates(
            regionName, documentState, imgWidth, imgHeight, scaleFactor, imageOffset);
        
        // Viewport culling: Only render if region intersects viewport
        if (canvasRect.intersects(viewportRect)) {
            RegionData region = documentState->getRegion(regionName);
            
            // Determine state
            bool isHovered = (hoveredRegion == regionName);
            bool isSelected = selectedRegions.contains(regionName);
            bool isPrimary = (primarySelectedRegion == regionName && selectedRegions.size() == 1);
            
            // Draw the region (with rotation if applicable)
            bool isRotatingThis = (rotatingRegion == regionName);
            // Use stored rotation angle from region data, or temporary rotation angle during drag
            double angleToUse = isRotatingThis ? rotationAngle : region.rotationAngle;
            drawRegion(painter, region, canvasRect, isHovered, isSelected, isPrimary, isRotateMode, isRotatingThis || (region.rotationAngle != 0.0), angleToUse);
        }
    }
}

void CanvasRenderer::drawRegion(QPainter& painter,
                                const RegionData& region,
                                const QRectF& canvasRect,
                                bool isHovered,
                                bool isSelected,
                                bool isPrimary,
                                bool isRotateMode,
                                bool isRotating,
                                double rotationAngle) {
    // Get region color
    QColor regionColor = getRegionColor(region.color);
    
    // Determine pen width and fill opacity based on state
    int penWidth = 2;
    int fillAlpha = 0;
    
    if (isSelected) {
        penWidth = 4;
        fillAlpha = 51; // 20% of 255
    } else if (isHovered) {
        penWidth = 3;
        fillAlpha = 26; // 10% of 255
    }
    
    // Set pen (reuse pen object for efficiency)
    static QPen cachedPen;
    cachedPen.setColor(regionColor);
    cachedPen.setWidth(penWidth);
    cachedPen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(cachedPen);
    
    // Set brush (with opacity if needed)
    if (fillAlpha > 0) {
        static QColor fillColor;
        fillColor = regionColor;
        fillColor.setAlpha(fillAlpha);
        painter.setBrush(QBrush(fillColor));
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    
    // Save painter state before applying rotation transformation
    // CRITICAL: Must save/restore to prevent rotation from affecting other regions
    painter.save();
    
    // Apply rotation transformation if region has rotation (stored or active)
    // Use active rotation angle during drag, or stored rotation angle otherwise
    double angleToApply = isRotating ? rotationAngle : region.rotationAngle;
    if (angleToApply != 0.0) {
        QPointF center = canvasRect.center();
        painter.translate(center);
        painter.rotate(angleToApply);
        painter.translate(-center);
    }
    
    // Draw shape based on region shape type
    if (region.shapeType == "circle" || region.shapeType == "ellipse") {
        painter.drawEllipse(canvasRect);
    } else if (region.shapeType == "triangle") {
        // Draw triangle: top center, bottom left, bottom right
        QPointF top(canvasRect.center().x(), canvasRect.top());
        QPointF bottomLeft(canvasRect.left(), canvasRect.bottom());
        QPointF bottomRight(canvasRect.right(), canvasRect.bottom());
        QPolygonF triangle;
        triangle << top << bottomLeft << bottomRight;
        painter.drawPolygon(triangle);
    } else {
        // Default: rectangle
        painter.drawRect(canvasRect);
    }
    
    // Restore painter state after drawing shape (removes rotation transformation)
    painter.restore();
    
    // Draw label (outside rotation transformation)
    drawRegionLabel(painter, region.name, canvasRect, regionColor, isSelected);
    
    // Draw resize handles or rotate icon if primary selected (single selection)
    if (isPrimary) {
        // Use stored rotation angle from region data, or temporary rotation angle during drag
        double angleToUse = isRotating ? rotationAngle : region.rotationAngle;
        drawResizeHandles(painter, canvasRect, isRotateMode, angleToUse);
    }
}

void CanvasRenderer::drawResizeHandles(QPainter& painter, const QRectF& rect, bool isRotateMode, double rotationAngle) {
    // Save painter state
    painter.save();
    
    // Apply rotation transformation if region is rotated
    if (rotationAngle != 0.0) {
        QPointF center = rect.center();
        painter.translate(center);
        painter.rotate(rotationAngle);
        painter.translate(-center);
    }
    
    if (isRotateMode) {
        // Draw rotate icon at bottom-right corner
        const double iconSize = 24.0;
        QPointF rotatePos(rect.right(), rect.bottom());
        
        // Draw white circle background with black border
        QRectF iconRect(
            rotatePos.x() - iconSize / 2.0,
            rotatePos.y() - iconSize / 2.0,
            iconSize,
            iconSize
        );
        
        painter.setPen(QPen(QColor(0, 0, 0), 1));
        painter.setBrush(QBrush(QColor(255, 255, 255)));
        painter.drawEllipse(iconRect);
        
        // Draw circular arrow (simplified rotate icon)
        painter.setPen(QPen(QColor(0, 0, 0), 2));
        painter.setBrush(Qt::NoBrush);
        
        QRectF arcRect = iconRect.adjusted(4, 4, -4, -4);
        painter.drawArc(arcRect, 45 * 16, 270 * 16); // Draw arc from 45 to 315 degrees
        
        // Draw arrow head at end of arc
        QPointF arrowStart(rotatePos.x() + iconSize / 3.0, rotatePos.y() - iconSize / 3.0);
        QPointF arrowTip(rotatePos.x() + iconSize / 2.0 - 2, rotatePos.y() - iconSize / 2.0 + 2);
        QPointF arrowBase(rotatePos.x() + iconSize / 3.0 - 2, rotatePos.y() - iconSize / 3.0);
        QPolygonF arrow;
        arrow << arrowTip << arrowBase << arrowStart;
        painter.setBrush(QBrush(QColor(0, 0, 0)));
        painter.drawPolygon(arrow);
    } else {
        // Draw resize handles (original behavior)
        // Handle size
        const double handleSize = RegionConstants::HANDLE_SIZE;
        const double halfHandle = handleSize / 2.0;
        
        // Define handle positions (8 handles: 4 corners, 4 edges)
        QPointF handles[8] = {
            QPointF(rect.left(), rect.top()),                    // Top-left
            QPointF(rect.center().x(), rect.top()),              // Top-center
            QPointF(rect.right(), rect.top()),                   // Top-right
            QPointF(rect.right(), rect.center().y()),           // Right-center
            QPointF(rect.right(), rect.bottom()),                // Bottom-right
            QPointF(rect.center().x(), rect.bottom()),          // Bottom-center
            QPointF(rect.left(), rect.bottom()),                 // Bottom-left
            QPointF(rect.left(), rect.center().y())              // Left-center
        };
        
        // Draw each handle
        painter.setPen(QPen(QColor(0, 0, 0), 1)); // Black outline
        painter.setBrush(QBrush(QColor(255, 255, 255))); // White fill
        
        for (const QPointF& handle : handles) {
            QRectF handleRect(
                handle.x() - halfHandle,
                handle.y() - halfHandle,
                handleSize,
                handleSize
            );
            painter.drawRect(handleRect);
        }
    }
    
    // Restore painter state (removes rotation transformation)
    painter.restore();
}

void CanvasRenderer::drawRegionLabel(QPainter& painter,
                                     const QString& name,
                                     const QRectF& rect,
                                     const QColor& color,
                                     bool isSelected) {
    // Save painter state
    painter.save();
    
    // Set font
    QFont font("Arial", isSelected ? 10 : 8);
    font.setBold(isSelected);
    painter.setFont(font);
    
    // Set pen color to match region color
    painter.setPen(QPen(color));
    
    // Position label above region, centered horizontally
    QPointF labelPos(
        rect.center().x(),
        rect.top() - 5  // 5 pixels above region
    );
    
    // Draw text
    painter.drawText(labelPos, name);
    
    // Restore painter state
    painter.restore();
}

void CanvasRenderer::drawTempRectangle(QPainter& painter, const QRectF& rect) {
    // Save painter state
    painter.save();
    
    // Draw red dashed outline (bright red, 2px width)
    QPen pen(QColor(255, 0, 0), 2);
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    // Draw rectangle
    painter.drawRect(rect);
    
    // Restore painter state
    painter.restore();
}

void CanvasRenderer::drawTempShape(QPainter& painter, const QRectF& rect, const QString& shapeType) {
    // Save painter state
    painter.save();
    
    // Draw red dashed outline (bright red, 2px width)
    QPen pen(QColor(255, 0, 0), 2);
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    // Draw shape based on type
    QString shape = shapeType.isEmpty() ? "rect" : shapeType;
    if (shape == "circle" || shape == "ellipse") {
        painter.drawEllipse(rect);
    } else if (shape == "triangle") {
        // Draw triangle: top center, bottom left, bottom right
        QPointF top(rect.center().x(), rect.top());
        QPointF bottomLeft(rect.left(), rect.bottom());
        QPointF bottomRight(rect.right(), rect.bottom());
        QPolygonF triangle;
        triangle << top << bottomLeft << bottomRight;
        painter.drawPolygon(triangle);
    } else {
        // Default: rectangle
        painter.drawRect(rect);
    }
    
    // Restore painter state
    painter.restore();
}

void CanvasRenderer::drawSelectionBox(QPainter& painter, const QRectF& rect) {
    // Save painter state
    painter.save();
    
    // Draw blue dashed outline for selection box
    QPen pen(QColor(0x00, 0x66, 0xff), 2); // Blue
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    
    // Draw semi-transparent fill
    QColor fillColor(0x00, 0x66, 0xff, 30); // Blue with low opacity
    painter.setBrush(QBrush(fillColor));
    
    // Draw rectangle
    painter.drawRect(rect);
    
    // Restore painter state
    painter.restore();
}

QColor CanvasRenderer::getRegionColor(const QString& colorName) const {
    // Color map matching UI specification
    static QMap<QString, QColor> colorMap = {
        {"blue", QColor(0x00, 0x66, 0xff)},
        {"red", QColor(0xff, 0x00, 0x00)},
        {"green", QColor(0x00, 0xcc, 0x00)},
        {"yellow", QColor(0xff, 0xcc, 0x00)},
        {"purple", QColor(0x99, 0x00, 0xcc)},
        {"orange", QColor(0xff, 0x66, 0x00)},
        {"cyan", QColor(0x00, 0xcc, 0xcc)}
    };
    
    // Return color from map, or default to blue
    if (colorMap.contains(colorName)) {
        return colorMap[colorName];
    }
    return colorMap["blue"]; // Default
}

} // namespace ocr_orc

