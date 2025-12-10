#ifndef MAIN_WINDOW_UNDO_REDO_H
#define MAIN_WINDOW_UNDO_REDO_H

#include <QtCore/QString>
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;

/**
 * @brief Handles undo/redo operations for MainWindow
 * 
 * Manages:
 * - Undo operations with state synchronization
 * - Redo operations with state synchronization
 * - UI button state updates
 */
class MainWindowUndoRedo {
public:
    // Callback types for UI updates
    using CanUndoCallback = std::function<bool()>;
    using CanRedoCallback = std::function<bool()>;
    using UndoActionCallback = std::function<void()>;
    using RedoActionCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void(bool, bool)>;
    using UpdateRegionListBoxCallback = std::function<void()>;
    using UpdateGroupListBoxCallback = std::function<void()>;
    using UpdateCanvasCallback = std::function<void()>;
    using ClearSelectionCallback = std::function<void()>;
    using ShowStatusMessageCallback = std::function<void(const QString&, int)>;
    
    MainWindowUndoRedo();
    ~MainWindowUndoRedo();
    
    /**
     * @brief Perform undo operation
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void undo(DocumentState* documentState,
              Canvas* canvas,
              const CanUndoCallback& canUndo,
              const UndoActionCallback& undoAction,
              const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
              const UpdateRegionListBoxCallback& updateRegionListBox,
              const UpdateGroupListBoxCallback& updateGroupListBox,
              const UpdateCanvasCallback& updateCanvas,
              const ClearSelectionCallback& clearSelection,
              const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Perform redo operation
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void redo(DocumentState* documentState,
              Canvas* canvas,
              const CanRedoCallback& canRedo,
              const RedoActionCallback& redoAction,
              const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
              const UpdateRegionListBoxCallback& updateRegionListBox,
              const UpdateGroupListBoxCallback& updateGroupListBox,
              const UpdateCanvasCallback& updateCanvas,
              const ClearSelectionCallback& clearSelection,
              const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Update undo/redo button states
     * @param documentState Document state
     * @param callbacks Callback functions
     */
    void updateButtons(DocumentState* documentState,
                      const CanUndoCallback& canUndo,
                      const CanRedoCallback& canRedo,
                      const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_UNDO_REDO_H

