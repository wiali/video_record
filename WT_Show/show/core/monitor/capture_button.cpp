#include "capture_button.h"

#include <QPainter>
#include <QDebug>
#include <QSequentialAnimationGroup>

#include <global_utilities.h>

#include "common/utilities.h"
#include "styled_message_box.h"
#include "event/capture_frame_event.h"
#include "event/prepare_frame_capture_event.h"
#include "global_utilities.h"

namespace capture {
namespace monitor {

CaptureButton::CaptureButton(QWidget *parent)
    : RightMenuButton(parent)
    , m_captureAnimationGroup(new QParallelAnimationGroup(this))
    , m_spanAngle(0)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setText(tr("Capture"));
    setIconName("icon-capture");
}

void CaptureButton::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
    setAnimation();

    auto liveCapture = m_model->liveCapture();

    connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &CaptureButton::onCameraStateChanged);
    connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this, &CaptureButton::onCameraStateChanged);
    connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this, &CaptureButton::onCameraStateChanged);
    connect(m_model.data(), &model::ApplicationStateModel::countDownTimerStateChanged, this, &CaptureButton::onCountDownTimerStateChanged);

    onCountDownTimerStateChanged();
    onCameraStateChanged();
}

void CaptureButton::setAnimation()
{
    connect(this, &RightMenuButton::clicked, this, &CaptureButton::onButtonClicked);

    auto settings = GlobalUtilities::applicationSettings("capture_button");

    auto countdownAnimation = new QPropertyAnimation(this, "spanAngle", this);
    countdownAnimation->setDuration(settings->value("countdown_delay_ms", 2000).toInt());
    countdownAnimation->setStartValue(0);
    countdownAnimation->setEndValue(-360);

    connect(countdownAnimation, &QPropertyAnimation::finished, this, &CaptureButton::onCountdownAnimationFinished);

    m_backgroundColor = QColor(settings->value("background_color", "#0096D6").toString());
    m_countdownIndicatorColor = QColor(settings->value("countdown_indicator_color", "#005479").toString());

    auto countdownLabelCount = settings->value("countdown_label_count", 3).toInt();
    auto countdownLabelDuration = countdownAnimation->duration() / countdownLabelCount;

    for (int i=0; i< countdownLabelCount; i++)
    {
        m_numberLabels << new AnimatedCountDownNumberLabel(this);

        m_numberLabels.last()->moveAnimation()->setDuration(countdownLabelDuration);
        m_numberLabels.last()->fadeOutAnimation()->setDuration(countdownLabelDuration);
        m_numberLabels.last()->setText(QString::number(countdownLabelCount - i));
        m_numberLabels.last()->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        m_numberLabels.last()->setStyleSheet("font-size: 34px; color: white; background: transparent; font-family: Segoe UI;");
    }

    auto numbersAnimation = new QSequentialAnimationGroup(this);

    for (int i=0; i < countdownLabelCount; i++)
    {
        auto numberAnimation = new QParallelAnimationGroup(numbersAnimation);
        numberAnimation->addAnimation(m_numberLabels[i]->fadeOutAnimation());

        if (i < (countdownLabelCount - 1))
        {
            numberAnimation->addAnimation(m_numberLabels[i+1]->moveAnimation());
        }

        numbersAnimation->addAnimation(numberAnimation);
    }

    m_captureAnimationGroup->addAnimation(numbersAnimation);
    m_captureAnimationGroup->addAnimation(countdownAnimation);
}

void CaptureButton::onCameraStateChanged()
{
    auto liveCapture = m_model->liveCapture();

    m_enabled = liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running &&
            liveCapture->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing &&
            !m_model->isInTransitionalMatMode();

    setEnabled(m_enabled);
    if (m_enabled)
    {
        m_backgroundColor = QColor("#0096D6");
    }
    else
    {
        m_backgroundColor = QColor("#2f3438");
    }
}

int CaptureButton::spanAngle() const
{
    return m_spanAngle;
}

void CaptureButton::setSpanAngle(int spanAngle)
{
    if (m_spanAngle != spanAngle)
    {
        m_spanAngle = spanAngle;

        repaint(rect());
        update();
    }
}

bool CaptureButton::captureWithFlash()
{
    auto liveCapture = m_model->liveCapture();

    // Flash mode is always off in Desktop mode
    auto flashMode = (m_model->matModeState() == model::ApplicationStateModel::MatModeState::Desktop) ? false : liveCapture->flashMode();
    // And only for downward facing camera (SPROUT-17629)
    return liveCapture->supportsDepthCapture() ? flashMode : false;
}

void CaptureButton::onButtonClicked()
{
    auto settings = GlobalUtilities::applicationSettings("capture");
    const auto captureCountCap = settings->value("count_cap", 50).toInt();

    m_model->setSendToStageModeCapture(true);
    if (m_model->projects()->count() >= captureCountCap) {
        auto msgBox = common::Utilities::createMessageBox();

        msgBox->setText(tr("Maximum capture count reached"));
        msgBox->setInformativeText(QString(tr("You have exceeded the amount of captures that you can take.\nThe limit is %1 captures.")).arg(m_model->projects()->count()));
        msgBox->addStyledButton(tr("OK"), QMessageBox::YesRole);
        msgBox->exec();
    } else {
        auto prepareCaptureEvent = new event::PrepareFrameCaptureEvent(captureWithFlash(), m_model->liveCapture()->selectedVideoStreamSources());
        prepareCaptureEvent->dispatch();

        foreach (auto label, m_numberLabels)
        {
            label->setGeometry(iconGeometry());

            qDebug() << this << iconGeometry();

            label->moveAnimation()->setStartValue(iconGeometry().bottomLeft());
            label->moveAnimation()->setEndValue(iconGeometry().topLeft());
        }

        setIconVisible(false);
        if(m_model->countDownTimerState())  {
            m_captureAnimationGroup->start();
        } else {
            performCapture(true);
        }
    }
}

void CaptureButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    const auto center = iconGeometry().center();

    if (m_spanAngle != 0)
    {
        int startAngle = 90 * 16;
        int spanAngle = m_spanAngle * 16;

        QBrush brush(m_countdownIndicatorColor);
        QPen pen(m_countdownIndicatorColor);

        const auto width = iconGeometry().width() - 1;

        // Draw countdown indicator
        painter.setBrush(brush);
        painter.setPen(pen);
        painter.drawPie(QRect(center.x() - width / 2, center.y() - width / 2, width, width), startAngle, spanAngle);
    }

    const auto radius = static_cast<int>(0.375f * static_cast<float>(iconGeometry().height()));

    // Draw circular background
    painter.setBrush(QBrush(m_backgroundColor));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, radius, radius);

    // Draw rest of things
    RightMenuButton::paintEvent(event);
}

QSharedPointer<model::VideoStreamSourceModel> CaptureButton::runningCamera() {
    QSharedPointer<model::VideoStreamSourceModel> camera;
    auto liveCapture = m_model->liveCapture();

    // ToDo! Define which sources are allowed to take viewport and in which order
    for(auto videoStreamSource : liveCapture->videoStreamSources()) {
        camera = liveCapture->selectedVideoStreamSources().contains(videoStreamSource->videoSource()) ? videoStreamSource : camera;
    }

    return camera;
}

void CaptureButton::performCapture(bool captureNextFrame) {
    common::Utilities::playSound("qrc:/Resources/production/Sounds/capture.wav");

    setIconVisible(true);

    setSpanAngle(0);
    auto liveCapture = m_model->liveCapture();

    // InkData is not displayed in Desktop
    auto inkData = m_model->matModeState() == model::ApplicationStateModel::MatModeState::Desktop ?
                   QSharedPointer<InkData>::create() : liveCapture->inkData();

    auto colorCorrection = liveCapture->autoFix() ?
                           liveCapture->fullscreenVideoStreamModel()->colorCorrectionMode() : model::VideoStreamSourceModel::ColorCorrectionMode::None;

    auto captureEvent = new event::CaptureFrameEvent(inkData, m_model->liveCapture()->viewport(), captureWithFlash(), captureNextFrame,
                                                     liveCapture->selectedVideoStreamSources(), colorCorrection);
    captureEvent->dispatch();
}

void CaptureButton::onCountdownAnimationFinished() {
    qDebug() << "animation finished";
    performCapture(false);
}

void CaptureButton::enterEvent(QEvent *event)
{
    if (m_enabled)
    {
        m_backgroundColor = QColor("#2EA8DD");
    }
    RightMenuButton::enterEvent(event);
}

void CaptureButton::leaveEvent(QEvent *event)
{
    if (m_enabled)
    {
        m_backgroundColor = QColor("#0096D6");
    }
    RightMenuButton::leaveEvent(event);
}

void CaptureButton::onCountDownTimerStateChanged()
{
    if(m_model->countDownTimerState())
    {
        setIconName("icon-shutter");
    }
    else
    {
        setIconName("icon-capture");
    }
}

} // namespace monitor
} // namespace capture
