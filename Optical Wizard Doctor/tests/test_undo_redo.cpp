// Test file for Undo/Redo functionality
// Tests state snapshot, undo/redo operations

#include <QtTest/QtTest>
#include "../src/models/DocumentState.h"
#include "../src/models/RegionData.h"
#include "../src/models/GroupData.h"
#include "../src/core/CoordinateSystem.h"

using namespace ocr_orc;

class TestUndoRedo : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testStateSnapshot();
    void testSaveState();
    void testUndoRedo();
    void testStackLimit();
    void testStateRestoration();
};

void TestUndoRedo::initTestCase() {
}

void TestUndoRedo::cleanupTestCase() {
}

void TestUndoRedo::testStateSnapshot() {
    StateSnapshot snapshot1;
    QVERIFY(snapshot1.isEmpty());
    
    StateSnapshot snapshot2;
    snapshot2.pdfPath = "test.pdf";
    snapshot2.imageSize = QSize(100, 200);
    QVERIFY(!snapshot2.isEmpty());
    
    QVERIFY(snapshot1 != snapshot2);
}

void TestUndoRedo::testSaveState() {
    DocumentState state;
    
    // Initially, no undo available
    QVERIFY(!state.canUndo());
    QCOMPARE(state.undoCount(), 0);
    
    // Add a region
    RegionData region;
    region.normalizedCoords = NormalizedCoords(0.1, 0.1, 0.5, 0.5);
    region.color = "blue";
    state.addRegion("Test Region", region);
    
    // Save state
    state.saveState();
    
    // Now undo should be available
    QVERIFY(state.canUndo());
    QCOMPARE(state.undoCount(), 1);
    QVERIFY(!state.canRedo());
}

void TestUndoRedo::testUndoRedo() {
    DocumentState state;
    
    // Create initial state with one region
    RegionData region1;
    region1.normalizedCoords = NormalizedCoords(0.1, 0.1, 0.3, 0.3);
    region1.color = "blue";
    state.addRegion("Region 1", region1);
    
    // Save state
    state.saveState();
    QVERIFY(state.canUndo());
    QCOMPARE(state.undoCount(), 1);
    
    // Add another region
    RegionData region2;
    region2.normalizedCoords = NormalizedCoords(0.5, 0.5, 0.7, 0.7);
    region2.color = "red";
    state.addRegion("Region 2", region2);
    
    // Verify both regions exist
    QVERIFY(state.hasRegion("Region 1"));
    QVERIFY(state.hasRegion("Region 2"));
    
    // Undo - should remove Region 2
    state.undoAction();
    QVERIFY(state.hasRegion("Region 1"));
    QVERIFY(!state.hasRegion("Region 2"));
    QVERIFY(state.canRedo());
    QCOMPARE(state.redoCount(), 1);
    
    // Redo - should restore Region 2
    state.redoAction();
    QVERIFY(state.hasRegion("Region 1"));
    QVERIFY(state.hasRegion("Region 2"));
    QVERIFY(!state.canRedo());
    QVERIFY(state.canUndo());
}

void TestUndoRedo::testStackLimit() {
    DocumentState state;
    
    // Add region and save state multiple times
    RegionData region;
    region.normalizedCoords = NormalizedCoords(0.1, 0.1, 0.2, 0.2);
    region.color = "blue";
    
    // Save more than MAX_UNDO_LEVELS states
    for (int i = 0; i < 60; i++) {
        state.addRegion(QString("Region %1").arg(i), region);
        state.saveState();
    }
    
    // Stack should be limited to MAX_UNDO_LEVELS
    QCOMPARE(state.undoCount(), 50);  // MAX_UNDO_LEVELS
}

void TestUndoRedo::testStateRestoration() {
    DocumentState state;
    
    // Create state with regions and groups
    RegionData region1;
    region1.normalizedCoords = NormalizedCoords(0.1, 0.1, 0.3, 0.3);
    region1.color = "blue";
    state.addRegion("Region 1", region1);
    
    RegionData region2;
    region2.normalizedCoords = NormalizedCoords(0.5, 0.5, 0.7, 0.7);
    region2.color = "red";
    state.addRegion("Region 2", region2);
    
    state.createGroup("Group 1");
    state.addRegionToGroup("Region 1", "Group 1");
    
    // Save state
    state.saveState();
    
    // Modify state
    state.removeRegion("Region 2");
    state.deleteGroup("Group 1");
    
    // Verify modifications
    QVERIFY(!state.hasRegion("Region 2"));
    QVERIFY(!state.hasGroup("Group 1"));
    
    // Undo - should restore everything
    state.undoAction();
    
    QVERIFY(state.hasRegion("Region 1"));
    QVERIFY(state.hasRegion("Region 2"));
    QVERIFY(state.hasGroup("Group 1"));
    
    RegionData restoredRegion1 = state.getRegion("Region 1");
    QCOMPARE(restoredRegion1.group, QString("Group 1"));
}

QTEST_MAIN(TestUndoRedo)
#include "test_undo_redo.moc"
