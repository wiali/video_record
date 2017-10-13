#include "send_to_stage_widget.h"

#include <global_utilities.h>

#include "event/export_projects_event.h"

namespace capture {
namespace monitor {

SendToStageWidget::SendToStageWidget(QWidget *parent)
    : RightMenuButton(parent) {
    setIconName("icon-sendtostage");
    setText(tr("To Stage"));

    connect(this, &RightMenuButton::clicked, this, &SendToStageWidget::onButtonClicked);
}

void SendToStageWidget::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;
    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &SendToStageWidget::onModeChange);
}

void SendToStageWidget::onButtonClicked() {
    auto selectedProject = m_model->selectedProject();

    if (selectedProject) {
        emit m_model->editButtonChanged(false);
        QVector<QSharedPointer<StageProject>> items;
        items << selectedProject;

        auto exportEvent = new event::ExportProjectsEvent(items);
        exportEvent->dispatch();
    }
}

void SendToStageWidget::onModeChange() {
    if(m_model->mode() == model::ApplicationStateModel::Mode::Preview &&
       m_model->sendToStageMode() && m_model->sendToStageModeCapture()) {

        m_model->setSendToStageMode(false);
        QTimer::singleShot(GlobalUtilities::applicationSettings()->value("send_to_stage_delay", 100).toInt(), this, &SendToStageWidget::onButtonClicked);
    }
    m_model->setSendToStageModeCapture(false);
}

} // namespace monitor
} // namespace capture
