// Test file for CanvasZoomController
// Tests zoom and pan operations

#include <QtTest/QtTest>
#include "../src/ui/canvas/core/zoom/CanvasZoomController.h"
#include "../src/ui/canvas/core/coordinate/CanvasCoordinateCache.h"
#include "../src/models/DocumentState.h"
#include <QtGui/QImage>

using namespace ocr_orc;

class TestCanvasZoomController : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testCalculateLayout();
    void testZoomIn();
    void testZoomOut();
    void testZoomReset();
    void testSetZoom();
    void testSetZoomAtPoint();
    void testGetZoom();
    void testPanWithWheel();

private:
    DocumentState* documentState;
    CanvasZoomController* zoomController;
    CanvasCoordinateCache* coordinateCache;
    QImage testImage;
    const int canvasWidth = 800;
    const int canvasHeight = 600;
};

void TestCanvasZoomController::initTestCase() {
    documentState = new DocumentState();
    zoomController = new CanvasZoomController();
    coordinateCache = new CanvasCoordinateCache();
    
    // Create test image
    testImage = QImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    documentState->setImage(testImage);
    documentState->zoomLevel = 1.0;
}

void TestCanvasZoomController::cleanupTestCase() {
    delete zoomController;
    delete coordinateCache;
    delete documentState;
}

void TestCanvasZoomController::testCalculateLayout() {
    double scaleFactor = 0.0;
    QPointF imageOffset(0.0, 0.0);
    QRectF imageRect;
    
    zoomController->calculateLayout(testImage, canvasWidth, canvasHeight,
                                   documentState, scaleFactor, imageOffset, imageRect);
    
    // Scale factor should be calculated to fit image
    QVERIFY(scaleFactor > 0.0);
    QVERIFY(scaleFactor <= 1.0); // Should fit within canvas
    
    // Image offset should center the image
    QVERIFY(imageOffset.x() >= 0.0);
    QVERIFY(imageOffset.y() >= 0.0);
    
    // Image rect should be valid
    QVERIFY(!imageRect.isEmpty());
    QVERIFY(imageRect.width() > 0);
    QVERIFY(imageRect.height() > 0);
}

void TestCanvasZoomController::testZoomIn() {
    double scaleFactor = 0.5;
    QPointF imageOffset(100.0, 50.0);
    documentState->zoomLevel = 1.0;
    
    double initialZoom = documentState->zoomLevel;
    zoomController->zoomIn(documentState, testImage, canvasWidth, canvasHeight,
                          scaleFactor, imageOffset, coordinateCache);
    
    // Zoom should increase
    QVERIFY(documentState->zoomLevel > initialZoom);
    QVERIFY(documentState->zoomLevel > 1.0);
}

void TestCanvasZoomController::testZoomOut() {
    double scaleFactor = 1.0;
    QPointF imageOffset(100.0, 50.0);
    documentState->zoomLevel = 2.0;
    
    double initialZoom = documentState->zoomLevel;
    zoomController->zoomOut(documentState, testImage, canvasWidth, canvasHeight,
                           scaleFactor, imageOffset, coordinateCache);
    
    // Zoom should decrease
    QVERIFY(documentState->zoomLevel < initialZoom);
    QVERIFY(documentState->zoomLevel < 2.0);
}

void TestCanvasZoomController::testZoomReset() {
    double scaleFactor = 2.0;
    QPointF imageOffset(200.0, 150.0);
    documentState->zoomLevel = 3.0;
    
    zoomController->zoomReset(documentState, testImage, canvasWidth, canvasHeight,
                             scaleFactor, imageOffset, coordinateCache);
    
    // Zoom should be reset to 1.0
    QCOMPARE(documentState->zoomLevel, 1.0);
}

void TestCanvasZoomController::testSetZoom() {
    double scaleFactor = 0.5;
    QPointF imageOffset(100.0, 50.0);
    documentState->zoomLevel = 1.0;
    
    double targetZoom = 2.5;
    zoomController->setZoom(targetZoom, documentState, testImage,
                            canvasWidth, canvasHeight,
                            scaleFactor, imageOffset, coordinateCache);
    
    // Zoom should be set to target
    QCOMPARE(documentState->zoomLevel, targetZoom);
}

void TestCanvasZoomController::testSetZoomAtPoint() {
    double scaleFactor = 0.5;
    QPointF imageOffset(100.0, 50.0);
    documentState->zoomLevel = 1.0;
    
    QPointF widgetPos(400.0, 300.0); // Center of canvas
    double targetZoom = 2.0;
    
    double initialOffset = imageOffset.x();
    zoomController->setZoomAtPoint(targetZoom, widgetPos, documentState, testImage,
                                  canvasWidth, canvasHeight,
                                  scaleFactor, imageOffset, coordinateCache);
    
    // Zoom should be set
    QCOMPARE(documentState->zoomLevel, targetZoom);
    
    // Offset should be adjusted to keep point under cursor
    // (exact values depend on implementation, but should be different)
    QVERIFY(imageOffset.x() != initialOffset || imageOffset.y() != 50.0);
}

void TestCanvasZoomController::testGetZoom() {
    documentState->zoomLevel = 1.5;
    double zoom = zoomController->getZoom(documentState);
    QCOMPARE(zoom, 1.5);
    
    // Test with null document state
    double defaultZoom = zoomController->getZoom(nullptr);
    QCOMPARE(defaultZoom, 1.0);
}

void TestCanvasZoomController::testPanWithWheel() {
    QPointF imageOffset(100.0, 50.0);
    documentState->imageOffset = imageOffset;
    
    // Pan right
    QPointF delta(50.0, 0.0);
    zoomController->panWithWheel(delta, documentState, imageOffset);
    
    // Offset should change
    QVERIFY(imageOffset.x() != 100.0 || imageOffset.y() != 50.0);
    
    // Pan down
    QPointF delta2(0.0, 30.0);
    QPointF imageOffset2(100.0, 50.0);
    documentState->imageOffset = imageOffset2;
    zoomController->panWithWheel(delta2, documentState, imageOffset2);
    
    // Offset should change
    QVERIFY(imageOffset2.x() != 100.0 || imageOffset2.y() != 50.0);
}

QTEST_MAIN(TestCanvasZoomController)
#include "test_canvas_zoom_controller.moc"

