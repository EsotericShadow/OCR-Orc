#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFileDialog>
#include <QtCore/QString>
#include <QtCore/QSettings>

namespace ocr_orc {

/**
 * @brief Preferences dialog for application settings
 * 
 * Provides comprehensive settings for:
 * - General: Default region color, name pattern, auto-save
 * - Canvas: Zoom, grid, handle size
 * - Export: Default format, export path
 * - UI: Visibility, icon size
 * - Region: Default size, minimum size
 */
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    virtual ~PreferencesDialog() = default;

    /**
     * @brief Load settings from QSettings
     */
    void loadSettings();

    /**
     * @brief Save settings to QSettings
     */
    void saveSettings();

private slots:
    /**
     * @brief Handle OK button click
     */
    void onAccept();

    /**
     * @brief Handle Cancel button click
     */
    void onReject();

    /**
     * @brief Handle Browse button for export path
     */
    void onBrowseExportPath();

    /**
     * @brief Handle Reset to Defaults button
     */
    void onResetDefaults();

private:
    void setupUI();
    void setupGeneralTab(QTabWidget* tabWidget);
    void setupCanvasTab(QTabWidget* tabWidget);
    void setupExportTab(QTabWidget* tabWidget);
    void setupUITab(QTabWidget* tabWidget);
    void setupRegionTab(QTabWidget* tabWidget);

    // General tab
    QComboBox* defaultRegionColorCombo;
    QLineEdit* defaultRegionNamePatternEdit;
    QCheckBox* autoSaveCheckBox;
    QSpinBox* autoSaveIntervalSpinBox;

    // Canvas tab
    QDoubleSpinBox* defaultZoomSpinBox;
    QCheckBox* showGridCheckBox;
    QCheckBox* snapToGridCheckBox;
    QSpinBox* gridSizeSpinBox;
    QDoubleSpinBox* handleSizeSpinBox;

    // Export tab
    QComboBox* defaultCoordinateFormatCombo;
    QComboBox* defaultMaskFormatCombo;
    QLineEdit* defaultExportPathEdit;
    QPushButton* browseExportPathButton;
    QCheckBox* rememberLastExportPathCheckBox;

    // UI tab
    QCheckBox* showToolbarCheckBox;
    QCheckBox* showStatusBarCheckBox;
    QCheckBox* showSidePanelCheckBox;
    QSpinBox* iconSizeSpinBox;

    // Region tab
    QSpinBox* defaultRegionWidthSpinBox;
    QSpinBox* defaultRegionHeightSpinBox;
    QSpinBox* minRegionSizeSpinBox;
    QDoubleSpinBox* minNormalizedSizeSpinBox;

    QSettings* settings;
};

} // namespace ocr_orc

#endif // PREFERENCES_DIALOG_H

