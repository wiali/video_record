#include "invalid_project_name_widget.h"
#include "ui_invalid_project_name_widget.h"

#include <global_utilities.h>

#include "event/change_invalid_project_name_visibility_event.h"
#include "common/utilities.h"

namespace capture {
namespace monitor {

InvalidProjectNameWidget::InvalidProjectNameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InvalidProjectNameWidget)
{
    ui->setupUi(this);
    setAutoFillBackground(true);

    QCoreApplication::instance()->installEventFilter(this);

    auto settings = GlobalUtilities::applicationSettings("invalid_project_name_notification");

    setVisible(false);

    m_opacityEffect.setParent(this);
    m_opacityEffect.setOpacity(1);

    setGraphicsEffect(&m_opacityEffect);

    m_fadeOutAnimation.reset(new QPropertyAnimation(&m_opacityEffect, "opacity"));
    m_fadeOutAnimation->setDuration(settings->value("tooltip_duration_ms", 2000).toInt());

    m_fadeOutAnimation->setKeyValueAt(0, 1);
    m_fadeOutAnimation->setKeyValueAt(1, 0);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::InQuint);
}

InvalidProjectNameWidget::~InvalidProjectNameWidget()
{
    delete ui;
}

QString InvalidProjectNameWidget::text() const
{
    return ui->messageLabel->text();
}

void InvalidProjectNameWidget::setText(QString text)
{
    ui->messageLabel->setText(text);
}

bool InvalidProjectNameWidget::eventFilter(QObject *obj, QEvent *event)
{
    bool accepted = false;

    if (event->type() == event::ChangeInvalidProjectNameVisibilityEvent::type()) {
        auto showInvalidProjectNameEvent = static_cast<event::ChangeInvalidProjectNameVisibilityEvent*>(event);

        if (showInvalidProjectNameEvent != nullptr) {
            if (showInvalidProjectNameEvent->visible()) {
                show();
                raise();

                qDebug() << this << "Showing at" << showInvalidProjectNameEvent->globalPosition();

                move(this->x(), parentWidget()->mapFromGlobal(showInvalidProjectNameEvent->globalPosition()).y() - 15);

                m_fadeOutAnimation->stop();
                m_fadeOutAnimation->start();
            } else {
                setVisible(false);
            }

            accepted = true;
        }
    }

    return accepted ? accepted : QWidget::eventFilter(obj, event) ;
}

void InvalidProjectNameWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    auto const geometry = ui->pointerLabel->geometry();

    QWidget::paintEvent(event);

    // Draw triangle pointing up
    QPainterPath path;
    path.moveTo(geometry.bottomLeft() + QPoint(0, 1));
    path.lineTo(QPoint(geometry.center().x(), geometry.top()));
    path.lineTo(geometry.bottomRight() + QPoint(0, 1));
    path.lineTo(geometry.bottomLeft() + QPoint(0, 1));

    painter.fillPath(path, ui->messageLabel->palette().background());
}

} // namespace monitor
} // namespace capture
