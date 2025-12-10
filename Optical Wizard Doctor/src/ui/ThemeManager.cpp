#include "ThemeManager.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtCore/QDebug>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

#ifdef Q_OS_MAC
#include <QtCore/QOperatingSystemVersion>
#include <QtCore/QStandardPaths>
#endif

namespace ocr_orc {

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
    : currentTheme(Theme::System)
    , settings(nullptr)
{
    loadSettings();
}

ThemeColors ThemeManager::getLightThemeColors() {
    ThemeColors colors;
    
    // Background colors
    colors.background = QColor(255, 255, 255);      // White
    colors.surface = QColor(248, 248, 248);         // Light gray
    colors.canvasBackground = QColor(240, 240, 240); // Canvas gray
    
    // Text colors
    colors.text = QColor(0, 0, 0);                  // Black
    colors.textSecondary = QColor(100, 100, 100);   // Dark gray
    colors.textDisabled = QColor(180, 180, 180);    // Light gray
    
    // Accent colors
    colors.primary = QColor(0, 102, 255);          // Blue
    colors.primaryHover = QColor(0, 122, 255);     // Lighter blue
    colors.selection = QColor(0, 102, 255);        // Blue selection
    colors.selectionHover = QColor(0, 122, 255);   // Lighter blue hover
    
    // Border colors
    colors.border = QColor(200, 200, 200);         // Gray border
    colors.borderLight = QColor(230, 230, 230);    // Light border
    
    // Toolbar colors
    colors.toolbarBackground = QColor(245, 245, 245); // Light toolbar
    colors.toolbarBorder = QColor(220, 220, 220);    // Toolbar border
    
    // Region colors
    colors.regionBorder = QColor(0, 102, 255);     // Blue border
    colors.regionFill = QColor(0, 102, 255, 30);   // Semi-transparent blue
    colors.regionSelected = QColor(0, 102, 255);    // Blue selected
    colors.regionHover = QColor(0, 122, 255);      // Lighter blue hover
    
    // Status colors
    colors.success = QColor(0, 200, 0);            // Green
    colors.warning = QColor(255, 165, 0);          // Orange
    colors.error = QColor(255, 0, 0);              // Red
    
    return colors;
}

ThemeColors ThemeManager::getDarkThemeColors() {
    ThemeColors colors;
    
    // Background colors
    colors.background = QColor(30, 30, 30);         // Dark gray
    colors.surface = QColor(45, 45, 45);           // Darker gray
    colors.canvasBackground = QColor(25, 25, 25);  // Very dark canvas
    
    // Text colors
    colors.text = QColor(201, 209, 217);           // Light gray text
    colors.textSecondary = QColor(150, 150, 150);  // Medium gray
    colors.textDisabled = QColor(100, 100, 100);    // Dark gray
    
    // Accent colors
    colors.primary = QColor(88, 166, 255);         // Light blue
    colors.primaryHover = QColor(108, 186, 255);   // Lighter blue
    colors.selection = QColor(88, 166, 255);       // Light blue selection
    colors.selectionHover = QColor(108, 186, 255); // Lighter blue hover
    
    // Border colors
    colors.border = QColor(60, 60, 60);           // Dark border
    colors.borderLight = QColor(50, 50, 50);       // Darker border
    
    // Toolbar colors
    colors.toolbarBackground = QColor(40, 40, 40);  // Dark toolbar
    colors.toolbarBorder = QColor(55, 55, 55);      // Toolbar border
    
    // Region colors
    colors.regionBorder = QColor(88, 166, 255);   // Light blue border
    colors.regionFill = QColor(88, 166, 255, 40);  // Semi-transparent light blue
    colors.regionSelected = QColor(88, 166, 255);  // Light blue selected
    colors.regionHover = QColor(108, 186, 255);    // Lighter blue hover
    
    // Status colors
    colors.success = QColor(0, 255, 0);            // Bright green
    colors.warning = QColor(255, 200, 0);          // Yellow
    colors.error = QColor(255, 80, 80);            // Light red
    
    return colors;
}

bool ThemeManager::isSystemDarkMode() {
#ifdef Q_OS_MAC
    // macOS: Check NSAppearance
    // Note: Qt doesn't have direct API, so we check system settings
    // This is a simplified check - in production, use native APIs
    QSettings macSettings("Apple", "System Preferences");
    QString appearance = macSettings.value("AppleInterfaceStyle", "Light").toString();
    return appearance == "Dark";
#elif defined(Q_OS_WIN)
    // Windows: Check registry
    QSettings winSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    int appsUseLightTheme = winSettings.value("AppsUseLightTheme", 1).toInt();
    return appsUseLightTheme == 0;
#elif defined(Q_OS_LINUX)
    // Linux: Check desktop environment
    // Try common environment variables
    QString desktop = qgetenv("XDG_CURRENT_DESKTOP");
    QString theme = qgetenv("GTK_THEME");
    
    // Check if dark theme is in use
    if (theme.contains("dark", Qt::CaseInsensitive) || 
        theme.contains("Dark", Qt::CaseInsensitive)) {
        return true;
    }
    
    // Default to light
    return false;
#else
    // Default to light mode
    return false;
#endif
}

void ThemeManager::setTheme(Theme theme) {
    if (currentTheme == theme) {
        return;
    }
    
    currentTheme = theme;
    saveSettings();
    
    // Apply theme to the application
    QApplication* app = qApp;
    if (app) {
        app->setStyleSheet(getStylesheet());
    }
}

Theme ThemeManager::getTheme() const {
    return currentTheme;
}

Theme ThemeManager::getEffectiveTheme() const {
    if (currentTheme == Theme::System) {
        return isSystemDarkMode() ? Theme::Dark : Theme::Light;
    }
    return currentTheme;
}

ThemeColors ThemeManager::getColors() const {
    return getColors(getEffectiveTheme());
}

ThemeColors ThemeManager::getColors(Theme theme) const {
    Theme effectiveTheme = (theme == Theme::System) ? 
        (isSystemDarkMode() ? Theme::Dark : Theme::Light) : theme;
    
    if (effectiveTheme == Theme::Dark) {
        return getDarkThemeColors();
    } else {
        return getLightThemeColors();
    }
}

QString ThemeManager::generateStylesheet(const ThemeColors& colors) const {
    QString stylesheet;
    
    stylesheet += QString(
        "QMainWindow {"
        "    background-color: %1;"
        "    color: %2;"
        "}"
    ).arg(colors.background.name(), colors.text.name());
    
    stylesheet += QString(
        "QToolBar {"
        "    background-color: %1;"
        "    border-bottom: 1px solid %2;"
        "}"
    ).arg(colors.toolbarBackground.name(), colors.toolbarBorder.name());
    
    stylesheet += QString(
        "QWidget {"
        "    background-color: %1;"
        "    color: %2;"
        "}"
    ).arg(colors.background.name(), colors.text.name());
    
    stylesheet += QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    padding: 4px 12px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %4;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %5;"
        "}"
    ).arg(colors.surface.name(), colors.text.name(), colors.border.name(),
          colors.primaryHover.name(), colors.primary.name());
    
    stylesheet += QString(
        "QListWidget {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: %4;"
        "    color: %2;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: %5;"
        "}"
    ).arg(colors.surface.name(), colors.text.name(), colors.border.name(),
          colors.selection.name(), colors.selectionHover.name());
    
    stylesheet += QString(
        "QLineEdit, QComboBox {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    padding: 4px;"
        "    border-radius: 4px;"
        "}"
    ).arg(colors.surface.name(), colors.text.name(), colors.border.name());
    
    stylesheet += QString(
        "QStatusBar {"
        "    background-color: %1;"
        "    color: %2;"
        "    border-top: 1px solid %3;"
        "}"
    ).arg(colors.toolbarBackground.name(), colors.text.name(), colors.toolbarBorder.name());
    
    stylesheet += QString(
        "QMenuBar {"
        "    background-color: %1;"
        "    color: %2;"
        "}"
        "QMenuBar::item:selected {"
        "    background-color: %3;"
        "}"
    ).arg(colors.toolbarBackground.name(), colors.text.name(), colors.selectionHover.name());
    
    stylesheet += QString(
        "QMenu {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "}"
        "QMenu::item:selected {"
        "    background-color: %4;"
        "}"
    ).arg(colors.surface.name(), colors.text.name(), colors.border.name(), colors.selection.name());
    
    return stylesheet;
}

QString ThemeManager::getStylesheet() const {
    return getStylesheet(getEffectiveTheme());
}

QString ThemeManager::getStylesheet(Theme theme) const {
    ThemeColors colors = getColors(theme);
    return generateStylesheet(colors);
}

void ThemeManager::applyTheme(::QWidget* widget) {
    if (!widget) {
        return;
    }
    
    QString stylesheet = getStylesheet();
    widget->setStyleSheet(stylesheet);
    
    // Apply to all children recursively
    QList<::QWidget*> children = widget->findChildren<::QWidget*>();
    for (::QWidget* child : children) {
        child->setStyleSheet(stylesheet);
    }
}

void ThemeManager::loadSettings() {
    if (!settings) {
        settings = new QSettings("OCROrc", "OCR-Orc");
    }
    
    QString themeStr = settings->value("theme", "System").toString();
    if (themeStr == "Light") {
        currentTheme = Theme::Light;
    } else if (themeStr == "Dark") {
        currentTheme = Theme::Dark;
    } else {
        currentTheme = Theme::System;
    }
}

void ThemeManager::saveSettings() {
    if (!settings) {
        settings = new QSettings("OCROrc", "OCR-Orc");
    }
    
    QString themeStr;
    switch (currentTheme) {
        case Theme::Light:
            themeStr = "Light";
            break;
        case Theme::Dark:
            themeStr = "Dark";
            break;
        case Theme::System:
        default:
            themeStr = "System";
            break;
    }
    
    settings->setValue("theme", themeStr);
    settings->sync();
}


} // namespace ocr_orc

