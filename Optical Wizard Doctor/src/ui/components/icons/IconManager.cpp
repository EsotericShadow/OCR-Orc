#include "IconManager.h"
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

QIcon IconManager::getIcon(const QString& iconName) const {
    QStyle* style = QApplication::style();
    if (!style) {
        return QIcon();
    }
    
    // Map icon names to Qt standard icons
    if (iconName == ICON_OPEN) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/load_pdf.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_DirOpenIcon);
    } else if (iconName == ICON_SAVE || iconName == ICON_EXPORT) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/export.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_DialogSaveButton);
    } else if (iconName == ICON_IMPORT) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/load_coordinates.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_FileDialogStart);
    } else if (iconName == ICON_UNDO) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/undo.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_ArrowBack);
    } else if (iconName == ICON_REDO) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/redo.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_ArrowForward);
    } else if (iconName == ICON_ZOOM_IN) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/zoom_in.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_FileDialogDetailedView);
    } else if (iconName == ICON_ZOOM_OUT) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/zoom_out.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_FileDialogListView);
    } else if (iconName == ICON_ZOOM_RESET) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/zoom_reset.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_FileDialogBack);
    } else if (iconName == ICON_CREATE) {
        // Try to load custom SVG icon first
        QIcon svgIcon = loadSvgIcon("resources/icons/create.svg");
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_FileDialogNewFolder);
    } else if (iconName == ICON_ADD) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/add_to_group.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_FileDialogNewFolder);
    } else if (iconName == ICON_SELECT || iconName == "select_move") {
        // Try to load custom SVG icon first
        QIcon svgIcon = loadSvgIcon("resources/icons/select_move.svg");
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_ArrowDown);
    } else if (iconName == ICON_GROUP) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/group.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_DirIcon);
    } else if (iconName == ICON_UNGROUP) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/ungroup.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_DirClosedIcon);
    } else if (iconName == ICON_DELETE) {
        // Try to load custom SVG icon first (red version for delete buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/delete_group.svg", Qt::red);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_TrashIcon);
    } else if (iconName == ICON_REMOVE) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/remove_from_group.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_TrashIcon);
    } else if (iconName == ICON_EDIT) {
        // Try to load custom SVG icon first (white version for buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/edit.svg", Qt::white);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_FileDialogInfoView);
    } else if (iconName == ICON_CLEAR) {
        // Try to load custom SVG icon first (red version for delete/clear buttons)
        QIcon svgIcon = loadSvgIconWithColor("resources/icons/clear.svg", Qt::red);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }
        return createStandardIcon(QStyle::SP_DialogResetButton);
    } else if (iconName == ICON_HELP) {
        // Theme-aware help icon: white on dark mode, black on light mode
        QPalette palette = QApplication::palette();
        QColor baseColor = palette.color(QPalette::Base);
        // Check if we're in dark mode (base color is dark)
        bool isDarkMode = baseColor.lightness() < 128;
        QColor iconColor = isDarkMode ? Qt::white : Qt::black;
        
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
                
                // Replace black color with desired color
                QString colorHex = color.name();
                svgContent.replace("#000000", colorHex, Qt::CaseInsensitive);
                svgContent.replace("color=\"#000000\"", QString("color=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                svgContent.replace("stroke=\"#000000\"", QString("stroke=\"%1\"").arg(colorHex), Qt::CaseInsensitive);
                
                // Render SVG with modified color
                QSvgRenderer renderer(svgContent.toUtf8());
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
    }
    
    return QIcon(); // Return empty icon if not found
}

} // namespace ocr_orc

