// Test file for CanvasRegionOperations
// Tests region manipulation operations

#include <QtTest/QtTest>
#include "../src/ui/canvas/core/regions/CanvasRegionOperations.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtGui/QImage>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <cmath>

using namespace ocr_orc;

// Helper function to rotate a point around a center
static QPointF rotatePoint(const QPointF& point, const QPointF& center, double angleDegrees) {
    double angleRad = angleDegrees * M_PI / 180.0;
    double cosA = std::cos(angleRad);
    double sinA = std::sin(angleRad);
    QPointF translated = point - center;
    return QPointF(
        translated.x() * cosA - translated.y() * sinA,
        translated.x() * sinA + translated.y() * cosA
    ) + center;
}

// Helper function to calculate expected rotated corner position
static QPointF calculateExpectedRotatedCorner(const QRectF& rect, const QString& handle, double angleDegrees) {
    QPointF center = rect.center();
    QPointF unrotatedCorner;
    
    if (handle == "nw") {
        unrotatedCorner = rect.topLeft();
    } else if (handle == "ne") {
        unrotatedCorner = rect.topRight();
    } else if (handle == "sw") {
        unrotatedCorner = rect.bottomLeft();
    } else if (handle == "se") {
        unrotatedCorner = rect.bottomRight();
    } else if (handle == "n") {
        unrotatedCorner = QPointF(rect.center().x(), rect.top());
    } else if (handle == "s") {
        unrotatedCorner = QPointF(rect.center().x(), rect.bottom());
    } else if (handle == "e") {
        unrotatedCorner = QPointF(rect.right(), rect.center().y());
    } else if (handle == "w") {
        unrotatedCorner = QPointF(rect.left(), rect.center().y());
    }
    
    return rotatePoint(unrotatedCorner, center, angleDegrees);
}

// Helper function to verify corner position within tolerance
static bool verifyCornerPosition(const QPointF& actual, const QPointF& expected, double tolerance = 1.0) {
    double dx = actual.x() - expected.x();
    double dy = actual.y() - expected.y();
    double distance = std::sqrt(dx * dx + dy * dy);
    return distance <= tolerance;
}

// Helper function to verify opposite corner is fixed in unrotated space
static bool verifyOppositeCornerFixed(const QRectF& before, const QRectF& after, const QString& handle, double tolerance = 0.1) {
    QPointF oppositeBefore, oppositeAfter;
    
    if (handle == "nw") {
        oppositeBefore = before.bottomRight();
        oppositeAfter = after.bottomRight();
    } else if (handle == "ne") {
        oppositeBefore = before.bottomLeft();
        oppositeAfter = after.bottomLeft();
    } else if (handle == "sw") {
        oppositeBefore = before.topRight();
        oppositeAfter = after.topRight();
    } else if (handle == "se") {
        oppositeBefore = before.topLeft();
        oppositeAfter = after.topLeft();
    } else {
        // For edge handles, check the opposite edge
        return true; // Simplified for edge handles
    }
    
    double dx = oppositeAfter.x() - oppositeBefore.x();
    double dy = oppositeAfter.y() - oppositeBefore.y();
    double distance = std::sqrt(dx * dx + dy * dy);
    return distance <= tolerance;
}

class TestCanvasRegionOperations : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testMoveRegion();
    void testMoveRegions();
    void testResizeRegion();
    void testResizeRotated45DegreesCorner();
    void testResizeRotated30DegreesCorner();
    void testResizeRotated90DegreesCorner();
    void testResizeRotatedNegativeAngle();
    void testResizeRotated30DegreesEdge();
    void testResizeRotatedNear45Degrees();
    void testResizeRotatedMathematicalVerification();
    void testResizeRotatedOppositeCornerFixed();
    void testResizeRotatedAnglePreservation();
    void testConstrainToBounds();
    void testValidateRegion();

private:
    DocumentState* documentState;
    CanvasRegionOperations* regionOperations;
    QImage testImage;
    const double scaleFactor = 0.5;
    const QPointF imageOffset = QPointF(100.0, 50.0);
    const QRectF imageRect = QRectF(100.0, 50.0, 1000.0, 1500.0);
};

void TestCanvasRegionOperations::initTestCase() {
    documentState = new DocumentState();
    regionOperations = new CanvasRegionOperations();
    
    // Create test image
    testImage = QImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    documentState->setImage(testImage);
    
    // Add test region
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    documentState->addRegion("TestRegion", RegionData("TestRegion", norm, "blue"));
    documentState->synchronizeCoordinates();
}

void TestCanvasRegionOperations::cleanupTestCase() {
    delete regionOperations;
    delete documentState;
}

void TestCanvasRegionOperations::testMoveRegion() {
    RegionData regionBefore = documentState->getRegion("TestRegion");
    QPointF delta(50.0, 30.0);
    
    regionOperations->moveRegion("TestRegion", delta, documentState, testImage,
                                scaleFactor, imageOffset);
    
    RegionData regionAfter = documentState->getRegion("TestRegion");
    
    // Normalized coordinates should be different
    QVERIFY(regionAfter.normalizedCoords.x1 != regionBefore.normalizedCoords.x1 ||
            regionAfter.normalizedCoords.y1 != regionBefore.normalizedCoords.y1);
    
    // Region should still be valid
    QVERIFY(regionAfter.isValid());
}

void TestCanvasRegionOperations::testMoveRegions() {
    // Add second region
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    documentState->addRegion("TestRegion2", RegionData("TestRegion2", norm2, "red"));
    documentState->synchronizeCoordinates();
    
    RegionData region1Before = documentState->getRegion("TestRegion");
    RegionData region2Before = documentState->getRegion("TestRegion2");
    
    QList<QString> regionNames = {"TestRegion", "TestRegion2"};
    QPointF delta(25.0, 15.0);
    
    regionOperations->moveRegions(regionNames, delta, documentState, testImage,
                                 scaleFactor, imageOffset);
    
    RegionData region1After = documentState->getRegion("TestRegion");
    RegionData region2After = documentState->getRegion("TestRegion2");
    
    // Both regions should have moved
    QVERIFY(region1After.normalizedCoords.x1 != region1Before.normalizedCoords.x1 ||
            region1After.normalizedCoords.y1 != region1Before.normalizedCoords.y1);
    QVERIFY(region2After.normalizedCoords.x1 != region2Before.normalizedCoords.x1 ||
            region2After.normalizedCoords.y1 != region2Before.normalizedCoords.y1);
}

void TestCanvasRegionOperations::testResizeRegion() {
    RegionData regionBefore = documentState->getRegion("TestRegion");
    regionBefore.syncFromNormalized(testImage.width(), testImage.height(), 
                                    scaleFactor, imageOffset);
    QRectF resizeStartRect = regionBefore.toQRectF();
    NormalizedCoords originalNormalized = regionBefore.normalizedCoords;
    
    // Resize using southeast handle
    QPointF newPos = resizeStartRect.bottomRight() + QPointF(50.0, 50.0);
    
    regionOperations->resizeRegion("TestRegion", "se", newPos, resizeStartRect,
                                  documentState, testImage, scaleFactor, imageOffset, imageRect,
                                  0.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("TestRegion");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), 
                                  scaleFactor, imageOffset);
    
    // Region should be larger
    double widthBefore = resizeStartRect.width();
    double heightBefore = resizeStartRect.height();
    QRectF rectAfter = regionAfter.toQRectF();
    double widthAfter = rectAfter.width();
    double heightAfter = rectAfter.height();
    
    QVERIFY(widthAfter > widthBefore);
    QVERIFY(heightAfter > heightBefore);
    
    // Region should still be valid
    QVERIFY(regionAfter.isValid());
}

void TestCanvasRegionOperations::testResizeRotated45DegreesCorner() {
    // Setup: Create region, rotate 45°
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("Rotated45", RegionData("Rotated45", norm, "green"));
    RegionData region = documentState->getRegion("Rotated45");
    region.rotationAngle = 45.0;
    documentState->addRegion("Rotated45", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    // Calculate expected rotated corner position
    QPointF expectedRotatedCorner = calculateExpectedRotatedCorner(resizeStartRect, "se", 45.0);
    
    // Simulate drag on "se" handle outward
    QPointF newPos = expectedRotatedCorner + QPointF(50.0, 50.0);
    
    regionOperations->resizeRegion("Rotated45", "se", newPos, resizeStartRect,
                                  documentState, testImage, scaleFactor, imageOffset, imageRect,
                                  45.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("Rotated45");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify rotated SE corner reaches target position (within tolerance)
    QPointF actualRotatedCorner = calculateExpectedRotatedCorner(rectAfter, "se", 45.0);
    QVERIFY2(verifyCornerPosition(actualRotatedCorner, newPos, 2.0),
             "Rotated SE corner should reach target position");
    
    // Verify unrotated NW corner unchanged
    QVERIFY2(verifyOppositeCornerFixed(resizeStartRect, rectAfter, "se", 1.0),
             "Opposite corner (NW) should be fixed in unrotated space");
    
    // Verify no dimension flip
    QVERIFY(rectAfter.width() > 0);
    QVERIFY(rectAfter.height() > 0);
    
    // Verify rotation angle preserved
    QCOMPARE(regionAfter.rotationAngle, 45.0);
}

void TestCanvasRegionOperations::testResizeRotated30DegreesCorner() {
    // Setup: Create region, rotate 30°
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("Rotated30", RegionData("Rotated30", norm, "green"));
    RegionData region = documentState->getRegion("Rotated30");
    region.rotationAngle = 30.0;
    documentState->addRegion("Rotated30", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    // Calculate expected rotated corner position
    QPointF expectedRotatedCorner = calculateExpectedRotatedCorner(resizeStartRect, "ne", 30.0);
    
    // Simulate drag on "ne" handle outward
    QPointF newPos = expectedRotatedCorner + QPointF(30.0, -20.0);
    
    regionOperations->resizeRegion("Rotated30", "ne", newPos, resizeStartRect,
                                  documentState, testImage, scaleFactor, imageOffset, imageRect,
                                  30.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("Rotated30");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify rectangle grows (not shrinks)
    QVERIFY(rectAfter.width() > resizeStartRect.width() || rectAfter.height() > resizeStartRect.height());
    
    // Verify opposite corner fixed
    QVERIFY2(verifyOppositeCornerFixed(resizeStartRect, rectAfter, "ne", 1.0),
             "Opposite corner (SW) should be fixed");
    
    // Verify rotation angle preserved
    QCOMPARE(regionAfter.rotationAngle, 30.0);
}

void TestCanvasRegionOperations::testResizeRotated90DegreesCorner() {
    // Setup: Create region, rotate 90°
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("Rotated90", RegionData("Rotated90", norm, "green"));
    RegionData region = documentState->getRegion("Rotated90");
    region.rotationAngle = 90.0;
    documentState->addRegion("Rotated90", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    // Drag "ne" handle (visually at right side when rotated 90°)
    QPointF expectedRotatedCorner = calculateExpectedRotatedCorner(resizeStartRect, "ne", 90.0);
    QPointF newPos = expectedRotatedCorner + QPointF(40.0, 0.0);
    
    regionOperations->resizeRegion("Rotated90", "ne", newPos, resizeStartRect,
                                  documentState, testImage, scaleFactor, imageOffset, imageRect,
                                  90.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("Rotated90");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify correct corner moves
    QPointF actualRotatedCorner = calculateExpectedRotatedCorner(rectAfter, "ne", 90.0);
    QVERIFY2(verifyCornerPosition(actualRotatedCorner, newPos, 2.0),
             "Rotated NE corner should reach target position");
    
    // Verify opposite corner fixed
    QVERIFY2(verifyOppositeCornerFixed(resizeStartRect, rectAfter, "ne", 1.0),
             "Opposite corner should be fixed");
    
    QCOMPARE(regionAfter.rotationAngle, 90.0);
}

void TestCanvasRegionOperations::testResizeRotatedNegativeAngle() {
    // Setup: Create region, rotate -45°
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("RotatedNeg45", RegionData("RotatedNeg45", norm, "green"));
    RegionData region = documentState->getRegion("RotatedNeg45");
    region.rotationAngle = -45.0;
    documentState->addRegion("RotatedNeg45", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    QPointF expectedRotatedCorner = calculateExpectedRotatedCorner(resizeStartRect, "se", -45.0);
    QPointF newPos = expectedRotatedCorner + QPointF(30.0, 30.0);
    
    regionOperations->resizeRegion("RotatedNeg45", "se", newPos, resizeStartRect,
                                   documentState, testImage, scaleFactor, imageOffset, imageRect,
                                   -45.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("RotatedNeg45");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify mirrors +45° behavior correctly
    QPointF actualRotatedCorner = calculateExpectedRotatedCorner(rectAfter, "se", -45.0);
    QVERIFY2(verifyCornerPosition(actualRotatedCorner, newPos, 2.0),
             "Rotated SE corner should reach target position");
    
    QVERIFY2(verifyOppositeCornerFixed(resizeStartRect, rectAfter, "se", 1.0),
             "Opposite corner should be fixed");
    
    QCOMPARE(regionAfter.rotationAngle, -45.0);
}

void TestCanvasRegionOperations::testResizeRotated30DegreesEdge() {
    // Setup: Create region, rotate 30°
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("Rotated30Edge", RegionData("Rotated30Edge", norm, "green"));
    RegionData region = documentState->getRegion("Rotated30Edge");
    region.rotationAngle = 30.0;
    documentState->addRegion("Rotated30Edge", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    // Drag "n" (top edge) handle
    QPointF expectedRotatedEdge = calculateExpectedRotatedCorner(resizeStartRect, "n", 30.0);
    QPointF newPos = expectedRotatedEdge + QPointF(0.0, -30.0);
    
    regionOperations->resizeRegion("Rotated30Edge", "n", newPos, resizeStartRect,
                                  documentState, testImage, scaleFactor, imageOffset, imageRect,
                                  30.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("Rotated30Edge");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify top edge moved, bottom edge unchanged in unrotated space
    QVERIFY(std::abs(rectAfter.top() - resizeStartRect.top()) > 0.1);
    QVERIFY(std::abs(rectAfter.bottom() - resizeStartRect.bottom()) < 1.0); // Bottom should be nearly fixed
    
    QCOMPARE(regionAfter.rotationAngle, 30.0);
}

void TestCanvasRegionOperations::testResizeRotatedNear45Degrees() {
    // Setup: Create region, rotate 44.9° (near boundary case)
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("Rotated44.9", RegionData("Rotated44.9", norm, "green"));
    RegionData region = documentState->getRegion("Rotated44.9");
    region.rotationAngle = 44.9;
    documentState->addRegion("Rotated44.9", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    QPointF expectedRotatedCorner = calculateExpectedRotatedCorner(resizeStartRect, "se", 44.9);
    QPointF newPos = expectedRotatedCorner + QPointF(25.0, 25.0);
    
    regionOperations->resizeRegion("Rotated44.9", "se", newPos, resizeStartRect,
                                  documentState, testImage, scaleFactor, imageOffset, imageRect,
                                  44.9, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("Rotated44.9");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify no fallback behavior, works smoothly
    QVERIFY(rectAfter.width() > 0);
    QVERIFY(rectAfter.height() > 0);
    QVERIFY2(verifyOppositeCornerFixed(resizeStartRect, rectAfter, "se", 1.0),
             "Opposite corner should be fixed even near 45°");
    
    QCOMPARE(regionAfter.rotationAngle, 44.9);
}

void TestCanvasRegionOperations::testResizeRotatedMathematicalVerification() {
    // Setup: Known rectangle dimensions and rotation angle
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("MathTest", RegionData("MathTest", norm, "green"));
    RegionData region = documentState->getRegion("MathTest");
    region.rotationAngle = 45.0;
    documentState->addRegion("MathTest", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    // Calculate expected values mathematically
    QPointF center = resizeStartRect.center();
    QPointF seCorner = resizeStartRect.bottomRight();
    QPointF expectedRotatedSE = rotatePoint(seCorner, center, 45.0);
    
    // Perform resize
    QPointF newPos = expectedRotatedSE + QPointF(50.0, 50.0);
    
    regionOperations->resizeRegion("MathTest", "se", newPos, resizeStartRect,
                                  documentState, testImage, scaleFactor, imageOffset, imageRect,
                                  45.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("MathTest");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify calculated values match expected
    QPointF actualRotatedSE = rotatePoint(rectAfter.bottomRight(), rectAfter.center(), 45.0);
    QVERIFY2(verifyCornerPosition(actualRotatedSE, newPos, 2.0),
             "Mathematical verification: rotated corner should match expected position");
}

void TestCanvasRegionOperations::testResizeRotatedOppositeCornerFixed() {
    // Setup: Rectangle with rotation
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("OppositeTest", RegionData("OppositeTest", norm, "green"));
    RegionData region = documentState->getRegion("OppositeTest");
    region.rotationAngle = 30.0;
    documentState->addRegion("OppositeTest", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    // Resize from any corner
    QPointF expectedRotatedCorner = calculateExpectedRotatedCorner(resizeStartRect, "nw", 30.0);
    QPointF newPos = expectedRotatedCorner + QPointF(-20.0, -20.0);
    
    regionOperations->resizeRegion("OppositeTest", "nw", newPos, resizeStartRect,
                                   documentState, testImage, scaleFactor, imageOffset, imageRect,
                                   30.0, originalNormalized);
    
    RegionData regionAfter = documentState->getRegion("OppositeTest");
    regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF rectAfter = regionAfter.toQRectF();
    
    // Verify opposite corner coordinates unchanged in unrotated space
    QVERIFY2(verifyOppositeCornerFixed(resizeStartRect, rectAfter, "nw", 0.5),
             "Opposite corner (SE) should be fixed in unrotated space");
}

void TestCanvasRegionOperations::testResizeRotatedAnglePreservation() {
    // Setup: Rectangle with rotation angle
    NormalizedCoords norm(0.4, 0.4, 0.5, 0.5);
    documentState->addRegion("AngleTest", RegionData("AngleTest", norm, "green"));
    RegionData region = documentState->getRegion("AngleTest");
    region.rotationAngle = 60.0;
    documentState->addRegion("AngleTest", region);
    documentState->synchronizeCoordinates();
    
    region.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
    QRectF resizeStartRect = region.toQRectF();
    NormalizedCoords originalNormalized = region.normalizedCoords;
    
    // Multiple resize operations
    for (int i = 0; i < 3; i++) {
        QPointF expectedRotatedCorner = calculateExpectedRotatedCorner(resizeStartRect, "se", 60.0);
        QPointF newPos = expectedRotatedCorner + QPointF(10.0 * (i + 1), 10.0 * (i + 1));
        
        regionOperations->resizeRegion("AngleTest", "se", newPos, resizeStartRect,
                                      documentState, testImage, scaleFactor, imageOffset, imageRect,
                                      60.0, originalNormalized);
        
        RegionData regionAfter = documentState->getRegion("AngleTest");
        QCOMPARE(regionAfter.rotationAngle, 60.0);
        
        // Update for next iteration
        regionAfter.syncFromNormalized(testImage.width(), testImage.height(), scaleFactor, imageOffset);
        resizeStartRect = regionAfter.toQRectF();
        originalNormalized = regionAfter.normalizedCoords;
    }
}

void TestCanvasRegionOperations::testConstrainToBounds() {
    // Create a region that goes out of bounds
    NormalizedCoords outOfBounds(1.5, -0.1, 2.0, 0.5);
    RegionData region("OutOfBounds", outOfBounds, "red");
    
    regionOperations->constrainToBounds(region);
    
    // Coordinates should be clamped to valid range
    QVERIFY(region.normalizedCoords.x1 >= 0.0 && region.normalizedCoords.x1 <= 1.0);
    QVERIFY(region.normalizedCoords.y1 >= 0.0 && region.normalizedCoords.y1 <= 1.0);
    QVERIFY(region.normalizedCoords.x2 >= 0.0 && region.normalizedCoords.x2 <= 1.0);
    QVERIFY(region.normalizedCoords.y2 >= 0.0 && region.normalizedCoords.y2 <= 1.0);
}

void TestCanvasRegionOperations::testValidateRegion() {
    // Valid region (inside image rect)
    QRectF validRect(200.0, 100.0, 300.0, 200.0);
    QVERIFY(regionOperations->validateRegion(validRect, imageRect));
    
    // Invalid region (too small)
    QRectF tooSmall(200.0, 100.0, 5.0, 5.0);
    QVERIFY(!regionOperations->validateRegion(tooSmall, imageRect));
    
    // Invalid region (outside image rect)
    QRectF outsideRect(2000.0, 3000.0, 2100.0, 3100.0);
    QVERIFY(!regionOperations->validateRegion(outsideRect, imageRect));
    
    // Invalid region (empty)
    QRectF emptyRect;
    QVERIFY(!regionOperations->validateRegion(emptyRect, imageRect));
}

QTEST_MAIN(TestCanvasRegionOperations)
#include "test_canvas_region_operations.moc"

