#ifndef MASK_GENERATOR_H
#define MASK_GENERATOR_H

#include "../models/DocumentState.h"
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Mask image generator for OCR preprocessing
 * 
 * Generates black/white mask images where:
 * - Background is black (everything hidden)
 * - Regions are white rectangles (visible for OCR)
 * 
 * Uses IMAGE coordinates (pixels), not canvas or normalized.
 */
class MaskGenerator {
public:
    /**
     * @brief Generate mask image from DocumentState
     * @param state DocumentState containing regions and image
     * @param filePath Path to output mask image file
     * @throws std::runtime_error if generation fails
     */
    static void generate(const DocumentState& state, const QString& filePath);
    
private:
    /**
     * @brief Create mask image with white rectangles for regions
     * @param state DocumentState containing regions and image
     * @return QImage mask (black background, white rectangles)
     */
    static QImage createMaskImage(const DocumentState& state);
    
    /**
     * @brief Validate and clamp coordinates to image bounds
     * @param x1 Left coordinate (output)
     * @param y1 Top coordinate (output)
     * @param x2 Right coordinate (output)
     * @param y2 Bottom coordinate (output)
     * @param width Image width
     * @param height Image height
     */
    static void validateAndClampCoordinates(int& x1, int& y1, int& x2, int& y2,
                                            int width, int height);
};

} // namespace ocr_orc

#endif // MASK_GENERATOR_H

