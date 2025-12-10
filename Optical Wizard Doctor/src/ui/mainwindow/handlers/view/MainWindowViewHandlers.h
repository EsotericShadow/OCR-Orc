#ifndef MAIN_WINDOW_VIEW_HANDLERS_H
#define MAIN_WINDOW_VIEW_HANDLERS_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <functional>

namespace ocr_orc {

// Forward declarations
class MainWindow;
class DocumentState;
class Canvas;
class ToolbarWidget;
class SidePanelWidget;

/**
 * @brief Handles UI state and view operation handlers for MainWindow
 * 
 * Manages:
 * - Mode selection (Create/Select)
 * - Zoom operations
 * - Undo/Redo operations
 * - UI updates (zoom label, region list, group list)
 * - Selection operations
 */
class MainWindowViewHandlers {
public:
    MainWindowViewHandlers();
    ~MainWindowViewHandlers();
    
    // Mode operations
    void onCreateModeSelected(Canvas* canvas);
    void onSelectModeSelected(Canvas* canvas);
    
    // Zoom operations
    void onZoomIn(Canvas* canvas, const std::function<void()>& updateZoomLabel);
    void onZoomOut(Canvas* canvas, const std::function<void()>& updateZoomLabel);
    void onZoomReset(Canvas* canvas, const std::function<void()>& updateZoomLabel);
    void updateZoomLabel(Canvas* canvas,
                        ToolbarWidget* toolbarWidget,
                        const std::function<double()>& getZoom,
                        const std::function<void(double)>& setZoomLabel);
    
    // Undo/Redo operations
    void onUndo(MainWindow* mainWindow,
                DocumentState* documentState,
                Canvas* canvas,
                ToolbarWidget* toolbarWidget,
                const std::function<bool()>& canUndo,
                const std::function<void()>& undoAction,
                const std::function<void(bool, bool)>& updateButtons,
                const std::function<void()>& updateRegionListBox,
                const std::function<void()>& updateGroupListBox,
                const std::function<void()>& updateCanvas,
                const std::function<void()>& clearSelection,
                const std::function<void(const QString&, int)>& showStatusMessage);
    
    void onRedo(MainWindow* mainWindow,
                DocumentState* documentState,
                Canvas* canvas,
                ToolbarWidget* toolbarWidget,
                const std::function<bool()>& canRedo,
                const std::function<void()>& redoAction,
                const std::function<void(bool, bool)>& updateButtons,
                const std::function<void()>& updateRegionListBox,
                const std::function<void()>& updateGroupListBox,
                const std::function<void()>& updateCanvas,
                const std::function<void()>& clearSelection,
                const std::function<void(const QString&, int)>& showStatusMessage);
    
    void updateUndoRedoButtons(DocumentState* documentState,
                               ToolbarWidget* toolbarWidget,
                               const std::function<bool()>& canUndo,
                               const std::function<bool()>& canRedo,
                               const std::function<void(bool, bool)>& updateButtons);
    
    // UI update operations
    void updateRegionListBox(MainWindow* mainWindow,
                            DocumentState* documentState,
                            Canvas* canvas,
                            SidePanelWidget* sidePanelWidget,
                            const std::function<QList<QString>()>& getAllRegionNames,
                            const std::function<QSet<QString>()>& getSelectedRegions,
                            const std::function<struct RegionData(const QString&)>& getRegion,
                            const std::function<void(const QList<QString>&, const QSet<QString>&, const QMap<QString, QString>&)>& updateRegionListBox);
    
    void updateGroupListBox(MainWindow* mainWindow,
                           DocumentState* documentState,
                           SidePanelWidget* sidePanelWidget,
                           const std::function<QList<QString>()>& getGroupNames,
                           const std::function<struct GroupData(const QString&)>& getGroup,
                           const std::function<void(const QList<QString>&, const QMap<QString, int>&)>& updateGroupListBox);
    
    // Selection operations
    void onSelectAll(MainWindow* mainWindow,
                    DocumentState* documentState,
                    Canvas* canvas,
                    const std::function<QList<QString>()>& getAllRegionNames,
                    const std::function<void(const QSet<QString>&)>& setSelectedRegions,
                    const std::function<void()>& updateCanvas,
                    const std::function<void()>& updateRegionListBox);
    
    void onInvertSelection(MainWindow* mainWindow,
                         DocumentState* documentState,
                         Canvas* canvas,
                         const std::function<QList<QString>()>& getAllRegionNames,
                         const std::function<QSet<QString>()>& getSelectedRegions,
                         const std::function<void(const QSet<QString>&)>& setSelectedRegions,
                         const std::function<void()>& updateCanvas,
                         const std::function<void()>& updateRegionListBox);
    
    // Help
    void showHelp(MainWindow* mainWindow);
    
    // Keyboard shortcuts
    void setupKeyboardShortcuts(MainWindow* mainWindow);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_VIEW_HANDLERS_H

