#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QtWidgets/QWidget>
#include <QtGui/QColor>
#include <QtCore/QString>
#include <QtCore/QSettings>

namespace ocr_orc {

/**
 * @brief Theme enumeration
 */
enum class Theme {
    Light,   // Light mode
    Dark,    // Dark mode
    System   // Follow system preference
};

/**
 * @brief Theme color structure
 * 
 * Defines all colors used in a theme
 */
struct ThemeColors {
    // Background colors
    QColor background;        // Main window background
    QColor surface;           // Panel/surface background
    QColor canvasBackground;  // Canvas background
    
    // Text colors
    QColor text;              // Primary text
    QColor textSecondary;     // Secondary text
    QColor textDisabled;      // Disabled text
    
    // Accent colors
    QColor primary;           // Primary accent color
    QColor primaryHover;      // Primary hover state
    QColor selection;         // Selection highlight
    QColor selectionHover;    // Selection hover
    
    // Border colors
    QColor border;            // Border color
    QColor borderLight;       // Light border
    
    // Toolbar colors
    QColor toolbarBackground; // Toolbar background
    QColor toolbarBorder;     // Toolbar border
    
    // Region colors (for canvas)
    QColor regionBorder;      // Region border color
    QColor regionFill;        // Region fill (semi-transparent)
    QColor regionSelected;    // Selected region color
    QColor regionHover;       // Hovered region color
    
    // Status colors
    QColor success;           // Success message color
    QColor warning;           // Warning message color
    QColor error;             // Error message color
};

/**
 * @brief Theme Manager
 * 
 * Manages application theming including:
 * - Light and dark themes
 * - System theme detection
 * - Theme application to widgets
 * - Theme preference persistence
 */
class ThemeManager {
public:
    /**
     * @brief Get singleton instance
     */
    static ThemeManager& instance();
    
    /**
     * @brief Set the current theme
     * @param theme Theme to set (Light, Dark, or System)
     */
    void setTheme(Theme theme);
    
    /**
     * @brief Get the current theme
     * @return Current theme
     */
    Theme getTheme() const;
    
    /**
     * @brief Get the effective theme (resolves System to Light/Dark)
     * @return Effective theme (Light or Dark)
     */
    Theme getEffectiveTheme() const;
    
    /**
     * @brief Get colors for current theme
     * @return ThemeColors structure
     */
    ThemeColors getColors() const;
    
    /**
     * @brief Get colors for specific theme
     * @param theme Theme to get colors for
     * @return ThemeColors structure
     */
    ThemeColors getColors(Theme theme) const;
    
    /**
     * @brief Apply theme to a widget and its children
     * @param widget Widget to apply theme to
     */
    void applyTheme(::QWidget* widget);
    
    /**
     * @brief Get stylesheet for current theme
     * @return Qt stylesheet string
     */
    QString getStylesheet() const;
    
    /**
     * @brief Get stylesheet for specific theme
     * @param theme Theme to get stylesheet for
     * @return Qt stylesheet string
     */
    QString getStylesheet(Theme theme) const;
    
    /**
     * @brief Detect if system is using dark mode
     * @return true if system is in dark mode
     */
    static bool isSystemDarkMode();
    
    /**
     * @brief Load theme preference from settings
     */
    void loadSettings();
    
    /**
     * @brief Save theme preference to settings
     */
    void saveSettings();
    

private:
    ThemeManager();
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    /**
     * @brief Get light theme colors
     */
    static ThemeColors getLightThemeColors();
    
    /**
     * @brief Get dark theme colors
     */
    static ThemeColors getDarkThemeColors();
    
    /**
     * @brief Generate stylesheet from colors
     */
    QString generateStylesheet(const ThemeColors& colors) const;
    
    Theme currentTheme;
    QSettings* settings;
};

} // namespace ocr_orc

#endif // THEME_MANAGER_H

