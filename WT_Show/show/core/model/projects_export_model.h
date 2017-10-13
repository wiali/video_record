#pragma once
#ifndef EXPORTMODEL_H
#define EXPORTMODEL_H

#include <QObject>
#include <QSharedPointer>

namespace capture {
namespace model {

class ProjectsExportModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(model::ProjectsExportModel::State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(model::ProjectsExportModel::Format format READ format WRITE setFormat NOTIFY formatChanged)
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged)

public:
    enum State
    {
        NotExporting,
        PrepairingToExport,
        Exporting,
        PerformingOcr,
        FinalizingExport
    };

    Q_ENUM(State)

    enum Format
    {
        PNG,
        JPG,
        PDF,
        OCR,
        Clipboard,
        Stage
    };

    Q_ENUM(Format)

    explicit ProjectsExportModel(QObject *parent = 0);

    capture::model::ProjectsExportModel::State state() const;
    int count() const;
    int index() const;
    capture::model::ProjectsExportModel::Format format() const;

signals:

    void stateChanged (capture::model::ProjectsExportModel::State state);
    void countChanged (int count);
    void indexChanged (int index);
    void formatChanged(capture::model::ProjectsExportModel::Format format);

public slots:

    void setState (capture::model::ProjectsExportModel::State state);
    void setCount (int count);
    void setIndex (int index);

    /*!
     * \brief Sets the export format for currently exported item.
     * \warning This method should be invoked ONLY from ExportImageProcessor.
     * \param format Format of exported item.
     */
    void setFormat(capture::model::ProjectsExportModel::Format format);

private:

    ProjectsExportModel::State m_state;
    int m_count;
    int m_index;
    ProjectsExportModel::Format m_format;
};

} // namespace model
} // namespace capture

#endif // EXPORTMODEL_H
