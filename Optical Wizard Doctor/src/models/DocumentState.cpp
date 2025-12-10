#include "DocumentState.h"
#include "../core/Constants.h"
#include "StateSnapshot.h"
#include "../utils/PdfLoader.h"
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QSize>
#include <QtCore/QSet>
#include <QtCore/QRegularExpression>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <algorithm>

namespace ocr_orc {

DocumentState::DocumentState()
    : zoomLevel(1.0)
    , scaleFactor(1.0)
    , imageOffset(0.0, 0.0)
{
}

void DocumentState::addRegion(const QString& name, const RegionData& region) {
    if (name.isEmpty()) {
        return;  // Reject empty names
    }
    
    // Store old group if updating existing region
    bool isUpdate = hasRegion(name);
    QString oldGroup;
    if (isUpdate) {
        oldGroup = regions[name].group;
    }
    
    // Update or add region
    regions[name] = region;
    regions[name].name = name;  // Ensure name matches key
    
    // Handle group membership changes
    if (isUpdate && oldGroup != region.group) {
        // Remove from old group if changed
        if (!oldGroup.isEmpty()) {
            removeRegionFromGroup(name);
        }
        // Add to new group if specified
        if (!region.group.isEmpty()) {
            addRegionToGroup(name, region.group);
        }
    } else if (!isUpdate && !region.group.isEmpty()) {
        // New region with group
        addRegionToGroup(name, region.group);
    }
}

void DocumentState::removeRegion(const QString& name) {
    if (!hasRegion(name)) {
        return;
    }
    
    // Remove from group if in one
    RegionData& region = regions[name];
    if (!region.group.isEmpty()) {
        removeRegionFromGroup(name);
    }
    
    // Remove region
    regions.remove(name);
}

QList<QString> DocumentState::duplicateRegions(const QList<QString>& regionNames) {
    QList<QString> duplicatedNames;
    
    for (const QString& originalName : regionNames) {
        if (!hasRegion(originalName)) {
            continue; // Skip if region doesn't exist
        }
        
        RegionData originalRegion = getRegion(originalName);
        
        // Generate new name by auto-incrementing
        QString baseName = originalName;
        QString newName;
        int counter = 1;
        
        // Check if name ends with a number (with or without space)
        QRegularExpression regex(R"((.+?)(\s*)(\d+)$)");
        QRegularExpressionMatch match = regex.match(baseName);
        
        if (match.hasMatch()) {
            // Extract base name and number
            QString base = match.captured(1);
            QString separator = match.captured(2);
            int existingNumber = match.captured(3).toInt();
            
            // Start from existing number + 1
            counter = existingNumber + 1;
            baseName = base + separator + QString::number(existingNumber);
            
            // Generate new name
            do {
                newName = base + separator + QString::number(counter);
                counter++;
            } while (hasRegion(newName));
        } else {
            // No number at end, add _1, _2, etc.
            do {
                newName = baseName + "_" + QString::number(counter);
                counter++;
            } while (hasRegion(newName));
        }
        
        // Create duplicate with new name (same coordinates, color, but no group)
        RegionData duplicateRegion = originalRegion;
        duplicateRegion.name = newName;
        duplicateRegion.group = QString(); // Duplicates don't inherit group membership
        
        addRegion(newName, duplicateRegion);
        duplicatedNames.append(newName);
    }
    
    return duplicatedNames;
}

bool DocumentState::hasRegion(const QString& name) const {
    return regions.contains(name);
}

RegionData DocumentState::getRegion(const QString& name) const {
    if (hasRegion(name)) {
        return regions[name];
    }
    return RegionData();  // Return default/empty region
}

QList<QString> DocumentState::getAllRegionNames() const {
    QList<QString> names = regions.keys();
    std::sort(names.begin(), names.end());  // Alphabetically sorted
    return names;
}

bool DocumentState::isValidRegionName(const QString& name, const QString& excludeName) const {
    // Check non-empty
    if (name.trimmed().isEmpty()) {
        return false;
    }
    
    // Check maximum length (255 characters)
    if (name.length() > 255) {
        return false;
    }
    
    // Check for invalid characters (control chars, newlines, tabs)
    for (QChar ch : name) {
        ushort unicode = ch.unicode();
        // Check for control characters (0x00-0x1F except space), newlines, tabs
        if ((unicode < 0x20 && unicode != 0x09 && unicode != 0x0A && unicode != 0x0D) || 
            ch == '\n' || ch == '\r' || ch == '\t') {
            return false;
        }
    }
    
    // Check for reserved names (if any)
    static const QStringList reservedNames = {"", "null", "undefined"};
    if (reservedNames.contains(name, Qt::CaseInsensitive)) {
        return false;
    }
    
    // Check uniqueness (exclude current name if renaming)
    if (name != excludeName && hasRegion(name)) {
        return false;
    }
    
    return true;
}

bool DocumentState::renameRegion(const QString& oldName, const QString& newName) {
    // Validate inputs
    if (oldName.isEmpty() || newName.isEmpty()) {
        return false;
    }
    
    // Check if old region exists
    if (!hasRegion(oldName)) {
        return false;
    }
    
    // Validate new name (exclude old name from uniqueness check)
    if (!isValidRegionName(newName, oldName)) {
        return false;
    }
    
    // If names are the same (after trimming), nothing to do
    if (oldName == newName.trimmed()) {
        return true; // Success, but no change
    }
    
    QString trimmedNewName = newName.trimmed();
    
    // Get region data
    RegionData regionData = regions[oldName];
    
    // Update all group references first (before removing from map)
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        GroupData& group = it.value();
        int index = group.regionNames.indexOf(oldName);
        if (index != -1) {
            group.regionNames[index] = trimmedNewName;
        }
    }
    
    // Update region's internal name
    regionData.name = trimmedNewName;
    
    // Remove old entry and add new entry
    regions.remove(oldName);
    regions[trimmedNewName] = regionData;
    
    // Recalculate coordinates (synchronize)
    synchronizeCoordinates();
    
    return true;
}

bool DocumentState::changeRegionColor(const QString& regionName, const QString& color) {
    // Validate inputs
    if (regionName.isEmpty() || color.isEmpty()) {
        return false;
    }
    
    // Check if region exists
    if (!hasRegion(regionName)) {
        return false;
    }
    
    // Validate color (must be one of the allowed colors)
    QStringList validColors = {"blue", "red", "green", "yellow", "purple", "orange", "cyan"};
    if (!validColors.contains(color)) {
        return false;
    }
    
    // Update region color
    regions[regionName].color = color;
    
    return true;
}

void DocumentState::createGroup(const QString& name) {
    if (name.isEmpty()) {
        return;
    }
    
    if (!hasGroup(name)) {
        groups[name] = GroupData(name);
    }
}

void DocumentState::deleteGroup(const QString& name) {
    if (!hasGroup(name)) {
        return;
    }
    
    // Remove group assignment from all regions in this group
    GroupData& group = groups[name];
    for (const QString& regionName : group.regionNames) {
        if (hasRegion(regionName)) {
            regions[regionName].group = "";
        }
    }
    
    // Delete group
    groups.remove(name);
}

void DocumentState::addRegionToGroup(const QString& regionName, const QString& groupName) {
    if (!hasRegion(regionName)) {
        return;
    }
    
    if (groupName.isEmpty()) {
        removeRegionFromGroup(regionName);
        return;
    }
    
    // Create group if it doesn't exist
    if (!hasGroup(groupName)) {
        createGroup(groupName);
    }
    
    // Remove from old group if in one
    RegionData& region = regions[regionName];
    if (!region.group.isEmpty() && region.group != groupName) {
        removeRegionFromGroup(regionName);
    }
    
    // Add to new group
    region.group = groupName;
    groups[groupName].addRegion(regionName);
}

void DocumentState::removeRegionFromGroup(const QString& regionName) {
    if (!hasRegion(regionName)) {
        return;
    }
    
    RegionData& region = regions[regionName];
    if (region.group.isEmpty()) {
        return;
    }
    
    QString groupName = region.group;
    region.group = "";
    
    // Remove from group
    if (hasGroup(groupName)) {
        groups[groupName].removeRegion(regionName);
        
        // Delete group if empty
        if (groups[groupName].isEmpty()) {
            groups.remove(groupName);
        }
    }
}

QList<QString> DocumentState::getGroupNames() const {
    QList<QString> names = groups.keys();
    std::sort(names.begin(), names.end());  // Alphabetically sorted
    return names;
}

GroupData DocumentState::getGroup(const QString& name) const {
    if (hasGroup(name)) {
        return groups[name];
    }
    return GroupData();  // Return default/empty group
}

bool DocumentState::hasGroup(const QString& name) const {
    return groups.contains(name);
}

void DocumentState::synchronizeCoordinates() {
    if (image.isNull()) {
        return;  // No image, can't synchronize
    }
    
    int imgWidth = image.width();
    int imgHeight = image.height();
    
    // Validate image dimensions before using in calculations
    if (!CoordinateSystem::isValidImageDimensions(imgWidth, imgHeight)) {
        return;  // Invalid dimensions, can't synchronize
    }
    
    // Recalculate all coordinates from normalized (source of truth)
    for (auto it = regions.begin(); it != regions.end(); ++it) {
        RegionData& region = it.value();
        region.syncFromNormalized(imgWidth, imgHeight, scaleFactor, imageOffset);
    }
}

QSize DocumentState::getImageSize() const {
    if (image.isNull()) {
        return QSize(0, 0);
    }
    return image.size();
}

bool DocumentState::isValid() const {
    // Check for duplicate region names (shouldn't happen with QMap, but verify)
    QSet<QString> seenNames;
    for (auto it = regions.begin(); it != regions.end(); ++it) {
        if (seenNames.contains(it.key())) {
            return false;  // Duplicate name
        }
        seenNames.insert(it.key());
        
        // Validate region
        if (!it.value().isValid()) {
            return false;
        }
    }
    
    // Check for orphaned groups (groups with no valid regions)
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        const GroupData& group = it.value();
        for (const QString& regionName : group.regionNames) {
            if (!hasRegion(regionName)) {
                return false;  // Orphaned group reference
            }
        }
    }
    
    return true;
}

QJsonObject DocumentState::toJson() const {
    QJsonObject json;
    
    // Document information
    json["pdf_path"] = pdfPath;
    if (!image.isNull()) {
        QJsonArray imageSize;
        imageSize.append(image.width());
        imageSize.append(image.height());
        json["image_size"] = imageSize;
        json["aspect_ratio"] = CoordinateSystem::calculateAspectRatio(image.width(), image.height());
    }
    
    // Regions
    QJsonObject regionsObj;
    for (auto it = regions.begin(); it != regions.end(); ++it) {
        const RegionData& region = it.value();
        QJsonObject regionObj;
        
        // Normalized coordinates (source of truth)
        QJsonObject coordsObj;
        coordsObj["x1"] = region.normalizedCoords.x1;
        coordsObj["y1"] = region.normalizedCoords.y1;
        coordsObj["x2"] = region.normalizedCoords.x2;
        coordsObj["y2"] = region.normalizedCoords.y2;
        regionObj["normalized_coords"] = coordsObj;
        
        regionObj["color"] = region.color;
        if (!region.group.isEmpty()) {
            regionObj["group"] = region.group;
        }
        
        regionsObj[region.name] = regionObj;
    }
    json["regions"] = regionsObj;
    
    // Groups
    QJsonObject groupsObj;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        const GroupData& group = it.value();
        QJsonArray regionNamesArray;
        for (const QString& regionName : group.regionNames) {
            regionNamesArray.append(regionName);
        }
        groupsObj[group.name] = regionNamesArray;
    }
    json["groups"] = groupsObj;
    
    return json;
}

DocumentState DocumentState::fromJson(const QJsonObject& json) {
    DocumentState state;
    
    // Document information
    if (json.contains("pdf_path")) {
        state.pdfPath = json["pdf_path"].toString();
    }
    
    // Regions
    if (json.contains("regions") && json["regions"].isObject()) {
        QJsonObject regionsObj = json["regions"].toObject();
        for (auto it = regionsObj.begin(); it != regionsObj.end(); ++it) {
            QString regionName = it.key();
            QJsonObject regionObj = it.value().toObject();
            
            RegionData region;
            region.name = regionName;
            
            // Normalized coordinates (source of truth)
            if (regionObj.contains("normalized_coords")) {
                QJsonObject coordsObj = regionObj["normalized_coords"].toObject();
                region.normalizedCoords.x1 = coordsObj["x1"].toDouble();
                region.normalizedCoords.y1 = coordsObj["y1"].toDouble();
                region.normalizedCoords.x2 = coordsObj["x2"].toDouble();
                region.normalizedCoords.y2 = coordsObj["y2"].toDouble();
            }
            
            // Color
            if (regionObj.contains("color")) {
                region.color = regionObj["color"].toString();
            } else {
                region.color = "blue";  // Default
            }
            
            // Group
            if (regionObj.contains("group")) {
                region.group = regionObj["group"].toString();
            }
            
            state.addRegion(regionName, region);
        }
    }
    
    // Groups (may be reconstructed from region data, but load if present)
    if (json.contains("groups") && json["groups"].isObject()) {
        QJsonObject groupsObj = json["groups"].toObject();
        for (auto it = groupsObj.begin(); it != groupsObj.end(); ++it) {
            QString groupName = it.key();
            QJsonArray regionNamesArray = it.value().toArray();
            
            state.createGroup(groupName);
            for (const QJsonValue& value : regionNamesArray) {
                QString regionName = value.toString();
                state.addRegionToGroup(regionName, groupName);
            }
        }
    }
    
    return state;
}

void DocumentState::clear() {
    pdfPath = "";
    image = QImage();
    regions.clear();
    groups.clear();
    zoomLevel = 1.0;
    scaleFactor = 1.0;
    imageOffset = QPointF(0.0, 0.0);
    clearUndoRedoStacks();
}

StateSnapshot DocumentState::createCurrentSnapshot() const {
    // DISABLED FOR PRODUCTION: Snapshot logging
    // OCR_ORC_DEBUG("createCurrentSnapshot: regions=" << regions.size() << "groups=" << groups.size() << "pdfPath=" << pdfPath);
    StateSnapshot snapshot;
    snapshot.regions = regions;  // Qt's implicit sharing - efficient copy
    snapshot.groups = groups;    // Qt's implicit sharing - efficient copy
    snapshot.pdfPath = pdfPath;
    snapshot.imageSize = image.size();
    // OCR_ORC_DEBUG("createCurrentSnapshot: snapshot created with" << snapshot.regions.size() << "regions");
    return snapshot;
}

void DocumentState::saveState() {
    // DISABLED FOR PRODUCTION: State save logging
    // OCR_ORC_DEBUG("=== saveState() CALLED ===");
    // OCR_ORC_DEBUG("  Current state: regions=" << regions.size() << "groups=" << groups.size());
    
    // Create snapshot of current state
    StateSnapshot snapshot = createCurrentSnapshot();
    
    // Add to undo stack
    undoStack.append(snapshot);
    OCR_ORC_DEBUG("  Added to undo stack. New size:" << undoStack.size());
    
    // Limit stack size (remove oldest if over limit)
    if (undoStack.size() > MAX_UNDO_LEVELS) {
        undoStack.removeFirst();  // Remove oldest state
        OCR_ORC_DEBUG("  Stack limit reached, removed oldest. New size:" << undoStack.size());
    }
    
    // Clear redo stack on new operation
    // (User can't redo after a new operation)
    int redoSize = redoStack.size();
    redoStack.clear();
    if (redoSize > 0) {
        OCR_ORC_DEBUG("  Cleared redo stack (had" << redoSize << "states)");
    }
    
    OCR_ORC_DEBUG("=== saveState() COMPLETED ===");
}

void DocumentState::restoreState(const StateSnapshot& snapshot) {
    // DISABLED FOR PRODUCTION: State restore logging
    // OCR_ORC_DEBUG("=== restoreState() CALLED ===");
    // OCR_ORC_DEBUG("  Snapshot: regions=" << snapshot.regions.size() << "groups=" << snapshot.groups.size() << "pdfPath=" << snapshot.pdfPath);
    // OCR_ORC_DEBUG("  Before restore: regions=" << regions.size() << "groups=" << groups.size());
    
    // Restore core data
    regions = snapshot.regions;
    groups = snapshot.groups;
    pdfPath = snapshot.pdfPath;
    
    // OCR_ORC_DEBUG("  After restore: regions=" << regions.size() << "groups=" << groups.size());
    
    // Attempt to reload image if PDF path exists and is valid
    if (!pdfPath.isEmpty()) {
        QFileInfo fileInfo(pdfPath);
        if (fileInfo.exists() && fileInfo.isReadable()) {
            // Reload image from PDF
            QImage reloadedImage = PdfLoader::loadPdfFirstPage(pdfPath);
            if (!reloadedImage.isNull() && 
                CoordinateSystem::isValidImageDimensions(reloadedImage.width(), reloadedImage.height())) {
                image = reloadedImage;
            } else {
                // PDF exists but couldn't load - clear image to prevent invalid state
                image = QImage();
            }
        } else {
            // PDF file doesn't exist or isn't readable - clear image
            image = QImage();
        }
    } else {
        // No PDF path - clear image
        image = QImage();
    }
    
    // Only synchronize coordinates if we have a valid image
    if (!image.isNull() && CoordinateSystem::isValidImageDimensions(image.width(), image.height())) {
        // Recalculate all coordinates from normalized (source of truth)
        // OCR_ORC_DEBUG("  Calling synchronizeCoordinates()...");
        synchronizeCoordinates();
    } else {
        // Clear coordinates if no valid image
        for (auto it = regions.begin(); it != regions.end(); ++it) {
            RegionData& region = it.value();
            region.imageCoords = ImageCoords();
            region.canvasCoords = CanvasCoords();
        }
    }
    // OCR_ORC_DEBUG("=== restoreState() COMPLETED ===");
}

void DocumentState::undoAction() {
    // DISABLED FOR PRODUCTION: Undo/redo logging
    // OCR_ORC_DEBUG("=== undoAction() CALLED ===");
    // OCR_ORC_DEBUG("  undoStack size=" << undoStack.size() << "redoStack size=" << redoStack.size());
    
    if (undoStack.isEmpty()) {
        OCR_ORC_DEBUG("  ERROR: Undo stack is empty, cannot undo");
        return;  // Nothing to undo
    }
    
    // Save current state to redo stack
    OCR_ORC_DEBUG("  Saving current state to redo stack...");
    StateSnapshot current = createCurrentSnapshot();
    redoStack.append(current);
    OCR_ORC_DEBUG("  Redo stack size now:" << redoStack.size());
    
    // Pop previous state from undo stack
    OCR_ORC_DEBUG("  Popping from undo stack...");
    StateSnapshot previous = undoStack.takeLast();
    OCR_ORC_DEBUG("  Undo stack size now:" << undoStack.size());
    OCR_ORC_DEBUG("  Previous state: regions=" << previous.regions.size() << "groups=" << previous.groups.size());
    
    // Restore previous state
    OCR_ORC_DEBUG("  Restoring previous state...");
    restoreState(previous);
    OCR_ORC_DEBUG("=== undoAction() COMPLETED ===");
}

void DocumentState::redoAction() {
    // DISABLED FOR PRODUCTION: Redo logging
    // OCR_ORC_DEBUG("=== redoAction() CALLED ===");
    // OCR_ORC_DEBUG("  undoStack size=" << undoStack.size() << "redoStack size=" << redoStack.size());
    
    if (redoStack.isEmpty()) {
        OCR_ORC_DEBUG("  ERROR: Redo stack is empty, cannot redo");
        return;  // Nothing to redo
    }
    
    // Save current state to undo stack
    OCR_ORC_DEBUG("  Saving current state to undo stack...");
    StateSnapshot current = createCurrentSnapshot();
    undoStack.append(current);
    OCR_ORC_DEBUG("  Undo stack size now:" << undoStack.size());
    
    // Pop next state from redo stack
    OCR_ORC_DEBUG("  Popping from redo stack...");
    StateSnapshot next = redoStack.takeLast();
    OCR_ORC_DEBUG("  Redo stack size now:" << redoStack.size());
    OCR_ORC_DEBUG("  Next state: regions=" << next.regions.size() << "groups=" << next.groups.size());
    
    // Restore next state
    OCR_ORC_DEBUG("  Restoring next state...");
    restoreState(next);
    OCR_ORC_DEBUG("=== redoAction() COMPLETED ===");
}

void DocumentState::clearUndoRedoStacks() {
    undoStack.clear();
    redoStack.clear();
}

void DocumentState::setImage(const QImage& img) {
    image = img;
    // Recalculate coordinates when image changes
    synchronizeCoordinates();
}

void DocumentState::setZoomLevel(double zoom) {
    zoomLevel = zoom;
    // Recalculate coordinates when zoom changes
    synchronizeCoordinates();
}

void DocumentState::setImageOffset(const QPointF& offset) {
    imageOffset = offset;
    // Recalculate coordinates when pan changes
    synchronizeCoordinates();
}

void DocumentState::updateScaleFactor(int canvasWidth, int canvasHeight) {
    if (image.isNull()) {
        scaleFactor = 1.0;
        return;
    }
    
    int imgWidth = image.width();
    int imgHeight = image.height();
    
    if (imgWidth <= 0 || imgHeight <= 0) {
        scaleFactor = 1.0;
        return;
    }
    
    // Calculate base scale (fit to canvas)
    double baseScaleX = static_cast<double>(canvasWidth) / imgWidth;
    double baseScaleY = static_cast<double>(canvasHeight) / imgHeight;
    double baseScale = std::min({baseScaleX, baseScaleY, 1.0});
    
    // Apply zoom level
    scaleFactor = baseScale * zoomLevel;
    
    // Recalculate coordinates
    synchronizeCoordinates();
}

} // namespace ocr_orc

