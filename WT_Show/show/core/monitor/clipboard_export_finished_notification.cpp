#include "clipboard_export_finished_notification.h"

#include <global_utilities.h>

namespace capture {
namespace monitor {

ClipboardExportFinishedNotification::ClipboardExportFinishedNotification(QWidget *parent)
    : FadeOutNotification(parent)
{
    auto settings = GlobalUtilities::applicationSettings("clipboard_finished_notification");
    setDuration(settings->value("fade_out_timeout_ms", 3000).toInt());

    setText(tr("Copied successfully."));
}

void ClipboardExportFinishedNotification::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;

    connect(model->projectsExport().data(), &model::ProjectsExportModel::stateChanged, this, &ClipboardExportFinishedNotification::onProjectsExportStateChanged);
}

void ClipboardExportFinishedNotification::onProjectsExportStateChanged (model::ProjectsExportModel::State state)
{
    auto projectsExport = m_model->projectsExport();

    if (state == model::ProjectsExportModel::State::Exporting &&
        projectsExport->format() == model::ProjectsExportModel::Format::Clipboard)
    {
        m_fadeOutAnimation->stop();
        m_fadeOutAnimation->start();
    }
}

} // namespace monitor
} // namespace capture
