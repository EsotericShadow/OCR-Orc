#include "SidePanelWidget.h"
#include "../icons/IconManager.h"
#include "../../../core/CoordinateSystem.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtCore/QSet>

namespace ocr_orc {

SidePanelWidget::SidePanelWidget(QWidget *parent)
    : QFrame(parent)
    , sidePanelTabs(nullptr)
    , regionsTab(nullptr)
    , groupsTab(nullptr)
    , regionListBox(nullptr)
    , groupListBox(nullptr)
    , infoText(nullptr)
    , helpButton(nullptr)
    , createGroupButton(nullptr)
    , addToGroupButton(nullptr)
    , removeFromGroupButton(nullptr)
    , deleteGroupButton(nullptr)
    , regionEditorFrame(nullptr)
    , regionEditorLabel(nullptr)
    , regionNameEdit(nullptr)
    , regionColorCombo(nullptr)
    , regionGroupCombo(nullptr)
{
    setFixedWidth(250);
    setStyleSheet(
        "QFrame {"
        "    background-color: #f5f5f5;"
        "    border: 1px solid #e5e5e5;"
        "    border-radius: 0px;"
        "}"
    );
    setFrameShape(QFrame::Box);
    setupUI();
}

void SidePanelWidget::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    sidePanelTabs = new QTabWidget(this);
    setupRegionsTab();
    setupGroupsTab();
    layout->addWidget(sidePanelTabs, 1);
    
    // Region editor section (initially hidden)
    setupRegionEditor();
    layout->addWidget(regionEditorFrame);
    regionEditorFrame->hide();
    
    infoText = new QTextEdit(this);
    infoText->setReadOnly(true);
    infoText->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    infoText->setPlainText("Select a region or group to view details");
    infoText->setFixedHeight(60);
    infoText->setStyleSheet("color: #000000; background-color: #ffffff;");
    layout->addWidget(infoText);
    
    helpButton = new QPushButton(this);
    helpButton->setFixedSize(36, 36);
    helpButton->setIcon(IconManager::instance().getIcon("help"));
    helpButton->setIconSize(QSize(24, 24));
    helpButton->setToolTip("Show help (Shortcut: F1)");
    helpButton->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid #2196F3;"
        "    border-radius: 18px;"
        "    background-color: #2196F3;"
        "    color: white;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "    border-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "}"
    );
    connect(helpButton, &QPushButton::clicked, this, &SidePanelWidget::helpClicked);
    layout->addWidget(helpButton);
}

void SidePanelWidget::setupRegionsTab() {
    regionsTab = new QWidget();
    QVBoxLayout* regionsLayout = new QVBoxLayout(regionsTab);
    regionsLayout->setContentsMargins(5, 5, 5, 5);
    regionsLayout->setSpacing(5);
    
    QLabel* regionsLabel = new QLabel("Defined Regions", regionsTab);
    regionsLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #000000;");
    regionsLayout->addWidget(regionsLabel);
    
    regionListBox = new QListWidget(regionsTab);
    regionListBox->addItem("No regions defined");
    regionListBox->setEnabled(false);
    regionListBox->setStyleSheet("color: #000000; background-color: #ffffff;");
    regionListBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
    regionsLayout->addWidget(regionListBox, 1);
    
    // Connect region listbox selection
    connect(regionListBox, &QListWidget::itemSelectionChanged, this, [this]() {
        QList<QListWidgetItem*> selectedItems = regionListBox->selectedItems();
        QSet<QString> selectedRegions;
        
        for (QListWidgetItem* item : selectedItems) {
            QString displayText = item->text();
            // Extract region name (remove [group] suffix if present)
            QString regionName = displayText.split(" [").first();
            selectedRegions.insert(regionName);
        }
        
        emit regionSelectionChanged(selectedRegions);
    });
    
    QFrame* regionButtonsFrame = new QFrame(regionsTab);
    QHBoxLayout* regionButtonsLayout = new QHBoxLayout(regionButtonsFrame);
    regionButtonsLayout->setContentsMargins(0, 0, 0, 0);
    
    // Note: Edit/Delete buttons are handled by MainWindow via signals
    // These buttons would be added here if needed, but for now
    // we rely on MainWindow's existing button connections
    
    regionsLayout->addWidget(regionButtonsFrame);
    sidePanelTabs->addTab(regionsTab, "Regions");
}

void SidePanelWidget::setupGroupsTab() {
    groupsTab = new QWidget();
    QVBoxLayout* groupsLayout = new QVBoxLayout(groupsTab);
    groupsLayout->setContentsMargins(5, 5, 5, 5);
    groupsLayout->setSpacing(5);
    
    QLabel* groupsLabel = new QLabel("Region Groups", groupsTab);
    groupsLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #000000;");
    groupsLayout->addWidget(groupsLabel);
    
    groupListBox = new QListWidget(groupsTab);
    groupListBox->addItem("No groups defined");
    groupListBox->setEnabled(false);
    groupListBox->setStyleSheet("color: #000000; background-color: #ffffff;");
    groupsLayout->addWidget(groupListBox, 1);
    
    QFrame* groupButtonsFrame = new QFrame(groupsTab);
    QHBoxLayout* groupButtonsLayout = new QHBoxLayout(groupButtonsFrame);
    groupButtonsLayout->setContentsMargins(0, 0, 0, 0);
    
    createGroupButton = new QPushButton(groupButtonsFrame);
    createGroupButton->setFixedSize(36, 36);
    createGroupButton->setIcon(IconManager::instance().getIcon("group"));
    createGroupButton->setIconSize(QSize(20, 20));
    createGroupButton->setToolTip("Create Group - Create a new group from selected regions (Shortcut: Cmd+G / Ctrl+G)\nRequires at least 2 selected regions");
    createGroupButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #4CAF50;"
        "    border-radius: 4px;"
        "    background-color: #4CAF50;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d8b40;"
        "}"
    );
    connect(createGroupButton, &QPushButton::clicked, this, &SidePanelWidget::createGroupClicked);
    
    addToGroupButton = new QPushButton(groupButtonsFrame);
    addToGroupButton->setFixedSize(36, 36);
    addToGroupButton->setIcon(IconManager::instance().getIcon("add"));
    addToGroupButton->setIconSize(QSize(20, 20));
    addToGroupButton->setToolTip("Add to Group - Add selected regions to an existing or new group");
    addToGroupButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #2196F3;"
        "    border-radius: 4px;"
        "    background-color: #2196F3;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "}"
    );
    connect(addToGroupButton, &QPushButton::clicked, this, &SidePanelWidget::addToGroupClicked);
    
    removeFromGroupButton = new QPushButton(groupButtonsFrame);
    removeFromGroupButton->setFixedSize(36, 36);
    removeFromGroupButton->setIcon(IconManager::instance().getIcon("remove"));
    removeFromGroupButton->setIconSize(QSize(20, 20));
    removeFromGroupButton->setToolTip("Remove from Group - Remove selected regions from their groups (Shortcut: Cmd+Shift+G / Ctrl+Shift+G)");
    removeFromGroupButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #FF9800;"
        "    border-radius: 4px;"
        "    background-color: #FF9800;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F57C00;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #EF6C00;"
        "}"
    );
    connect(removeFromGroupButton, &QPushButton::clicked, this, &SidePanelWidget::removeFromGroupClicked);
    
    deleteGroupButton = new QPushButton(groupButtonsFrame);
    deleteGroupButton->setFixedSize(36, 36);
    deleteGroupButton->setIcon(IconManager::instance().getIcon("delete"));
    deleteGroupButton->setIconSize(QSize(20, 20));
    deleteGroupButton->setToolTip("Delete Group - Delete the selected group (regions are not deleted, only removed from group)");
    deleteGroupButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #f44336;"
        "    border-radius: 4px;"
        "    background-color: #f44336;"
        "}"
        "QPushButton:hover {"
        "    background-color: #d32f2f;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c62828;"
        "}"
    );
    connect(deleteGroupButton, &QPushButton::clicked, this, &SidePanelWidget::deleteGroupClicked);
    
    groupButtonsLayout->addWidget(createGroupButton);
    groupButtonsLayout->addWidget(addToGroupButton);
    groupButtonsLayout->addWidget(removeFromGroupButton);
    groupButtonsLayout->addWidget(deleteGroupButton);
    
    groupsLayout->addWidget(groupButtonsFrame);
    sidePanelTabs->addTab(groupsTab, "Groups");
    
    // Connect group listbox selection to info display
    connect(groupListBox, &QListWidget::itemSelectionChanged, this, [this]() {
        QList<QListWidgetItem*> selectedItems = groupListBox->selectedItems();
        if (selectedItems.isEmpty()) {
            return;
        }
        
        QString displayText = selectedItems[0]->text();
        QString groupName = displayText.split(" (").first();
        emit groupSelectionChanged(groupName);
    });
}

void SidePanelWidget::updateRegionListBox(const QList<QString>& regionNames, const QSet<QString>& selectedRegions, const QMap<QString, QString>& regionGroups) {
    if (!regionListBox) {
        return;
    }
    
    // Block signals while updating to prevent cascading updates
    regionListBox->blockSignals(true);
    
    regionListBox->clear();
    
    if (regionNames.isEmpty()) {
        regionListBox->addItem("No regions defined");
        regionListBox->setEnabled(false);
        regionListBox->blockSignals(false);
        return;
    }
    
    regionListBox->setEnabled(true);
    
    // Store current selection to restore after update
    QSet<QString> currentSelection = selectedRegions;
    
    for (const QString& regionName : regionNames) {
        QString displayText = regionName;
        // Add group info if region is in a group
        if (regionGroups.contains(regionName) && !regionGroups[regionName].isEmpty()) {
            displayText += " [" + regionGroups[regionName] + "]";
        }
        QListWidgetItem* item = new QListWidgetItem(displayText, regionListBox);
        if (currentSelection.contains(regionName)) {
            item->setSelected(true);
        }
    }
    
    // Re-enable signals
    regionListBox->blockSignals(false);
}

void SidePanelWidget::updateGroupListBox(const QList<QString>& groupNames, const QMap<QString, int>& groupSizes) {
    groupListBox->clear();
    
    if (groupNames.isEmpty()) {
        groupListBox->addItem("No groups defined");
        groupListBox->setEnabled(false);
        return;
    }
    
    groupListBox->setEnabled(true);
    
    for (const QString& groupName : groupNames) {
        int size = groupSizes.value(groupName, 0);
        QString displayText = QString("%1 (%2)").arg(groupName).arg(size);
        groupListBox->addItem(displayText);
    }
}

void SidePanelWidget::setInfoText(const QString& text) {
    infoText->setPlainText(text);
}

void SidePanelWidget::setupRegionEditor() {
    regionEditorFrame = new QFrame(this);
    regionEditorFrame->setStyleSheet(
        "QFrame {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 4px;"
        "}"
    );
    
    // Create main layout for the frame
    QVBoxLayout* frameLayout = new QVBoxLayout(regionEditorFrame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);
    
    // Create scroll area for the entire editor content
    QScrollArea* editorScrollArea = new QScrollArea(regionEditorFrame);
    editorScrollArea->setWidgetResizable(true);
    editorScrollArea->setFrameShape(QFrame::NoFrame);
    editorScrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
    );
    
    // Create container widget for all editor content
    QWidget* editorContainer = new QWidget(editorScrollArea);
    editorContainer->setStyleSheet("background-color: #ffffff;");
    QVBoxLayout* editorLayout = new QVBoxLayout(editorContainer);
    editorLayout->setContentsMargins(10, 10, 10, 10);
    editorLayout->setSpacing(8);
    
    // Title
    regionEditorLabel = new QLabel("Edit Region", editorContainer);
    regionEditorLabel->setStyleSheet("font-weight: bold; font-size: 11pt; color: #000000;");
    editorLayout->addWidget(regionEditorLabel);
    
    // Name field
    QLabel* nameLabel = new QLabel("Name:", editorContainer);
    nameLabel->setStyleSheet("color: #000000;");
    editorLayout->addWidget(nameLabel);
    
    regionNameEdit = new QLineEdit(editorContainer);
    regionNameEdit->setStyleSheet(
        "QLineEdit {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #2196F3;"
        "}"
    );
    connect(regionNameEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!currentEditingRegion.isEmpty()) {
            QString newName = regionNameEdit->text().trimmed();
            if (!newName.isEmpty() && newName != currentEditingRegion) {
                emit regionNameChanged(currentEditingRegion, newName);
            }
        }
    });
    editorLayout->addWidget(regionNameEdit);
    
    // Color selector
    QLabel* colorLabel = new QLabel("Color:", editorContainer);
    colorLabel->setStyleSheet("color: #000000;");
    editorLayout->addWidget(colorLabel);
    
    regionColorCombo = new QComboBox(editorContainer);
    regionColorCombo->addItems({"blue", "red", "green", "yellow", "purple", "orange", "cyan"});
    regionColorCombo->setStyleSheet(
        "QComboBox {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #2196F3;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
    );
    connect(regionColorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!currentEditingRegion.isEmpty()) {
            QString newColor = regionColorCombo->itemText(index);
            emit regionColorChanged(currentEditingRegion, newColor);
        }
    });
    editorLayout->addWidget(regionColorCombo);
    
    // Group selector
    QLabel* groupLabel = new QLabel("Group:", editorContainer);
    groupLabel->setStyleSheet("color: #000000;");
    editorLayout->addWidget(groupLabel);
    
    regionGroupCombo = new QComboBox(editorContainer);
    regionGroupCombo->addItem("(None)", "");  // Empty string for no group
    regionGroupCombo->setStyleSheet(
        "QComboBox {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #2196F3;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
    );
    connect(regionGroupCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!currentEditingRegion.isEmpty()) {
            QString newGroup = regionGroupCombo->itemData(index).toString();
            emit regionGroupChanged(currentEditingRegion, newGroup);
        }
    });
    editorLayout->addWidget(regionGroupCombo);
    
    // Region Type selector
    QLabel* typeLabel = new QLabel("Type:", editorContainer);
    typeLabel->setStyleSheet("color: #000000;");
    editorLayout->addWidget(typeLabel);
    
    regionTypeCombo = new QComboBox(editorContainer);
    regionTypeCombo->addItems({"none", "text", "alphanumeric", "letters", "numbers", "roman", "unicode"});
    regionTypeCombo->setStyleSheet(
        "QComboBox {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #2196F3;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
    );
    connect(regionTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!currentEditingRegion.isEmpty()) {
            QString newType = regionTypeCombo->itemText(index);
            emit regionTypeChanged(currentEditingRegion, newType);
        }
    });
    editorLayout->addWidget(regionTypeCombo);
    
    // Percentage Fill selector
    QLabel* percentageFillLabel = new QLabel("Percentage Fill:", editorContainer);
    percentageFillLabel->setStyleSheet("color: #000000;");
    editorLayout->addWidget(percentageFillLabel);
    
    regionPercentageFillCombo = new QComboBox(editorContainer);
    regionPercentageFillCombo->addItems({"none", "standard"});
    regionPercentageFillCombo->setStyleSheet(
        "QComboBox {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #2196F3;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
    );
    connect(regionPercentageFillCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!currentEditingRegion.isEmpty()) {
            QString newPercentageFill = regionPercentageFillCombo->itemText(index);
            emit regionPercentageFillChanged(currentEditingRegion, newPercentageFill);
        }
    });
    editorLayout->addWidget(regionPercentageFillCombo);
    
    // Coordinates section
    QLabel* coordsLabel = new QLabel("Coordinates (Normalized):", editorContainer);
    coordsLabel->setStyleSheet("color: #000000; font-weight: bold; margin-top: 8px;");
    editorLayout->addWidget(coordsLabel);
    
    // Helper lambda to create a coordinate control (label + - button + value + + button)
    auto createCoordinateControl = [editorContainer](const QString& labelText) -> QHBoxLayout* {
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setSpacing(4);
        
        QLabel* label = new QLabel(labelText + ":", editorContainer);
        label->setStyleSheet("color: #000000; min-width: 30px;");
        layout->addWidget(label);
        
        QPushButton* decreaseBtn = new QPushButton("−", editorContainer);
        decreaseBtn->setFixedSize(30, 30);
        decreaseBtn->setAutoRepeat(true);
        decreaseBtn->setAutoRepeatDelay(300);
        decreaseBtn->setAutoRepeatInterval(50);
        decreaseBtn->setStyleSheet(
            "QPushButton {"
            "    border: 1px solid #d0d0d0;"
            "    border-radius: 3px;"
            "    background-color: #f5f5f5;"
            "    color: #000000;"
            "    font-weight: bold;"
            "    font-size: 16px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e0e0e0;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #d0d0d0;"
            "}"
        );
        
        QLabel* valueLabel = new QLabel("0.000000", editorContainer);
        valueLabel->setFixedWidth(100);
        valueLabel->setStyleSheet(
            "QLabel {"
            "    border: 2px solid #000000;"
            "    border-radius: 4px;"
            "    padding: 6px;"
            "    background-color: #000000;"
            "    color: #00ffff;"
            "    font-family: 'Courier New', monospace;"
            "    font-size: 12px;"
            "    font-weight: bold;"
            "}"
        );
        valueLabel->setAlignment(Qt::AlignCenter);
        
        QPushButton* increaseBtn = new QPushButton("+", editorContainer);
        increaseBtn->setFixedSize(30, 30);
        increaseBtn->setAutoRepeat(true);
        increaseBtn->setAutoRepeatDelay(300);
        increaseBtn->setAutoRepeatInterval(50);
        increaseBtn->setStyleSheet(
            "QPushButton {"
            "    border: 1px solid #d0d0d0;"
            "    border-radius: 3px;"
            "    background-color: #f5f5f5;"
            "    color: #000000;"
            "    font-weight: bold;"
            "    font-size: 18px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e0e0e0;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #d0d0d0;"
            "}"
        );
        
        layout->addWidget(decreaseBtn);
        layout->addWidget(valueLabel);
        layout->addWidget(increaseBtn);
        layout->addStretch();
        
        return layout;
    };
    
    // Create coordinate controls
    QVBoxLayout* coordsLayout = new QVBoxLayout();
    coordsLayout->setSpacing(6);
    
    // X1 control
    QHBoxLayout* x1Layout = createCoordinateControl("X1");
    regionX1DecreaseBtn = qobject_cast<QPushButton*>(x1Layout->itemAt(1)->widget());
    regionX1ValueLabel = qobject_cast<QLabel*>(x1Layout->itemAt(2)->widget());
    regionX1IncreaseBtn = qobject_cast<QPushButton*>(x1Layout->itemAt(3)->widget());
    coordsLayout->addLayout(x1Layout);
    
    // Y1 control
    QHBoxLayout* y1Layout = createCoordinateControl("Y1");
    regionY1DecreaseBtn = qobject_cast<QPushButton*>(y1Layout->itemAt(1)->widget());
    regionY1ValueLabel = qobject_cast<QLabel*>(y1Layout->itemAt(2)->widget());
    regionY1IncreaseBtn = qobject_cast<QPushButton*>(y1Layout->itemAt(3)->widget());
    coordsLayout->addLayout(y1Layout);
    
    // X2 control
    QHBoxLayout* x2Layout = createCoordinateControl("X2");
    regionX2DecreaseBtn = qobject_cast<QPushButton*>(x2Layout->itemAt(1)->widget());
    regionX2ValueLabel = qobject_cast<QLabel*>(x2Layout->itemAt(2)->widget());
    regionX2IncreaseBtn = qobject_cast<QPushButton*>(x2Layout->itemAt(3)->widget());
    coordsLayout->addLayout(x2Layout);
    
    // Y2 control
    QHBoxLayout* y2Layout = createCoordinateControl("Y2");
    regionY2DecreaseBtn = qobject_cast<QPushButton*>(y2Layout->itemAt(1)->widget());
    regionY2ValueLabel = qobject_cast<QLabel*>(y2Layout->itemAt(2)->widget());
    regionY2IncreaseBtn = qobject_cast<QPushButton*>(y2Layout->itemAt(3)->widget());
    coordsLayout->addLayout(y2Layout);
    
    editorLayout->addLayout(coordsLayout);
    
    // Connect coordinate buttons
    connect(regionX1DecreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("x1", -0.001); });
    connect(regionX1IncreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("x1", 0.001); });
    connect(regionY1DecreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("y1", -0.001); });
    connect(regionY1IncreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("y1", 0.001); });
    connect(regionX2DecreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("x2", -0.001); });
    connect(regionX2IncreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("x2", 0.001); });
    connect(regionY2DecreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("y2", -0.001); });
    connect(regionY2IncreaseBtn, &QPushButton::clicked, this, [this]() { updateCoordinateValue("y2", 0.001); });
    
    editorLayout->addStretch();
    
    // Set the container as the scroll area's widget
    editorScrollArea->setWidget(editorContainer);
    
    // Add scroll area to frame layout
    frameLayout->addWidget(editorScrollArea);
}

void SidePanelWidget::showRegionEditor(const QString& regionName, const QString& regionColor, 
                                      const QString& regionGroup, const QList<QString>& availableGroups,
                                      const QString& regionType, const QString& percentageFill,
                                      double x1, double y1, double x2, double y2) {
    currentEditingRegion = regionName;
    
    // Block signals while updating to prevent infinite recursion
    regionNameEdit->blockSignals(true);
    regionColorCombo->blockSignals(true);
    regionGroupCombo->blockSignals(true);
    regionTypeCombo->blockSignals(true);
    regionPercentageFillCombo->blockSignals(true);
    
    // Update label
    regionEditorLabel->setText(QString("Edit: %1").arg(regionName));
    
    // Update name field
    regionNameEdit->setText(regionName);
    
    // Update color combo
    int colorIndex = regionColorCombo->findText(regionColor);
    if (colorIndex >= 0) {
        regionColorCombo->setCurrentIndex(colorIndex);
    } else {
        regionColorCombo->setCurrentIndex(0);  // Default to blue
    }
    
    // Update group combo
    regionGroupCombo->clear();
    regionGroupCombo->addItem("(None)", "");
    for (const QString& group : availableGroups) {
        regionGroupCombo->addItem(group, group);
    }
    
    // Set current group
    int groupIndex = regionGroupCombo->findData(regionGroup);
    if (groupIndex >= 0) {
        regionGroupCombo->setCurrentIndex(groupIndex);
    } else {
        regionGroupCombo->setCurrentIndex(0);  // None
    }
    
    // Update type combo
    int typeIndex = regionTypeCombo->findText(regionType);
    if (typeIndex >= 0) {
        regionTypeCombo->setCurrentIndex(typeIndex);
    } else {
        regionTypeCombo->setCurrentIndex(0);  // Default to "none"
    }
    
    // Update percentage fill combo
    int percentageFillIndex = regionPercentageFillCombo->findText(percentageFill);
    if (percentageFillIndex >= 0) {
        regionPercentageFillCombo->setCurrentIndex(percentageFillIndex);
    } else {
        regionPercentageFillCombo->setCurrentIndex(0);  // Default to "none"
    }
    
    // Update coordinates
    regionX1ValueLabel->setText(QString::number(x1, 'f', 6));
    regionY1ValueLabel->setText(QString::number(y1, 'f', 6));
    regionX2ValueLabel->setText(QString::number(x2, 'f', 6));
    regionY2ValueLabel->setText(QString::number(y2, 'f', 6));
    
    // Re-enable signals
    regionNameEdit->blockSignals(false);
    regionColorCombo->blockSignals(false);
    regionGroupCombo->blockSignals(false);
    regionTypeCombo->blockSignals(false);
    regionPercentageFillCombo->blockSignals(false);
    
    // Show the editor
    regionEditorFrame->show();
}

void SidePanelWidget::hideRegionEditor() {
    currentEditingRegion.clear();
    regionEditorFrame->hide();
}

void SidePanelWidget::updateCoordinateValue(const QString& coordName, double delta) {
    if (currentEditingRegion.isEmpty()) {
        return;
    }
    
    // Validate delta for NaN/Infinity
    if (!CoordinateSystem::isValidDouble(delta)) {
        return; // Invalid delta
    }
    
    // Get current values from labels
    bool ok;
    double x1 = regionX1ValueLabel->text().toDouble(&ok);
    if (!ok) return;
    double y1 = regionY1ValueLabel->text().toDouble(&ok);
    if (!ok) return;
    double x2 = regionX2ValueLabel->text().toDouble(&ok);
    if (!ok) return;
    double y2 = regionY2ValueLabel->text().toDouble(&ok);
    if (!ok) return;
    
    // Validate parsed values for NaN/Infinity
    if (!CoordinateSystem::isValidDouble(x1) || !CoordinateSystem::isValidDouble(y1) ||
        !CoordinateSystem::isValidDouble(x2) || !CoordinateSystem::isValidDouble(y2)) {
        return; // Invalid coordinates
    }
    
    // Update the specified coordinate
    if (coordName == "x1") {
        x1 += delta;
    } else if (coordName == "y1") {
        y1 += delta;
    } else if (coordName == "x2") {
        x2 += delta;
    } else if (coordName == "y2") {
        y2 += delta;
    }
    
    // Validate updated values for NaN/Infinity
    if (!CoordinateSystem::isValidDouble(x1) || !CoordinateSystem::isValidDouble(y1) ||
        !CoordinateSystem::isValidDouble(x2) || !CoordinateSystem::isValidDouble(y2)) {
        return; // Invalid coordinates after update
    }
    
    // Clamp coordinates to valid range [0.0, 1.0]
    x1 = qBound(0.0, x1, 1.0);
    y1 = qBound(0.0, y1, 1.0);
    x2 = qBound(0.0, x2, 1.0);
    y2 = qBound(0.0, y2, 1.0);
    
    // Auto-correct swapped coordinates (ensure x1 < x2 and y1 < y2)
    if (x1 > x2) {
        std::swap(x1, x2);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
    }
    
    // Ensure minimum size (at least 0.001 difference)
    if (x2 - x1 < 0.001) {
        if (x2 < 0.999) {
            x2 = x1 + 0.001;
        } else {
            x1 = x2 - 0.001;
        }
    }
    if (y2 - y1 < 0.001) {
        if (y2 < 0.999) {
            y2 = y1 + 0.001;
        } else {
            y1 = y2 - 0.001;
        }
    }
    
    // Update the labels with corrected values
    regionX1ValueLabel->setText(QString::number(x1, 'f', 6));
    regionY1ValueLabel->setText(QString::number(y1, 'f', 6));
    regionX2ValueLabel->setText(QString::number(x2, 'f', 6));
    regionY2ValueLabel->setText(QString::number(y2, 'f', 6));
    
    // Emit signal for real-time update
    emit regionCoordinatesChanged(currentEditingRegion, x1, y1, x2, y2);
}

void SidePanelWidget::toggleRegionEditor() {
    if (regionEditorFrame->isVisible()) {
        hideRegionEditor();
    } else {
        // Show editor - if there's a current editing region, show it
        // The selectionChanged signal should have populated it, but if not, just show the frame
        regionEditorFrame->show();
    }
}

bool SidePanelWidget::isRegionEditorVisible() const {
    return regionEditorFrame && regionEditorFrame->isVisible();
}

} // namespace ocr_orc

