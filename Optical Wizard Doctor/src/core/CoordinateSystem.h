#ifndef COORDINATE_SYSTEM_H
#define COORDINATE_SYSTEM_H

#include <QtCore/QPointF>

/**
 * @brief Coordinate system structures and transformation functions
 * 
 * Three coordinate systems are used:
 * 1. Normalized (0.0-1.0): Source of truth, resolution-independent
 * 2. Image (pixels): Pixel positions in original PDF image
 * 3. Canvas (display): Screen positions for rendering
 */

namespace ocr_orc {

/**
 * @brief Normalized coordinates (0.0 to 1.0)
 * Source of truth for all coordinate storage
 */
struct NormalizedCoords {
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;
    
    NormalizedCoords() = default;
    NormalizedCoords(double x1, double y1, double x2, double y2)
        : x1(x1), y1(y1), x2(x2), y2(y2) {}
};

/**
 * @brief Image coordinates (pixel positions)
 * Integer pixel positions in the original PDF image
 */
struct ImageCoords {
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    
    ImageCoords() = default;
    ImageCoords(int x1, int y1, int x2, int y2)
        : x1(x1), y1(y1), x2(x2), y2(y2) {}
};

/**
 * @brief Canvas coordinates (display pixels)
 * Floating-point screen positions for rendering
 */
struct CanvasCoords {
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;
    
    CanvasCoords() = default;
    CanvasCoords(double x1, double y1, double x2, double y2)
        : x1(x1), y1(y1), x2(x2), y2(y2) {}
};

/**
 * @brief Coordinate transformation system
 * 
 * Provides functions to transform between three coordinate systems:
 * - Normalized (0.0-1.0): Resolution-independent storage
 * - Image (pixels): Original PDF pixel positions
 * - Canvas (display): Screen rendering positions
 */
class CoordinateSystem {
public:
    /**
     * @brief Convert normalized coordinates to image coordinates
     * @param norm Normalized coordinates (0.0-1.0)
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     * @return Image coordinates in pixels
     */
    static ImageCoords normalizedToImage(const NormalizedCoords& norm, 
                                         int imgWidth, 
                                         int imgHeight);
    
    /**
     * @brief Convert image coordinates to normalized coordinates
     * @param img Image coordinates in pixels
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     * @return Normalized coordinates (0.0-1.0)
     */
    static NormalizedCoords imageToNormalized(const ImageCoords& img, 
                                               int imgWidth, 
                                               int imgHeight);
    
    /**
     * @brief Convert image coordinates to canvas coordinates
     * @param img Image coordinates in pixels
     * @param scaleFactor Display scale factor (accounts for zoom)
     * @param offset Image position offset on canvas
     * @return Canvas coordinates for rendering
     */
    static CanvasCoords imageToCanvas(const ImageCoords& img, 
                                       double scaleFactor, 
                                       const QPointF& offset);
    
    /**
     * @brief Convert canvas coordinates to image coordinates
     * @param canvas Canvas coordinates from user input
     * @param scaleFactor Display scale factor (accounts for zoom)
     * @param offset Image position offset on canvas
     * @return Image coordinates in pixels
     */
    static ImageCoords canvasToImage(const CanvasCoords& canvas, 
                                      double scaleFactor, 
                                      const QPointF& offset);
    
    /**
     * @brief Convert normalized coordinates to canvas coordinates (via image)
     * @param norm Normalized coordinates (0.0-1.0)
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     * @param scaleFactor Display scale factor
     * @param offset Image position offset on canvas
     * @return Canvas coordinates for rendering
     */
    static CanvasCoords normalizedToCanvas(const NormalizedCoords& norm,
                                            int imgWidth,
                                            int imgHeight,
                                            double scaleFactor,
                                            const QPointF& offset);
    
    /**
     * @brief Validate normalized coordinates
     * @param coords Coordinates to validate
     * @return true if valid (0.0-1.0 range, x1<x2, y1<y2)
     */
    static bool isValidNormalized(const NormalizedCoords& coords);
    
    /**
     * @brief Validate image coordinates
     * @param coords Coordinates to validate
     * @param imgWidth Image width in pixels
     * @param imgHeight Image height in pixels
     * @return true if valid (within bounds, x1<x2, y1<y2)
     */
    static bool isValidImage(const ImageCoords& coords, 
                             int imgWidth, 
                             int imgHeight);
    
    /**
     * @brief Validate canvas coordinates
     * @param coords Coordinates to validate
     * @return true if valid (x1<x2, y1<y2)
     */
    static bool isValidCanvas(const CanvasCoords& coords);
    
    /**
     * @brief Normalize coordinates (ensure x1<x2, y1<y2)
     * @param coords Coordinates to normalize (modified in place)
     */
    static void normalizeCoords(NormalizedCoords& coords);
    
    /**
     * @brief Clamp coordinates to valid bounds (0.0-1.0)
     * @param coords Coordinates to clamp (modified in place)
     */
    static void clampToBounds(NormalizedCoords& coords);
    
    /**
     * @brief Calculate aspect ratio from dimensions
     * @param width Image width
     * @param height Image height
     * @return Aspect ratio (width/height)
     */
    static double calculateAspectRatio(int width, int height);
    
    /**
     * @brief Validate image dimensions
     * @param width Image width
     * @param height Image height
     * @return true if dimensions are valid (both > 0)
     */
    static bool isValidImageDimensions(int width, int height);
    
    /**
     * @brief Validate double value (check for NaN and Infinity)
     * @param value Value to validate
     * @return true if value is finite and not NaN
     */
    static bool isValidDouble(double value);
    
    /**
     * @brief Validate QPointF (check both x and y)
     * @param point Point to validate
     * @return true if both coordinates are valid
     */
    static bool isValidPoint(const QPointF& point);
    
    /**
     * @brief Validate normalized coordinates (check all 4 values)
     * @param coords Coordinates to validate
     * @return true if all coordinates are valid
     */
    static bool isValidNormalizedCoords(const NormalizedCoords& coords);
};

} // namespace ocr_orc

#endif // COORDINATE_SYSTEM_H

