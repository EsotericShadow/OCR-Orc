// Test file for CanvasHitTester
// Tests hit testing operations

#include <QtTest/QtTest>
#include "../src/ui/canvas/core/coordinate/CanvasHitTester.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtGui/QImage>

using namespace ocr_orc;

class TestCanvasHitTester : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetRegionAt();
    void testGetRegionAtNoHit();
    void testGetHandleAt();
    void testGetHandleAtNoHit();

private:
    DocumentState* documentState;
    CanvasHitTester* hitTester;
    QImage testImage;
    QRectF imageRect;
    const double scaleFactor = 0.5;
    const QPointF imageOffset = QPointF(100.0, 50.0);
};

void TestCanvasHitTester::initTestCase() {
    documentState = new DocumentState();
    hitTester = new CanvasHitTester();
    
    // Create test image
    testImage = QImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    documentState->setImage(testImage);
    
    // Add test region
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    documentState->addRegion("TestRegion", RegionData("TestRegion", norm, "blue"));
    documentState->synchronizeCoordinates();
    
    // Calculate image rect
    imageRect = QRectF(100.0, 50.0, 1000.0, 1500.0);
}

void TestCanvasHitTester::cleanupTestCase() {
    delete hitTester;
    delete documentState;
}

void TestCanvasHitTester::testGetRegionAt() {
    // Get region's canvas coordinates
    RegionData region = documentState->getRegion("TestRegion");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    // Test hit at center of region
    QPointF centerPos = regionRect.center();
    QString found = hitTester->getRegionAt(centerPos, documentState, testImage,
                                           imageRect, scaleFactor, imageOffset);
    
    QCOMPARE(found, QString("TestRegion"));
    
    // Test hit near edge of region
    QPointF edgePos(regionRect.left() + 5.0, regionRect.top() + 5.0);
    QString foundEdge = hitTester->getRegionAt(edgePos, documentState, testImage,
                                               imageRect, scaleFactor, imageOffset);
    
    QCOMPARE(foundEdge, QString("TestRegion"));
}

void TestCanvasHitTester::testGetRegionAtNoHit() {
    // Test position outside any region
    QPointF outsidePos(2000.0, 3000.0);
    QString found = hitTester->getRegionAt(outsidePos, documentState, testImage,
                                           imageRect, scaleFactor, imageOffset);
    
    QVERIFY(found.isEmpty());
    
    // Test position in image but not in region
    QPointF insideImageButNotRegion(500.0, 500.0);
    QString found2 = hitTester->getRegionAt(insideImageButNotRegion, documentState, testImage,
                                            imageRect, scaleFactor, imageOffset);
    
    // Should be empty (region is at y=0.10-0.20, which is around 50-150 in canvas coords)
    QVERIFY(found2.isEmpty());
}

void TestCanvasHitTester::testGetHandleAt() {
    // Get region's canvas coordinates
    RegionData region = documentState->getRegion("TestRegion");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    // Test northwest handle
    QPointF nwHandle = regionRect.topLeft();
    QString handle = hitTester->getHandleAt(nwHandle, regionRect);
    QCOMPARE(handle, QString("nw"));
    
    // Test northeast handle
    QPointF neHandle = regionRect.topRight();
    handle = hitTester->getHandleAt(neHandle, regionRect);
    QCOMPARE(handle, QString("ne"));
    
    // Test southwest handle
    QPointF swHandle = regionRect.bottomLeft();
    handle = hitTester->getHandleAt(swHandle, regionRect);
    QCOMPARE(handle, QString("sw"));
    
    // Test southeast handle
    QPointF seHandle = regionRect.bottomRight();
    handle = hitTester->getHandleAt(seHandle, regionRect);
    QCOMPARE(handle, QString("se"));
    
    // Test north handle
    QPointF nHandle(regionRect.center().x(), regionRect.top());
    handle = hitTester->getHandleAt(nHandle, regionRect);
    QCOMPARE(handle, QString("n"));
    
    // Test south handle
    QPointF sHandle(regionRect.center().x(), regionRect.bottom());
    handle = hitTester->getHandleAt(sHandle, regionRect);
    QCOMPARE(handle, QString("s"));
    
    // Test east handle
    QPointF eHandle(regionRect.right(), regionRect.center().y());
    handle = hitTester->getHandleAt(eHandle, regionRect);
    QCOMPARE(handle, QString("e"));
    
    // Test west handle
    QPointF wHandle(regionRect.left(), regionRect.center().y());
    handle = hitTester->getHandleAt(wHandle, regionRect);
    QCOMPARE(handle, QString("w"));
}

void TestCanvasHitTester::testGetHandleAtNoHit() {
    // Get region's canvas coordinates
    RegionData region = documentState->getRegion("TestRegion");
    region.syncFromNormalized(testImage.width(), testImage.height(), 
                              scaleFactor, imageOffset);
    QRectF regionRect = region.toQRectF();
    
    // Test position far from any handle
    QPointF farPos = regionRect.center();
    QString handle = hitTester->getHandleAt(farPos, regionRect);
    
    QVERIFY(handle.isEmpty());
    
    // Test position outside region
    QPointF outsidePos(regionRect.right() + 100.0, regionRect.bottom() + 100.0);
    handle = hitTester->getHandleAt(outsidePos, regionRect);
    
    QVERIFY(handle.isEmpty());
}

QTEST_MAIN(TestCanvasHitTester)
#include "test_canvas_hit_tester.moc"

