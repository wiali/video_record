#include "setting_sub_menu_widget.h"
#include "ui_setting_sub_menu_widget.h"

#include <QDebug>
#include <QCheckBox>

#include <global_utilities.h>
#include "common/utilities.h"

namespace capture {
namespace monitor {

SettingSubMenuWidget::SettingSubMenuWidget(QWidget *parent)
    : MenuBase(parent)
    , ui(new Ui::SettingSubMenuWidget)
    , lostFocusByChildren(false)
    , m_timer(new QTimer(this)) {
    ui->setupUi(this);

    this->setContent(ui->frame_content);
    this->setupConnectionButton();
    this->setArrowWidth(15);
    this->setHasCaption(false);
    this->installEventFilter(this);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeToClose()));
    m_timer->setSingleShot(true);
    m_timer->setInterval(20);

    connect(ui->colorCalibrationButton, &QPushButton::clicked, this, &SettingSubMenuWidget::onColorCalibrationButtonClicked);
    connect(ui->keystoneCalibrationButton, &QPushButton::clicked, this, &SettingSubMenuWidget::onKeystoneCalibrationButtonClicked);

    if (!GlobalUtilities::applicationSettings()->value("color_calibration_enabled", false).toBool()) {
        ui->colorCalibrationButton->hide();
        setGeometry(0, 0, width(), height() - 70);
        ui->keystoneCalibrationButton->move(ui->keystoneCalibrationButton->x(), ui->keystoneCalibrationButton->y() - 70);
    }    
}

void SettingSubMenuWidget::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;

    m_countDownTimerButton = new ToggleButton(this, m_model->countDownTimerState());
    m_picInPicButton = new ToggleButton(this, m_model->picInPicMode());

    m_countDownTimerButton->move(ui->countdown_timer_label->geometry().topRight());
    m_picInPicButton->move(ui->pip_label->geometry().topRight());

    connect(m_countDownTimerButton, &ToggleButton::clicked, m_model.data(), &model::ApplicationStateModel::setCountDownTimerState);
    connect(m_picInPicButton, &ToggleButton::clicked, m_model.data(), &model::ApplicationStateModel::setPicInPicMode);
    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &SettingSubMenuWidget::updateButtonsEnabledState);
    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &SettingSubMenuWidget::updateButtonsEnabledState);
    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this, &SettingSubMenuWidget::updateButtonsEnabledState);

    if (!m_model->singleScreenMode()) {
        ui->keystoneCalibrationButton->hide();
        setGeometry(0, 0, width(), height() - 70);
    }
}

void SettingSubMenuWidget::updateButtonsEnabledState() {
    auto liveCapture = m_model->liveCapture();

    const auto areCalibrationButtonsEnabled = m_model->mode() == model::ApplicationStateModel::Mode::LiveCapture &&
                                              liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running &&
                                              liveCapture->selectedVideoStreamSources().count() == 1;

    ui->keystoneCalibrationButton->setEnabled(areCalibrationButtonsEnabled && liveCapture->selectedVideoStreamSources().contains(common::VideoSourceInfo::SproutCamera()));
    ui->colorCalibrationButton->setEnabled(areCalibrationButtonsEnabled &&
                                           (liveCapture->selectedVideoStreamSources().contains(common::VideoSourceInfo::DownwardFacingCamera()) ||
                                            liveCapture->selectedVideoStreamSources().contains(common::VideoSourceInfo::SproutCamera())));
}

void SettingSubMenuWidget::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    setFocus();
    lostFocusByChildren = false;
}

void SettingSubMenuWidget::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    m_timer->start();
}

int SettingSubMenuWidget::xOffsetOfArrow() const {
    return 1;
}

int SettingSubMenuWidget::yOffsetOfArrow() const {
    return 39;
}

bool SettingSubMenuWidget::eventFilter(QObject *watched, QEvent *event) {
    Q_UNUSED(watched);
    switch (event->type()) {
    case QEvent::FocusIn:
        lostFocusByChildren = true;
        break;
    }

    return false;
}

void SettingSubMenuWidget::timeToClose() {
    if (!lostFocusByChildren) {
        fold();
    } else {
        lostFocusByChildren = false;
    }
}

void SettingSubMenuWidget::setupConnectionButton() {
    auto pushButton = qobject_cast<QPushButton*>(m_target);
    auto menuButton = qobject_cast<AbstractMenuButton*>(m_target);

    if (pushButton != nullptr) {
        connect(pushButton, &QPushButton::clicked, this, &SettingSubMenuWidget::onTargetClicked);
    } else if (menuButton != nullptr) {
        connect(menuButton, &AbstractMenuButton::clicked, this, &SettingSubMenuWidget::onTargetClicked);
    }
    else {
        qWarning() << this << "Cannot connect to clicked signal on target" << m_target;
    }
}

void SettingSubMenuWidget::onTargetClicked(bool checked) {
    if(animationRunning()) {
        return;
    }

    if (checked) {
        resize(maximumWidth(), height());
        m_content->move(0, 0);
        mAnimateInWidth->setFoldedState(false);
        unfold();
    } else {
        fold();
    }
}

void SettingSubMenuWidget::onColorCalibrationButtonClicked() {
    m_model->setMode(model::ApplicationStateModel::Mode::ColorCorrectionCalibration);
}

void SettingSubMenuWidget::onKeystoneCalibrationButtonClicked() {
    // Remember current state so we can revert back once we finish calibration
    m_model->keystoneCalibration()->setPreCalibrationCorrectionMode(m_model->liveCapture()->keystoneCorrectionMode());
    m_model->liveCapture()->setKeystoneCorrectionMode(model::LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection);
    m_model->setMode(model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration);
}

} // namespace monitor
} // namespace capture
