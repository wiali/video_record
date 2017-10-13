#pragma once
#ifndef STAGEPROJECTEXPORTER_H
#define STAGEPROJECTEXPORTER_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <QMutex>
#include <QXmlStreamWriter>
#include <QQueue>

#include <quazip.h>

#include <stage_project.h>

#include "model/projects_export_model.h"

namespace capture {
namespace components {

/*!
 * \brief The StageProjectExporter class is responsible for asynchronous export of the stage project images to Stage-compatible format.
 * \details This class monitors occurences of ExportProjectsEvent start exporting. Only single export operation is being executed at the same time.
 */
class StageProjectExporter : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit StageProjectExporter(QSharedPointer<model::ProjectsExportModel> model, QObject *parent = 0);
    virtual void run();

signals:
    void exportFailed(const QString& msg);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:

    struct StageItemExportData;
    typedef QList<StageItemExportData> FlatExportList;

    void tryExportNextProject();
    QString imageFormat();

    void exportToStage(QVector<QSharedPointer<StageProject> > projects);

    void zipObjectsDataFile(QuaZip &zipFile, FlatExportList &flatItemList, QSharedPointer<StageProject> &project);
    void zipImageFiles(QuaZip &zipFile, FlatExportList& flatItemList);
    void zipSensorDataFiles(QuaZip &zipFile, FlatExportList& flatItemList);
    void zipImageFile(QuaZip &zipFile, const QString &fileName, const QImage& image);

    void flattenItemList(FlatExportList &flattenItemList, const QList<QSharedPointer<StageItem> > &stageItems);
    void writeUUIDGenerator(QXmlStreamWriter &xmlWriter, int itemCount);
    void writeStageItem(QXmlStreamWriter &xmlWriter, FlatExportList &flattenItemList, QSharedPointer<StageItem> stageItem);

    void cleanupTemporaryFiles(const QVector<QString>& fileNames);

    QQueue<QVector<QSharedPointer<StageProject>>> m_exportQueue;
    QMutex m_mutex;
    QScopedPointer<QThreadPool> m_threadPool;
    QSharedPointer<model::ProjectsExportModel> m_model;
};

} // namespace components
} // namespace capture

#endif // STAGEPROJECTEXPORTER_H
