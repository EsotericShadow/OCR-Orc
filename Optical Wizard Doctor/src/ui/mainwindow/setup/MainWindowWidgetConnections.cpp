#include "MainWindowWidgetConnections.h"
#include "../../MainWindow.h"
#include "../../components/widgets/ToolbarWidget.h"
#include "../../components/widgets/ControlPanelWidget.h"
#include "../../components/widgets/SidePanelWidget.h"
#include "../../canvas/Canvas.h"
#include "../../canvas/ui/CanvasUiSync.h"
#include "../../../models/DocumentState.h"
#include "../../../models/GroupData.h"
#include "../../../models/RegionData.h"
#include "../../../core/CoordinateSystem.h"
#include "../../mainwindow/handlers/regions/MainWindowRegionEditorHandlers.h"
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtCore/QPointF>

namespace ocr_orc {

MainWindowWidgetConnections::MainWindowWidgetConnections() {
}

MainWindowWidgetConnections::~MainWindowWidgetConnections() {
}

void MainWindowWidgetConnections::connectWidgets(MainWindow* mainWindow,
                                                 ToolbarWidget* toolbarWidget,
                                                 ControlPanelWidget* controlPanelWidget,
                                                 SidePanelWidget* sidePanelWidget,
                                                 Canvas* canvas,
                                                 DocumentState* documentState) {
    if (!mainWindow || !toolbarWidget || !controlPanelWidget || !sidePanelWidget || !canvas || !documentState) {
        return;
    }
    
    // Connect ToolbarWidget signals
    QObject::connect(toolbarWidget, &ToolbarWidget::createModeSelected, mainWindow, &MainWindow::onCreateModeSelected);
    QObject::connect(toolbarWidget, &ToolbarWidget::selectModeSelected, mainWindow, &MainWindow::onSelectModeSelected);
    QObject::connect(toolbarWidget, &ToolbarWidget::zoomInClicked, mainWindow, &MainWindow::onZoomIn);
    QObject::connect(toolbarWidget, &ToolbarWidget::zoomOutClicked, mainWindow, &MainWindow::onZoomOut);
    QObject::connect(toolbarWidget, &ToolbarWidget::zoomResetClicked, mainWindow, &MainWindow::onZoomReset);
    QObject::connect(canvas, &Canvas::zoomChanged, toolbarWidget, &ToolbarWidget::updateZoomLabel);
    QObject::connect(toolbarWidget, &ToolbarWidget::magicDetectClicked, mainWindow, &MainWindow::onMagicDetect);
    QObject::connect(toolbarWidget, &ToolbarWidget::rotateClicked, canvas, [canvas, toolbarWidget](bool checked) {
        canvas->setRotateMode(checked);
        // Sync button state with canvas state
        if (toolbarWidget) {
            toolbarWidget->updateRotateButton(checked);
        }
    });
    
    // Sync rotate button state when canvas rotate mode changes externally
    // (This ensures button state stays in sync if rotate mode is changed programmatically)
    QObject::connect(canvas, &Canvas::stateChanged, mainWindow, [canvas, toolbarWidget]() {
        if (canvas && toolbarWidget) {
            toolbarWidget->updateRotateButton(canvas->getRotateMode());
        }
    });
    QObject::connect(toolbarWidget, &ToolbarWidget::groupClicked, mainWindow, &MainWindow::onCreateGroup);
    QObject::connect(toolbarWidget, &ToolbarWidget::ungroupClicked, mainWindow, &MainWindow::onRemoveFromGroup);
    QObject::connect(toolbarWidget, &ToolbarWidget::undoClicked, mainWindow, &MainWindow::onUndo);
    QObject::connect(toolbarWidget, &ToolbarWidget::redoClicked, mainWindow, &MainWindow::onRedo);
    
    // Connect ControlPanelWidget signals
    QObject::connect(controlPanelWidget, &ControlPanelWidget::loadPdfClicked, mainWindow, &MainWindow::onLoadPdf);
    QObject::connect(controlPanelWidget, &ControlPanelWidget::exportCoordinatesClicked, mainWindow, &MainWindow::onExportCoordinates);
    // Export mask is now consolidated into export button - will open export dialog
    QObject::connect(controlPanelWidget, &ControlPanelWidget::loadCoordinatesClicked, mainWindow, &MainWindow::onLoadCoordinates);
    QObject::connect(controlPanelWidget, &ControlPanelWidget::clearSelectionClicked, mainWindow, &MainWindow::onDeleteSelected);
    QObject::connect(controlPanelWidget, &ControlPanelWidget::editSelectedClicked, mainWindow, &MainWindow::onEditSelected);
    
    // Connect SidePanelWidget signals
    QObject::connect(sidePanelWidget, &SidePanelWidget::regionSelectionChanged, mainWindow, [canvas, documentState](const QSet<QString>& selectedRegions) {
        if (canvas && documentState) {
            QSet<QString> validRegions;
            for (const QString& regionName : selectedRegions) {
                if (documentState->hasRegion(regionName)) {
                    validRegions.insert(regionName);
                }
            }
            canvas->setSelectedRegions(validRegions);
            canvas->update();
        }
    });
    
    QObject::connect(sidePanelWidget, &SidePanelWidget::groupSelectionChanged, mainWindow, [sidePanelWidget, documentState](const QString& groupName) {
        if (!documentState || !sidePanelWidget) {
            return;
        }
        
        if (documentState->hasGroup(groupName)) {
            GroupData group = documentState->getGroup(groupName);
            QString info = QString("Group: %1\n\nRegions: %2\n\nRegion List:\n")
                .arg(groupName).arg(group.regionNames.size());
            
            for (const QString& regionName : group.regionNames) {
                info += QString("  • %1\n").arg(regionName);
            }
            
            sidePanelWidget->setInfoText(info);
        }
    });
    
    QObject::connect(sidePanelWidget, &SidePanelWidget::createGroupClicked, mainWindow, &MainWindow::onCreateGroup);
    QObject::connect(sidePanelWidget, &SidePanelWidget::addToGroupClicked, mainWindow, &MainWindow::onAddToGroup);
    QObject::connect(sidePanelWidget, &SidePanelWidget::removeFromGroupClicked, mainWindow, &MainWindow::onRemoveFromGroup);
    QObject::connect(sidePanelWidget, &SidePanelWidget::deleteGroupClicked, mainWindow, &MainWindow::onDeleteGroup);
    QObject::connect(sidePanelWidget, &SidePanelWidget::helpClicked, mainWindow, &MainWindow::showHelp);
    
    // Connect region editor signals
    QObject::connect(sidePanelWidget, &SidePanelWidget::regionNameChanged, mainWindow, &MainWindow::onRegionNameChanged);
    QObject::connect(sidePanelWidget, &SidePanelWidget::regionColorChanged, mainWindow, &MainWindow::onRegionColorChanged);
    QObject::connect(sidePanelWidget, &SidePanelWidget::regionGroupChanged, mainWindow, &MainWindow::onRegionGroupChanged);
    QObject::connect(sidePanelWidget, &SidePanelWidget::regionTypeChanged, mainWindow, [mainWindow, documentState, canvas](const QString& regionName, const QString& newType) {
        if (!mainWindow || !documentState || !canvas || !documentState->hasRegion(regionName)) {
            return;
        }
        mainWindow->regionEditorHandlers->onRegionTypeChanged(
            mainWindow,
            documentState,
            canvas,
            regionName,
            newType,
            [documentState]() { documentState->saveState(); },
            [canvas]() { if (canvas) canvas->invalidateCoordinateCache(); },
            [canvas]() { if (canvas) canvas->update(); },
            [mainWindow]() { mainWindow->updateUndoRedoButtons(); }
        );
    });
    QObject::connect(sidePanelWidget, &SidePanelWidget::regionPercentageFillChanged, mainWindow, [mainWindow, documentState, canvas](const QString& regionName, const QString& newPercentageFill) {
        if (!mainWindow || !documentState || !canvas || !documentState->hasRegion(regionName)) {
            return;
        }
        mainWindow->regionEditorHandlers->onRegionPercentageFillChanged(
            mainWindow,
            documentState,
            canvas,
            regionName,
            newPercentageFill,
            [documentState]() { documentState->saveState(); },
            [canvas]() { if (canvas) canvas->invalidateCoordinateCache(); },
            [canvas]() { if (canvas) canvas->update(); },
            [mainWindow]() { mainWindow->updateUndoRedoButtons(); }
        );
    });
    QObject::connect(sidePanelWidget, &SidePanelWidget::regionCoordinatesChanged, mainWindow, [mainWindow, documentState, canvas](const QString& regionName, double x1, double y1, double x2, double y2) {
        if (!mainWindow || !documentState || !canvas || !documentState->hasRegion(regionName)) {
            return;
        }
        mainWindow->regionEditorHandlers->onRegionCoordinatesChanged(
            mainWindow,
            documentState,
            canvas,
            regionName,
            x1, y1, x2, y2,
            [documentState]() { documentState->saveState(); },
            [canvas]() { if (canvas) canvas->invalidateCoordinateCache(); },
            [canvas]() { if (canvas) canvas->update(); },
            [mainWindow]() { mainWindow->updateRegionListBox(); },
            [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
            [canvas]() { return canvas ? canvas->getDocumentImage() : QImage(); },
            [canvas]() { return canvas ? canvas->getScaleFactor() : 1.0; },
            [canvas]() { return canvas ? canvas->getImageOffset() : QPointF(); },
            [documentState]() { documentState->synchronizeCoordinates(); }
        );
    });
    
    // Connect canvas selection to show/hide region editor (using CanvasUiSync)
    QObject::connect(canvas, &Canvas::selectionChanged, mainWindow, [mainWindow, sidePanelWidget, documentState, canvas]() {
        if (!mainWindow || !sidePanelWidget || !documentState || !canvas) {
            return;
        }
        
        QSet<QString> selectedRegions = canvas->getSelectedRegions();
        
        // Clear hover state when selection changes to prevent old resize handles from showing
        canvas->setHoveredRegion(QString());
        
        // Use CanvasUiSync to handle selection change with proper refresh order
        CanvasUiSync::handleSelectionChanged(
            selectedRegions,
            canvas->getPrimarySelectedRegion(),
            documentState,
            [canvas]() { if (canvas) canvas->invalidateCoordinateCache(); },
            [mainWindow]() { if (mainWindow) mainWindow->updateRegionListBox(); },
            [mainWindow]() { if (mainWindow) mainWindow->updateGroupListBox(); },
            [canvas]() { if (canvas) canvas->update(); },
            [sidePanelWidget](const QString& regionName, const QString& color, const QString& group,
                              const QList<QString>& availableGroups, const QString& regionType,
                              const QString& percentageFill, double x1, double y1, double x2, double y2) {
                if (sidePanelWidget) {
                    sidePanelWidget->showRegionEditor(regionName, color, group, availableGroups,
                                                      regionType, percentageFill, x1, y1, x2, y2);
                }
            },
            [sidePanelWidget]() { if (sidePanelWidget) sidePanelWidget->hideRegionEditor(); },
            [documentState](const QString& name) { return documentState->getRegion(name); },
            [documentState]() { return documentState->getGroupNames(); }
        );
    });
}

} // namespace ocr_orc

