#ifndef SIDE_PANEL_WIDGET_H
#define SIDE_PANEL_WIDGET_H

#include <QtWidgets/QFrame>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief Side panel widget containing Regions tab, Groups tab, Info display, and Help button
 */
class SidePanelWidget : public QFrame {
    Q_OBJECT

public:
    explicit SidePanelWidget(QWidget *parent = nullptr);
    ~SidePanelWidget() = default;

    // Getters for UI components
    QListWidget* getRegionListBox() const { return regionListBox; }
    QListWidget* getGroupListBox() const { return groupListBox; }
    QTextEdit* getInfoText() const { return infoText; }
    QPushButton* getHelpButton() const { return helpButton; }
    QPushButton* getCreateGroupButton() const { return createGroupButton; }
    QPushButton* getAddToGroupButton() const { return addToGroupButton; }
    QPushButton* getRemoveFromGroupButton() const { return removeFromGroupButton; }
    QPushButton* getDeleteGroupButton() const { return deleteGroupButton; }

    /**
     * @brief Update region listbox with current regions
     * @param regionNames List of all region names
     * @param selectedRegions Set of currently selected region names
     * @param regionGroups Map of region names to their group names (empty string if no group)
     */
    void updateRegionListBox(const QList<QString>& regionNames, const QSet<QString>& selectedRegions, const QMap<QString, QString>& regionGroups);

    /**
     * @brief Update group listbox with current groups
     * @param groupNames List of all group names
     * @param groupSizes Map of group names to region counts
     */
    void updateGroupListBox(const QList<QString>& groupNames, const QMap<QString, int>& groupSizes);
    
    /**
     * @brief Refresh all icons based on current theme
     * Call this when theme changes to update icon colors
     */
    void refreshIcons();

    /**
     * @brief Set info text content
     * @param text Info text to display
     */
    void setInfoText(const QString& text);
    
    /**
     * @brief Show region editor for a selected region
     * @param regionName Name of the region to edit
     * @param regionColor Current color of the region
     * @param regionGroup Current group of the region (empty if none)
     * @param availableGroups List of available group names
     * @param regionType Current region type
     * @param percentageFill Current percentage fill option
     * @param x1 Normalized x1 coordinate
     * @param y1 Normalized y1 coordinate
     * @param x2 Normalized x2 coordinate
     * @param y2 Normalized y2 coordinate
     */
    void showRegionEditor(const QString& regionName, const QString& regionColor, 
                         const QString& regionGroup, const QList<QString>& availableGroups,
                         const QString& regionType = "none", const QString& percentageFill = "none",
                         double x1 = 0.0, double y1 = 0.0, double x2 = 1.0, double y2 = 1.0);
    
    /**
     * @brief Hide region editor (when no region is selected)
     */
    void hideRegionEditor();
    
    /**
     * @brief Toggle region editor visibility
     * Shows editor if hidden, hides if visible
     */
    void toggleRegionEditor();
    
    /**
     * @brief Check if region editor is currently visible
     * @return true if editor is visible, false otherwise
     */
    bool isRegionEditorVisible() const;
    
    /**
     * @brief Get the currently editing region name
     * @return Name of region being edited, or empty string if none
     */
    QString getCurrentEditingRegion() const { return currentEditingRegion; }

signals:
    /**
     * @brief Emitted when region listbox selection changes
     * @param selectedRegions Set of selected region names
     */
    void regionSelectionChanged(const QSet<QString>& selectedRegions);
    
    /**
     * @brief Emitted when group listbox selection changes
     * @param groupName Selected group name
     */
    void groupSelectionChanged(const QString& groupName);
    
    /**
     * @brief Emitted when Create Group button is clicked
     */
    void createGroupClicked();
    
    /**
     * @brief Emitted when Add to Group button is clicked
     */
    void addToGroupClicked();
    
    /**
     * @brief Emitted when Remove from Group button is clicked
     */
    void removeFromGroupClicked();
    
    /**
     * @brief Emitted when Delete Group button is clicked
     */
    void deleteGroupClicked();
    
    /**
     * @brief Emitted when Help button is clicked
     */
    void helpClicked();
    
    /**
     * @brief Emitted when region name is edited
     * @param oldName Original region name
     * @param newName New region name
     */
    void regionNameChanged(const QString& oldName, const QString& newName);
    
    /**
     * @brief Emitted when region color is changed
     * @param regionName Name of the region
     * @param newColor New color value
     */
    void regionColorChanged(const QString& regionName, const QString& newColor);
    
    /**
     * @brief Emitted when region group is changed
     * @param regionName Name of the region
     * @param newGroup New group name (empty string to remove from group)
     */
    void regionGroupChanged(const QString& regionName, const QString& newGroup);
    
    /**
     * @brief Emitted when region coordinates are changed
     * @param regionName Name of the region
     * @param x1 New x1 coordinate (normalized)
     * @param y1 New y1 coordinate (normalized)
     * @param x2 New x2 coordinate (normalized)
     * @param y2 New y2 coordinate (normalized)
     */
    void regionCoordinatesChanged(const QString& regionName, double x1, double y1, double x2, double y2);
    
    /**
     * @brief Emitted when region type is changed
     * @param regionName Name of the region
     * @param newType New region type
     */
    void regionTypeChanged(const QString& regionName, const QString& newType);
    
    /**
     * @brief Emitted when percentage fill option is changed
     * @param regionName Name of the region
     * @param newPercentageFill New percentage fill option
     */
    void regionPercentageFillChanged(const QString& regionName, const QString& newPercentageFill);

private:
    void setupUI();
    void setupRegionsTab();
    void setupGroupsTab();
    void setupRegionEditor();

    QTabWidget* sidePanelTabs;
    QWidget* regionsTab;
    QWidget* groupsTab;
    QListWidget* regionListBox;
    QListWidget* groupListBox;
    QTextEdit* infoText;
    QPushButton* helpButton;
    QPushButton* createGroupButton;
    QPushButton* addToGroupButton;
    QPushButton* removeFromGroupButton;
    QPushButton* deleteGroupButton;
    
    // Region editor widgets
    QFrame* regionEditorFrame;
    QLabel* regionEditorLabel;
    QLineEdit* regionNameEdit;
    QComboBox* regionColorCombo;
    QComboBox* regionGroupCombo;
    QComboBox* regionTypeCombo;
    QComboBox* regionPercentageFillCombo;
    QPushButton* regionX1DecreaseBtn;
    QPushButton* regionX1IncreaseBtn;
    QLabel* regionX1ValueLabel;
    QPushButton* regionY1DecreaseBtn;
    QPushButton* regionY1IncreaseBtn;
    QLabel* regionY1ValueLabel;
    QPushButton* regionX2DecreaseBtn;
    QPushButton* regionX2IncreaseBtn;
    QLabel* regionX2ValueLabel;
    QPushButton* regionY2DecreaseBtn;
    QPushButton* regionY2IncreaseBtn;
    QLabel* regionY2ValueLabel;
    QString currentEditingRegion;  // Track which region is being edited
    
    // Helper method to update coordinate value
    void updateCoordinateValue(const QString& coordName, double delta);
};

} // namespace ocr_orc

#endif // SIDE_PANEL_WIDGET_H

