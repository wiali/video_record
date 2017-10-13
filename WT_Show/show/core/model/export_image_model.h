#pragma once
#ifndef EXPORT_IMAGE_MODEL_H
#define EXPORT_IMAGE_MODEL_H

#include <QObject>
#include <QSharedPointer>

#include "camera_item_metadata.h"
#include "stage_project.h"

namespace capture {
namespace model {

class ExportImageModel : public QObject
{
    Q_OBJECT

public:

    explicit ExportImageModel(QString name, QSharedPointer<StageItem> item, QSharedPointer<model::CameraItemMetadata> metadata,
                              QSharedPointer<InkData> inkData, QObject *parent = 0);

    /*! \brief Current metadata o the item
     */
    QSharedPointer<model::CameraItemMetadata> metadata() const;

    /*! \brief Gets the image representing this item
     */
    QSharedPointer<StageItem> item() const;

    QString name() const;

    QSharedPointer<InkData> inkData() const;

    static QSharedPointer<model::ExportImageModel> fromStageProject(QSharedPointer<StageProject> project);

public slots:
    void setName(QString name);

private:

    /*! \brief Member storing an image.
     */
    QSharedPointer<StageItem> m_item;

    /*! \brief Member holding the metadata for this item
     */
    QSharedPointer<model::CameraItemMetadata> m_metadata;

    /*!
     * \brief Name of this item.
     */
    QString m_name;

    /*!
     * \brief Ink content of this item.
     */
    QSharedPointer<InkData> m_inkData;
};

} // namespace model
} // namespace capture

#endif // EXPORT_IMAGE_MODEL_H
