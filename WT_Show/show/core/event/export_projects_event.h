#pragma once
#ifndef EXPORTPROJECTSEVENT_H
#define EXPORTPROJECTSEVENT_H

#include <QEvent>

#include <stage_project.h>

namespace capture {
namespace event {

class ExportProjectsEvent : public QEvent
{
    Q_GADGET
public:
    explicit ExportProjectsEvent(QVector<QSharedPointer<StageProject>> projects);

    static QEvent::Type type();
    void dispatch();

    inline QVector<QSharedPointer<StageProject>> projects() const { return m_projects; }

private:

    QVector<QSharedPointer<StageProject>> m_projects;
};

} // namespace event
} // namespace capture

#endif // EXPORTPROJECTSEVENT_H
