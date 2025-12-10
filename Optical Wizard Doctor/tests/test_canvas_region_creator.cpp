// Test file for CanvasRegionCreator
// Tests region creation logic

#include <QtTest/QtTest>
#include "../src/ui/canvas/core/regions/CanvasRegionCreator.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtGui/QImage>

using namespace ocr_orc;

class TestCanvasRegionCreator : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testStartRegionCreation();
    void testStartRegionCreationInvalid();
    void testUpdateRegionCreation();
    void testFinishRegionCreation();
    void testFinishRegionCreationInvalid();

private:
    DocumentState* documentState;
    CanvasRegionCreator* regionCreator;
    QImage testImage;
    QRectF imageRect;
    const double scaleFactor = 0.5;
    const QPointF imageOffset = QPointF(100.0, 50.0);
    
    bool validateRegion(const QRectF& rect, const QRectF& imgRect) {
        return !rect.isEmpty() && 
               rect.width() >= 10.0 && 
               rect.height() >= 10.0 &&
               imgRect.contains(rect);
    }
};

void TestCanvasRegionCreator::initTestCase() {
    documentState = new DocumentState();
    regionCreator = new CanvasRegionCreator();
    
    // Create test image
    testImage = QImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    documentState->setImage(testImage);
    
    // Calculate image rect
    imageRect = QRectF(100.0, 50.0, 1000.0, 1500.0);
}

void TestCanvasRegionCreator::cleanupTestCase() {
    delete regionCreator;
    delete documentState;
}

void TestCanvasRegionCreator::testStartRegionCreation() {
    bool isCreating = false;
    QPointF creationStartPos;
    QRectF tempRect;
    
    QPointF validPos(200.0, 100.0); // Inside image rect
    
    bool started = regionCreator->startRegionCreation(validPos, imageRect,
                                                      isCreating, creationStartPos, tempRect);
    
    QVERIFY(started);
    QVERIFY(isCreating);
    QCOMPARE(creationStartPos, validPos);
    QVERIFY(tempRect.isEmpty());
}

void TestCanvasRegionCreator::testStartRegionCreationInvalid() {
    bool isCreating = false;
    QPointF creationStartPos;
    QRectF tempRect;
    
    QPointF invalidPos(2000.0, 3000.0); // Outside image rect
    
    bool started = regionCreator->startRegionCreation(invalidPos, imageRect,
                                                      isCreating, creationStartPos, tempRect);
    
    QVERIFY(!started);
    QVERIFY(!isCreating);
}

void TestCanvasRegionCreator::testUpdateRegionCreation() {
    bool isCreating = true;
    QPointF creationStartPos(200.0, 100.0);
    QRectF tempRect;
    
    QPointF currentPos(300.0, 200.0);
    
    regionCreator->updateRegionCreation(currentPos, isCreating, creationStartPos,
                                       imageRect, tempRect);
    
    QVERIFY(!tempRect.isEmpty());
    QVERIFY(tempRect.width() > 0);
    QVERIFY(tempRect.height() > 0);
    QCOMPARE(tempRect.topLeft(), creationStartPos);
    QCOMPARE(tempRect.bottomRight(), currentPos);
}

void TestCanvasRegionCreator::testFinishRegionCreation() {
    bool isCreating = true;
    QRectF tempRect(200.0, 100.0, 300.0, 200.0);
    
    QString regionName = "NewRegion";
    QString color = "blue";
    QString group = "";
    
    bool finished = regionCreator->finishRegionCreation(regionName, color, group, "rect",
                                                        isCreating, tempRect,
                                                        documentState, testImage,
                                                        scaleFactor, imageOffset,
                                                        [this](const QRectF& rect, const QRectF& imgRect) {
                                                            return this->validateRegion(rect, imgRect);
                                                        });
    
    QVERIFY(finished);
    QVERIFY(!isCreating);
    QVERIFY(documentState->hasRegion(regionName));
    
    RegionData region = documentState->getRegion(regionName);
    QCOMPARE(region.name, regionName);
    QCOMPARE(region.color, color);
    QVERIFY(region.isValid());
}

void TestCanvasRegionCreator::testFinishRegionCreationInvalid() {
    bool isCreating = true;
    QRectF tempRect(200.0, 100.0, 5.0, 5.0); // Too small
    
    QString regionName = "InvalidRegion";
    QString color = "red";
    
    int regionCountBefore = documentState->getAllRegionNames().size();
    
    bool finished = regionCreator->finishRegionCreation(regionName, color, QString(), "rect",
                                                        isCreating, tempRect,
                                                        documentState, testImage,
                                                        scaleFactor, imageOffset,
                                                        [this](const QRectF& rect, const QRectF& imgRect) {
                                                            return this->validateRegion(rect, imgRect);
                                                        });
    
    QVERIFY(!finished);
    QVERIFY(!isCreating);
    
    // Region should not have been added
    int regionCountAfter = documentState->getAllRegionNames().size();
    QCOMPARE(regionCountAfter, regionCountBefore);
    QVERIFY(!documentState->hasRegion(regionName));
}

QTEST_MAIN(TestCanvasRegionCreator)
#include "test_canvas_region_creator.moc"

