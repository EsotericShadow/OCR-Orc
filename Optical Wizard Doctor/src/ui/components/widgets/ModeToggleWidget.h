#ifndef MODE_TOGGLE_WIDGET_H
#define MODE_TOGGLE_WIDGET_H

#include <QtWidgets/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QEasingCurve>

namespace ocr_orc {

/**
 * @brief Custom toggle slider widget for mode selection (Create/Select-Move)
 * 
 * Displays two icons in a container with a sliding dark indicator that highlights
 * the selected mode. The indicator slides smoothly between positions.
 */
class ModeToggleWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double indicatorPosition READ getIndicatorPosition WRITE setIndicatorPosition)

public:
    enum Mode {
        Create = 0,
        SelectMove = 1
    };

    explicit ModeToggleWidget(QWidget *parent = nullptr);
    ~ModeToggleWidget() = default;

    /**
     * @brief Get current mode
     * @return Current mode (Create or SelectMove)
     */
    Mode getMode() const { return currentMode; }

    /**
     * @brief Set current mode
     * @param mode Mode to set
     */
    void setMode(Mode mode);

signals:
    /**
     * @brief Emitted when create mode is selected
     */
    void createModeSelected();
    
    /**
     * @brief Emitted when select/move mode is selected
     */
    void selectModeSelected();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    double getIndicatorPosition() const { return indicatorPosition; }
    void setIndicatorPosition(double position);

    void updateIndicatorPosition();
    QRectF getIndicatorRect() const;
    Mode getModeAtPosition(const QPointF& pos) const;

    Mode currentMode;
    double indicatorPosition;  // 0.0 for Create, 1.0 for SelectMove
    QPropertyAnimation* animation;
    
    // Icon sizes and positions
    QSize iconSize;
    QPointF createIconPos;
    QPointF selectIconPos;
    
    // Colors
    static constexpr int LIGHT_GREY = 0xE0E0E0;
    static constexpr int DARK_GREY = 0x555555;
    static constexpr int ICON_MARGIN = 8;
};

} // namespace ocr_orc

#endif // MODE_TOGGLE_WIDGET_H

