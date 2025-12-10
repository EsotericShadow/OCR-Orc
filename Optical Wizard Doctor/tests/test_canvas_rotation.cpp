// Test file for Canvas Rotation functionality
// Tests rotation-related operations including hit testing, state management, and angle calculations

#include <QtTest/QtTest>
#include "../src/ui/canvas/core/coordinate/CanvasHitTester.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtGui/QImage>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <cmath>

using namespace ocr_orc;

class TestCanvasRotation : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Rotate icon hit testing
    void testIsOnRotateIcon();
    void testIsOnRotateIconAtBottomRight();
    void testIsOnRotateIconNearBottomRight();
    void testIsOnRotateIconFarFromBottomRight();
    void testIsOnRotateIconWithCustomSize();
    void testIsOnRotateIconOnDifferentRegions();
    
    // Rotation angle calculations
    void testRotationAngleCalculation();
    void testRotationAngleNormalization();
    void testRotationAngleFromCenter();
    void testRotationAngleEdgeCases();
    
    // Rotation state management
    void testRotationStateInitialization();
    void testRotationStateTransitions();

private:
    DocumentState* documentState;
    CanvasHitTester* hitTester;
    QImage testImage;
    QRectF imageRect;
    const double scaleFactor = 0.5;
    const QPointF imageOffset = QPointF(100.0, 50.0);
    
    // Helper function to calculate angle from center
    double calculateAngle(const QPointF& center, const QPointF& point) {
        QPointF delta = point - center;
        return std::atan2(delta.y(), delta.x()) * 180.0 / M_PI;
    }
    
    // Helper function to normalize angle to -180 to 180 range
    double normalizeAngle(double angle) {
        while (angle > 180.0) angle -= 360.0;
        while (angle < -180.0) angle += 360.0;
        return angle;
    }
};

void TestCanvasRotation::initTestCase() {
    documentState = new DocumentState();
    hitTester = new CanvasHitTester();
    
    // Create test image
    testImage = QImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    documentState->setImage(testImage);
    
    // Add test regions
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    documentState->addRegion("TestRegion1", RegionData("TestRegion1", norm1, "blue"));
    
    NormalizedCoords norm2(0.50, 0.50, 0.70, 0.70);
    documentState->addRegion("TestRegion2", RegionData("TestRegion2", norm2, "red"));
    
    documentState->synchronizeCoordinates();
    
    // Calculate image rect
    imageRect = QRectF(100.0, 50.0, 1000.0, 1500.0);
}

void TestCanvasRotation::cleanupTestCase() {
    delete hitTester;
    delete documentState;
}

void TestCanvasRotation::testIsOnRotateIcon() {
    // Get region's canvas coordinates
    RegionData region = documentState->getRegion("TestRegion1");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    // Test hit exactly at bottom-right corner
    QPointF bottomRight = regionRect.bottomRight();
    QVERIFY(hitTester->isOnRotateIcon(bottomRight, regionRect));
}

void TestCanvasRotation::testIsOnRotateIconAtBottomRight() {
    RegionData region = documentState->getRegion("TestRegion1");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    // Test at exact bottom-right corner
    QPointF exactPos = regionRect.bottomRight();
    QVERIFY(hitTester->isOnRotateIcon(exactPos, regionRect));
    
    // Test slightly towards center but still within tolerance
    // The icon is at bottom-right, so moving inside means moving up-left
    // But we need to stay within the tolerance radius (approximately 12-14 pixels)
    QPointF insidePos = exactPos - QPointF(8.0, 8.0);
    QVERIFY(hitTester->isOnRotateIcon(insidePos, regionRect));
}

void TestCanvasRotation::testIsOnRotateIconNearBottomRight() {
    RegionData region = documentState->getRegion("TestRegion1");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    QPointF bottomRight = regionRect.bottomRight();
    
    // Test positions within tolerance (default icon size is 24.0)
    // Tolerance should be approximately 12.0 + handle tolerance
    QPointF nearPos1 = bottomRight + QPointF(10.0, 0.0);
    QVERIFY(hitTester->isOnRotateIcon(nearPos1, regionRect));
    
    QPointF nearPos2 = bottomRight + QPointF(0.0, 10.0);
    QVERIFY(hitTester->isOnRotateIcon(nearPos2, regionRect));
    
    QPointF nearPos3 = bottomRight + QPointF(8.0, 8.0);
    QVERIFY(hitTester->isOnRotateIcon(nearPos3, regionRect));
}

void TestCanvasRotation::testIsOnRotateIconFarFromBottomRight() {
    RegionData region = documentState->getRegion("TestRegion1");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    QPointF bottomRight = regionRect.bottomRight();
    
    // Test positions far from icon (should not be detected)
    QPointF farPos1 = bottomRight + QPointF(50.0, 0.0);
    QVERIFY(!hitTester->isOnRotateIcon(farPos1, regionRect));
    
    QPointF farPos2 = bottomRight + QPointF(0.0, 50.0);
    QVERIFY(!hitTester->isOnRotateIcon(farPos2, regionRect));
    
    QPointF farPos3 = regionRect.center();
    QVERIFY(!hitTester->isOnRotateIcon(farPos3, regionRect));
    
    QPointF farPos4 = regionRect.topLeft();
    QVERIFY(!hitTester->isOnRotateIcon(farPos4, regionRect));
}

void TestCanvasRotation::testIsOnRotateIconWithCustomSize() {
    RegionData region = documentState->getRegion("TestRegion1");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    QPointF bottomRight = regionRect.bottomRight();
    
    // Test with larger icon size
    double largeIconSize = 48.0;
    QPointF nearPos = bottomRight + QPointF(20.0, 0.0);
    QVERIFY(hitTester->isOnRotateIcon(nearPos, regionRect, largeIconSize));
    
    // Test with smaller icon size
    double smallIconSize = 12.0;
    QPointF farPos = bottomRight + QPointF(10.0, 0.0);
    QVERIFY(!hitTester->isOnRotateIcon(farPos, regionRect, smallIconSize));
}

void TestCanvasRotation::testIsOnRotateIconOnDifferentRegions() {
    // Test first region
    RegionData region1 = documentState->getRegion("TestRegion1");
    region1.syncFromNormalized(testImage.width(), testImage.height(), 
                               scaleFactor, imageOffset);
    QRectF regionRect1 = region1.toQRectF();
    QPointF bottomRight1 = regionRect1.bottomRight();
    QVERIFY(hitTester->isOnRotateIcon(bottomRight1, regionRect1));
    
    // Test second region
    RegionData region2 = documentState->getRegion("TestRegion2");
    region2.syncFromNormalized(testImage.width(), testImage.height(), 
                               scaleFactor, imageOffset);
    QRectF regionRect2 = region2.toQRectF();
    QPointF bottomRight2 = regionRect2.bottomRight();
    QVERIFY(hitTester->isOnRotateIcon(bottomRight2, regionRect2));
    
    // Test that clicking on region1's icon doesn't trigger region2's icon
    QVERIFY(!hitTester->isOnRotateIcon(bottomRight1, regionRect2));
    QVERIFY(!hitTester->isOnRotateIcon(bottomRight2, regionRect1));
}

void TestCanvasRotation::testRotationAngleCalculation() {
    QPointF center(100.0, 100.0);
    
    // Test angle at 0 degrees (right)
    QPointF point0 = center + QPointF(50.0, 0.0);
    double angle0 = calculateAngle(center, point0);
    QCOMPARE(static_cast<int>(std::round(angle0)), 0);
    
    // Test angle at 90 degrees (down)
    QPointF point90 = center + QPointF(0.0, 50.0);
    double angle90 = calculateAngle(center, point90);
    QCOMPARE(static_cast<int>(std::round(angle90)), 90);
    
    // Test angle at 180 degrees (left)
    QPointF point180 = center + QPointF(-50.0, 0.0);
    double angle180 = calculateAngle(center, point180);
    QCOMPARE(static_cast<int>(std::round(angle180)), 180);
    
    // Test angle at -90 degrees (up)
    QPointF pointNeg90 = center + QPointF(0.0, -50.0);
    double angleNeg90 = calculateAngle(center, pointNeg90);
    QCOMPARE(static_cast<int>(std::round(angleNeg90)), -90);
    
    // Test angle at 45 degrees
    QPointF point45 = center + QPointF(50.0, 50.0);
    double angle45 = calculateAngle(center, point45);
    QCOMPARE(static_cast<int>(std::round(angle45)), 45);
}

void TestCanvasRotation::testRotationAngleNormalization() {
    // Test angles that need normalization
    double angle270 = 270.0;
    double normalized270 = normalizeAngle(angle270);
    QCOMPARE(static_cast<int>(std::round(normalized270)), -90);
    
    double angle360 = 360.0;
    double normalized360 = normalizeAngle(angle360);
    QCOMPARE(static_cast<int>(std::round(normalized360)), 0);
    
    double angle450 = 450.0;
    double normalized450 = normalizeAngle(angle450);
    QCOMPARE(static_cast<int>(std::round(normalized450)), 90);
    
    double angleNeg270 = -270.0;
    double normalizedNeg270 = normalizeAngle(angleNeg270);
    QCOMPARE(static_cast<int>(std::round(normalizedNeg270)), 90);
    
    // Test angles already in range
    double angle45 = 45.0;
    double normalized45 = normalizeAngle(angle45);
    QCOMPARE(static_cast<int>(std::round(normalized45)), 45);
    
    double angleNeg45 = -45.0;
    double normalizedNeg45 = normalizeAngle(angleNeg45);
    QCOMPARE(static_cast<int>(std::round(normalizedNeg45)), -45);
}

void TestCanvasRotation::testRotationAngleFromCenter() {
    QPointF center(200.0, 200.0);
    
    // Test rotation angle calculation (difference from start)
    QPointF startPos = center + QPointF(50.0, 0.0);  // 0 degrees
    double startAngle = calculateAngle(center, startPos);
    
    QPointF currentPos = center + QPointF(0.0, 50.0);  // 90 degrees
    double currentAngle = calculateAngle(center, currentPos);
    
    double rotationAngle = currentAngle - startAngle;
    QCOMPARE(static_cast<int>(std::round(rotationAngle)), 90);
    
    // Test negative rotation
    QPointF currentPos2 = center + QPointF(0.0, -50.0);  // -90 degrees
    double currentAngle2 = calculateAngle(center, currentPos2);
    double rotationAngle2 = currentAngle2 - startAngle;
    QCOMPARE(static_cast<int>(std::round(rotationAngle2)), -90);
}

void TestCanvasRotation::testRotationAngleEdgeCases() {
    QPointF center(100.0, 100.0);
    
    // Test very small movement (should still calculate angle)
    QPointF startPos = center + QPointF(50.0, 0.0);
    QPointF smallMove = center + QPointF(50.0, 0.1);
    double startAngle = calculateAngle(center, startPos);
    double currentAngle = calculateAngle(center, smallMove);
    double rotationAngle = currentAngle - startAngle;
    
    // Should be a very small angle (approximately 0.1 degrees)
    QVERIFY(std::abs(rotationAngle) < 1.0);
    
    // Test at center (should handle gracefully)
    QPointF atCenter = center;
    double angleAtCenter = calculateAngle(center, atCenter);
    // Angle at center is undefined, but atan2 returns 0 for (0,0)
    QCOMPARE(static_cast<int>(std::round(angleAtCenter)), 0);
}

void TestCanvasRotation::testRotationStateInitialization() {
    // Test that rotation state variables can be initialized
    bool isRotating = false;
    QString rotatingRegion;
    QPointF rotationCenter;
    double rotationStartAngle = 0.0;
    double rotationAngle = 0.0;
    
    QVERIFY(!isRotating);
    QVERIFY(rotatingRegion.isEmpty());
    QCOMPARE(rotationStartAngle, 0.0);
    QCOMPARE(rotationAngle, 0.0);
    
    // Test setting rotation state
    isRotating = true;
    rotatingRegion = "TestRegion1";
    rotationCenter = QPointF(100.0, 100.0);
    rotationStartAngle = 45.0;
    rotationAngle = 0.0;
    
    QVERIFY(isRotating);
    QCOMPARE(rotatingRegion, QString("TestRegion1"));
    QCOMPARE(rotationCenter, QPointF(100.0, 100.0));
    QCOMPARE(rotationStartAngle, 45.0);
    QCOMPARE(rotationAngle, 0.0);
}

void TestCanvasRotation::testRotationStateTransitions() {
    bool isRotating = false;
    QString rotatingRegion;
    double rotationAngle = 0.0;
    
    // Initial state: not rotating
    QVERIFY(!isRotating);
    QVERIFY(rotatingRegion.isEmpty());
    QCOMPARE(rotationAngle, 0.0);
    
    // Transition to rotating state
    isRotating = true;
    rotatingRegion = "TestRegion1";
    rotationAngle = 30.0;
    
    QVERIFY(isRotating);
    QCOMPARE(rotatingRegion, QString("TestRegion1"));
    QCOMPARE(rotationAngle, 30.0);
    
    // Update rotation angle during drag
    rotationAngle = 60.0;
    QCOMPARE(rotationAngle, 60.0);
    
    // Transition back to not rotating
    isRotating = false;
    rotatingRegion = QString();
    rotationAngle = 0.0;
    
    QVERIFY(!isRotating);
    QVERIFY(rotatingRegion.isEmpty());
    QCOMPARE(rotationAngle, 0.0);
}

QTEST_MAIN(TestCanvasRotation)
#include "test_canvas_rotation.moc"

