// Test file for CanvasSelectionManager
// Tests selection management operations

#include <QtTest/QtTest>
#include "../src/ui/canvas/core/selection/CanvasSelectionManager.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtGui/QImage>

using namespace ocr_orc;

class TestCanvasSelectionManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testClearSelection();
    void testSelectRegion();
    void testToggleRegionSelection();
    void testAddToSelection();
    void testFindRegionsInBox();

private:
    DocumentState* documentState;
    CanvasSelectionManager* selectionManager;
    QImage testImage;
    QSet<QString> selectedRegions;
    QString primarySelectedRegion;
    const double scaleFactor = 0.5;
    const QPointF imageOffset = QPointF(100.0, 50.0);
};

void TestCanvasSelectionManager::initTestCase() {
    documentState = new DocumentState();
    selectionManager = new CanvasSelectionManager();
    
    // Create test image
    testImage = QImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    documentState->setImage(testImage);
    
    // Add test regions
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    NormalizedCoords norm3(0.10, 0.30, 0.20, 0.40);
    documentState->addRegion("Region1", RegionData("Region1", norm1, "blue"));
    documentState->addRegion("Region2", RegionData("Region2", norm2, "red"));
    documentState->addRegion("Region3", RegionData("Region3", norm3, "green"));
    documentState->synchronizeCoordinates();
}

void TestCanvasSelectionManager::cleanupTestCase() {
    delete selectionManager;
    delete documentState;
}

void TestCanvasSelectionManager::testClearSelection() {
    selectedRegions.insert("Region1");
    selectedRegions.insert("Region2");
    primarySelectedRegion = "Region1";
    
    selectionManager->clearSelection(selectedRegions, primarySelectedRegion);
    
    QVERIFY(selectedRegions.isEmpty());
    QVERIFY(primarySelectedRegion.isEmpty());
}

void TestCanvasSelectionManager::testSelectRegion() {
    selectedRegions.clear();
    primarySelectedRegion.clear();
    
    selectionManager->selectRegion("Region1", selectedRegions, primarySelectedRegion);
    
    QCOMPARE(selectedRegions.size(), 1);
    QVERIFY(selectedRegions.contains("Region1"));
    QCOMPARE(primarySelectedRegion, QString("Region1"));
    
    // Selecting another region should clear previous selection
    selectionManager->selectRegion("Region2", selectedRegions, primarySelectedRegion);
    
    QCOMPARE(selectedRegions.size(), 1);
    QVERIFY(selectedRegions.contains("Region2"));
    QVERIFY(!selectedRegions.contains("Region1"));
    QCOMPARE(primarySelectedRegion, QString("Region2"));
}

void TestCanvasSelectionManager::testToggleRegionSelection() {
    selectedRegions.clear();
    primarySelectedRegion.clear();
    
    // Toggle on
    selectionManager->toggleRegionSelection("Region1", selectedRegions, primarySelectedRegion);
    
    QVERIFY(selectedRegions.contains("Region1"));
    QCOMPARE(primarySelectedRegion, QString("Region1"));
    
    // Toggle off
    selectionManager->toggleRegionSelection("Region1", selectedRegions, primarySelectedRegion);
    
    QVERIFY(!selectedRegions.contains("Region1"));
    QVERIFY(primarySelectedRegion.isEmpty());
    
    // Toggle multiple regions
    selectionManager->toggleRegionSelection("Region1", selectedRegions, primarySelectedRegion);
    selectionManager->toggleRegionSelection("Region2", selectedRegions, primarySelectedRegion);
    
    QCOMPARE(selectedRegions.size(), 2);
    QVERIFY(selectedRegions.contains("Region1"));
    QVERIFY(selectedRegions.contains("Region2"));
    // Primary is the last selected region (Region2)
    QCOMPARE(primarySelectedRegion, QString("Region2"));
}

void TestCanvasSelectionManager::testAddToSelection() {
    selectedRegions.clear();
    primarySelectedRegion.clear();
    
    // Add single region
    QSet<QString> toAdd1;
    toAdd1.insert("Region1");
    selectionManager->addToSelection(toAdd1, selectedRegions, primarySelectedRegion);
    
    QVERIFY(selectedRegions.contains("Region1"));
    QCOMPARE(primarySelectedRegion, QString("Region1"));
    
    // Add multiple regions
    QSet<QString> toAdd2;
    toAdd2.insert("Region2");
    toAdd2.insert("Region3");
    selectionManager->addToSelection(toAdd2, selectedRegions, primarySelectedRegion);
    
    QCOMPARE(selectedRegions.size(), 3);
    QVERIFY(selectedRegions.contains("Region1"));
    QVERIFY(selectedRegions.contains("Region2"));
    QVERIFY(selectedRegions.contains("Region3"));
    // Primary stays as Region1 (still in selection)
    QCOMPARE(primarySelectedRegion, QString("Region1"));
}

void TestCanvasSelectionManager::testFindRegionsInBox() {
    // Synchronize coordinates first
    documentState->synchronizeCoordinates();
    
    // Get actual canvas coordinates of Region1 to create a proper selection box
    RegionData region1 = documentState->getRegion("Region1");
    region1.syncFromNormalized(testImage.width(), testImage.height(), 
                               scaleFactor, imageOffset);
    QRectF region1Rect = region1.toQRectF();
    
    // Create a selection box that overlaps Region1
    QRectF selectionBox(region1Rect.x() - 10.0, region1Rect.y() - 10.0,
                       region1Rect.width() + 20.0, region1Rect.height() + 20.0);
    
    QSet<QString> found = selectionManager->findRegionsInBox(selectionBox, documentState,
                                                             testImage, scaleFactor, imageOffset);
    
    // Should find Region1
    QVERIFY(found.size() >= 1);
    QVERIFY(found.contains("Region1"));
    
    // Create a box that shouldn't contain any regions
    QRectF emptyBox(2000.0, 3000.0, 2100.0, 3100.0);
    QSet<QString> foundEmpty = selectionManager->findRegionsInBox(emptyBox, documentState,
                                                                  testImage, scaleFactor, imageOffset);
    
    QVERIFY(foundEmpty.isEmpty());
}

QTEST_MAIN(TestCanvasSelectionManager)
#include "test_canvas_selection_manager.moc"

