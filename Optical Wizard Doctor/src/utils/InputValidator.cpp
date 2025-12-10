#include "InputValidator.h"
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QDir>
#include <algorithm>

namespace ocr_orc {

QString InputValidator::validateRegionName(const QString& name, const QList<QString>& existingNames) {
    if (name.isEmpty()) {
        return "Region name cannot be empty";
    }
    
    if (name.length() > 100) {
        return "Region name cannot exceed 100 characters";
    }
    
    // Check for invalid characters (no control characters, no path separators)
    QRegularExpression invalidChars("[\\x00-\\x1F\\x7F/\\\\:*?\"<>|]");
    if (invalidChars.match(name).hasMatch()) {
        return "Region name contains invalid characters";
    }
    
    // Check for reserved names
    QString lowerName = name.toLower();
    if (lowerName == "null" || lowerName == "undefined" || lowerName == "true" || lowerName == "false") {
        return "Region name cannot be a reserved word";
    }
    
    // Check for uniqueness
    if (existingNames.contains(name, Qt::CaseSensitive)) {
        return "Region name already exists";
    }
    
    return QString(); // Valid
}

QString InputValidator::validateGroupName(const QString& name, const QList<QString>& existingNames) {
    if (name.isEmpty()) {
        return "Group name cannot be empty";
    }
    
    if (name.length() > 100) {
        return "Group name cannot exceed 100 characters";
    }
    
    // Check for invalid characters
    QRegularExpression invalidChars("[\\x00-\\x1F\\x7F/\\\\:*?\"<>|]");
    if (invalidChars.match(name).hasMatch()) {
        return "Group name contains invalid characters";
    }
    
    // Check for reserved names
    QString lowerName = name.toLower();
    if (lowerName == "null" || lowerName == "undefined" || lowerName == "true" || lowerName == "false") {
        return "Group name cannot be a reserved word";
    }
    
    // Check for uniqueness
    if (existingNames.contains(name, Qt::CaseSensitive)) {
        return "Group name already exists";
    }
    
    return QString(); // Valid
}

QString InputValidator::validateNormalizedCoords(const NormalizedCoords& coords) {
    // Check all coordinates are in valid range [0.0, 1.0]
    if (coords.x1 < CoordinateConstants::MIN_NORMALIZED || 
        coords.x1 > CoordinateConstants::MAX_NORMALIZED ||
        coords.y1 < CoordinateConstants::MIN_NORMALIZED || 
        coords.y1 > CoordinateConstants::MAX_NORMALIZED ||
        coords.x2 < CoordinateConstants::MIN_NORMALIZED || 
        coords.x2 > CoordinateConstants::MAX_NORMALIZED ||
        coords.y2 < CoordinateConstants::MIN_NORMALIZED || 
        coords.y2 > CoordinateConstants::MAX_NORMALIZED) {
        return "Coordinates must be between 0.0 and 1.0";
    }
    
    // Check coordinates form a valid rectangle (x1 < x2, y1 < y2)
    if (coords.x1 >= coords.x2) {
        return "Left coordinate must be less than right coordinate";
    }
    
    if (coords.y1 >= coords.y2) {
        return "Top coordinate must be less than bottom coordinate";
    }
    
    // Check minimum size
    double width = coords.x2 - coords.x1;
    double height = coords.y2 - coords.y1;
    if (width < RegionConstants::MIN_NORMALIZED_SIZE || 
        height < RegionConstants::MIN_NORMALIZED_SIZE) {
        return QString("Region is too small (minimum size: %1)").arg(RegionConstants::MIN_NORMALIZED_SIZE);
    }
    
    return QString(); // Valid
}

QString InputValidator::validateFilePath(const QString& filePath, bool mustExist) {
    if (filePath.isEmpty()) {
        return "File path cannot be empty";
    }
    
    // Check for path traversal attempts
    if (filePath.contains("..") || filePath.contains("~")) {
        return "File path contains invalid characters";
    }
    
    // Check if file exists (if required)
    if (mustExist && !QFileInfo::exists(filePath)) {
        return "File does not exist";
    }
    
    // Check if path is absolute (for security)
    QFileInfo fileInfo(filePath);
    if (!fileInfo.isAbsolute() && mustExist) {
        // Relative paths are OK for new files, but should be validated
        return QString();
    }
    
    return QString(); // Valid
}

QString InputValidator::validateDpi(int dpi) {
    if (dpi < PdfConstants::MIN_DPI || dpi > PdfConstants::MAX_DPI) {
        return QString("DPI must be between %1 and %2").arg(PdfConstants::MIN_DPI).arg(PdfConstants::MAX_DPI);
    }
    
    return QString(); // Valid
}

QString InputValidator::sanitizeRegionName(const QString& name) {
    QString sanitized = name;
    
    // Remove control characters and path separators
    QRegularExpression invalidChars("[\\x00-\\x1F\\x7F/\\\\:*?\"<>|]");
    sanitized.remove(invalidChars);
    
    // Trim whitespace
    sanitized = sanitized.trimmed();
    
    // Limit length
    if (sanitized.length() > 100) {
        sanitized = sanitized.left(100);
    }
    
    return sanitized;
}

QString InputValidator::sanitizeGroupName(const QString& name) {
    // Same as region name
    return sanitizeRegionName(name);
}

QString InputValidator::sanitizeFilePath(const QString& filePath) {
    QString sanitized = filePath;
    
    // Remove path traversal attempts
    sanitized.remove("..");
    sanitized.remove("~");
    
    // Remove control characters
    QRegularExpression invalidChars("[\\x00-\\x1F\\x7F]");
    sanitized.remove(invalidChars);
    
    // Normalize path separators (platform-specific)
    #ifdef Q_OS_WIN
        sanitized.replace('/', '\\');
    #else
        sanitized.replace('\\', '/');
    #endif
    
    return sanitized;
}

} // namespace ocr_orc

