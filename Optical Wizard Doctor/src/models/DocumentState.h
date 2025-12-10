#ifndef DOCUMENT_STATE_H
#define DOCUMENT_STATE_H

#include "RegionData.h"
#include "GroupData.h"
#include "StateSnapshot.h"
#include "../core/CoordinateSystem.h"
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtGui/QImage>
#include <QtCore/QJsonObject>

namespace ocr_orc {

/**
 * @brief Document state management
 * 
 * Stores all application state including:
 * - PDF document information
 * - All regions and their coordinates
 * - All groups
 * - Display state (zoom, pan, scale)
 * 
 * This is the central data store for the application.
 */
class DocumentState {
public:
    // Document information
    QString pdfPath;
    QImage image;  // First page as image
    
    // Region and group storage
    QMap<QString, RegionData> regions;  // Key: region name
    QMap<QString, GroupData> groups;    // Key: group name
    
    // Display state
    double zoomLevel;      // Current zoom (1.0 = 100%)
    double scaleFactor;    // Display scale factor
    QPointF imageOffset;   // Image position on canvas
    
    // Constructors
    DocumentState();
    
    // Region management
    void addRegion(const QString& name, const RegionData& region);
    void removeRegion(const QString& name);
    bool hasRegion(const QString& name) const;
    RegionData getRegion(const QString& name) const;
    QList<QString> getAllRegionNames() const;
    QList<QString> duplicateRegions(const QList<QString>& regionNames);
    
    /**
     * @brief Rename a region atomically
     * Updates all references: groups, selections, region map
     * @param oldName Current region name
     * @param newName New region name
     * @return true if rename successful, false if validation failed
     */
    bool renameRegion(const QString& oldName, const QString& newName);
    
    /**
     * @brief Validate a region name
     * @param name Name to validate
     * @param excludeName Optional name to exclude from uniqueness check (for rename)
     * @return true if name is valid (non-empty, unique)
     */
    bool isValidRegionName(const QString& name, const QString& excludeName = QString()) const;
    
    /**
     * @brief Change a region's color
     * @param regionName Name of region to change
     * @param color New color name ("blue", "red", "green", etc.)
     * @return true if color change successful
     */
    bool changeRegionColor(const QString& regionName, const QString& color);
    
    // Group management
    void createGroup(const QString& name);
    void deleteGroup(const QString& name);
    void addRegionToGroup(const QString& regionName, const QString& groupName);
    void removeRegionFromGroup(const QString& regionName);
    QList<QString> getGroupNames() const;
    GroupData getGroup(const QString& name) const;
    bool hasGroup(const QString& name) const;
    
    // Coordinate synchronization
    /**
     * @brief Synchronize all region coordinates
     * Recalculates image and canvas coordinates from normalized (source of truth)
     * Call this whenever zoom/pan changes or image loads
     */
    void synchronizeCoordinates();
    
    /**
     * @brief Get image dimensions
     * @return QSize of image, or (0,0) if no image
     */
    QSize getImageSize() const;
    
    // State validation
    bool isValid() const;
    
    // Serialization
    QJsonObject toJson() const;
    static DocumentState fromJson(const QJsonObject& json);
    
    // State management
    void clear();
    void setImage(const QImage& img);
    void setZoomLevel(double zoom);
    void setImageOffset(const QPointF& offset);
    
    // Calculate scale factor from current state
    void updateScaleFactor(int canvasWidth, int canvasHeight);
    
    // Undo/Redo state management
    /**
     * @brief Save current state to undo stack
     * Call this BEFORE any state-changing operation
     */
    void saveState();
    
    /**
     * @brief Restore state from snapshot
     * @param snapshot State snapshot to restore
     */
    void restoreState(const StateSnapshot& snapshot);
    
    /**
     * @brief Undo last operation
     * Restores previous state from undo stack
     */
    void undoAction();
    
    /**
     * @brief Redo last undone operation
     * Restores next state from redo stack
     */
    void redoAction();
    
    /**
     * @brief Check if undo is available
     * @return true if undo stack has states
     */
    bool canUndo() const { return !undoStack.isEmpty(); }
    
    /**
     * @brief Check if redo is available
     * @return true if redo stack has states
     */
    bool canRedo() const { return !redoStack.isEmpty(); }
    
    /**
     * @brief Get number of undo levels available
     * @return Number of states in undo stack
     */
    int undoCount() const { return undoStack.size(); }
    
    /**
     * @brief Get number of redo levels available
     * @return Number of states in redo stack
     */
    int redoCount() const { return redoStack.size(); }
    
    /**
     * @brief Clear undo/redo stacks
     * Call this when loading new document or clearing state
     */
    void clearUndoRedoStacks();
    
    /**
     * @brief Create snapshot of current state
     * @return StateSnapshot with current state
     */
    StateSnapshot createCurrentSnapshot() const;

private:
    // Undo/Redo stacks
    QList<StateSnapshot> undoStack;  // Stack of previous states
    QList<StateSnapshot> redoStack;  // Stack of undone states
    
    static constexpr int MAX_UNDO_LEVELS = 50;  // Maximum undo levels
};

} // namespace ocr_orc

#endif // DOCUMENT_STATE_H

