#include "ToolbarWidget.h"
#include "../icons/IconManager.h"
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyle>
#include <QtWidgets/QLabel>
#include <QtGui/QPixmap>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QDebug>

namespace ocr_orc {

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : QFrame(parent)
    , modeToggleWidget(nullptr)
    , modeButtonGroup(nullptr)
    , createModeButton(nullptr)
    , selectModeButton(nullptr)
    , zoomOutButton(nullptr)
    , zoomLabel(nullptr)
    , zoomInButton(nullptr)
    , zoomResetButton(nullptr)
    , groupButton(nullptr)
    , ungroupButton(nullptr)
    , undoButton(nullptr)
    , redoButton(nullptr)
    , logoLabel(nullptr)
{
    setFixedHeight(50);
    setStyleSheet(
        "QFrame {"
        "    background-color: #e0e0e0;"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 0px;"
        "}"
    );
    setFrameShape(QFrame::Box);
    setupUI();
}

void ToolbarWidget::setupUI() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(10);
    
    // Add logo on the left
    setupLogo(layout);
    layout->addSpacing(20);
    
    setupMode(layout);
    layout->addSpacing(20);
    setupZoom(layout);
    layout->addSpacing(20);
    setupGroups(layout);
    layout->addSpacing(20);
    setupUndoRedo(layout);
    layout->addStretch();
}

void ToolbarWidget::setupLogo(QHBoxLayout* layout) {
    // Find logo file (look in resources directory)
    QString logoPath;
    QDir appDir(QCoreApplication::applicationDirPath());
    
    // Try multiple possible locations
    QStringList possiblePaths = {
        appDir.absoluteFilePath("../Resources/OCR-Orc-LOGO.png"),
        appDir.absoluteFilePath("../../OCR-Orc-LOGO.png"),
        appDir.absoluteFilePath("../../../OCR-Orc-LOGO.png"),
        QDir::home().absoluteFilePath("Desktop/OWD-OCR_Coordinates/OCR-Orc-LOGO.png"),
        QDir::home().absoluteFilePath("Desktop/OWD-OCR_Coordinates/Optical Wizard Doctor/../OCR-Orc-LOGO.png"),
    };
    
    // Also try relative to source/build
    QDir sourceDir = appDir;
    sourceDir.cdUp();
    sourceDir.cdUp();
    possiblePaths << sourceDir.absoluteFilePath("OCR-Orc-LOGO.png");
    
    for (const QString& path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            logoPath = path;
            break;
        }
    }
    
    logoLabel = new QLabel(this);
    logoLabel->setFixedSize(40, 40);
    logoLabel->setScaledContents(true);
    logoLabel->setStyleSheet("background-color: transparent; border: none;");
    
    if (!logoPath.isEmpty() && QFileInfo::exists(logoPath)) {
        QPixmap logo(logoPath);
        if (!logo.isNull()) {
            logoLabel->setPixmap(logo.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            logoLabel->setToolTip("OCR-Orc");
        }
    }
    
    layout->addWidget(logoLabel);
}

void ToolbarWidget::setupMode(QHBoxLayout* layout) {
    QLabel* modeLabel = new QLabel("Mode:", this);
    modeLabel->setStyleSheet("font-weight: bold; color: #000000;");
    layout->addWidget(modeLabel);
    
    // Create custom toggle slider widget
    modeToggleWidget = new ModeToggleWidget(this);
    connect(modeToggleWidget, &ModeToggleWidget::createModeSelected, this, &ToolbarWidget::createModeSelected);
    connect(modeToggleWidget, &ModeToggleWidget::selectModeSelected, this, &ToolbarWidget::selectModeSelected);
    layout->addWidget(modeToggleWidget);
    
    // Set default to Create mode
    modeToggleWidget->setMode(ModeToggleWidget::Create);
}

void ToolbarWidget::setupZoom(QHBoxLayout* layout) {
    QLabel* zoomLabelText = new QLabel("Zoom:", this);
    zoomLabelText->setStyleSheet("font-weight: bold; color: #000000;");
    layout->addWidget(zoomLabelText);
    
    zoomOutButton = new QPushButton(this);
    zoomOutButton->setFixedSize(32, 32);
    zoomOutButton->setIcon(IconManager::instance().getIcon("zoom_out"));
    zoomOutButton->setIconSize(QSize(20, 20));
    zoomOutButton->setToolTip("Zoom out (Shortcut: -)");
    zoomOutButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    background-color: #ffffff;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f0f0f0;"
        "    border-color: #999;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e0e0e0;"
        "}"
    );
    connect(zoomOutButton, &QPushButton::clicked, this, &ToolbarWidget::zoomOutClicked);
    layout->addWidget(zoomOutButton);
    
    zoomLabel = new QLabel("100%", this);
    zoomLabel->setMinimumWidth(50);
    zoomLabel->setMaximumWidth(80);
    zoomLabel->setAlignment(Qt::AlignCenter);
    zoomLabel->setStyleSheet("color: #000000;");
    zoomLabel->setTextFormat(Qt::PlainText);
    layout->addWidget(zoomLabel);
    
    zoomInButton = new QPushButton(this);
    zoomInButton->setFixedSize(32, 32);
    zoomInButton->setIcon(IconManager::instance().getIcon("zoom_in"));
    zoomInButton->setIconSize(QSize(20, 20));
    zoomInButton->setToolTip("Zoom in (Shortcut: +/=)");
    zoomInButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    background-color: #ffffff;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f0f0f0;"
        "    border-color: #999;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e0e0e0;"
        "}"
    );
    connect(zoomInButton, &QPushButton::clicked, this, &ToolbarWidget::zoomInClicked);
    layout->addWidget(zoomInButton);
    
    zoomResetButton = new QPushButton("100%", this);
    zoomResetButton->setFixedSize(60, 32);
    zoomResetButton->setToolTip("Reset zoom to 100% (Shortcut: 0)");
    zoomResetButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    background-color: #ffffff;"
        "    color: #000000;"
        "    font-size: 11px;"
        "    font-weight: bold;"
        "    padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f0f0f0;"
        "    border-color: #999;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e0e0e0;"
        "}"
    );
    connect(zoomResetButton, &QPushButton::clicked, this, &ToolbarWidget::zoomResetClicked);
    layout->addWidget(zoomResetButton);
    
    // Rotate button (using zoom_reset icon)
    rotateButton = new QPushButton(this);
    rotateButton->setFixedSize(32, 32);
    rotateButton->setIcon(IconManager::instance().getIcon("zoom_reset"));
    rotateButton->setIconSize(QSize(20, 20));
    rotateButton->setToolTip("Toggle Rotate Mode - Show rotate icon instead of resize handles");
    rotateButton->setCheckable(true);
    rotateButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    background-color: #ffffff;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f0f0f0;"
        "    border-color: #999;"
        "}"
        "QPushButton:pressed, QPushButton:checked {"
        "    background-color: #e0e0e0;"
        "}"
    );
    connect(rotateButton, &QPushButton::clicked, this, &ToolbarWidget::rotateClicked);
    layout->addWidget(rotateButton);
    
    // Magic Detect button
    magicDetectButton = new QPushButton(this);
    magicDetectButton->setFixedSize(32, 32);
    magicDetectButton->setIcon(IconManager::instance().getIcon("create")); // Temporary icon
    magicDetectButton->setIconSize(QSize(20, 20));
    magicDetectButton->setToolTip("Magic Detect - Auto-detect regions (Shortcut: Cmd+M)");
    magicDetectButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #2196F3;"
        "    border-radius: 4px;"
        "    background-color: #2196F3;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "    border-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "}"
    );
    connect(magicDetectButton, &QPushButton::clicked, this, &ToolbarWidget::magicDetectClicked);
    layout->addWidget(magicDetectButton);
}

void ToolbarWidget::updateRotateButton(bool enabled) {
    if (rotateButton) {
        rotateButton->setChecked(enabled);
    }
}

void ToolbarWidget::setupGroups(QHBoxLayout* layout) {
    groupButton = new QPushButton(this);
    groupButton->setFixedSize(36, 36);
    groupButton->setIcon(IconManager::instance().getIcon("group"));
    groupButton->setIconSize(QSize(20, 20));
    groupButton->setToolTip("Group - Group selected regions (Shortcut: Cmd+G)");
    groupButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #4CAF50;"
        "    border-radius: 4px;"
        "    background-color: #4CAF50;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "    border-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d8b40;"
        "}"
    );
    connect(groupButton, &QPushButton::clicked, this, &ToolbarWidget::groupClicked);
    layout->addWidget(groupButton);
    
    ungroupButton = new QPushButton(this);
    ungroupButton->setFixedSize(36, 36);
    ungroupButton->setIcon(IconManager::instance().getIcon("ungroup"));
    ungroupButton->setIconSize(QSize(20, 20));
    ungroupButton->setToolTip("Ungroup - Ungroup selected regions (Shortcut: Cmd+Shift+G)");
    ungroupButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #ff9800;"
        "    border-radius: 4px;"
        "    background-color: #ff9800;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f57c00;"
        "    border-color: #f57c00;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #ef6c00;"
        "}"
    );
    connect(ungroupButton, &QPushButton::clicked, this, &ToolbarWidget::ungroupClicked);
    layout->addWidget(ungroupButton);
}

void ToolbarWidget::setupUndoRedo(QHBoxLayout* layout) {
    undoButton = new QPushButton(this);
    undoButton->setFixedSize(36, 36);
    undoButton->setEnabled(false);
    undoButton->setIcon(IconManager::instance().getIcon("undo"));
    undoButton->setIconSize(QSize(20, 20));
    undoButton->setToolTip("Undo - Undo last action (Shortcut: Cmd+Z)");
    undoButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #2196F3;"
        "    border-radius: 4px;"
        "    background-color: #2196F3;"
        "}"
        "QPushButton:hover:enabled {"
        "    background-color: #1976D2;"
        "    border-color: #1976D2;"
        "}"
        "QPushButton:pressed:enabled {"
        "    background-color: #1565C0;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    border-color: #aaaaaa;"
        "}"
    );
    connect(undoButton, &QPushButton::clicked, this, &ToolbarWidget::undoClicked);
    layout->addWidget(undoButton);
    
    redoButton = new QPushButton(this);
    redoButton->setFixedSize(36, 36);
    redoButton->setEnabled(false);
    redoButton->setIcon(IconManager::instance().getIcon("redo"));
    redoButton->setIconSize(QSize(20, 20));
    redoButton->setToolTip("Redo - Redo last undone action (Shortcut: Cmd+Shift+Z)");
    redoButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #2196F3;"
        "    border-radius: 4px;"
        "    background-color: #2196F3;"
        "}"
        "QPushButton:hover:enabled {"
        "    background-color: #1976D2;"
        "    border-color: #1976D2;"
        "}"
        "QPushButton:pressed:enabled {"
        "    background-color: #1565C0;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    border-color: #aaaaaa;"
        "}"
    );
    connect(redoButton, &QPushButton::clicked, this, &ToolbarWidget::redoClicked);
    layout->addWidget(redoButton);
}

void ToolbarWidget::updateZoomLabel(double zoomLevel) {
    qDebug() << "[ZOOM DEBUG] ToolbarWidget::updateZoomLabel(" << zoomLevel << ") called";
    int percentage = static_cast<int>(zoomLevel * 100.0);
    QString newText = QString("%1%").arg(percentage);
    qDebug() << "[ZOOM DEBUG] Setting zoom label text to:" << newText;
    if (zoomLabel) {
        zoomLabel->setText(newText);
        qDebug() << "[ZOOM DEBUG] Zoom label text updated successfully";
    } else {
        qDebug() << "[ZOOM DEBUG] zoomLabel is null!";
    }
}

void ToolbarWidget::updateUndoRedoButtons(bool canUndo, bool canRedo) {
    undoButton->setEnabled(canUndo);
    redoButton->setEnabled(canRedo);
}

void ToolbarWidget::refreshIcons() {
    // Clear icon cache first to force regeneration with new colors
    IconManager::instance().clearIconCache();
    
    // Refresh all icons to pick up theme changes
    if (zoomOutButton) {
        zoomOutButton->setIcon(QIcon()); // Clear first to force refresh
        QCoreApplication::processEvents(); // Process events to clear icon
        zoomOutButton->setIcon(IconManager::instance().getIcon("zoom_out"));
    }
    if (zoomInButton) {
        zoomInButton->setIcon(QIcon());
        QCoreApplication::processEvents();
        zoomInButton->setIcon(IconManager::instance().getIcon("zoom_in"));
    }
    if (rotateButton) {
        rotateButton->setIcon(QIcon());
        QCoreApplication::processEvents();
        rotateButton->setIcon(IconManager::instance().getIcon("zoom_reset"));
    }
    if (magicDetectButton) {
        magicDetectButton->setIcon(QIcon());
        QCoreApplication::processEvents();
        magicDetectButton->setIcon(IconManager::instance().getIcon("create"));
    }
    if (groupButton) {
        groupButton->setIcon(QIcon());
        QCoreApplication::processEvents();
        groupButton->setIcon(IconManager::instance().getIcon("group"));
    }
    if (ungroupButton) {
        ungroupButton->setIcon(QIcon());
        QCoreApplication::processEvents();
        ungroupButton->setIcon(IconManager::instance().getIcon("ungroup"));
    }
    if (undoButton) {
        undoButton->setIcon(QIcon());
        QCoreApplication::processEvents();
        undoButton->setIcon(IconManager::instance().getIcon("undo"));
    }
    if (redoButton) {
        redoButton->setIcon(QIcon());
        QCoreApplication::processEvents();
        redoButton->setIcon(IconManager::instance().getIcon("redo"));
    }
    // Force widget update
    update();
}

} // namespace ocr_orc

