#include "MainWindowGroupOperations.h"
#include "../../../../models/DocumentState.h"
#include "../../../../models/GroupData.h"
#include "../../../canvas/Canvas.h"
#include "../../../components/widgets/SidePanelWidget.h"
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QListWidget>

namespace ocr_orc {

MainWindowGroupOperations::MainWindowGroupOperations() {
}

MainWindowGroupOperations::~MainWindowGroupOperations() {
}

QSet<QString> MainWindowGroupOperations::getSelectedRegions(Canvas* canvas,
                                                           SidePanelWidget* sidePanelWidget,
                                                           DocumentState* documentState,
                                                           const GetCanvasSelectedRegionsCallback& getCanvasSelectedRegions,
                                                           const GetListBoxSelectedItemsCallback& getListBoxSelectedItems,
                                                           const HasRegionCallback& hasRegion) const {
    QSet<QString> selectedRegions;
    
    // First, try canvas selection
    if (canvas) {
        selectedRegions = getCanvasSelectedRegions();
    }
    
    // If nothing selected on canvas, try region listbox
    if (selectedRegions.isEmpty() && sidePanelWidget && sidePanelWidget->getRegionListBox() && documentState) {
        QList<QListWidgetItem*> selectedItems = getListBoxSelectedItems();
        for (QListWidgetItem* item : selectedItems) {
            QString displayText = item->text();
            QString regionName = displayText.split(" [").first();
            if (hasRegion(regionName)) {
                selectedRegions.insert(regionName);
            }
        }
    }
    
    return selectedRegions;
}

void MainWindowGroupOperations::createGroup(QWidget* parentWidget,
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
                                            const SetIsCreatingGroupCallback& setIsCreatingGroup) {
    // Prevent double execution (can happen if button click triggers both signal and shortcut)
    if (isCreatingGroup()) {
        return;
    }
    setIsCreatingGroup(true);
    
    if (!documentState || !canvas) {
        setIsCreatingGroup(false);
        return;
    }
    
    // Get selected regions from canvas OR region listbox
    QSet<QString> selectedRegions = getSelectedRegions();
    
    if (selectedRegions.size() < 2) {
        if (showWarning) {
            showWarning("Create Group",
                "Please select at least 2 regions to create a group.\n\n"
                "You can select regions by:\n"
                "- Clicking on them in the canvas (Cmd/Ctrl+Click for multiple)\n"
                "- Selecting them in the Regions list (Cmd/Ctrl+Click for multiple)");
        }
        setIsCreatingGroup(false);
        return;
    }
    
    // Show input dialog for group name
    bool ok = false;
    QString groupName = getTextInput(&ok);
    
    if (!ok || groupName.trimmed().isEmpty()) {
        setIsCreatingGroup(false);
        return; // User cancelled or entered empty name
    }
    
    groupName = groupName.trimmed();
    
    // Check if group already exists
    if (hasGroup(groupName)) {
        int reply = showQuestion("Group Exists",
            QString("Group '%1' already exists. Add selected regions to existing group?")
                .arg(groupName));
        
        if (reply == QMessageBox::Yes) {
            // Add to existing group
            if (saveState) {
                saveState();
            }
            for (const QString& regionName : selectedRegions) {
                if (addRegionToGroup) {
                    addRegionToGroup(regionName, groupName);
                }
            }
            if (synchronizeCoordinates) {
                synchronizeCoordinates();
            }
            if (updateGroupListBox) {
                updateGroupListBox();
            }
            if (updateRegionListBox) {
                updateRegionListBox();
            }
            if (updateCanvas) {
                updateCanvas();
            }
            if (updateUndoRedoButtons) {
                updateUndoRedoButtons();
            }
            if (showStatusMessage) {
                showStatusMessage(QString("Added %1 regions to group '%2'")
                    .arg(selectedRegions.size()).arg(groupName), 2000);
            }
        }
        setIsCreatingGroup(false);
        return;
    }
    
    // Create new group
    if (saveState) {
        saveState();
    }
    if (createGroup) {
        createGroup(groupName);
    }
    for (const QString& regionName : selectedRegions) {
        if (addRegionToGroup) {
            addRegionToGroup(regionName, groupName);
        }
    }
    if (synchronizeCoordinates) {
        synchronizeCoordinates();
    }
    if (updateGroupListBox) {
        updateGroupListBox();
    }
    if (updateRegionListBox) {
        updateRegionListBox();
    }
    if (updateCanvas) {
        updateCanvas();
    }
    if (updateUndoRedoButtons) {
        updateUndoRedoButtons();
    }
    if (showStatusMessage) {
        showStatusMessage(QString("Created group '%1' with %2 regions")
            .arg(groupName).arg(selectedRegions.size()), 2000);
    }
    
    setIsCreatingGroup(false);
}

void MainWindowGroupOperations::addToGroup(QWidget* parentWidget,
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
                                            const ShowStatusMessageCallback& showStatusMessage) {
    if (!documentState || !canvas) {
        return;
    }
    
    // Get selected regions from canvas OR region listbox
    QSet<QString> selectedRegions = getSelectedRegions();
    
    if (selectedRegions.isEmpty()) {
        if (showWarning) {
            showWarning("Add to Group",
                "Please select at least one region to add to a group.\n\n"
                "You can select regions by:\n"
                "- Clicking on them in the canvas\n"
                "- Selecting them in the Regions list");
        }
        return;
    }
    
    // Show input dialog for group name
    bool ok = false;
    QString groupName = getTextInput(&ok);
    
    if (!ok || groupName.trimmed().isEmpty()) {
        return;
    }
    
    groupName = groupName.trimmed();
    
    // Save state and add regions to group
    if (saveState) {
        saveState();
    }
    int addedCount = 0;
    for (const QString& regionName : selectedRegions) {
        if (hasRegion(regionName)) {
            if (addRegionToGroup) {
                addRegionToGroup(regionName, groupName);
            }
            addedCount++;
        }
    }
    if (synchronizeCoordinates) {
        synchronizeCoordinates();
    }
    if (updateGroupListBox) {
        updateGroupListBox();
    }
    if (updateRegionListBox) {
        updateRegionListBox();
    }
    if (updateCanvas) {
        updateCanvas();
    }
    if (updateUndoRedoButtons) {
        updateUndoRedoButtons();
    }
    if (showStatusMessage) {
        showStatusMessage(QString("Added %1 regions to group '%2'")
            .arg(addedCount).arg(groupName), 2000);
    }
}

void MainWindowGroupOperations::removeFromGroup(QWidget* parentWidget,
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
                                               const ShowStatusMessageCallback& showStatusMessage) {
    if (!documentState || !canvas) {
        return;
    }
    
    // Get selected regions from canvas OR region listbox
    QSet<QString> selectedRegions = getSelectedRegions();
    
    if (selectedRegions.isEmpty()) {
        if (showWarning) {
            showWarning("Remove from Group",
                "Please select at least one region to remove from its group.\n\n"
                "You can select regions by:\n"
                "- Clicking on them in the canvas\n"
                "- Selecting them in the Regions list");
        }
        return;
    }
    
    // Check if any selected regions are actually in groups
    // Use direct access to regions map to get current state
    int regionsInGroups = 0;
    QStringList debugInfo;
    for (const QString& regionName : selectedRegions) {
        if (hasRegion(regionName)) {
            QString group = getRegionGroup(regionName);
            debugInfo << QString("Region '%1': group='%2'").arg(regionName).arg(group);
            if (!group.isEmpty()) {
                regionsInGroups++;
            }
        }
    }
    
    // Debug: Check groups map as well
    QList<QString> allGroupNames = getGroupNames();
    QStringList groupDebugInfo;
    for (const QString& groupName : allGroupNames) {
        GroupData group = getGroup(groupName);
        groupDebugInfo << QString("Group '%1': %2 regions").arg(groupName).arg(group.regionNames.size());
    }
    
    if (regionsInGroups == 0) {
        QString debugMsg = "Debug info:\n\nSelected regions:\n" + debugInfo.join("\n");
        debugMsg += "\n\nAll groups:\n" + (groupDebugInfo.isEmpty() ? "No groups" : groupDebugInfo.join("\n"));
        if (showInformation) {
            showInformation("Remove from Group",
                QString("None of the selected regions are in a group.\n\n%1").arg(debugMsg));
        }
        return;
    }
    
    // Save state and remove regions from groups
    if (saveState) {
        saveState();
    }
    int removedCount = 0;
    QSet<QString> groupsAffected;
    
    for (const QString& regionName : selectedRegions) {
        if (hasRegion(regionName)) {
            QString groupName = getRegionGroup(regionName);
            if (!groupName.isEmpty()) {
                if (removeRegionFromGroup) {
                    removeRegionFromGroup(regionName);
                }
                groupsAffected.insert(groupName);
                removedCount++;
            }
        }
    }
    if (synchronizeCoordinates) {
        synchronizeCoordinates();
    }
    if (updateGroupListBox) {
        updateGroupListBox();
    }
    if (updateRegionListBox) {
        updateRegionListBox();
    }
    if (updateCanvas) {
        updateCanvas();
    }
    if (updateUndoRedoButtons) {
        updateUndoRedoButtons();
    }
    if (showStatusMessage) {
        showStatusMessage(QString("Removed %1 region(s) from their groups")
            .arg(removedCount), 2000);
    }
}

void MainWindowGroupOperations::deleteGroup(QWidget* parentWidget,
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
                                            const ShowStatusMessageCallback& showStatusMessage) {
    if (!documentState || !sidePanelWidget || !sidePanelWidget->getGroupListBox()) {
        return;
    }
    
    // Get selected group from listbox
    QString groupName = getSelectedGroup();
    
    if (groupName.isEmpty()) {
        if (showWarning) {
            showWarning("Delete Group", "Please select a group to delete.");
        }
        return;
    }
    
    if (!hasGroup(groupName)) {
        if (showWarning) {
            showWarning("Delete Group", QString("Group '%1' not found.").arg(groupName));
        }
        return;
    }
    
    // Show confirmation dialog
    GroupData group = getGroup(groupName);
    int reply = showQuestion("Delete Group",
        QString("Are you sure you want to delete group '%1'?\n\nThis will remove %2 regions from the group, but the regions themselves will not be deleted.")
            .arg(groupName).arg(group.regionNames.size()));
    
    if (reply == QMessageBox::Yes) {
        if (saveState) {
            saveState();
        }
        if (deleteGroup) {
            deleteGroup(groupName);
        }
        if (synchronizeCoordinates) {
            synchronizeCoordinates();
        }
        if (updateGroupListBox) {
            updateGroupListBox();
        }
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateCanvas) {
            updateCanvas();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        if (setInfoText) {
            setInfoText("Select a region or group to view details");
        }
        if (showStatusMessage) {
            showStatusMessage(QString("Deleted group '%1'").arg(groupName), 2000);
        }
    }
}

} // namespace ocr_orc

