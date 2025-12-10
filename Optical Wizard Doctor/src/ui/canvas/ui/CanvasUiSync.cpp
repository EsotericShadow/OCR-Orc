#include "CanvasUiSync.h"
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"

namespace ocr_orc {

void CanvasUiSync::handleSelectionChanged(const QSet<QString>& selectedRegions,
                                         const QString& primarySelectedRegion,
                                         DocumentState* documentState,
                                         const InvalidateCacheCallback& invalidateCache,
                                         const UpdateRegionListCallback& updateRegionList,
                                         const UpdateGroupListCallback& updateGroupList,
                                         const UpdateCanvasCallback& updateCanvas,
                                         const ShowRegionEditorCallback& showEditor,
                                         const HideRegionEditorCallback& hideEditor,
                                         const GetRegionCallback& getRegion,
                                         const GetGroupNamesCallback& getGroupNames) {
    if (!documentState) {
        return;
    }
    
    // Step 1: Invalidate coordinate cache (selection affects hit-testing)
    if (invalidateCache) {
        invalidateCache();
    }
    
    // Step 2: Show/hide region editor based on selection
    if (selectedRegions.isEmpty()) {
        // Hide editor for no selection
        if (hideEditor) {
            hideEditor();
        }
    } else {
        // Show editor for the primary selected region
        QString regionName = primarySelectedRegion;
        if (!regionName.isEmpty() && documentState->hasRegion(regionName)) {
            RegionData region = getRegion(regionName);
            QList<QString> availableGroups = getGroupNames();
            if (showEditor) {
                showEditor(regionName, region.color, region.group, availableGroups,
                          region.regionType, region.percentageFill,
                          region.normalizedCoords.x1, region.normalizedCoords.y1,
                          region.normalizedCoords.x2, region.normalizedCoords.y2);
            }
        }
    }
    
    // Step 3: Update region/group lists (selection may affect display)
    if (updateRegionList) {
        updateRegionList();
    }
    if (updateGroupList) {
        updateGroupList();
    }
    
    // Step 4: Repaint canvas (selection visuals)
    if (updateCanvas) {
        updateCanvas();
    }
}

void CanvasUiSync::handleStateChanged(DocumentState* documentState,
                                     const InvalidateCacheCallback& invalidateCache,
                                     const UpdateRegionListCallback& updateRegionList,
                                     const UpdateGroupListCallback& updateGroupList,
                                     const UpdateCanvasCallback& updateCanvas,
                                     const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons) {
    if (!documentState) {
        return;
    }
    
    // Full refresh sequence for state changes
    refreshAll(documentState, invalidateCache, updateRegionList, updateGroupList,
              updateCanvas, updateUndoRedoButtons);
}

void CanvasUiSync::refreshAll(DocumentState* documentState,
                             const InvalidateCacheCallback& invalidateCache,
                             const UpdateRegionListCallback& updateRegionList,
                             const UpdateGroupListCallback& updateGroupList,
                             const UpdateCanvasCallback& updateCanvas,
                             const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons) {
    if (!documentState) {
        return;
    }
    
    // Enforce refresh order:
    // 1. Invalidate coordinate cache
    if (invalidateCache) {
        invalidateCache();
    }
    
    // 2. Update region/group lists
    if (updateRegionList) {
        updateRegionList();
    }
    if (updateGroupList) {
        updateGroupList();
    }
    
    // 3. Repaint canvas
    if (updateCanvas) {
        updateCanvas();
    }
    
    // 4. Update toolbar buttons (undo/redo state)
    if (updateUndoRedoButtons) {
        updateUndoRedoButtons();
    }
}

} // namespace ocr_orc

