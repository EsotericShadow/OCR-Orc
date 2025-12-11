#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QScrollArea>
#include <QtGui/QAction>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include "../models/DocumentState.h"
#include "canvas/Canvas.h"
#include "components/widgets/ToolbarWidget.h"
#include "components/widgets/ControlPanelWidget.h"
#include "components/widgets/SidePanelWidget.h"
#include "mainwindow/operations/file/MainWindowFileOperations.h"
#include "mainwindow/operations/region/MainWindowRegionOperations.h"
#include "mainwindow/shortcuts/MainWindowKeyboardShortcuts.h"
#include "mainwindow/operations/group/MainWindowGroupOperations.h"
#include "mainwindow/operations/undo/MainWindowUndoRedo.h"
#include "mainwindow/ui/MainWindowUIUpdates.h"
#include "mainwindow/ui/MainWindowToolbarAdapter.h"
#include "mainwindow/adapters/MainWindowUndoRedoAdapter.h"
#include "mainwindow/operations/selection/MainWindowSelectionOperations.h"
#include "mainwindow/dialogs/MainWindowDialogsController.h"
#include "mainwindow/setup/MainWindowUISetup.h"
#include "mainwindow/setup/MainWindowWidgetConnections.h"
#include "mainwindow/setup/MainWindowCanvasWiring.h"
#include "mainwindow/events/MainWindowEventHandlers.h"
#include "mainwindow/handlers/actions/MainWindowActionHandlers.h"
#include "mainwindow/handlers/regions/MainWindowRegionGroupHandlers.h"
#include "mainwindow/handlers/regions/MainWindowRegionEditorHandlers.h"
#include "mainwindow/handlers/view/MainWindowViewHandlers.h"
#include "ThemeManager.h"

namespace ocr_orc {

/**
 * @brief Main application window
 * 
 * The main window contains:
 * - Menu bar (File, Edit, View, Help)
 * - Toolbar (Mode, Zoom, Groups, Undo/Redo)
 * - Control panel (File controls, region input)
 * - Canvas area (PDF display and region rendering)
 * - Side panel (Regions list, Groups list, Info display)
 * - Status bar (Status messages)
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
    friend class MainWindowKeyboardShortcuts;
    friend class MainWindowUISetup;
    friend class MainWindowEventHandlers;
    friend class MainWindowWidgetConnections;
    friend class MainWindowCanvasWiring;
    friend class MainWindowActionHandlers;
    friend class MainWindowRegionGroupHandlers;
    friend class MainWindowRegionEditorHandlers;
    friend class MainWindowViewHandlers;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    /**
     * @brief Handle window close event
     * @param event Close event
     */
    void closeEvent(QCloseEvent *event) override;
    
    /**
     * @brief Handle drag enter event (for drag and drop PDF loading)
     * @param event Drag enter event
     */
    void dragEnterEvent(QDragEnterEvent *event) override;
    
    /**
     * @brief Handle drag move event (for visual feedback)
     * @param event Drag move event
     */
    void dragMoveEvent(QDragMoveEvent *event) override;
    
    /**
     * @brief Handle drop event (load PDF when dropped)
     * @param event Drop event
     */
    void dropEvent(QDropEvent *event) override;

public slots:
    /**
     * @brief Update the zoom percentage label with provided zoom level
     * @param zoomLevel The zoom level (0.0-1.0+)
     */
    void updateZoomLabel(double zoomLevel);
    
private slots:
    /**
     * @brief Handle File > Exit action
     */
    void onExit();
    
    /**
     * @brief Handle Load PDF button click
     */
    void onLoadPdf();
    
    /**
     * @brief Handle export coordinates button click
     */
    void onExportCoordinates();
    
    /**
     * @brief Handle load coordinates button click
     */
    void onLoadCoordinates();
    
    /**
     * @brief Handle export mask button click
     */
    void onExportMask();
    
    /**
     * @brief Handle Create mode button selection
     */
    void onCreateModeSelected();
    
    /**
     * @brief Handle Select mode button selection
     */
    void onSelectModeSelected();
    
    /**
     * @brief Handle zoom in button click
     */
    void onZoomIn();
    
    /**
     * @brief Handle zoom out button click
     */
    void onZoomOut();
    
    /**
     * @brief Handle zoom reset button click
     */
    void onZoomReset();
    
    /**
     * @brief Handle region creation completion
     * @param regionName Name of the created region
     */
    void onRegionCreated(const QString& regionName);
    
    /**
     * @brief Handle region creation request (mouse release during creation)
     * Completes region creation with name/color/group from control panel
     */
    void onRegionCreationRequested();
    
    /**
     * @brief Handle regions duplication
     * @param duplicatedNames List of names of newly created duplicate regions
     */
    void onRegionsDuplicated(const QList<QString>& duplicatedNames);
    
    /**
     * @brief Handle Create Group button click
     */
    void onCreateGroup();
    
    /**
     * @brief Handle Add to Group button click
     */
    void onAddToGroup();
    
    /**
     * @brief Handle Remove from Group button click
     */
    void onRemoveFromGroup();
    
    /**
     * @brief Handle Delete Group button click
     */
    void onDeleteGroup();
    
    /**
     * @brief Handle Edit Selected button click
     * Opens name editing dialog for selected region
     */
    void onEditSelected();
    
    /**
     * @brief Handle region name change from editor
     * @param oldName Original region name
     * @param newName New region name
     */
    void onRegionNameChanged(const QString& oldName, const QString& newName);
    
    /**
     * @brief Handle region color change from editor
     * @param regionName Name of the region
     * @param newColor New color value
     */
    void onRegionColorChanged(const QString& regionName, const QString& newColor);
    
    /**
     * @brief Handle region group change from editor
     * @param regionName Name of the region
     * @param newGroup New group name (empty string to remove from group)
     */
    void onRegionGroupChanged(const QString& regionName, const QString& newGroup);
    
    /**
     * @brief Delete selected regions
     */
    void onDeleteSelected();
    
    /**
     * @brief Select all regions
     */
    void onSelectAll();
    
    /**
     * @brief Invert selection (select all unselected, deselect all selected)
     */
    void onInvertSelection();
    
    /**
     * @brief Show help window
     */
    void showHelp();
    
    /**
     * @brief Show preferences dialog
     */
    void showPreferences();
    
    /**
     * @brief Set theme to Light
     */
    void onSetThemeLight();
    
    /**
     * @brief Set theme to Dark
     */
    void onSetThemeDark();
    
    /**
     * @brief Set theme to System
     */
    void onSetThemeSystem();
    
    
    /**
     * @brief Show name editing dialog
     * @param currentName Current region name to edit
     * @return New name if user accepted, empty string if cancelled
     */
    QString showEditNameDialog(const QString& currentName);
    
    /**
     * @brief Show color editing dialog
     * @param regionName Region to change color for
     * @return New color name if user accepted, empty string if cancelled
     */
    QString showChangeColorDialog(const QString& regionName);
    
    /**
     * @brief Generate auto-incremented region name
     * @return Generated region name
     */
    QString generateRegionName() const;
    
    /**
     * @brief Auto-increment region name in input field if pattern detected
     */
    void autoIncrementRegionName();
    
    /**
     * @brief Update the zoom percentage label (reads from canvas)
     */
    void updateZoomLabel();
    
    /**
     * @brief Handle undo button click
     */
    void onUndo();
    
    /**
     * @brief Handle redo button click
     */
    void onRedo();
    
    /**
     * @brief Update undo/redo button states based on stack availability
     */
    void updateUndoRedoButtons();
    
    /**
     * @brief Update region listbox with current regions
     */
    void updateRegionListBox();
    
    /**
     * @brief Update group listbox with current groups
     */
    void updateGroupListBox();
    
    /**
     * @brief Get selected regions from canvas or region listbox
     * @return Set of selected region names
     */
    QSet<QString> getSelectedRegions() const;

private:
    /**
     * @brief Set up the user interface
     */
    void setupUI();
    
    /**
     * @brief Set up the menu bar
     */
    void setupMenuBar();
    
    /**
     * @brief Set up the status bar
     */
    void setupStatusBar();
    
    /**
     * @brief Set up the basic layout structure
     */
    void setupLayout();
    
    /**
     * @brief Set up all keyboard shortcuts
     */
    void setupKeyboardShortcuts();
    
    /**
     * @brief Set up the canvas area with status label
     */
    void setupCanvas();
    
    /**
     * @brief Connect signals/slots between widgets
     */
    void connectWidgets();
    
    /**
     * @brief Set window properties (title, size, etc.)
     */
    void setWindowProperties();
    
    /**
     * @brief Initialize theme system and apply theme
     */
    void initializeTheme();
    
    /**
     * @brief Apply current theme to all widgets
     */
    void applyTheme();

    // UI Components - Layout
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    
    // Frames
    QFrame* mainFrame;
    QWidget* canvasContainer;
    
    // Modular widgets
    ToolbarWidget* toolbarWidget;
    ControlPanelWidget* controlPanelWidget;
    SidePanelWidget* sidePanelWidget;
    
    // Canvas components
    QLabel* canvasStatusLabel;
    Canvas* canvas;
    
    // Menu bar
    QMenu* fileMenu;
    QAction* exitAction;
    
    // Application state
    DocumentState* documentState;
    
    // Guard flags to prevent double execution
    bool isCreatingGroup;
    
    // Modular operations
    MainWindowFileOperations* fileOperations;
    MainWindowRegionOperations* regionOperations;
    MainWindowGroupOperations* groupOperations;
    MainWindowUndoRedo* undoRedo;
    MainWindowUndoRedoAdapter* undoRedoAdapter;
    MainWindowUIUpdates* uiUpdates;
    MainWindowToolbarAdapter* toolbarAdapter;
    MainWindowSelectionOperations* selectionOperations;
    MainWindowKeyboardShortcuts* keyboardShortcuts;
    MainWindowDialogsController* dialogsController;
    MainWindowUISetup* uiSetup;
    MainWindowWidgetConnections* widgetConnections;
    MainWindowCanvasWiring* canvasWiring;
    MainWindowEventHandlers* eventHandlers;
    MainWindowActionHandlers* actionHandlers;
    MainWindowRegionGroupHandlers* regionGroupHandlers;
    MainWindowRegionEditorHandlers* regionEditorHandlers;
    MainWindowViewHandlers* viewHandlers;
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_H

