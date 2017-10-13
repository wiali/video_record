#pragma once
#ifndef CAPTUREBUTTON_H
#define CAPTUREBUTTON_H

#include <QWidget>
#include <QPropertyAnimation>

#include <right_menu_button.h>

#include "animated_countdown_number_label.h"

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class CaptureButton : public RightMenuButton
{
    Q_OBJECT
    Q_PROPERTY(int spanAngle READ spanAngle WRITE setSpanAngle)

public:
    explicit CaptureButton(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);
    int spanAngle() const;

protected:
    virtual void paintEvent(QPaintEvent* event);

public slots:

    void setSpanAngle(int spanAngle);

private:
    QSharedPointer<model::ApplicationStateModel> m_model;
    QVector<AnimatedCountDownNumberLabel*> m_numberLabels;
    QSharedPointer<QParallelAnimationGroup> m_captureAnimationGroup;
    QSharedPointer<model::VideoStreamSourceModel> runningCamera();
    bool captureWithFlash();
    void performCapture(bool captureNextFrame);

    int m_spanAngle;
    QColor m_countdownIndicatorColor;
    QColor m_backgroundColor;
    bool m_enabled;

private slots:
    void onCameraStateChanged();
    void onCountdownAnimationFinished();
    void onButtonClicked();
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void setAnimation();
    void onCountDownTimerStateChanged();
};

} // namespace monitor
} // namespace capture

#endif // CAPTUREBUTTON_H
