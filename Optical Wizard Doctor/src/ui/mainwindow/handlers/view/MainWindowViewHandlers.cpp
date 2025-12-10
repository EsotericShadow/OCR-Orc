#include "MainWindowViewHandlers.h"
#include "../../../MainWindow.h"
#include "../../../../models/DocumentState.h"
#include "../../../canvas/Canvas.h"
#include "../../../components/widgets/ToolbarWidget.h"
#include "../../../components/widgets/SidePanelWidget.h"
#include "../../../../models/RegionData.h"
#include "../../../../models/GroupData.h"
// MouseMode enum is defined in Canvas.h
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <functional>

namespace ocr_orc {

MainWindowViewHandlers::MainWindowViewHandlers() {
}

MainWindowViewHandlers::~MainWindowViewHandlers() {
}

void MainWindowViewHandlers::onCreateModeSelected(Canvas* canvas) {
    if (canvas) {
        canvas->setMode(ocr_orc::Create);
    }
}

void MainWindowViewHandlers::onSelectModeSelected(Canvas* canvas) {
    if (canvas) {
        canvas->setMode(ocr_orc::Select);
    }
}

void MainWindowViewHandlers::onZoomIn(Canvas* canvas, const std::function<void()>& updateZoomLabel) {
    if (canvas) {
        canvas->zoomIn();
        if (updateZoomLabel) {
            updateZoomLabel();
        }
    }
}

void MainWindowViewHandlers::onZoomOut(Canvas* canvas, const std::function<void()>& updateZoomLabel) {
    if (canvas) {
        canvas->zoomOut();
        if (updateZoomLabel) {
            updateZoomLabel();
        }
    }
}

void MainWindowViewHandlers::onZoomReset(Canvas* canvas, const std::function<void()>& updateZoomLabel) {
    if (canvas) {
        canvas->zoomReset();
        if (updateZoomLabel) {
            updateZoomLabel();
        }
    }
}

void MainWindowViewHandlers::updateZoomLabel(Canvas* canvas,
                                            ToolbarWidget* toolbarWidget,
                                            const std::function<double()>& getZoom,
                                            const std::function<void(double)>& setZoomLabel) {
    if (!canvas || !toolbarWidget) {
        return;
    }
    
    double zoom = getZoom ? getZoom() : 1.0;
    if (setZoomLabel) {
        setZoomLabel(zoom);
    }
}

void MainWindowViewHandlers::onUndo(MainWindow* mainWindow,
                                    DocumentState* documentState,
                                    Canvas* canvas,
                                    ToolbarWidget* /*toolbarWidget*/,
                                    const std::function<bool()>& canUndo,
                                    const std::function<void()>& undoAction,
                                    const std::function<void(bool, bool)>& updateButtons,
                                    const std::function<void()>& updateRegionListBox,
                                    const std::function<void()>& updateGroupListBox,
                                    const std::function<void()>& updateCanvas,
                                    const std::function<void()>& clearSelection,
                                    const std::function<void(const QString&, int)>& showStatusMessage) {
    if (!mainWindow || !documentState || !canvas) {
        return;
    }
    
    mainWindow->undoRedo->undo(
        documentState,
        canvas,
        canUndo,
        undoAction,
        updateButtons,
        updateRegionListBox,
        updateGroupListBox,
        updateCanvas,
        clearSelection,
        showStatusMessage
    );
}

void MainWindowViewHandlers::onRedo(MainWindow* mainWindow,
                                   DocumentState* documentState,
                                   Canvas* canvas,
                                   ToolbarWidget* /*toolbarWidget*/,
                                   const std::function<bool()>& canRedo,
                                   const std::function<void()>& redoAction,
                                   const std::function<void(bool, bool)>& updateButtons,
                                   const std::function<void()>& updateRegionListBox,
                                   const std::function<void()>& updateGroupListBox,
                                   const std::function<void()>& updateCanvas,
                                   const std::function<void()>& clearSelection,
                                   const std::function<void(const QString&, int)>& showStatusMessage) {
    if (!mainWindow || !documentState || !canvas) {
        return;
    }
    
    mainWindow->undoRedo->redo(
        documentState,
        canvas,
        canRedo,
        redoAction,
        updateButtons,
        updateRegionListBox,
        updateGroupListBox,
        updateCanvas,
        clearSelection,
        showStatusMessage
    );
}

void MainWindowViewHandlers::updateUndoRedoButtons(DocumentState* documentState,
                                                   ToolbarWidget* toolbarWidget,
                                                   const std::function<bool()>& canUndo,
                                                   const std::function<bool()>& canRedo,
                                                   const std::function<void(bool, bool)>& updateButtons) {
    if (!documentState || !toolbarWidget) {
        return;
    }
    
    bool undoAvailable = canUndo ? canUndo() : false;
    bool redoAvailable = canRedo ? canRedo() : false;
    
    if (updateButtons) {
        updateButtons(undoAvailable, redoAvailable);
    }
}

void MainWindowViewHandlers::updateRegionListBox(MainWindow* mainWindow,
                                                DocumentState* documentState,
                                                Canvas* canvas,
                                                SidePanelWidget* sidePanelWidget,
                                                const std::function<QList<QString>()>& getAllRegionNames,
                                                const std::function<QSet<QString>()>& getSelectedRegions,
                                                const std::function<RegionData(const QString&)>& getRegion,
                                                const std::function<void(const QList<QString>&, const QSet<QString>&, const QMap<QString, QString>&)>& updateRegionListBox) {
    if (!mainWindow || !documentState || !canvas || !sidePanelWidget) {
        return;
    }
    
    mainWindow->uiUpdates->updateRegionListBox(
        documentState,
        canvas,
        sidePanelWidget,
        getAllRegionNames,
        getSelectedRegions,
        getRegion,
        updateRegionListBox
    );
}

void MainWindowViewHandlers::updateGroupListBox(MainWindow* mainWindow,
                                               DocumentState* documentState,
                                               SidePanelWidget* sidePanelWidget,
                                               const std::function<QList<QString>()>& getGroupNames,
                                               const std::function<GroupData(const QString&)>& getGroup,
                                               const std::function<void(const QList<QString>&, const QMap<QString, int>&)>& updateGroupListBox) {
    if (!mainWindow || !documentState || !sidePanelWidget) {
        return;
    }
    
    mainWindow->uiUpdates->updateGroupListBox(
        documentState,
        sidePanelWidget,
        getGroupNames,
        getGroup,
        updateGroupListBox
    );
}

void MainWindowViewHandlers::onSelectAll(MainWindow* mainWindow,
                                        DocumentState* documentState,
                                        Canvas* canvas,
                                        const std::function<QList<QString>()>& getAllRegionNames,
                                        const std::function<void(const QSet<QString>&)>& setSelectedRegions,
                                        const std::function<void()>& updateCanvas,
                                        const std::function<void()>& updateRegionListBox) {
    if (!mainWindow || !documentState || !canvas) {
        return;
    }
    
    mainWindow->selectionOperations->selectAll(
        documentState,
        canvas,
        getAllRegionNames,
        setSelectedRegions,
        updateCanvas,
        updateRegionListBox
    );
}

void MainWindowViewHandlers::onInvertSelection(MainWindow* mainWindow,
                                              DocumentState* documentState,
                                              Canvas* canvas,
                                              const std::function<QList<QString>()>& getAllRegionNames,
                                              const std::function<QSet<QString>()>& getSelectedRegions,
                                              const std::function<void(const QSet<QString>&)>& setSelectedRegions,
                                              const std::function<void()>& updateCanvas,
                                              const std::function<void()>& updateRegionListBox) {
    if (!mainWindow || !documentState || !canvas) {
        return;
    }
    
    mainWindow->selectionOperations->invertSelection(
        documentState,
        canvas,
        getAllRegionNames,
        getSelectedRegions,
        setSelectedRegions,
        updateCanvas,
        updateRegionListBox
    );
}

void MainWindowViewHandlers::showHelp(MainWindow* mainWindow) {
    if (!mainWindow) {
        return;
    }
    
    mainWindow->dialogsController->showHelp(mainWindow);
}

void MainWindowViewHandlers::setupKeyboardShortcuts(MainWindow* mainWindow) {
    if (!mainWindow) {
        return;
    }
    
    mainWindow->keyboardShortcuts->setupShortcuts(mainWindow);
}

} // namespace ocr_orc

