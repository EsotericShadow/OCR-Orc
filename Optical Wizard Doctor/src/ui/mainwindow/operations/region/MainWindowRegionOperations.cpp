#include "MainWindowRegionOperations.h"
#include "../../../../models/DocumentState.h"
#include "../../../../models/RegionData.h"
#include "../../../canvas/Canvas.h"
#include "../../../components/widgets/ControlPanelWidget.h"
#include "../../../ThemeManager.h"
#include <QtCore/QObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPalette>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>

namespace ocr_orc {

namespace {
void applyDialogPalette(QDialog& dialog, const ThemeColors& colors) {
    QPalette palette = dialog.palette();
    palette.setColor(QPalette::Window, colors.surface);
    palette.setColor(QPalette::WindowText, colors.text);
    palette.setColor(QPalette::Base, colors.background);
    palette.setColor(QPalette::Text, colors.text);
    palette.setColor(QPalette::Button, colors.surface);
    palette.setColor(QPalette::ButtonText, colors.text);
    dialog.setPalette(palette);
}
} // namespace

MainWindowRegionOperations::MainWindowRegionOperations() {
}

MainWindowRegionOperations::~MainWindowRegionOperations() {
}

QString MainWindowRegionOperations::showEditNameDialog(QWidget* parentWidget, const QString& currentName) const {
    ThemeColors themeColors = ThemeManager::instance().getColors();

    QDialog dialog(parentWidget);
    dialog.setWindowTitle("Edit Region Name");
    dialog.setMinimumSize(350, 120);
    dialog.setStyleSheet(QString("QDialog { background-color: %1; color: %2; }")
                             .arg(themeColors.surface.name(), themeColors.text.name()));
    applyDialogPalette(dialog, themeColors);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    
    QLabel* label = new QLabel("New Name:", &dialog);
    label->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    layout->addWidget(label);
    
    QLineEdit* lineEdit = new QLineEdit(&dialog);
    lineEdit->setText(currentName);
    lineEdit->selectAll();  // Select all for easy replacement
    lineEdit->setFocus();
    lineEdit->setStyleSheet(QString("color: %1; background-color: %2;")
                                .arg(themeColors.text.name(), themeColors.background.name()));
    layout->addWidget(lineEdit);
    
    QPushButton* saveButton = new QPushButton("Save", &dialog);
    saveButton->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);
    cancelButton->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    // Connect Enter key to save
    QObject::connect(lineEdit, &QLineEdit::returnPressed, saveButton, &QPushButton::click);
    QObject::connect(saveButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        return lineEdit->text().trimmed();
    }
    
    return QString(); // User cancelled
}

QString MainWindowRegionOperations::showChangeColorDialog(QWidget* parentWidget,
                                                         DocumentState* documentState,
                                                         const QString& regionName,
                                                         const GetRegionCallback& getRegion) const {
    if (!documentState || !documentState->hasRegion(regionName)) {
        return QString();
    }
    
    ThemeColors themeColors = ThemeManager::instance().getColors();

    RegionData region = getRegion(regionName);
    QString currentColor = region.color;
    
    QDialog dialog(parentWidget);
    dialog.setWindowTitle("Change Region Color");
    dialog.setMinimumSize(300, 150);
    dialog.setStyleSheet(QString("QDialog { background-color: %1; color: %2; }")
                             .arg(themeColors.surface.name(), themeColors.text.name()));
    applyDialogPalette(dialog, themeColors);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    
    QLabel* label = new QLabel("Select Color:", &dialog);
    label->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    layout->addWidget(label);
    
    QComboBox* colorCombo = new QComboBox(&dialog);
    colorCombo->setStyleSheet(QString("color: %1; background-color: %2;")
                                  .arg(themeColors.text.name(), themeColors.background.name()));
    QStringList colorsList = {"blue", "red", "green", "yellow", "purple", "orange", "cyan"};
    colorCombo->addItems(colorsList);
    
    // Pre-select current color
    int currentIndex = colorsList.indexOf(currentColor);
    if (currentIndex >= 0) {
        colorCombo->setCurrentIndex(currentIndex);
    } else {
        colorCombo->setCurrentIndex(0); // Default to blue
    }
    
    layout->addWidget(colorCombo);
    
    QPushButton* saveButton = new QPushButton("Save", &dialog);
    saveButton->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);
    cancelButton->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    QObject::connect(saveButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        return colorCombo->currentText();
    }
    
    return QString(); // User cancelled
}

QString MainWindowRegionOperations::generateRegionName(DocumentState* documentState) const {
    // Generate "Cell 1", "Cell 2", etc.
    int counter = 1;
    QString baseName = "Cell";
    QString name;
    
    do {
        name = baseName + " " + QString::number(counter);
        counter++;
    } while (documentState && documentState->hasRegion(name));
    
    return name;
}

QString MainWindowRegionOperations::showCreateRegionNameDialog(QWidget* parentWidget,
                                                               DocumentState* documentState,
                                                               const GenerateRegionNameCallback& generateRegionNameCallback) const {
    // Use callback if provided, otherwise use internal method
    QString defaultName;
    if (generateRegionNameCallback) {
        defaultName = generateRegionNameCallback();
    } else {
        defaultName = generateRegionName(documentState);
    }
    
    ThemeColors themeColors = ThemeManager::instance().getColors();

    QDialog dialog(parentWidget);
    dialog.setWindowTitle("Name Region");
    dialog.setMinimumSize(350, 120);
    dialog.setStyleSheet(QString("QDialog { background-color: %1; color: %2; }")
                             .arg(themeColors.surface.name(), themeColors.text.name()));
    applyDialogPalette(dialog, themeColors);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    
    QLabel* label = new QLabel("Region Name:", &dialog);
    label->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    layout->addWidget(label);
    
    QLineEdit* lineEdit = new QLineEdit(&dialog);
    lineEdit->setText(defaultName);
    lineEdit->selectAll();  // Select all for easy replacement
    lineEdit->setFocus();
    lineEdit->setStyleSheet(QString("color: %1; background-color: %2;")
                                .arg(themeColors.text.name(), themeColors.background.name()));
    layout->addWidget(lineEdit);
    
    QPushButton* createButton = new QPushButton("Create", &dialog);
    createButton->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);
    cancelButton->setStyleSheet(QString("color: %1;").arg(themeColors.text.name()));
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    // Connect Enter key to create
    QObject::connect(lineEdit, &QLineEdit::returnPressed, createButton, &QPushButton::click);
    QObject::connect(createButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = lineEdit->text().trimmed();
        if (!name.isEmpty()) {
            return name;
        }
    }
    
    return QString(); // User cancelled or empty name
}

void MainWindowRegionOperations::autoIncrementRegionName(ControlPanelWidget* controlPanelWidget,
                                                         DocumentState* documentState,
                                                         const GetRegionNameCallback& getRegionName,
                                                         const SetRegionNameEditCallback& setRegionNameEdit,
                                                         const HasRegionCallback& hasRegion) {
    if (!controlPanelWidget) {
        return;
    }
    
    QString currentName = getRegionName();
    
    // Check if name ends with a number (with or without space)
    // Pattern: any text followed by optional space and digits at the end
    QRegularExpression regex(R"((.+?)(\s*)(\d+)$)");
    QRegularExpressionMatch match = regex.match(currentName);
    
    if (match.hasMatch()) {
        // Extract base name, separator (space if any), and number
        QString baseName = match.captured(1);
        QString separator = match.captured(2); // Preserve space if it exists
        int number = match.captured(3).toInt();
        
        // Increment number
        number++;
        
        // Generate new name (preserve separator style)
        QString newName = baseName + separator + QString::number(number);
        
        // Check if name already exists, keep incrementing if needed
        // Use documentState directly if available, otherwise fall back to callback
        while ((documentState && documentState->hasRegion(newName)) || 
               (!documentState && hasRegion && hasRegion(newName))) {
            number++;
            newName = baseName + separator + QString::number(number);
        }
        
        // Update input field
        setRegionNameEdit(newName);
    }
    // Removed special case for "Cell" - now works for any name ending with number
}

void MainWindowRegionOperations::handleRegionCreationRequested(Canvas* canvas,
                                                                DocumentState* documentState,
                                                                ControlPanelWidget* /* controlPanelWidget */,
                                                                const GetRegionNameCallback& /* getRegionName */,
                                                                const GetSelectedColorCallback& /* getSelectedColor */,
                                                                const GetGroupNameCallback& /* getGroupName */,
                                                                const ClearGroupNameCallback& /* clearGroupName */,
                                                                const FinishRegionCreationCallback& finishRegionCreation,
                                                                const HasRegionCallback& hasRegion,
                                                                const GenerateRegionNameCallback& generateRegionName,
                                                                const AutoIncrementRegionNameCallback& /* autoIncrementRegionName */) {
    if (!canvas || !documentState) {
        return;
    }
    
    // Show dialog to get region name (default: "Cell N")
    QWidget* parentWidget = qobject_cast<QWidget*>(canvas->parent());
    QString regionName = showCreateRegionNameDialog(parentWidget, documentState, generateRegionName);
    
    // If user cancelled, don't create region
    if (regionName.isEmpty()) {
        return;
    }
    
    // Check for duplicate name - auto-increment instead of showing warning
    QString finalRegionName = regionName;
    if (hasRegion(finalRegionName)) {
        // Auto-increment: add _1, _2, etc.
        int counter = 1;
        do {
            finalRegionName = regionName + "_" + QString::number(counter);
            counter++;
        } while (hasRegion(finalRegionName));
    }
    
    // Default color is blue, no group assigned (user can add to group later in sidebar)
    QString color = "blue";
    QString group = QString(); // Empty group - user can add to group later
    
    // Complete region creation
    finishRegionCreation(finalRegionName, color, group);
}

void MainWindowRegionOperations::handleRegionCreated(const QString& regionName,
                                                     const UpdateRegionListBoxCallback& updateRegionListBox,
                                                     const ShowStatusMessageCallback& showStatusMessage,
                                                     const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons) {
    // Update region list in side panel
    if (updateRegionListBox) {
        updateRegionListBox();
    }
    
    // Update status bar
    if (showStatusMessage) {
        showStatusMessage("Region created: " + regionName, 2000);
    }
    
    // Update undo/redo buttons (state was saved before region creation)
    if (updateUndoRedoButtons) {
        updateUndoRedoButtons();
    }
}

void MainWindowRegionOperations::handleRegionsDuplicated(const QList<QString>& duplicatedNames,
                                                          const UpdateRegionListBoxCallback& updateRegionListBox,
                                                          const ShowStatusMessageCallback& showStatusMessage,
                                                          const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons) {
    // Update region list in side panel
    if (updateRegionListBox) {
        updateRegionListBox();
    }
    
    // Update status bar
    if (showStatusMessage) {
        if (duplicatedNames.size() == 1) {
            showStatusMessage("Region duplicated: " + duplicatedNames.first(), 2000);
        } else {
            showStatusMessage(QString("Duplicated %1 regions").arg(duplicatedNames.size()), 2000);
        }
    }
    
    // Update undo/redo buttons (state was saved before duplication)
    if (updateUndoRedoButtons) {
        updateUndoRedoButtons();
    }
}

void MainWindowRegionOperations::editSelected(QWidget* parentWidget,
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
                                               const ShowStatusMessageCallback& showStatusMessage) {
    if (!documentState || !canvas) {
        return;
    }
    
    // Use parentWidget if provided, otherwise fall back to canvas parent
    // This ensures dialogs are properly parented
    QWidget* effectiveParent = parentWidget;
    if (!effectiveParent && canvas) {
        effectiveParent = qobject_cast<QWidget*>(canvas->parent());
    }
    
    // Get selected regions from canvas
    QSet<QString> selectedRegions = getSelectedRegions();
    
    if (selectedRegions.isEmpty()) {
        if (showWarning) {
            showWarning("Edit Region", "Please select a region to edit.");
        }
        return;
    }
    
    // For now, only allow editing single region
    if (selectedRegions.size() > 1) {
        if (showWarning) {
            showWarning("Edit Region", "Please select only one region to edit its name.");
        }
        return;
    }
    
    QString regionName = *selectedRegions.begin();
    
    if (!documentState->hasRegion(regionName)) {
        if (showWarning) {
            showWarning("Edit Region", QString("Region '%1' not found.").arg(regionName));
        }
        return;
    }
    
    // Show edit dialog
    QString newName = showEditNameDialog(regionName);
    
    if (newName.isEmpty()) {
        return; // User cancelled
    }
    
    // Validate new name
    if (!documentState->isValidRegionName(newName, regionName)) {
        if (newName.trimmed().isEmpty()) {
            if (showWarning) {
                showWarning("Invalid Name", "Region name cannot be empty.");
            }
        } else if (documentState->hasRegion(newName)) {
            if (showWarning) {
                showWarning("Invalid Name",
                    QString("Region '%1' already exists. Please choose a different name.")
                        .arg(newName));
            }
        }
        return;
    }
    
    // Save state before rename
    if (saveState) {
        saveState();
    }
    
    // Perform rename
    if (renameRegion(regionName, newName)) {
        // Update selection to new name
        if (clearSelection) {
            clearSelection();
        }
        if (selectRegion) {
            selectRegion(newName);
        }
        
        // Update UI
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateGroupListBox) {
            updateGroupListBox();
        }
        if (updateCanvas) {
            updateCanvas();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        
        if (showStatusMessage) {
            showStatusMessage(
                QString("Renamed region '%1' to '%2'").arg(regionName).arg(newName),
                2000
            );
        }
    } else {
        if (showWarning) {
            showWarning("Rename Failed", QString("Failed to rename region '%1'.").arg(regionName));
        }
    }
}

void MainWindowRegionOperations::handleRegionEditRequested(const QString& regionName,
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
                                                           const ShowStatusMessageCallback& showStatusMessage) {
    // Handle region edit request from double-click
    if (!documentState || !documentState->hasRegion(regionName)) {
        return;
    }
    
    QString newName = showEditNameDialog(regionName);
    if (newName.isEmpty()) {
        return; // User cancelled
    }
    
    // Validate and rename
    if (!isValidRegionName(newName, regionName)) {
        if (newName.trimmed().isEmpty()) {
            if (showWarning) {
                showWarning("Invalid Name", "Region name cannot be empty.");
            }
        } else if (documentState->hasRegion(newName)) {
            if (showWarning) {
                showWarning("Invalid Name",
                    QString("Region '%1' already exists.").arg(newName));
            }
        }
        return;
    }
    
    if (saveState) {
        saveState();
    }
    if (renameRegion(regionName, newName)) {
        if (clearSelection) {
            clearSelection();
        }
        if (selectRegion) {
            selectRegion(newName);
        }
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateGroupListBox) {
            updateGroupListBox();
        }
        if (updateCanvas) {
            updateCanvas();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        if (showStatusMessage) {
            showStatusMessage(
                QString("Renamed region '%1' to '%2'").arg(regionName).arg(newName),
                2000
            );
        }
    }
}

void MainWindowRegionOperations::handleRegionColorChangeRequested(const QString& regionName,
                                                                   DocumentState* documentState,
                                                                   const GetRegionCallback& getRegion,
                                                                   const ShowChangeColorDialogCallback& showChangeColorDialog,
                                                                   const SaveStateCallback& saveState,
                                                                   const ChangeRegionColorCallback& changeRegionColor,
                                                                   const UpdateCanvasCallback& updateCanvas,
                                                                   const UpdateRegionListBoxCallback& updateRegionListBox,
                                                                   const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                                                   const ShowStatusMessageCallback& showStatusMessage) {
    if (!documentState || !documentState->hasRegion(regionName)) {
        return;
    }
    
    // Use getRegion callback to validate and get current region data
    if (getRegion) {
        RegionData currentRegion = getRegion(regionName);
        // Validate region exists and has valid data
        if (currentRegion.name.isEmpty()) {
            return; // Invalid region
        }
    }
    
    QString newColor = showChangeColorDialog(regionName);
    if (newColor.isEmpty()) {
        return; // User cancelled
    }
    
    // Save state before color change
    if (saveState) {
        saveState();
    }
    
    // Change color
    if (changeRegionColor(regionName, newColor)) {
        if (updateCanvas) {
            updateCanvas();
        }
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        if (showStatusMessage) {
            showStatusMessage(
                QString("Changed color of region '%1' to '%2'").arg(regionName).arg(newColor),
                2000
            );
        }
    }
}

void MainWindowRegionOperations::deleteSelected(QWidget* parentWidget,
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
                                                 const ShowStatusMessageCallback& showStatusMessage) {
    if (!documentState || !canvas) {
        return;
    }
    
    // Use parentWidget if provided, otherwise fall back to canvas parent
    // This ensures dialogs are properly parented
    QWidget* effectiveParent = parentWidget;
    if (!effectiveParent && canvas) {
        effectiveParent = qobject_cast<QWidget*>(canvas->parent());
    }
    
    QSet<QString> selectedRegions = getSelectedRegions();
    if (selectedRegions.isEmpty()) {
        if (showInformation) {
            showInformation("Delete Regions", "Please select one or more regions to delete.");
        }
        return;
    }
    
    // Show confirmation dialog
    int reply = showQuestion("Delete Regions",
        QString("Are you sure you want to delete %1 selected region(s)?")
            .arg(selectedRegions.size()));
    
    if (reply == QMessageBox::Yes) {
        if (saveState) {
            saveState();
        }
        
        // Delete all selected regions
        for (const QString& regionName : selectedRegions) {
            if (removeRegion) {
                removeRegion(regionName);
            }
        }
        
        if (synchronizeCoordinates) {
            synchronizeCoordinates();
        }
        if (clearSelection) {
            clearSelection();
        }
        if (updateCanvas) {
            updateCanvas();
        }
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateGroupListBox) {
            updateGroupListBox();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        
        if (showStatusMessage) {
            showStatusMessage(
                QString("Deleted %1 region(s)").arg(selectedRegions.size()),
                2000
            );
        }
    }
}

} // namespace ocr_orc

