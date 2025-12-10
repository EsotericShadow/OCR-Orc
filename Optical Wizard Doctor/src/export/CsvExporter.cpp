#include "CsvExporter.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QStringConverter>
#include <QtCore/QList>
#include <algorithm>
#include <stdexcept>

namespace ocr_orc {

void CsvExporter::exportToFile(const DocumentState& state, const QString& filePath) {
    // Create temp file path for atomic write
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
    
    // Write header row
    out << "Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)\n";
    
    // Get all region names and sort alphabetically
    QList<QString> regionNames = state.getAllRegionNames();
    std::sort(regionNames.begin(), regionNames.end());
    
    // Write region rows
    for (const QString& regionName : regionNames) {
        RegionData region = state.getRegion(regionName);
        
        out << escapeCsvField(regionName) << ","
            << escapeCsvField(region.group.isEmpty() ? "" : region.group) << ","
            << escapeCsvField(region.color) << ","
            << formatCoordinate(region.normalizedCoords.x1) << ","
            << formatCoordinate(region.normalizedCoords.y1) << ","
            << formatCoordinate(region.normalizedCoords.x2) << ","
            << formatCoordinate(region.normalizedCoords.y2) << "\n";
    }
    
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

QString CsvExporter::escapeCsvField(const QString& field) {
    // RFC 4180: If field contains comma, quote, newline, tab, or carriage return, wrap in quotes
    if (field.contains(',') || field.contains('"') || field.contains('\n') || 
        field.contains('\r') || field.contains('\t')) {
        // Escape quotes by doubling them
        QString escaped = field;
        escaped.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return field;
}

QString CsvExporter::formatCoordinate(double value) {
    // Format to 6 decimal places as specified in requirements
    // Keep all 6 decimal places for consistency and precision
    return QString::number(value, 'f', 6);
}

} // namespace ocr_orc

