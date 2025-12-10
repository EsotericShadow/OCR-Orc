#include "PreferencesDialog.h"
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>

namespace ocr_orc {

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , defaultRegionColorCombo(nullptr)
    , defaultRegionNamePatternEdit(nullptr)
    , autoSaveCheckBox(nullptr)
    , autoSaveIntervalSpinBox(nullptr)
    , defaultZoomSpinBox(nullptr)
    , showGridCheckBox(nullptr)
    , snapToGridCheckBox(nullptr)
    , gridSizeSpinBox(nullptr)
    , handleSizeSpinBox(nullptr)
    , defaultCoordinateFormatCombo(nullptr)
    , defaultMaskFormatCombo(nullptr)
    , defaultExportPathEdit(nullptr)
    , browseExportPathButton(nullptr)
    , rememberLastExportPathCheckBox(nullptr)
    , showToolbarCheckBox(nullptr)
    , showStatusBarCheckBox(nullptr)
    , showSidePanelCheckBox(nullptr)
    , iconSizeSpinBox(nullptr)
    , defaultRegionWidthSpinBox(nullptr)
    , defaultRegionHeightSpinBox(nullptr)
    , minRegionSizeSpinBox(nullptr)
    , minNormalizedSizeSpinBox(nullptr)
    , settings(new QSettings("OCROrc", "OCR-Orc"))
{
    setWindowTitle("Preferences");
    setMinimumWidth(600);
    setMinimumHeight(500);
    setModal(true);
    
    setupUI();
    loadSettings();
}

void PreferencesDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Create tab widget
    QTabWidget* tabWidget = new QTabWidget(this);
    
    // Create tabs (they will add themselves to tabWidget)
    setupGeneralTab(tabWidget);
    setupCanvasTab(tabWidget);
    setupExportTab(tabWidget);
    setupUITab(tabWidget);
    setupRegionTab(tabWidget);
    
    mainLayout->addWidget(tabWidget);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults,
        this
    );
    
    QPushButton* resetButton = buttonBox->button(QDialogButtonBox::RestoreDefaults);
    resetButton->setText("Reset to Defaults");
    connect(resetButton, &QPushButton::clicked, this, &PreferencesDialog::onResetDefaults);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::onReject);
    
    mainLayout->addWidget(buttonBox);
}

void PreferencesDialog::setupGeneralTab(QTabWidget* tabWidget) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    QGroupBox* generalGroup = new QGroupBox("General Settings", widget);
    QFormLayout* formLayout = new QFormLayout(generalGroup);
    
    defaultRegionColorCombo = new QComboBox(generalGroup);
    defaultRegionColorCombo->addItems({"blue", "red", "green", "yellow", "purple", "orange", "cyan"});
    formLayout->addRow("Default Region Color:", defaultRegionColorCombo);
    
    defaultRegionNamePatternEdit = new QLineEdit(generalGroup);
    defaultRegionNamePatternEdit->setPlaceholderText("Region {n}");
    formLayout->addRow("Default Region Name Pattern:", defaultRegionNamePatternEdit);
    
    autoSaveCheckBox = new QCheckBox("Enable Auto-Save", generalGroup);
    formLayout->addRow(autoSaveCheckBox);
    
    autoSaveIntervalSpinBox = new QSpinBox(generalGroup);
    autoSaveIntervalSpinBox->setRange(1, 60);
    autoSaveIntervalSpinBox->setSuffix(" minutes");
    autoSaveIntervalSpinBox->setEnabled(false);
    connect(autoSaveCheckBox, &QCheckBox::toggled, autoSaveIntervalSpinBox, &QSpinBox::setEnabled);
    formLayout->addRow("Auto-Save Interval:", autoSaveIntervalSpinBox);
    
    layout->addWidget(generalGroup);
    layout->addStretch();
    
    tabWidget->addTab(widget, "General");
}

void PreferencesDialog::setupCanvasTab(QTabWidget* tabWidget) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    QGroupBox* canvasGroup = new QGroupBox("Canvas Settings", widget);
    QFormLayout* formLayout = new QFormLayout(canvasGroup);
    
    defaultZoomSpinBox = new QDoubleSpinBox(canvasGroup);
    defaultZoomSpinBox->setRange(0.1, 10.0);
    defaultZoomSpinBox->setSingleStep(0.1);
    defaultZoomSpinBox->setDecimals(1);
    defaultZoomSpinBox->setSuffix("x");
    formLayout->addRow("Default Zoom Level:", defaultZoomSpinBox);
    
    showGridCheckBox = new QCheckBox("Show Grid", canvasGroup);
    formLayout->addRow(showGridCheckBox);
    
    snapToGridCheckBox = new QCheckBox("Snap to Grid", canvasGroup);
    snapToGridCheckBox->setEnabled(false);
    connect(showGridCheckBox, &QCheckBox::toggled, snapToGridCheckBox, &QCheckBox::setEnabled);
    formLayout->addRow(snapToGridCheckBox);
    
    gridSizeSpinBox = new QSpinBox(canvasGroup);
    gridSizeSpinBox->setRange(5, 100);
    gridSizeSpinBox->setSuffix(" pixels");
    gridSizeSpinBox->setEnabled(false);
    connect(showGridCheckBox, &QCheckBox::toggled, gridSizeSpinBox, &QSpinBox::setEnabled);
    formLayout->addRow("Grid Size:", gridSizeSpinBox);
    
    handleSizeSpinBox = new QDoubleSpinBox(canvasGroup);
    handleSizeSpinBox->setRange(4.0, 20.0);
    handleSizeSpinBox->setSingleStep(1.0);
    handleSizeSpinBox->setDecimals(1);
    handleSizeSpinBox->setSuffix(" pixels");
    formLayout->addRow("Resize Handle Size:", handleSizeSpinBox);
    
    layout->addWidget(canvasGroup);
    layout->addStretch();
    
    tabWidget->addTab(widget, "Canvas");
}

void PreferencesDialog::setupExportTab(QTabWidget* tabWidget) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    QGroupBox* exportGroup = new QGroupBox("Export Settings", widget);
    QFormLayout* formLayout = new QFormLayout(exportGroup);
    
    defaultCoordinateFormatCombo = new QComboBox(exportGroup);
    defaultCoordinateFormatCombo->addItems({"JSON", "CSV"});
    formLayout->addRow("Default Coordinate Format:", defaultCoordinateFormatCombo);
    
    defaultMaskFormatCombo = new QComboBox(exportGroup);
    defaultMaskFormatCombo->addItems({"PNG", "JPEG", "TIFF"});
    formLayout->addRow("Default Mask Format:", defaultMaskFormatCombo);
    
    QHBoxLayout* pathLayout = new QHBoxLayout();
    defaultExportPathEdit = new QLineEdit(exportGroup);
    browseExportPathButton = new QPushButton("Browse...", exportGroup);
    connect(browseExportPathButton, &QPushButton::clicked, this, &PreferencesDialog::onBrowseExportPath);
    pathLayout->addWidget(defaultExportPathEdit);
    pathLayout->addWidget(browseExportPathButton);
    formLayout->addRow("Default Export Path:", pathLayout);
    
    rememberLastExportPathCheckBox = new QCheckBox("Remember Last Export Path", exportGroup);
    formLayout->addRow(rememberLastExportPathCheckBox);
    
    layout->addWidget(exportGroup);
    layout->addStretch();
    
    tabWidget->addTab(widget, "Export");
}

void PreferencesDialog::setupUITab(QTabWidget* tabWidget) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    QGroupBox* uiGroup = new QGroupBox("UI Settings", widget);
    QFormLayout* formLayout = new QFormLayout(uiGroup);
    
    showToolbarCheckBox = new QCheckBox("Show Toolbar", uiGroup);
    formLayout->addRow(showToolbarCheckBox);
    
    showStatusBarCheckBox = new QCheckBox("Show Status Bar", uiGroup);
    formLayout->addRow(showStatusBarCheckBox);
    
    showSidePanelCheckBox = new QCheckBox("Show Side Panel", uiGroup);
    formLayout->addRow(showSidePanelCheckBox);
    
    iconSizeSpinBox = new QSpinBox(uiGroup);
    iconSizeSpinBox->setRange(16, 48);
    iconSizeSpinBox->setSuffix(" pixels");
    formLayout->addRow("Icon Size:", iconSizeSpinBox);
    
    layout->addWidget(uiGroup);
    layout->addStretch();
    
    tabWidget->addTab(widget, "UI");
}

void PreferencesDialog::setupRegionTab(QTabWidget* tabWidget) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    
    QGroupBox* regionGroup = new QGroupBox("Region Settings", widget);
    QFormLayout* formLayout = new QFormLayout(regionGroup);
    
    defaultRegionWidthSpinBox = new QSpinBox(regionGroup);
    defaultRegionWidthSpinBox->setRange(10, 1000);
    defaultRegionWidthSpinBox->setSuffix(" pixels");
    formLayout->addRow("Default Region Width:", defaultRegionWidthSpinBox);
    
    defaultRegionHeightSpinBox = new QSpinBox(regionGroup);
    defaultRegionHeightSpinBox->setRange(10, 1000);
    defaultRegionHeightSpinBox->setSuffix(" pixels");
    formLayout->addRow("Default Region Height:", defaultRegionHeightSpinBox);
    
    minRegionSizeSpinBox = new QSpinBox(regionGroup);
    minRegionSizeSpinBox->setRange(1, 100);
    minRegionSizeSpinBox->setSuffix(" pixels");
    formLayout->addRow("Minimum Region Size:", minRegionSizeSpinBox);
    
    minNormalizedSizeSpinBox = new QDoubleSpinBox(regionGroup);
    minNormalizedSizeSpinBox->setRange(0.0001, 0.1);
    minNormalizedSizeSpinBox->setSingleStep(0.001);
    minNormalizedSizeSpinBox->setDecimals(4);
    formLayout->addRow("Minimum Normalized Size:", minNormalizedSizeSpinBox);
    
    layout->addWidget(regionGroup);
    layout->addStretch();
    
    tabWidget->addTab(widget, "Region");
}

void PreferencesDialog::loadSettings() {
    // General
    QString defaultColor = settings->value("general/defaultRegionColor", "blue").toString();
    int colorIndex = defaultRegionColorCombo->findText(defaultColor);
    if (colorIndex >= 0) {
        defaultRegionColorCombo->setCurrentIndex(colorIndex);
    }
    
    defaultRegionNamePatternEdit->setText(
        settings->value("general/defaultRegionNamePattern", "Region {n}").toString()
    );
    
    autoSaveCheckBox->setChecked(settings->value("general/autoSave", false).toBool());
    autoSaveIntervalSpinBox->setValue(settings->value("general/autoSaveInterval", 5).toInt());
    
    // Canvas
    defaultZoomSpinBox->setValue(settings->value("canvas/defaultZoom", 1.0).toDouble());
    showGridCheckBox->setChecked(settings->value("canvas/showGrid", false).toBool());
    snapToGridCheckBox->setChecked(settings->value("canvas/snapToGrid", false).toBool());
    gridSizeSpinBox->setValue(settings->value("canvas/gridSize", 20).toInt());
    handleSizeSpinBox->setValue(settings->value("canvas/handleSize", 8.0).toDouble());
    
    // Export
    QString coordFormat = settings->value("export/defaultCoordinateFormat", "JSON").toString();
    int coordIndex = defaultCoordinateFormatCombo->findText(coordFormat);
    if (coordIndex >= 0) {
        defaultCoordinateFormatCombo->setCurrentIndex(coordIndex);
    }
    
    QString maskFormat = settings->value("export/defaultMaskFormat", "PNG").toString();
    int maskIndex = defaultMaskFormatCombo->findText(maskFormat);
    if (maskIndex >= 0) {
        defaultMaskFormatCombo->setCurrentIndex(maskIndex);
    }
    
    QString defaultPath = settings->value("export/defaultExportPath", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    defaultExportPathEdit->setText(defaultPath);
    
    rememberLastExportPathCheckBox->setChecked(
        settings->value("export/rememberLastExportPath", true).toBool()
    );
    
    // UI
    showToolbarCheckBox->setChecked(settings->value("ui/showToolbar", true).toBool());
    showStatusBarCheckBox->setChecked(settings->value("ui/showStatusBar", true).toBool());
    showSidePanelCheckBox->setChecked(settings->value("ui/showSidePanel", true).toBool());
    iconSizeSpinBox->setValue(settings->value("ui/iconSize", 24).toInt());
    
    // Region
    defaultRegionWidthSpinBox->setValue(settings->value("region/defaultWidth", 100).toInt());
    defaultRegionHeightSpinBox->setValue(settings->value("region/defaultHeight", 100).toInt());
    minRegionSizeSpinBox->setValue(settings->value("region/minSize", 10).toInt());
    minNormalizedSizeSpinBox->setValue(settings->value("region/minNormalizedSize", 0.001).toDouble());
}

void PreferencesDialog::saveSettings() {
    // General
    settings->setValue("general/defaultRegionColor", defaultRegionColorCombo->currentText());
    settings->setValue("general/defaultRegionNamePattern", defaultRegionNamePatternEdit->text());
    settings->setValue("general/autoSave", autoSaveCheckBox->isChecked());
    settings->setValue("general/autoSaveInterval", autoSaveIntervalSpinBox->value());
    
    // Canvas
    settings->setValue("canvas/defaultZoom", defaultZoomSpinBox->value());
    settings->setValue("canvas/showGrid", showGridCheckBox->isChecked());
    settings->setValue("canvas/snapToGrid", snapToGridCheckBox->isChecked());
    settings->setValue("canvas/gridSize", gridSizeSpinBox->value());
    settings->setValue("canvas/handleSize", handleSizeSpinBox->value());
    
    // Export
    settings->setValue("export/defaultCoordinateFormat", defaultCoordinateFormatCombo->currentText());
    settings->setValue("export/defaultMaskFormat", defaultMaskFormatCombo->currentText());
    settings->setValue("export/defaultExportPath", defaultExportPathEdit->text());
    settings->setValue("export/rememberLastExportPath", rememberLastExportPathCheckBox->isChecked());
    
    // UI
    settings->setValue("ui/showToolbar", showToolbarCheckBox->isChecked());
    settings->setValue("ui/showStatusBar", showStatusBarCheckBox->isChecked());
    settings->setValue("ui/showSidePanel", showSidePanelCheckBox->isChecked());
    settings->setValue("ui/iconSize", iconSizeSpinBox->value());
    
    // Region
    settings->setValue("region/defaultWidth", defaultRegionWidthSpinBox->value());
    settings->setValue("region/defaultHeight", defaultRegionHeightSpinBox->value());
    settings->setValue("region/minSize", minRegionSizeSpinBox->value());
    settings->setValue("region/minNormalizedSize", minNormalizedSizeSpinBox->value());
    
    settings->sync();
}

void PreferencesDialog::onAccept() {
    saveSettings();
    accept();
}

void PreferencesDialog::onReject() {
    reject();
}

void PreferencesDialog::onBrowseExportPath() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Default Export Directory",
        defaultExportPathEdit->text()
    );
    
    if (!dir.isEmpty()) {
        defaultExportPathEdit->setText(dir);
    }
}

void PreferencesDialog::onResetDefaults() {
    int ret = QMessageBox::question(
        this,
        "Reset to Defaults",
        "Are you sure you want to reset all settings to their default values?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (ret == QMessageBox::Yes) {
        // Reset to defaults
        defaultRegionColorCombo->setCurrentIndex(0); // blue
        defaultRegionNamePatternEdit->setText("Region {n}");
        autoSaveCheckBox->setChecked(false);
        autoSaveIntervalSpinBox->setValue(5);
        
        defaultZoomSpinBox->setValue(1.0);
        showGridCheckBox->setChecked(false);
        snapToGridCheckBox->setChecked(false);
        gridSizeSpinBox->setValue(20);
        handleSizeSpinBox->setValue(8.0);
        
        defaultCoordinateFormatCombo->setCurrentIndex(0); // JSON
        defaultMaskFormatCombo->setCurrentIndex(0); // PNG
        defaultExportPathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        rememberLastExportPathCheckBox->setChecked(true);
        
        showToolbarCheckBox->setChecked(true);
        showStatusBarCheckBox->setChecked(true);
        showSidePanelCheckBox->setChecked(true);
        iconSizeSpinBox->setValue(24);
        
        defaultRegionWidthSpinBox->setValue(100);
        defaultRegionHeightSpinBox->setValue(100);
        minRegionSizeSpinBox->setValue(10);
        minNormalizedSizeSpinBox->setValue(0.001);
    }
}

} // namespace ocr_orc

