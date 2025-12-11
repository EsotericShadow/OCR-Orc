// Test file for TextRegionRefiner
// Tests text region refinement functionality

#include <QtTest/QtTest>
#include "../src/utils/TextRegionRefiner.h"
#include "../src/utils/OcrTextExtractor.h"
#include <opencv2/opencv.hpp>
#include <QtGui/QImage>

using namespace ocr_orc;

class TestTextRegionRefiner : public QObject {
    Q_OBJECT

private slots:
    void testRefinerCreation();
    void testExpansionRadius();
    void testScoring();
};

void TestTextRegionRefiner::testRefinerCreation() {
    TextRegionRefiner refiner;
    QVERIFY(refiner.getExpansionRadius() == 20);
    
    refiner.setExpansionRadius(30);
    QVERIFY(refiner.getExpansionRadius() == 30);
}

void TestTextRegionRefiner::testExpansionRadius() {
    TextRegionRefiner refiner;
    refiner.setExpansionRadius(25);
    QVERIFY(refiner.getExpansionRadius() == 25);
}

void TestTextRegionRefiner::testScoring() {
    TextRegionRefiner refiner;
    
    // Test that scores are in valid range
    double lineScore = refiner.getLineDetectionScore();
    double rectScore = refiner.getRectangularityScore();
    
    QVERIFY(lineScore >= 0.0 && lineScore <= 1.0);
    QVERIFY(rectScore >= 0.0 && rectScore <= 1.0);
}

QTEST_MAIN(TestTextRegionRefiner)
#include "test_text_region_refiner.moc"
