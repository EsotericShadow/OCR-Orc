#ifndef CANVAS_MOUSE_MODE_H
#define CANVAS_MOUSE_MODE_H

namespace ocr_orc {

/**
 * @brief Mouse mode enum for Canvas operations
 * 
 * This enum is shared across Canvas and its input handlers.
 */
enum MouseMode {
    Create,  // Create new regions by dragging
    Select   // Select and manipulate existing regions
};

} // namespace ocr_orc

#endif // CANVAS_MOUSE_MODE_H

