#pragma once
#ifndef COLOR_BUTTON_H
#define COLOR_BUTTON_H

#include <QWidget>

namespace capture {
namespace mat {

class ColorButton : public QWidget
{
    Q_OBJECT

public:
    explicit ColorButton(QWidget* parent = 0);

public slots:
    void setColor(const QColor &color);
    void setChecked(bool checked);

protected slots:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void toggled(bool checked);

private:
    bool m_checked;
    QColor m_color;
};

} // namespace mat
} // namespace capture

#endif // COLOR_BUTTON_H
