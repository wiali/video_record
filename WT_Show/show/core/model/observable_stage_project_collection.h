#pragma once
#ifndef OBSERVABLEPLAYGROUNDCOLLECTION_H
#define OBSERVABLEPLAYGROUNDCOLLECTION_H

#include <QObject>
#include <QSharedPointer>

#include <stage_project.h>

namespace capture {
namespace model {

class ObservableStageProjectCollection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVector<QSharedPointer<StageProject>> items READ items)
public:
    explicit ObservableStageProjectCollection(QObject *parent = 0);

    QVector<QSharedPointer<StageProject>> items() const;

    void add(QSharedPointer<StageProject> project);
    void removeAt(int index);
    void remove(QSharedPointer<StageProject> project);
    void moveItem(int projectIndex, int destinationIndex);
    void clear();

    int count() const;

    QSharedPointer<StageProject> at(int index) const;

signals:

    void added(QSharedPointer<StageProject> project);
    void removed(QSharedPointer<StageProject> project);

public slots:

private:

    QVector<QSharedPointer<StageProject>> m_items;
};

} // namespace model
} // namespace capture


#endif // OBSERVABLEPLAYGROUNDCOLLECTION_H
