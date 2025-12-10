#ifndef MAIN_WINDOW_EVENT_HANDLERS_H
#define MAIN_WINDOW_EVENT_HANDLERS_H

#include <QtGui/QCloseEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>

namespace ocr_orc {

// Forward declarations
class MainWindow;
class DocumentState;
class Canvas;
class ControlPanelWidget;

/**
 * @brief Handles event handlers for MainWindow
 * 
 * Manages:
 * - Window close events
 * - Drag and drop events (PDF loading)
 */
class MainWindowEventHandlers {
public:
    MainWindowEventHandlers();
    ~MainWindowEventHandlers();
    
    /**
     * @brief Handle window close event
     * @param mainWindow Main window instance
     * @param event Close event
     */
    void handleCloseEvent(MainWindow* mainWindow, QCloseEvent* event);
    
    /**
     * @brief Handle drag enter event
     * @param mainWindow Main window instance
     * @param event Drag enter event
     */
    void handleDragEnterEvent(MainWindow* mainWindow, QDragEnterEvent* event);
    
    /**
     * @brief Handle drag move event
     * @param mainWindow Main window instance
     * @param event Drag move event
     */
    void handleDragMoveEvent(MainWindow* mainWindow, QDragMoveEvent* event);
    
    /**
     * @brief Handle drop event
     * @param mainWindow Main window instance
     * @param event Drop event
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param controlPanelWidget Control panel widget
     * @param callbacks Callback functions for UI updates
     */
    void handleDropEvent(MainWindow* mainWindow,
                        QDropEvent* event,
                        DocumentState* documentState,
                        Canvas* canvas,
                        ControlPanelWidget* controlPanelWidget,
                        const std::function<void()>& updateZoomLabel,
                        const std::function<void()>& updateUndoRedoButtons,
                        const std::function<void()>& updateRegionListBox,
                        const std::function<void()>& updateGroupListBox,
                        const std::function<void(const QString&)>& setFileLabel,
                        const std::function<void(const QString&, const QString&)>& showWarning);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_EVENT_HANDLERS_H

