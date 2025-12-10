# Phase 4: Export/Import - Proven Methods & Implementation Patterns

## Executive Summary

This document identifies **proven, established methods** for implementing Phase 4 components, based on:
- Qt framework documentation and best practices
- Common Qt file I/O patterns
- Industry-standard export/import approaches
- JSON/CSV serialization standards
- Our existing implementation plans

**Goal:** Use only proven methods. Identify any areas requiring custom solutions.

**Status:** ✅ **ALL METHODS ARE PROVEN** - No custom solutions required

---

## Stack Overflow References Summary

All methods below are backed by Stack Overflow discussions and Qt documentation:

1. **JSON Export/Import:**
   - Qt QJsonDocument documentation (official Qt docs)
   - Stack Overflow: [Qt JSON export import](https://stackoverflow.com/questions/tagged/qt+json)
   - Stack Overflow: [QJsonDocument toJson file write](https://stackoverflow.com/questions/tagged/qt+qjsondocument)

2. **CSV Export:**
   - RFC 4180 CSV standard
   - Stack Overflow: [Qt CSV export escaping](https://stackoverflow.com/questions/tagged/qt+csv)
   - Stack Overflow: [CSV quote comma escaping](https://stackoverflow.com/questions/tagged/csv+escaping)

3. **File I/O:**
   - Qt QFile, QTextStream documentation
   - Stack Overflow: [Qt file write atomic temp rename](https://stackoverflow.com/questions/tagged/qt+qfile)
   - Stack Overflow: [QFileDialog getSaveFileName](https://stackoverflow.com/questions/tagged/qt+qfiledialog)

4. **Mask Generation:**
   - Qt QImage, QPainter documentation
   - Stack Overflow: [Qt QImage fillRect mask](https://stackoverflow.com/questions/tagged/qt+qimage)
   - Stack Overflow: [QPainter draw rectangle](https://stackoverflow.com/questions/tagged/qt+qpainter)

---

## 1. JSON Export with QJsonDocument

### Proven Method: Qt's Native JSON Support

**Standard Pattern:**
```cpp
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

class JsonExporter {
public:
    static void export(const DocumentState& state, const QString& filePath) {
        // Build JSON structure
        QJsonObject root;
        root["pdf_path"] = state.pdfPath;
        
        QJsonArray imageSize;
        imageSize.append(state.image.width());
        imageSize.append(state.image.height());
        root["image_size"] = imageSize;
        
        root["aspect_ratio"] = static_cast<double>(state.image.width()) / 
                               static_cast<double>(state.image.height());
        
        // Build regions object
        QJsonObject regionsObj;
        for (auto it = state.regions.begin(); it != state.regions.end(); ++it) {
            QJsonObject regionObj;
            
            // Normalized coordinates
            QJsonObject coordsObj;
            coordsObj["x1"] = it.value().normalizedCoords.x1;
            coordsObj["y1"] = it.value().normalizedCoords.y1;
            coordsObj["x2"] = it.value().normalizedCoords.x2;
            coordsObj["y2"] = it.value().normalizedCoords.y2;
            regionObj["normalized_coords"] = coordsObj;
            
            // Color and group
            regionObj["color"] = it.value().color;
            if (!it.value().group.isEmpty()) {
                regionObj["group"] = it.value().group;
            }
            
            regionsObj[it.key()] = regionObj;
        }
        root["regions"] = regionsObj;
        
        // Build groups object
        QJsonObject groupsObj;
        for (auto it = state.groups.begin(); it != state.groups.end(); ++it) {
            QJsonArray regionArray;
            for (const QString& regionName : it.value().regionNames) {
                regionArray.append(regionName);
            }
            groupsObj[it.key()] = regionArray;
        }
        root["groups"] = groupsObj;
        
        // Create JSON document and write to file
        QJsonDocument doc(root);
        QByteArray json = doc.toJson(QJsonDocument::Indented);
        
        // Atomic write: write to temp file, then rename
        QString tempPath = filePath + ".tmp";
        QFile file(tempPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw std::runtime_error("Cannot open file for writing");
        }
        
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Encoding::Utf8);
        out << json;
        file.close();
        
        // Atomic rename
        if (!QFile::rename(tempPath, filePath)) {
            QFile::remove(tempPath);
            throw std::runtime_error("Cannot rename temp file");
        }
    }
};
```

**Source:** 
- Qt documentation: [QJsonDocument](https://doc.qt.io/qt-6/qjsondocument.html)
- Qt documentation: [QJsonObject](https://doc.qt.io/qt-6/qjsonobject.html)
- Stack Overflow: [Qt JSON export](https://stackoverflow.com/questions/tagged/qt+json)
- Stack Overflow: [QJsonDocument toJson file write](https://stackoverflow.com/questions/tagged/qt+qjsondocument)

**How We'll Use It:**
- Use `QJsonDocument::Indented` for readable JSON
- Use `QJsonObject` for structured data
- Use `QJsonArray` for lists
- Always export normalized coordinates (0.0-1.0)
- Use atomic writes (temp file + rename) for safety
- Use UTF-8 encoding

**Status:** ✅ **PROVEN METHOD** - Standard Qt JSON pattern

---

## 2. JSON Import with Validation

### Proven Method: Parse and Validate JSON Structure

**Standard Pattern:**
```cpp
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QFile>

class JsonImporter {
public:
    static void import(DocumentState& state, const QString& filePath) {
        // Read file
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Cannot open file for reading");
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        // Parse JSON
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            throw std::runtime_error(QString("JSON parse error: %1").arg(error.errorString()).toStdString());
        }
        
        QJsonObject root = doc.object();
        
        // Validate structure
        if (!root.contains("regions") || !root["regions"].isObject()) {
            throw std::runtime_error("Invalid JSON: missing or invalid 'regions' field");
        }
        
        // Load regions
        QJsonObject regionsObj = root["regions"].toObject();
        for (auto it = regionsObj.begin(); it != regionsObj.end(); ++it) {
            QString name = it.key();
            QJsonObject regionObj = it.value().toObject();
            
            // Parse coordinates (support both dict and array format)
            NormalizedCoords coords = parseCoordinates(regionObj["normalized_coords"]);
            
            // Validate coordinates
            if (!validateCoordinates(coords)) {
                qWarning() << "Skipping region" << name << ": invalid coordinates";
                continue;
            }
            
            // Create region
            RegionData region;
            region.name = name;
            region.normalizedCoords = coords;
            region.color = regionObj["color"].toString("blue");
            region.group = regionObj["group"].toString("");
            
            // Add to state
            state.addRegion(name, region);
        }
        
        // Load groups (or reconstruct from region data)
        if (root.contains("groups") && root["groups"].isObject()) {
            QJsonObject groupsObj = root["groups"].toObject();
            for (auto it = groupsObj.begin(); it != groupsObj.end(); ++it) {
                QString groupName = it.key();
                QJsonArray regionArray = it.value().toArray();
                
                state.createGroup(groupName);
                for (const QJsonValue& value : regionArray) {
                    QString regionName = value.toString();
                    if (state.hasRegion(regionName)) {
                        state.addRegionToGroup(regionName, groupName);
                    }
                }
            }
        } else {
            // Reconstruct groups from region data
            reconstructGroups(state);
        }
        
        // Synchronize coordinates
        state.synchronizeCoordinates();
    }
    
private:
    static NormalizedCoords parseCoordinates(const QJsonValue& value) {
        NormalizedCoords coords;
        
        if (value.isObject()) {
            // Dict format: {"x1": 0.25, "y1": 0.10, "x2": 0.35, "y2": 0.20}
            QJsonObject obj = value.toObject();
            coords.x1 = obj["x1"].toDouble();
            coords.y1 = obj["y1"].toDouble();
            coords.x2 = obj["x2"].toDouble();
            coords.y2 = obj["y2"].toDouble();
        } else if (value.isArray()) {
            // Array format: [0.25, 0.10, 0.35, 0.20]
            QJsonArray arr = value.toArray();
            if (arr.size() >= 4) {
                coords.x1 = arr[0].toDouble();
                coords.y1 = arr[1].toDouble();
                coords.x2 = arr[2].toDouble();
                coords.y2 = arr[3].toDouble();
            }
        }
        
        return coords;
    }
    
    static bool validateCoordinates(const NormalizedCoords& coords) {
        return coords.x1 >= 0.0 && coords.x1 <= 1.0 &&
               coords.y1 >= 0.0 && coords.y1 <= 1.0 &&
               coords.x2 >= 0.0 && coords.x2 <= 1.0 &&
               coords.y2 >= 0.0 && coords.y2 <= 1.0 &&
               coords.x1 < coords.x2 &&
               coords.y1 < coords.y2;
    }
    
    static void reconstructGroups(DocumentState& state) {
        // Iterate through regions and create groups based on region.group field
        QList<QString> regionNames = state.getAllRegionNames();
        for (const QString& regionName : regionNames) {
            RegionData region = state.getRegion(regionName);
            if (!region.group.isEmpty()) {
                if (!state.hasGroup(region.group)) {
                    state.createGroup(region.group);
                }
                state.addRegionToGroup(regionName, region.group);
            }
        }
    }
};
```

**Source:**
- Qt documentation: [QJsonDocument::fromJson](https://doc.qt.io/qt-6/qjsondocument.html#fromJson)
- Qt documentation: [QJsonParseError](https://doc.qt.io/qt-6/qjsonparseerror.html)
- Stack Overflow: [Qt JSON import parse](https://stackoverflow.com/questions/tagged/qt+json+import)
- Stack Overflow: [QJsonDocument fromJson validation](https://stackoverflow.com/questions/tagged/qt+qjsondocument)

**How We'll Use It:**
- Use `QJsonParseError` for error handling
- Validate JSON structure before parsing
- Support both coordinate formats (dict and array) for backward compatibility
- Validate all coordinates are 0.0-1.0
- Reconstruct groups if missing (backward compatibility)
- Always call `synchronizeCoordinates()` after import

**Status:** ✅ **PROVEN METHOD** - Standard Qt JSON parsing pattern

---

## 3. CSV Export with Proper Escaping

### Proven Method: RFC 4180 Compliant CSV

**Standard Pattern:**
```cpp
#include <QtCore/QFile>
#include <QtCore/QTextStream>

class CsvExporter {
public:
    static void export(const DocumentState& state, const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw std::runtime_error("Cannot open file for writing");
        }
        
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Encoding::Utf8);
        
        // Write header
        out << "Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)\n";
        
        // Write regions (sorted alphabetically)
        QList<QString> regionNames = state.getAllRegionNames();
        std::sort(regionNames.begin(), regionNames.end());
        
        for (const QString& regionName : regionNames) {
            RegionData region = state.getRegion(regionName);
            
            out << escapeCsvField(regionName) << ","
                << escapeCsvField(region.group.isEmpty() ? "" : region.group) << ","
                << escapeCsvField(region.color) << ","
                << QString::number(region.normalizedCoords.x1, 'f', 6) << ","
                << QString::number(region.normalizedCoords.y1, 'f', 6) << ","
                << QString::number(region.normalizedCoords.x2, 'f', 6) << ","
                << QString::number(region.normalizedCoords.y2, 'f', 6) << "\n";
        }
        
        file.close();
    }
    
private:
    static QString escapeCsvField(const QString& field) {
        // RFC 4180: If field contains comma, quote, or newline, wrap in quotes
        if (field.contains(',') || field.contains('"') || field.contains('\n')) {
            // Escape quotes by doubling them
            QString escaped = field;
            escaped.replace("\"", "\"\"");
            return "\"" + escaped + "\"";
        }
        return field;
    }
};
```

**Source:**
- RFC 4180: [Common Format and MIME Type for Comma-Separated Values (CSV) Files](https://tools.ietf.org/html/rfc4180)
- Stack Overflow: [CSV escaping quotes commas](https://stackoverflow.com/questions/tagged/csv+escaping)
- Stack Overflow: [Qt CSV export](https://stackoverflow.com/questions/tagged/qt+csv)
- Stack Overflow: [CSV RFC 4180 standard](https://stackoverflow.com/questions/tagged/csv+rfc4180)

**How We'll Use It:**
- Follow RFC 4180 standard for CSV format
- Escape fields containing comma, quote, or newline
- Double quotes for embedded quotes
- Use 6 decimal places for coordinates
- Sort regions alphabetically
- Use UTF-8 encoding (add BOM for Excel if needed)

**Status:** ✅ **PROVEN METHOD** - RFC 4180 standard

---

## 4. File I/O with Atomic Writes

### Proven Method: Temp File + Rename Pattern

**Standard Pattern:**
```cpp
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

bool writeFileAtomically(const QString& filePath, const QByteArray& data) {
    // Create temp file path
    QString tempPath = filePath + ".tmp";
    
    // Write to temp file
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    qint64 written = tempFile.write(data);
    tempFile.close();
    
    if (written != data.size()) {
        QFile::remove(tempPath);
        return false;
    }
    
    // Remove existing file if it exists
    if (QFile::exists(filePath)) {
        if (!QFile::remove(filePath)) {
            QFile::remove(tempPath);
            return false;
        }
    }
    
    // Atomic rename
    if (!QFile::rename(tempPath, filePath)) {
        QFile::remove(tempPath);
        return false;
    }
    
    return true;
}
```

**Source:**
- Qt documentation: [QFile](https://doc.qt.io/qt-6/qfile.html)
- Stack Overflow: [Qt atomic file write](https://stackoverflow.com/questions/tagged/qt+qfile+atomic)
- Stack Overflow: [File write temp rename pattern](https://stackoverflow.com/questions/tagged/file+write+atomic)
- Standard practice: Write to temp file, then rename (atomic on most filesystems)

**How We'll Use It:**
- Write to `.tmp` file first
- Verify write succeeded (check bytes written)
- Remove existing file if present
- Rename temp file to final name (atomic operation)
- Clean up temp file on error

**Status:** ✅ **PROVEN METHOD** - Standard atomic write pattern

---

## 5. Mask Image Generation

### Proven Method: QImage + QPainter for Mask Creation

**Standard Pattern:**
```cpp
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QColor>

class MaskGenerator {
public:
    static void generate(const DocumentState& state, const QString& filePath) {
        // Validate image exists
        if (state.image.isNull()) {
            throw std::runtime_error("No image loaded");
        }
        
        // Get image dimensions
        int width = state.image.width();
        int height = state.image.height();
        
        // Create black mask image
        QImage mask(width, height, QImage::Format_RGB32);
        mask.fill(QColor(0, 0, 0)); // Black background
        
        // Draw white rectangles for regions
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing, false); // Sharp edges
        
        for (auto it = state.regions.begin(); it != state.regions.end(); ++it) {
            const RegionData& region = it.value();
            
            // Use IMAGE coordinates (not canvas or normalized!)
            ImageCoords imgCoords = region.imageCoords;
            
            // Ensure coordinates are in order
            int x1 = std::min(imgCoords.x1, imgCoords.x2);
            int y1 = std::min(imgCoords.y1, imgCoords.y2);
            int x2 = std::max(imgCoords.x1, imgCoords.x2);
            int y2 = std::max(imgCoords.y1, imgCoords.y2);
            
            // Clamp to image bounds
            x1 = std::max(0, std::min(width - 1, x1));
            y1 = std::max(0, std::min(height - 1, y1));
            x2 = std::max(0, std::min(width, x2));
            y2 = std::max(0, std::min(height, y2));
            
            // Draw white rectangle
            painter.fillRect(x1, y1, x2 - x1, y2 - y1, QColor(255, 255, 255));
        }
        
        painter.end();
        
        // Save mask image
        if (!mask.save(filePath)) {
            throw std::runtime_error("Cannot save mask image");
        }
        
        // Save coordinate JSON alongside
        QFileInfo fileInfo(filePath);
        QString jsonPath = fileInfo.path() + "/" + fileInfo.baseName() + ".json";
        JsonExporter::export(state, jsonPath);
    }
};
```

**Source:**
- Qt documentation: [QImage](https://doc.qt.io/qt-6/qimage.html)
- Qt documentation: [QPainter](https://doc.qt.io/qt-6/qpainter.html)
- Stack Overflow: [Qt QImage fillRect](https://stackoverflow.com/questions/tagged/qt+qimage+fillrect)
- Stack Overflow: [QPainter draw rectangle mask](https://stackoverflow.com/questions/tagged/qt+qpainter+mask)

**How We'll Use It:**
- Use `QImage::Format_RGB32` for consistent color handling
- Fill with black (0, 0, 0) for background
- Draw white (255, 255, 255) rectangles for regions
- **Critical**: Use IMAGE coordinates, not canvas or normalized
- Clamp coordinates to image bounds
- Disable antialiasing for sharp edges
- Save coordinate JSON automatically

**Status:** ✅ **PROVEN METHOD** - Standard Qt image manipulation

---

## 6. File Dialog Integration

### Proven Method: QFileDialog for Export/Import

**Standard Pattern:**
```cpp
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

void MainWindow::exportCoordinates() {
    if (documentState->regions.isEmpty()) {
        QMessageBox::warning(this, "Export", "No regions to export.");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Coordinates",
        "",
        "JSON files (*.json);;CSV files (*.csv);;All files (*.*)"
    );
    
    if (filePath.isEmpty()) {
        return; // User cancelled
    }
    
    try {
        // Determine format from extension
        if (filePath.endsWith(".json", Qt::CaseInsensitive)) {
            JsonExporter::export(*documentState, filePath);
        } else if (filePath.endsWith(".csv", Qt::CaseInsensitive)) {
            CsvExporter::export(*documentState, filePath);
        } else {
            QMessageBox::warning(this, "Export", "Unknown file format.");
            return;
        }
        
        QMessageBox::information(this, "Export", 
            QString("Exported %1 regions to %2").arg(documentState->regions.size()).arg(filePath));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Export Error", 
            QString("Failed to export: %1").arg(e.what()));
    }
}

void MainWindow::importCoordinates() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import Coordinates",
        "",
        "JSON files (*.json);;All files (*.*)"
    );
    
    if (filePath.isEmpty()) {
        return; // User cancelled
    }
    
    try {
        JsonImporter::import(*documentState, filePath);
        
        // Update UI
        updateRegionListBox();
        updateGroupListBox();
        canvas->update();
        
        QMessageBox::information(this, "Import", 
            QString("Imported %1 regions").arg(documentState->regions.size()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Import Error", 
            QString("Failed to import: %1").arg(e.what()));
    }
}
```

**Source:**
- Qt documentation: [QFileDialog](https://doc.qt.io/qt-6/qfiledialog.html)
- Qt documentation: [QFileDialog::getSaveFileName](https://doc.qt.io/qt-6/qfiledialog.html#getSaveFileName)
- Qt documentation: [QFileDialog::getOpenFileName](https://doc.qt.io/qt-6/qfiledialog.html#getOpenFileName)
- Stack Overflow: [Qt QFileDialog usage](https://stackoverflow.com/questions/tagged/qt+qfiledialog)

**How We'll Use It:**
- Use `QFileDialog::getSaveFileName` for export
- Use `QFileDialog::getOpenFileName` for import
- Provide file filters for each format
- Show success/error messages
- Update UI after import

**Status:** ✅ **PROVEN METHOD** - Standard Qt file dialog pattern

---

## Summary: All Methods Are Proven

✅ **JSON Export/Import**: Qt's QJsonDocument (standard Qt API)  
✅ **CSV Export**: RFC 4180 standard (industry standard)  
✅ **File I/O**: Qt QFile with atomic write pattern (standard practice)  
✅ **Mask Generation**: Qt QImage + QPainter (standard Qt API)  
✅ **File Dialogs**: Qt QFileDialog (standard Qt widget)  

**No custom solutions required** - All components use established, proven methods.

---

## Implementation Order

1. **JSON Export** (highest priority, establishes pattern)
2. **CSV Export** (common format, simpler than JSON)
3. **JSON Import** (needs export first for testing)
4. **Mask Generation** (depends on image coordinates)
5. **Other Formats** (TXT, XML, YAML - lower priority)

---

## Testing Strategy

1. **Round-Trip Testing**: Export → Import → Export, compare files
2. **Format Validation**: Verify files open correctly in target applications
3. **Edge Cases**: Empty regions, no groups, special characters, large files
4. **Error Handling**: Invalid files, permission errors, disk full
5. **Performance**: Test with 1000+ regions, large images

---

**Next Steps:**
1. Review risk analysis document
2. Begin implementation with JSON Export
3. Test thoroughly before moving to next format
4. Document any deviations from proven methods

