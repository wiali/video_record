#include "presentation_mode_button.h"

#include <QGuiApplication>

#include <global_utilities.h>

namespace capture {
namespace monitor {

PresentationModeButton::PresentationModeButton(QWidget *parent)
    : RightMenuButton(parent)
    , m_eventHandler(new user_event_handler::EventHandler)
{
    setIconName("icon-present");
    setCheckable(true);

    connect(m_eventHandler.data(), &user_event_handler::EventHandler::displayCountChanged, this, &PresentationModeButton::updatePresentButtonState);
    connect(this, &RightMenuButton::clicked, this, &PresentationModeButton::onButtonClicked);
}

void PresentationModeButton::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;

    connect(m_model.data(), &model::ApplicationStateModel::mainWindowLocationChanged, this, &PresentationModeButton::updatePresentButtonState);
    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &PresentationModeButton::updatePresentButtonState);
    connect(m_model.data(), &model::ApplicationStateModel::presentationModeChanged, this, &PresentationModeButton::onPresentationModeChanged);

    onPresentationModeChanged(m_model->presentationMode());
    updatePresentButtonState();
}

void PresentationModeButton::onButtonClicked()
{
    m_model->setPresentationMode(!m_model->presentationMode()
        && m_model->mainWindowLocation() != model::ApplicationStateModel::MainWindowLocation::MonitorOnExtend);
}

void PresentationModeButton::onPresentationModeChanged(bool presentationMode)
{
    if(presentationMode)
    {
        setText(tr("Present on"));
        setChecked(true);
    }
    else
    {
        setText(tr("Present off"));
        setChecked(false);
    }
}

void PresentationModeButton::updatePresentButtonState()
{
    bool modeEnable = m_model->mode() == model::ApplicationStateModel::Mode::LiveCapture ||
            m_model->mode() == model::ApplicationStateModel::Mode::Preview;
    bool locationEnable = m_model->mainWindowLocation() != model::ApplicationStateModel::MainWindowLocation::MonitorOnExtend;
    bool presentEnable = !GlobalUtilities::findScreenGeometry(GlobalUtilities::PresentScreen).isNull();

    if(modeEnable && locationEnable && presentEnable)
    {
        this->setEnabled(true);
    }
    else
    {
        this->setEnabled(false);
        m_model->setPresentationMode(false);
    }
}

} // namespace monitor
} // namespace capture
