#include "MainWindow.h"
#include "canvas/Canvas.h"
#include "canvas/ui/CanvasUiSync.h"
#include "components/widgets/ToolbarWidget.h"
#include "components/widgets/ControlPanelWidget.h"
#include "components/widgets/SidePanelWidget.h"
#include "components/dialogs/PreferencesDialog.h"
#include "components/dialogs/DetectionPreviewDialog.h"
#include "components/dialogs/MagicDetectParamsDialog.h"
#include "utils/DetectionWorker.h"
#include "../models/DocumentState.h"
#include "../utils/RegionDetector.h"
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
#include <QtCore/QDebug>
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
            , detectionThread(nullptr)
            , detectionWorker(nullptr)
            , isDetecting(false)
            , currentDetectionResult(nullptr)
{
    setWindowProperties();
    setupUI();
    // connectWidgets() is called in setupUI(), no need to call it here
    
    // Initialize and apply theme
    initializeTheme();
    
    // Detection worker thread will be initialized lazily when first needed
}

MainWindow::~MainWindow() {
    // Stop detection worker thread
    if (detectionThread) {
        detectionThread->quit();
        detectionThread->wait();
    }
    if (detectionWorker) {
        delete detectionWorker;
    }
    
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
    
    // Connect zoom changed signal directly here (after canvas is fully initialized)
    qDebug() << "[ZOOM DEBUG] Connecting zoomChanged in MainWindow...";
    bool zoomConnected = QObject::connect(canvas, SIGNAL(zoomChanged(double)), 
                                           this, SLOT(updateZoomLabel(double)));
    qDebug() << "[ZOOM DEBUG] Connection result:" << zoomConnected;
    if (!zoomConnected) {
        qWarning() << "[ZOOM DEBUG] FAILED to connect zoomChanged!";
    }
    
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

void MainWindow::updateZoomLabel(double zoomLevel) {
    // Direct update with provided zoom level (from signal)
    qDebug() << "[ZOOM DEBUG] *** MainWindow::updateZoomLabel(" << zoomLevel << ") CALLED ***";
    if (toolbarWidget) {
        qDebug() << "[ZOOM DEBUG] Calling toolbarWidget->updateZoomLabel(" << zoomLevel << ")";
        toolbarWidget->updateZoomLabel(zoomLevel);
        qDebug() << "[ZOOM DEBUG] toolbarWidget->updateZoomLabel completed";
    } else {
        qWarning() << "[ZOOM DEBUG] ERROR: toolbarWidget is null!";
    }
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

void MainWindow::onMagicDetect() {
    fprintf(stderr, "[MainWindow::onMagicDetect] ========== MAGIC DETECT STARTED ==========\n");
    fflush(stderr);
    qDebug() << "[MainWindow::onMagicDetect] Entry point - Magic Detect button clicked";
    
    try {
        // Check if PDF is loaded
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 1: Checking if PDF is loaded...\n");
        fflush(stderr);
        qDebug() << "[MainWindow::onMagicDetect] documentState:" << (documentState ? "exists" : "null");
        
        if (!documentState || documentState->image.isNull()) {
            fprintf(stderr, "[MainWindow::onMagicDetect] ERROR: No PDF loaded\n");
            fflush(stderr);
            statusBar()->showMessage("Please load a PDF first", 3000);
            return;
        }
        
        // Show parameter configuration dialog
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 1.5: Showing parameter configuration dialog...\n");
        fflush(stderr);
        DetectionParameters defaultParams;
        MagicDetectParamsDialog* paramsDialog = new MagicDetectParamsDialog(this, defaultParams);
        int dialogResult = paramsDialog->exec();
        
        if (dialogResult != QDialog::Accepted || !paramsDialog->shouldRun()) {
            fprintf(stderr, "[MainWindow::onMagicDetect] Step 1.5: User cancelled parameter dialog\n");
            fflush(stderr);
            delete paramsDialog;
            return;
        }
        
        // Get parameters from dialog
        DetectionParameters params = paramsDialog->getParameters();
        delete paramsDialog;
        
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 1.5: ✓ Parameters configured - OCR overlap: %.2f, Edge density: %.3f\n",
                params.ocrOverlapThreshold, params.edgeDensityThreshold);
        fflush(stderr);
        
        // Store parameters for use in detection (we'll pass these through later)
        currentDetectionParams = params;
        
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 1: ✓ PDF loaded - Image size: %dx%d\n", 
                documentState->image.width(), documentState->image.height());
        fflush(stderr);
        qDebug() << "[MainWindow::onMagicDetect] Image dimensions:" << documentState->image.width() << "x" << documentState->image.height();
        
        // Lazy initialization of detection worker thread
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 2: Checking detection worker thread...\n");
        fflush(stderr);
        qDebug() << "[MainWindow::onMagicDetect] detectionWorker:" << (detectionWorker ? "exists" : "null");
        qDebug() << "[MainWindow::onMagicDetect] detectionThread:" << (detectionThread ? "exists" : "null");
        
        if (!detectionWorker || !detectionThread) {
            fprintf(stderr, "[MainWindow::onMagicDetect] Step 2: Creating new detection worker thread...\n");
            fflush(stderr);
            qDebug() << "[MainWindow::onMagicDetect] Creating new worker thread and worker object";
            
            try {
                detectionThread = new QThread(this);
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.1: QThread created\n");
                fflush(stderr);
                
                detectionWorker = new DetectionWorker();
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.2: DetectionWorker created\n");
                fflush(stderr);
                
                // Move worker to thread BEFORE connecting signals
                detectionWorker->moveToThread(detectionThread);
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.3: Worker moved to thread\n");
                fflush(stderr);
                
                // Connect detection worker signals
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.4: Connecting signals...\n");
                fflush(stderr);
                QObject::connect(detectionWorker, &DetectionWorker::detectionComplete, 
                                 this, &MainWindow::onDetectionComplete, Qt::QueuedConnection);
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.4.1: detectionComplete signal connected\n");
                fflush(stderr);
                
                QObject::connect(detectionWorker, &DetectionWorker::detectionError, 
                                 this, &MainWindow::onDetectionError, Qt::QueuedConnection);
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.4.2: detectionError signal connected\n");
                fflush(stderr);
                
                QObject::connect(detectionWorker, &DetectionWorker::detectionProgress, 
                                 this, &MainWindow::onDetectionProgress, Qt::QueuedConnection);
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.4.3: detectionProgress signal connected\n");
                fflush(stderr);
                
                // Connect thread finished signal to delete worker
                QObject::connect(detectionThread, &QThread::finished, detectionWorker, &QObject::deleteLater);
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.5: Thread finished signal connected\n");
                fflush(stderr);
                
                // Start worker thread
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.6: Starting worker thread...\n");
                fflush(stderr);
                detectionThread->start();
                fprintf(stderr, "[MainWindow::onMagicDetect] Step 2.6: ✓ Worker thread started\n");
                fflush(stderr);
                qDebug() << "[MainWindow::onMagicDetect] Worker thread started successfully";
            } catch (const std::exception& e) {
                fprintf(stderr, "[MainWindow::onMagicDetect] EXCEPTION in worker thread setup: %s\n", e.what());
                fflush(stderr);
                statusBar()->showMessage(QString("Failed to initialize detection: %1").arg(e.what()), 5000);
                isDetecting = false;
                if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
                    toolbarWidget->getMagicDetectButton()->setEnabled(true);
                }
                return;
            } catch (...) {
                fprintf(stderr, "[MainWindow::onMagicDetect] UNKNOWN EXCEPTION in worker thread setup\n");
                fflush(stderr);
                statusBar()->showMessage("Failed to initialize detection: Unknown error", 5000);
                isDetecting = false;
                if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
                    toolbarWidget->getMagicDetectButton()->setEnabled(true);
                }
                return;
            }
        } else {
            fprintf(stderr, "[MainWindow::onMagicDetect] Step 2: ✓ Worker thread already exists\n");
            fflush(stderr);
        }
        
        // Check if already detecting
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 3: Checking if already detecting...\n");
        fflush(stderr);
        qDebug() << "[MainWindow::onMagicDetect] isDetecting:" << isDetecting;
        
        if (isDetecting) {
            fprintf(stderr, "[MainWindow::onMagicDetect] Step 3: Already detecting, returning\n");
            fflush(stderr);
            statusBar()->showMessage("Detection already in progress...", 2000);
            return;
        }
        
        // Set flag and disable button
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 4: Setting detection flag and disabling button...\n");
        fflush(stderr);
        isDetecting = true;
        if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
            toolbarWidget->getMagicDetectButton()->setEnabled(false);
        }
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 4: ✓ Flag set, button disabled\n");
        fflush(stderr);
        
    // Start detection in worker thread (using OCR-first method)
    fprintf(stderr, "[MainWindow::onMagicDetect] Step 5: Invoking detectRegions in worker thread...\n");
    fflush(stderr);
    qDebug() << "[MainWindow::onMagicDetect] Invoking detectRegions with method: ocr-first";
    statusBar()->showMessage("Starting detection... OCR processing may take 1-2 minutes. Please wait...", 0);
        
        // Store parameters in worker for access (Q_ARG doesn't work with custom types)
        detectionWorker->setDetectionParameters(params);
        bool invokeSuccess = QMetaObject::invokeMethod(detectionWorker, "detectRegions", Qt::QueuedConnection,
                                  Q_ARG(QImage, documentState->image),
                                  Q_ARG(QString, QString("ocr-first")));
        
        if (!invokeSuccess) {
            fprintf(stderr, "[MainWindow::onMagicDetect] ERROR: Failed to invoke detectRegions method!\n");
            fflush(stderr);
            qWarning() << "[MainWindow::onMagicDetect] Failed to invoke detectRegions";
            statusBar()->showMessage("Failed to start detection. Please try again.", 5000);
            isDetecting = false;
            if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
                toolbarWidget->getMagicDetectButton()->setEnabled(true);
            }
            return;
        }
        
        fprintf(stderr, "[MainWindow::onMagicDetect] Step 5: ✓ detectRegions invoked successfully\n");
        fflush(stderr);
        fprintf(stderr, "[MainWindow::onMagicDetect] ========== MAGIC DETECT INITIATED ==========\n");
        fflush(stderr);
        qDebug() << "[MainWindow::onMagicDetect] Magic detect initiated successfully";
        
    } catch (const std::exception& e) {
        fprintf(stderr, "[MainWindow::onMagicDetect] CRITICAL EXCEPTION: %s\n", e.what());
        fflush(stderr);
        qCritical() << "[MainWindow::onMagicDetect] Exception:" << e.what();
        statusBar()->showMessage(QString("Detection error: %1").arg(e.what()), 10000);
        isDetecting = false;
        if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
            toolbarWidget->getMagicDetectButton()->setEnabled(true);
        }
    } catch (...) {
        fprintf(stderr, "[MainWindow::onMagicDetect] CRITICAL UNKNOWN EXCEPTION\n");
        fflush(stderr);
        qCritical() << "[MainWindow::onMagicDetect] Unknown exception occurred";
        statusBar()->showMessage("Detection error: Unknown exception occurred", 10000);
        isDetecting = false;
        if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
            toolbarWidget->getMagicDetectButton()->setEnabled(true);
        }
    }
}

void MainWindow::onDetectionProgress(int percent, const QString& message) {
    QString progressMessage;
    if (percent < 20) {
        progressMessage = QString("Initializing... %1% - %2").arg(percent).arg(message);
    } else if (percent < 50) {
        progressMessage = QString("Processing OCR... %1% - %2").arg(percent).arg(message);
    } else if (percent < 90) {
        progressMessage = QString("Analyzing document... %1% - %2").arg(percent).arg(message);
    } else {
        progressMessage = QString("Finalizing... %1% - %2").arg(percent).arg(message);
    }
    statusBar()->showMessage(progressMessage, 0);
    fprintf(stderr, "[MainWindow::onDetectionProgress] Progress: %d%% - %s\n", percent, message.toLocal8Bit().constData());
    fflush(stderr);
}

void MainWindow::onRegionsAcceptedFromDetection(const QList<DetectedRegion>& regions) {
    if (!currentDetectionResult) {
        statusBar()->showMessage("Error: No detection result available", 3000);
        return;
    }
    const DetectionResult& result = *currentDetectionResult;
    if (regions.isEmpty()) {
        statusBar()->showMessage("No regions selected", 2000);
        return;
    }
    
    if (!documentState) {
        statusBar()->showMessage("Error: No document state", 3000);
        return;
    }
    
    // Save state for undo (entire batch creation can be undone)
    documentState->saveState();
    
    // Create mapping from region index to generated name
    QMap<int, QString> regionIndexToName;
    
    // Generate region names and create regions with inferred types and colors
    int regionNumber = 1;
    for (int i = 0; i < regions.size(); ++i) {
        const DetectedRegion& detectedRegion = regions[i];
        
        // Generate name: "Cell_1", "Cell_2", etc.
        QString name;
        do {
            name = QString("Cell_%1").arg(regionNumber);
            regionNumber++;
        } while (documentState->hasRegion(name));
        
        regionIndexToName[i] = name;
        
        // Use inferred type from detection, or default to "letters"
        QString regionType = detectedRegion.inferredType;
        if (regionType.isEmpty() || regionType == "unknown") {
            regionType = "letters"; // Default type
        }
        
        // Use suggested color from detection, or default to "blue"
        QString color = detectedRegion.suggestedColor;
        if (color.isEmpty()) {
            color = "blue"; // Default color
        }
        
        // Create RegionData from DetectedRegion with inferred type and color
        RegionData region(name, detectedRegion.coords, color, "");
        region.regionType = regionType;
        
        // Set group if suggested (will be added to group later)
        if (!detectedRegion.suggestedGroup.isEmpty()) {
            region.group = detectedRegion.suggestedGroup;
        }
        
        // Add to document state
        documentState->addRegion(name, region);
    }
    
    // Create groups from inferred groups
    for (const DetectedGroup& inferredGroup : result.inferredGroups) {
        // Create the group if it doesn't exist
        if (!documentState->hasGroup(inferredGroup.name)) {
            documentState->createGroup(inferredGroup.name);
        }
        
        // Map group region names to actual created region names
        // The inferredGroup.regionNames are placeholder names like "Postal_code_cell_1"
        // We need to map them to actual regions based on suggestedGroup field
        for (int i = 0; i < regions.size(); ++i) {
            const DetectedRegion& detectedRegion = regions[i];
            if (detectedRegion.suggestedGroup == inferredGroup.name) {
                QString actualRegionName = regionIndexToName[i];
                documentState->addRegionToGroup(actualRegionName, inferredGroup.name);
            }
        }
    }
    
    // Also handle regions with suggestedGroup that weren't in inferredGroups
    // (fallback for regions that have suggestedGroup but weren't part of a formal group)
    for (int i = 0; i < regions.size(); ++i) {
        const DetectedRegion& detectedRegion = regions[i];
        if (!detectedRegion.suggestedGroup.isEmpty()) {
            QString groupName = detectedRegion.suggestedGroup;
            QString actualRegionName = regionIndexToName[i];
            
            // Check if region is already in a group (from inferredGroups above)
            RegionData region = documentState->getRegion(actualRegionName);
            if (region.group.isEmpty()) {
                // Not in a group yet, create one if needed and add
                if (!documentState->hasGroup(groupName)) {
                    documentState->createGroup(groupName);
                }
                documentState->addRegionToGroup(actualRegionName, groupName);
            }
        }
    }
    
    // Update UI
    updateRegionListBox();
    if (canvas) {
        canvas->update();
    }
    
    int groupsCreated = result.inferredGroups.size();
    QString message = QString("Created %1 regions").arg(regions.size());
    if (groupsCreated > 0) {
        message += QString(" in %1 group(s)").arg(groupsCreated);
    }
    statusBar()->showMessage(message, 3000);
}

void MainWindow::onDetectionError(const QString& error) {
    fprintf(stderr, "[MainWindow::onDetectionError] ========== DETECTION ERROR ==========\n");
    fprintf(stderr, "[MainWindow::onDetectionError] Error message: %s\n", error.toLocal8Bit().constData());
    fflush(stderr);
    qCritical() << "[MainWindow::onDetectionError] Detection failed:" << error;
    
    // Convert technical error messages to user-friendly ones
    QString userMessage;
    QString lowerError = error.toLower();
    
    if (lowerError.contains("invalid image") || lowerError.contains("image is null")) {
        userMessage = "Invalid document. Please load a valid PDF file.";
    } else if (lowerError.contains("tesseract") || lowerError.contains("ocr")) {
        if (lowerError.contains("initialize") || lowerError.contains("init")) {
            userMessage = "OCR engine failed to start. Please check Tesseract installation.";
        } else if (lowerError.contains("timeout") || lowerError.contains("too long")) {
            userMessage = "OCR processing took too long. The document may be too complex. Try a simpler document or use manual mode.";
        } else {
            userMessage = "OCR processing failed. The document may be too complex or contain no readable text. Try using manual mode.";
        }
    } else if (lowerError.contains("detector") || lowerError.contains("create")) {
        userMessage = "Failed to initialize detection engine. Please try again or restart the application.";
    } else if (lowerError.contains("memory") || lowerError.contains("out of memory")) {
        userMessage = "Not enough memory to process this document. Try closing other applications or use a smaller document.";
    } else if (lowerError.contains("exception") || lowerError.contains("crash")) {
        userMessage = "An unexpected error occurred during detection. Please try again. If the problem persists, try using manual mode.";
    } else {
        // Keep original message but make it more user-friendly
        userMessage = QString("Detection failed: %1. Please try again or use manual mode.").arg(error);
    }
    
    statusBar()->showMessage(userMessage, 10000);
    isDetecting = false;
    if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
        toolbarWidget->getMagicDetectButton()->setEnabled(true);
    }
    fprintf(stderr, "[MainWindow::onDetectionError] User message: %s\n", userMessage.toLocal8Bit().constData());
    fprintf(stderr, "[MainWindow::onDetectionError] ====================================\n");
    fflush(stderr);
}

void MainWindow::onDetectionComplete(const DetectionResult& result) {
    isDetecting = false;
    if (toolbarWidget && toolbarWidget->getMagicDetectButton()) {
        toolbarWidget->getMagicDetectButton()->setEnabled(true);
    }
    
    // Check if any regions were detected
    if (result.totalDetected == 0) {
        statusBar()->showMessage("No regions detected. The document may not contain form fields, or OCR found no text. Try using manual mode to create regions.", 8000);
        return;
    }
    
    // Store result in member variable for access
    currentDetectionResult = const_cast<DetectionResult*>(&result);
    
    // Show preview dialog
    DetectionPreviewDialog* dialog = new DetectionPreviewDialog(this, result, documentState->image);
    connect(dialog, &DetectionPreviewDialog::regionsAccepted, this, &MainWindow::onRegionsAcceptedFromDetection);
    
    int resultCode = dialog->exec();
    currentDetectionResult = nullptr;
    delete dialog;
    
    if (resultCode == QDialog::Accepted) {
        statusBar()->showMessage("Regions accepted and created", 3000);
    } else {
        statusBar()->showMessage("Detection cancelled", 2000);
    }
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
        toolbarWidget->refreshIcons(); // Refresh icons when theme changes
    }
    if (controlPanelWidget) {
        themeManager.applyTheme(controlPanelWidget);
        controlPanelWidget->refreshIcons(); // Refresh icons when theme changes
    }
    if (sidePanelWidget) {
        themeManager.applyTheme(sidePanelWidget);
        sidePanelWidget->refreshIcons(); // Refresh icons when theme changes
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

