#ifndef MAIN_WINDOW_WIDGET_CONNECTIONS_H
#define MAIN_WINDOW_WIDGET_CONNECTIONS_H

namespace ocr_orc {

// Forward declarations
class MainWindow;
class ToolbarWidget;
class ControlPanelWidget;
class SidePanelWidget;
class Canvas;
class DocumentState;

/**
 * @brief Handles widget signal/slot connections for MainWindow
 * 
 * Manages all connections between widgets and MainWindow slots.
 */
class MainWindowWidgetConnections {
public:
    MainWindowWidgetConnections();
    ~MainWindowWidgetConnections();
    
    /**
     * @brief Connect all widget signals to MainWindow slots
     * @param mainWindow Main window instance
     * @param toolbarWidget Toolbar widget
     * @param controlPanelWidget Control panel widget
     * @param sidePanelWidget Side panel widget
     * @param canvas Canvas widget
     * @param documentState Document state
     */
    void connectWidgets(MainWindow* mainWindow,
                       ToolbarWidget* toolbarWidget,
                       ControlPanelWidget* controlPanelWidget,
                       SidePanelWidget* sidePanelWidget,
                       Canvas* canvas,
                       DocumentState* documentState);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_WIDGET_CONNECTIONS_H

