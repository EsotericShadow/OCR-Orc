#ifndef CANVAS_KEYBOARD_HANDLER_H
#define CANVAS_KEYBOARD_HANDLER_H

#include <QtGui/QKeyEvent>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <functional>
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"
#include "../CanvasMouseMode.h"  // Include before namespace to avoid double nesting

namespace ocr_orc {

/**
 * @brief Handles keyboard event processing for Canvas
 */
class CanvasKeyboardHandler {
public:
    // Callback types
    using UpdateCallback = std::function<void()>;
    using SetCursorCallback = std::function<void(Qt::CursorShape)>;
    using EmitSignalCallback = std::function<void(const QString&)>;
    using EmitStateChangedCallback = std::function<void()>;
    using DuplicateCallback = std::function<void()>;
    using ZoomInCallback = std::function<void()>;
    using ZoomOutCallback = std::function<void()>;
    using ZoomResetCallback = std::function<void()>;
    using SetModeCallback = std::function<void(MouseMode)>;
    
    CanvasKeyboardHandler();
    ~CanvasKeyboardHandler();
    
    void handleKeyPress(QKeyEvent* event,
                       DocumentState* documentState,
                       QSet<QString>& selectedRegions,
                       bool& isDragging,
                       QMap<QString, RegionData>& dragStartState,
                       bool& isResizing,
                       QString& resizingRegion,
                       QString& resizeHandle,
                       RegionData& resizeStartRegionData,
                       MouseMode& mouseMode,
                       const DuplicateCallback& duplicateSelectedRegions,
                       const ZoomInCallback& zoomIn,
                       const ZoomOutCallback& zoomOut,
                       const ZoomResetCallback& zoomReset,
                       const SetModeCallback& setMode,
                       const UpdateCallback& update,
                       const SetCursorCallback& setCursor,
                       const EmitSignalCallback& emitUndoRequested,
                       const EmitSignalCallback& emitRedoRequested,
                       const EmitStateChangedCallback& emitStateChanged);
};

} // namespace ocr_orc

#endif // CANVAS_KEYBOARD_HANDLER_H

