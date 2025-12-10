# Data Models - Implementation Steps

## Objective

Implement the data structures for Region, Group, and Document state.

## Prerequisites

- Coordinate system implemented
- Basic understanding of Qt data structures

## Data Structures

1. **RegionData** - Stores region information (coordinates, color, group)
2. **GroupData** - Stores group information (name, region list)
3. **DocumentState** - Stores document information (PDF path, image, regions, groups)

## Steps

### Step 1: Create RegionData Structure

1. Create `src/models/RegionData.h`
2. Define `struct RegionData`:
   ```cpp
   struct RegionData {
       NormalizedCoords normalizedCoords;
       ImageCoords imageCoords;
       CanvasCoords canvasCoords;
       QString name;
       QString color;  // "blue", "red", etc.
       QString group;  // Group name or empty
   };
   ```
3. Add default constructor
4. Add equality operator (for testing)

### Step 2: Add RegionData Methods

1. `bool isValid() const` - Validate all coordinates
2. `void updateCanvasCoords(const CoordinateSystem& cs)` - Recalculate canvas coords
3. `void updateImageCoords(int imgWidth, int imgHeight)` - Recalculate image coords
4. `QRect toQRect() const` - Convert to Qt rectangle (for rendering)

### Step 3: Create GroupData Structure

1. Create `src/models/GroupData.h`
2. Define `struct GroupData`:
   ```cpp
   struct GroupData {
       QString name;
       QList<QString> regionNames;  // List of region names in this group
   };
   ```
3. Add methods:
   - `void addRegion(const QString& name)`
   - `void removeRegion(const QString& name)`
   - `bool contains(const QString& name) const`
   - `int count() const`

### Step 4: Create DocumentState Class

1. Create `src/models/DocumentState.h`
2. Define `class DocumentState`:
   ```cpp
   class DocumentState {
   public:
       QString pdfPath;
       QImage image;  // First page as image
       QMap<QString, RegionData> regions;  // Key: region name
       QMap<QString, GroupData> groups;    // Key: group name
       
       // State management
       double zoomLevel;
       double scaleFactor;
       QPointF imageOffset;
   };
   ```
3. Add methods for state management

### Step 5: Implement Region Management

1. Add to DocumentState:
   - `void addRegion(const QString& name, const RegionData& region)`
   - `void removeRegion(const QString& name)`
   - `bool hasRegion(const QString& name) const`
   - `RegionData getRegion(const QString& name) const`
   - `QList<QString> getAllRegionNames() const`

### Step 6: Implement Group Management

1. Add to DocumentState:
   - `void createGroup(const QString& name)`
   - `void deleteGroup(const QString& name)`
   - `void addRegionToGroup(const QString& regionName, const QString& groupName)`
   - `void removeRegionFromGroup(const QString& regionName)`
   - `QList<QString> getGroupNames() const`
   - `GroupData getGroup(const QString& name) const`

### Step 7: Implement Coordinate Synchronization

1. Add method: `void synchronizeCoordinates(const CoordinateSystem& cs)`
2. For each region:
   - Recalculate image coords from normalized
   - Recalculate canvas coords from normalized
3. Call this whenever zoom/pan changes

### Step 8: Add State Validation

1. `bool isValid() const` - Validate all regions and groups
2. Check for duplicate region names
3. Check for orphaned groups (groups with no regions)
4. Check coordinate validity

### Step 9: Add Serialization Support

1. Add methods for JSON export:
   - `QJsonObject toJson() const`
   - `static DocumentState fromJson(const QJsonObject& json)`
2. Handle missing fields gracefully
3. Validate on deserialization

### Step 10: Write Unit Tests

1. Create `tests/test_data_models.cpp`
2. Test region creation and retrieval
3. Test group management
4. Test coordinate synchronization
5. Test serialization/deserialization
6. Test validation

### Step 11: Integrate with Application

1. Create DocumentState instance in main application
2. Test basic operations
3. Verify memory management (use smart pointers if needed)

## Acceptance Criteria

- [ ] All data structures are defined
- [ ] Region management works correctly
- [ ] Group management works correctly
- [ ] Coordinate synchronization works
- [ ] Serialization works (JSON)
- [ ] Unit tests pass
- [ ] No memory leaks

## Design Decisions

- Use `QMap` for regions and groups (fast lookup by name)
- Use `QList` for region names in groups (preserves order)
- Store all three coordinate systems (performance vs memory trade-off)
- Use Qt types (QString, QImage) for Qt integration

## Notes

- Consider using smart pointers for DocumentState if shared ownership needed
- DocumentState should be copyable for undo/redo (deep copy)
- Keep data structures simple; add complexity only when needed

