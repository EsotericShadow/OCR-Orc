#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtCore/QString>

// Conditional compilation for debug output
// ENABLED FOR ROTATION DEBUGGING
#define OCR_ORC_DEBUG_ENABLED 1

// Debug macro that only outputs in debug builds
#if OCR_ORC_DEBUG_ENABLED
    #include <QtCore/QDebug>
    #define OCR_ORC_DEBUG(x) qDebug() << "[ROTATION DEBUG]" << x
    #define OCR_ORC_WARNING(x) qWarning() << "[ROTATION WARNING]" << x
#else
    #define OCR_ORC_DEBUG(x) do {} while(0)
    #define OCR_ORC_WARNING(x) do {} while(0)
#endif

namespace ocr_orc {

/**
 * @brief Application-wide constants
 */

// Coordinate system constants
namespace CoordinateConstants {
    constexpr double EPSILON = 1e-9;  // Epsilon for floating-point comparisons
    constexpr double MIN_NORMALIZED = 0.0;
    constexpr double MAX_NORMALIZED = 1.0;
}

// Canvas and rendering constants
namespace CanvasConstants {
    constexpr double MIN_ZOOM = 0.01;  // Never allow 0.0 to prevent division by zero
    constexpr double MAX_ZOOM = 10.0;
    constexpr double DEFAULT_ZOOM = 1.0;
    constexpr double ZOOM_FACTOR = 1.2;  // Zoom in/out multiplier
    constexpr double ZOOM_WHEEL_FACTOR = 0.1;  // Zoom per wheel step
    constexpr double SCROLL_SPEED = 0.5;  // Scroll speed multiplier
    constexpr int WHEEL_DELTA_NORMALIZATION = 120;  // Standard wheel delta
    constexpr double MAX_SCALE = 1.0;  // Maximum initial scale to fit
}

// PDF loading constants
namespace PdfConstants {
    constexpr int DEFAULT_DPI = 150;
    constexpr int MIN_DPI = 72;
    constexpr int MAX_DPI = 300;
}

// Region constants
namespace RegionConstants {
    constexpr int MIN_REGION_SIZE = 10;  // Minimum region size in pixels
    constexpr double HANDLE_SIZE = 8.0;  // Resize handle size
    constexpr double HANDLE_TOLERANCE_OFFSET = 2.0;  // Additional tolerance for handle clicking
    constexpr double MIN_NORMALIZED_SIZE = 0.001;  // Minimum normalized region size
}

// UI constants
namespace UIConstants {
    const QString ZOOM_IN_SYMBOL = "+";
    const QString ZOOM_OUT_SYMBOL = "−";
}

} // namespace ocr_orc

#endif // CONSTANTS_H

