#include "MainWindow.h"
#include "canvas/Canvas.h"
#include "canvas/ui/CanvasUiSync.h"
#include "components/widgets/ToolbarWidget.h"
#include "components/widgets/ControlPanelWidget.h"
#include "components/widgets/SidePanelWidget.h"
#include "components/dialogs/PreferencesDialog.h"
#include "../models/DocumentState.h"
#include "../utils/PdfLoader.h"
#include "../export/JsonExporter.h"
#include "../export/CsvExporter.h"
#include "../export/JsonImporter.h"
#include "../export/MaskGenerator.h"
#include "../core/Constants.h"
#include "../utils/InputValidator.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtGui/QAction>
#include <QtGui/QShortcut>
#include <QtGui/QCloseEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QRegularExpressionMatch>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , mainLayout(nullptr)
    , mainFrame(nullptr)
    , canvasContainer(nullptr)
    , toolbarWidget(nullptr)
    , controlPanelWidget(nullptr)
    , sidePanelWidget(nullptr)
    , canvasStatusLabel(nullptr)
    , canvas(nullptr)
    , fileMenu(nullptr)
    , exitAction(nullptr)
    , documentState(new DocumentState())
    , isCreatingGroup(false)
    , fileOperations(new MainWindowFileOperations())
    , regionOperations(new MainWindowRegionOperations())
    , groupOperations(new MainWindowGroupOperations())
    , undoRedo(new MainWindowUndoRedo())
    , undoRedoAdapter(new MainWindowUndoRedoAdapter())
    , uiUpdates(new MainWindowUIUpdates())
    , toolbarAdapter(new MainWindowToolbarAdapter())
    , selectionOperations(new MainWindowSelectionOperations())
    , keyboardShortcuts(new MainWindowKeyboardShortcuts())
    , dialogsController(new MainWindowDialogsController())
    , uiSetup(new MainWindowUISetup())
    , widgetConnections(new MainWindowWidgetConnections())
    , canvasWiring(new MainWindowCanvasWiring())
    , eventHandlers(new MainWindowEventHandlers())
    , actionHandlers(new MainWindowActionHandlers())
    , regionGroupHandlers(new MainWindowRegionGroupHandlers())
    , regionEditorHandlers(new MainWindowRegionEditorHandlers())
    , viewHandlers(new MainWindowViewHandlers())
{
    setWindowProperties();
    setupUI();
    // connectWidgets() is called in setupUI(), no need to call it here
    
    // Initialize and apply theme
    initializeTheme();
}

MainWindow::~MainWindow() {
    delete documentState;
    delete fileOperations;
    delete regionOperations;
    delete groupOperations;
    delete undoRedo;
    delete undoRedoAdapter;
    delete uiUpdates;
    delete toolbarAdapter;
    delete selectionOperations;
    delete keyboardShortcuts;
    delete dialogsController;
    delete uiSetup;
    delete widgetConnections;
    delete canvasWiring;
    delete eventHandlers;
    delete actionHandlers;
    delete regionGroupHandlers;
    delete regionEditorHandlers;
    delete viewHandlers;
}

void MainWindow::setWindowProperties() {
    uiSetup->setWindowProperties(this);
}

void MainWindow::setupUI() {
    uiSetup->setupUI(
        this,
        centralWidget,
        mainLayout,
        mainFrame,
        canvasContainer,
        toolbarWidget,
        controlPanelWidget,
        sidePanelWidget,
        canvas,
        canvasStatusLabel,
        documentState,
        [this]() { connectWidgets(); }
    );
}

void MainWindow::setupMenuBar() {
    uiSetup->setupMenuBar(this, fileMenu, exitAction);
}

void MainWindow::setupStatusBar() {
    uiSetup->setupStatusBar(this);
}

void MainWindow::setupLayout() {
    uiSetup->setupLayout(this, centralWidget, mainLayout, mainFrame, canvasContainer);
}

// Old setup methods removed - now handled by ToolbarWidget, ControlPanelWidget, and SidePanelWidget

void MainWindow::setupCanvas() {
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
    
    // Connect all canvas signals using MainWindowCanvasWiring
    canvasWiring->connectCanvasSignals(
        this,
        canvas,
        documentState,
        [this](const QString& name) { return showEditNameDialog(name); },
        [this](const QString& name) { return showChangeColorDialog(name); },
        [this]() { updateRegionListBox(); },
        [this]() { updateGroupListBox(); },
        [this]() { if (canvas) canvas->update(); },
        [this]() { updateUndoRedoButtons(); },
        [this](const QString& message, int timeout) {
            statusBar()->showMessage(message, timeout);
        },
        [this]() { if (canvas) canvas->clearSelection(); },
        [this](const QString& name) { if (canvas) canvas->selectRegion(name); },
        [this](const QString& name) { return documentState->getRegion(name); },
        [this]() { documentState->saveState(); },
        [this](const QString& name, const QString& color) {
            return documentState->changeRegionColor(name, color);
        },
        [this](const QString& oldName, const QString& newName) {
            return documentState->renameRegion(oldName, newName);
        },
        [this]() { if (canvas) canvas->invalidateCoordinateCache(); }
    );
    
    // Setup keyboard shortcuts for canvas (undo/redo)
    canvasWiring->setupCanvasShortcuts(this, canvas);
    
    // Initial button state update
    updateUndoRedoButtons();
    
    // Setup all keyboard shortcuts (includes group shortcuts)
    setupKeyboardShortcuts();
    
    canvasLayout->addWidget(canvas, 1);  // Expandable
}

// Old setup methods removed - now handled by SidePanelWidget

void MainWindow::closeEvent(QCloseEvent *event) {
    eventHandlers->handleCloseEvent(this, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    eventHandlers->handleDragEnterEvent(this, event);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    eventHandlers->handleDragMoveEvent(this, event);
}

void MainWindow::dropEvent(QDropEvent *event) {
    eventHandlers->handleDropEvent(
        this,
        event,
        documentState,
        canvas,
        controlPanelWidget,
        [this]() { updateZoomLabel(); },
        [this]() { updateUndoRedoButtons(); },
        [this]() { updateRegionListBox(); },
        [this]() { updateGroupListBox(); },
        [this](const QString& label) { controlPanelWidget->setFileLabel(label); },
        [this](const QString& title, const QString& message) {
            QMessageBox::warning(this, title, message);
        }
    );
}

void MainWindow::onExit() {
    close();
}

void MainWindow::onLoadPdf() {
    actionHandlers->onLoadPdf(this);
}

void MainWindow::onCreateModeSelected() {
    viewHandlers->onCreateModeSelected(canvas);
}

void MainWindow::onSelectModeSelected() {
    viewHandlers->onSelectModeSelected(canvas);
}

void MainWindow::onZoomIn() {
    viewHandlers->onZoomIn(canvas, [this]() { updateZoomLabel(); });
}

void MainWindow::onZoomOut() {
    viewHandlers->onZoomOut(canvas, [this]() { updateZoomLabel(); });
}

void MainWindow::onZoomReset() {
    viewHandlers->onZoomReset(canvas, [this]() { updateZoomLabel(); });
}

void MainWindow::updateZoomLabel() {
    toolbarAdapter->updateZoomLabel(
        canvas,
        [this]() { return canvas ? canvas->getZoom() : 1.0; },
        [this](double zoom) {
            if (toolbarWidget) {
                toolbarWidget->updateZoomLabel(zoom);
            }
        }
    );
}

void MainWindow::onUndo() {
    undoRedoAdapter->performUndo(
        documentState,
        canvas,
        [this]() { return documentState ? documentState->canUndo() : false; },
        [this]() { if (documentState) documentState->undoAction(); },
        [this]() { if (canvas) canvas->invalidateCoordinateCache(); },
        [this]() { if (canvas) canvas->update(); },
        [this]() { updateRegionListBox(); },
        [this]() { updateGroupListBox(); },
        [this](bool canUndo, bool canRedo) {
            if (toolbarWidget) {
                toolbarWidget->updateUndoRedoButtons(canUndo, canRedo);
            }
        },
        [this]() { if (canvas) canvas->clearSelection(); },
        [this](const QString& message, int timeout) {
            statusBar()->showMessage(message, timeout);
        }
    );
}

void MainWindow::onRedo() {
    undoRedoAdapter->performRedo(
        documentState,
        canvas,
        [this]() { return documentState ? documentState->canRedo() : false; },
        [this]() { if (documentState) documentState->redoAction(); },
        [this]() { if (canvas) canvas->invalidateCoordinateCache(); },
        [this]() { if (canvas) canvas->update(); },
        [this]() { updateRegionListBox(); },
        [this]() { updateGroupListBox(); },
        [this](bool canUndo, bool canRedo) {
            if (toolbarWidget) {
                toolbarWidget->updateUndoRedoButtons(canUndo, canRedo);
            }
        },
        [this]() { if (canvas) canvas->clearSelection(); },
        [this](const QString& message, int timeout) {
            statusBar()->showMessage(message, timeout);
        }
    );
}

void MainWindow::updateUndoRedoButtons() {
    toolbarAdapter->updateUndoRedoButtons(
        documentState,
        [this]() { return documentState ? documentState->canUndo() : false; },
        [this]() { return documentState ? documentState->canRedo() : false; },
        [this](bool canUndo, bool canRedo) {
            if (toolbarWidget) {
                toolbarWidget->updateUndoRedoButtons(canUndo, canRedo);
            }
        }
    );
}

void MainWindow::updateRegionListBox() {
    viewHandlers->updateRegionListBox(
        this,
        documentState,
        canvas,
        sidePanelWidget,
        [this]() { return documentState ? documentState->getAllRegionNames() : QList<QString>(); },
        [this]() { return canvas ? canvas->getSelectedRegions() : QSet<QString>(); },
        [this](const QString& name) { return documentState->getRegion(name); },
        [this](const QList<QString>& regionNames, const QSet<QString>& selectedRegions, const QMap<QString, QString>& regionGroups) {
            if (sidePanelWidget) {
                sidePanelWidget->updateRegionListBox(regionNames, selectedRegions, regionGroups);
            }
        }
    );
}

void MainWindow::updateGroupListBox() {
    viewHandlers->updateGroupListBox(
        this,
        documentState,
        sidePanelWidget,
        [this]() { return documentState ? documentState->getGroupNames() : QList<QString>(); },
        [this](const QString& name) { return documentState->getGroup(name); },
        [this](const QList<QString>& groupNames, const QMap<QString, int>& groupSizes) {
            if (sidePanelWidget) {
                sidePanelWidget->updateGroupListBox(groupNames, groupSizes);
            }
        }
    );
}

void MainWindow::onCreateGroup() {
    regionGroupHandlers->onCreateGroup(this);
}

QSet<QString> MainWindow::getSelectedRegions() const {
    return regionGroupHandlers->getSelectedRegions(const_cast<MainWindow*>(this));
}

void MainWindow::onAddToGroup() {
    regionGroupHandlers->onAddToGroup(this);
}

void MainWindow::onRemoveFromGroup() {
    regionGroupHandlers->onRemoveFromGroup(this);
}

void MainWindow::onDeleteGroup() {
    regionGroupHandlers->onDeleteGroup(this);
}

QString MainWindow::showEditNameDialog(const QString& currentName) {
    return regionGroupHandlers->showEditNameDialog(this, currentName);
}

void MainWindow::onEditSelected() {
    regionGroupHandlers->onEditSelected(this);
}

void MainWindow::onRegionNameChanged(const QString& oldName, const QString& newName) {
    regionGroupHandlers->onRegionNameChanged(this, oldName, newName);
}

void MainWindow::onRegionColorChanged(const QString& regionName, const QString& newColor) {
    regionGroupHandlers->onRegionColorChanged(this, regionName, newColor);
}

void MainWindow::onRegionGroupChanged(const QString& regionName, const QString& newGroup) {
    regionGroupHandlers->onRegionGroupChanged(this, regionName, newGroup);
}

QString MainWindow::showChangeColorDialog(const QString& regionName) {
    return regionGroupHandlers->showChangeColorDialog(this, regionName);
}

void MainWindow::onRegionCreationRequested() {
    regionGroupHandlers->onRegionCreationRequested(this);
}

void MainWindow::onRegionCreated(const QString& regionName) {
    regionGroupHandlers->onRegionCreated(this, regionName);
}

void MainWindow::onRegionsDuplicated(const QList<QString>& duplicatedNames) {
    regionGroupHandlers->onRegionsDuplicated(this, duplicatedNames);
}

QString MainWindow::generateRegionName() const {
    return regionGroupHandlers->generateRegionName(const_cast<MainWindow*>(this));
}

void MainWindow::autoIncrementRegionName() {
    regionGroupHandlers->autoIncrementRegionName(this);
}

void MainWindow::onExportCoordinates() {
    actionHandlers->onExportCoordinates(this);
}

void MainWindow::onLoadCoordinates() {
    actionHandlers->onLoadCoordinates(this);
}

void MainWindow::onExportMask() {
    actionHandlers->onExportMask(this);
}

void MainWindow::setupKeyboardShortcuts() {
    viewHandlers->setupKeyboardShortcuts(this);
}

void MainWindow::onDeleteSelected() {
    regionGroupHandlers->onDeleteSelected(this);
}

void MainWindow::onSelectAll() {
    viewHandlers->onSelectAll(
        this,
        documentState,
        canvas,
        [this]() { return documentState ? documentState->getAllRegionNames() : QList<QString>(); },
        [this](const QSet<QString>& regions) {
            if (canvas) {
                canvas->setSelectedRegions(regions);
            }
        },
        [this]() { if (canvas) canvas->update(); },
        [this]() { updateRegionListBox(); }
    );
}

void MainWindow::onInvertSelection() {
    viewHandlers->onInvertSelection(
        this,
        documentState,
        canvas,
        [this]() { return documentState ? documentState->getAllRegionNames() : QList<QString>(); },
        [this]() { return canvas ? canvas->getSelectedRegions() : QSet<QString>(); },
        [this](const QSet<QString>& regions) {
            if (canvas) {
                canvas->setSelectedRegions(regions);
            }
        },
        [this]() { if (canvas) canvas->update(); },
        [this]() { updateRegionListBox(); }
    );
}

void MainWindow::showHelp() {
    dialogsController->showHelp(this);
}

void MainWindow::showPreferences() {
    dialogsController->showPreferences(this);
}

void MainWindow::connectWidgets() {
    widgetConnections->connectWidgets(
        this,
        toolbarWidget,
        controlPanelWidget,
        sidePanelWidget,
        canvas,
        documentState
    );
}

void MainWindow::initializeTheme() {
    ThemeManager& themeManager = ThemeManager::instance();
    themeManager.loadSettings();
    applyTheme();
}

void MainWindow::applyTheme() {
    ThemeManager& themeManager = ThemeManager::instance();
    themeManager.applyTheme(this);
    
    // Apply to all child widgets
    if (toolbarWidget) {
        themeManager.applyTheme(toolbarWidget);
    }
    if (controlPanelWidget) {
        themeManager.applyTheme(controlPanelWidget);
    }
    if (sidePanelWidget) {
        themeManager.applyTheme(sidePanelWidget);
    }
    if (canvas) {
        themeManager.applyTheme(canvas);
    }
    
    // Update canvas background color
    ThemeColors colors = themeManager.getColors();
    if (canvasContainer) {
        canvasContainer->setStyleSheet(
            QString("background-color: %1; color: %2;")
                .arg(colors.canvasBackground.name(), colors.text.name())
        );
    }
}

void MainWindow::onSetThemeLight() {
    ThemeManager& themeManager = ThemeManager::instance();
    themeManager.setTheme(Theme::Light);
    applyTheme();
}

void MainWindow::onSetThemeDark() {
    ThemeManager& themeManager = ThemeManager::instance();
    themeManager.setTheme(Theme::Dark);
    applyTheme();
}

void MainWindow::onSetThemeSystem() {
    ThemeManager& themeManager = ThemeManager::instance();
    themeManager.setTheme(Theme::System);
    applyTheme();
}

} // namespace ocr_orc

