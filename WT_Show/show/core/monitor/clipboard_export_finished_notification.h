#pragma once
#ifndef CLIPBOARDEXPORTFINISHEDNOTIFICATION_H
#define CLIPBOARDEXPORTFINISHEDNOTIFICATION_H

#include <QWidget>

#include "model/application_state_model.h"
#include "fade_out_notification.h"

namespace capture {
namespace monitor {

class ClipboardExportFinishedNotification : public FadeOutNotification
{
    Q_OBJECT
public:
    explicit ClipboardExportFinishedNotification(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:

    void onProjectsExportStateChanged(model::ProjectsExportModel::State state);

private:

    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // CLIPBOARDEXPORTFINISHEDNOTIFICATION_H
