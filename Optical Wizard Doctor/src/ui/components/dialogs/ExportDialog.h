#ifndef EXPORT_DIALOG_H
#define EXPORT_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFileDialog>
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Export dialog for configuring and executing exports
 * 
 * Provides options for:
 * - Export type (Coordinates, Mask, or both)
 * - Format selection (JSON, CSV, PNG, JPEG, TIFF, etc.)
 * - Destination selection
 * - Export options (selected regions only, etc.)
 */
class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog() = default;

    /**
     * @brief Get selected export types
     * @return Bit flags: 1 = Coordinates, 2 = Mask
     */
    int getExportTypes() const;

    /**
     * @brief Get selected coordinate format
     * @return Format name: "json", "csv", etc.
     */
    QString getCoordinateFormat() const;

    /**
     * @brief Get selected mask format
     * @return Format name: "png", "jpg", "tiff", etc.
     */
    QString getMaskFormat() const;

    /**
     * @brief Get destination directory
     * @return Directory path
     */
    QString getDestinationDirectory() const;

    /**
     * @brief Get base file name (without extension)
     * @return Base file name
     */
    QString getBaseFileName() const;

    /**
     * @brief Check if only selected regions should be exported
     * @return true if export selected only
     */
    bool exportSelectedOnly() const;

    /**
     * @brief Check if coordinates should be exported with mask
     * @return true if export coordinates with mask
     */
    bool exportCoordinatesWithMask() const;

public slots:
    void onBrowseDestination();
    void onExportTypeChanged();
    void accept() override;

private:
    void setupUI();
    void setupExportTypeSection();
    void setupFormatSection();
    void setupDestinationSection();
    void setupOptionsSection();
    void updateUI();

    // Export type selection
    QGroupBox* exportTypeGroup;
    QCheckBox* exportCoordinatesCheck;
    QCheckBox* exportMaskCheck;

    // Coordinate format
    QGroupBox* coordinateFormatGroup;
    QComboBox* coordinateFormatCombo;

    // Mask format
    QGroupBox* maskFormatGroup;
    QComboBox* maskFormatCombo;

    // Destination
    QGroupBox* destinationGroup;
    QLineEdit* destinationEdit;
    QPushButton* browseButton;
    QLineEdit* baseFileNameEdit;

    // Options
    QGroupBox* optionsGroup;
    QCheckBox* selectedRegionsOnlyCheck;
    QCheckBox* exportCoordinatesWithMaskCheck;
};

} // namespace ocr_orc

#endif // EXPORT_DIALOG_H

