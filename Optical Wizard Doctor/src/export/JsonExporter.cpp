#include "JsonExporter.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QStringConverter>
#include <stdexcept>

namespace ocr_orc {

void JsonExporter::exportToFile(const DocumentState& state, const QString& filePath) {
    // Build JSON structure
    QJsonObject root = buildRootObject(state);
    
    // Create JSON document
    QJsonDocument doc(root);
    QByteArray json = doc.toJson(QJsonDocument::Indented);
    
    // Write to file atomically
    writeFileAtomically(json, filePath);
}

QJsonObject JsonExporter::buildRootObject(const DocumentState& state) {
    QJsonObject root;
    
    // PDF path
    root["pdf_path"] = state.pdfPath;
    
    // Image size
    QSize imageSize = state.getImageSize();
    QJsonArray imageSizeArray;
    imageSizeArray.append(imageSize.width());
    imageSizeArray.append(imageSize.height());
    root["image_size"] = imageSizeArray;
    
    // Aspect ratio
    if (imageSize.height() > 0) {
        double aspectRatio = static_cast<double>(imageSize.width()) / 
                             static_cast<double>(imageSize.height());
        root["aspect_ratio"] = aspectRatio;
    } else {
        root["aspect_ratio"] = 1.0;
    }
    
    // Version field for future compatibility
    root["version"] = "1.0";
    
    // Regions
    root["regions"] = buildRegionsObject(state);
    
    // Groups
    root["groups"] = buildGroupsObject(state);
    
    return root;
}

QJsonObject JsonExporter::buildRegionsObject(const DocumentState& state) {
    QJsonObject regionsObj;
    
    QList<QString> regionNames = state.getAllRegionNames();
    for (const QString& name : regionNames) {
        RegionData region = state.getRegion(name);
        regionsObj[name] = regionToJson(region);
    }
    
    return regionsObj;
}

QJsonObject JsonExporter::buildGroupsObject(const DocumentState& state) {
    QJsonObject groupsObj;
    
    QList<QString> groupNames = state.getGroupNames();
    for (const QString& groupName : groupNames) {
        GroupData group = state.getGroup(groupName);
        
        QJsonArray regionArray;
        for (const QString& regionName : group.regionNames) {
            regionArray.append(regionName);
        }
        
        groupsObj[groupName] = regionArray;
    }
    
    return groupsObj;
}

QJsonObject JsonExporter::regionToJson(const RegionData& region) {
    QJsonObject regionObj;
    
    // Normalized coordinates (source of truth)
    QJsonObject coordsObj;
    coordsObj["x1"] = region.normalizedCoords.x1;
    coordsObj["y1"] = region.normalizedCoords.y1;
    coordsObj["x2"] = region.normalizedCoords.x2;
    coordsObj["y2"] = region.normalizedCoords.y2;
    regionObj["normalized_coords"] = coordsObj;
    
    // Color
    regionObj["color"] = region.color;
    
    // Rotation angle (if non-zero)
    if (region.rotationAngle != 0.0) {
        regionObj["rotation_angle"] = region.rotationAngle;
    }
    
    // Group (only include if not empty)
    if (!region.group.isEmpty()) {
        regionObj["group"] = region.group;
    }
    
    // Shape type (optional, defaults to "rect")
    if (region.shapeType != "rect") {
        regionObj["shape_type"] = region.shapeType;
    }
    
    // Region type (optional, defaults to "none")
    if (region.regionType != "none") {
        regionObj["region_type"] = region.regionType;
    }
    
    // Percentage fill (optional, defaults to "none")
    if (region.percentageFill != "none") {
        regionObj["percentage_fill"] = region.percentageFill;
    }
    
    return regionObj;
}

void JsonExporter::writeFileAtomically(const QByteArray& jsonData, const QString& filePath) {
    // Create temp file path
    QString tempPath = filePath + ".tmp";
    
    // Write to temp file
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error(
            QString("Cannot open file for writing: %1").arg(tempFile.errorString()).toStdString()
        );
    }
    
    QTextStream out(&tempFile);
    out.setEncoding(QStringConverter::Encoding::Utf8);
    out << jsonData;
    tempFile.close();
    
    // Verify write succeeded
    if (tempFile.error() != QFile::NoError) {
        QFile::remove(tempPath);
        throw std::runtime_error(
            QString("Error writing file: %1").arg(tempFile.errorString()).toStdString()
        );
    }
    
    // Remove existing file if it exists
    if (QFile::exists(filePath)) {
        if (!QFile::remove(filePath)) {
            QFile::remove(tempPath);
            throw std::runtime_error("Cannot remove existing file");
        }
    }
    
    // Atomic rename
    if (!QFile::rename(tempPath, filePath)) {
        QFile::remove(tempPath);
        throw std::runtime_error("Cannot rename temp file to final file");
    }
}

} // namespace ocr_orc

