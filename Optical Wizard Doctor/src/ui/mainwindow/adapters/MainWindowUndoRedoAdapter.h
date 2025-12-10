#ifndef MAIN_WINDOW_UNDO_REDO_ADAPTER_H
#define MAIN_WINDOW_UNDO_REDO_ADAPTER_H

#include <QtCore/QString>
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;

/**
 * @brief Unified adapter for undo/redo operations with UI refresh
 * 
 * Encapsulates the complete undo/redo pattern:
 * 1. Call DocumentState undo/redo
 * 2. Then: invalidate cache → update canvas → update lists → update toolbar buttons
 * 3. Selection clear/restoration per current policy
 * 
 * Ensures consistent refresh sequence and prevents partial refresh issues.
 */
class MainWindowUndoRedoAdapter {
public:
    // Callback types
    using CanUndoCallback = std::function<bool()>;
    using CanRedoCallback = std::function<bool()>;
    using UndoActionCallback = std::function<void()>;
    using RedoActionCallback = std::function<void()>;
    using InvalidateCacheCallback = std::function<void()>;
    using UpdateCanvasCallback = std::function<void()>;
    using UpdateRegionListBoxCallback = std::function<void()>;
    using UpdateGroupListBoxCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void(bool, bool)>;
    using ClearSelectionCallback = std::function<void()>;
    using ShowStatusMessageCallback = std::function<void(const QString&, int)>;
    
    MainWindowUndoRedoAdapter();
    ~MainWindowUndoRedoAdapter();
    
    /**
     * @brief Perform undo operation with full UI refresh
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks UI update callbacks
     * @return true if undo was performed, false if not available
     */
    bool performUndo(DocumentState* documentState,
                    Canvas* canvas,
                    const CanUndoCallback& canUndo,
                    const UndoActionCallback& undoAction,
                    const InvalidateCacheCallback& invalidateCache,
                    const UpdateCanvasCallback& updateCanvas,
                    const UpdateRegionListBoxCallback& updateRegionListBox,
                    const UpdateGroupListBoxCallback& updateGroupListBox,
                    const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                    const ClearSelectionCallback& clearSelection,
                    const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Perform redo operation with full UI refresh
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks UI update callbacks
     * @return true if redo was performed, false if not available
     */
    bool performRedo(DocumentState* documentState,
                    Canvas* canvas,
                    const CanRedoCallback& canRedo,
                    const RedoActionCallback& redoAction,
                    const InvalidateCacheCallback& invalidateCache,
                    const UpdateCanvasCallback& updateCanvas,
                    const UpdateRegionListBoxCallback& updateRegionListBox,
                    const UpdateGroupListBoxCallback& updateGroupListBox,
                    const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                    const ClearSelectionCallback& clearSelection,
                    const ShowStatusMessageCallback& showStatusMessage);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_UNDO_REDO_ADAPTER_H

