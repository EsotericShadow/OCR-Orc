#include "MainWindowSelectionOperations.h"
#include "../../../../models/DocumentState.h"
#include "../../../canvas/Canvas.h"

namespace ocr_orc {

MainWindowSelectionOperations::MainWindowSelectionOperations() {
}

MainWindowSelectionOperations::~MainWindowSelectionOperations() {
}

void MainWindowSelectionOperations::selectAll(DocumentState* documentState,
                                             Canvas* canvas,
                                             const GetAllRegionNamesCallback& getAllRegionNames,
                                             const SetSelectedRegionsCallback& setSelectedRegions,
                                             const UpdateCanvasCallback& updateCanvas,
                                             const UpdateRegionListBoxCallback& updateRegionListBox) {
    if (!documentState || !canvas) {
        return;
    }
    
    QList<QString> allRegions = getAllRegionNames();
    if (allRegions.isEmpty()) {
        return;
    }
    
    QSet<QString> regionSet;
    for (const QString& name : allRegions) {
        regionSet.insert(name);
    }
    
    if (setSelectedRegions) {
        setSelectedRegions(regionSet);
    }
    if (updateCanvas) {
        updateCanvas();
    }
    if (updateRegionListBox) {
        updateRegionListBox();
    }
}

void MainWindowSelectionOperations::invertSelection(DocumentState* documentState,
                                                   Canvas* canvas,
                                                   const GetAllRegionNamesCallback& getAllRegionNames,
                                                   const GetSelectedRegionsCallback& getSelectedRegions,
                                                   const SetSelectedRegionsCallback& setSelectedRegions,
                                                   const UpdateCanvasCallback& updateCanvas,
                                                   const UpdateRegionListBoxCallback& updateRegionListBox) {
    if (!documentState || !canvas) {
        return;
    }
    
    QList<QString> allRegions = getAllRegionNames();
    QSet<QString> currentSelection = getSelectedRegions();
    
    QSet<QString> newSelection;
    for (const QString& name : allRegions) {
        if (!currentSelection.contains(name)) {
            newSelection.insert(name);
        }
    }
    
    if (setSelectedRegions) {
        setSelectedRegions(newSelection);
    }
    if (updateCanvas) {
        updateCanvas();
    }
    if (updateRegionListBox) {
        updateRegionListBox();
    }
}

} // namespace ocr_orc

