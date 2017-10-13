#include "styled_color_dialog.h"

#include <QPainter>

namespace capture {
namespace mat {

StyledColorDialog::StyledColorDialog(QSharedPointer<InkWidget> inkWidget, QWidget *target, QWidget *parent)
    : ColorPicker(parent)
    , m_target(target)
    , m_inkWidget(inkWidget)
    , m_offsetY(38)
{
    setObjectName("StyledColorDialog");
    setAutoFillBackground(true);

    m_borderDrawer = QSharedPointer<BorderPainter>(new BorderPainter);
    m_borderDrawer->setArrowOffsetY(m_offsetY);
    m_borderDrawer->setWidget(this);

    connect(this, &StyledColorDialog::positionModeChanged, this, &StyledColorDialog::onPositionModeChanged);

    this->setLayoutDirection(Qt::LeftToRight);
}

void StyledColorDialog::setOffsetY(int offsetY)
{
    m_offsetY = offsetY;
    m_borderDrawer->setArrowOffsetY(m_offsetY);
    m_borderDrawer->setWidget(this);
}

void StyledColorDialog::moveToTarget()
{
    QRect rc = m_target->visibleRegion().boundingRect();
    QPoint inGlobal = m_target->mapToGlobal(rc.topLeft() + QPoint(0, rc.height()/2 - 1));

    QPoint point = parentWidget()->mapFromGlobal(inGlobal);
    int posX = point.x() - width() + 18;
    int posY = point.y() - m_offsetY;

    if (!this->positionMode())
    {
        inGlobal = m_target->mapToGlobal(rc.topRight() + QPoint(0, rc.height()/2 - 1));

        point = parentWidget()->mapFromGlobal(inGlobal);
        posX = point.x() - 16;
        posY = point.y() - m_offsetY;
    }
    move(posX, posY);
}

void StyledColorDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    moveToTarget();
    m_inkWidget->enablePen(false);
}

void StyledColorDialog::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    m_inkWidget->enablePen(true);
}

void StyledColorDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    m_borderDrawer->draw(painter);
}

void StyledColorDialog::onPositionModeChanged(bool leftPosition)
{
    m_borderDrawer->setPosition(leftPosition);
    m_borderDrawer->setWidget(this);
}

} // namespace mat
} // namespace capture

