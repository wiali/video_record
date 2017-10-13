#include "color_button.h"

#include <QPainter>

namespace capture {
namespace mat {

ColorButton::ColorButton(QWidget* parent)
    : QWidget(parent)
    , m_checked(false)
    , m_color(QColor("#0096d6"))
{
    this->setMouseTracking(true);
    this->resize(64, 64);
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform |
                           QPainter::Antialiasing |
                           QPainter::HighQualityAntialiasing);

    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QBrush(m_color, Qt::SolidPattern));
    painter.drawEllipse(16, 16, 32, 32);
    painter.end();
}

void ColorButton::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    setChecked(!m_checked);
}

void ColorButton::setColor(const QColor& color)
{
    m_color = color;
    update();
}

void ColorButton::setChecked(bool checked)
{
    if(m_checked != checked)
    {
        qDebug() << "The color button checked statue:" << checked;
        m_checked = checked;
        emit toggled(m_checked);
    }
}

} // namespace mat
} // namespace capture
