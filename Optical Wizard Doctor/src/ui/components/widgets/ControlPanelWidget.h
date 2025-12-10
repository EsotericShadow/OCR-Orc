#ifndef CONTROL_PANEL_WIDGET_H
#define CONTROL_PANEL_WIDGET_H

#include <QtWidgets/QFrame>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>

namespace ocr_orc {

/**
 * @brief Control panel widget containing File controls, Region input, and Action buttons
 */
class ControlPanelWidget : public QFrame {
    Q_OBJECT

public:
    explicit ControlPanelWidget(QWidget *parent = nullptr);
    ~ControlPanelWidget() = default;

    // Getters for UI components
    QPushButton* getLoadPdfButton() const { return loadPdfButton; }
    QLabel* getFileLabel() const { return fileLabel; }
    QPushButton* getExportCoordinatesButton() const { return exportCoordinatesButton; }
    QPushButton* getExportMaskButton() const { return nullptr; }  // Deprecated - consolidated into export button
    QPushButton* getLoadCoordinatesButton() const { return loadCoordinatesButton; }
    QPushButton* getTestExtractionButton() const { return testExtractionButton; }
    QLineEdit* getRegionNameEdit() const { return regionNameEdit; }
    QLineEdit* getGroupEdit() const { return groupEdit; }
    QComboBox* getColorComboBox() const { return colorComboBox; }
    QCheckBox* getMaskOutsideCheckBox() const { return maskOutsideCheckBox; }
    QPushButton* getClearSelectionButton() const { return clearSelectionButton; }
    QPushButton* getEditSelectedButton() const { return editSelectedButton; }

    /**
     * @brief Get region name from input field
     * @return Region name, or empty string if auto-generate
     */
    QString getRegionName() const;

    /**
     * @brief Get group name from input field
     * @return Group name, or empty string if none
     */
    QString getGroupName() const;

    /**
     * @brief Get selected color from combo box
     * @return Color name
     */
    QString getSelectedColor() const;

    /**
     * @brief Clear region name input field
     */
    void clearRegionName();

    /**
     * @brief Clear group name input field
     */
    void clearGroupName();

    /**
     * @brief Set file label text
     * @param text File name or status text
     */
    void setFileLabel(const QString& text);

signals:
    /**
     * @brief Emitted when Load PDF button is clicked
     */
    void loadPdfClicked();
    
    /**
     * @brief Emitted when Export button is clicked
     * Opens export dialog with options for coordinates, masks, and other formats
     */
    void exportCoordinatesClicked();
    
    /**
     * @brief Emitted when Export Mask button is clicked (deprecated - use exportCoordinatesClicked)
     */
    void exportMaskClicked();
    
    /**
     * @brief Emitted when Load Coordinates button is clicked
     */
    void loadCoordinatesClicked();
    
    /**
     * @brief Emitted when Delete Selected button is clicked
     */
    void clearSelectionClicked();
    
    /**
     * @brief Emitted when Edit Selected button is clicked
     */
    void editSelectedClicked();

private:
    void setupUI();
    void setupFile(QHBoxLayout* layout);
    void setupRegionInput(QHBoxLayout* layout);
    void setupActions(QHBoxLayout* layout);

    QPushButton* loadPdfButton;
    QLabel* fileLabel;
    QPushButton* exportCoordinatesButton;
    QPushButton* exportMaskButton;
    QPushButton* loadCoordinatesButton;
    QPushButton* testExtractionButton;
    QLabel* regionNameLabel;
    QLineEdit* regionNameEdit;
    QLabel* groupLabel;
    QLineEdit* groupEdit;
    QLabel* colorLabel;
    QComboBox* colorComboBox;
    QCheckBox* maskOutsideCheckBox;
    QPushButton* clearSelectionButton;
    QPushButton* editSelectedButton;
};

} // namespace ocr_orc

#endif // CONTROL_PANEL_WIDGET_H

