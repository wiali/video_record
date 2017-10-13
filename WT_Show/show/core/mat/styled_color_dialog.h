#pragma once
#ifndef STYLEDCOLORDIALOG_H
#define STYLEDCOLORDIALOG_H

#include "ink/ColorPicker/ColorPicker.h"
#include "ink_widget.h"
#include "BorderPainter.h"

namespace capture {
namespace mat {

class StyledColorDialog : public ColorPicker
{
    Q_OBJECT
public:
    explicit StyledColorDialog(QSharedPointer<InkWidget> inkWidget, QWidget *target, QWidget *parent = 0);
    void moveToTarget();
    void setOffsetY(int offsetY);

Q_SIGNALS:
    void positionChanged(bool leftPosition);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void onPositionModeChanged(bool leftPosition);

private:
    int m_offsetY;
    QWidget *m_target;
    QSharedPointer<InkWidget> m_inkWidget;
    QSharedPointer<BorderPainter> m_borderDrawer;
};

} // namespace mat
} // namespace capture

#endif // STYLEDCOLORDIALOG_H
