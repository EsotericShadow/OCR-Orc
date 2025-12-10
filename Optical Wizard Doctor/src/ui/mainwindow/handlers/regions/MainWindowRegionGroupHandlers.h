#ifndef MAIN_WINDOW_REGION_GROUP_HANDLERS_H
#define MAIN_WINDOW_REGION_GROUP_HANDLERS_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtWidgets/QListWidgetItem>
#include <functional>

namespace ocr_orc {

// Forward declarations
class MainWindow;
class DocumentState;
class Canvas;
class ControlPanelWidget;
class SidePanelWidget;

/**
 * @brief Handles region and group operation handlers for MainWindow
 * 
 * Manages:
 * - Group operations (create, add, remove, delete)
 * - Region operations (edit, delete, creation, duplication)
 * - Region name management
 */
class MainWindowRegionGroupHandlers {
public:
    MainWindowRegionGroupHandlers();
    ~MainWindowRegionGroupHandlers();
    
    // Group operations
    void onCreateGroup(MainWindow* mainWindow);
    
    QSet<QString> getSelectedRegions(MainWindow* mainWindow);
    
    void onAddToGroup(MainWindow* mainWindow);
    void onRemoveFromGroup(MainWindow* mainWindow);
    void onDeleteGroup(MainWindow* mainWindow);
    
    // Region operations
    QString showEditNameDialog(MainWindow* mainWindow, const QString& currentName);
    void onEditSelected(MainWindow* mainWindow);
    QString showChangeColorDialog(MainWindow* mainWindow, const QString& regionName);
    void onRegionCreationRequested(MainWindow* mainWindow);
    void onRegionCreated(MainWindow* mainWindow, const QString& regionName);
    void onRegionsDuplicated(MainWindow* mainWindow, const QList<QString>& duplicatedNames);
    QString generateRegionName(MainWindow* mainWindow);
    void autoIncrementRegionName(MainWindow* mainWindow);
    void onDeleteSelected(MainWindow* mainWindow);
    
    // Region editor handlers
    void onRegionNameChanged(MainWindow* mainWindow, const QString& oldName, const QString& newName);
    void onRegionColorChanged(MainWindow* mainWindow, const QString& regionName, const QString& newColor);
    void onRegionGroupChanged(MainWindow* mainWindow, const QString& regionName, const QString& newGroup);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_REGION_GROUP_HANDLERS_H

