#include "ControlPanelWidget.h"
#include "../icons/IconManager.h"
#include <QtWidgets/QHBoxLayout>

namespace ocr_orc {

ControlPanelWidget::ControlPanelWidget(QWidget *parent)
    : QFrame(parent)
    , loadPdfButton(nullptr)
    , fileLabel(nullptr)
    , exportCoordinatesButton(nullptr)
    , exportMaskButton(nullptr)
    , loadCoordinatesButton(nullptr)
    , testExtractionButton(nullptr)
    , regionNameLabel(nullptr)
    , regionNameEdit(nullptr)
    , groupLabel(nullptr)
    , groupEdit(nullptr)
    , colorLabel(nullptr)
    , colorComboBox(nullptr)
    , maskOutsideCheckBox(nullptr)
    , clearSelectionButton(nullptr)
    , editSelectedButton(nullptr)
{
    setFixedHeight(80);
    setStyleSheet(
        "QFrame {"
        "    background-color: #f0f0f0;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 0px;"
        "}"
    );
    setFrameShape(QFrame::Box);
    setupUI();
}

void ControlPanelWidget::setupUI() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(10);
    
    setupFile(layout);
    layout->addSpacing(20);
    setupRegionInput(layout);
    layout->addSpacing(20);
    setupActions(layout);
    layout->addStretch();
}

void ControlPanelWidget::setupFile(QHBoxLayout* layout) {
    loadPdfButton = new QPushButton(this);
    loadPdfButton->setFixedSize(40, 40);
    loadPdfButton->setIcon(IconManager::instance().getIcon("open"));
    loadPdfButton->setIconSize(QSize(24, 24));
    loadPdfButton->setToolTip("Load PDF - Load a PDF document (Shortcut: Cmd+O / Ctrl+O)\nOr drag and drop a PDF file onto the window");
    loadPdfButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #2196F3;"
        "    border-radius: 4px;"
        "    background-color: #2196F3;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "    border-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "}"
    );
    connect(loadPdfButton, &QPushButton::clicked, this, &ControlPanelWidget::loadPdfClicked);
    layout->addWidget(loadPdfButton);
    
    fileLabel = new QLabel("No file loaded", this);
    fileLabel->setMinimumWidth(100);
    fileLabel->setMaximumWidth(200);
    fileLabel->setToolTip("Currently loaded PDF file name");
    fileLabel->setStyleSheet("QLabel { color: #000000; }");
    fileLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(fileLabel);
    
    // Consolidated export button - opens export dialog
    exportCoordinatesButton = new QPushButton(this);
    exportCoordinatesButton->setFixedSize(40, 40);
    exportCoordinatesButton->setIcon(IconManager::instance().getIcon("export"));
    exportCoordinatesButton->setIconSize(QSize(24, 24));
    exportCoordinatesButton->setToolTip("Export - Export coordinates, masks, and other formats (Shortcut: Cmd+S / Ctrl+S)");
    exportCoordinatesButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #4CAF50;"
        "    border-radius: 4px;"
        "    background-color: #4CAF50;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "    border-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d8b40;"
        "}"
    );
    connect(exportCoordinatesButton, &QPushButton::clicked, this, &ControlPanelWidget::exportCoordinatesClicked);
    layout->addWidget(exportCoordinatesButton);
    
    // Hide the separate export mask button - consolidated into export dialog
    exportMaskButton = nullptr;
    
    loadCoordinatesButton = new QPushButton(this);
    loadCoordinatesButton->setFixedSize(40, 40);
    loadCoordinatesButton->setIcon(IconManager::instance().getIcon("import"));
    loadCoordinatesButton->setIconSize(QSize(24, 24));
    loadCoordinatesButton->setToolTip("Load Coordinates - Import coordinates from JSON file (Shortcut: Cmd+Shift+S / Ctrl+Shift+S)");
    loadCoordinatesButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #FF9800;"
        "    border-radius: 4px;"
        "    background-color: #FF9800;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F57C00;"
        "    border-color: #F57C00;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #EF6C00;"
        "}"
    );
    connect(loadCoordinatesButton, &QPushButton::clicked, this, &ControlPanelWidget::loadCoordinatesClicked);
    layout->addWidget(loadCoordinatesButton);
    
    testExtractionButton = new QPushButton(this);
    testExtractionButton->setFixedSize(40, 40);
    testExtractionButton->setIcon(IconManager::instance().getIcon("edit"));
    testExtractionButton->setIconSize(QSize(24, 24));
    testExtractionButton->setToolTip("Test Extraction - Test OCR extraction (future feature)");
    testExtractionButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #757575;"
        "    border-radius: 4px;"
        "    background-color: #757575;"
        "    color: white;"
        "}"
        "QPushButton:hover {"
        "    background-color: #616161;"
        "    border-color: #616161;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #424242;"
        "}"
    );
    layout->addWidget(testExtractionButton);
}

void ControlPanelWidget::setupRegionInput(QHBoxLayout* layout) {
    regionNameLabel = new QLabel("Region Name:", this);
    regionNameLabel->setStyleSheet("color: #000000;");
    layout->addWidget(regionNameLabel);
    
    regionNameEdit = new QLineEdit("Last Name Cell 1", this);
    regionNameEdit->setMinimumWidth(120);
    regionNameEdit->setMaximumWidth(250);
    regionNameEdit->setStyleSheet("color: #000000; background-color: #ffffff;");
    regionNameEdit->setToolTip("Name for the new region\nLeave empty for auto-generated names");
    layout->addWidget(regionNameEdit);
    
    groupLabel = new QLabel("Group:", this);
    groupLabel->setStyleSheet("color: #000000;");
    groupLabel->setToolTip("Optional group name for the new region");
    layout->addWidget(groupLabel);
    
    groupEdit = new QLineEdit("", this);
    groupEdit->setMinimumWidth(100);
    groupEdit->setMaximumWidth(180);
    groupEdit->setStyleSheet("color: #000000; background-color: #ffffff;");
    groupEdit->setPlaceholderText("(optional)");
    groupEdit->setToolTip("Optional group name (e.g., 'lastname', 'firstname')\nRegions can be organized into groups");
    layout->addWidget(groupEdit);
    
    colorLabel = new QLabel("Color:", this);
    colorLabel->setStyleSheet("color: #000000;");
    colorLabel->setToolTip("Color for the new region");
    layout->addWidget(colorLabel);
    
    colorComboBox = new QComboBox(this);
    colorComboBox->addItems(QStringList() << "blue" << "red" << "green" << "yellow" << "purple" << "orange" << "cyan");
    colorComboBox->setMinimumWidth(70);
    colorComboBox->setMaximumWidth(120);
    colorComboBox->setStyleSheet("color: #000000; background-color: #ffffff;");
    colorComboBox->setToolTip("Color for the new region border and fill");
    layout->addWidget(colorComboBox);
}

void ControlPanelWidget::setupActions(QHBoxLayout* layout) {
    maskOutsideCheckBox = new QCheckBox("Mask Outside", this);
    maskOutsideCheckBox->setStyleSheet("color: #000000;");
    maskOutsideCheckBox->setToolTip("Mask areas outside regions (future feature)");
    layout->addWidget(maskOutsideCheckBox);
    
    clearSelectionButton = new QPushButton(this);
    clearSelectionButton->setFixedSize(40, 40);
    clearSelectionButton->setIcon(IconManager::instance().getIcon("clear"));
    clearSelectionButton->setIconSize(QSize(24, 24));
    clearSelectionButton->setToolTip("Delete Selected - Delete all selected regions");
    clearSelectionButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #757575;"
        "    border-radius: 4px;"
        "    background-color: #757575;"
        "}"
        "QPushButton:hover {"
        "    background-color: #616161;"
        "    border-color: #616161;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #424242;"
        "}"
    );
    connect(clearSelectionButton, &QPushButton::clicked, this, &ControlPanelWidget::clearSelectionClicked);
    layout->addWidget(clearSelectionButton);
    
    editSelectedButton = new QPushButton(this);
    editSelectedButton->setFixedSize(40, 40);
    editSelectedButton->setIcon(IconManager::instance().getIcon("edit"));
    editSelectedButton->setIconSize(QSize(24, 24));
    editSelectedButton->setToolTip("Edit Selected - Edit name of selected region (Shortcut: E)");
    editSelectedButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #009688;"
        "    border-radius: 4px;"
        "    background-color: #009688;"
        "}"
        "QPushButton:hover {"
        "    background-color: #00796B;"
        "    border-color: #00796B;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #00695C;"
        "}"
    );
    connect(editSelectedButton, &QPushButton::clicked, this, &ControlPanelWidget::editSelectedClicked);
    layout->addWidget(editSelectedButton);
}

QString ControlPanelWidget::getRegionName() const {
    return regionNameEdit->text().trimmed();
}

QString ControlPanelWidget::getGroupName() const {
    return groupEdit->text().trimmed();
}

QString ControlPanelWidget::getSelectedColor() const {
    return colorComboBox->currentText();
}

void ControlPanelWidget::clearRegionName() {
    regionNameEdit->clear();
}

void ControlPanelWidget::clearGroupName() {
    groupEdit->clear();
}

void ControlPanelWidget::setFileLabel(const QString& text) {
    fileLabel->setText(text);
}

} // namespace ocr_orc

