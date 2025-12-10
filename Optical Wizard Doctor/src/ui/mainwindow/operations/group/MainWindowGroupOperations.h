#ifndef MAIN_WINDOW_GROUP_OPERATIONS_H
#define MAIN_WINDOW_GROUP_OPERATIONS_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidgetItem>
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;
class SidePanelWidget;
struct GroupData;

/**
 * @brief Handles all group operations for MainWindow
 * 
 * Manages:
 * - Group creation from selected regions
 * - Adding regions to existing groups
 * - Removing regions from groups
 * - Group deletion
 * - Selection management (canvas + listbox)
 */
class MainWindowGroupOperations {
public:
    // Callback types for UI updates
    using GetSelectedRegionsCallback = std::function<QSet<QString>()>;
    using ShowWarningCallback = std::function<void(const QString&, const QString&)>;
    using ShowInformationCallback = std::function<void(const QString&, const QString&)>;
    using ShowQuestionCallback = std::function<int(const QString&, const QString&)>;
    using GetTextInputCallback = std::function<QString(bool*)>;
    using SaveStateCallback = std::function<void()>;
    using CreateGroupCallback = std::function<void(const QString&)>;
    using AddRegionToGroupCallback = std::function<void(const QString&, const QString&)>;
    using RemoveRegionFromGroupCallback = std::function<void(const QString&)>;
    using DeleteGroupCallback = std::function<void(const QString&)>;
    using HasGroupCallback = std::function<bool(const QString&)>;
    using GetGroupCallback = std::function<GroupData(const QString&)>;
    using SynchronizeCoordinatesCallback = std::function<void()>;
    using UpdateGroupListBoxCallback = std::function<void()>;
    using UpdateRegionListBoxCallback = std::function<void()>;
    using UpdateCanvasCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void()>;
    using ShowStatusMessageCallback = std::function<void(const QString&, int)>;
    using SetInfoTextCallback = std::function<void(const QString&)>;
    using GetRegionGroupCallback = std::function<QString(const QString&)>;
    using HasRegionCallback = std::function<bool(const QString&)>;
    using IsCreatingGroupCallback = std::function<bool()>;
    using SetIsCreatingGroupCallback = std::function<void(bool)>;
    using GetGroupNamesCallback = std::function<QList<QString>()>;
    using GetSelectedGroupCallback = std::function<QString()>;
    using GetCanvasSelectedRegionsCallback = std::function<QSet<QString>()>;
    using GetListBoxSelectedItemsCallback = std::function<QList<QListWidgetItem*>()>;
    
    MainWindowGroupOperations();
    ~MainWindowGroupOperations();
    
    /**
     * @brief Create a group from selected regions
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void createGroup(QWidget* parentWidget,
                     DocumentState* documentState,
                     Canvas* canvas,
                     const GetSelectedRegionsCallback& getSelectedRegions,
                     const ShowWarningCallback& showWarning,
                     const GetTextInputCallback& getTextInput,
                     const HasGroupCallback& hasGroup,
                     const ShowQuestionCallback& showQuestion,
                     const SaveStateCallback& saveState,
                     const CreateGroupCallback& createGroup,
                     const AddRegionToGroupCallback& addRegionToGroup,
                     const SynchronizeCoordinatesCallback& synchronizeCoordinates,
                     const UpdateGroupListBoxCallback& updateGroupListBox,
                     const UpdateRegionListBoxCallback& updateRegionListBox,
                     const UpdateCanvasCallback& updateCanvas,
                     const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                     const ShowStatusMessageCallback& showStatusMessage,
                     const IsCreatingGroupCallback& isCreatingGroup,
                     const SetIsCreatingGroupCallback& setIsCreatingGroup);
    
    /**
     * @brief Add selected regions to a group
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void addToGroup(QWidget* parentWidget,
                   DocumentState* documentState,
                   Canvas* canvas,
                   const GetSelectedRegionsCallback& getSelectedRegions,
                   const ShowWarningCallback& showWarning,
                   const GetTextInputCallback& getTextInput,
                   const SaveStateCallback& saveState,
                   const HasRegionCallback& hasRegion,
                   const AddRegionToGroupCallback& addRegionToGroup,
                   const SynchronizeCoordinatesCallback& synchronizeCoordinates,
                   const UpdateGroupListBoxCallback& updateGroupListBox,
                   const UpdateRegionListBoxCallback& updateRegionListBox,
                   const UpdateCanvasCallback& updateCanvas,
                   const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                   const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Remove selected regions from their groups
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void removeFromGroup(QWidget* parentWidget,
                        DocumentState* documentState,
                        Canvas* canvas,
                        const GetSelectedRegionsCallback& getSelectedRegions,
                        const ShowWarningCallback& showWarning,
                        const ShowInformationCallback& showInformation,
                        const GetRegionGroupCallback& getRegionGroup,
                        const HasRegionCallback& hasRegion,
                        const GetGroupNamesCallback& getGroupNames,
                        const GetGroupCallback& getGroup,
                        const SaveStateCallback& saveState,
                        const RemoveRegionFromGroupCallback& removeRegionFromGroup,
                        const SynchronizeCoordinatesCallback& synchronizeCoordinates,
                        const UpdateGroupListBoxCallback& updateGroupListBox,
                        const UpdateRegionListBoxCallback& updateRegionListBox,
                        const UpdateCanvasCallback& updateCanvas,
                        const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                        const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Delete a group
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param sidePanelWidget Side panel widget
     * @param callbacks Callback functions
     */
    void deleteGroup(QWidget* parentWidget,
                    DocumentState* documentState,
                    Canvas* canvas,
                    SidePanelWidget* sidePanelWidget,
                    const GetSelectedGroupCallback& getSelectedGroup,
                    const ShowWarningCallback& showWarning,
                    const HasGroupCallback& hasGroup,
                    const GetGroupCallback& getGroup,
                    const ShowQuestionCallback& showQuestion,
                    const SaveStateCallback& saveState,
                    const DeleteGroupCallback& deleteGroup,
                    const SynchronizeCoordinatesCallback& synchronizeCoordinates,
                    const UpdateGroupListBoxCallback& updateGroupListBox,
                    const UpdateRegionListBoxCallback& updateRegionListBox,
                    const UpdateCanvasCallback& updateCanvas,
                    const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                    const SetInfoTextCallback& setInfoText,
                    const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Get selected regions from canvas or listbox
     * @param canvas Canvas widget
     * @param sidePanelWidget Side panel widget
     * @param documentState Document state
     * @param callbacks Callback functions
     * @return Set of selected region names
     */
    QSet<QString> getSelectedRegions(Canvas* canvas,
                                   SidePanelWidget* sidePanelWidget,
                                   DocumentState* documentState,
                                   const GetCanvasSelectedRegionsCallback& getCanvasSelectedRegions,
                                   const GetListBoxSelectedItemsCallback& getListBoxSelectedItems,
                                   const HasRegionCallback& hasRegion) const;
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_GROUP_OPERATIONS_H

