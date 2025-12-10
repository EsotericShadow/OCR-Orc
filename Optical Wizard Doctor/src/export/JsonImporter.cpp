#include "JsonImporter.h"
#include "../core/Constants.h"
#include "../core/CoordinateSystem.h"
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonArray>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif
#include <stdexcept>

namespace ocr_orc {

void JsonImporter::importFromFile(DocumentState& state, const QString& filePath) {
    // Read file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(
            QString("Cannot open file for reading: %1").arg(file.errorString()).toStdString()
        );
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    // Parse JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        throw std::runtime_error(
            QString("JSON parse error at offset %1: %2")
                .arg(error.offset)
                .arg(error.errorString())
                .toStdString()
        );
    }
    
    QJsonObject root = doc.object();
    
    // Validate structure
    if (!root.contains("regions") || !root["regions"].isObject()) {
        throw std::runtime_error("Invalid JSON: missing or invalid 'regions' field");
    }
    
    // Clear existing regions and groups (fresh import)
    state.regions.clear();
    state.groups.clear();
    
    // Load PDF path if provided
    if (root.contains("pdf_path") && root["pdf_path"].isString()) {
        state.pdfPath = root["pdf_path"].toString();
    }
    
    // Load regions
    QJsonObject regionsObj = root["regions"].toObject();
    loadRegions(state, regionsObj);
    
    // Load groups (or reconstruct from region data)
    if (root.contains("groups") && root["groups"].isObject()) {
        QJsonObject groupsObj = root["groups"].toObject();
        loadGroups(state, groupsObj);
    } else {
        // Reconstruct groups from region data (backward compatibility)
        reconstructGroups(state);
    }
    
    // Synchronize coordinates (recalculate image/canvas from normalized)
    state.synchronizeCoordinates();
}

NormalizedCoords JsonImporter::parseCoordinates(const QJsonValue& value) {
    NormalizedCoords coords;
    
    if (value.isObject()) {
        // Dict format: {"x1": 0.25, "y1": 0.10, "x2": 0.35, "y2": 0.20}
        QJsonObject obj = value.toObject();
        
        if (!obj.contains("x1") || !obj.contains("y1") || 
            !obj.contains("x2") || !obj.contains("y2")) {
            throw std::runtime_error("Invalid coordinate format: missing x1, y1, x2, or y2");
        }
        
        coords.x1 = obj["x1"].toDouble();
        coords.y1 = obj["y1"].toDouble();
        coords.x2 = obj["x2"].toDouble();
        coords.y2 = obj["y2"].toDouble();
    } else if (value.isArray()) {
        // Array format: [0.25, 0.10, 0.35, 0.20]
        QJsonArray arr = value.toArray();
        if (arr.size() < 4) {
            throw std::runtime_error("Invalid coordinate format: array must have at least 4 elements");
        }
        coords.x1 = arr[0].toDouble();
        coords.y1 = arr[1].toDouble();
        coords.x2 = arr[2].toDouble();
        coords.y2 = arr[3].toDouble();
    } else {
        throw std::runtime_error("Invalid coordinate format: must be object or array");
    }
    
    // Validate coordinates for NaN/Infinity
    if (!CoordinateSystem::isValidNormalizedCoords(coords)) {
        throw std::runtime_error("Invalid coordinate values: NaN or Infinity detected");
    }
    
    return coords;
}

bool JsonImporter::validateCoordinates(const NormalizedCoords& coords) {
    // Check all coordinates are in valid range [0.0, 1.0]
    if (coords.x1 < 0.0 || coords.x1 > 1.0 ||
        coords.y1 < 0.0 || coords.y1 > 1.0 ||
        coords.x2 < 0.0 || coords.x2 > 1.0 ||
        coords.y2 < 0.0 || coords.y2 > 1.0) {
        return false;
    }
    
    // Check coordinates form a valid rectangle (x1 < x2, y1 < y2)
    if (coords.x1 >= coords.x2 || coords.y1 >= coords.y2) {
        return false;
    }
    
    return true;
}

void JsonImporter::loadRegions(DocumentState& state, const QJsonObject& regionsObj) {
    int skippedCount = 0;
    QStringList duplicateNames;
    
    for (auto it = regionsObj.begin(); it != regionsObj.end(); ++it) {
        QString name = it.key();
        QJsonObject regionObj = it.value().toObject();
        
        try {
            // Check for duplicate region names
            if (state.hasRegion(name)) {
                duplicateNames.append(name);
                OCR_ORC_WARNING("Skipping duplicate region" << name);
                skippedCount++;
                continue; // Skip duplicate
            }
            
            // Parse coordinates
            if (!regionObj.contains("normalized_coords")) {
                OCR_ORC_WARNING("Skipping region" << name << ": missing normalized_coords");
                skippedCount++;
                continue;
            }
            
            NormalizedCoords coords = parseCoordinates(regionObj["normalized_coords"]);
            
            // Validate coordinates
            if (!validateCoordinates(coords)) {
                OCR_ORC_WARNING("Skipping region" << name << ": invalid coordinates");
                skippedCount++;
                continue;
            }
            
            // Create region
            RegionData region;
            region.name = name;
            region.normalizedCoords = coords;
            region.color = regionObj.contains("color") ? 
                          regionObj["color"].toString("blue") : "blue";
            region.group = regionObj.contains("group") && !regionObj["group"].isNull() ?
                          regionObj["group"].toString("") : "";
            
            // Shape type (optional, defaults to "rect")
            region.shapeType = regionObj.contains("shape_type") ?
                              regionObj["shape_type"].toString("rect") : "rect";
            
            // Region type (optional, defaults to "none")
            region.regionType = regionObj.contains("region_type") ?
                               regionObj["region_type"].toString("none") : "none";
            
            // Percentage fill (optional, defaults to "none")
            region.percentageFill = regionObj.contains("percentage_fill") ?
                                   regionObj["percentage_fill"].toString("none") : "none";
            
            // Rotation angle (optional, defaults to 0.0)
            region.rotationAngle = regionObj.contains("rotation_angle") ?
                                  regionObj["rotation_angle"].toDouble(0.0) : 0.0;
            // Validate rotation angle
            if (!CoordinateSystem::isValidDouble(region.rotationAngle) || std::abs(region.rotationAngle) > 360.0) {
                OCR_ORC_WARNING("Invalid rotation angle for region" << name << ", resetting to 0.0");
                region.rotationAngle = 0.0;
            }
            
            // Add to state
            state.addRegion(name, region);
            
        } catch (const std::exception& e) {
            OCR_ORC_WARNING("Error loading region" << name << ":" << e.what());
            skippedCount++;
        }
    }
    
    // Report duplicates to user
    if (!duplicateNames.isEmpty()) {
        OCR_ORC_WARNING("Duplicate region names found and skipped:" << duplicateNames);
    }
    
    if (skippedCount > 0) {
        OCR_ORC_WARNING("Skipped" << skippedCount << "invalid regions during import");
    }
}

void JsonImporter::loadGroups(DocumentState& state, const QJsonObject& groupsObj) {
    for (auto it = groupsObj.begin(); it != groupsObj.end(); ++it) {
        QString groupName = it.key();
        
        if (!it.value().isArray()) {
            OCR_ORC_WARNING("Skipping invalid group" << groupName << ": not an array");
            continue;
        }
        
        QJsonArray regionArray = it.value().toArray();
        
        // Create group
        state.createGroup(groupName);
        
        // Add regions to group
        for (const QJsonValue& value : regionArray) {
            QString regionName = value.toString();
            if (state.hasRegion(regionName)) {
                state.addRegionToGroup(regionName, groupName);
            } else {
                OCR_ORC_WARNING("Group" << groupName << "references non-existent region:" << regionName);
            }
        }
    }
}

void JsonImporter::reconstructGroups(DocumentState& state) {
    // Iterate through regions and create groups based on region.group field
    QList<QString> regionNames = state.getAllRegionNames();
    
    for (const QString& regionName : regionNames) {
        RegionData region = state.getRegion(regionName);
        if (!region.group.isEmpty()) {
            // Create group if it doesn't exist
            if (!state.hasGroup(region.group)) {
                state.createGroup(region.group);
            }
            // Add region to group
            state.addRegionToGroup(regionName, region.group);
        }
    }
}

} // namespace ocr_orc

