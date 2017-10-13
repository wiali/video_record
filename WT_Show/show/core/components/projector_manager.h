#pragma once
#ifndef PROJECTORMONITOR_H
#define PROJECTORMONITOR_H

#include <QObject>

#include <projector/projector.h>

#include "model/application_state_model.h"

namespace capture {
namespace components {

class ProjectorManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectorManager(QSharedPointer<model::ApplicationStateModel> model, QObject *parent = 0);
    ~ProjectorManager();

signals:

private slots:
    void onProjectorStandby();
    void onProjectorOn();
    void onProjectorFailed();
    void updateProjectorState();

private:
    void turnProjectorOn();

    QSharedPointer<proapi::projector::Projector> m_projector;
    QSharedPointer<model::ApplicationStateModel> m_model;
    QString m_originalProjectorState;
    bool m_waitingForStandby;
    bool m_waitingForOn;
    bool m_projectorState;
};

} // namespace components
} // namespace capture

#endif  // PROJECTORMONITOR_H
