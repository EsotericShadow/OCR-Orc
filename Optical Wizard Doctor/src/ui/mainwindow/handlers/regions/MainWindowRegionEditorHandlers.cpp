#include "MainWindowRegionEditorHandlers.h"
#include "../../../MainWindow.h"
#include "../../../../models/DocumentState.h"
#include "../../../../models/RegionData.h"
#include "../../../canvas/Canvas.h"
#include "../../../../core/CoordinateSystem.h"
#include <QtCore/QString>

namespace ocr_orc {

MainWindowRegionEditorHandlers::MainWindowRegionEditorHandlers() {
}

MainWindowRegionEditorHandlers::~MainWindowRegionEditorHandlers() {
}

void MainWindowRegionEditorHandlers::onRegionNameChanged(MainWindow* mainWindow, const QString& oldName, const QString& newName) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    // Delegate to existing handler in MainWindowRegionGroupHandlers
    mainWindow->regionGroupHandlers->onRegionNameChanged(mainWindow, oldName, newName);
}

void MainWindowRegionEditorHandlers::onRegionColorChanged(MainWindow* mainWindow, const QString& regionName, const QString& newColor) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    // Delegate to existing handler in MainWindowRegionGroupHandlers
    mainWindow->regionGroupHandlers->onRegionColorChanged(mainWindow, regionName, newColor);
}

void MainWindowRegionEditorHandlers::onRegionGroupChanged(MainWindow* mainWindow, const QString& regionName, const QString& newGroup) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    // Delegate to existing handler in MainWindowRegionGroupHandlers
    mainWindow->regionGroupHandlers->onRegionGroupChanged(mainWindow, regionName, newGroup);
}

void MainWindowRegionEditorHandlers::onRegionTypeChanged(MainWindow* /* mainWindow */,
                                                         DocumentState* documentState,
                                                         Canvas* canvas,
                                                         const QString& regionName,
                                                         const QString& newType,
                                                         const SaveStateCallback& saveState,
                                                         const InvalidateCacheCallback& invalidateCache,
                                                         const UpdateCanvasCallback& updateCanvas,
                                                         const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons) {
    if (!documentState || !canvas || !documentState->hasRegion(regionName)) {
        return;
    }
    
    // Save state BEFORE modification for undo/redo
    saveState();
    
    RegionData& region = documentState->regions[regionName];
    region.regionType = newType;
    
    invalidateCache();
    updateCanvas();
    updateUndoRedoButtons();
}

void MainWindowRegionEditorHandlers::onRegionPercentageFillChanged(MainWindow* /* mainWindow */,
                                                                   DocumentState* documentState,
                                                                   Canvas* canvas,
                                                                   const QString& regionName,
                                                                   const QString& newPercentageFill,
                                                                   const SaveStateCallback& saveState,
                                                                   const InvalidateCacheCallback& invalidateCache,
                                                                   const UpdateCanvasCallback& updateCanvas,
                                                                   const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons) {
    if (!documentState || !canvas || !documentState->hasRegion(regionName)) {
        return;
    }
    
    // Save state BEFORE modification for undo/redo
    saveState();
    
    RegionData& region = documentState->regions[regionName];
    region.percentageFill = newPercentageFill;
    
    invalidateCache();
    updateCanvas();
    updateUndoRedoButtons();
}

void MainWindowRegionEditorHandlers::onRegionCoordinatesChanged(MainWindow* /* mainWindow */,
                                                                DocumentState* documentState,
                                                                Canvas* canvas,
                                                                const QString& regionName,
                                                                double x1, double y1, double x2, double y2,
                                                                const SaveStateCallback& saveState,
                                                                const InvalidateCacheCallback& invalidateCache,
                                                                const UpdateCanvasCallback& updateCanvas,
                                                                const UpdateRegionListBoxCallback& updateRegionListBox,
                                                                const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                                                const GetDocumentImageCallback& getDocumentImage,
                                                                const GetScaleFactorCallback& getScaleFactor,
                                                                const GetImageOffsetCallback& getImageOffset,
                                                                const SynchronizeCoordinatesCallback& synchronizeCoordinates) {
    if (!documentState || !canvas || !documentState->hasRegion(regionName)) {
        return;
    }
    
    // Validate coordinates for NaN/Infinity
    NormalizedCoords coords(x1, y1, x2, y2);
    if (!CoordinateSystem::isValidNormalizedCoords(coords)) {
        return; // Invalid coordinates
    }
    
    // Save state BEFORE modification for undo/redo
    saveState();
    
    RegionData& region = documentState->regions[regionName];
    region.normalizedCoords.x1 = x1;
    region.normalizedCoords.y1 = y1;
    region.normalizedCoords.x2 = x2;
    region.normalizedCoords.y2 = y2;
    
    // Sync coordinates using actual canvas scale and offset
    QImage docImage = getDocumentImage();
    if (!docImage.isNull()) {
        double scaleFactor = getScaleFactor();
        QPointF imageOffset = getImageOffset();
        region.syncFromNormalized(docImage.width(), docImage.height(), scaleFactor, imageOffset);
    }
    
    // Synchronize all coordinates in document state to ensure consistency
    synchronizeCoordinates();
    
    // Invalidate coordinate cache to force recalculation
    invalidateCache();
    
    // Force canvas repaint
    updateCanvas();
    updateRegionListBox();
    updateUndoRedoButtons();
}

} // namespace ocr_orc

