#include "CanvasMouseHandler.h"

namespace ocr_orc {

CanvasMouseHandler::CanvasMouseHandler() = default;
CanvasMouseHandler::~CanvasMouseHandler() = default;

void CanvasMouseHandler::resetAllFlags(bool& isCreating,
                                      bool& isBoxSelecting,
                                      bool& isDragging,
                                      bool& isResizing,
                                      bool& isRotating,
                                      bool& isPanning) {
    isCreating = false;
    isBoxSelecting = false;
    isDragging = false;
    isResizing = false;
    isRotating = false;
    isPanning = false;
}

} // namespace ocr_orc


