#include "CanvasRegionOperations.h"
#include "../../../../core/Constants.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

CanvasRegionOperations::CanvasRegionOperations() {
}

CanvasRegionOperations::~CanvasRegionOperations() {
}

void CanvasRegionOperations::moveRegion(const QString& regionName,
                                        const QPointF& delta,
                                        DocumentState* documentState,
                                        const QImage& documentImage,
                                        double scaleFactor,
                                        const QPointF& imageOffset) {
    if (!documentState || !documentState->hasRegion(regionName)) {
        return;
    }
    
    RegionData region = documentState->getRegion(regionName);
    
    // DISABLED FOR PRODUCTION: Verbose region operation logging
    // OCR_ORC_DEBUG("=== moveRegion ===");
    // OCR_ORC_DEBUG("  regionName:" << regionName);
    // OCR_ORC_DEBUG("  delta (canvas):" << delta);
    // OCR_ORC_DEBUG("  scaleFactor:" << scaleFactor);
    
    // Convert delta from canvas to image coordinates
    // delta_canvas = delta_image * scaleFactor
    // delta_image = delta_canvas / scaleFactor
    if (std::abs(scaleFactor) < CoordinateConstants::EPSILON) {
        return; // Avoid division by zero
    }
    
    QPointF imageDelta(delta.x() / scaleFactor, delta.y() / scaleFactor);
    
    // Convert delta from image to normalized coordinates
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    if (!CoordinateSystem::isValidImageDimensions(imgWidth, imgHeight)) {
        return;
    }
    
    QPointF normalizedDelta(
        imageDelta.x() / static_cast<double>(imgWidth),
        imageDelta.y() / static_cast<double>(imgHeight)
    );
    
    // OCR_ORC_DEBUG("  imageDelta:" << imageDelta);
    // OCR_ORC_DEBUG("  normalizedDelta:" << normalizedDelta);
    // OCR_ORC_DEBUG("  old normalized:" << region.normalizedCoords.x1 << "," << region.normalizedCoords.y1 << " to " << region.normalizedCoords.x2 << "," << region.normalizedCoords.y2);
    
    // Update normalized coordinates (source of truth)
    region.normalizedCoords.x1 += normalizedDelta.x();
    region.normalizedCoords.y1 += normalizedDelta.y();
    region.normalizedCoords.x2 += normalizedDelta.x();
    region.normalizedCoords.y2 += normalizedDelta.y();
    
    // Constrain to bounds
    constrainToBounds(region);
    
    // Synchronize all coordinates from normalized
    region.syncFromNormalized(imgWidth, imgHeight, scaleFactor, imageOffset);
    
    // OCR_ORC_DEBUG("  new normalized:" << region.normalizedCoords.x1 << "," << region.normalizedCoords.y1 << " to " << region.normalizedCoords.x2 << "," << region.normalizedCoords.y2);
    
    // Update in DocumentState
    documentState->addRegion(regionName, region);
}

void CanvasRegionOperations::moveRegions(const QList<QString>& regionNames,
                                         const QPointF& delta,
                                         DocumentState* documentState,
                                         const QImage& documentImage,
                                         double scaleFactor,
                                         const QPointF& imageOffset) {
    for (const QString& regionName : regionNames) {
        moveRegion(regionName, delta, documentState, documentImage,
                  scaleFactor, imageOffset);
    }
}

void CanvasRegionOperations::resizeRegion(const QString& regionName,
                                         const QString& handle,
                                         const QPointF& newPos,
                                         const QRectF& resizeStartRect,
                                         DocumentState* documentState,
                                         const QImage& documentImage,
                                         double scaleFactor,
                                         const QPointF& imageOffset,
                                         const QRectF& imageRect,
                                         double rotationAngle,
                                         const NormalizedCoords& originalNormalizedCoords) {
    OCR_ORC_DEBUG("[RESIZE] ========================================");
    OCR_ORC_DEBUG("[RESIZE] === resizeRegion START ===");
    OCR_ORC_DEBUG("[RESIZE]   INPUT PARAMETERS:");
    OCR_ORC_DEBUG("[RESIZE]     regionName: " << regionName);
    OCR_ORC_DEBUG("[RESIZE]     handle: " << handle);
    OCR_ORC_DEBUG("[RESIZE]     newPos (canvas): (" << newPos.x() << "," << newPos.y() << ")");
    OCR_ORC_DEBUG("[RESIZE]     resizeStartRect (canvas): (" << resizeStartRect.left() << "," << resizeStartRect.top() << ") size: " << resizeStartRect.width() << "x" << resizeStartRect.height());
    OCR_ORC_DEBUG("[RESIZE]     rotationAngle: " << rotationAngle << "degrees");
    OCR_ORC_DEBUG("[RESIZE]     scaleFactor: " << scaleFactor);
    OCR_ORC_DEBUG("[RESIZE]     imageOffset: (" << imageOffset.x() << "," << imageOffset.y() << ")");
    OCR_ORC_DEBUG("[RESIZE]     imageRect: (" << imageRect.left() << "," << imageRect.top() << ") size: " << imageRect.width() << "x" << imageRect.height());
    
    if (!documentState || !documentState->hasRegion(regionName)) {
        OCR_ORC_WARNING("  Invalid documentState or region not found");
        return;
    }
    
    // Validate resize handle string
    // Note: Handle names match CanvasHitTester::getHandleAt() which returns:
    // "nw", "n", "ne", "e", "se", "s", "sw", "w"
    static const QStringList validHandles = {
        "nw", "n", "ne", "e", "se", "s", "sw", "w"
    };
    if (handle.isEmpty() || !validHandles.contains(handle)) {
        OCR_ORC_WARNING("  Invalid handle: " << handle);
        return; // Invalid handle
    }
    
    RegionData region = documentState->getRegion(regionName);
    
    OCR_ORC_DEBUG("[RESIZE]   CURRENT REGION STATE:");
    OCR_ORC_DEBUG("[RESIZE]     normalizedCoords: x1=" << region.normalizedCoords.x1 << " y1=" << region.normalizedCoords.y1 << " x2=" << region.normalizedCoords.x2 << " y2=" << region.normalizedCoords.y2);
    OCR_ORC_DEBUG("[RESIZE]     rotationAngle (stored): " << region.rotationAngle << "degrees");
    
    // CRITICAL FIX: Use the ORIGINAL unrotated rectangle from when resize started,
    // not the current one (which changes on each resize move). This prevents the region
    // from drifting or all sides changing.
    
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    OCR_ORC_DEBUG("[RESIZE]   COORDINATE TRANSFORMATION:");
    OCR_ORC_DEBUG("[RESIZE]     imgWidth: " << imgWidth << " imgHeight: " << imgHeight);
    
    // Use ORIGINAL normalized coords (from when resize started) to calculate original unrotated rect
    NormalizedCoords norm = originalNormalizedCoords;
    OCR_ORC_DEBUG("[RESIZE]   Using ORIGINAL normalized coords: x1=" << norm.x1 << " y1=" << norm.y1 << " x2=" << norm.x2 << " y2=" << norm.y2);
    ImageCoords imgCoords = CoordinateSystem::normalizedToImage(norm, imgWidth, imgHeight);
    OCR_ORC_DEBUG("[RESIZE]     Normalized -> Image: (" << imgCoords.x1 << "," << imgCoords.y1 << ") to (" << imgCoords.x2 << "," << imgCoords.y2 << ")");
    
    CanvasCoords canvasCoords = CoordinateSystem::imageToCanvas(imgCoords, scaleFactor, imageOffset);
    OCR_ORC_DEBUG("[RESIZE]     Image -> Canvas: (" << canvasCoords.x1 << "," << canvasCoords.y1 << ") to (" << canvasCoords.x2 << "," << canvasCoords.y2 << ")");
    
    // This is the unrotated rectangle (normalized coords represent unrotated rectangle)
    QRectF unrotatedRect(
        canvasCoords.x1, canvasCoords.y1,
        canvasCoords.x2 - canvasCoords.x1,
        canvasCoords.y2 - canvasCoords.y1
    );
    OCR_ORC_DEBUG("[RESIZE]     unrotatedRect (canvas): (" << unrotatedRect.left() << "," << unrotatedRect.top() << ") size: " << unrotatedRect.width() << "x" << unrotatedRect.height());
    OCR_ORC_DEBUG("[RESIZE]     unrotatedRect center: (" << unrotatedRect.center().x() << "," << unrotatedRect.center().y() << ")");
    
    OCR_ORC_DEBUG("[RESIZE]   RESIZE OPERATION:");
    OCR_ORC_DEBUG("[RESIZE]     Before resize - unrotatedRect: (" << unrotatedRect.left() << "," << unrotatedRect.top() << ") size: " << unrotatedRect.width() << "x" << unrotatedRect.height());
    OCR_ORC_DEBUG("[RESIZE]     Handle: " << handle);
    OCR_ORC_DEBUG("[RESIZE]     newPos (canvas): (" << newPos.x() << "," << newPos.y() << ")");
    
    QRectF newRect = unrotatedRect;
    
    if (rotationAngle != 0.0) {
        // Force output to stderr for debugging
        std::cerr << "[RESIZE-ROT] Angle: " << rotationAngle << "° newPos: (" << newPos.x() << "," << newPos.y() << ")" << std::endl;
        
        OCR_ORC_DEBUG("[RESIZE]   ROTATED RESIZE - ANCHOR-BASED approach");
        OCR_ORC_DEBUG("[RESIZE]     Rotation angle: " << rotationAngle << " degrees");
        
        double angleRad = rotationAngle * M_PI / 180.0;
        double cosA = std::cos(angleRad);
        double sinA = std::sin(angleRad);
        QPointF oldCenter = unrotatedRect.center();
        
        OCR_ORC_DEBUG("[RESIZE]     Rotation matrix: cos=" << cosA << " sin=" << sinA);
        OCR_ORC_DEBUG("[RESIZE]     Old center: (" << oldCenter.x() << "," << oldCenter.y() << ")");
        
        // Check if this is a corner handle (use anchor-based approach)
        bool isCornerHandle = (handle == "nw" || handle == "ne" || 
                               handle == "sw" || handle == "se");
        
        if (isCornerHandle) {
            OCR_ORC_DEBUG("[RESIZE]     CORNER HANDLE - using anchor-based resize");
            
            // Step 1: Determine the ANCHOR corner (opposite to dragged handle)
            // The anchor stays fixed in VISUAL space
            QPointF anchorUnrotated;
        if (handle == "nw") {
                anchorUnrotated = unrotatedRect.bottomRight();  // Opposite corner
        } else if (handle == "ne") {
                anchorUnrotated = unrotatedRect.bottomLeft();
        } else if (handle == "sw") {
                anchorUnrotated = unrotatedRect.topRight();
            } else { // handle == "se"
                anchorUnrotated = unrotatedRect.topLeft();
        }
            
            OCR_ORC_DEBUG("[RESIZE]     Step 1 - Anchor (opposite corner):");
            OCR_ORC_DEBUG("[RESIZE]       Anchor unrotated: (" << anchorUnrotated.x() << "," << anchorUnrotated.y() << ")");
        
            // Step 2: Rotate anchor to get its VISUAL position (where it appears on screen)
            QPointF anchorTranslated = anchorUnrotated - oldCenter;
            QPointF anchorVisual = QPointF(
                anchorTranslated.x() * cosA - anchorTranslated.y() * sinA,
                anchorTranslated.x() * sinA + anchorTranslated.y() * cosA
            ) + oldCenter;
            
            OCR_ORC_DEBUG("[RESIZE]       Anchor visual: (" << anchorVisual.x() << "," << anchorVisual.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Dragged to (newPos): (" << newPos.x() << "," << newPos.y() << ")");
        
            // Step 3: New center = midpoint of anchor visual and dragged position
            // KEY: The center is invariant under rotation (rotating around itself keeps it fixed)
            // So newCenter in visual space = newCenter in unrotated space
            QPointF newCenter = (anchorVisual + newPos) / 2.0;
            OCR_ORC_DEBUG("[RESIZE]     Step 2 - New center (midpoint):");
            OCR_ORC_DEBUG("[RESIZE]       New center: (" << newCenter.x() << "," << newCenter.y() << ")");
            
            // Step 4: Calculate half-diagonal in visual space (from anchor to newPos)
            QPointF halfDiagVisual = (newPos - anchorVisual) / 2.0;
            OCR_ORC_DEBUG("[RESIZE]     Step 3 - Half-diagonal:");
            OCR_ORC_DEBUG("[RESIZE]       Half-diag visual: (" << halfDiagVisual.x() << "," << halfDiagVisual.y() << ")");
        
            // Step 5: Transform half-diagonal to unrotated space (inverse rotation)
            // halfDiagVisual points from anchor to dragged, so half of it points from center to dragged
            QPointF halfDiagUnrot = QPointF(
                halfDiagVisual.x() * cosA + halfDiagVisual.y() * sinA,
                -halfDiagVisual.x() * sinA + halfDiagVisual.y() * cosA
            );
            OCR_ORC_DEBUG("[RESIZE]       Half-diag unrot: (" << halfDiagUnrot.x() << "," << halfDiagUnrot.y() << ")");
            
            // Step 6: Compute both corners in unrotated space from center and half-diagonal
            // draggedUnrot = center + halfDiagUnrot (half diag points from anchor to dragged)
            // anchorUnrot = center - halfDiagUnrot
            QPointF draggedUnrot = newCenter + halfDiagUnrot;
            QPointF anchorUnrot_new = newCenter - halfDiagUnrot;
            
            OCR_ORC_DEBUG("[RESIZE]       Dragged corner unrot: (" << draggedUnrot.x() << "," << draggedUnrot.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Anchor corner unrot: (" << anchorUnrot_new.x() << "," << anchorUnrot_new.y() << ")");
        
            // Build rectangle from the two corners (handles any orientation)
            double minX = std::min(draggedUnrot.x(), anchorUnrot_new.x());
            double minY = std::min(draggedUnrot.y(), anchorUnrot_new.y());
            double maxX = std::max(draggedUnrot.x(), anchorUnrot_new.x());
            double maxY = std::max(draggedUnrot.y(), anchorUnrot_new.y());
            
            newRect = QRectF(minX, minY, maxX - minX, maxY - minY);
            
            OCR_ORC_DEBUG("[RESIZE]     Step 4 - New rectangle:");
            OCR_ORC_DEBUG("[RESIZE]       New rect: (" << newRect.left() << "," << newRect.top() 
                          << ") size: " << newRect.width() << "x" << newRect.height());
            
            // Verification: check that anchor lands at original position
            QPointF newAnchorUnrot;
            if (handle == "nw") newAnchorUnrot = newRect.bottomRight();
            else if (handle == "ne") newAnchorUnrot = newRect.bottomLeft();
            else if (handle == "sw") newAnchorUnrot = newRect.topRight();
            else newAnchorUnrot = newRect.topLeft();  // se
            
            QPointF verifyTranslated = newAnchorUnrot - newCenter;
            QPointF verifyAnchorVisual = QPointF(
                verifyTranslated.x() * cosA - verifyTranslated.y() * sinA,
                verifyTranslated.x() * sinA + verifyTranslated.y() * cosA
            ) + newCenter;
            
            double anchorError = std::sqrt(
                std::pow(verifyAnchorVisual.x() - anchorVisual.x(), 2) +
                std::pow(verifyAnchorVisual.y() - anchorVisual.y(), 2)
            );
            std::cerr << "[RESIZE-ROT] AnchorErr: " << anchorError << "px (anchor@" << anchorVisual.x() << "," << anchorVisual.y() << " -> " << verifyAnchorVisual.x() << "," << verifyAnchorVisual.y() << ")" << std::endl;
            OCR_ORC_DEBUG("[RESIZE]     VERIFICATION:");
            OCR_ORC_DEBUG("[RESIZE]       Anchor should be at: (" << anchorVisual.x() << "," << anchorVisual.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Anchor is at: (" << verifyAnchorVisual.x() << "," << verifyAnchorVisual.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Anchor error: " << anchorError << " pixels (should be ~0)");
        
            // Verify dragged corner reaches target
            QPointF newDraggedUnrot;
            if (handle == "nw") newDraggedUnrot = newRect.topLeft();
            else if (handle == "ne") newDraggedUnrot = newRect.topRight();
            else if (handle == "sw") newDraggedUnrot = newRect.bottomLeft();
            else newDraggedUnrot = newRect.bottomRight();  // se
            
            QPointF draggedTranslated = newDraggedUnrot - newCenter;
            QPointF verifyDraggedVisual = QPointF(
                draggedTranslated.x() * cosA - draggedTranslated.y() * sinA,
                draggedTranslated.x() * sinA + draggedTranslated.y() * cosA
            ) + newCenter;
            
            double draggedError = std::sqrt(
                std::pow(verifyDraggedVisual.x() - newPos.x(), 2) +
                std::pow(verifyDraggedVisual.y() - newPos.y(), 2)
            );
            OCR_ORC_DEBUG("[RESIZE]       Dragged should be at: (" << newPos.x() << "," << newPos.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Dragged is at: (" << verifyDraggedVisual.x() << "," << verifyDraggedVisual.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Dragged error: " << draggedError << " pixels (should be ~0)");
            
        } else {
            // Edge handles - keep opposite EDGE fixed in visual space
            OCR_ORC_DEBUG("[RESIZE]     EDGE HANDLE - using constrained anchor-based resize");
            
            // For edge handles, we need to keep the opposite edge fixed
            // This is more constrained than corner handles
            
            // Get the opposite edge midpoint in unrotated space
            QPointF oppositeEdgeMidUnrot;
            bool isVerticalEdge = (handle == "e" || handle == "w");
            
            if (handle == "n") {
                oppositeEdgeMidUnrot = QPointF(unrotatedRect.center().x(), unrotatedRect.bottom());
        } else if (handle == "s") {
                oppositeEdgeMidUnrot = QPointF(unrotatedRect.center().x(), unrotatedRect.top());
        } else if (handle == "e") {
                oppositeEdgeMidUnrot = QPointF(unrotatedRect.left(), unrotatedRect.center().y());
            } else { // handle == "w"
                oppositeEdgeMidUnrot = QPointF(unrotatedRect.right(), unrotatedRect.center().y());
        }
        
            // Rotate to get visual position of opposite edge midpoint
            QPointF oppTranslated = oppositeEdgeMidUnrot - oldCenter;
            QPointF oppositeEdgeVisual = QPointF(
                oppTranslated.x() * cosA - oppTranslated.y() * sinA,
                oppTranslated.x() * sinA + oppTranslated.y() * cosA
            ) + oldCenter;
            
            OCR_ORC_DEBUG("[RESIZE]       Opposite edge mid (unrot): (" << oppositeEdgeMidUnrot.x() << "," << oppositeEdgeMidUnrot.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Opposite edge mid (visual): (" << oppositeEdgeVisual.x() << "," << oppositeEdgeVisual.y() << ")");
            
            // Get the dragged edge midpoint visual position
            QPointF draggedEdgeMidUnrot;
            if (handle == "n") {
                draggedEdgeMidUnrot = QPointF(unrotatedRect.center().x(), unrotatedRect.top());
        } else if (handle == "s") {
                draggedEdgeMidUnrot = QPointF(unrotatedRect.center().x(), unrotatedRect.bottom());
        } else if (handle == "e") {
                draggedEdgeMidUnrot = QPointF(unrotatedRect.right(), unrotatedRect.center().y());
            } else { // handle == "w"
                draggedEdgeMidUnrot = QPointF(unrotatedRect.left(), unrotatedRect.center().y());
        }
        
            QPointF dragTranslated = draggedEdgeMidUnrot - oldCenter;
            QPointF draggedEdgeVisual = QPointF(
                dragTranslated.x() * cosA - dragTranslated.y() * sinA,
                dragTranslated.x() * sinA + dragTranslated.y() * cosA
            ) + oldCenter;
            
            OCR_ORC_DEBUG("[RESIZE]       Dragged edge mid (visual): (" << draggedEdgeVisual.x() << "," << draggedEdgeVisual.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       newPos: (" << newPos.x() << "," << newPos.y() << ")");
        
            // Calculate the direction perpendicular to the edge (in visual space)
            // For horizontal edges (n/s), perpendicular is the rotated Y axis
            // For vertical edges (e/w), perpendicular is the rotated X axis
            QPointF perpDir;
            if (isVerticalEdge) {
                // Rotated X axis direction
                perpDir = QPointF(cosA, sinA);
        } else {
                // Rotated Y axis direction
                perpDir = QPointF(-sinA, cosA);
            }
            
            // Project newPos onto the line from opposite edge through the perpendicular
            // Calculate how far along the perpendicular direction newPos is from opposite edge
            QPointF toNewPos = newPos - oppositeEdgeVisual;
            double perpDistance = toNewPos.x() * perpDir.x() + toNewPos.y() * perpDir.y();
            
            OCR_ORC_DEBUG("[RESIZE]       Perpendicular direction: (" << perpDir.x() << "," << perpDir.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       Perpendicular distance: " << perpDistance);
            
            // The new edge midpoint in visual space
            QPointF newDraggedEdgeVisual = oppositeEdgeVisual + perpDir * perpDistance;
            
            // New center is midpoint between opposite edge and new dragged edge
            QPointF newCenter = (oppositeEdgeVisual + newDraggedEdgeVisual) / 2.0;
            
            OCR_ORC_DEBUG("[RESIZE]       New dragged edge (visual): (" << newDraggedEdgeVisual.x() << "," << newDraggedEdgeVisual.y() << ")");
            OCR_ORC_DEBUG("[RESIZE]       New center: (" << newCenter.x() << "," << newCenter.y() << ")");
            
            // Calculate new dimensions
            // The dimension perpendicular to the edge changes; the parallel dimension stays the same
            double newPerpDim = std::abs(perpDistance);
            
            if (isVerticalEdge) {
                // Width changes, height stays the same
                double newWidth = newPerpDim;
                double newHeight = unrotatedRect.height();
                newRect = QRectF(
                    newCenter.x() - newWidth / 2.0,
                    newCenter.y() - newHeight / 2.0,
                    newWidth,
                    newHeight
                );
            } else {
                // Height changes, width stays the same
                double newWidth = unrotatedRect.width();
                double newHeight = newPerpDim;
                newRect = QRectF(
                    newCenter.x() - newWidth / 2.0,
                    newCenter.y() - newHeight / 2.0,
                    newWidth,
                    newHeight
                );
            }
            
            OCR_ORC_DEBUG("[RESIZE]       New rect: (" << newRect.left() << "," << newRect.top() 
                          << ") size: " << newRect.width() << "x" << newRect.height());
        }
        
    } else {
        // No rotation - simple resize
        OCR_ORC_DEBUG("[RESIZE]   NO ROTATION - simple resize");
        OCR_ORC_DEBUG("[RESIZE]     Step 1 - Handle position:");
        
        QPointF originalHandlePos;
        if (handle == "nw") {
            originalHandlePos = unrotatedRect.topLeft();
        } else if (handle == "ne") {
            originalHandlePos = unrotatedRect.topRight();
        } else if (handle == "sw") {
            originalHandlePos = unrotatedRect.bottomLeft();
        } else if (handle == "se") {
            originalHandlePos = unrotatedRect.bottomRight();
        } else if (handle == "n") {
            originalHandlePos = QPointF(unrotatedRect.center().x(), unrotatedRect.top());
        } else if (handle == "s") {
            originalHandlePos = QPointF(unrotatedRect.center().x(), unrotatedRect.bottom());
        } else if (handle == "e") {
            originalHandlePos = QPointF(unrotatedRect.right(), unrotatedRect.center().y());
        } else if (handle == "w") {
            originalHandlePos = QPointF(unrotatedRect.left(), unrotatedRect.center().y());
        }
        OCR_ORC_DEBUG("[RESIZE]       Handle: " << handle);
        OCR_ORC_DEBUG("[RESIZE]       Original handle position: (" << originalHandlePos.x() << "," << originalHandlePos.y() << ")");
        
        QPointF delta = newPos - originalHandlePos;
        double deltaMagnitude = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
        OCR_ORC_DEBUG("[RESIZE]     Step 2 - Delta calculation:");
        OCR_ORC_DEBUG("[RESIZE]       newPos: (" << newPos.x() << "," << newPos.y() << ")");
        OCR_ORC_DEBUG("[RESIZE]       Delta: (" << delta.x() << "," << delta.y() << ")");
        OCR_ORC_DEBUG("[RESIZE]       Delta magnitude: " << deltaMagnitude);
        
        OCR_ORC_DEBUG("[RESIZE]     Step 3 - Rectangle update:");
        OCR_ORC_DEBUG("[RESIZE]       Original rect: (" << unrotatedRect.left() << "," << unrotatedRect.top() << ") size: " << unrotatedRect.width() << "x" << unrotatedRect.height());
        
        QPointF oppositeCornerBefore;
        if (handle == "nw") {
            newRect.setTopLeft(originalHandlePos + delta);
            oppositeCornerBefore = unrotatedRect.bottomRight();
        } else if (handle == "ne") {
            newRect.setTopRight(originalHandlePos + delta);
            oppositeCornerBefore = unrotatedRect.bottomLeft();
        } else if (handle == "sw") {
            newRect.setBottomLeft(originalHandlePos + delta);
            oppositeCornerBefore = unrotatedRect.topRight();
        } else if (handle == "se") {
            newRect.setBottomRight(originalHandlePos + delta);
            oppositeCornerBefore = unrotatedRect.topLeft();
        } else if (handle == "n") {
            newRect.setTop(unrotatedRect.top() + delta.y());
            oppositeCornerBefore = QPointF(unrotatedRect.center().x(), unrotatedRect.bottom());
        } else if (handle == "s") {
            newRect.setBottom(unrotatedRect.bottom() + delta.y());
            oppositeCornerBefore = QPointF(unrotatedRect.center().x(), unrotatedRect.top());
        } else if (handle == "e") {
            newRect.setRight(unrotatedRect.right() + delta.x());
            oppositeCornerBefore = QPointF(unrotatedRect.left(), unrotatedRect.center().y());
        } else if (handle == "w") {
            newRect.setLeft(unrotatedRect.left() + delta.x());
            oppositeCornerBefore = QPointF(unrotatedRect.right(), unrotatedRect.center().y());
        }
        
        QPointF oppositeCornerAfter;
        if (handle == "nw") {
            oppositeCornerAfter = newRect.bottomRight();
        } else if (handle == "ne") {
            oppositeCornerAfter = newRect.bottomLeft();
        } else if (handle == "sw") {
            oppositeCornerAfter = newRect.topRight();
        } else if (handle == "se") {
            oppositeCornerAfter = newRect.topLeft();
        } else if (handle == "n") {
            oppositeCornerAfter = QPointF(newRect.center().x(), newRect.bottom());
        } else if (handle == "s") {
            oppositeCornerAfter = QPointF(newRect.center().x(), newRect.top());
        } else if (handle == "e") {
            oppositeCornerAfter = QPointF(newRect.left(), newRect.center().y());
        } else if (handle == "w") {
            oppositeCornerAfter = QPointF(newRect.right(), newRect.center().y());
        }
        
        OCR_ORC_DEBUG("[RESIZE]       New rect: (" << newRect.left() << "," << newRect.top() << ") size: " << newRect.width() << "x" << newRect.height());
        OCR_ORC_DEBUG("[RESIZE]       Size change: width=" << (newRect.width() - unrotatedRect.width()) << " height=" << (newRect.height() - unrotatedRect.height()));
        OCR_ORC_DEBUG("[RESIZE]       Opposite corner (should be fixed):");
        OCR_ORC_DEBUG("[RESIZE]         Before: (" << oppositeCornerBefore.x() << "," << oppositeCornerBefore.y() << ")");
        OCR_ORC_DEBUG("[RESIZE]         After: (" << oppositeCornerAfter.x() << "," << oppositeCornerAfter.y() << ")");
        OCR_ORC_DEBUG("[RESIZE]         Difference: (" << (oppositeCornerAfter.x() - oppositeCornerBefore.x()) << "," << (oppositeCornerAfter.y() - oppositeCornerBefore.y()) << ")");
    }
    
    OCR_ORC_DEBUG("[RESIZE]     After resize - newRect: (" << newRect.left() << "," << newRect.top() << ") size: " << newRect.width() << "x" << newRect.height());
    OCR_ORC_DEBUG("[RESIZE]     Size change: width=" << (newRect.width() - unrotatedRect.width()) << " height=" << (newRect.height() - unrotatedRect.height()));
    OCR_ORC_DEBUG("[RESIZE]     Corner positions:");
    OCR_ORC_DEBUG("[RESIZE]       Top-left: (" << newRect.left() << "," << newRect.top() << ") [was: (" << unrotatedRect.left() << "," << unrotatedRect.top() << ")]");
    OCR_ORC_DEBUG("[RESIZE]       Top-right: (" << newRect.right() << "," << newRect.top() << ") [was: (" << unrotatedRect.right() << "," << unrotatedRect.top() << ")]");
    OCR_ORC_DEBUG("[RESIZE]       Bottom-left: (" << newRect.left() << "," << newRect.bottom() << ") [was: (" << unrotatedRect.left() << "," << unrotatedRect.bottom() << ")]");
    OCR_ORC_DEBUG("[RESIZE]       Bottom-right: (" << newRect.right() << "," << newRect.bottom() << ") [was: (" << unrotatedRect.right() << "," << unrotatedRect.bottom() << ")]");
    
    // Normalize rectangle if it became inverted (Qt will do this, but we ensure it explicitly)
    if (newRect.width() < 0) {
        OCR_ORC_DEBUG("[RESIZE]     WARNING: Width became negative, normalizing...");
        newRect.setLeft(newRect.right());
        newRect.setRight(newRect.left() + std::abs(newRect.width()));
    }
    if (newRect.height() < 0) {
        OCR_ORC_DEBUG("[RESIZE]     WARNING: Height became negative, normalizing...");
        newRect.setTop(newRect.bottom());
        newRect.setBottom(newRect.top() + std::abs(newRect.height()));
    }
    
    // Ensure valid rectangle (width and height > 0)
    if (newRect.width() < RegionConstants::MIN_REGION_SIZE) {
        if (handle.contains("e")) {
            newRect.setRight(newRect.left() + RegionConstants::MIN_REGION_SIZE);
        } else {
            newRect.setLeft(newRect.right() - RegionConstants::MIN_REGION_SIZE);
        }
    }
    if (newRect.height() < RegionConstants::MIN_REGION_SIZE) {
        if (handle.contains("s")) {
            newRect.setBottom(newRect.top() + RegionConstants::MIN_REGION_SIZE);
        } else {
            newRect.setTop(newRect.bottom() - RegionConstants::MIN_REGION_SIZE);
        }
    }
    
    // Clamp to image bounds - use simple intersection
    newRect = newRect.intersected(imageRect);
    
    // Ensure minimum size after clamping
    if (newRect.width() < RegionConstants::MIN_REGION_SIZE) {
        if (newRect.left() + RegionConstants::MIN_REGION_SIZE <= imageRect.right()) {
            newRect.setRight(newRect.left() + RegionConstants::MIN_REGION_SIZE);
        } else {
            newRect.setLeft(newRect.right() - RegionConstants::MIN_REGION_SIZE);
        }
    }
    if (newRect.height() < RegionConstants::MIN_REGION_SIZE) {
        if (newRect.top() + RegionConstants::MIN_REGION_SIZE <= imageRect.bottom()) {
            newRect.setBottom(newRect.top() + RegionConstants::MIN_REGION_SIZE);
        } else {
            newRect.setTop(newRect.bottom() - RegionConstants::MIN_REGION_SIZE);
        }
    }
    
    // Convert back to normalized coordinates
    // imgWidth and imgHeight already declared above
    
    // Validate image dimensions before using in calculations
    if (!CoordinateSystem::isValidImageDimensions(imgWidth, imgHeight)) {
        return;
    }
    
    OCR_ORC_DEBUG("[RESIZE]   COORDINATE CONVERSION:");
    OCR_ORC_DEBUG("[RESIZE]     Step 1 - Canvas coordinates:");
    OCR_ORC_DEBUG("[RESIZE]       newRect before conversion: (" << newRect.left() << "," << newRect.top() << ") size: " << newRect.width() << "x" << newRect.height());
    
    CanvasCoords newCanvasCoords(
        newRect.left(), newRect.top(),
        newRect.right(), newRect.bottom()
    );
    OCR_ORC_DEBUG("[RESIZE]       newCanvasCoords: (" << newCanvasCoords.x1 << "," << newCanvasCoords.y1 << ") to (" << newCanvasCoords.x2 << "," << newCanvasCoords.y2 << ")");
    
    OCR_ORC_DEBUG("[RESIZE]     Step 2 - Canvas to Image conversion:");
    OCR_ORC_DEBUG("[RESIZE]       scaleFactor: " << scaleFactor);
    OCR_ORC_DEBUG("[RESIZE]       imageOffset: (" << imageOffset.x() << "," << imageOffset.y() << ")");
    
    ImageCoords newImageCoords = CoordinateSystem::canvasToImage(
        newCanvasCoords, scaleFactor, imageOffset
    );
    OCR_ORC_DEBUG("[RESIZE]       newImageCoords: (" << newImageCoords.x1 << "," << newImageCoords.y1 << ") to (" << newImageCoords.x2 << "," << newImageCoords.y2 << ")");
    OCR_ORC_DEBUG("[RESIZE]       Original imageCoords: (" << imgCoords.x1 << "," << imgCoords.y1 << ") to (" << imgCoords.x2 << "," << imgCoords.y2 << ")");
    int widthChange = (newImageCoords.x2 - newImageCoords.x1) - (imgCoords.x2 - imgCoords.x1);
    int heightChange = (newImageCoords.y2 - newImageCoords.y1) - (imgCoords.y2 - imgCoords.y1);
    OCR_ORC_DEBUG("[RESIZE]       Image size change: width=" << widthChange << " height=" << heightChange);
    
    OCR_ORC_DEBUG("[RESIZE]     Step 3 - Image to Normalized conversion:");
    OCR_ORC_DEBUG("[RESIZE]       imgWidth: " << imgWidth << " imgHeight: " << imgHeight);
    
    NormalizedCoords newNormalized = CoordinateSystem::imageToNormalized(
        newImageCoords, imgWidth, imgHeight
    );
    OCR_ORC_DEBUG("[RESIZE]       newNormalized: x1=" << newNormalized.x1 << " y1=" << newNormalized.y1 << " x2=" << newNormalized.x2 << " y2=" << newNormalized.y2);
    OCR_ORC_DEBUG("[RESIZE]       originalNormalizedCoords: x1=" << originalNormalizedCoords.x1 << " y1=" << originalNormalizedCoords.y1 << " x2=" << originalNormalizedCoords.x2 << " y2=" << originalNormalizedCoords.y2);
    
    OCR_ORC_DEBUG("[RESIZE]     Step 4 - Normalized coordinate changes:");
    OCR_ORC_DEBUG("[RESIZE]       x1 change: " << (newNormalized.x1 - originalNormalizedCoords.x1) << " [original: " << originalNormalizedCoords.x1 << " -> new: " << newNormalized.x1 << "]");
    OCR_ORC_DEBUG("[RESIZE]       y1 change: " << (newNormalized.y1 - originalNormalizedCoords.y1) << " [original: " << originalNormalizedCoords.y1 << " -> new: " << newNormalized.y1 << "]");
    OCR_ORC_DEBUG("[RESIZE]       x2 change: " << (newNormalized.x2 - originalNormalizedCoords.x2) << " [original: " << originalNormalizedCoords.x2 << " -> new: " << newNormalized.x2 << "]");
    OCR_ORC_DEBUG("[RESIZE]       y2 change: " << (newNormalized.y2 - originalNormalizedCoords.y2) << " [original: " << originalNormalizedCoords.y2 << " -> new: " << newNormalized.y2 << "]");
    
    OCR_ORC_DEBUG("[RESIZE]     Step 5 - Rotation angle preservation:");
    OCR_ORC_DEBUG("[RESIZE]       Original rotation angle: " << region.rotationAngle << " degrees");
    OCR_ORC_DEBUG("[RESIZE]       Rotation angle should remain: " << region.rotationAngle << " degrees");
    
    // Update region
    OCR_ORC_DEBUG("[RESIZE]   BEFORE constrainToBounds:");
    OCR_ORC_DEBUG("[RESIZE]     normalizedCoords: x1=" << newNormalized.x1 << " y1=" << newNormalized.y1 << " x2=" << newNormalized.x2 << " y2=" << newNormalized.y2);
    region.normalizedCoords = newNormalized;
    constrainToBounds(region);
    OCR_ORC_DEBUG("[RESIZE]   AFTER constrainToBounds:");
    OCR_ORC_DEBUG("[RESIZE]     normalizedCoords: x1=" << region.normalizedCoords.x1 << " y1=" << region.normalizedCoords.y1 << " x2=" << region.normalizedCoords.x2 << " y2=" << region.normalizedCoords.y2);
    
    region.syncFromNormalized(imgWidth, imgHeight, scaleFactor, imageOffset);
    
    OCR_ORC_DEBUG("[RESIZE]   FINAL STATE:");
    OCR_ORC_DEBUG("[RESIZE]     normalizedCoords: x1=" << region.normalizedCoords.x1 << " y1=" << region.normalizedCoords.y1 << " x2=" << region.normalizedCoords.x2 << " y2=" << region.normalizedCoords.y2);
    OCR_ORC_DEBUG("[RESIZE]     rotationAngle: " << region.rotationAngle << "degrees");
    
    // Update in DocumentState
    documentState->addRegion(regionName, region);
    OCR_ORC_DEBUG("[RESIZE] === resizeRegion COMPLETE ===");
    OCR_ORC_DEBUG("[RESIZE] ========================================");
}

void CanvasRegionOperations::constrainToBounds(RegionData& region) {
    // Clamp normalized coordinates to 0.0-1.0
    NormalizedCoords& norm = region.normalizedCoords;
    
    // Ensure x1 < x2 and y1 < y2 BEFORE clamping
    if (norm.x1 > norm.x2) std::swap(norm.x1, norm.x2);
    if (norm.y1 > norm.y2) std::swap(norm.y1, norm.y2);
    
    // Clamp to bounds
    norm.x1 = std::max(0.0, std::min(1.0, norm.x1));
    norm.y1 = std::max(0.0, std::min(1.0, norm.y1));
    norm.x2 = std::max(0.0, std::min(1.0, norm.x2));
    norm.y2 = std::max(0.0, std::min(1.0, norm.y2));
    
    // CRITICAL: Re-validate ordering after clamping
    // Clamping might have made x1 == x2 or y1 == y2, or even x1 > x2 if both were clamped to same value
    if (norm.x1 >= norm.x2) {
        // Region too small or invalid - enforce minimum size
        const double minSize = 0.001;
        if (norm.x1 >= 1.0 - minSize) {
            // At right boundary - move left
            norm.x1 = std::max(0.0, 1.0 - minSize);
            norm.x2 = 1.0;
        } else {
            // Expand to minimum size
            norm.x2 = std::min(1.0, norm.x1 + minSize);
        }
    }
    
    if (norm.y1 >= norm.y2) {
        const double minSize = 0.001;
        if (norm.y1 >= 1.0 - minSize) {
            // At bottom boundary - move up
            norm.y1 = std::max(0.0, 1.0 - minSize);
            norm.y2 = 1.0;
        } else {
            // Expand to minimum size
            norm.y2 = std::min(1.0, norm.y1 + minSize);
        }
    }
    
    // Final validation - ensure x1 < x2 and y1 < y2
    if (norm.x1 >= norm.x2 || norm.y1 >= norm.y2) {
        // Last resort: create minimal valid region at origin
        const double minSize = 0.001;
        norm.x1 = 0.0;
        norm.y1 = 0.0;
        norm.x2 = minSize;
        norm.y2 = minSize;
    }
    
    // Ensure minimum size (this may have been affected by the above fixes)
    const double minSize = 0.001;
    if (norm.x2 - norm.x1 < minSize) {
        norm.x2 = std::min(1.0, norm.x1 + minSize);
        if (norm.x2 > 1.0) {
            norm.x1 = std::max(0.0, norm.x2 - minSize);
        }
    }
    if (norm.y2 - norm.y1 < minSize) {
        norm.y2 = std::min(1.0, norm.y1 + minSize);
        if (norm.y2 > 1.0) {
            norm.y1 = std::max(0.0, norm.y2 - minSize);
        }
    }
}

bool CanvasRegionOperations::validateRegion(const QRectF& rect, const QRectF& imageRect) const {
    // Check minimum size
    if (rect.width() < RegionConstants::MIN_REGION_SIZE || rect.height() < RegionConstants::MIN_REGION_SIZE) {
        return false;
    }
    
    // Check if within image bounds
    if (!imageRect.contains(rect)) {
        return false;
    }
    
    return true;
}

void CanvasRegionOperations::rotateRegion(const QString& regionName,
                                         double rotationAngle,
                                         DocumentState* documentState,
                                         const QImage& documentImage,
                                         double scaleFactor,
                                         const QPointF& imageOffset) {
    OCR_ORC_DEBUG("=== rotateRegion START ===");
    OCR_ORC_DEBUG("  regionName:" << regionName);
    OCR_ORC_DEBUG("  rotationAngle:" << rotationAngle << "degrees");
    
    if (!documentState || !documentState->hasRegion(regionName)) {
        OCR_ORC_WARNING("  Invalid documentState or region not found:" << regionName);
        return;
    }
    
    // Validate rotation angle (check for NaN, Infinity, and reasonable range)
    if (!CoordinateSystem::isValidDouble(rotationAngle) || std::abs(rotationAngle) > 360.0) {
        OCR_ORC_WARNING("  Invalid rotation angle:" << rotationAngle);
        return; // Invalid angle
    }
    
    // Skip if rotation angle is negligible
    if (std::abs(rotationAngle) < 0.1) {
        OCR_ORC_DEBUG("  Rotation angle too small, skipping:" << rotationAngle);
        return;
    }
    
    RegionData region = documentState->getRegion(regionName);
    OCR_ORC_DEBUG("  Input normalized coords: x1=" << region.normalizedCoords.x1 
                  << " y1=" << region.normalizedCoords.y1 
                  << " x2=" << region.normalizedCoords.x2 
                  << " y2=" << region.normalizedCoords.y2);
    
    int imgWidth = documentImage.width();
    int imgHeight = documentImage.height();
    OCR_ORC_DEBUG("  Image dimensions: width=" << imgWidth << " height=" << imgHeight);
    
    // Validate image dimensions before using in calculations
    if (!CoordinateSystem::isValidImageDimensions(imgWidth, imgHeight)) {
        OCR_ORC_WARNING("  Invalid image dimensions");
        return;
    }
    
    // Validate input coordinates
    if (!CoordinateSystem::isValidNormalizedCoords(region.normalizedCoords)) {
        OCR_ORC_WARNING("  Invalid input coordinates");
        return; // Invalid input coordinates
    }
    
    // Get current region center in normalized coordinates
    double centerX = (region.normalizedCoords.x1 + region.normalizedCoords.x2) / 2.0;
    double centerY = (region.normalizedCoords.y1 + region.normalizedCoords.y2) / 2.0;
    OCR_ORC_DEBUG("  Normalized center: x=" << centerX << " y=" << centerY);
    
    // Validate center calculation
    if (!CoordinateSystem::isValidDouble(centerX) || !CoordinateSystem::isValidDouble(centerY)) {
        OCR_ORC_WARNING("  Invalid center calculation");
        return; // Invalid center
    }
    
    // Convert to image coordinates for rotation
    double imgCenterX = centerX * imgWidth;
    double imgCenterY = centerY * imgHeight;
    OCR_ORC_DEBUG("  Image center: x=" << imgCenterX << " y=" << imgCenterY);
    
    // Get current corners in image coordinates
    double x1 = region.normalizedCoords.x1 * imgWidth;
    double y1 = region.normalizedCoords.y1 * imgHeight;
    double x2 = region.normalizedCoords.x2 * imgWidth;
    double y2 = region.normalizedCoords.y2 * imgHeight;
    OCR_ORC_DEBUG("  Image corners: (" << x1 << "," << y1 << ") (" << x2 << "," << y2 << ")");
    
    // Convert angle to radians
    double angleRad = rotationAngle * M_PI / 180.0;
    double cosAngle = std::cos(angleRad);
    double sinAngle = std::sin(angleRad);
    OCR_ORC_DEBUG("  Angle in radians:" << angleRad << " cos=" << cosAngle << " sin=" << sinAngle);
    
    // Rotate each corner around center
    auto rotatePoint = [&](double x, double y) -> QPointF {
        double dx = x - imgCenterX;
        double dy = y - imgCenterY;
        double rotatedX = dx * cosAngle - dy * sinAngle + imgCenterX;
        double rotatedY = dx * sinAngle + dy * cosAngle + imgCenterY;
        OCR_ORC_DEBUG("    Rotated point: (" << x << "," << y << ") -> (" << rotatedX << "," << rotatedY << ")");
        return QPointF(rotatedX, rotatedY);
    };
    
    QPointF p1 = rotatePoint(x1, y1);
    QPointF p2 = rotatePoint(x2, y1);
    QPointF p3 = rotatePoint(x1, y2);
    QPointF p4 = rotatePoint(x2, y2);
    
    OCR_ORC_DEBUG("  Rotated corners: p1=(" << p1.x() << "," << p1.y() 
                  << ") p2=(" << p2.x() << "," << p2.y()
                  << ") p3=(" << p3.x() << "," << p3.y()
                  << ") p4=(" << p4.x() << "," << p4.y() << ")");
    
    // Validate rotated points before min/max calculation
    if (!CoordinateSystem::isValidPoint(p1) || !CoordinateSystem::isValidPoint(p2) ||
        !CoordinateSystem::isValidPoint(p3) || !CoordinateSystem::isValidPoint(p4)) {
        OCR_ORC_WARNING("  Invalid rotated points");
        return; // Invalid rotation result
    }
    
    // Find bounding box of rotated corners
    double minX = std::min({p1.x(), p2.x(), p3.x(), p4.x()});
    double maxX = std::max({p1.x(), p2.x(), p3.x(), p4.x()});
    double minY = std::min({p1.y(), p2.y(), p3.y(), p4.y()});
    double maxY = std::max({p1.y(), p2.y(), p3.y(), p4.y()});
    OCR_ORC_DEBUG("  Bounding box: minX=" << minX << " maxX=" << maxX 
                  << " minY=" << minY << " maxY=" << maxY);
    
    // Validate bounding box result
    if (!CoordinateSystem::isValidDouble(minX) || !CoordinateSystem::isValidDouble(maxX) ||
        !CoordinateSystem::isValidDouble(minY) || !CoordinateSystem::isValidDouble(maxY)) {
        OCR_ORC_WARNING("  Invalid bounding box");
        return; // Invalid bounding box
    }
    
    // Convert back to normalized coordinates
    NormalizedCoords newNorm;
    newNorm.x1 = minX / imgWidth;
    newNorm.y1 = minY / imgHeight;
    newNorm.x2 = maxX / imgWidth;
    newNorm.y2 = maxY / imgHeight;
    OCR_ORC_DEBUG("  Before clamping: x1=" << newNorm.x1 << " y1=" << newNorm.y1 
                  << " x2=" << newNorm.x2 << " y2=" << newNorm.y2);
    
    // Clamp to bounds
    newNorm.x1 = std::max(0.0, std::min(1.0, newNorm.x1));
    newNorm.y1 = std::max(0.0, std::min(1.0, newNorm.y1));
    newNorm.x2 = std::max(0.0, std::min(1.0, newNorm.x2));
    newNorm.y2 = std::max(0.0, std::min(1.0, newNorm.y2));
    OCR_ORC_DEBUG("  After clamping: x1=" << newNorm.x1 << " y1=" << newNorm.y1 
                  << " x2=" << newNorm.x2 << " y2=" << newNorm.y2);
    
    // Ensure minimum size
    const double minSize = 0.001;
    if (newNorm.x2 - newNorm.x1 < minSize) {
        OCR_ORC_DEBUG("  Enforcing minimum X size");
        newNorm.x2 = newNorm.x1 + minSize;
        if (newNorm.x2 > 1.0) {
            newNorm.x2 = 1.0;
            newNorm.x1 = newNorm.x2 - minSize;
        }
    }
    if (newNorm.y2 - newNorm.y1 < minSize) {
        OCR_ORC_DEBUG("  Enforcing minimum Y size");
        newNorm.y2 = newNorm.y1 + minSize;
        if (newNorm.y2 > 1.0) {
            newNorm.y2 = 1.0;
            newNorm.y1 = newNorm.y2 - minSize;
        }
    }
    
    // CRITICAL FIX: DO NOT update normalized coordinates when rotating!
    // Normalized coordinates should ALWAYS represent the unrotated rectangle.
    // Rotation is applied only during rendering.
    // This allows resize to work correctly on rotated regions.
    
    // Only update rotation angle (cumulative rotation)
    region.rotationAngle += rotationAngle;
    // Normalize to -180 to 180 range
    while (region.rotationAngle > 180.0) region.rotationAngle -= 360.0;
    while (region.rotationAngle < -180.0) region.rotationAngle += 360.0;
    
    OCR_ORC_DEBUG("  Stored rotation angle:" << region.rotationAngle << "degrees (cumulative)");
    OCR_ORC_DEBUG("  Normalized coords unchanged (unrotated rectangle): x1=" << region.normalizedCoords.x1 
                  << " y1=" << region.normalizedCoords.y1 
                  << " x2=" << region.normalizedCoords.x2 
                  << " y2=" << region.normalizedCoords.y2);
    
    // Sync coordinates (this will recalculate image/canvas from normalized)
    region.syncFromNormalized(imgWidth, imgHeight, scaleFactor, imageOffset);
    
    // Update in DocumentState
    documentState->addRegion(regionName, region);
    OCR_ORC_DEBUG("=== rotateRegion COMPLETE ===");
}

} // namespace ocr_orc

