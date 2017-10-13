#ifndef FADEOUTNOTIFICATION_H
#define FADEOUTNOTIFICATION_H

#include "shared_global.h"

#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class SHAREDSHARED_EXPORT FadeOutNotification : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int duration READ duration WRITE setDuration)

public:
    explicit FadeOutNotification(QWidget *parent = 0);

    int duration() const;

signals:

public slots:

    void start();
    void stop();
    void setDuration(int duration);

private slots:
    void onAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

protected:
    QGraphicsOpacityEffect m_opacityEffect;
    QScopedPointer<QPropertyAnimation> m_fadeOutAnimation;
};

#endif // FADEOUTNOTIFICATION_H
