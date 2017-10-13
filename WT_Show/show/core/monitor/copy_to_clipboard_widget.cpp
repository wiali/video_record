#include "copy_to_clipboard_widget.h"

#include <QDebug>

#include "event/export_project_items_event.h"

namespace capture {
namespace monitor {

CopyToClipboardWidget::CopyToClipboardWidget(QWidget *parent) :
    RightMenuButton(parent)
{
    setIconName("icon-copy");
    setText(tr("Copy"));

    connect(this, &RightMenuButton::clicked, this, &CopyToClipboardWidget::onButtonClicked);
}

void CopyToClipboardWidget::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
}

void CopyToClipboardWidget::onButtonClicked()
{
    auto selectedProject = m_model->selectedProject();
    Q_ASSERT(selectedProject != nullptr);

    if (selectedProject)
    {
        emit m_model->editButtonChanged(false);
        QVector<QSharedPointer<model::ExportImageModel>> exportItems;
        exportItems << model::ExportImageModel::fromStageProject(selectedProject);

        auto projectsExport = m_model->projectsExport();
        if(projectsExport->state() == model::ProjectsExportModel::State::NotExporting)
        {
            auto exportEvent = new event::ExportProjectItemsEvent(event::ExportProjectItemsEvent::Clipboard, QString(), exportItems);
            exportEvent->dispatch();
        }
    }
    else
    {
        qWarning() << this << "Copy to clipboard button clicked but no project is selected (?)";
    }
}

} // namespace monitor
} // namespace capture
