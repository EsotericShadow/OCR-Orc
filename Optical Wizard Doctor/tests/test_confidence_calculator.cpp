// Test file for ConfidenceCalculator
// Tests confidence calculation and filtering

#include <QtTest/QtTest>
#include "../src/utils/ConfidenceCalculator.h"
#include "../src/utils/RegionDetector.h"
#include <QtCore/QList>

using namespace ocr_orc;

class TestConfidenceCalculator : public QObject {
    Q_OBJECT

private slots:
    void testConfidenceCalculation();
    void testFiltering();
    void testWeights();
    void testThresholds();
};

void TestConfidenceCalculator::testConfidenceCalculation() {
    // Test with sample values
    double ocrConf = 80.0;  // 80% OCR confidence
    double lineScore = 0.8;
    double rectScore = 0.7;
    double patternScore = 1.0;
    
    double confidence = ConfidenceCalculator::calculate(ocrConf, lineScore, rectScore, patternScore);
    
    // Should be in valid range
    QVERIFY(confidence >= 0.0 && confidence <= 1.0);
    
    // With these values and default weights (40/30/20/10), should be high
    QVERIFY(confidence > 0.7);
}

void TestConfidenceCalculator::testFiltering() {
    QList<DetectedRegion> regions;
    
    DetectedRegion region1;
    region1.confidence = 0.8;  // High
    regions.append(region1);
    
    DetectedRegion region2;
    region2.confidence = 0.6;  // Medium
    regions.append(region2);
    
    DetectedRegion region3;
    region3.confidence = 0.3;  // Low
    regions.append(region3);
    
    // Filter with 0.5 threshold
    QList<DetectedRegion> filtered = ConfidenceCalculator::filterRegions(regions, 0.5);
    
    // Should include high and medium, exclude low
    QVERIFY(filtered.size() >= 2);
    QVERIFY(filtered[0].confidence >= 0.5);
}

void TestConfidenceCalculator::testWeights() {
    double ocrW, lineW, rectW, patternW;
    ConfidenceCalculator::getWeights(ocrW, lineW, rectW, patternW);
    
    // Check default weights
    QVERIFY(ocrW == 0.4);
    QVERIFY(lineW == 0.3);
    QVERIFY(rectW == 0.2);
    QVERIFY(patternW == 0.1);
    
    // Set new weights
    ConfidenceCalculator::setWeights(0.5, 0.3, 0.15, 0.05);
    ConfidenceCalculator::getWeights(ocrW, lineW, rectW, patternW);
    QVERIFY(ocrW == 0.5);
    QVERIFY(lineW == 0.3);
}

void TestConfidenceCalculator::testThresholds() {
    double high, medium;
    ConfidenceCalculator::getThresholds(high, medium);
    
    // Check default thresholds
    QVERIFY(high == 0.7);
    QVERIFY(medium == 0.5);
    
    // Set new thresholds
    ConfidenceCalculator::setThresholds(0.8, 0.6);
    ConfidenceCalculator::getThresholds(high, medium);
    QVERIFY(high == 0.8);
    QVERIFY(medium == 0.6);
}

QTEST_MAIN(TestConfidenceCalculator)
#include "test_confidence_calculator.moc"
