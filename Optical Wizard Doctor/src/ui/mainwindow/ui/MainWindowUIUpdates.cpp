#include "MainWindowUIUpdates.h"
#include "../adapters/UiDataAdapters.h"
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"
#include "../../../models/GroupData.h"
#include "../../canvas/Canvas.h"
#include "../../components/widgets/SidePanelWidget.h"
#include "../../components/widgets/ToolbarWidget.h"

namespace ocr_orc {

MainWindowUIUpdates::MainWindowUIUpdates() {
}

MainWindowUIUpdates::~MainWindowUIUpdates() {
}

void MainWindowUIUpdates::updateRegionListBox(DocumentState* documentState,
                                             Canvas* /* canvas */,
                                             SidePanelWidget* sidePanelWidget,
                                             const GetAllRegionNamesCallback& /* getAllRegionNames */,
                                             const GetSelectedRegionsCallback& getSelectedRegions,
                                             const GetRegionCallback& /* getRegion */,
                                             const UpdateRegionListBoxCallback& updateRegionListBox) {
    if (!sidePanelWidget || !documentState) {
        return;
    }
    
    // Use UiDataAdapters for pure data transformation
    QSet<QString> selectedRegions = getSelectedRegions();
    UiDataAdapters::RegionListModel model = UiDataAdapters::toRegionListModel(documentState, selectedRegions);
    
    if (updateRegionListBox) {
        updateRegionListBox(model.regionNames, model.selectedRegions, model.regionGroups);
    }
}

void MainWindowUIUpdates::updateGroupListBox(DocumentState* documentState,
                                            SidePanelWidget* sidePanelWidget,
                                            const GetGroupNamesCallback& /* getGroupNames */,
                                            const GetGroupCallback& /* getGroup */,
                                            const UpdateGroupListBoxCallback& updateGroupListBox) {
    if (!sidePanelWidget || !documentState) {
        return;
    }
    
    // Use UiDataAdapters for pure data transformation
    UiDataAdapters::GroupListModel model = UiDataAdapters::toGroupListModel(documentState);
    
    if (updateGroupListBox) {
        updateGroupListBox(model.groupNames, model.groupSizes);
    }
}

void MainWindowUIUpdates::updateZoomLabel(Canvas* canvas,
                                         ToolbarWidget* toolbarWidget,
                                         const GetZoomCallback& getZoom,
                                         const UpdateZoomLabelCallback& updateZoomLabel) {
    if (!canvas || !toolbarWidget) {
        return;
    }
    
    double zoom = getZoom();
    if (updateZoomLabel) {
        updateZoomLabel(zoom);
    }
}

} // namespace ocr_orc

