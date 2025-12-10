#include "UiDataAdapters.h"
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"
#include "../../../models/GroupData.h"

namespace ocr_orc {

UiDataAdapters::RegionListModel UiDataAdapters::toRegionListModel(DocumentState* documentState,
                                                                  const QSet<QString>& selectedRegions) {
    RegionListModel model;
    
    if (!documentState) {
        return model;
    }
    
    // Get all region names
    model.regionNames = documentState->getAllRegionNames();
    
    // Set selected regions (filter to only include valid regions)
    for (const QString& name : selectedRegions) {
        if (documentState->hasRegion(name)) {
            model.selectedRegions.insert(name);
        }
    }
    
    // Build region name → group name mapping
    for (const QString& name : model.regionNames) {
        if (documentState->hasRegion(name)) {
            RegionData region = documentState->getRegion(name);
            if (!region.group.isEmpty()) {
                model.regionGroups.insert(name, region.group);
            }
        }
    }
    
    return model;
}

UiDataAdapters::GroupListModel UiDataAdapters::toGroupListModel(DocumentState* documentState) {
    GroupListModel model;
    
    if (!documentState) {
        return model;
    }
    
    // Get all group names
    model.groupNames = documentState->getGroupNames();
    
    // Build group name → size mapping
    for (const QString& name : model.groupNames) {
        if (documentState->hasGroup(name)) {
            GroupData group = documentState->getGroup(name);
            model.groupSizes.insert(name, group.regionNames.size());
        }
    }
    
    return model;
}

QString UiDataAdapters::getSelectionStatusString(int selectedCount, int totalCount) {
    if (selectedCount == 0) {
        return QString("No selection");
    } else if (selectedCount == 1) {
        return QString("1 region selected");
    } else if (selectedCount == totalCount) {
        return QString("All %1 regions selected").arg(totalCount);
    } else {
        return QString("%1 of %2 regions selected").arg(selectedCount).arg(totalCount);
    }
}

QString UiDataAdapters::getRegionCountStatusString(int regionCount) {
    if (regionCount == 0) {
        return QString("No regions");
    } else if (regionCount == 1) {
        return QString("1 region");
    } else {
        return QString("%1 regions").arg(regionCount);
    }
}

QString UiDataAdapters::getGroupCountStatusString(int groupCount) {
    if (groupCount == 0) {
        return QString("No groups");
    } else if (groupCount == 1) {
        return QString("1 group");
    } else {
        return QString("%1 groups").arg(groupCount);
    }
}

} // namespace ocr_orc

