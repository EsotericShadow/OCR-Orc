#include "MainWindowRegionGroupHandlers.h"
#include "../../../MainWindow.h"
#include "../../../../models/DocumentState.h"
#include "../../../canvas/Canvas.h"
#include "../../../components/widgets/ControlPanelWidget.h"
#include "../../../components/widgets/SidePanelWidget.h"
#include "../../../../models/RegionData.h"
#include "../../../../models/GroupData.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QStatusBar>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QRegularExpression>
#include <functional>

namespace ocr_orc {

MainWindowRegionGroupHandlers::MainWindowRegionGroupHandlers() {
}

MainWindowRegionGroupHandlers::~MainWindowRegionGroupHandlers() {
}

void MainWindowRegionGroupHandlers::onCreateGroup(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState || !mainWindow->canvas) {
        return;
    }
    
    mainWindow->groupOperations->createGroup(
        mainWindow,
        mainWindow->documentState,
        mainWindow->canvas,
        [this, mainWindow]() { return getSelectedRegions(mainWindow); },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::warning(mainWindow, title, message);
        },
        [mainWindow](bool* ok) {
            return QInputDialog::getText(
                mainWindow,
                "Create Group",
                "Enter group name:",
                QLineEdit::Normal,
                "",
                ok
            );
        },
        [mainWindow](const QString& name) { return mainWindow->documentState->hasGroup(name); },
        [mainWindow](const QString& title, const QString& message) {
            return QMessageBox::question(
                mainWindow,
                title,
                message,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );
        },
        [mainWindow]() { mainWindow->documentState->saveState(); },
        [mainWindow](const QString& name) { mainWindow->documentState->createGroup(name); },
        [mainWindow](const QString& regionName, const QString& groupName) {
            mainWindow->documentState->addRegionToGroup(regionName, groupName);
        },
        [mainWindow]() { mainWindow->documentState->synchronizeCoordinates(); },
        [mainWindow]() { mainWindow->updateGroupListBox(); },
        [mainWindow]() { mainWindow->updateRegionListBox(); },
        [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->update(); },
        [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
        [mainWindow](const QString& message, int timeout) {
            mainWindow->statusBar()->showMessage(message, timeout);
        },
        [mainWindow]() { return mainWindow->isCreatingGroup; },
        [mainWindow](bool value) { mainWindow->isCreatingGroup = value; }
    );
}

QSet<QString> MainWindowRegionGroupHandlers::getSelectedRegions(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->canvas || !mainWindow->sidePanelWidget || !mainWindow->documentState) {
        return QSet<QString>();
    }
    
    QSet<QString> selectedRegions = mainWindow->canvas->getSelectedRegions();
    
    // Also check list box selections
    if (mainWindow->sidePanelWidget && mainWindow->sidePanelWidget->getRegionListBox()) {
        QList<QListWidgetItem*> selectedItems = mainWindow->sidePanelWidget->getRegionListBox()->selectedItems();
        for (QListWidgetItem* item : selectedItems) {
            QString regionName = item->text().split(" (").first();
            if (mainWindow->documentState->hasRegion(regionName)) {
                selectedRegions.insert(regionName);
            }
        }
    }
    
    return selectedRegions;
}

void MainWindowRegionGroupHandlers::onAddToGroup(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState || !mainWindow->canvas) {
        return;
    }
    
    mainWindow->groupOperations->addToGroup(
        mainWindow,
        mainWindow->documentState,
        mainWindow->canvas,
        [this, mainWindow]() { return getSelectedRegions(mainWindow); },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::warning(mainWindow, title, message);
        },
        [mainWindow](bool* ok) {
            return QInputDialog::getText(
                mainWindow,
                "Add to Group",
                "Enter group name:",
                QLineEdit::Normal,
                "",
                ok
            );
        },
        [mainWindow]() { mainWindow->documentState->saveState(); },
        [mainWindow](const QString& name) { return mainWindow->documentState->hasRegion(name); },
        [mainWindow](const QString& regionName, const QString& groupName) {
            mainWindow->documentState->addRegionToGroup(regionName, groupName);
        },
        [mainWindow]() { mainWindow->documentState->synchronizeCoordinates(); },
        [mainWindow]() { mainWindow->updateGroupListBox(); },
        [mainWindow]() { mainWindow->updateRegionListBox(); },
        [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->update(); },
        [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
        [mainWindow](const QString& message, int timeout) {
            mainWindow->statusBar()->showMessage(message, timeout);
        }
    );
}

void MainWindowRegionGroupHandlers::onRemoveFromGroup(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState || !mainWindow->canvas) {
        return;
    }
    
        mainWindow->groupOperations->removeFromGroup(
        mainWindow,
        mainWindow->documentState,
        mainWindow->canvas,
        [this, mainWindow]() { return getSelectedRegions(mainWindow); },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::warning(mainWindow, title, message);
        },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::information(mainWindow, title, message);
        },
        [mainWindow](const QString& regionName) {
            if (mainWindow->documentState && mainWindow->documentState->hasRegion(regionName)) {
                RegionData region = mainWindow->documentState->getRegion(regionName);
                return region.group;
            }
            return QString();
        },
        [mainWindow](const QString& name) { return mainWindow->documentState ? mainWindow->documentState->hasRegion(name) : false; },
        [mainWindow]() { return mainWindow->documentState ? mainWindow->documentState->getGroupNames() : QList<QString>(); },
        [mainWindow](const QString& name) { return mainWindow->documentState->getGroup(name); },
        [mainWindow]() { mainWindow->documentState->saveState(); },
        [mainWindow](const QString& regionName) { mainWindow->documentState->removeRegionFromGroup(regionName); },
        [mainWindow]() { mainWindow->documentState->synchronizeCoordinates(); },
        [mainWindow]() { mainWindow->updateGroupListBox(); },
        [mainWindow]() { mainWindow->updateRegionListBox(); },
        [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->update(); },
        [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
        [mainWindow](const QString& message, int timeout) {
            mainWindow->statusBar()->showMessage(message, timeout);
        }
    );
}

void MainWindowRegionGroupHandlers::onDeleteGroup(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState || !mainWindow->canvas || !mainWindow->sidePanelWidget) {
        return;
    }
    
    mainWindow->groupOperations->deleteGroup(
        mainWindow,
        mainWindow->documentState,
        mainWindow->canvas,
        mainWindow->sidePanelWidget,
        [mainWindow]() {
            if (!mainWindow->sidePanelWidget || !mainWindow->sidePanelWidget->getGroupListBox()) {
                return QString();
            }
            QList<QListWidgetItem*> selectedItems = mainWindow->sidePanelWidget->getGroupListBox()->selectedItems();
            if (selectedItems.isEmpty()) {
                return QString();
            }
            QString displayText = selectedItems[0]->text();
            return displayText.split(" (").first();
        },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::warning(mainWindow, title, message);
        },
        [mainWindow](const QString& name) { return mainWindow->documentState ? mainWindow->documentState->hasGroup(name) : false; },
        [mainWindow](const QString& name) { return mainWindow->documentState->getGroup(name); },
        [mainWindow](const QString& title, const QString& message) {
            return QMessageBox::question(
                mainWindow,
                title,
                message,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            );
        },
        [mainWindow]() { mainWindow->documentState->saveState(); },
        [mainWindow](const QString& name) { mainWindow->documentState->deleteGroup(name); },
        [mainWindow]() { mainWindow->documentState->synchronizeCoordinates(); },
        [mainWindow]() { mainWindow->updateGroupListBox(); },
        [mainWindow]() { mainWindow->updateRegionListBox(); },
        [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->update(); },
        [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
        [mainWindow](const QString& text) {
            if (mainWindow->sidePanelWidget) {
                mainWindow->sidePanelWidget->setInfoText(text);
            }
        },
        [mainWindow](const QString& message, int timeout) {
            mainWindow->statusBar()->showMessage(message, timeout);
        }
    );
}

QString MainWindowRegionGroupHandlers::showEditNameDialog(MainWindow* mainWindow, const QString& currentName) {
    if (!mainWindow) {
        return QString();
    }
    
    return mainWindow->dialogsController->showEditNameDialog(mainWindow, currentName);
}

void MainWindowRegionGroupHandlers::onEditSelected(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->sidePanelWidget) {
        return;
    }
    
    // Toggle region editor visibility
    QSet<QString> selectedRegions = mainWindow->canvas ? mainWindow->canvas->getSelectedRegions() : QSet<QString>();
    
    if (mainWindow->sidePanelWidget->isRegionEditorVisible()) {
        // Hide editor
        mainWindow->sidePanelWidget->toggleRegionEditor();
    } else {
        // Show editor - if there's a selection, it will be populated by selectionChanged signal
        // Otherwise, we need to populate it manually
        if (!selectedRegions.isEmpty() && mainWindow->documentState && mainWindow->canvas) {
            QString regionName = mainWindow->canvas->getPrimarySelectedRegion();
            if (!regionName.isEmpty() && mainWindow->documentState->hasRegion(regionName)) {
                RegionData region = mainWindow->documentState->getRegion(regionName);
                QList<QString> availableGroups = mainWindow->documentState->getGroupNames();
                mainWindow->sidePanelWidget->showRegionEditor(regionName, region.color, region.group, availableGroups,
                                                               region.regionType, region.percentageFill,
                                                              region.normalizedCoords.x1, region.normalizedCoords.y1,
                                                              region.normalizedCoords.x2, region.normalizedCoords.y2);
            }
        } else {
            // Just toggle (will show empty if no selection)
            mainWindow->sidePanelWidget->toggleRegionEditor();
        }
    }
}

QString MainWindowRegionGroupHandlers::showChangeColorDialog(MainWindow* mainWindow, const QString& regionName) {
    if (!mainWindow || !mainWindow->documentState) {
        return QString();
    }
    
    return mainWindow->dialogsController->showChangeColorDialog(
        mainWindow,
        mainWindow->documentState,
        regionName,
        [mainWindow](const QString& name) { return mainWindow->documentState->getRegion(name); }
    );
}

void MainWindowRegionGroupHandlers::onRegionCreationRequested(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->canvas || !mainWindow->documentState || !mainWindow->controlPanelWidget) {
        return;
    }
    
    mainWindow->regionOperations->handleRegionCreationRequested(
        mainWindow->canvas,
        mainWindow->documentState,
        mainWindow->controlPanelWidget,
        [mainWindow]() { return mainWindow->controlPanelWidget ? mainWindow->controlPanelWidget->getRegionName() : QString(); },
        [mainWindow]() { return mainWindow->controlPanelWidget ? mainWindow->controlPanelWidget->getSelectedColor() : "blue"; },
        [mainWindow]() { return mainWindow->controlPanelWidget ? mainWindow->controlPanelWidget->getGroupName() : QString(); },
        [mainWindow]() { if (mainWindow->controlPanelWidget) mainWindow->controlPanelWidget->clearGroupName(); },
        [mainWindow](const QString& name, const QString& color, const QString& group) {
            return mainWindow->canvas ? mainWindow->canvas->finishRegionCreation(name, color, group) : false;
        },
        [mainWindow](const QString& name) { return mainWindow->documentState ? mainWindow->documentState->hasRegion(name) : false; },
        [mainWindow]() { return mainWindow->generateRegionName(); },
        [mainWindow]() { mainWindow->autoIncrementRegionName(); }
    );
}

void MainWindowRegionGroupHandlers::onRegionCreated(MainWindow* mainWindow, const QString& regionName) {
    if (!mainWindow) {
        return;
    }
    
    mainWindow->updateRegionListBox();
    mainWindow->statusBar()->showMessage(QString("Created region '%1'").arg(regionName), 2000);
    mainWindow->updateUndoRedoButtons();
}

void MainWindowRegionGroupHandlers::onRegionsDuplicated(MainWindow* mainWindow, const QList<QString>& duplicatedNames) {
    if (!mainWindow) {
        return;
    }
    
    mainWindow->updateRegionListBox();
    if (!duplicatedNames.isEmpty()) {
        mainWindow->statusBar()->showMessage(QString("Duplicated %1 region(s)").arg(duplicatedNames.size()), 2000);
    }
    mainWindow->updateUndoRedoButtons();
}

QString MainWindowRegionGroupHandlers::generateRegionName(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState) {
        return QString("Region_1");
    }
    
    return mainWindow->regionOperations->generateRegionName(mainWindow->documentState);
}

void MainWindowRegionGroupHandlers::autoIncrementRegionName(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->controlPanelWidget || !mainWindow->documentState) {
        return;
    }
    
    mainWindow->regionOperations->autoIncrementRegionName(
        mainWindow->controlPanelWidget,
        mainWindow->documentState,
        [mainWindow]() { return mainWindow->controlPanelWidget ? mainWindow->controlPanelWidget->getRegionName() : QString(); },
        [mainWindow](const QString& name) { 
            if (mainWindow->controlPanelWidget) {
                mainWindow->controlPanelWidget->getRegionNameEdit()->setText(name);
            }
        },
        [mainWindow](const QString& name) { return mainWindow->documentState ? mainWindow->documentState->hasRegion(name) : false; }
    );
}

void MainWindowRegionGroupHandlers::onDeleteSelected(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState || !mainWindow->canvas) {
        return;
    }
    
    mainWindow->regionOperations->deleteSelected(
        mainWindow,
        mainWindow->documentState,
        mainWindow->canvas,
        [mainWindow]() { return mainWindow->canvas ? mainWindow->canvas->getSelectedRegions() : QSet<QString>(); },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::information(mainWindow, title, message);
        },
        [mainWindow](const QString& title, const QString& message) {
            return QMessageBox::question(
                mainWindow,
                title,
                message,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            );
        },
        [mainWindow]() { mainWindow->documentState->saveState(); },
        [mainWindow](const QString& name) { mainWindow->documentState->removeRegion(name); },
        [mainWindow]() { mainWindow->documentState->synchronizeCoordinates(); },
        [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->clearSelection(); },
        [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->update(); },
        [mainWindow]() { mainWindow->updateRegionListBox(); },
        [mainWindow]() { mainWindow->updateGroupListBox(); },
        [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
        [mainWindow](const QString& message, int timeout) {
            mainWindow->statusBar()->showMessage(message, timeout);
        }
    );
}

void MainWindowRegionGroupHandlers::onRegionNameChanged(MainWindow* mainWindow, const QString& oldName, const QString& newName) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    // Use the existing rename functionality
    if (mainWindow->documentState->renameRegion(oldName, newName)) {
        mainWindow->documentState->saveState();
        mainWindow->updateRegionListBox();
        mainWindow->updateGroupListBox();
        if (mainWindow->canvas) {
            mainWindow->canvas->update();
        }
        mainWindow->updateUndoRedoButtons();
        mainWindow->statusBar()->showMessage(QString("Renamed region '%1' to '%2'").arg(oldName).arg(newName), 2000);
    } else {
        QMessageBox::warning(mainWindow, "Rename Failed", 
                           QString("Could not rename region '%1' to '%2'.\nName may be invalid or already in use.").arg(oldName).arg(newName));
    }
}

void MainWindowRegionGroupHandlers::onRegionColorChanged(MainWindow* mainWindow, const QString& regionName, const QString& newColor) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    if (!mainWindow->documentState->hasRegion(regionName)) {
        return;
    }
    
    // Save state BEFORE modification for undo/redo
    mainWindow->documentState->saveState();
    
    RegionData region = mainWindow->documentState->getRegion(regionName);
    region.color = newColor;
    mainWindow->documentState->addRegion(regionName, region);
    
    if (mainWindow->canvas) {
        mainWindow->canvas->update();
    }
    mainWindow->updateUndoRedoButtons();
    mainWindow->statusBar()->showMessage(QString("Changed color of '%1' to %2").arg(regionName).arg(newColor), 1500);
}

void MainWindowRegionGroupHandlers::onRegionGroupChanged(MainWindow* mainWindow, const QString& regionName, const QString& newGroup) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    if (!mainWindow->documentState->hasRegion(regionName)) {
        return;
    }
    
    // Save state BEFORE modification for undo/redo
    mainWindow->documentState->saveState();
    
    // Remove from current group if any
    QString currentGroup = mainWindow->documentState->getRegion(regionName).group;
    if (!currentGroup.isEmpty()) {
        mainWindow->documentState->removeRegionFromGroup(regionName);
    }
    
    // Add to new group if specified
    if (!newGroup.isEmpty()) {
        if (!mainWindow->documentState->hasGroup(newGroup)) {
            // Create group if it doesn't exist
            mainWindow->documentState->createGroup(newGroup);
        }
        mainWindow->documentState->addRegionToGroup(regionName, newGroup);
    }
    mainWindow->updateRegionListBox();
    mainWindow->updateGroupListBox();
    if (mainWindow->canvas) {
        mainWindow->canvas->update();
    }
    mainWindow->updateUndoRedoButtons();
    
    if (newGroup.isEmpty()) {
        mainWindow->statusBar()->showMessage(QString("Removed '%1' from group").arg(regionName), 1500);
    } else {
        mainWindow->statusBar()->showMessage(QString("Added '%1' to group '%2'").arg(regionName).arg(newGroup), 1500);
    }
}

} // namespace ocr_orc

