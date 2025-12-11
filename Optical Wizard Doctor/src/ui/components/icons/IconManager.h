#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <QtGui/QIcon>
#include <QtGui/QColor>
#include <QtWidgets/QStyle>
#include <QtCore/QString>
#include <QtCore/QMap>

namespace ocr_orc {

/**
 * @brief Icon Manager
 * 
 * Provides a centralized system for managing application icons.
 * Uses Qt's standard icons where possible, with fallbacks for custom icons.
 * 
 * Icons are theme-aware and adapt to light/dark modes.
 */
class IconManager {
public:
    /**
     * @brief Get singleton instance
     */
    static IconManager& instance();
    
    /**
     * @brief Get icon for a specific action
     * @param iconName Name of the icon (e.g., "open", "save", "undo")
     * @return QIcon for the action
     */
    QIcon getIcon(const QString& iconName) const;
    
    /**
     * @brief Get standard Qt icon
     * @param standardIcon Qt standard icon type
     * @return QIcon
     */
    QIcon getStandardIcon(QStyle::StandardPixmap standardIcon) const;
    
    /**
     * @brief Set icon size for buttons
     * @param size Icon size in pixels
     */
    void setIconSize(int size) { iconSize = size; }
    
    /**
     * @brief Get current icon size
     */
    int getIconSize() const { return iconSize; }
    
    /**
     * @brief Load SVG icon with specific color
     * @param svgPath Path to SVG file
     * @param color Color to use for the icon (replaces #000000 in SVG)
     * @return QIcon from SVG with specified color, or empty icon if file not found
     */
    QIcon loadSvgIconWithColor(const QString& svgPath, const QColor& color) const;
    
    /**
     * @brief Clear icon cache - call when theme changes to force icon regeneration
     */
    void clearIconCache();

private:
    IconManager();
    ~IconManager() = default;
    IconManager(const IconManager&) = delete;
    IconManager& operator=(const IconManager&) = delete;
    
    // Icon name constants
    static constexpr const char* ICON_OPEN = "open";
    static constexpr const char* ICON_SAVE = "save";
    static constexpr const char* ICON_EXPORT = "export";
    static constexpr const char* ICON_IMPORT = "import";
    static constexpr const char* ICON_UNDO = "undo";
    static constexpr const char* ICON_REDO = "redo";
    static constexpr const char* ICON_ZOOM_IN = "zoom_in";
    static constexpr const char* ICON_ZOOM_OUT = "zoom_out";
    static constexpr const char* ICON_ZOOM_RESET = "zoom_reset";
    static constexpr const char* ICON_CREATE = "create";
    static constexpr const char* ICON_SELECT = "select";
    static constexpr const char* ICON_GROUP = "group";
    static constexpr const char* ICON_UNGROUP = "ungroup";
    static constexpr const char* ICON_DELETE = "delete";
    static constexpr const char* ICON_EDIT = "edit";
    static constexpr const char* ICON_CLEAR = "clear";
    static constexpr const char* ICON_HELP = "help";
    static constexpr const char* ICON_MASK = "mask";
    static constexpr const char* ICON_COLOR = "color";
    static constexpr const char* ICON_ADD = "add";
    static constexpr const char* ICON_REMOVE = "remove";
    
    int iconSize;
    mutable QMap<QString, QIcon> iconCache; // Cache icons by path+color key (mutable for const methods)
    
    /**
     * @brief Create icon from standard pixmap with fallback
     */
    QIcon createStandardIcon(QStyle::StandardPixmap standardIcon) const;
    
    /**
     * @brief Create text-based icon as fallback
     */
    QIcon createTextIcon(const QString& text, const QString& color = "#000000") const;
    
    /**
     * @brief Load SVG icon from resources
     * @param svgPath Path to SVG file (relative to resources directory or absolute)
     * @return QIcon from SVG, or empty icon if file not found
     */
    QIcon loadSvgIcon(const QString& svgPath) const;
};

} // namespace ocr_orc

#endif // ICON_MANAGER_H

