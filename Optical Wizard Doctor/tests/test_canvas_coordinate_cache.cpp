// Test file for CanvasCoordinateCache
// Tests coordinate caching functionality

#include <QtTest/QtTest>
#include "../src/ui/canvas/core/coordinate/CanvasCoordinateCache.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtGui/QImage>

using namespace ocr_orc;

class TestCanvasCoordinateCache : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testCacheInitialization();
    void testCacheUpdate();
    void testCacheInvalidation();
    void testNeedsUpdate();
    void testGetCachedCoordinates();
    void testCacheWithMultipleRegions();
    void testCacheAfterZoom();
    void testCacheAfterPan();

private:
    DocumentState* documentState;
    CanvasCoordinateCache* cache;
    QImage testImage;
};

void TestCanvasCoordinateCache::initTestCase() {
    documentState = new DocumentState();
    cache = new CanvasCoordinateCache();
    
    // Create test image
    testImage = QImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    documentState->setImage(testImage);
    
    // Add test regions
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    documentState->addRegion("Region1", RegionData("Region1", norm1, "blue"));
    documentState->addRegion("Region2", RegionData("Region2", norm2, "red"));
}

void TestCanvasCoordinateCache::cleanupTestCase() {
    delete cache;
    delete documentState;
}

void TestCanvasCoordinateCache::testCacheInitialization() {
    QVERIFY(!cache->isValid());
    QVERIFY(cache->getAllCachedCoordinates().isEmpty());
}

void TestCanvasCoordinateCache::testCacheUpdate() {
    double scaleFactor = 0.5;
    QPointF imageOffset(100.0, 50.0);
    double zoomLevel = 1.0;
    
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor, imageOffset, zoomLevel);
    
    QVERIFY(cache->isValid());
    QCOMPARE(cache->getAllCachedCoordinates().size(), 2);
    QVERIFY(cache->getAllCachedCoordinates().contains("Region1"));
    QVERIFY(cache->getAllCachedCoordinates().contains("Region2"));
}

void TestCanvasCoordinateCache::testCacheInvalidation() {
    cache->invalidate();
    QVERIFY(!cache->isValid());
    QVERIFY(cache->getAllCachedCoordinates().isEmpty());
}

void TestCanvasCoordinateCache::testNeedsUpdate() {
    // Cache should need update when invalid
    QVERIFY(cache->needsUpdate(1.0, QPointF(0, 0), testImage.size()));
    
    // Update cache
    double scaleFactor = 0.5;
    QPointF imageOffset(100.0, 50.0);
    double zoomLevel = 1.0;
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor, imageOffset, zoomLevel);
    
    // Should not need update with same parameters
    QVERIFY(!cache->needsUpdate(zoomLevel, imageOffset, testImage.size()));
    
    // Should need update when zoom changes
    QVERIFY(cache->needsUpdate(2.0, imageOffset, testImage.size()));
    
    // Should need update when offset changes
    QVERIFY(cache->needsUpdate(zoomLevel, QPointF(200.0, 100.0), testImage.size()));
    
    // Should need update when image size changes
    QVERIFY(cache->needsUpdate(zoomLevel, imageOffset, QSize(3000, 4000)));
}

void TestCanvasCoordinateCache::testGetCachedCoordinates() {
    double scaleFactor = 0.5;
    QPointF imageOffset(100.0, 50.0);
    double zoomLevel = 1.0;
    
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor, imageOffset, zoomLevel);
    
    // Get cached coordinates
    QRectF coords1 = cache->getCachedCoordinates("Region1", documentState,
                                                 testImage.width(), testImage.height(),
                                                 scaleFactor, imageOffset);
    
    QVERIFY(!coords1.isEmpty());
    QVERIFY(coords1.width() > 0);
    QVERIFY(coords1.height() > 0);
    
    // Get coordinates for non-existent region (should calculate on the fly)
    QRectF coords3 = cache->getCachedCoordinates("NonExistent", documentState,
                                                 testImage.width(), testImage.height(),
                                                 scaleFactor, imageOffset);
    // Should return empty or calculated coordinates
    QVERIFY(coords3.isEmpty() || coords3.isValid());
}

void TestCanvasCoordinateCache::testCacheWithMultipleRegions() {
    // Add more regions
    NormalizedCoords norm3(0.10, 0.30, 0.20, 0.40);
    NormalizedCoords norm4(0.50, 0.30, 0.60, 0.40);
    documentState->addRegion("Region3", RegionData("Region3", norm3, "green"));
    documentState->addRegion("Region4", RegionData("Region4", norm4, "yellow"));
    
    double scaleFactor = 0.5;
    QPointF imageOffset(100.0, 50.0);
    double zoomLevel = 1.0;
    
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor, imageOffset, zoomLevel);
    
    QCOMPARE(cache->getAllCachedCoordinates().size(), 4);
    QVERIFY(cache->getAllCachedCoordinates().contains("Region1"));
    QVERIFY(cache->getAllCachedCoordinates().contains("Region2"));
    QVERIFY(cache->getAllCachedCoordinates().contains("Region3"));
    QVERIFY(cache->getAllCachedCoordinates().contains("Region4"));
}

void TestCanvasCoordinateCache::testCacheAfterZoom() {
    double scaleFactor1 = 0.5;
    QPointF imageOffset(100.0, 50.0);
    double zoomLevel1 = 1.0;
    
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor1, imageOffset, zoomLevel1);
    
    QRectF coords1 = cache->getCachedCoordinates("Region1", documentState,
                                                 testImage.width(), testImage.height(),
                                                 scaleFactor1, imageOffset);
    
    // Zoom in
    double zoomLevel2 = 2.0;
    double scaleFactor2 = 1.0;
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor2, imageOffset, zoomLevel2);
    
    QRectF coords2 = cache->getCachedCoordinates("Region1", documentState,
                                                 testImage.width(), testImage.height(),
                                                 scaleFactor2, imageOffset);
    
    // Coordinates should be different after zoom
    QVERIFY(coords1 != coords2);
    // Zoomed in coordinates should be larger
    QVERIFY(coords2.width() > coords1.width());
    QVERIFY(coords2.height() > coords1.height());
}

void TestCanvasCoordinateCache::testCacheAfterPan() {
    double scaleFactor = 0.5;
    QPointF imageOffset1(100.0, 50.0);
    double zoomLevel = 1.0;
    
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor, imageOffset1, zoomLevel);
    
    QRectF coords1 = cache->getCachedCoordinates("Region1", documentState,
                                                 testImage.width(), testImage.height(),
                                                 scaleFactor, imageOffset1);
    
    // Pan (change offset)
    QPointF imageOffset2(200.0, 150.0);
    cache->updateCache(documentState, 
                      testImage.width(), testImage.height(),
                      scaleFactor, imageOffset2, zoomLevel);
    
    QRectF coords2 = cache->getCachedCoordinates("Region1", documentState,
                                                 testImage.width(), testImage.height(),
                                                 scaleFactor, imageOffset2);
    
    // Coordinates should be different after pan
    QVERIFY(coords1 != coords2);
    // Pan should shift coordinates
    QVERIFY(coords2.x() != coords1.x() || coords2.y() != coords1.y());
}

QTEST_MAIN(TestCanvasCoordinateCache)
#include "test_canvas_coordinate_cache.moc"

