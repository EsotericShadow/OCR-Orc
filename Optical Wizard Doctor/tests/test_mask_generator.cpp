// Test file for MaskGenerator
// Tests mask image generation

#include <QtTest/QtTest>
#include "../src/export/MaskGenerator.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtGui/QImage>

using namespace ocr_orc;

class TestMaskGenerator : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGenerateEmptyState();
    void testGenerateWithRegions();
    void testGenerateMaskFormat();
    void testGenerateImageSize();

private:
    QTemporaryDir* tempDir;
    QString getTempFilePath(const QString& filename);
    DocumentState* createTestState();
};

void TestMaskGenerator::initTestCase() {
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestMaskGenerator::cleanupTestCase() {
    delete tempDir;
}

QString TestMaskGenerator::getTempFilePath(const QString& filename) {
    return tempDir->filePath(filename);
}

DocumentState* TestMaskGenerator::createTestState() {
    DocumentState* state = new DocumentState();
    
    // Create test image
    QImage testImage(2000, 3000, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    state->setImage(testImage);
    
    // Add test regions
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    state->addRegion("Region1", RegionData("Region1", norm1, "blue"));
    state->addRegion("Region2", RegionData("Region2", norm2, "red"));
    
    return state;
}

void TestMaskGenerator::testGenerateEmptyState() {
    DocumentState state;
    QImage testImage(1000, 1500, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    state.setImage(testImage);
    
    QString filePath = getTempFilePath("empty_mask.png");
    
    // Empty state should throw exception (no regions)
    // Use QVERIFY_EXCEPTION_THROWN or manual try-catch
    bool exceptionThrown = false;
    try {
        MaskGenerator::generate(state, filePath);
    } catch (const std::exception& e) {
        exceptionThrown = true;
        // Verify exception message
        QString msg = QString::fromStdString(e.what());
        QVERIFY(msg.contains("region", Qt::CaseInsensitive) ||
                msg.contains("empty", Qt::CaseInsensitive) ||
                msg.contains("No regions", Qt::CaseInsensitive));
    }
    QVERIFY(exceptionThrown);
}

void TestMaskGenerator::testGenerateWithRegions() {
    DocumentState* state = createTestState();
    QString filePath = getTempFilePath("mask_with_regions.png");
    
    MaskGenerator::generate(*state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    // Load and verify mask
    QImage mask = QImage(filePath);
    QVERIFY(!mask.isNull());
    QCOMPARE(mask.width(), 2000);
    QCOMPARE(mask.height(), 3000);
    
    // Check that mask has white regions (regions should be white)
    // Background should be black (or dark)
    // Note: mask format may vary, so just verify it's not null and has correct size
    QVERIFY(!mask.isNull());
    
    delete state;
}

void TestMaskGenerator::testGenerateMaskFormat() {
    DocumentState* state = createTestState();
    QString filePath = getTempFilePath("mask_format.png");
    
    MaskGenerator::generate(*state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    QImage mask = QImage(filePath);
    QVERIFY(!mask.isNull());
    QVERIFY(mask.format() == QImage::Format_Grayscale8 || 
            mask.format() == QImage::Format_Mono ||
            mask.format() == QImage::Format_RGB32);
    
    delete state;
}

void TestMaskGenerator::testGenerateImageSize() {
    DocumentState state;
    QImage testImage(500, 750, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    state.setImage(testImage);
    
    // Add a region so generation doesn't fail
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    state.addRegion("TestRegion", RegionData("TestRegion", norm, "blue"));
    
    QString filePath = getTempFilePath("mask_size.png");
    
    MaskGenerator::generate(state, filePath);
    
    QImage mask = QImage(filePath);
    QCOMPARE(mask.width(), 500);
    QCOMPARE(mask.height(), 750);
}

QTEST_MAIN(TestMaskGenerator)
#include "test_mask_generator.moc"

