#include "CoordinateSystem.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ocr_orc {

ImageCoords CoordinateSystem::normalizedToImage(const NormalizedCoords& norm, 
                                                  int imgWidth, 
                                                  int imgHeight) {
    // Formula: image_x = normalized_x * image_width
    ImageCoords img;
    img.x1 = static_cast<int>(std::round(norm.x1 * imgWidth));
    img.y1 = static_cast<int>(std::round(norm.y1 * imgHeight));
    img.x2 = static_cast<int>(std::round(norm.x2 * imgWidth));
    img.y2 = static_cast<int>(std::round(norm.y2 * imgHeight));
    
    // Clamp to image bounds
    img.x1 = std::clamp(img.x1, 0, imgWidth);
    img.y1 = std::clamp(img.y1, 0, imgHeight);
    img.x2 = std::clamp(img.x2, 0, imgWidth);
    img.y2 = std::clamp(img.y2, 0, imgHeight);
    
    return img;
}

NormalizedCoords CoordinateSystem::imageToNormalized(const ImageCoords& img, 
                                                       int imgWidth, 
                                                       int imgHeight) {
    // Formula: normalized_x = image_x / image_width
    // Handle division by zero
    if (imgWidth <= 0 || imgHeight <= 0) {
        return NormalizedCoords(0.0, 0.0, 0.0, 0.0);
    }
    
    NormalizedCoords norm;
    norm.x1 = static_cast<double>(img.x1) / imgWidth;
    norm.y1 = static_cast<double>(img.y1) / imgHeight;
    norm.x2 = static_cast<double>(img.x2) / imgWidth;
    norm.y2 = static_cast<double>(img.y2) / imgHeight;
    
    // Clamp to valid range
    clampToBounds(norm);
    
    return norm;
}

CanvasCoords CoordinateSystem::imageToCanvas(const ImageCoords& img, 
                                              double scaleFactor, 
                                              const QPointF& offset) {
    // Formula: canvas_x = (image_x * scale_factor) + offset_x
    CanvasCoords canvas;
    canvas.x1 = (img.x1 * scaleFactor) + offset.x();
    canvas.y1 = (img.y1 * scaleFactor) + offset.y();
    canvas.x2 = (img.x2 * scaleFactor) + offset.x();
    canvas.y2 = (img.y2 * scaleFactor) + offset.y();
    
    return canvas;
}

ImageCoords CoordinateSystem::canvasToImage(const CanvasCoords& canvas, 
                                               double scaleFactor, 
                                               const QPointF& offset) {
    // Formula: image_x = (canvas_x - offset_x) / scale_factor
    // Handle division by zero
    if (std::abs(scaleFactor) < CoordinateConstants::EPSILON) {
        return ImageCoords(0, 0, 0, 0);
    }
    
    ImageCoords img;
    img.x1 = static_cast<int>(std::round((canvas.x1 - offset.x()) / scaleFactor));
    img.y1 = static_cast<int>(std::round((canvas.y1 - offset.y()) / scaleFactor));
    img.x2 = static_cast<int>(std::round((canvas.x2 - offset.x()) / scaleFactor));
    img.y2 = static_cast<int>(std::round((canvas.y2 - offset.y()) / scaleFactor));
    
    return img;
}

CanvasCoords CoordinateSystem::normalizedToCanvas(const NormalizedCoords& norm,
                                                    int imgWidth,
                                                    int imgHeight,
                                                    double scaleFactor,
                                                    const QPointF& offset) {
    // Step 1: Normalized → Image
    ImageCoords img = normalizedToImage(norm, imgWidth, imgHeight);
    
    // Step 2: Image → Canvas
    return imageToCanvas(img, scaleFactor, offset);
}

bool CoordinateSystem::isValidNormalized(const NormalizedCoords& coords) {
    // Check range: 0.0 <= x,y <= 1.0
    const bool inRange = (coords.x1 >= 0.0 && coords.x1 <= 1.0) &&
                        (coords.y1 >= 0.0 && coords.y1 <= 1.0) &&
                        (coords.x2 >= 0.0 && coords.x2 <= 1.0) &&
                        (coords.y2 >= 0.0 && coords.y2 <= 1.0);
    
    if (!inRange) {
        return false;
    }
    
    // Check order: x1 < x2, y1 < y2
    return (coords.x1 < coords.x2) && (coords.y1 < coords.y2);
}

bool CoordinateSystem::isValidImage(const ImageCoords& coords, 
                                     int imgWidth, 
                                     int imgHeight) {
    // Check bounds: 0 <= x,y <= dimensions
    const bool inBounds = (coords.x1 >= 0 && coords.x1 <= imgWidth) &&
                         (coords.y1 >= 0 && coords.y1 <= imgHeight) &&
                         (coords.x2 >= 0 && coords.x2 <= imgWidth) &&
                         (coords.y2 >= 0 && coords.y2 <= imgHeight);
    
    if (!inBounds) {
        return false;
    }
    
    // Check order: x1 < x2, y1 < y2
    return (coords.x1 < coords.x2) && (coords.y1 < coords.y2);
}

bool CoordinateSystem::isValidCanvas(const CanvasCoords& coords) {
    // Check order: x1 < x2, y1 < y2
    // Canvas coordinates can be negative or exceed canvas bounds during operations
    return coords.x1 < coords.x2 && coords.y1 < coords.y2;
}

void CoordinateSystem::normalizeCoords(NormalizedCoords& coords) {
    // Ensure x1 < x2, y1 < y2
    if (coords.x1 > coords.x2) {
        std::swap(coords.x1, coords.x2);
    }
    if (coords.y1 > coords.y2) {
        std::swap(coords.y1, coords.y2);
    }
}

void CoordinateSystem::clampToBounds(NormalizedCoords& coords) {
    // Clamp to 0.0-1.0 range
    coords.x1 = std::clamp(coords.x1, 0.0, 1.0);
    coords.y1 = std::clamp(coords.y1, 0.0, 1.0);
    coords.x2 = std::clamp(coords.x2, 0.0, 1.0);
    coords.y2 = std::clamp(coords.y2, 0.0, 1.0);
    
    // Ensure order after clamping
    normalizeCoords(coords);
}

double CoordinateSystem::calculateAspectRatio(int width, int height) {
    if (height <= 0) {
        return 0.0;
    }
    return static_cast<double>(width) / static_cast<double>(height);
}

bool CoordinateSystem::isValidImageDimensions(int width, int height) {
    return width > 0 && height > 0;
}

bool CoordinateSystem::isValidDouble(double value) {
    return std::isfinite(value) && !std::isnan(value);
}

bool CoordinateSystem::isValidPoint(const QPointF& point) {
    return isValidDouble(point.x()) && isValidDouble(point.y());
}

bool CoordinateSystem::isValidNormalizedCoords(const NormalizedCoords& coords) {
    return isValidDouble(coords.x1) && isValidDouble(coords.y1) &&
           isValidDouble(coords.x2) && isValidDouble(coords.y2);
}

} // namespace ocr_orc

