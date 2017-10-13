#include "animated_countdown_number_label.h"

#include <QDebug>

namespace capture {
namespace monitor {

AnimatedCountDownNumberLabel::AnimatedCountDownNumberLabel(QWidget *parent)
    : QLabel(parent)    
    , m_fadeOutAnimation(new QPropertyAnimation(this))
    , m_moveAnimation(new QPropertyAnimation(this))
    , m_opacityEffect(new QGraphicsOpacityEffect(this))
{
    setVisible(false);
    setAutoFillBackground(true);

    m_opacityEffect->setOpacity(1);

    setGraphicsEffect(m_opacityEffect);

    m_fadeOutAnimation->setTargetObject(m_opacityEffect);
    m_fadeOutAnimation->setPropertyName("opacity");

    m_fadeOutAnimation->setDuration(1000);
    m_fadeOutAnimation->setStartValue(1);
    m_fadeOutAnimation->setEndValue(0);
    m_fadeOutAnimation->setKeyValueAt(0.5, 1);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::InQuint);

    m_moveAnimation->setTargetObject(this);
    m_moveAnimation->setPropertyName("pos");

    m_moveAnimation->setDuration(1000);
    m_moveAnimation->setEasingCurve(QEasingCurve::InQuint);

    connect(m_fadeOutAnimation, &QPropertyAnimation::stateChanged, this, &AnimatedCountDownNumberLabel::onFadeOutAnimationStateChanged);
    connect(m_moveAnimation, &QPropertyAnimation::stateChanged, this, &AnimatedCountDownNumberLabel::onMoveAnimationStateChanged);
}

QPropertyAnimation* AnimatedCountDownNumberLabel::fadeOutAnimation() const
{
    return m_fadeOutAnimation;
}

QPropertyAnimation* AnimatedCountDownNumberLabel::moveAnimation() const
{
    return m_moveAnimation;
}

void AnimatedCountDownNumberLabel::onFadeOutAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    Q_UNUSED(oldState);

    qDebug() << this << "fadeout" << newState;

    switch(newState) {
    case QAbstractAnimation::State::Running:
        setVisible(true);
        break;
    case QAbstractAnimation::State::Stopped:
        setVisible(false);
        break;
    }
}

void AnimatedCountDownNumberLabel::onMoveAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    Q_UNUSED(oldState);

    qDebug() << this << "move" << newState;

    if (newState == QAbstractAnimation::State::Running)
    {
        //setVisible(true);
    }
}

} // namespace monitor
} // namespace capture
