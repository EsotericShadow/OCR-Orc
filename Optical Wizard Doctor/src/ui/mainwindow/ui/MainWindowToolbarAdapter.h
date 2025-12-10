#ifndef MAIN_WINDOW_TOOLBAR_ADAPTER_H
#define MAIN_WINDOW_TOOLBAR_ADAPTER_H

#include <QtCore/QString>
#include <functional>

namespace ocr_orc {

// Forward declarations
class ToolbarWidget;
class DocumentState;
class Canvas;

/**
 * @brief Adapter for toolbar button state and zoom label updates
 * 
 * Centralizes logic for updating:
 * - Undo/redo button states (based on DocumentState undo/redo stack)
 * - Zoom label (based on Canvas zoom level)
 * - Rotate button state (based on Canvas rotate mode)
 * 
 * ToolbarWidget remains a thin view with setters.
 */
class MainWindowToolbarAdapter {
public:
    // Callback types
    using GetCanUndoCallback = std::function<bool()>;
    using GetCanRedoCallback = std::function<bool()>;
    using GetZoomCallback = std::function<double()>;
    using GetRotateModeCallback = std::function<bool()>;
    using UpdateZoomLabelCallback = std::function<void(double)>;
    using UpdateUndoRedoButtonsCallback = std::function<void(bool, bool)>;
    using UpdateRotateButtonCallback = std::function<void(bool)>;
    
    MainWindowToolbarAdapter();
    ~MainWindowToolbarAdapter();
    
    /**
     * @brief Update undo/redo button states
     * @param documentState Document state (for checking undo/redo availability)
     * @param callbacks Callback functions
     */
    void updateUndoRedoButtons(DocumentState* documentState,
                              const GetCanUndoCallback& getCanUndo,
                              const GetCanRedoCallback& getCanRedo,
                              const UpdateUndoRedoButtonsCallback& updateButtons);
    
    /**
     * @brief Update zoom label
     * @param canvas Canvas widget (for getting zoom level)
     * @param callbacks Callback functions
     */
    void updateZoomLabel(Canvas* canvas,
                        const GetZoomCallback& getZoom,
                        const UpdateZoomLabelCallback& updateLabel);
    
    /**
     * @brief Update rotate button state
     * @param canvas Canvas widget (for getting rotate mode)
     * @param callbacks Callback functions
     */
    void updateRotateButton(Canvas* canvas,
                          const GetRotateModeCallback& getRotateMode,
                          const UpdateRotateButtonCallback& updateButton);
    
    /**
     * @brief Update all toolbar states (undo/redo, zoom, rotate)
     * Convenience method for full refresh
     */
    void updateAll(DocumentState* documentState,
                  Canvas* canvas,
                  const GetCanUndoCallback& getCanUndo,
                  const GetCanRedoCallback& getCanRedo,
                  const GetZoomCallback& getZoom,
                  const GetRotateModeCallback& getRotateMode,
                  const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                  const UpdateZoomLabelCallback& updateZoomLabel,
                  const UpdateRotateButtonCallback& updateRotateButton);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_TOOLBAR_ADAPTER_H

