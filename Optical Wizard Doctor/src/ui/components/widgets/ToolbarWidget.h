#ifndef TOOLBAR_WIDGET_H
#define TOOLBAR_WIDGET_H

#include <QtWidgets/QFrame>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include "ModeToggleWidget.h"

namespace ocr_orc {

class DocumentState;

/**
 * @brief Toolbar widget containing Mode, Zoom, Groups, and Undo/Redo controls
 */
class ToolbarWidget : public QFrame {
    Q_OBJECT

public:
    explicit ToolbarWidget(QWidget *parent = nullptr);
    ~ToolbarWidget() = default;

    // Getters for UI components (for connections in MainWindow)
    ModeToggleWidget* getModeToggleWidget() const { return modeToggleWidget; }
    // Legacy getters for compatibility (deprecated - use getModeToggleWidget)
    QButtonGroup* getModeButtonGroup() const { return nullptr; }
    QRadioButton* getCreateModeButton() const { return nullptr; }
    QRadioButton* getSelectModeButton() const { return nullptr; }
    QPushButton* getZoomOutButton() const { return zoomOutButton; }
    QPushButton* getZoomInButton() const { return zoomInButton; }
    QPushButton* getZoomResetButton() const { return zoomResetButton; }
    QPushButton* getRotateButton() const { return rotateButton; }
    QLabel* getZoomLabel() const { return zoomLabel; }
    QPushButton* getGroupButton() const { return groupButton; }
    QPushButton* getUngroupButton() const { return ungroupButton; }
    QPushButton* getUndoButton() const { return undoButton; }
    QPushButton* getRedoButton() const { return redoButton; }
    QLabel* getLogoLabel() const { return logoLabel; }

    /**
     * @brief Update zoom label with current zoom percentage
     * @param zoomLevel Current zoom level (1.0 = 100%)
     */
    void updateZoomLabel(double zoomLevel);

    /**
     * @brief Update undo/redo button states
     * @param canUndo Whether undo is available
     * @param canRedo Whether redo is available
     */
    void updateUndoRedoButtons(bool canUndo, bool canRedo);
    
    /**
     * @brief Update rotate button state to match rotate mode
     * @param enabled Whether rotate mode is enabled
     */
    void updateRotateButton(bool enabled);
    
    /**
     * @brief Refresh all icons based on current theme
     * Call this when theme changes to update icon colors
     */
    void refreshIcons();

signals:
    /**
     * @brief Emitted when create mode is selected
     */
    void createModeSelected();
    
    /**
     * @brief Emitted when select mode is selected
     */
    void selectModeSelected();
    
    /**
     * @brief Emitted when zoom in is clicked
     */
    void zoomInClicked();
    
    /**
     * @brief Emitted when zoom out is clicked
     */
    void zoomOutClicked();
    
    /**
     * @brief Emitted when zoom reset is clicked
     */
    void zoomResetClicked();
    
    /**
     * @brief Emitted when rotate button is clicked (toggled)
     */
    void rotateClicked(bool checked);
    
    /**
     * @brief Emitted when group button is clicked
     */
    void groupClicked();
    
    /**
     * @brief Emitted when ungroup button is clicked
     */
    void ungroupClicked();
    
    /**
     * @brief Emitted when undo button is clicked
     */
    void undoClicked();
    
    /**
     * @brief Emitted when redo button is clicked
     */
    void redoClicked();

private:
    void setupUI();
    void setupLogo(QHBoxLayout* layout);
    void setupMode(QHBoxLayout* layout);
    void setupZoom(QHBoxLayout* layout);
    void setupGroups(QHBoxLayout* layout);
    void setupUndoRedo(QHBoxLayout* layout);

    ModeToggleWidget* modeToggleWidget;
    // Legacy members (kept for compatibility but not used)
    QButtonGroup* modeButtonGroup;
    QRadioButton* createModeButton;
    QRadioButton* selectModeButton;
    QPushButton* zoomOutButton;
    QLabel* zoomLabel;
    QPushButton* zoomInButton;
    QPushButton* zoomResetButton;
    QPushButton* rotateButton;
    QPushButton* groupButton;
    QPushButton* ungroupButton;
    QPushButton* undoButton;
    QPushButton* redoButton;
    QLabel* logoLabel;
};

} // namespace ocr_orc

#endif // TOOLBAR_WIDGET_H

