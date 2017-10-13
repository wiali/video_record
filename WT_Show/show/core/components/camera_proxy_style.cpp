#include "camera_proxy_style.h"

#include <QPainter>

namespace capture {
namespace components {

void CameraProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,const QWidget *widget) const {
    switch(element){
    case PE_IndicatorItemViewItemDrop:
    {
        painter->setRenderHint(QPainter::Antialiasing, true);
        QColor c(0,150,214); //same color used as item selection
        QPen pen(c);
        QBrush brush(c);

        pen.setWidth(3);
        painter->setPen(pen);
        painter->setBrush(brush);

        QPoint myPoint = QPoint(option->rect.topLeft().x(), option->rect.topLeft().y()+2);
        painter->drawLine(myPoint, option->rect.topRight());

    } break;
    default: QProxyStyle::drawPrimitive(element, option,painter,widget);
    }
}

} // namespace components
} // namespace capture
