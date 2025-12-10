#include "ModeToggleWidget.h"
#include "../icons/IconManager.h"
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QStyle>
#include <QtCore/QEasingCurve>
#include <QtCore/QPropertyAnimation>
#include <cmath>

namespace ocr_orc {

ModeToggleWidget::ModeToggleWidget(QWidget *parent)
    : QWidget(parent)
    , currentMode(Create)
    , indicatorPosition(0.0)
    , animation(nullptr)
    , iconSize(24, 24)
{
    setFixedSize(90, 50);  // 1.25:2.25 ratio (50:90 = 1.25:2.25)
    setToolTip("Mode: Create (1) / Select-Move (2)");
    
    // Setup animation for smooth sliding
    animation = new QPropertyAnimation(this, "indicatorPosition", this);
    animation->setDuration(200);  // 200ms animation
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    
    updateIndicatorPosition();
}

void ModeToggleWidget::setMode(Mode mode) {
    if (currentMode == mode) {
        return;
    }
    
    currentMode = mode;
    
    // Animate indicator to new position
    double targetPosition = (mode == Create) ? 0.0 : 1.0;
    animation->setStartValue(indicatorPosition);
    animation->setEndValue(targetPosition);
    animation->start();
    
    update();
}

void ModeToggleWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRectF rect = this->rect();
    
    // Draw main container (light grey)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(LIGHT_GREY));
    painter.drawRoundedRect(rect, 8, 8);
    
    // Calculate positions
    double containerWidth = rect.width();
    double containerHeight = rect.height();
    
    // Calculate icon positions (centered in each half)
    double halfWidth = containerWidth / 2.0;
    createIconPos = QPointF(halfWidth / 2.0, containerHeight / 2.0);
    selectIconPos = QPointF(halfWidth * 1.5, containerHeight / 2.0);
    
    // Draw dark indicator (sliding container)
    QRectF indicatorRect = getIndicatorRect();
    painter.setBrush(QColor(DARK_GREY));
    painter.drawRoundedRect(indicatorRect, 6, 6);
    
    // Draw icons
    IconManager& iconManager = IconManager::instance();
    
    // Determine icon color based on whether it's under the indicator
    bool createUnderIndicator = indicatorRect.contains(createIconPos);
    bool selectUnderIndicator = indicatorRect.contains(selectIconPos);
    
    // Draw Create icon with appropriate color
    QIcon createIcon = iconManager.loadSvgIconWithColor("resources/icons/create.svg", 
                                                         createUnderIndicator ? Qt::white : Qt::black);
    if (createIcon.isNull()) {
        // Fallback to standard icon
        createIcon = iconManager.getIcon("create");
        if (createIcon.isNull()) {
            createIcon = iconManager.getIcon("add");
        }
    }
    
    QPixmap createPixmap = createIcon.pixmap(iconSize);
    if (createUnderIndicator && createPixmap.isNull()) {
        // If fallback icon, make it white
        createPixmap = createIcon.pixmap(iconSize);
        QPixmap whitePixmap(iconSize);
        whitePixmap.fill(Qt::transparent);
        QPainter iconPainter(&whitePixmap);
        iconPainter.setRenderHint(QPainter::Antialiasing);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        iconPainter.drawPixmap(0, 0, createPixmap);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        iconPainter.fillRect(whitePixmap.rect(), Qt::white);
        iconPainter.end();
        createPixmap = whitePixmap;
    }
    painter.drawPixmap(createIconPos.x() - iconSize.width() / 2.0,
                      createIconPos.y() - iconSize.height() / 2.0,
                      createPixmap);
    
    // Draw Select/Move icon with appropriate color
    QIcon selectIcon = iconManager.loadSvgIconWithColor("resources/icons/select_move.svg",
                                                         selectUnderIndicator ? Qt::white : Qt::black);
    if (selectIcon.isNull()) {
        // Fallback to standard icon
        selectIcon = iconManager.getIcon("select_move");
        if (selectIcon.isNull()) {
            selectIcon = iconManager.getIcon("select");
            if (selectIcon.isNull()) {
                selectIcon = iconManager.getStandardIcon(QStyle::SP_ArrowUp);
            }
        }
    }
    
    QPixmap selectPixmap = selectIcon.pixmap(iconSize);
    if (selectUnderIndicator && selectPixmap.isNull()) {
        // If fallback icon, make it white
        selectPixmap = selectIcon.pixmap(iconSize);
        QPixmap whitePixmap(iconSize);
        whitePixmap.fill(Qt::transparent);
        QPainter iconPainter(&whitePixmap);
        iconPainter.setRenderHint(QPainter::Antialiasing);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        iconPainter.drawPixmap(0, 0, selectPixmap);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        iconPainter.fillRect(whitePixmap.rect(), Qt::white);
        iconPainter.end();
        selectPixmap = whitePixmap;
    }
    painter.drawPixmap(selectIconPos.x() - iconSize.width() / 2.0,
                      selectIconPos.y() - iconSize.height() / 2.0,
                      selectPixmap);
}

void ModeToggleWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        Mode clickedMode = getModeAtPosition(event->pos());
        if (clickedMode != currentMode) {
            setMode(clickedMode);
            
            if (clickedMode == Create) {
                emit createModeSelected();
            } else {
                emit selectModeSelected();
            }
        }
    }
}

void ModeToggleWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateIndicatorPosition();
    update();
}

void ModeToggleWidget::setIndicatorPosition(double position) {
    indicatorPosition = position;
    update();
}

void ModeToggleWidget::updateIndicatorPosition() {
    // Ensure position matches current mode
    indicatorPosition = (currentMode == Create) ? 0.0 : 1.0;
}

QRectF ModeToggleWidget::getIndicatorRect() const {
    QRectF rect = this->rect();
    double containerWidth = rect.width();
    double containerHeight = rect.height();
    double indicatorSize = containerHeight * 0.8;  // 80% of height
    double margin = (containerHeight - indicatorSize) / 2.0;
    
    // Calculate position
    double halfWidth = containerWidth / 2.0;
    
    // Position indicator: 0.0 = left (Create), 1.0 = right (SelectMove)
    double xPos = margin + (indicatorPosition * (halfWidth - margin * 2));
    
    // Add 6 pixels offset to the right when in SelectMove mode
    if (currentMode == SelectMove) {
        xPos += 6.0;
    }
    
    return QRectF(xPos, margin, indicatorSize, indicatorSize);
}

ModeToggleWidget::Mode ModeToggleWidget::getModeAtPosition(const QPointF& pos) const {
    double halfWidth = width() / 2.0;
    return (pos.x() < halfWidth) ? Create : SelectMove;
}

} // namespace ocr_orc

