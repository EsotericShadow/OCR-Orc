#include "MaskGenerator.h"
#include "../export/JsonExporter.h"
#include "../core/Constants.h"
#include "../core/CoordinateSystem.h"
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtCore/QFileInfo>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif
#include <algorithm>
#include <stdexcept>

namespace ocr_orc {

void MaskGenerator::generate(const DocumentState& state, const QString& filePath) {
    // Validate image exists
    if (state.image.isNull()) {
        throw std::runtime_error("No image loaded. Please load a PDF first.");
    }
    
    // Validate regions exist
    if (state.regions.isEmpty()) {
        throw std::runtime_error("No regions defined. Please create regions first.");
    }
    
    // Create mask image
    QImage mask = createMaskImage(state);
    
    // Save mask image
    if (!mask.save(filePath)) {
        throw std::runtime_error("Cannot save mask image. Check file path and permissions.");
    }
    
    // Save coordinate JSON alongside mask
    QFileInfo fileInfo(filePath);
    QString jsonPath = fileInfo.path() + "/" + fileInfo.baseName() + ".json";
    
    try {
        JsonExporter::exportToFile(state, jsonPath);
    } catch (const std::exception& e) {
        // Mask was saved, but JSON failed - warn but don't fail
        OCR_ORC_WARNING("Mask saved but coordinate JSON export failed:" << e.what());
    }
}

QImage MaskGenerator::createMaskImage(const DocumentState& state) {
    // Get image dimensions
    int width = state.image.width();
    int height = state.image.height();
    
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Invalid image dimensions");
    }
    
    // Create black mask image
    QImage mask(width, height, QImage::Format_RGB32);
    mask.fill(QColor(0, 0, 0)); // Black background
    
    // Draw white rectangles for regions
    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing, false); // Sharp edges for OCR
    
    for (auto it = state.regions.begin(); it != state.regions.end(); ++it) {
        const RegionData& region = it.value();
        
        // Use normalized coordinates (source of truth) instead of potentially stale imageCoords
        NormalizedCoords norm = region.normalizedCoords;
        
        // Validate normalized coordinates before conversion
        if (!CoordinateSystem::isValidNormalized(norm)) {
            continue; // Skip invalid region
        }
        
        // Convert normalized coordinates to image coordinates
        ImageCoords imgCoords = CoordinateSystem::normalizedToImage(norm, width, height);
        
        // Validate converted coordinates
        if (!CoordinateSystem::isValidImage(imgCoords, width, height)) {
            continue; // Skip invalid region
        }
        
        // Ensure coordinates are in order
        int x1 = std::min(imgCoords.x1, imgCoords.x2);
        int y1 = std::min(imgCoords.y1, imgCoords.y2);
        int x2 = std::max(imgCoords.x1, imgCoords.x2);
        int y2 = std::max(imgCoords.y1, imgCoords.y2);
        
        // Validate and clamp to image bounds
        validateAndClampCoordinates(x1, y1, x2, y2, width, height);
        
        // Ensure rectangle has positive size
        if (x2 > x1 && y2 > y1) {
            // Draw white rectangle
            painter.fillRect(x1, y1, x2 - x1, y2 - y1, QColor(255, 255, 255));
        }
    }
    
    painter.end();
    
    return mask;
}

void MaskGenerator::validateAndClampCoordinates(int& x1, int& y1, int& x2, int& y2,
                                                int width, int height) {
    // Clamp to image bounds
    x1 = std::max(0, std::min(width - 1, x1));
    y1 = std::max(0, std::min(height - 1, y1));
    x2 = std::max(0, std::min(width, x2));
    y2 = std::max(0, std::min(height, y2));
    
    // Ensure x2 > x1 and y2 > y1
    if (x2 <= x1) {
        if (x1 < width - 1) {
            x2 = x1 + 1;
        } else {
            x1 = x2 - 1;
        }
    }
    
    if (y2 <= y1) {
        if (y1 < height - 1) {
            y2 = y1 + 1;
        } else {
            y1 = y2 - 1;
        }
    }
}

} // namespace ocr_orc

