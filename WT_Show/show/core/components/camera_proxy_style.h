#pragma once
#ifndef CAMERAPROXYSTYLE_H
#define CAMERAPROXYSTYLE_H

#include <QProxyStyle>
#include <QStyleOption>

namespace capture {
namespace components {

class CameraProxyStyle: public QProxyStyle
{
public:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,const QWidget *widget) const;
};

} // namespace components
} // namespace capture

#endif // CAMERAPROXYSTYLE_H
