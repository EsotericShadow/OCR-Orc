// Test file for JsonImporter
// Tests JSON import functionality

#include <QtTest/QtTest>
#include "../src/export/JsonImporter.h"
#include "../src/models/DocumentState.h"
#include "../src/core/CoordinateSystem.h"
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTemporaryFile>
#include <QtGui/QImage>

using namespace ocr_orc;

class TestJsonImporter : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testImportEmptyState();
    void testImportWithRegions();
    void testImportWithGroups();
    void testImportWithPdfPath();
    void testImportInvalidJson();
    void testImportMissingFields();
    void testImportRoundTrip();
    void testImportWithNewFields();
    void testImportBackwardCompatibility();
    void testImportInvalidValues();

private:
    QTemporaryDir* tempDir;
    QString createTestJsonFile(const QJsonObject& json);
};

void TestJsonImporter::initTestCase() {
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestJsonImporter::cleanupTestCase() {
    delete tempDir;
}

QString TestJsonImporter::createTestJsonFile(const QJsonObject& json) {
    QString filePath = tempDir->filePath("test.json");
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString(); // Return empty on failure
    }
    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
    return filePath;
}

void TestJsonImporter::testImportEmptyState() {
    QJsonObject json;
    json["regions"] = QJsonObject();
    json["groups"] = QJsonObject();
    
    QString filePath = createTestJsonFile(json);
    
    DocumentState state;
    JsonImporter::importFromFile(state, filePath);
    
    QVERIFY(state.regions.isEmpty());
    QVERIFY(state.groups.isEmpty());
}

void TestJsonImporter::testImportWithRegions() {
    QJsonObject region1;
    region1["name"] = "Region1";
    region1["color"] = "blue";
    QJsonObject coords1;
    coords1["x1"] = 0.25;
    coords1["y1"] = 0.10;
    coords1["x2"] = 0.35;
    coords1["y2"] = 0.20;
    region1["normalized_coords"] = coords1;
    
    QJsonObject regions;
    regions["Region1"] = region1;
    
    QJsonObject json;
    json["regions"] = regions;
    json["groups"] = QJsonObject();
    
    QString filePath = createTestJsonFile(json);
    
    DocumentState state;
    JsonImporter::importFromFile(state, filePath);
    
    QVERIFY(state.hasRegion("Region1"));
    RegionData region = state.getRegion("Region1");
    QCOMPARE(region.name, QString("Region1"));
    QCOMPARE(region.color, QString("blue"));
    QCOMPARE(region.normalizedCoords.x1, 0.25);
}

void TestJsonImporter::testImportWithGroups() {
    QJsonObject region1;
    region1["name"] = "Region1";
    region1["color"] = "blue";
    region1["group"] = "Group1";
    QJsonObject coords1;
    coords1["x1"] = 0.25;
    coords1["y1"] = 0.10;
    coords1["x2"] = 0.35;
    coords1["y2"] = 0.20;
    region1["normalized_coords"] = coords1;
    
    QJsonObject regions;
    regions["Region1"] = region1;
    
    QJsonObject group1;
    group1["name"] = "Group1";
    QJsonArray groupRegions;
    groupRegions.append("Region1");
    group1["regions"] = groupRegions;
    
    QJsonObject groups;
    groups["Group1"] = group1;
    
    QJsonObject json;
    json["regions"] = regions;
    json["groups"] = groups;
    
    QString filePath = createTestJsonFile(json);
    
    DocumentState state;
    JsonImporter::importFromFile(state, filePath);
    
    QVERIFY(state.hasGroup("Group1"));
    GroupData group = state.getGroup("Group1");
    QVERIFY(group.contains("Region1"));
}

void TestJsonImporter::testImportWithPdfPath() {
    QJsonObject json;
    json["pdf_path"] = "/test/document.pdf";
    json["regions"] = QJsonObject();
    json["groups"] = QJsonObject();
    
    QString filePath = createTestJsonFile(json);
    
    DocumentState state;
    JsonImporter::importFromFile(state, filePath);
    
    QCOMPARE(state.pdfPath, QString("/test/document.pdf"));
}

void TestJsonImporter::testImportInvalidJson() {
    QString filePath = tempDir->filePath("invalid.json");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("This is not valid JSON {");
    file.close();
    
    DocumentState state;
    try {
        JsonImporter::importFromFile(state, filePath);
        QFAIL("Should have thrown exception for invalid JSON");
    } catch (const std::exception&) {
        // Expected
    }
}

void TestJsonImporter::testImportMissingFields() {
    QJsonObject json;
    // Missing "regions" field
    json["groups"] = QJsonObject();
    
    QString filePath = createTestJsonFile(json);
    
    DocumentState state;
    try {
        JsonImporter::importFromFile(state, filePath);
        QFAIL("Should have thrown exception for missing regions field");
    } catch (const std::exception&) {
        // Expected
    }
}

void TestJsonImporter::testImportRoundTrip() {
    // Create original state
    DocumentState original;
    original.pdfPath = "/test/roundtrip.pdf";
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    original.addRegion("TestRegion", RegionData("TestRegion", norm, "blue", "TestGroup"));
    original.createGroup("TestGroup");
    original.addRegionToGroup("TestRegion", "TestGroup");
    
    // Export
    QString exportPath = tempDir->filePath("export.json");
    QJsonDocument doc(original.toJson());
    QFile exportFile(exportPath);
    QVERIFY(exportFile.open(QIODevice::WriteOnly));
    exportFile.write(doc.toJson());
    exportFile.close();
    
    // Import
    DocumentState imported;
    JsonImporter::importFromFile(imported, exportPath);
    
    // Verify
    QCOMPARE(imported.pdfPath, original.pdfPath);
    QVERIFY(imported.hasRegion("TestRegion"));
    QVERIFY(imported.hasGroup("TestGroup"));
}

void TestJsonImporter::testImportWithNewFields() {
    QJsonObject region1;
    QJsonObject coords1;
    coords1["x1"] = 0.25;
    coords1["y1"] = 0.10;
    coords1["x2"] = 0.35;
    coords1["y2"] = 0.20;
    region1["normalized_coords"] = coords1;
    region1["color"] = "blue";
    region1["region_type"] = "text";
    region1["percentage_fill"] = "standard";
    
    QJsonObject regions;
    regions["TestRegion"] = region1;
    
    QJsonObject root;
    root["regions"] = regions;
    root["groups"] = QJsonObject();
    
    QString filePath = createTestJsonFile(root);
    QVERIFY(!filePath.isEmpty());
    
    DocumentState state;
    JsonImporter::importFromFile(state, filePath);
    
    QVERIFY(state.hasRegion("TestRegion"));
    RegionData region = state.getRegion("TestRegion");
    QCOMPARE(region.regionType, QString("text"));
    QCOMPARE(region.percentageFill, QString("standard"));
}

void TestJsonImporter::testImportBackwardCompatibility() {
    // Old JSON format without region_type or percentage_fill
    QJsonObject region1;
    QJsonObject coords1;
    coords1["x1"] = 0.25;
    coords1["y1"] = 0.10;
    coords1["x2"] = 0.35;
    coords1["y2"] = 0.20;
    region1["normalized_coords"] = coords1;
    region1["color"] = "blue";
    // No region_type or percentage_fill fields
    
    QJsonObject regions;
    regions["TestRegion"] = region1;
    
    QJsonObject root;
    root["regions"] = regions;
    root["groups"] = QJsonObject();
    
    QString filePath = createTestJsonFile(root);
    QVERIFY(!filePath.isEmpty());
    
    DocumentState state;
    JsonImporter::importFromFile(state, filePath);
    
    QVERIFY(state.hasRegion("TestRegion"));
    RegionData region = state.getRegion("TestRegion");
    // Should default to "none" for both fields
    QCOMPARE(region.regionType, QString("none"));
    QCOMPARE(region.percentageFill, QString("none"));
}

void TestJsonImporter::testImportInvalidValues() {
    QJsonObject region1;
    QJsonObject coords1;
    coords1["x1"] = 0.25;
    coords1["y1"] = 0.10;
    coords1["x2"] = 0.35;
    coords1["y2"] = 0.20;
    region1["normalized_coords"] = coords1;
    region1["color"] = "blue";
    region1["region_type"] = "invalid_type";  // Invalid value
    region1["percentage_fill"] = "invalid_fill";  // Invalid value
    
    QJsonObject regions;
    regions["TestRegion"] = region1;
    
    QJsonObject root;
    root["regions"] = regions;
    root["groups"] = QJsonObject();
    
    QString filePath = createTestJsonFile(root);
    QVERIFY(!filePath.isEmpty());
    
    DocumentState state;
    JsonImporter::importFromFile(state, filePath);
    
    QVERIFY(state.hasRegion("TestRegion"));
    RegionData region = state.getRegion("TestRegion");
    // Invalid values should be stored as-is (no validation in importer)
    // The importer just reads the string values
    QCOMPARE(region.regionType, QString("invalid_type"));
    QCOMPARE(region.percentageFill, QString("invalid_fill"));
}

QTEST_MAIN(TestJsonImporter)
#include "test_json_importer.moc"

