#include "ExportDialog.h"
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

namespace ocr_orc {

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , exportTypeGroup(nullptr)
    , exportCoordinatesCheck(nullptr)
    , exportMaskCheck(nullptr)
    , coordinateFormatGroup(nullptr)
    , coordinateFormatCombo(nullptr)
    , maskFormatGroup(nullptr)
    , maskFormatCombo(nullptr)
    , destinationGroup(nullptr)
    , destinationEdit(nullptr)
    , browseButton(nullptr)
    , baseFileNameEdit(nullptr)
    , optionsGroup(nullptr)
    , selectedRegionsOnlyCheck(nullptr)
    , exportCoordinatesWithMaskCheck(nullptr)
{
    setWindowTitle("Export Configuration");
    setMinimumWidth(500);
    setMinimumHeight(400);
    setModal(true);
    
    setupUI();
    updateUI();
}

void ExportDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    setupExportTypeSection();
    mainLayout->addWidget(exportTypeGroup);

    setupFormatSection();
    mainLayout->addWidget(coordinateFormatGroup);
    mainLayout->addWidget(maskFormatGroup);

    setupDestinationSection();
    mainLayout->addWidget(destinationGroup);

    setupOptionsSection();
    mainLayout->addWidget(optionsGroup);

    mainLayout->addStretch();

    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        this
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ExportDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ExportDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void ExportDialog::setupExportTypeSection() {
    exportTypeGroup = new QGroupBox("Export Type", this);
    QVBoxLayout* layout = new QVBoxLayout(exportTypeGroup);
    layout->setSpacing(10);

    exportCoordinatesCheck = new QCheckBox("Export Coordinates", exportTypeGroup);
    exportCoordinatesCheck->setChecked(true);
    exportCoordinatesCheck->setToolTip("Export region coordinates in selected format");
    connect(exportCoordinatesCheck, &QCheckBox::toggled, this, &ExportDialog::onExportTypeChanged);
    layout->addWidget(exportCoordinatesCheck);

    exportMaskCheck = new QCheckBox("Export Mask Image", exportTypeGroup);
    exportMaskCheck->setToolTip("Export mask image with white rectangles for regions");
    connect(exportMaskCheck, &QCheckBox::toggled, this, &ExportDialog::onExportTypeChanged);
    layout->addWidget(exportMaskCheck);
}

void ExportDialog::setupFormatSection() {
    // Coordinate format
    coordinateFormatGroup = new QGroupBox("Coordinate Format", this);
    QHBoxLayout* coordLayout = new QHBoxLayout(coordinateFormatGroup);
    
    QLabel* coordLabel = new QLabel("Format:", coordinateFormatGroup);
    coordLayout->addWidget(coordLabel);
    
    coordinateFormatCombo = new QComboBox(coordinateFormatGroup);
    coordinateFormatCombo->addItem("JSON (.json)", "json");
    coordinateFormatCombo->addItem("CSV (.csv)", "csv");
    coordinateFormatCombo->setCurrentIndex(0);
    coordinateFormatCombo->setToolTip("File format for coordinate export");
    coordLayout->addWidget(coordinateFormatCombo);
    coordLayout->addStretch();

    // Mask format
    maskFormatGroup = new QGroupBox("Mask Format", this);
    QHBoxLayout* maskLayout = new QHBoxLayout(maskFormatGroup);
    
    QLabel* maskLabel = new QLabel("Format:", maskFormatGroup);
    maskLayout->addWidget(maskLabel);
    
    maskFormatCombo = new QComboBox(maskFormatGroup);
    maskFormatCombo->addItem("PNG (.png)", "png");
    maskFormatCombo->addItem("JPEG (.jpg)", "jpg");
    maskFormatCombo->addItem("TIFF (.tiff)", "tiff");
    maskFormatCombo->setCurrentIndex(0);
    maskFormatCombo->setToolTip("Image format for mask export");
    maskLayout->addWidget(maskFormatCombo);
    maskLayout->addStretch();
}

void ExportDialog::setupDestinationSection() {
    destinationGroup = new QGroupBox("Destination", this);
    QVBoxLayout* layout = new QVBoxLayout(destinationGroup);
    layout->setSpacing(10);

    // Destination directory
    QHBoxLayout* dirLayout = new QHBoxLayout();
    QLabel* dirLabel = new QLabel("Directory:", destinationGroup);
    dirLayout->addWidget(dirLabel);
    
    destinationEdit = new QLineEdit(destinationGroup);
    destinationEdit->setPlaceholderText("Select destination folder...");
    destinationEdit->setReadOnly(true);
    dirLayout->addWidget(destinationEdit, 1);
    
    browseButton = new QPushButton("Browse...", destinationGroup);
    connect(browseButton, &QPushButton::clicked, this, &ExportDialog::onBrowseDestination);
    dirLayout->addWidget(browseButton);
    layout->addLayout(dirLayout);

    // Base file name
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("File Name:", destinationGroup);
    nameLayout->addWidget(nameLabel);
    
    baseFileNameEdit = new QLineEdit("coordinates", destinationGroup);
    baseFileNameEdit->setPlaceholderText("Enter base file name (extensions added automatically)");
    baseFileNameEdit->setToolTip("Base name for exported files. Extensions will be added based on format.");
    nameLayout->addWidget(baseFileNameEdit, 1);
    layout->addLayout(nameLayout);
}

void ExportDialog::setupOptionsSection() {
    optionsGroup = new QGroupBox("Options", this);
    QVBoxLayout* layout = new QVBoxLayout(optionsGroup);
    layout->setSpacing(10);

    selectedRegionsOnlyCheck = new QCheckBox("Export selected regions only", optionsGroup);
    selectedRegionsOnlyCheck->setToolTip("If checked, only currently selected regions will be exported");
    layout->addWidget(selectedRegionsOnlyCheck);

    exportCoordinatesWithMaskCheck = new QCheckBox("Export coordinates JSON with mask", optionsGroup);
    exportCoordinatesWithMaskCheck->setChecked(true);
    exportCoordinatesWithMaskCheck->setToolTip("When exporting mask, also save coordinates as JSON file");
    layout->addWidget(exportCoordinatesWithMaskCheck);
}

void ExportDialog::updateUI() {
    bool exportCoords = exportCoordinatesCheck->isChecked();
    bool exportMask = exportMaskCheck->isChecked();

    coordinateFormatGroup->setEnabled(exportCoords);
    maskFormatGroup->setEnabled(exportMask);
    exportCoordinatesWithMaskCheck->setEnabled(exportMask);
}

void ExportDialog::onExportTypeChanged() {
    updateUI();
}

void ExportDialog::onBrowseDestination() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Export Destination",
        destinationEdit->text().isEmpty() ? QDir::homePath() : destinationEdit->text()
    );

    if (!dir.isEmpty()) {
        destinationEdit->setText(dir);
    }
}

int ExportDialog::getExportTypes() const {
    int types = 0;
    if (exportCoordinatesCheck->isChecked()) {
        types |= 1; // Coordinates
    }
    if (exportMaskCheck->isChecked()) {
        types |= 2; // Mask
    }
    return types;
}

QString ExportDialog::getCoordinateFormat() const {
    return coordinateFormatCombo->currentData().toString();
}

QString ExportDialog::getMaskFormat() const {
    return maskFormatCombo->currentData().toString();
}

QString ExportDialog::getDestinationDirectory() const {
    return destinationEdit->text();
}

QString ExportDialog::getBaseFileName() const {
    QString name = baseFileNameEdit->text().trimmed();
    if (name.isEmpty()) {
        return "coordinates";
    }
    return name;
}

bool ExportDialog::exportSelectedOnly() const {
    return selectedRegionsOnlyCheck->isChecked();
}

bool ExportDialog::exportCoordinatesWithMask() const {
    return exportCoordinatesWithMaskCheck->isChecked();
}

void ExportDialog::accept() {
    // Validate
    if (getExportTypes() == 0) {
        QMessageBox::warning(this, "Export", "Please select at least one export type.");
        return;
    }

    if (destinationEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Export", "Please select a destination directory.");
        return;
    }

    QDir dir(destinationEdit->text());
    if (!dir.exists()) {
        QMessageBox::warning(this, "Export", "Selected directory does not exist.");
        return;
    }

    QString baseName = getBaseFileName();
    if (baseName.isEmpty()) {
        QMessageBox::warning(this, "Export", "Please enter a file name.");
        return;
    }

    QDialog::accept();
}

} // namespace ocr_orc

