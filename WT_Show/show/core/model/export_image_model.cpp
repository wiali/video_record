#include "export_image_model.h"

namespace capture {
namespace model {

ExportImageModel::ExportImageModel(QString name, QSharedPointer<StageItem> item, QSharedPointer<model::CameraItemMetadata> metadata,QSharedPointer<InkData> inkdata, QObject *parent)
    : QObject(parent)    
    , m_item(item)
    , m_metadata(metadata)
    , m_name(name)
    , m_inkData(inkdata)
{
}

QSharedPointer<model::CameraItemMetadata> model::ExportImageModel::metadata() const
{
    return m_metadata;
}

QSharedPointer<StageItem> model::ExportImageModel::item() const
{
    return m_item;
}

QString model::ExportImageModel::name() const
{
    return m_name;
}

QSharedPointer<InkData> model::ExportImageModel::inkData() const
{
    return m_inkData;
}

QSharedPointer<model::ExportImageModel> model::ExportImageModel::fromStageProject(QSharedPointer<StageProject> project)
{
    qDebug() << "%%%%%% From stage project %%%%";
    Q_ASSERT(project);
    Q_ASSERT(project->items().count() == 1);

    auto item = project->items().first();
    auto cameraMetadata = item->metadata().dynamicCast<model::CameraItemMetadata>();

    Q_ASSERT(cameraMetadata);

    return QSharedPointer<model::ExportImageModel>::create(project->name(), item, cameraMetadata, project->inkData());
}

void model::ExportImageModel::setName(QString name)
{
    m_name = name;
}

} // namespace model
} // namespace capture
