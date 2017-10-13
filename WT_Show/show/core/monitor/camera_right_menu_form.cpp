#include "camera_right_menu_form.h"
#include "ui_camera_right_menu_form.h"

#include <global_utilities.h>

#include "help_widget.h"
#include "delete_project_widget.h"

#include "common/utilities.h"
#include "common/history_manager.h"

namespace capture {
namespace monitor {

const int IconHeight = 64;

CameraRightMenuForm::CameraRightMenuForm(QWidget *parent)
    : SharedRightMenuForm(parent)
    , ui(new Ui::CameraRightMenuForm)
    , m_isCaptionOn(false) {
    // have Qt setup the UI Form baed on the .UI file for us
    ui->setupUi(this);
    m_settingSubMenuWidget = QSharedPointer<SettingSubMenuWidget>::create(settingButton());

    auto previewContext = QVariant::fromValue(model::ApplicationStateModel::Mode::Preview).toString();

    addButtonToContext(Top, ui->helpButton, previewContext, IconHeight);
    addButtonToContext(Top, ui->presentationModeButton, previewContext, IconHeight);
    addButtonToContext(Top, ui->undoWidget, previewContext, IconHeight);
    addButtonToContext(Top, ui->redoWidget, previewContext, IconHeight);

    if (GlobalUtilities::applicationSettings()->value("edit_button_enabled", true).toBool()) {
        addButtonToContext(Top, ui->editButton, previewContext, IconHeight);
    } else {
        ui->editButton->hide();
    }

    addButtonToContext(Top, ui->copyToClipboardWidget, previewContext, IconHeight);
    addButtonToContext(Top, ui->sendToStageWidget, previewContext, IconHeight);

    addButtonToContext(Top, ui->saveProjectToImageWidget, previewContext, IconHeight);
    addButtonToContext(Top, ui->deleteProjectWidget, previewContext, IconHeight);

    QVector<model::ApplicationStateModel::Mode> liveCaptureContexts {
        model::ApplicationStateModel::Mode::LiveCapture,
                model::ApplicationStateModel::Mode::CameraFailedToStart,
                model::ApplicationStateModel::Mode::NoCalibrationData,
                model::ApplicationStateModel::Mode::NoVideoSource,
                model::ApplicationStateModel::Mode::ColorCorrectionCalibration,
    };

    for (auto context : liveCaptureContexts) {
        auto contextName = QVariant::fromValue(context).toString();

        addButtonToContext(Top, ui->helpButton, contextName, IconHeight);
        addButtonToContext(Top, ui->presentationModeButton, contextName, IconHeight);
        addButtonToContext(Center, ui->downwardFacingCameraButton, contextName);
        addButtonToContext(Center, ui->forwardFacingCameraButton, contextName);
        addButtonToContext(Center, ui->primaryDesktopButton, contextName);
        addButtonToContext(Center, ui->matDesktopButton, contextName);
    }

    auto keystoneCorrectionContext = QVariant::fromValue(model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration).toString();
    addButtonToContext(Top, ui->helpButton, keystoneCorrectionContext, IconHeight);
    addButtonToContext(Top, ui->presentationModeButton, keystoneCorrectionContext, IconHeight);

    ui->downwardFacingCameraButton->setIconName("icon-downcam");
    ui->downwardFacingCameraButton->setText(tr("Down cam"));

    ui->forwardFacingCameraButton->setText(tr("Front cam"));
    ui->forwardFacingCameraButton->setIconName("icon-frontcam");

    ui->primaryDesktopButton->setText(tr("Vertical screen"));
    ui->primaryDesktopButton->setIconName("icon-vertical-screen");

    ui->matDesktopButton->setText(tr("Mat screen"));
    ui->matDesktopButton->setIconName("icon-mat-screen");

    connect(parent->window(), SIGNAL(monitorWindowResized()), this, SIGNAL(positionChanged()));
    //Below connect is used for notifying size change to Crop, parent-window() is monitor window,
    //don't find a way to use stageViewer directly
    connect(parent->window(), SIGNAL(stageViewerSizeChanged(QSize)), this, SLOT(onStageViewerSizeChanged(QSize)));
    auto historyManager = common::Utilities::getHistoryManager();
    connect(historyManager.data(), &common::HistoryManager::historyChanged, this, &CameraRightMenuForm::updateEditButtonState);
}

void CameraRightMenuForm::setupAnimation() {
    // We need this method to give access to protected member
    SharedRightMenuForm::setupAnimation();
}

void CameraRightMenuForm::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;
    m_settingSubMenuWidget->setModel(m_model);
    connect(m_model.data(), &model::ApplicationStateModel::selectedProjectChanged, this, &CameraRightMenuForm::onSelectedProjectChanged, Qt::UniqueConnection);

    QVector<common::VideoSourceInfo> pipelineTypes;
    pipelineTypes << common::VideoSourceInfo::DownwardFacingCamera();
    pipelineTypes << common::VideoSourceInfo::SproutCamera();

    ui->downwardFacingCameraButton->setModel(pipelineTypes, model);
    ui->forwardFacingCameraButton->setModel(model);
    ui->primaryDesktopButton->setModel(common::VideoSourceInfo::PrimaryDesktop(), model);
    ui->matDesktopButton->setModel(common::VideoSourceInfo::MatDesktop(), model);

    ui->flashModeToggleButton->setModel(model);
    ui->captureButton->setModel(model);
    ui->captureModeWidget->setModel(model);
    ui->copyToClipboardWidget->setModel(model);
    ui->saveProjectToImageWidget->setModel(model);
    ui->deleteProjectWidget->setModel(model);
    ui->presentationModeButton->setModel(model);
    ui->sendToStageWidget->setModel(model);
    ui->undoWidget->setModel(model);
    ui->redoWidget->setModel(model);
    ui->colorCorrectionToggleButton->setModel(model);
    ui->keystoneCorrectionModeButton->setModel(model);
    ui->acceptKeystoneCalibrationButton->setModel(model);
    ui->rejectKeystoneCalibrationButton->setModel(model);

    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &CameraRightMenuForm::onApplicationModeChanged);
    connect(m_model.data(), &model::ApplicationStateModel::editButtonChanged, this, &CameraRightMenuForm::onEditButtonChanged);
    onApplicationModeChanged(model->mode());
    connect(m_model.data(), &model::ApplicationStateModel::inkWidgetChanged, this, &CameraRightMenuForm::updateEditButtonState);
}

void CameraRightMenuForm::onApplicationModeChanged(model::ApplicationStateModel::Mode mode) {
    auto modeName = QVariant::fromValue(mode).toString();
    switchToContext(modeName);

    removeAllBottomItems();

    switch(mode) {
    case model::ApplicationStateModel::Mode::None:
        // No action
        break;
    case model::ApplicationStateModel::Mode::LiveCapture:
    case model::ApplicationStateModel::Mode::CameraFailedToStart:
    case model::ApplicationStateModel::Mode::NoCalibrationData:
    case model::ApplicationStateModel::Mode::NoVideoSource:
    case model::ApplicationStateModel::Mode::ColorCorrectionCalibration:
        if (m_model->singleScreenMode()) {
            addItemToBottom(ui->keystoneCorrectionModeButton);
        }

        ui->flashModeToggleButton->show();
        ui->colorCorrectionToggleButton->show();
        ui->captureButton->show();
        if(ui->editButton->isChecked()) {
            ui->editButton->click();
        }

        addItemToBottom(ui->colorCorrectionToggleButton);
        addItemToBottom(ui->flashModeToggleButton);
        addItemToBottom(ui->captureModeWidget);
        addItemToBottom(ui->captureButton);

        ui->acceptKeystoneCalibrationButton->hide();
        ui->rejectKeystoneCalibrationButton->hide();

        break;
    case model::ApplicationStateModel::Mode::Preview:
        // I don't know why removeAllBottomItems method didn't remove flash button. So I hide it.
        ui->flashModeToggleButton->hide();
        ui->colorCorrectionToggleButton->hide();
        ui->captureButton->hide();
        ui->keystoneCorrectionModeButton->hide();
        ui->acceptKeystoneCalibrationButton->hide();
        ui->rejectKeystoneCalibrationButton->hide();

        addItemToBottom(ui->captureModeWidget);
        break;
    case model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration:
        ui->flashModeToggleButton->hide();
        ui->colorCorrectionToggleButton->hide();
        ui->captureButton->hide();        

        ui->acceptKeystoneCalibrationButton->show();
        ui->rejectKeystoneCalibrationButton->show();

        if (m_model->singleScreenMode()) {
            ui->keystoneCorrectionModeButton->show();
            addItemToBottom(ui->keystoneCorrectionModeButton);
        }

        addItemToBottom(ui->acceptKeystoneCalibrationButton);
        addItemToBottom(ui->rejectKeystoneCalibrationButton);
        break;
    default:
        Q_UNREACHABLE();
    }
}

void CameraRightMenuForm::onSelectedProjectChanged() {
    if (m_model->selectedProject() && GlobalUtilities::applicationSettings()->value("edit_button_enabled", true).toBool()) {
        disconnect(m_inkAddConnection);
        disconnect(m_inkRemoveConnection);

        ui->editButton->setItem(m_model->selectedProject()->items().first());
        QSharedPointer<InkData> inkData = m_model->selectedProject()->inkData();
        m_inkAddConnection = connect(inkData.data(), &InkData::strokeAdded, this, &CameraRightMenuForm::updateEditButtonState);
        m_inkRemoveConnection = connect(inkData.data(), &InkData::strokeRemoved, this, &CameraRightMenuForm::updateEditButtonState);
        updateEditButtonState();
        onEditButtonChanged(false);
    }
}

void CameraRightMenuForm::updateEditButtonState() {
    if(GlobalUtilities::applicationSettings()->value("edit_button_disable_by_ink", false).toBool()) {
        if(auto inkData = m_model->selectedProject()->inkData()) {
            ui->editButton->setEnabled(inkData->strokeCount() == 0);
        }
    }

    ui->editButton->updateState();
}

void CameraRightMenuForm::setEditTargetWidget(QPointer<QWidget> targetWidget) {
    ui->editButton->setTargetWindow(targetWidget);
}

void CameraRightMenuForm::on_button_roller_clicked(bool checked) {
    m_settingSubMenuWidget->hide();
    if (!ui->editButton->hasMenuAnimationRunning()) {
        m_isCaptionOn = !m_isCaptionOn;
        ui->editButton->notifyMenusCaptionState(m_isCaptionOn);

        if (m_isCaptionOn) {
            if(!ui->editButton->isMenuOn() && checked) {
                roll();
            }
        } else {
            if(!checked) {
                unroll();
            }
        }
        setButtonMoreChecked(m_isCaptionOn);
    }
}

void CameraRightMenuForm::on_editButton_clicked(bool checked) {
    if (checked) {
        unroll();
    } else {
        if(m_isCaptionOn) {
            roll();
        }
    }
}

void CameraRightMenuForm::onEditModeChanged(bool checked) {
    m_model->setEditMode(checked ?
        model::ApplicationStateModel::EditMenuMode::SubMenuOpen :
        model::ApplicationStateModel::EditMenuMode::SubMenuClose);
    setButtonMoreEnabled(!checked);
}

void CameraRightMenuForm::onEditButtonChanged(bool checked) {
    if(ui->editButton->isChecked()) {
        if(!checked) {
            ui->editButton->click();
        }
    } else {
        if(checked) {
            ui->editButton->click();
        }
    }
}

void CameraRightMenuForm::onMenuClosedChanged(bool isClosed) {
    m_model->setEditMode(isClosed ?
                             model::ApplicationStateModel::EditMenuMode::MainMenuClose :
                             model::ApplicationStateModel::EditMenuMode::MainMenuOpen);
}

void CameraRightMenuForm::on_undoWidget_clicked()
{
    //QTimer::singleShot(10, this, [this]{
    //    onEditButtonChanged(false);
    //});
}

void CameraRightMenuForm::on_redoWidget_clicked()
{
    //QTimer::singleShot(10, this, [this]{
    //    onEditButtonChanged(false);
    //});
}

void CameraRightMenuForm::onStageViewerSizeChanged(QSize size)
{
    emit ui->editButton->targetWindowSizeChanged(size);
}

} // namespace monitor
} // namespace capture
