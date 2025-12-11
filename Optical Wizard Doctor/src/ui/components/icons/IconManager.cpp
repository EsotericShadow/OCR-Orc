#include "IconManager.h"
#include "../../ThemeManager.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPalette>
#include <QtSvg/QSvgRenderer>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegularExpression>
#include <QtCore/QMap>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

IconManager& IconManager::instance() {
    static IconManager instance;
    return instance;
}

IconManager::IconManager()
    : iconSize(24)
{
}

void IconManager::clearIconCache() {
    // Clear the icon cache to force regeneration when theme changes
    iconCache.clear();
}

QIcon IconManager::getIcon(const QString& iconName) const {
    QStyle* style = QApplication::style();
    if (!style) {
        return QIcon();
    }
    
    // Use ThemeManager to determine the actual theme (not palette, which may not reflect stylesheet changes)
    ThemeManager& themeManager = ThemeManager::instance();
    Theme effectiveTheme = themeManager.getEffectiveTheme();
    bool isDarkMode = (effectiveTheme == Theme::Dark);
    
    // Debug: log what theme we're using
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[ICON DEBUG] getIcon:" << iconName << "effectiveTheme:" << (isDarkMode ? "Dark" : "Light");
    #endif
    
    // Helper function to get icon file path based on theme
    // For light mode: use original black SVGs
    // For dark mode: use white versions (_white.svg)
    auto getIconPath = [isDarkMode](const QString& iconFile) -> QString {
        if (isDarkMode) {
            // Use white version for dark mode
            QString baseName = iconFile;
            if (baseName.endsWith(".svg")) {
                baseName.chop(4); // Remove .svg
            }
            return QString("resources/icons/%1_white.svg").arg(baseName);
        } else {
            // Use original black version for light mode
            return QString("resources/icons/%1").arg(iconFile);
        }
    };
    
    // Map icon names to icon files - use separate files for light/dark themes
    // For delete/clear, always use red (use original file, not white version)
    if (iconName == ICON_OPEN) {
        QIcon svgIcon = loadSvgIcon(getIconPath("load_pdf.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_DirOpenIcon);
    } else if (iconName == ICON_SAVE || iconName == ICON_EXPORT) {
        QIcon svgIcon = loadSvgIcon(getIconPath("export.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_DialogSaveButton);
    } else if (iconName == ICON_IMPORT) {
        QIcon svgIcon = loadSvgIcon(getIconPath("load_coordinates.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_FileDialogStart);
    } else if (iconName == ICON_UNDO) {
        QIcon svgIcon = loadSvgIcon(getIconPath("undo.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_ArrowBack);
    } else if (iconName == ICON_REDO) {
        QIcon svgIcon = loadSvgIcon(getIconPath("redo.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_ArrowForward);
    } else if (iconName == ICON_ZOOM_IN) {
        QIcon svgIcon = loadSvgIcon(getIconPath("zoom_in.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_FileDialogDetailedView);
    } else if (iconName == ICON_ZOOM_OUT) {
        QIcon svgIcon = loadSvgIcon(getIconPath("zoom_out.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_FileDialogListView);
    } else if (iconName == ICON_ZOOM_RESET) {
        QIcon svgIcon = loadSvgIcon(getIconPath("zoom_reset.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_FileDialogBack);
    } else if (iconName == ICON_CREATE) {
        QIcon svgIcon = loadSvgIcon(getIconPath("create.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_FileDialogNewFolder);
    } else if (iconName == ICON_ADD) {
        QIcon svgIcon = loadSvgIcon(getIconPath("add_to_group.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_FileDialogNewFolder);
    } else if (iconName == ICON_SELECT || iconName == "select_move") {
        QIcon svgIcon = loadSvgIcon(getIconPath("select_move.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_ArrowDown);
    } else if (iconName == ICON_GROUP) {
        QIcon svgIcon = loadSvgIcon(getIconPath("group.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_DirIcon);
    } else if (iconName == ICON_UNGROUP) {
        QIcon svgIcon = loadSvgIcon(getIconPath("ungroup.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_DirClosedIcon);
    } else if (iconName == ICON_DELETE) {
        // Always keep delete red - use original file (not white version) and colorize red
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/delete_group.svg", Qt::red);
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_TrashIcon);
    } else if (iconName == ICON_REMOVE) {
        QIcon svgIcon = loadSvgIcon(getIconPath("remove_from_group.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_TrashIcon);
    } else if (iconName == ICON_EDIT) {
        QIcon svgIcon = loadSvgIcon(getIconPath("edit.svg"));
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_FileDialogInfoView);
    } else if (iconName == ICON_CLEAR) {
        // Always keep clear red - use original file (not white version) and colorize red
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/clear.svg", Qt::red);
        if (!svgIcon.isNull()) return svgIcon;
        return createStandardIcon(QStyle::SP_DialogResetButton);
    } else if (iconName == ICON_HELP) {
        // Theme-aware help icon: white on dark mode, black on light mode
        // Use ThemeManager to determine theme (same as other icons)
        QColor iconColor = isDarkMode ? Qt::white : Qt::black;
        
        // Create a simple "?" icon with theme-appropriate color
        return createTextIcon("?", iconColor.name());
        
        // Create a simple "?" icon with theme-appropriate color
        return createTextIcon("?", iconColor.name());
    } else if (iconName == ICON_MASK) {
        return createStandardIcon(QStyle::SP_FileIcon);
    } else if (iconName == ICON_COLOR) {
        return createStandardIcon(QStyle::SP_FileDialogContentsView);
    }
    
    // Fallback: return empty icon
    return QIcon();
}

QIcon IconManager::getStandardIcon(QStyle::StandardPixmap standardIcon) const {
    return createStandardIcon(standardIcon);
}

QIcon IconManager::createStandardIcon(QStyle::StandardPixmap standardIcon) const {
    QStyle* style = QApplication::style();
    if (!style) {
        return QIcon();
    }
    
    QIcon icon = style->standardIcon(standardIcon);
    
    // If standard icon is not available, create a text-based fallback
    if (icon.isNull()) {
        QString text;
        switch (standardIcon) {
            case QStyle::SP_DirOpenIcon:
                text = "📂";
                break;
            case QStyle::SP_DialogSaveButton:
                text = "💾";
                break;
            case QStyle::SP_ArrowBack:
                text = "↶";
                break;
            case QStyle::SP_ArrowForward:
                text = "↷";
                break;
            case QStyle::SP_TrashIcon:
                text = "🗑";
                break;
            case QStyle::SP_MessageBoxQuestion:
                text = "?";
                break;
            default:
                text = "•";
                break;
        }
        return createTextIcon(text);
    }
    
    return icon;
}

QIcon IconManager::createTextIcon(const QString& text, const QString& color) const {
    QPixmap pixmap(iconSize, iconSize);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Set font
    QFont font = painter.font();
    font.setPixelSize(iconSize * 0.7);
    font.setBold(true);
    painter.setFont(font);
    
    // Set color
    painter.setPen(QColor(color));
    
    // Draw text centered
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    QPoint pos((iconSize - textRect.width()) / 2, 
               (iconSize - textRect.height()) / 2 + fm.ascent());
    painter.drawText(pos, text);
    
    QIcon icon(pixmap);
    return icon;
}

QIcon IconManager::loadSvgIcon(const QString& svgPath) const {
    // Try multiple paths: absolute, relative to current directory, relative to executable
    QStringList searchPaths;
    
    // Absolute path (if provided)
    if (QFileInfo(svgPath).isAbsolute()) {
        searchPaths << svgPath;
    }
    
    // Relative to current working directory
    searchPaths << QDir::current().absoluteFilePath(svgPath);
    
    // Relative to executable directory (for deployed apps)
    QDir appDir(QApplication::applicationDirPath());
    searchPaths << appDir.absoluteFilePath(svgPath);
    
    // For macOS app bundle: Contents/MacOS/ocr-orc -> Contents/Resources/resources/icons/...
    QDir execDir(QApplication::applicationDirPath());
    if (execDir.cdUp() && execDir.dirName() == "MacOS") {
        // We're in Contents/MacOS, go to Contents/Resources
        if (execDir.cdUp() && execDir.cd("Resources")) {
            QString resourcesPath = execDir.absoluteFilePath(svgPath);
            searchPaths << resourcesPath;
        }
    }
    
    // Try to find resources directory relative to executable (fallback for dev builds)
    // For macOS: bin/owd.app/Contents/MacOS/owd -> go up to project root
    execDir.setPath(QApplication::applicationDirPath());
    for (int i = 0; i < 5; ++i) {
        if (execDir.cdUp()) {
            QString testPath = execDir.absoluteFilePath(svgPath);
            searchPaths << testPath;
        }
    }
    
    // Try each path
    for (const QString& path : searchPaths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists() && fileInfo.isFile()) {
            QSvgRenderer renderer(path);
            if (renderer.isValid()) {
                QPixmap pixmap(iconSize, iconSize);
                pixmap.fill(Qt::transparent);
                
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                renderer.render(&painter);
                painter.end();
                
                QIcon icon(pixmap);
                return icon;
            }
        }
    }
    
    return QIcon(); // Return empty icon if not found
}

QIcon IconManager::loadSvgIconWithColor(const QString& svgPath, const QColor& color) const {
    // Use a cache key that includes the color to prevent wrong caching
    // This ensures black and white versions are treated as different icons
    QString cacheKey = QString("%1_%2").arg(svgPath, color.name());
    
    // Check cache first
    if (iconCache.contains(cacheKey)) {
        #if OCR_ORC_DEBUG_ENABLED
        qDebug() << "[ICON DEBUG] Using cached icon:" << cacheKey;
        #endif
        return iconCache[cacheKey];
    }
    
    // Note: For most icons, we now use separate files (black/white versions)
    // This function is still used for delete/clear icons which need red coloring
    
    // Try multiple paths: absolute, relative to current directory, relative to executable
    QStringList searchPaths;
    
    // Absolute path (if provided)
    if (QFileInfo(svgPath).isAbsolute()) {
        searchPaths << svgPath;
    }
    
    // Relative to current working directory
    searchPaths << QDir::current().absoluteFilePath(svgPath);
    
    // Relative to executable directory (for deployed apps)
    QDir appDir(QApplication::applicationDirPath());
    searchPaths << appDir.absoluteFilePath(svgPath);
    
    // For macOS app bundle: Contents/MacOS/ocr-orc -> Contents/Resources/resources/icons/...
    QDir execDir(QApplication::applicationDirPath());
    if (execDir.cdUp() && execDir.dirName() == "MacOS") {
        // We're in Contents/MacOS, go to Contents/Resources
        if (execDir.cdUp() && execDir.cd("Resources")) {
            QString resourcesPath = execDir.absoluteFilePath(svgPath);
            searchPaths << resourcesPath;
        }
    }
    
    // Try to find resources directory relative to executable (fallback for dev builds)
    // For macOS: bin/owd.app/Contents/MacOS/owd -> go up to project root
    execDir.setPath(QApplication::applicationDirPath());
    for (int i = 0; i < 5; ++i) {
        if (execDir.cdUp()) {
            QString testPath = execDir.absoluteFilePath(svgPath);
            searchPaths << testPath;
        }
    }
    
    // Try each path
    for (const QString& path : searchPaths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists() && fileInfo.isFile()) {
            // Read SVG file
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString svgContent = in.readAll();
                file.close();
                
                // Replace ALL color references with desired color (black or white based on theme)
                QString colorHex = color.name();
                
                #if OCR_ORC_DEBUG_ENABLED
                qDebug() << "[ICON DEBUG] Replacing colors in SVG:" << svgPath << "with color:" << colorHex;
                QString originalContent = svgContent.left(300); // First 300 chars for debug
                int originalBlackCount = svgContent.count("#000000", Qt::CaseInsensitive);
                qDebug() << "[ICON DEBUG] Found" << originalBlackCount << "instances of #000000 in original";
                #endif
                
                // AGGRESSIVE replacement: Replace ALL instances of black/white colors
                // Replace in all possible formats and locations
                
                // 1. Standalone hex colors anywhere in the file (do this FIRST, most common)
                svgContent.replace("#000000", colorHex, Qt::CaseInsensitive);
                svgContent.replace("#000", colorHex, Qt::CaseInsensitive);
                svgContent.replace("#ffffff", colorHex, Qt::CaseInsensitive);
                svgContent.replace("#fff", colorHex, Qt::CaseInsensitive);
                
                #if OCR_ORC_DEBUG_ENABLED
                int afterBlackCount = svgContent.count("#000000", Qt::CaseInsensitive);
                int newColorCount = svgContent.count(colorHex, Qt::CaseInsensitive);
                qDebug() << "[ICON DEBUG] After replacement - #000000 count:" << afterBlackCount << "new color count:" << newColorCount;
                #endif
                
                // 2. In stroke attributes (with quotes)
                svgContent.replace(QRegularExpression("stroke=\"#000000\"", QRegularExpression::CaseInsensitiveOption), QString("stroke=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("stroke=\"#000\"", QRegularExpression::CaseInsensitiveOption), QString("stroke=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("stroke=\"#ffffff\"", QRegularExpression::CaseInsensitiveOption), QString("stroke=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("stroke=\"#fff\"", QRegularExpression::CaseInsensitiveOption), QString("stroke=\"%1\"").arg(colorHex));
                svgContent.replace("stroke=\"black\"", QString("stroke=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                svgContent.replace("stroke=\"white\"", QString("stroke=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                
                // 3. In fill attributes
                svgContent.replace(QRegularExpression("fill=\"#000000\"", QRegularExpression::CaseInsensitiveOption), QString("fill=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("fill=\"#000\"", QRegularExpression::CaseInsensitiveOption), QString("fill=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("fill=\"#ffffff\"", QRegularExpression::CaseInsensitiveOption), QString("fill=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("fill=\"#fff\"", QRegularExpression::CaseInsensitiveOption), QString("fill=\"%1\"").arg(colorHex));
                svgContent.replace("fill=\"black\"", QString("fill=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                svgContent.replace("fill=\"white\"", QString("fill=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                
                // 4. In color attribute (root SVG element)
                svgContent.replace(QRegularExpression("color=\"#000000\"", QRegularExpression::CaseInsensitiveOption), QString("color=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("color=\"#000\"", QRegularExpression::CaseInsensitiveOption), QString("color=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("color=\"#ffffff\"", QRegularExpression::CaseInsensitiveOption), QString("color=\"%1\"").arg(colorHex));
                svgContent.replace(QRegularExpression("color=\"#fff\"", QRegularExpression::CaseInsensitiveOption), QString("color=\"%1\"").arg(colorHex));
                svgContent.replace("color=\"black\"", QString("color=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                svgContent.replace("color=\"white\"", QString("color=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                
                #if OCR_ORC_DEBUG_ENABLED
                QString modifiedContent = svgContent.left(300);
                qDebug() << "[ICON DEBUG] Original SVG start:" << originalContent;
                qDebug() << "[ICON DEBUG] Modified SVG start:" << modifiedContent;
                // Verify replacement worked
                if (svgContent.contains("#000000", Qt::CaseInsensitive)) {
                    qDebug() << "[ICON DEBUG] WARNING: Still contains #000000 after replacement!";
                }
                if (svgContent.contains(colorHex)) {
                    qDebug() << "[ICON DEBUG] SUCCESS: SVG now contains target color" << colorHex;
                }
                #endif
                
                // Render SVG with modified color
                QSvgRenderer renderer(svgContent.toUtf8());
                if (renderer.isValid()) {
                    // Create icon with multiple pixmap sizes to avoid caching issues
                    QIcon icon;
                    
                    // Render at different sizes to ensure proper scaling
                    QList<int> sizes = {iconSize, iconSize * 2, iconSize / 2};
                    for (int size : sizes) {
                        if (size > 0) {
                            QPixmap pixmap(size, size);
                            pixmap.fill(Qt::transparent);
                            
                            QPainter painter(&pixmap);
                            painter.setRenderHint(QPainter::Antialiasing);
                            renderer.render(&painter);
                            painter.end();
                            
                            // Add pixmap with color in the cache key to prevent wrong caching
                            icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
                        }
                    }
                    
                    #if OCR_ORC_DEBUG_ENABLED
                    qDebug() << "[ICON DEBUG] Created icon with color:" << colorHex << "size:" << iconSize;
                    #endif
                    
                    // Cache the icon with color-specific key (iconCache is mutable)
                    iconCache[cacheKey] = icon;
                    return icon;
                } else {
                    #if OCR_ORC_DEBUG_ENABLED
                    qDebug() << "[ICON DEBUG] SVG renderer invalid after color replacement!";
                    #endif
                }
            }
        }
    }
    
    return QIcon(); // Return empty icon if not found
}

} // namespace ocr_orc

