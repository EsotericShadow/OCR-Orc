// Test file for OcrTextExtractor
// Tests OCR text extraction functionality

#include <QtTest/QtTest>
#include "../src/utils/OcrTextExtractor.h"
#include <QtGui/QImage>

using namespace ocr_orc;

class TestOcrTextExtractor : public QObject {
    Q_OBJECT

private slots:
    void testExtractorCreation();
    void testPreprocessing();
    void testTypeInference();
    void testConfidenceFiltering();
    // Note: Full OCR extraction tests require Tesseract installation
    // and test images - these would be integration tests
};

void TestOcrTextExtractor::testExtractorCreation() {
    OcrTextExtractor extractor;
    QVERIFY(extractor.getMinConfidence() == 50.0);
    
    extractor.setMinConfidence(70.0);
    QVERIFY(extractor.getMinConfidence() == 70.0);
}

void TestOcrTextExtractor::testTypeInference() {
    OcrTextExtractor extractor;
    
    // Note: inferTypeFromText is private, so we test indirectly
    // through extractTextRegions if we had test images
    // For now, this is a placeholder
    QVERIFY(true);  // Placeholder test
}

void TestOcrTextExtractor::testPreprocessing() {
    // Create a test image
    QImage testImage(200, 100, QImage::Format_RGB32);
    testImage.fill(Qt::white);
    
    OcrTextExtractor extractor;
    // Note: preprocessImage is private, so we test indirectly
    // through extractTextRegions
    QVERIFY(true);  // Placeholder test
}

void TestOcrTextExtractor::testConfidenceFiltering() {
    OcrTextExtractor extractor;
    
    QList<OCRTextRegion> regions;
    OCRTextRegion region1;
    region1.confidence = 60.0;
    regions.append(region1);
    
    OCRTextRegion region2;
    region2.confidence = 40.0;
    regions.append(region2);
    
    OCRTextRegion region3;
    region3.confidence = 80.0;
    regions.append(region3);
    
    QList<OCRTextRegion> filtered = extractor.filterByConfidence(regions, 50.0);
    QVERIFY(filtered.size() == 2);  // Should filter out region2 (40%)
    QVERIFY(filtered[0].confidence == 60.0);
    QVERIFY(filtered[1].confidence == 80.0);
}

QTEST_MAIN(TestOcrTextExtractor)
#include "test_ocr_text_extractor.moc"
