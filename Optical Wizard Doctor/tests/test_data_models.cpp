// Comprehensive test suite for Data Models
// Tests RegionData, GroupData, and DocumentState

#include "../src/models/RegionData.h"
#include "../src/models/GroupData.h"
#include "../src/models/DocumentState.h"
#include <QtCore/QPointF>
#include <QtCore/QSize>
#include <QtGui/QImage>
#include <iostream>
#include <cassert>

using namespace ocr_orc;

const double EPSILON = 0.0001;

void testRegionData() {
    std::cout << "\n=== Testing RegionData ===\n";
    
    // Test 1: Default constructor
    std::cout << "  Testing default constructor...\n";
    RegionData region;
    assert(region.name.isEmpty());
    assert(region.color == "blue");  // Default from constructor
    assert(region.group.isEmpty());
    assert(region.regionType == "none");  // Default for regionType
    assert(region.percentageFill == "none");  // Default for percentageFill
    std::cout << "    ✓ Default constructor works\n";
    
    // Test 2: Constructor with parameters
    std::cout << "  Testing constructor with parameters...\n";
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    RegionData region2("Test Region", norm, "red", "testgroup");
    assert(region2.name == "Test Region");
    assert(region2.color == "red");
    assert(region2.group == "testgroup");
    assert(region2.regionType == "none");  // Default when not specified
    assert(region2.percentageFill == "none");  // Default when not specified
    std::cout << "    ✓ Constructor with parameters works\n";
    
    // Test 2b: Constructor with new fields
    std::cout << "  Testing constructor with regionType and percentageFill...\n";
    RegionData region2b("Test Region 2", norm, "blue", "", "rect", "text", "standard");
    assert(region2b.regionType == "text");
    assert(region2b.percentageFill == "standard");
    std::cout << "    ✓ Constructor with new fields works\n";
    
    // Test 3: Validation
    std::cout << "  Testing validation...\n";
    RegionData valid("Valid", NormalizedCoords(0.1, 0.1, 0.2, 0.2), "blue");
    assert(valid.isValid());
    
    RegionData invalidName("", NormalizedCoords(0.1, 0.1, 0.2, 0.2));
    assert(!invalidName.isValid());
    
    RegionData invalidColor("Test", NormalizedCoords(0.1, 0.1, 0.2, 0.2), "invalid");
    assert(!invalidColor.isValid());
    std::cout << "    ✓ Validation works\n";
    
    // Test 4: syncFromNormalized
    std::cout << "  Testing syncFromNormalized...\n";
    RegionData region3("Test", NormalizedCoords(0.25, 0.10, 0.35, 0.20));
    QPointF offset(100.0, 50.0);
    region3.syncFromNormalized(2000, 3000, 0.4, offset);
    
    assert(region3.imageCoords.x1 == 500);
    assert(region3.imageCoords.y1 == 300);
    assert(std::abs(region3.canvasCoords.x1 - 300.0) < EPSILON);
    std::cout << "    ✓ syncFromNormalized works\n";
    
    // Test 5: syncFromCanvas
    std::cout << "  Testing syncFromCanvas...\n";
    RegionData region4;
    region4.name = "Test";
    CanvasCoords canvas(300.0, 170.0, 380.0, 290.0);
    region4.syncFromCanvas(canvas, 2000, 3000, 0.4, offset);
    
    // Should have normalized coords calculated
    assert(region4.normalizedCoords.x1 >= 0.0 && region4.normalizedCoords.x1 <= 1.0);
    assert(region4.normalizedCoords.y1 >= 0.0 && region4.normalizedCoords.y1 <= 1.0);
    std::cout << "    ✓ syncFromCanvas works\n";
    
    // Test 6: Qt conversions
    std::cout << "  Testing Qt conversions...\n";
    region3.syncFromNormalized(2000, 3000, 0.4, offset);
    QRectF qrect = region3.toQRectF();
    assert(std::abs(qrect.x() - region3.canvasCoords.x1) < EPSILON);
    assert(std::abs(qrect.y() - region3.canvasCoords.y1) < EPSILON);
    
    QRect qrectInt = region3.toQRect();
    assert(qrectInt.x() == region3.imageCoords.x1);
    assert(qrectInt.y() == region3.imageCoords.y1);
    std::cout << "    ✓ Qt conversions work\n";
}

void testGroupData() {
    std::cout << "\n=== Testing GroupData ===\n";
    
    // Test 1: Default constructor
    std::cout << "  Testing default constructor...\n";
    GroupData group;
    assert(group.name.isEmpty());
    assert(group.count() == 0);
    assert(group.isEmpty());
    std::cout << "    ✓ Default constructor works\n";
    
    // Test 2: Constructor with name
    std::cout << "  Testing constructor with name...\n";
    GroupData group2("testgroup");
    assert(group2.name == "testgroup");
    assert(group2.isEmpty());
    std::cout << "    ✓ Constructor with name works\n";
    
    // Test 3: Add/remove regions
    std::cout << "  Testing add/remove regions...\n";
    group2.addRegion("Region1");
    group2.addRegion("Region2");
    assert(group2.count() == 2);
    assert(group2.contains("Region1"));
    assert(group2.contains("Region2"));
    
    group2.removeRegion("Region1");
    assert(group2.count() == 1);
    assert(!group2.contains("Region1"));
    assert(group2.contains("Region2"));
    std::cout << "    ✓ Add/remove regions works\n";
    
    // Test 4: Duplicate prevention
    std::cout << "  Testing duplicate prevention...\n";
    group2.addRegion("Region2");  // Already exists
    assert(group2.count() == 1);  // Should not add duplicate
    std::cout << "    ✓ Duplicate prevention works\n";
}

void testDocumentState() {
    std::cout << "\n=== Testing DocumentState ===\n";
    
    DocumentState state;
    
    // Test 1: Initial state
    std::cout << "  Testing initial state...\n";
    assert(state.regions.isEmpty());
    assert(state.groups.isEmpty());
    assert(state.zoomLevel == 1.0);
    assert(state.pdfPath.isEmpty());
    std::cout << "    ✓ Initial state correct\n";
    
    // Test 2: Add region
    std::cout << "  Testing add region...\n";
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    RegionData region("Test Region", norm, "blue");
    state.addRegion("Test Region", region);
    
    assert(state.hasRegion("Test Region"));
    assert(state.getAllRegionNames().size() == 1);
    std::cout << "    ✓ Add region works\n";
    
    // Test 3: Get region
    std::cout << "  Testing get region...\n";
    RegionData retrieved = state.getRegion("Test Region");
    assert(retrieved.name == "Test Region");
    assert(retrieved.color == "blue");
    std::cout << "    ✓ Get region works\n";
    
    // Test 4: Remove region
    std::cout << "  Testing remove region...\n";
    state.removeRegion("Test Region");
    assert(!state.hasRegion("Test Region"));
    assert(state.regions.isEmpty());
    std::cout << "    ✓ Remove region works\n";
    
    // Test 5: Group management
    std::cout << "  Testing group management...\n";
    state.addRegion("Region1", RegionData("Region1", norm, "blue"));
    state.addRegion("Region2", RegionData("Region2", norm, "red"));
    
    state.createGroup("testgroup");
    assert(state.hasGroup("testgroup"));
    
    state.addRegionToGroup("Region1", "testgroup");
    state.addRegionToGroup("Region2", "testgroup");
    
    GroupData group = state.getGroup("testgroup");
    assert(group.count() == 2);
    assert(group.contains("Region1"));
    assert(group.contains("Region2"));
    
    // Check region group assignment
    assert(state.getRegion("Region1").group == "testgroup");
    std::cout << "    ✓ Group management works\n";
    
    // Test 6: Remove from group
    std::cout << "  Testing remove from group...\n";
    state.removeRegionFromGroup("Region1");
    assert(state.getRegion("Region1").group.isEmpty());
    assert(state.getGroup("testgroup").count() == 1);
    std::cout << "    ✓ Remove from group works\n";
    
    // Test 7: Delete group
    std::cout << "  Testing delete group...\n";
    state.deleteGroup("testgroup");
    assert(!state.hasGroup("testgroup"));
    assert(state.getRegion("Region2").group.isEmpty());
    std::cout << "    ✓ Delete group works\n";
    
    // Test 8: Coordinate synchronization
    std::cout << "  Testing coordinate synchronization...\n";
    QImage testImage(2000, 3000, QImage::Format_RGB32);
    state.setImage(testImage);
    state.imageOffset = QPointF(100.0, 50.0);
    state.scaleFactor = 0.4;
    
    state.addRegion("SyncTest", RegionData("SyncTest", norm, "blue"));
    state.synchronizeCoordinates();
    
    RegionData synced = state.getRegion("SyncTest");
    assert(synced.imageCoords.x1 == 500);
    assert(std::abs(synced.canvasCoords.x1 - 300.0) < EPSILON);
    std::cout << "    ✓ Coordinate synchronization works\n";
    
    // Test 9: Validation
    std::cout << "  Testing validation...\n";
    assert(state.isValid());
    
    // Add invalid region
    RegionData invalid("", NormalizedCoords(0.1, 0.1, 0.2, 0.2));
    state.addRegion("", invalid);  // Should be rejected (empty name)
    assert(state.isValid());  // Should still be valid (invalid region not added)
    std::cout << "    ✓ Validation works\n";
}

void testJsonSerialization() {
    std::cout << "\n=== Testing JSON Serialization ===\n";
    
    DocumentState state;
    state.pdfPath = "/test/path.pdf";
    
    // Create test image
    QImage testImage(2000, 3000, QImage::Format_RGB32);
    state.setImage(testImage);
    
    // Add regions
    NormalizedCoords norm1(0.25, 0.10, 0.35, 0.20);
    NormalizedCoords norm2(0.40, 0.10, 0.50, 0.20);
    
    state.addRegion("Region1", RegionData("Region1", norm1, "blue", "group1", "rect", "text", "standard"));
    state.addRegion("Region2", RegionData("Region2", norm2, "red", "group1"));
    
    // Test toJson
    std::cout << "  Testing toJson...\n";
    QJsonObject json = state.toJson();
    assert(json["pdf_path"].toString() == "/test/path.pdf");
    assert(json.contains("regions"));
    assert(json.contains("groups"));
    std::cout << "    ✓ toJson works\n";
    
    // Test fromJson
    std::cout << "  Testing fromJson...\n";
    DocumentState loaded = DocumentState::fromJson(json);
    assert(loaded.pdfPath == "/test/path.pdf");
    assert(loaded.hasRegion("Region1"));
    assert(loaded.hasRegion("Region2"));
    assert(loaded.hasGroup("group1"));
    
    RegionData loadedRegion = loaded.getRegion("Region1");
    assert(std::abs(loadedRegion.normalizedCoords.x1 - 0.25) < EPSILON);
    assert(loadedRegion.color == "blue");
    assert(loadedRegion.group == "group1");
    std::cout << "    ✓ fromJson works\n";
    
    // Test round-trip
    std::cout << "  Testing round-trip serialization...\n";
    QJsonObject json2 = loaded.toJson();
    DocumentState loaded2 = DocumentState::fromJson(json2);
    assert(loaded2.hasRegion("Region1"));
    assert(loaded2.hasRegion("Region2"));
    assert(loaded2.hasGroup("group1"));
    std::cout << "    ✓ Round-trip serialization works\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Data Models Comprehensive Test Suite\n";
    std::cout << "========================================\n";
    
    try {
        testRegionData();
        testGroupData();
        testDocumentState();
        testJsonSerialization();
        
        std::cout << "\n========================================\n";
        std::cout << "✓ ALL TESTS PASSED!\n";
        std::cout << "========================================\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ TEST FAILED: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n✗ UNKNOWN TEST FAILURE\n";
        return 1;
    }
}

