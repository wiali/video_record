#pragma once
#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QSharedPointer>
#include <QMap>
#include <QObject>
#include <QVector>

#include "command_manager.h"
#include <stage_project.h>
#include "model/application_state_model.h"

namespace capture {
namespace common {

class HistoryManager : public QObject 
{
    Q_OBJECT

public:
    explicit HistoryManager();
    virtual ~HistoryManager();

    void setModel(QSharedPointer<model::ApplicationStateModel> model);
    bool canUndo();
    bool canRedo();
    void undo();
    void redo();

    template<typename U>
    void addInkCommand(U command_args, bool bFirst = false);

    template<typename U>
    void addEditCommand(U command_args);

signals:
    void historyChanged();

public slots:
    void onInkHistoryChanged(bool first);

private slots :
    void onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode);
    void onSelectedProjectChanged(QSharedPointer<StageProject> project);
    void onEditHistoryChanged(bool push);
    void onCommandHistoryChanged();
    void onProjectRemoved(QSharedPointer<StageProject> project);
    void onRefresh();

private:
    typedef QMap<QSharedPointer<StageProject>, QSharedPointer<CommandManager>> Map_Project_CmdMgr;
    QSharedPointer<CommandManager> getCmdMgr();
    bool isEditMode();

    QSharedPointer<model::ApplicationStateModel> m_model;
    Map_Project_CmdMgr m_map_global;
    Map_Project_CmdMgr m_map_edit;
    QVector<QMetaObject::Connection> m_connections;
};


template<typename U>
void HistoryManager::addInkCommand(U command_args, bool bFirst)
{
    //make sure only insert command on the first time.
    if (bFirst && !m_map_global[m_model->selectedProject()]->empty())
        return;

    //make sure the live ink data can be reverted.
    if (bFirst && command_args->inkStrokes().count() > 0)
    {
        U tempInkData = U::create();
        tempInkData->clone(command_args);

        U emptyInkData = U::create();
        command_args->clone(emptyInkData);

        m_map_global[m_model->selectedProject()]->add<InkCommand>(command_args);
        command_args->clone(tempInkData);
    }
    m_map_global[m_model->selectedProject()]->add<InkCommand>(command_args);
}

template<typename U>
void HistoryManager::addEditCommand(U command_args)
{
    m_map_edit[m_model->selectedProject()]->add<EditCommand>(command_args);
}

} // namespace common
} // namespace capture

#endif  // HISTORYMANAGER_H
