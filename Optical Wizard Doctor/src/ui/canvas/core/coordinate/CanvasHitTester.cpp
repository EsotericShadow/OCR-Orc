#include "CanvasHitTester.h"
#include "../../../../models/RegionData.h"
#include "../../../../core/Constants.h"
#include <QtGui/QImage>
#include <cmath>

namespace ocr_orc {

CanvasHitTester::CanvasHitTester() {
}

CanvasHitTester::~CanvasHitTester() {
}

QString CanvasHitTester::getRegionAt(const QPointF& canvasPos,
                                     DocumentState* documentState,
                                     const QImage& documentImage,
                                     const QRectF& imageRect,
                                     double scaleFactor,
                                     const QPointF& imageOffset) const {
    if (!documentState || documentImage.isNull()) {
        return QString();
    }
    
    // Only check within image bounds
    if (!imageRect.contains(canvasPos)) {
        return QString();
    }
    
    // Get all region names (iterate in reverse to get topmost first)
    QList<QString> regionNames = documentState->getAllRegionNames();
    
    // Iterate through regions to find one containing the point
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    
    for (const QString& regionName : regionNames) {
        RegionData region = documentState->getRegion(regionName);
        
        // Convert normalized coordinates to canvas coordinates
        NormalizedCoords norm = region.normalizedCoords;
        CanvasCoords canvasCoords = CoordinateSystem::normalizedToCanvas(
            norm, imgWidth, imgHeight, scaleFactor, imageOffset
        );
        
        // Create QRectF from canvas coordinates
        QRectF canvasRect(
            canvasCoords.x1,
            canvasCoords.y1,
            canvasCoords.x2 - canvasCoords.x1,
            canvasCoords.y2 - canvasCoords.y1
        );
        
        // Check if point is inside region
        if (canvasRect.contains(canvasPos)) {
            return regionName;
        }
    }
    
    return QString(); // No region found
}

QString CanvasHitTester::getHandleAt(const QPointF& canvasPos, const QRectF& regionRect, double rotationAngle) const {
    OCR_ORC_DEBUG("[HIT TEST] === getHandleAt ===");
    OCR_ORC_DEBUG("  canvasPos: (" << canvasPos.x() << "," << canvasPos.y() << ")");
    OCR_ORC_DEBUG("  regionRect: (" << regionRect.left() << "," << regionRect.top() << ") size: " << regionRect.width() << "x" << regionRect.height());
    OCR_ORC_DEBUG("  rotationAngle: " << rotationAngle << "degrees");
    
    const double handleSize = RegionConstants::HANDLE_SIZE;
    const double halfHandle = handleSize / 2.0;
    const double tolerance = halfHandle + RegionConstants::HANDLE_TOLERANCE_OFFSET; // Slightly larger for easier clicking
    
    // Transform click point back to unrotated coordinate system if region is rotated
    QPointF testPos = canvasPos;
    if (rotationAngle != 0.0) {
        QPointF center = regionRect.center();
        double angleRad = -rotationAngle * M_PI / 180.0; // Negative to reverse rotation
        double cosAngle = std::cos(angleRad);
        double sinAngle = std::sin(angleRad);
        
        // Translate to origin, rotate, translate back
        QPointF translated = testPos - center;
        double rotatedX = translated.x() * cosAngle - translated.y() * sinAngle;
        double rotatedY = translated.x() * sinAngle + translated.y() * cosAngle;
        testPos = QPointF(rotatedX, rotatedY) + center;
        
        OCR_ORC_DEBUG("  Transformed testPos: (" << testPos.x() << "," << testPos.y() << ")");
    }
    
    // Check each handle position (in unrotated coordinates)
    QPointF handles[8] = {
        QPointF(regionRect.left(), regionRect.top()),                    // Top-left
        QPointF(regionRect.center().x(), regionRect.top()),              // Top-center
        QPointF(regionRect.right(), regionRect.top()),                   // Top-right
        QPointF(regionRect.right(), regionRect.center().y()),           // Right-center
        QPointF(regionRect.right(), regionRect.bottom()),                // Bottom-right
        QPointF(regionRect.center().x(), regionRect.bottom()),          // Bottom-center
        QPointF(regionRect.left(), regionRect.bottom()),                 // Bottom-left
        QPointF(regionRect.left(), regionRect.center().y())              // Left-center
    };
    
    QString handleNames[8] = {"nw", "n", "ne", "e", "se", "s", "sw", "w"};
    
    for (int i = 0; i < 8; i++) {
        QPointF delta = testPos - handles[i];
        double distance = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
        OCR_ORC_DEBUG("  Handle " << handleNames[i] << " at (" << handles[i].x() << "," << handles[i].y() << ") distance: " << distance);
        if (distance <= tolerance) {
            OCR_ORC_DEBUG("  ✓ Hit handle: " << handleNames[i]);
            return handleNames[i];
        }
    }
    
    OCR_ORC_DEBUG("  ✗ No handle found");
    return QString(); // No handle found
}

bool CanvasHitTester::isOnRotateIcon(const QPointF& canvasPos, const QRectF& regionRect, double iconSize, double rotationAngle) const {
    OCR_ORC_DEBUG("[HIT TEST] === isOnRotateIcon ===");
    OCR_ORC_DEBUG("  canvasPos: (" << canvasPos.x() << "," << canvasPos.y() << ")");
    OCR_ORC_DEBUG("  regionRect: (" << regionRect.left() << "," << regionRect.top() << ") size: " << regionRect.width() << "x" << regionRect.height());
    OCR_ORC_DEBUG("  rotationAngle: " << rotationAngle << "degrees");
    
    // Rotate icon is at bottom-right corner (in unrotated coordinates)
    QPointF rotatePos(regionRect.right(), regionRect.bottom());
    
    // Transform click point back to unrotated coordinate system if region is rotated
    QPointF testPos = canvasPos;
    if (rotationAngle != 0.0) {
        QPointF center = regionRect.center();
        double angleRad = -rotationAngle * M_PI / 180.0; // Negative to reverse rotation
        double cosAngle = std::cos(angleRad);
        double sinAngle = std::sin(angleRad);
        
        // Translate to origin, rotate, translate back
        QPointF translated = testPos - center;
        double rotatedX = translated.x() * cosAngle - translated.y() * sinAngle;
        double rotatedY = translated.x() * sinAngle + translated.y() * cosAngle;
        testPos = QPointF(rotatedX, rotatedY) + center;
        
        OCR_ORC_DEBUG("  Transformed testPos: (" << testPos.x() << "," << testPos.y() << ")");
    }
    
    const double halfIcon = iconSize / 2.0;
    const double tolerance = halfIcon + RegionConstants::HANDLE_TOLERANCE_OFFSET;
    
    QPointF delta = testPos - rotatePos;
    double distance = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
    OCR_ORC_DEBUG("  rotatePos: (" << rotatePos.x() << "," << rotatePos.y() << ") distance: " << distance << " tolerance: " << tolerance);
    bool hit = distance <= tolerance;
    OCR_ORC_DEBUG("  " << (hit ? "✓" : "✗") << " Hit rotate icon: " << hit);
    return hit;
}

} // namespace ocr_orc

