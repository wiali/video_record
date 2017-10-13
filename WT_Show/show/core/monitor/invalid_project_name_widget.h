#pragma once
#ifndef INVALID_PROJECT_NAME_WIDGET_H
#define INVALID_PROJECT_NAME_WIDGET_H

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace Ui {
class InvalidProjectNameWidget;
}

namespace capture {
namespace monitor {

class InvalidProjectNameWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit InvalidProjectNameWidget(QWidget *parent = 0);
    ~InvalidProjectNameWidget();

    QString text() const;

protected:

    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent* event) override;

public slots:

    void setText(QString text);

private:
    Ui::InvalidProjectNameWidget *ui;

    QGraphicsOpacityEffect m_opacityEffect;
    QScopedPointer<QPropertyAnimation> m_fadeOutAnimation;
};

} // namespace monitor
} // namespace capture

#endif // INVALID_PROJECT_NAME_WIDGET_H
