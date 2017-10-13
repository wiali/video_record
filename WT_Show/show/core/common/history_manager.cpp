#include "history_manager.h"
#include "utilities.h"
#include "stage_item_metadata.h"

namespace capture {
namespace common {

/**************************************************************************************************
@brief:
**************************************************************************************************/
HistoryManager::HistoryManager()
{
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
HistoryManager::~HistoryManager()
{
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;

    connect(m_model->projects().data(), &model::ObservableStageProjectCollection::removed,
        this, &common::HistoryManager::onProjectRemoved);

    connect(m_model.data(), &model::ApplicationStateModel::selectedProjectChanged,
        this, &common::HistoryManager::onSelectedProjectChanged);
    onSelectedProjectChanged(m_model->selectedProject());

    connect(m_model.data(), &model::ApplicationStateModel::editModeChanged,
        this, &common::HistoryManager::onEditModeChanged);
}

void HistoryManager::onProjectRemoved(QSharedPointer<StageProject> project)
{
   m_map_global.remove(project);
   m_map_edit.remove(project);
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::onSelectedProjectChanged(QSharedPointer<StageProject> project)
{
    foreach(auto connection, m_connections)
    {
        disconnect(connection);
    }
    m_connections.clear();

    if (project && project->items().count() > 0)
    {
        if (!m_map_global.contains(project))
            m_map_global[project] = QSharedPointer<CommandManager>::create();

        if (!m_map_edit.contains(project))
            m_map_edit[project] = QSharedPointer<CommandManager>::create();

        auto cmdMgr_edit = m_map_edit[project];
        auto cmdMgr_global = m_map_global[project];

        m_connections << connect(cmdMgr_global.data(), &CommandManager::commandHistoryChanged,
            this, &common::HistoryManager::onCommandHistoryChanged);
        m_connections << connect(cmdMgr_global.data(), &CommandManager::refresh,
            this, &common::HistoryManager::onRefresh);
        m_connections << connect(cmdMgr_edit.data(), &CommandManager::commandHistoryChanged,
            this, &common::HistoryManager::onCommandHistoryChanged);
        m_connections << connect(cmdMgr_edit.data(), &CommandManager::refresh,
            this, &common::HistoryManager::onRefresh);

        auto currentMetadata = project->items().first()->metadata();

        m_connections << connect(currentMetadata.data(),
            &EditableItemMetadata::metadataHistoryChanged, [this] {
            if (isEditMode())
                onEditHistoryChanged(true);
            else
                onInkHistoryChanged(false);
        });

        emit historyChanged();
    }    
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
QSharedPointer<CommandManager> HistoryManager::getCmdMgr()
{
    auto ret = QSharedPointer<CommandManager>();

    if (!m_model)
        return ret;

    auto currentProject = m_model->selectedProject();

    if (!currentProject)
        return ret;

    if (isEditMode())
    {
        if (m_map_edit.contains(currentProject))
            return m_map_edit[currentProject];
    }
    else if (m_map_global.contains(currentProject))
    {
        return m_map_global[currentProject];
    }

    return ret;
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
bool HistoryManager::canUndo()
{
    auto cmdMgr = getCmdMgr();
    if(cmdMgr)
        return getCmdMgr()->canUndo();
    return false;
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
bool HistoryManager::canRedo()
{
    auto cmdMgr = getCmdMgr();
    if (cmdMgr)
        return getCmdMgr()->canRedo();
    return false;
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode)
{
    auto currentProject = m_model->selectedProject();

    if (mode == model::ApplicationStateModel::EditMenuMode::MainMenuOpen)
    {
        auto cmdMgr_edit = m_map_edit[currentProject];
        cmdMgr_edit->clear();
        onEditHistoryChanged(true);
    }
    else if (mode == model::ApplicationStateModel::EditMenuMode::MainMenuClose)
    {
        auto cmdMgr_edit = m_map_edit[currentProject];
        auto cmdMgr_global = m_map_global[currentProject];
        auto lastEditCmd = cmdMgr_edit->lastCommand();
        if (lastEditCmd)
        {
            QVector<QSharedPointer<TCommand>> commands;
            auto firstEditCmd = cmdMgr_edit->firstCommand();
            if (firstEditCmd && firstEditCmd != lastEditCmd)
            {
                commands.push_back(firstEditCmd);
                commands.push_back(cmdMgr_global->lastCommand());

                cmdMgr_global->removeLastCommand();

                QSharedPointer<TCommand> compositeCmd(new CompositeCommand(commands));
                cmdMgr_global->addCommand(compositeCmd);
                cmdMgr_global->addCommand(lastEditCmd);
            }
        }
        else
        {
            emit historyChanged();
        }
        cmdMgr_edit->clear();
    }
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::undo()
{
    if (!canUndo())
        return;

    getCmdMgr()->undo();
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::redo()
{
    if (!canRedo())
        return;

    getCmdMgr()->redo();
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::onEditHistoryChanged(bool push)
{
    Q_UNUSED(push);

    auto project = m_model->selectedProject();
    auto currentMetadata = project->items().first()->metadata();
    addEditCommand(currentMetadata);
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::onInkHistoryChanged(bool first)
{
    auto project = m_model->selectedProject();
    auto currentMetadata = project->items().first()->metadata();

    addInkCommand(currentMetadata, first);
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::onCommandHistoryChanged()
{
    //notify the UndoWidget and RedoWidget
    emit historyChanged();
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
void HistoryManager::onRefresh()
{
    //update the ink_widget and ink_layer_widget
    m_model->inkWidgetChanged();
}

/**************************************************************************************************
@brief:
**************************************************************************************************/
bool HistoryManager::isEditMode()
{
    return m_model->editMode() == model::ApplicationStateModel::EditMenuMode::SubMenuOpen ||
        m_model->editMode() == model::ApplicationStateModel::EditMenuMode::SubMenuClose ||
        m_model->editMode() == model::ApplicationStateModel::EditMenuMode::MainMenuOpen;
}

} // namespace common
} // namespace capture
