# Export - Implementation Steps

## Objective

Implement export functionality for all coordinate formats.

## Prerequisites

- DocumentState with regions and groups
- Qt JSON/XML support
- File dialogs working

## Sub-Projects

1. **JSON Export** - Primary format (structured data)
2. **CSV Export** - Spreadsheet-friendly format
3. **Other Formats** - TXT, XML, YAML exports

## Order of Implementation

1. JSON Export (foundation, most important)
2. CSV Export (simple, builds on JSON structure)
3. Other Formats (TXT, XML, YAML)

## Steps

### Step 1: Create Export Base Class (Optional)

1. Create abstract base class `Exporter`
2. Define interface: `virtual void export(const DocumentState& state, const QString& filePath) = 0`
3. Or use standalone functions (simpler)

### Step 2: Implement JSON Export

1. Use Qt's QJsonDocument
2. Create JSON structure with regions and groups
3. Use normalized coordinates only
4. Include metadata (PDF path, image size, aspect ratio)
5. Write to file

### Step 3: Implement CSV Export

1. Create CSV with header row
2. One row per region
3. Include: name, group, color, x1, y1, x2, y2 (all as percentages)
4. Sort alphabetically
5. Write to file

### Step 4: Implement Other Formats

1. TXT: Human-readable format
2. XML: Structured XML format
3. YAML: YAML format (requires yaml-cpp)

### Step 5: Add Export UI

1. "Export Coordinates" button
2. Show file dialog with format options
3. Determine format from file extension
4. Call appropriate exporter
5. Show success message

## Acceptance Criteria

- [ ] All export formats work
- [ ] Exports use normalized coordinates only
- [ ] Exports include all metadata
- [ ] File dialogs work correctly
- [ ] Success messages shown
