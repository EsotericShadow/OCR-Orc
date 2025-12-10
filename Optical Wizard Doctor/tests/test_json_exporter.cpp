// Test file for JsonExporter
// Tests JSON export functionality

#include <QtTest/QtTest>
#include "../src/export/JsonExporter.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTemporaryDir>
#include <QtGui/QImage>

using namespace ocr_orc;

class TestJsonExporter : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testExportEmptyState();
    void testExportWithRegions();
    void testExportWithGroups();
    void testExportWithPdfPath();
    void testExportRoundTrip();
    void testExportFileCreation();
    void testExportWithRegionType();
    void testExportWithPercentageFill();
    void testExportBackwardCompatibility();

private:
    QTemporaryDir* tempDir;
    QString getTempFilePath(const QString& filename);
};

void TestJsonExporter::initTestCase() {
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestJsonExporter::cleanupTestCase() {
    delete tempDir;
}

QString TestJsonExporter::getTempFilePath(const QString& filename) {
    return tempDir->filePath(filename);
}

void TestJsonExporter::testExportEmptyState() {
    DocumentState state;
    QString filePath = getTempFilePath("empty.json");
    
    JsonExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    // Read and verify JSON
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QVERIFY(root.contains("regions"));
    QVERIFY(root.contains("groups"));
    QVERIFY(root["regions"].isObject());
    QVERIFY(root["groups"].isObject());
}

void TestJsonExporter::testExportWithRegions() {
    DocumentState state;
    state.pdfPath = "/test/path.pdf";
    
    // Add test regions
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    state.addRegion("Region1", RegionData("Region1", norm1, "blue"));
    state.addRegion("Region2", RegionData("Region2", norm2, "red"));
    
    QString filePath = getTempFilePath("regions.json");
    JsonExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    // Read and verify
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QVERIFY(root.contains("regions"));
    QJsonObject regions = root["regions"].toObject();
    QCOMPARE(regions.size(), 2);
    QVERIFY(regions.contains("Region1"));
    QVERIFY(regions.contains("Region2"));
    
    // Verify region data (name is the key, not in the object)
    QJsonObject region1Obj = regions["Region1"].toObject();
    QCOMPARE(region1Obj["color"].toString(), QString("blue"));
    QVERIFY(region1Obj.contains("normalized_coords"));
    // Verify new fields are not present when default values
    QVERIFY(!region1Obj.contains("region_type"));  // Should not be present when "none"
    QVERIFY(!region1Obj.contains("percentage_fill"));  // Should not be present when "none"
}

void TestJsonExporter::testExportWithGroups() {
    DocumentState state;
    
    // Add regions
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    state.addRegion("Region1", RegionData("Region1", norm1, "blue", "Group1"));
    state.addRegion("Region2", RegionData("Region2", norm2, "red", "Group1"));
    
    // Create group
    state.createGroup("Group1");
    state.addRegionToGroup("Region1", "Group1");
    state.addRegionToGroup("Region2", "Group1");
    
    QString filePath = getTempFilePath("groups.json");
    JsonExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    // Read and verify
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QVERIFY(root.contains("groups"));
    QJsonObject groups = root["groups"].toObject();
    QVERIFY(groups.contains("Group1"));
    
    // Groups are stored as arrays of region names
    QJsonArray groupRegions = groups["Group1"].toArray();
    QCOMPARE(groupRegions.size(), 2);
}

void TestJsonExporter::testExportWithPdfPath() {
    DocumentState state;
    state.pdfPath = "/test/document.pdf";
    
    QString filePath = getTempFilePath("pdf_path.json");
    JsonExporter::exportToFile(state, filePath);
    
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QVERIFY(root.contains("pdf_path"));
    QCOMPARE(root["pdf_path"].toString(), QString("/test/document.pdf"));
}

void TestJsonExporter::testExportRoundTrip() {
    DocumentState original;
    original.pdfPath = "/test/roundtrip.pdf";
    
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    original.addRegion("TestRegion", RegionData("TestRegion", norm, "blue", "TestGroup"));
    original.createGroup("TestGroup");
    original.addRegionToGroup("TestRegion", "TestGroup");
    
    QString filePath = getTempFilePath("roundtrip.json");
    JsonExporter::exportToFile(original, filePath);
    
    // Import back
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    DocumentState imported = DocumentState::fromJson(doc.object());
    
    QCOMPARE(imported.pdfPath, original.pdfPath);
    QVERIFY(imported.hasRegion("TestRegion"));
    QVERIFY(imported.hasGroup("TestGroup"));
    
    RegionData origRegion = original.getRegion("TestRegion");
    RegionData impRegion = imported.getRegion("TestRegion");
    
    QCOMPARE(impRegion.name, origRegion.name);
    QCOMPARE(impRegion.color, origRegion.color);
    QCOMPARE(impRegion.group, origRegion.group);
}

void TestJsonExporter::testExportFileCreation() {
    DocumentState state;
    QString filePath = getTempFilePath("new_file.json");
    
    // File should not exist before export
    QVERIFY(!QFile::exists(filePath));
    
    JsonExporter::exportToFile(state, filePath);
    
    // File should exist after export
    QVERIFY(QFile::exists(filePath));
    
    // File should be readable
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY(file.size() > 0);
}

void TestJsonExporter::testExportWithRegionType() {
    DocumentState state;
    state.pdfPath = "/test/path.pdf";
    
    // Add region with regionType
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    RegionData region("TestRegion", norm, "blue", "", "rect", "text", "none");
    state.addRegion("TestRegion", region);
    
    QString filePath = getTempFilePath("region_type.json");
    JsonExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    // Read and verify JSON
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QVERIFY(root.contains("regions"));
    QJsonObject regions = root["regions"].toObject();
    QVERIFY(regions.contains("TestRegion"));
    
    QJsonObject regionObj = regions["TestRegion"].toObject();
    QVERIFY(regionObj.contains("region_type"));
    QCOMPARE(regionObj["region_type"].toString(), QString("text"));
}

void TestJsonExporter::testExportWithPercentageFill() {
    DocumentState state;
    state.pdfPath = "/test/path.pdf";
    
    // Add region with percentageFill
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    RegionData region("TestRegion", norm, "blue", "", "rect", "none", "standard");
    state.addRegion("TestRegion", region);
    
    QString filePath = getTempFilePath("percentage_fill.json");
    JsonExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    // Read and verify JSON
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QVERIFY(root.contains("regions"));
    QJsonObject regions = root["regions"].toObject();
    QVERIFY(regions.contains("TestRegion"));
    
    QJsonObject regionObj = regions["TestRegion"].toObject();
    QVERIFY(regionObj.contains("percentage_fill"));
    QCOMPARE(regionObj["percentage_fill"].toString(), QString("standard"));
}

void TestJsonExporter::testExportBackwardCompatibility() {
    DocumentState state;
    state.pdfPath = "/test/path.pdf";
    
    // Add region with default values (should not appear in JSON)
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    RegionData region("TestRegion", norm, "blue");
    // regionType and percentageFill default to "none"
    state.addRegion("TestRegion", region);
    
    QString filePath = getTempFilePath("backward_compat.json");
    JsonExporter::exportToFile(state, filePath);
    
    QVERIFY(QFile::exists(filePath));
    
    // Read and verify JSON - new fields should NOT be present
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    
    QVERIFY(root.contains("regions"));
    QJsonObject regions = root["regions"].toObject();
    QVERIFY(regions.contains("TestRegion"));
    
    QJsonObject regionObj = regions["TestRegion"].toObject();
    // Should not contain region_type or percentage_fill when they are "none"
    QVERIFY(!regionObj.contains("region_type"));
    QVERIFY(!regionObj.contains("percentage_fill"));
}

QTEST_MAIN(TestJsonExporter)
#include "test_json_exporter.moc"

