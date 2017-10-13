#include "fade_out_notification.h"

FadeOutNotification::FadeOutNotification(QWidget *parent)
    : QLabel(parent)
{
    setStyleSheet("QLabel {"
                  "font-family: Segoe UI;"
                  "font-size: 20px;"
                  "font-weight: normal;"
                  "font-style: normal;"
                  "text-align: center center;"
                  "color: white;"
                  "border-radius: 3px;"
                  "}");

    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setMinimumSize(300, 74);

    // Notification needs to be set to invisible otherwise will be catching mouse click events
    // http://doc.qt.io/qt-4.8/qwidget.html#mouseTracking-prop
    setVisible(false);

    m_opacityEffect.setParent(this);
    m_opacityEffect.setOpacity(0);

    setGraphicsEffect(&m_opacityEffect);
    setAutoFillBackground(true);

    m_fadeOutAnimation.reset(new QPropertyAnimation(&m_opacityEffect, "opacity"));

    m_fadeOutAnimation->setKeyValueAt(0, 0.75);
    m_fadeOutAnimation->setKeyValueAt(0.50, 0.5);
    m_fadeOutAnimation->setKeyValueAt(1, 0);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::InQuint);

    connect(m_fadeOutAnimation.data(), &QPropertyAnimation::stateChanged, this, &FadeOutNotification::onAnimationStateChanged);
}

int FadeOutNotification::duration() const
{
    return m_fadeOutAnimation->duration();
}

void FadeOutNotification::setDuration(int duration)
{
    m_fadeOutAnimation->setDuration(duration);
}

void FadeOutNotification::start()
{    
    m_fadeOutAnimation->start();
    setVisible(true);
}

void FadeOutNotification::stop()
{
    m_fadeOutAnimation->stop();
}

void FadeOutNotification::onAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    Q_UNUSED(oldState);

    switch(newState) {
    case QAbstractAnimation::State::Paused:
        break;
    case QAbstractAnimation::State::Running:
        setVisible(true);
        break;
    case QAbstractAnimation::State::Stopped:
        setVisible(false);
        break;
    }
}
