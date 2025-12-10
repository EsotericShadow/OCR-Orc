#ifndef MAIN_WINDOW_UI_SETUP_H
#define MAIN_WINDOW_UI_SETUP_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <functional>

namespace ocr_orc {
// Forward declarations for setupUI
class ToolbarWidget;
class ControlPanelWidget;
class SidePanelWidget;
}

namespace ocr_orc {

// Forward declarations
class MainWindow;
class Canvas;
class DocumentState;

/**
 * @brief Handles UI setup for MainWindow
 * 
 * Manages:
 * - Layout setup (central widget, main frame, canvas container)
 * - Menu bar setup
 * - Status bar setup
 * - Canvas setup (widget creation, signal connections, keyboard shortcuts)
 */
class MainWindowUISetup {
public:
    MainWindowUISetup();
    ~MainWindowUISetup();
    
    /**
     * @brief Setup the basic layout structure
     * @param mainWindow Main window instance
     * @param centralWidget Output: Created central widget
     * @param mainLayout Output: Created main layout
     * @param mainFrame Output: Created main frame
     * @param canvasContainer Output: Created canvas container
     */
    void setupLayout(MainWindow* mainWindow,
                    ::QWidget*& centralWidget,
                    ::QVBoxLayout*& mainLayout,
                    ::QFrame*& mainFrame,
                    ::QWidget*& canvasContainer);
    
    /**
     * @brief Setup the menu bar
     * @param mainWindow Main window instance
     * @param fileMenu Output: Created file menu
     * @param exitAction Output: Created exit action
     */
    void setupMenuBar(MainWindow* mainWindow,
                     ::QMenu*& fileMenu,
                     ::QAction*& exitAction);
    
    /**
     * @brief Setup the status bar
     * @param mainWindow Main window instance
     */
    void setupStatusBar(MainWindow* mainWindow);
    
    /**
     * @brief Set window properties
     * @param mainWindow Main window instance
     */
    void setWindowProperties(MainWindow* mainWindow);
    
    /**
     * @brief Setup the complete UI
     * @param mainWindow Main window instance
     * @param centralWidget Output: Created central widget
     * @param mainLayout Output: Created main layout
     * @param mainFrame Output: Created main frame
     * @param canvasContainer Output: Created canvas container
     * @param toolbarWidget Output: Created toolbar widget
     * @param controlPanelWidget Output: Created control panel widget
     * @param sidePanelWidget Output: Created side panel widget
     * @param canvas Output: Created canvas widget
     * @param canvasStatusLabel Output: Created canvas status label
     * @param documentState Document state
     * @param connectWidgets Callback to connect widgets
     */
    void setupUI(MainWindow* mainWindow,
                ::QWidget*& centralWidget,
                ::QVBoxLayout*& mainLayout,
                ::QFrame*& mainFrame,
                ::QWidget*& canvasContainer,
                ocr_orc::ToolbarWidget*& toolbarWidget,
                ocr_orc::ControlPanelWidget*& controlPanelWidget,
                ocr_orc::SidePanelWidget*& sidePanelWidget,
                Canvas*& canvas,
                ::QLabel*& canvasStatusLabel,
                DocumentState* documentState,
                const std::function<void()>& connectWidgets);
    
    /**
     * @brief Setup the canvas widget
     * @param mainWindow Main window instance
     * @param canvasContainer Canvas container widget
     * @param documentState Document state
     * @param canvas Output: Created canvas widget
     * @param canvasStatusLabel Output: Created canvas status label
     */
    void setupCanvas(MainWindow* mainWindow,
                    ::QWidget* canvasContainer,
                    DocumentState* documentState,
                    Canvas*& canvas,
                    ::QLabel*& canvasStatusLabel);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_UI_SETUP_H

