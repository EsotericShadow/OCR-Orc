#ifndef CANVAS_UI_SYNC_H
#define CANVAS_UI_SYNC_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <functional>

namespace ocr_orc {

// Forward declarations
class Canvas;
class DocumentState;
class SidePanelWidget;

/**
 * @brief Centralizes UI refresh logic for canvas state changes
 * 
 * Ensures consistent refresh order:
 * 1. Invalidate coordinate cache
 * 2. Update region/group lists
 * 3. Repaint canvas
 * 4. Update toolbar buttons (if needed)
 * 
 * Manages region editor show/hide based on selection changes.
 */
class CanvasUiSync {
public:
    // Callback types
    using InvalidateCacheCallback = std::function<void()>;
    using UpdateRegionListCallback = std::function<void()>;
    using UpdateGroupListCallback = std::function<void()>;
    using UpdateCanvasCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void()>;
    using ShowRegionEditorCallback = std::function<void(const QString&, const QString&, const QString&, const QList<QString>&, const QString&, const QString&, double, double, double, double)>;
    using HideRegionEditorCallback = std::function<void()>;
    using GetRegionCallback = std::function<struct RegionData(const QString&)>;
    using GetGroupNamesCallback = std::function<QList<QString>()>;
    
    /**
     * @brief Handle selection change - refresh UI and show/hide editor
     * @param selectedRegions Current selection set
     * @param primarySelectedRegion Primary selected region name
     * @param documentState Document state
     * @param callbacks UI update callbacks
     */
    static void handleSelectionChanged(const QSet<QString>& selectedRegions,
                                      const QString& primarySelectedRegion,
                                      DocumentState* documentState,
                                      const InvalidateCacheCallback& invalidateCache,
                                      const UpdateRegionListCallback& updateRegionList,
                                      const UpdateGroupListCallback& updateGroupList,
                                      const UpdateCanvasCallback& updateCanvas,
                                      const ShowRegionEditorCallback& showEditor,
                                      const HideRegionEditorCallback& hideEditor,
                                      const GetRegionCallback& getRegion,
                                      const GetGroupNamesCallback& getGroupNames);
    
    /**
     * @brief Handle state change - refresh all UI components
     * @param documentState Document state
     * @param callbacks UI update callbacks
     */
    static void handleStateChanged(DocumentState* documentState,
                                  const InvalidateCacheCallback& invalidateCache,
                                  const UpdateRegionListCallback& updateRegionList,
                                  const UpdateGroupListCallback& updateGroupList,
                                  const UpdateCanvasCallback& updateCanvas,
                                  const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons);
    
    /**
     * @brief Full UI refresh sequence (for undo/redo)
     * Order: invalidate cache → update lists → repaint canvas → update buttons
     */
    static void refreshAll(DocumentState* documentState,
                          const InvalidateCacheCallback& invalidateCache,
                          const UpdateRegionListCallback& updateRegionList,
                          const UpdateGroupListCallback& updateGroupList,
                          const UpdateCanvasCallback& updateCanvas,
                          const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons);
};

} // namespace ocr_orc

#endif // CANVAS_UI_SYNC_H

