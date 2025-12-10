#ifndef MAIN_WINDOW_UI_UPDATES_H
#define MAIN_WINDOW_UI_UPDATES_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;
class SidePanelWidget;
class ToolbarWidget;
struct RegionData;
struct GroupData;

/**
 * @brief Handles UI update operations for MainWindow
 * 
 * Manages:
 * - Region list box updates
 * - Group list box updates
 * - Zoom label updates
 */
class MainWindowUIUpdates {
public:
    // Callback types
    using GetAllRegionNamesCallback = std::function<QList<QString>()>;
    using GetSelectedRegionsCallback = std::function<QSet<QString>()>;
    using GetRegionCallback = std::function<RegionData(const QString&)>;
    using GetGroupNamesCallback = std::function<QList<QString>()>;
    using GetGroupCallback = std::function<GroupData(const QString&)>;
    using GetZoomCallback = std::function<double()>;
    using UpdateRegionListBoxCallback = std::function<void(const QList<QString>&, const QSet<QString>&, const QMap<QString, QString>&)>;
    using UpdateGroupListBoxCallback = std::function<void(const QList<QString>&, const QMap<QString, int>&)>;
    using UpdateZoomLabelCallback = std::function<void(double)>;
    
    MainWindowUIUpdates();
    ~MainWindowUIUpdates();
    
    /**
     * @brief Update the region list box
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param sidePanelWidget Side panel widget
     * @param callbacks Callback functions
     */
    void updateRegionListBox(DocumentState* documentState,
                            Canvas* canvas,
                            SidePanelWidget* sidePanelWidget,
                            const GetAllRegionNamesCallback& getAllRegionNames,
                            const GetSelectedRegionsCallback& getSelectedRegions,
                            const GetRegionCallback& getRegion,
                            const UpdateRegionListBoxCallback& updateRegionListBox);
    
    /**
     * @brief Update the group list box
     * @param documentState Document state
     * @param sidePanelWidget Side panel widget
     * @param callbacks Callback functions
     */
    void updateGroupListBox(DocumentState* documentState,
                           SidePanelWidget* sidePanelWidget,
                           const GetGroupNamesCallback& getGroupNames,
                           const GetGroupCallback& getGroup,
                           const UpdateGroupListBoxCallback& updateGroupListBox);
    
    /**
     * @brief Update the zoom label
     * @param canvas Canvas widget
     * @param toolbarWidget Toolbar widget
     * @param callbacks Callback functions
     */
    void updateZoomLabel(Canvas* canvas,
                        ToolbarWidget* toolbarWidget,
                        const GetZoomCallback& getZoom,
                        const UpdateZoomLabelCallback& updateZoomLabel);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_UI_UPDATES_H

