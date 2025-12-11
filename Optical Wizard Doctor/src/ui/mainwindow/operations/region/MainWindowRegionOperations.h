#ifndef MAIN_WINDOW_REGION_OPERATIONS_H
#define MAIN_WINDOW_REGION_OPERATIONS_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtWidgets/QWidget>
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;
class ControlPanelWidget;
struct RegionData;

/**
 * @brief Handles all region operations for MainWindow
 * 
 * Manages:
 * - Region creation workflow
 * - Region editing (name, color)
 * - Region deletion
 * - Region name generation and auto-increment
 */
class MainWindowRegionOperations {
public:
    // Callback types for UI updates
    using UpdateRegionListBoxCallback = std::function<void()>;
    using UpdateGroupListBoxCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void()>;
    using ShowStatusMessageCallback = std::function<void(const QString&, int)>;
    using ShowWarningCallback = std::function<void(const QString&, const QString&)>;
    using ClearSelectionCallback = std::function<void()>;
    using SelectRegionCallback = std::function<void(const QString&)>;
    using UpdateCanvasCallback = std::function<void()>;
    using SetRegionNameEditCallback = std::function<void(const QString&)>;
    using GetRegionNameCallback = std::function<QString()>;
    using GetSelectedColorCallback = std::function<QString()>;
    using GetGroupNameCallback = std::function<QString()>;
    using ClearGroupNameCallback = std::function<void()>;
    using FinishRegionCreationCallback = std::function<bool(const QString&, const QString&, const QString&)>;
    using HasRegionCallback = std::function<bool(const QString&)>;
    using SaveStateCallback = std::function<void()>;
    using RenameRegionCallback = std::function<bool(const QString&, const QString&)>;
    using ChangeRegionColorCallback = std::function<bool(const QString&, const QString&)>;
    using RemoveRegionCallback = std::function<void(const QString&)>;
    using GetSelectedRegionsCallback = std::function<QSet<QString>()>;
    using GetRegionCallback = std::function<RegionData(const QString&)>;
    using IsValidRegionNameCallback = std::function<bool(const QString&, const QString&)>;
    using GenerateRegionNameCallback = std::function<QString()>;
    using AutoIncrementRegionNameCallback = std::function<void()>;
    using ShowEditNameDialogCallback = std::function<QString(const QString&)>;
    using ShowQuestionCallback = std::function<int(const QString&, const QString&)>;
    using ShowInformationCallback = std::function<void(const QString&, const QString&)>;
    using SynchronizeCoordinatesCallback = std::function<void()>;
    using ShowChangeColorDialogCallback = std::function<QString(const QString&)>;
    
    MainWindowRegionOperations();
    ~MainWindowRegionOperations();
    
    /**
     * @brief Handle region creation request from canvas
     * @param canvas Canvas widget
     * @param documentState Document state
     * @param controlPanelWidget Control panel widget
     * @param callbacks Callback functions for operations
     */
    void handleRegionCreationRequested(Canvas* canvas,
                                       DocumentState* documentState,
                                       ControlPanelWidget* controlPanelWidget,
                                       const GetRegionNameCallback& getRegionName,
                                       const GetSelectedColorCallback& getSelectedColor,
                                       const GetGroupNameCallback& getGroupName,
                                       const ClearGroupNameCallback& clearGroupName,
                                       const FinishRegionCreationCallback& finishRegionCreation,
                                       const HasRegionCallback& hasRegion,
                                       const GenerateRegionNameCallback& generateRegionName,
                                       const AutoIncrementRegionNameCallback& autoIncrementRegionName);
    
    /**
     * @brief Handle region created signal
     * @param regionName Name of created region
     * @param callbacks Callback functions for UI updates
     */
    void handleRegionCreated(const QString& regionName,
                            const UpdateRegionListBoxCallback& updateRegionListBox,
                            const ShowStatusMessageCallback& showStatusMessage,
                            const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons);
    
    /**
     * @brief Handle regions duplicated signal
     * @param duplicatedNames List of duplicated region names
     * @param callbacks Callback functions for UI updates
     */
    void handleRegionsDuplicated(const QList<QString>& duplicatedNames,
                                const UpdateRegionListBoxCallback& updateRegionListBox,
                                const ShowStatusMessageCallback& showStatusMessage,
                                const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons);
    
    /**
     * @brief Generate a unique region name
     * @param documentState Document state to check for existing names
     * @return Generated region name
     */
    QString generateRegionName(DocumentState* documentState) const;
    
    /**
     * @brief Show dialog to get region name when creating a new region
     * @param parentWidget Parent widget for dialog
     * @param documentState Document state to generate default name (used if callback not provided)
     * @param generateRegionNameCallback Optional callback to generate default name
     * @return Region name if accepted, empty string if cancelled
     */
    QString showCreateRegionNameDialog(QWidget* parentWidget, DocumentState* documentState, const GenerateRegionNameCallback& generateRegionNameCallback = nullptr) const;
    
    /**
     * @brief Auto-increment region name in control panel
     * @param controlPanelWidget Control panel widget
     * @param documentState Document state to check for existing names
     * @param callbacks Callback functions
     */
    void autoIncrementRegionName(ControlPanelWidget* controlPanelWidget,
                                DocumentState* documentState,
                                const GetRegionNameCallback& getRegionName,
                                const SetRegionNameEditCallback& setRegionNameEdit,
                                const HasRegionCallback& hasRegion);
    
    /**
     * @brief Edit selected region (rename)
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void editSelected(QWidget* parentWidget,
                     DocumentState* documentState,
                     Canvas* canvas,
                     const GetSelectedRegionsCallback& getSelectedRegions,
                     const ShowEditNameDialogCallback& showEditNameDialog,
                     const ShowWarningCallback& showWarning,
                     const SaveStateCallback& saveState,
                     const RenameRegionCallback& renameRegion,
                     const ClearSelectionCallback& clearSelection,
                     const SelectRegionCallback& selectRegion,
                     const UpdateRegionListBoxCallback& updateRegionListBox,
                     const UpdateGroupListBoxCallback& updateGroupListBox,
                     const UpdateCanvasCallback& updateCanvas,
                     const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                     const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Show edit name dialog
     * @param parentWidget Parent widget
     * @param currentName Current region name
     * @return New name if accepted, empty string if cancelled
     */
    QString showEditNameDialog(QWidget* parentWidget, const QString& currentName) const;
    
    /**
     * @brief Show change color dialog
     * @param parentWidget Parent widget
     * @param documentState Document state
     * @param regionName Region name
     * @param callbacks Callback functions
     * @return New color if accepted, empty string if cancelled
     */
    QString showChangeColorDialog(QWidget* parentWidget,
                                 DocumentState* documentState,
                                 const QString& regionName,
                                 const GetRegionCallback& getRegion) const;
    
    /**
     * @brief Handle region edit request from signal (double-click)
     * @param regionName Region name to edit
     * @param documentState Document state
     * @param callbacks Callback functions
     */
    void handleRegionEditRequested(const QString& regionName,
                                   DocumentState* documentState,
                                   const ShowEditNameDialogCallback& showEditNameDialog,
                                   const ShowWarningCallback& showWarning,
                                   const SaveStateCallback& saveState,
                                   const RenameRegionCallback& renameRegion,
                                   const IsValidRegionNameCallback& isValidRegionName,
                                   const ClearSelectionCallback& clearSelection,
                                   const SelectRegionCallback& selectRegion,
                                   const UpdateRegionListBoxCallback& updateRegionListBox,
                                   const UpdateGroupListBoxCallback& updateGroupListBox,
                                   const UpdateCanvasCallback& updateCanvas,
                                   const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                   const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Handle region color change request from signal
     * @param regionName Region name to change color
     * @param documentState Document state
     * @param callbacks Callback functions
     */
    void handleRegionColorChangeRequested(const QString& regionName,
                                         DocumentState* documentState,
                                         const GetRegionCallback& getRegion,
                                         const ShowChangeColorDialogCallback& showChangeColorDialog,
                                         const SaveStateCallback& saveState,
                                         const ChangeRegionColorCallback& changeRegionColor,
                                         const UpdateCanvasCallback& updateCanvas,
                                         const UpdateRegionListBoxCallback& updateRegionListBox,
                                         const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                         const ShowStatusMessageCallback& showStatusMessage);
    
    /**
     * @brief Delete selected regions
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void deleteSelected(QWidget* parentWidget,
                      DocumentState* documentState,
                      Canvas* canvas,
                      const GetSelectedRegionsCallback& getSelectedRegions,
                      const ShowInformationCallback& showInformation,
                      const ShowQuestionCallback& showQuestion,
                      const SaveStateCallback& saveState,
                      const RemoveRegionCallback& removeRegion,
                      const SynchronizeCoordinatesCallback& synchronizeCoordinates,
                      const ClearSelectionCallback& clearSelection,
                      const UpdateCanvasCallback& updateCanvas,
                      const UpdateRegionListBoxCallback& updateRegionListBox,
                      const UpdateGroupListBoxCallback& updateGroupListBox,
                      const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                      const ShowStatusMessageCallback& showStatusMessage);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_REGION_OPERATIONS_H

