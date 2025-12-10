#include "MainWindowCanvasWiring.h"
#include "../../MainWindow.h"
#include "../../canvas/Canvas.h"
#include "../../canvas/ui/CanvasUiSync.h"
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"
#include <QtCore/QObject>
#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <QtWidgets/QMessageBox>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

MainWindowCanvasWiring::MainWindowCanvasWiring() {
}

MainWindowCanvasWiring::~MainWindowCanvasWiring() {
}

void MainWindowCanvasWiring::connectCanvasSignals(MainWindow* mainWindow,
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
                                                   const InvalidateCacheCallback& invalidateCache) {
    if (!mainWindow || !canvas || !documentState) {
        return;
    }
    
    // Connect region creation signals
    QObject::connect(canvas, &Canvas::regionCreated, mainWindow, &MainWindow::onRegionCreated);
    QObject::connect(canvas, &Canvas::regionCreationRequested, mainWindow, &MainWindow::onRegionCreationRequested);
    QObject::connect(canvas, &Canvas::regionsDuplicated, mainWindow, &MainWindow::onRegionsDuplicated);
    
    // Connect undo/redo signals
    QObject::connect(canvas, &Canvas::undoRequested, mainWindow, &MainWindow::onUndo);
    QObject::connect(canvas, &Canvas::redoRequested, mainWindow, &MainWindow::onRedo);
    
    // Connect region edit request (double-click)
    QObject::connect(canvas, &Canvas::regionEditRequested, mainWindow, [mainWindow, documentState,
                                                                          showEditNameDialog, clearSelection,
                                                                          selectRegion, updateRegionListBox,
                                                                          updateGroupListBox, updateCanvas,
                                                                          updateUndoRedoButtons, showStatusMessage,
                                                                          saveState, renameRegion](const QString& regionName) {
        if (!documentState || !documentState->hasRegion(regionName)) {
            return;
        }
        
        QString newName = showEditNameDialog(regionName);
        if (newName.isEmpty()) {
            return; // User cancelled
        }
        
        // Validate and rename
        if (!documentState->isValidRegionName(newName, regionName)) {
            if (newName.trimmed().isEmpty()) {
                // Use QMessageBox through mainWindow
                QMessageBox::warning(mainWindow, "Invalid Name", "Region name cannot be empty.");
            } else if (documentState->hasRegion(newName)) {
                QMessageBox::warning(mainWindow, "Invalid Name",
                    QString("Region '%1' already exists.").arg(newName));
            }
            return;
        }
        
        saveState();
        if (renameRegion(regionName, newName)) {
            clearSelection();
            selectRegion(newName);
            updateRegionListBox();
            updateGroupListBox();
            updateCanvas();
            updateUndoRedoButtons();
            showStatusMessage(
                QString("Renamed region '%1' to '%2'").arg(regionName).arg(newName), 2000);
        }
    });
    
    // Connect state changed signal (using CanvasUiSync)
    QObject::connect(canvas, &Canvas::stateChanged, mainWindow, [documentState, invalidateCache,
                                                                  updateRegionListBox, updateGroupListBox,
                                                                  updateCanvas, updateUndoRedoButtons]() {
        CanvasUiSync::handleStateChanged(
            documentState,
            invalidateCache,
            updateRegionListBox,
            updateGroupListBox,
            updateCanvas,
            updateUndoRedoButtons
        );
    });
    
    // Connect color change request
    QObject::connect(canvas, &Canvas::regionColorChangeRequested, mainWindow, [documentState,
                                                                                getRegion, showChangeColorDialog,
                                                                                saveState, changeRegionColor,
                                                                                updateCanvas, updateRegionListBox,
                                                                                updateUndoRedoButtons, showStatusMessage](const QString& regionName) {
        // Use MainWindowRegionOperations for this
        // For now, handle directly here to avoid circular dependency
        if (!documentState || !documentState->hasRegion(regionName)) {
            return;
        }
        
        QString newColor = showChangeColorDialog(regionName);
        if (newColor.isEmpty()) {
            return; // User cancelled
        }
        
        saveState();
        if (changeRegionColor(regionName, newColor)) {
            updateCanvas();
            updateRegionListBox();
            updateUndoRedoButtons();
            showStatusMessage(
                QString("Changed color of region '%1' to '%2'").arg(regionName).arg(newColor), 2000);
        }
    });
}

void MainWindowCanvasWiring::setupCanvasShortcuts(MainWindow* mainWindow, Canvas* canvas) {
    if (!mainWindow || !canvas) {
        return;
    }
    
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
    
    // Create undo action for MainWindow
    QAction* undoAction = new QAction(mainWindow);
    undoAction->setShortcut(undoKey);
    undoAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(undoAction, &QAction::triggered, mainWindow, [mainWindow]() {
        OCR_ORC_DEBUG("=== UNDO ACTION TRIGGERED ===");
        mainWindow->onUndo();
    });
    mainWindow->addAction(undoAction); // Add to MainWindow so it works globally
    OCR_ORC_DEBUG("Undo action created:" << undoAction->shortcut().toString() << "(should be Meta+Z on macOS)");
    
    // Create redo action for MainWindow
    QAction* redoAction = new QAction(mainWindow);
    redoAction->setShortcut(redoKey);
    redoAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(redoAction, &QAction::triggered, mainWindow, [mainWindow]() {
        OCR_ORC_DEBUG("=== REDO ACTION TRIGGERED ===");
        mainWindow->onRedo();
    });
    mainWindow->addAction(redoAction); // Add to MainWindow so it works globally
    OCR_ORC_DEBUG("Redo action created:" << redoAction->shortcut().toString() << "(should be Meta+Shift+Z on macOS)");
    
    // Also add actions to Canvas widget for when it has focus
    QAction* undoActionCanvas = new QAction(canvas);
    undoActionCanvas->setShortcut(undoKey);
    undoActionCanvas->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(undoActionCanvas, &QAction::triggered, mainWindow, [mainWindow]() {
        OCR_ORC_DEBUG("=== UNDO ACTION TRIGGERED (Canvas) ===");
        mainWindow->onUndo();
    });
    canvas->addAction(undoActionCanvas);
    OCR_ORC_DEBUG("Undo action created on Canvas:" << undoActionCanvas->shortcut().toString());
    
    QAction* redoActionCanvas = new QAction(canvas);
    redoActionCanvas->setShortcut(redoKey);
    redoActionCanvas->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(redoActionCanvas, &QAction::triggered, mainWindow, [mainWindow]() {
        OCR_ORC_DEBUG("=== REDO ACTION TRIGGERED (Canvas) ===");
        mainWindow->onRedo();
    });
    canvas->addAction(redoActionCanvas);
    OCR_ORC_DEBUG("Redo action created on Canvas:" << redoActionCanvas->shortcut().toString());
}

} // namespace ocr_orc

