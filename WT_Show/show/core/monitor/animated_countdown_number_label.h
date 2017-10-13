#pragma once
#ifndef ANIMATEDCOUNTDOWNNUMBERLABEL_H
#define ANIMATEDCOUNTDOWNNUMBERLABEL_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>

namespace capture {
namespace monitor {

class AnimatedCountDownNumberLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AnimatedCountDownNumberLabel(QWidget *parent = 0);

    QPropertyAnimation* fadeOutAnimation() const;
    QPropertyAnimation* moveAnimation() const;

private:

    QPropertyAnimation* m_fadeOutAnimation;
    QPropertyAnimation* m_moveAnimation;
    QGraphicsOpacityEffect* m_opacityEffect;

private slots:

    void onFadeOutAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void onMoveAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
};

} // namespace monitor
} // namespace capture

#endif // ANIMATEDCOUNTDOWNNUMBERLABEL_H
