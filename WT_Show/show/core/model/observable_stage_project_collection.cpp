#include "observable_stage_project_collection.h"

#include <QDebug>

namespace capture {
namespace model {

ObservableStageProjectCollection::ObservableStageProjectCollection(QObject *parent)
    : QObject(parent)
{
    // Needed for proper cross-thread signals
    static struct Initialize
    {
        Initialize()
        {
            qRegisterMetaType<QVector<QSharedPointer<StageProject>>>();
        }
    } initialize;
}

QVector<QSharedPointer<StageProject>> ObservableStageProjectCollection::items() const
{
    return m_items;
}

void ObservableStageProjectCollection::clear()
{
    m_items.clear();
}

int ObservableStageProjectCollection::count() const
{
    return m_items.count();
}

void ObservableStageProjectCollection::add(QSharedPointer<StageProject> project)
{
    m_items.push_back(project);

    qInfo() << this << "Item added" << project;

    emit added(project);
}

void ObservableStageProjectCollection::moveItem(int projectIndex, int destinationIndex)
{
    //move project item from one position to destination index
    qDebug() << "Moving project item from " << projectIndex << " to " << destinationIndex;
    m_items.move(projectIndex,destinationIndex);
}

QSharedPointer<StageProject> ObservableStageProjectCollection::at(int index) const
{
    QSharedPointer<StageProject> result;

    if (index >= 0 && index < m_items.count())
    {
        result = m_items.at(index);
    }

    return result;
}

void ObservableStageProjectCollection::removeAt(int index)
{
    if (index >= 0 && index < m_items.count())
    {
        auto removedItem = m_items[index];
        m_items.removeAt(index);

        emit removed(removedItem);
    }
    else
    {
        qWarning() << this << "Index" << index << "is out of range (0;" << m_items.count() << ")";
    }
}

void ObservableStageProjectCollection::remove(QSharedPointer<StageProject> project) {
    if (m_items.contains(project)) {
        removeAt(m_items.indexOf(project));
    } else {
        qWarning() << this << "Provided project is not in the list of projects";
    }
}

} // namespace model
} // namespace capture

