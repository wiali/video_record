#pragma once
#ifndef TCOMMAND_H
#define TCOMMAND_H

#include <QSharedPointer>

#include "ink_data.h"
#include "stage_item_metadata.h"
#include "editable_item_metadata.h"

namespace capture {
namespace common {

class TCommand
{
public:
    explicit TCommand() {};
    virtual ~TCommand() {};
    virtual void execute() {};
    virtual void undo() { execute(); };
    virtual void redo() { execute(); };
    virtual void clone_s2o() {};
    virtual void clone_o2s() {};
};

template<typename T>
class GCommand : public TCommand
{
public:
    explicit GCommand(QSharedPointer<T> data) 
        : m_origin_data(data){};

    virtual void execute() 
    {
        clone_s2o();
        TCommand::execute();
    };

    virtual void undo()
    {
        TCommand::undo();
    };

    virtual void redo() 
    {
        TCommand::redo();
    };

protected:
    QSharedPointer<T> m_origin_data;
    QSharedPointer<T> m_snapshot_data;
};

class EditCommand : public GCommand<StageItemMetadata>
{
public:
    explicit EditCommand(QSharedPointer<StageItemMetadata> data)
        : GCommand<StageItemMetadata>(data) 
    {
        m_snapshot_data = QSharedPointer<EditableItemMetadata>::create();
        clone_o2s(); 
    };

    virtual void clone_s2o()
    {
        m_origin_data->updateProperties(m_snapshot_data);
    }

    virtual void clone_o2s()
    {
        m_snapshot_data->clone(m_origin_data);
    }
};

class InkCommand : public EditCommand
{
public:
    explicit InkCommand(QSharedPointer<StageItemMetadata> data)
        : EditCommand(data)
    {
    };
};

class CompositeCommand : public TCommand
{
public:
    explicit CompositeCommand(QVector<QSharedPointer<TCommand>> commands)
        : m_commands(commands){};

    virtual void undo()
    {
        for (auto& command : m_commands)
            command->undo();
    };

    virtual void redo()
    {
        for (auto& command : m_commands)
            command->redo();
    };

private:
    QVector<QSharedPointer<TCommand>> m_commands;
};

} // namespace common
} // namespace capture

#endif // TCOMMAND_H
