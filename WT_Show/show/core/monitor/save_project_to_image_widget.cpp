#include "save_project_to_image_widget.h"

#include <QFileInfo>
#include <QDir>

#include <global_utilities.h>

#include "common/utilities.h"
#include "model/export_image_model.h"
#include "event/export_project_items_event.h"

namespace capture {
namespace monitor {

SaveProjectToImageWidget::SaveProjectToImageWidget(QWidget *parent)
    : RightMenuButton(parent)
{
    setIconName("icon-save as");
    setText(tr("Save"));

    connect(this, &RightMenuButton::clicked, this, &SaveProjectToImageWidget::onButtonClicked);
}

void SaveProjectToImageWidget::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
}

void SaveProjectToImageWidget::onButtonClicked()
{
    auto selectedProject = m_model->selectedProject();
    Q_ASSERT(selectedProject != nullptr);

    if (selectedProject)
    {
        emit m_model->editButtonChanged(false);
        QString originalName = selectedProject->name();
        QString selectedFilter;

        const auto isOcrEnabled = GlobalUtilities::applicationSettings("ocr")->value("is_enabled", false).toBool();
        QString filter = tr("PNG Files(*.png);;JPG Files(*.jpg);;PDF Files(*.pdf)");
        const auto ocrFilterName = tr("PDF Files with optical character recognition(*.pdf)");

        filter += isOcrEnabled ? QString(";;%1").arg(ocrFilterName) : QString();

        QString fileName = common::Utilities::saveDialog(common::Utilities::Image, tr("Save as"), true, filter, &selectedFilter, originalName);
        QFileInfo info(fileName);

        auto exportImageModel = model::ExportImageModel::fromStageProject(selectedProject);
        exportImageModel->setName(info.completeBaseName());

        QVector<QSharedPointer<model::ExportImageModel>> items;
        items << exportImageModel;

        event::ExportProjectItemsEvent::Format format;

        QString suffix = info.suffix().toLower();
        if(suffix == "png")
        {
            format = event::ExportProjectItemsEvent::Format::PNG;
            auto exportImageEvent = new event::ExportProjectItemsEvent(format, info.dir().absolutePath(), items, true);
            exportImageEvent->dispatch();
        }
        else if(suffix == "jpg")
        {
            format = event::ExportProjectItemsEvent::Format::JPG;
            auto exportImageEvent = new event::ExportProjectItemsEvent(format, info.dir().absolutePath(), items, true);
            exportImageEvent->dispatch();
        }
        else if(suffix == "pdf")
        {
            format = selectedFilter == ocrFilterName ? event::ExportProjectItemsEvent::Format::OCR : event::ExportProjectItemsEvent::Format::PDF;
            auto exportImageEvent = new event::ExportProjectItemsEvent(format, fileName, items, true);
            exportImageEvent->dispatch();
        }
        else
        {
            qCritical() << "Export file is wrong!" << fileName;
        }
    }
    else
    {
        qCritical() << this << "Save project clicked but no project is selected";
    }
}

} // namespace monitor
} // namespace capture
