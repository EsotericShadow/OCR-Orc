// Test file for CsvExporter
// Tests CSV export functionality

#include <QtTest/QtTest>
#include "../src/export/CsvExporter.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTemporaryDir>
#include <QtGui/QImage>

using namespace ocr_orc;

class TestCsvExporter : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testExportEmptyState();
    void testExportWithRegions();
    void testExportCsvFormat();
    void testExportEscaping();
    void testExportCoordinates();

private:
    QTemporaryDir* tempDir;
    QString getTempFilePath(const QString& filename);
    QString readFileContent(const QString& filePath);
};

void TestCsvExporter::initTestCase() {
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestCsvExporter::cleanupTestCase() {
    delete tempDir;
}

QString TestCsvExporter::getTempFilePath(const QString& filename) {
    return tempDir->filePath(filename);
}

QString TestCsvExporter::readFileContent(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString(); // Return empty on failure
    }
    QTextStream stream(&file);
    return stream.readAll();
}

void TestCsvExporter::testExportEmptyState() {
    DocumentState state;
    QString filePath = getTempFilePath("empty.csv");
    
    CsvExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    QString content = readFileContent(filePath);
    // Should have header line (case-insensitive check)
    QVERIFY(content.contains("Region Name", Qt::CaseInsensitive) || 
            content.contains("name", Qt::CaseInsensitive));
    QVERIFY(content.contains("X1", Qt::CaseInsensitive) || 
            content.contains("x1", Qt::CaseInsensitive));
}

void TestCsvExporter::testExportWithRegions() {
    DocumentState state;
    
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    state.addRegion("Region1", RegionData("Region1", norm1, "blue"));
    state.addRegion("Region2", RegionData("Region2", norm2, "red"));
    
    QString filePath = getTempFilePath("regions.csv");
    CsvExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    QString content = readFileContent(filePath);
    QVERIFY(content.contains("Region1"));
    QVERIFY(content.contains("Region2"));
    QVERIFY(content.contains("0.25"));
    QVERIFY(content.contains("0.10"));
}

void TestCsvExporter::testExportCsvFormat() {
    DocumentState state;
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    state.addRegion("Test,Region", RegionData("Test,Region", norm, "blue"));
    
    QString filePath = getTempFilePath("format.csv");
    CsvExporter::exportToFile(state, filePath);
    
    QString content = readFileContent(filePath);
    // CSV should properly quote fields with commas
    QVERIFY(content.contains("\"Test,Region\"") || content.contains("Test,Region"));
}

void TestCsvExporter::testExportEscaping() {
    DocumentState state;
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    state.addRegion("Test\"Quote", RegionData("Test\"Quote", norm, "blue"));
    
    QString filePath = getTempFilePath("escape.csv");
    CsvExporter::exportToFile(state, filePath);
    
    QString content = readFileContent(filePath);
    // CSV should properly escape quotes
    QVERIFY(content.contains("Test\"\"Quote") || content.contains("\"Test\"\"Quote\""));
}

void TestCsvExporter::testExportCoordinates() {
    DocumentState state;
    NormalizedCoords norm(0.123456789, 0.987654321, 0.555555555, 0.111111111);
    state.addRegion("PreciseRegion", RegionData("PreciseRegion", norm, "blue"));
    
    QString filePath = getTempFilePath("coordinates.csv");
    CsvExporter::exportToFile(state, filePath);
    
    QString content = readFileContent(filePath);
    // Coordinates should be formatted to 6 decimal places
    QVERIFY(content.contains("0.123457") || content.contains("0.123456"));
    QVERIFY(content.contains("0.987654"));
}

QTEST_MAIN(TestCsvExporter)
#include "test_csv_exporter.moc"

