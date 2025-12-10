#include "MainWindowUISetup.h"
#include "../../MainWindow.h"
#include "../../canvas/Canvas.h"
#include "../../components/widgets/ToolbarWidget.h"
#include "../../components/widgets/ControlPanelWidget.h"
#include "../../components/widgets/SidePanelWidget.h"
#include "../../../models/DocumentState.h"
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
#include <QtCore/Qt>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtGui/QIcon>
#include <QtWidgets/QMessageBox>
#include <functional>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

MainWindowUISetup::MainWindowUISetup() {
}

MainWindowUISetup::~MainWindowUISetup() {
}

void MainWindowUISetup::setupLayout(MainWindow* mainWindow,
                                   ::QWidget*& centralWidget,
                                   ::QVBoxLayout*& mainLayout,
                                   ::QFrame*& mainFrame,
                                   ::QWidget*& canvasContainer) {
    if (!mainWindow) {
        return;
    }
    
    // Create central widget
    centralWidget = new QWidget(mainWindow);
    mainWindow->setCentralWidget(centralWidget);
    
    // Create main vertical layout
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create main frame (contains canvas and side panel)
    mainFrame = new QFrame(centralWidget);
    mainFrame->setFrameShape(QFrame::NoFrame);
    
    QHBoxLayout* mainFrameLayout = new QHBoxLayout(mainFrame);
    mainFrameLayout->setContentsMargins(0, 0, 0, 0);
    mainFrameLayout->setSpacing(0);
    
    // Create canvas container (will be populated in setupCanvas)
    canvasContainer = new QWidget(mainFrame);
    canvasContainer->setMinimumWidth(400);
    canvasContainer->setStyleSheet("background-color: #d0d0d0; color: #000000;");
    mainFrameLayout->addWidget(canvasContainer, 1);  // Expandable
    
    // Note: mainFrame will be added to mainLayout in setupUI() after toolbar and control panel
}

void MainWindowUISetup::setupMenuBar(MainWindow* mainWindow,
                                    ::QMenu*& fileMenu,
                                    ::QAction*& exitAction) {
    if (!mainWindow) {
        return;
    }
    
    // File menu
    fileMenu = mainWindow->menuBar()->addMenu("&File");
    
    QAction* openPdfAction = fileMenu->addAction("&Open PDF...");
    openPdfAction->setShortcut(QKeySequence::Open);
    QObject::connect(openPdfAction, SIGNAL(triggered()), mainWindow, SLOT(onLoadPdf()));
    
    QAction* loadCoordinatesAction = fileMenu->addAction("&Load Coordinates...");
    loadCoordinatesAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    QObject::connect(loadCoordinatesAction, SIGNAL(triggered()), mainWindow, SLOT(onLoadCoordinates()));
    
    fileMenu->addSeparator();
    
    QAction* exportCoordinatesAction = fileMenu->addAction("&Export Coordinates...");
    exportCoordinatesAction->setShortcut(QKeySequence::Save);
    QObject::connect(exportCoordinatesAction, SIGNAL(triggered()), mainWindow, SLOT(onExportCoordinates()));
    
    QAction* exportMaskAction = fileMenu->addAction("Export &Mask...");
    QObject::connect(exportMaskAction, SIGNAL(triggered()), mainWindow, SLOT(onExportMask()));
    
    fileMenu->addSeparator();
    
    // Exit action
    exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    QObject::connect(exitAction, SIGNAL(triggered()), mainWindow, SLOT(onExit()));
    
    // Edit menu
    QMenu* editMenu = mainWindow->menuBar()->addMenu("&Edit");
    
    QAction* undoAction = editMenu->addAction("&Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    QObject::connect(undoAction, SIGNAL(triggered()), mainWindow, SLOT(onUndo()));
    
    QAction* redoAction = editMenu->addAction("&Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    QObject::connect(redoAction, SIGNAL(triggered()), mainWindow, SLOT(onRedo()));
    
    editMenu->addSeparator();
    
    QAction* cutAction = editMenu->addAction("Cu&t");
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setEnabled(false); // Not implemented yet
    
    QAction* copyAction = editMenu->addAction("&Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(false); // Not implemented yet
    
    QAction* pasteAction = editMenu->addAction("&Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setEnabled(false); // Not implemented yet
    
    QAction* deleteAction = editMenu->addAction("&Delete");
    deleteAction->setShortcut(QKeySequence::Delete);
    QObject::connect(deleteAction, SIGNAL(triggered()), mainWindow, SLOT(onDeleteSelected()));
    
    editMenu->addSeparator();
    
    QAction* selectAllAction = editMenu->addAction("Select &All");
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    QObject::connect(selectAllAction, SIGNAL(triggered()), mainWindow, SLOT(onSelectAll()));
    
    QAction* invertSelectionAction = editMenu->addAction("&Invert Selection");
    QObject::connect(invertSelectionAction, SIGNAL(triggered()), mainWindow, SLOT(onInvertSelection()));
    
    // Selection menu
    QMenu* selectionMenu = mainWindow->menuBar()->addMenu("&Selection");
    
    QAction* selectAllAction2 = selectionMenu->addAction("Select &All");
    selectAllAction2->setShortcut(QKeySequence::SelectAll);
    QObject::connect(selectAllAction2, SIGNAL(triggered()), mainWindow, SLOT(onSelectAll()));
    
    QAction* invertSelectionAction2 = selectionMenu->addAction("&Invert Selection");
    QObject::connect(invertSelectionAction2, SIGNAL(triggered()), mainWindow, SLOT(onInvertSelection()));
    
    QAction* clearSelectionAction = selectionMenu->addAction("&Clear Selection");
    clearSelectionAction->setShortcut(QKeySequence("Esc"));
    if (mainWindow->canvas) {
        QObject::connect(clearSelectionAction, &QAction::triggered, mainWindow->canvas, &Canvas::clearSelection);
    }
    
    selectionMenu->addSeparator();
    
    QAction* createGroupAction = selectionMenu->addAction("&Create Group...");
    createGroupAction->setShortcut(QKeySequence("Ctrl+G"));
    QObject::connect(createGroupAction, SIGNAL(triggered()), mainWindow, SLOT(onCreateGroup()));
    
    QAction* addToGroupAction = selectionMenu->addAction("&Add to Group...");
    QObject::connect(addToGroupAction, SIGNAL(triggered()), mainWindow, SLOT(onAddToGroup()));
    
    QAction* removeFromGroupAction = selectionMenu->addAction("&Remove from Group");
    removeFromGroupAction->setShortcut(QKeySequence("Ctrl+Shift+G"));
    QObject::connect(removeFromGroupAction, SIGNAL(triggered()), mainWindow, SLOT(onRemoveFromGroup()));
    
    QAction* deleteGroupAction = selectionMenu->addAction("Delete &Group...");
    QObject::connect(deleteGroupAction, SIGNAL(triggered()), mainWindow, SLOT(onDeleteGroup()));
    
    // View menu
    QMenu* viewMenu = mainWindow->menuBar()->addMenu("&View");
    
    QAction* zoomInAction = viewMenu->addAction("Zoom &In");
    zoomInAction->setShortcut(QKeySequence("Ctrl+="));
    QObject::connect(zoomInAction, SIGNAL(triggered()), mainWindow, SLOT(onZoomIn()));
    
    QAction* zoomOutAction = viewMenu->addAction("Zoom &Out");
    zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
    QObject::connect(zoomOutAction, SIGNAL(triggered()), mainWindow, SLOT(onZoomOut()));
    
    QAction* zoomResetAction = viewMenu->addAction("&Reset Zoom");
    zoomResetAction->setShortcut(QKeySequence("Ctrl+0"));
    QObject::connect(zoomResetAction, SIGNAL(triggered()), mainWindow, SLOT(onZoomReset()));
    
    viewMenu->addSeparator();
    
    QAction* themeLightAction = viewMenu->addAction("&Light Theme");
    QObject::connect(themeLightAction, SIGNAL(triggered()), mainWindow, SLOT(onSetThemeLight()));
    
    QAction* themeDarkAction = viewMenu->addAction("&Dark Theme");
    QObject::connect(themeDarkAction, SIGNAL(triggered()), mainWindow, SLOT(onSetThemeDark()));
    
    QAction* themeSystemAction = viewMenu->addAction("&System Theme");
    QObject::connect(themeSystemAction, SIGNAL(triggered()), mainWindow, SLOT(onSetThemeSystem()));
    
    // Settings menu
    QMenu* settingsMenu = mainWindow->menuBar()->addMenu("&Settings");
    QAction* preferencesAction = settingsMenu->addAction("&Preferences...");
    preferencesAction->setShortcut(QKeySequence::Preferences);
    QObject::connect(preferencesAction, &QAction::triggered, mainWindow, &MainWindow::showPreferences);
    
    // Help menu
    QMenu* helpMenu = mainWindow->menuBar()->addMenu("&Help");
    QAction* helpAction = helpMenu->addAction("&Help");
    helpAction->setShortcut(QKeySequence::HelpContents);
    QObject::connect(helpAction, SIGNAL(triggered()), mainWindow, SLOT(showHelp()));
    
    QAction* aboutAction = helpMenu->addAction("&About");
    aboutAction->setEnabled(false); // Not implemented yet
}

void MainWindowUISetup::setupStatusBar(MainWindow* mainWindow) {
    if (!mainWindow) {
        return;
    }
    
    mainWindow->statusBar()->showMessage("Ready");
}

void MainWindowUISetup::setWindowProperties(MainWindow* mainWindow) {
    if (!mainWindow) {
        return;
    }
    
    // Window title
    mainWindow->setWindowTitle("OCR-Orc");
    
    // Set window icon
    QDir appDir(QCoreApplication::applicationDirPath());
    QString iconPath;
    
    // Try to find AppIcon.icns in Resources
    QStringList possibleIconPaths = {
        appDir.absoluteFilePath("../Resources/AppIcon.icns"),
        appDir.absoluteFilePath("../../AppIcon.icns"),
        appDir.absoluteFilePath("../../../AppIcon.icns"),
    };
    
    // Also try source directory
    QDir sourceDir = appDir;
    sourceDir.cdUp();
    sourceDir.cdUp();
    possibleIconPaths << sourceDir.absoluteFilePath("AppIcon.icns");
    
    for (const QString& path : possibleIconPaths) {
        if (QFileInfo::exists(path)) {
            iconPath = path;
            break;
        }
    }
    
    if (!iconPath.isEmpty()) {
        QIcon appIcon(iconPath);
        if (!appIcon.isNull()) {
            mainWindow->setWindowIcon(appIcon);
        }
    }
    
    // Window size
    mainWindow->resize(1200, 800);
    
    // Minimum size
    mainWindow->setMinimumSize(800, 600);
    
    // Window is resizable (default)
    
    // Enable drag and drop for PDF files
    mainWindow->setAcceptDrops(true);
}

void MainWindowUISetup::setupUI(MainWindow* mainWindow,
                                ::QWidget*& centralWidget,
                                ::QVBoxLayout*& mainLayout,
                                ::QFrame*& mainFrame,
                                ::QWidget*& canvasContainer,
                                ToolbarWidget*& toolbarWidget,
                                ControlPanelWidget*& controlPanelWidget,
                                SidePanelWidget*& sidePanelWidget,
                                Canvas*& canvas,
                                ::QLabel*& canvasStatusLabel,
                                DocumentState* documentState,
                                const std::function<void()>& connectWidgets) {
    if (!mainWindow || !documentState) {
        return;
    }
    
    setupMenuBar(mainWindow, mainWindow->fileMenu, mainWindow->exitAction);
    setupStatusBar(mainWindow);
    setupLayout(mainWindow, centralWidget, mainLayout, mainFrame, canvasContainer);
    
    // Create modular widgets
    toolbarWidget = new ToolbarWidget(centralWidget);
    controlPanelWidget = new ControlPanelWidget(centralWidget);
    sidePanelWidget = new SidePanelWidget(centralWidget);
    
    // Add widgets to layout IN ORDER (top to bottom)
    mainLayout->addWidget(toolbarWidget);        // Top: Toolbar
    mainLayout->addWidget(controlPanelWidget);   // Second: Control Panel
    
    setupCanvas(mainWindow, canvasContainer, documentState, canvas, canvasStatusLabel);
    
    // Add side panel to main frame layout (canvas and side panel side-by-side)
    QHBoxLayout* mainFrameLayout = qobject_cast<QHBoxLayout*>(mainFrame->layout());
    if (mainFrameLayout) {
        mainFrameLayout->addWidget(sidePanelWidget);
    }
    
    // Add main frame AFTER toolbar and control panel (bottom section)
    mainLayout->addWidget(mainFrame, 1);  // Expandable, takes remaining space
    
    // Connect all widget signals/slots
    if (connectWidgets) {
        connectWidgets();
    }
}

void MainWindowUISetup::setupCanvas(MainWindow* mainWindow,
                                   ::QWidget* canvasContainer,
                                   DocumentState* documentState,
                                   Canvas*& canvas,
                                   ::QLabel*& canvasStatusLabel) {
    if (!mainWindow || !canvasContainer || !documentState) {
        return;
    }
    
    QVBoxLayout* canvasLayout = new QVBoxLayout(canvasContainer);
    canvasLayout->setContentsMargins(5, 5, 5, 5);
    canvasLayout->setSpacing(5);
    
    // Status label
    canvasStatusLabel = new QLabel("Click and drag to create regions", canvasContainer);
    canvasStatusLabel->setStyleSheet("font: Arial, 10pt; color: #000000; background-color: transparent;");
    canvasStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    canvasLayout->addWidget(canvasStatusLabel);
    
    // Canvas widget for PDF display and region manipulation
    canvas = new Canvas(canvasContainer);
    canvas->setDocumentState(documentState);
    
    // Connect Canvas signals to MainWindow slots
    QObject::connect(canvas, SIGNAL(regionCreated(const QString&)), mainWindow, SLOT(onRegionCreated(const QString&)));
    QObject::connect(canvas, SIGNAL(regionCreationRequested()), mainWindow, SLOT(onRegionCreationRequested()));
    QObject::connect(canvas, SIGNAL(regionsDuplicated(const QList<QString>&)), mainWindow, SLOT(onRegionsDuplicated(const QList<QString>&)));
    QObject::connect(canvas, SIGNAL(undoRequested()), mainWindow, SLOT(onUndo()));
    QObject::connect(canvas, SIGNAL(redoRequested()), mainWindow, SLOT(onRedo()));
    
    // Connect region edit request (double-click) - use new-style connect for lambda
    QObject::connect(canvas, &Canvas::regionEditRequested, mainWindow, [mainWindow](const QString& regionName) {
        // Handle region edit request from double-click
        if (!mainWindow->documentState || !mainWindow->documentState->hasRegion(regionName)) {
            return;
        }
        
        QString newName = mainWindow->showEditNameDialog(regionName);
        if (newName.isEmpty()) {
            return; // User cancelled
        }
        
        // Validate and rename
        if (!mainWindow->documentState->isValidRegionName(newName, regionName)) {
            if (newName.trimmed().isEmpty()) {
                QMessageBox::warning(mainWindow, "Invalid Name", "Region name cannot be empty.");
            } else if (mainWindow->documentState->hasRegion(newName)) {
                QMessageBox::warning(mainWindow, "Invalid Name",
                    QString("Region '%1' already exists.").arg(newName));
            }
            return;
        }
        
        mainWindow->documentState->saveState();
        if (mainWindow->documentState->renameRegion(regionName, newName)) {
            mainWindow->canvas->clearSelection();
            mainWindow->canvas->selectRegion(newName);
            mainWindow->updateRegionListBox();
            mainWindow->updateGroupListBox();
            mainWindow->canvas->update();
            mainWindow->updateUndoRedoButtons();
            mainWindow->statusBar()->showMessage(
                QString("Renamed region '%1' to '%2'").arg(regionName).arg(newName), 2000);
        }
    });
    
    // Connect state changed signal to update UI - use new-style connect for lambda
    QObject::connect(canvas, &Canvas::stateChanged, mainWindow, [mainWindow]() {
        mainWindow->updateUndoRedoButtons();
        mainWindow->updateRegionListBox();
        mainWindow->updateGroupListBox();
    });
    
    // Connect color change request - use new-style connect for lambda
    QObject::connect(canvas, &Canvas::regionColorChangeRequested, mainWindow, [mainWindow](const QString& regionName) {
        mainWindow->regionOperations->handleRegionColorChangeRequested(
            regionName,
            mainWindow->documentState,
            [mainWindow](const QString& name) { return mainWindow->documentState->getRegion(name); },
            [mainWindow](const QString& name) { return mainWindow->showChangeColorDialog(name); },
            [mainWindow]() { mainWindow->documentState->saveState(); },
            [mainWindow](const QString& name, const QString& color) {
                return mainWindow->documentState->changeRegionColor(name, color);
            },
            [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->update(); },
            [mainWindow]() { mainWindow->updateRegionListBox(); },
            [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
            [mainWindow](const QString& message, int timeout) {
                mainWindow->statusBar()->showMessage(message, timeout);
            }
        );
    });
    
    // Setup keyboard shortcuts for undo/redo using QAction
    // On macOS, explicitly use Meta (Cmd) key, not Ctrl
    // On Windows/Linux, use Ctrl key
    
    #ifdef Q_OS_MACOS
    // macOS: Use Meta (Cmd) key
    QKeySequence undoKey = QKeySequence(Qt::META | Qt::Key_Z);
    QKeySequence redoKey = QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_Z);
    #else
    // Windows/Linux: Use Ctrl key
    QKeySequence undoKey = QKeySequence(Qt::CTRL | Qt::Key_Z);
    QKeySequence redoKey = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z);
    #endif
    
    QAction* undoAction = new QAction(mainWindow);
    undoAction->setShortcut(undoKey);
    undoAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(undoAction, SIGNAL(triggered()), mainWindow, SLOT(onUndo()));
    mainWindow->addAction(undoAction); // Add to MainWindow so it works globally
    OCR_ORC_DEBUG("Undo action created:" << undoAction->shortcut().toString() << "(should be Meta+Z on macOS)");
    
    QAction* redoAction = new QAction(mainWindow);
    redoAction->setShortcut(redoKey);
    redoAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(redoAction, SIGNAL(triggered()), mainWindow, SLOT(onRedo()));
    mainWindow->addAction(redoAction); // Add to MainWindow so it works globally
    OCR_ORC_DEBUG("Redo action created:" << redoAction->shortcut().toString() << "(should be Meta+Shift+Z on macOS)");
    
    // Also add actions to Canvas widget for when it has focus
    QAction* undoActionCanvas = new QAction(canvas);
    undoActionCanvas->setShortcut(undoKey);
    undoActionCanvas->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(undoActionCanvas, SIGNAL(triggered()), mainWindow, SLOT(onUndo()));
    canvas->addAction(undoActionCanvas);
    OCR_ORC_DEBUG("Undo action created on Canvas:" << undoActionCanvas->shortcut().toString());
    
    QAction* redoActionCanvas = new QAction(canvas);
    redoActionCanvas->setShortcut(redoKey);
    redoActionCanvas->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(redoActionCanvas, SIGNAL(triggered()), mainWindow, SLOT(onRedo()));
    canvas->addAction(redoActionCanvas);
    OCR_ORC_DEBUG("Redo action created on Canvas:" << redoActionCanvas->shortcut().toString());
    
    // Initial button state update
    mainWindow->updateUndoRedoButtons();
    
    // Setup all keyboard shortcuts (includes group shortcuts)
    mainWindow->setupKeyboardShortcuts();
    
    canvasLayout->addWidget(canvas, 1);  // Expandable
}

} // namespace ocr_orc

