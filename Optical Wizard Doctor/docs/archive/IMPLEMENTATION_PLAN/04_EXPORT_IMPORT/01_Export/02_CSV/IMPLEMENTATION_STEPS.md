# CSV Export - Implementation Steps

## Objective

Export coordinates in CSV format for spreadsheet applications.

## Prerequisites

- DocumentState with regions
- JSON export working (for reference)
- File I/O working

## CSV Format

```csv
Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)
Last Name Cell 1,lastname,blue,0.250000,0.100000,0.350000,0.200000
Last Name Cell 2,lastname,blue,0.350000,0.100000,0.450000,0.200000
```

## Steps

### Step 1: Create CsvExporter Class

1. Create `src/export/CsvExporter.h` and `.cpp`
2. Create class `CsvExporter`
3. Add static method: `static void export(const DocumentState& state, const QString& filePath)`

### Step 2: Define CSV Columns

1. Header row:
   - Region Name
   - Group
   - Color
   - X1 (%)
   - Y1 (%)
   - X2 (%)
   - Y2 (%)
2. All coordinates as percentages (0.0-1.0)

### Step 3: Write Header Row

1. Create header string:
   ```cpp
   QString header = "Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)\n";
   ```
2. Write to file

### Step 4: Write Region Rows

1. Sort regions alphabetically by name
2. For each region:
   ```cpp
   QString row = QString("%1,%2,%3,%4,%5,%6,%7\n")
       .arg(region.name)
       .arg(region.group.isEmpty() ? "" : region.group)
       .arg(region.color)
       .arg(region.normalizedCoords.x1, 0, 'f', 6)
       .arg(region.normalizedCoords.y1, 0, 'f', 6)
       .arg(region.normalizedCoords.x2, 0, 'f', 6)
       .arg(region.normalizedCoords.y2, 0, 'f', 6);
   ```
3. Use 6 decimal places for coordinates
4. Handle empty group (empty string, not "None")

### Step 5: Handle CSV Escaping

1. If region name contains comma:
   - Wrap in quotes: `"Name, with comma"`
2. If region name contains quotes:
   - Escape quotes: `"Name with ""quotes"""`
3. Or use Qt's CSV handling if available

### Step 6: Write to File

1. Open file for writing (text mode)
2. Write header
3. Write all region rows
4. Close file
5. Handle errors

### Step 7: Add CSV to Export Dialog

1. Update file dialog to include CSV:
   ```cpp
   "CSV files (*.csv);;JSON files (*.json);;All files (*.*)"
   ```
2. Detect format from extension
3. Call CsvExporter::export() for .csv files

### Step 8: Test CSV Export

1. Create test regions
2. Export to CSV
3. Open in Excel/Google Sheets
4. Verify columns are correct
5. Verify coordinates are percentages
6. Verify sorting is alphabetical
7. Test with special characters in names

## Acceptance Criteria

- [ ] CSV file is created correctly
- [ ] Header row is correct
- [ ] All regions are included
- [ ] Coordinates are percentages (6 decimal places)
- [ ] File opens correctly in spreadsheets
- [ ] Special characters are handled

## Code Structure

```cpp
class CsvExporter {
public:
    static void export(const DocumentState& state, const QString& filePath);
    
private:
    static QString escapeCsvField(const QString& field);
    static QString formatCoordinate(double value);
};
```

## Notes

- Use 6 decimal places for coordinates
- Sort regions alphabetically
- Handle CSV escaping for special characters
- Test with spreadsheet applications

