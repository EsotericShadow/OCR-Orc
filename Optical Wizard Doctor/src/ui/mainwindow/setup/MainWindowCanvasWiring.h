#ifndef MAIN_WINDOW_CANVAS_WIRING_H
#define MAIN_WINDOW_CANVAS_WIRING_H

#include <QtCore/QString>
#include <functional>

namespace ocr_orc {

// Forward declarations
class MainWindow;
class Canvas;
class DocumentState;
class ToolbarWidget;

/**
 * @brief Handles canvas-specific signal/slot connections
 * 
 * Manages all connections from Canvas signals to MainWindow slots.
 * Separates canvas wiring from general widget connections.
 */
class MainWindowCanvasWiring {
public:
    // Callback types
    using ShowEditNameDialogCallback = std::function<QString(const QString&)>;
    using ShowChangeColorDialogCallback = std::function<QString(const QString&)>;
    using UpdateRegionListBoxCallback = std::function<void()>;
    using UpdateGroupListBoxCallback = std::function<void()>;
    using UpdateCanvasCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void()>;
    using ShowStatusMessageCallback = std::function<void(const QString&, int)>;
    using ClearSelectionCallback = std::function<void()>;
    using SelectRegionCallback = std::function<void(const QString&)>;
    using GetRegionCallback = std::function<struct RegionData(const QString&)>;
    using SaveStateCallback = std::function<void()>;
    using ChangeRegionColorCallback = std::function<bool(const QString&, const QString&)>;
    using RenameRegionCallback = std::function<bool(const QString&, const QString&)>;
    using InvalidateCacheCallback = std::function<void()>;
    
    MainWindowCanvasWiring();
    ~MainWindowCanvasWiring();
    
    /**
     * @brief Connect all canvas signals to MainWindow slots
     * @param mainWindow Main window instance
     * @param canvas Canvas widget
     * @param documentState Document state
     * @param callbacks Callback functions for UI updates
     */
    void connectCanvasSignals(MainWindow* mainWindow,
                             Canvas* canvas,
                             DocumentState* documentState,
                             const ShowEditNameDialogCallback& showEditNameDialog,
                             const ShowChangeColorDialogCallback& showChangeColorDialog,
                             const UpdateRegionListBoxCallback& updateRegionListBox,
                             const UpdateGroupListBoxCallback& updateGroupListBox,
                             const UpdateCanvasCallback& updateCanvas,
                             const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                             const ShowStatusMessageCallback& showStatusMessage,
                             const ClearSelectionCallback& clearSelection,
                             const SelectRegionCallback& selectRegion,
                             const GetRegionCallback& getRegion,
                             const SaveStateCallback& saveState,
                             const ChangeRegionColorCallback& changeRegionColor,
                             const RenameRegionCallback& renameRegion,
                             const InvalidateCacheCallback& invalidateCache);
    
    /**
     * @brief Setup keyboard shortcuts for canvas (undo/redo actions)
     * @param mainWindow Main window instance
     * @param canvas Canvas widget
     */
    void setupCanvasShortcuts(MainWindow* mainWindow, Canvas* canvas);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_CANVAS_WIRING_H

