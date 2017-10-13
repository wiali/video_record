#pragma once
#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <QObject>
#include <QStack>
#include <QSharedPointer>

#include "t_command.h"

namespace capture {
namespace common {

class CommandManager : public QObject
{
Q_OBJECT
    typedef QStack<QSharedPointer<TCommand>> commandStack_t;

public:
    CommandManager() {}

    void addCommand(QSharedPointer<TCommand> cmd)
    {
        //clear the redo stack to make sure this operation is last operation 
        //whatever how many operations will be left in RedoStack. The reason
        //is user only can remember the most recent operation as the top of 
        //the redo.
        m_RedoStack.clear(); 
        m_UndoStack.push(cmd);
        emit commandHistoryChanged();
    }

    template<typename T, typename U>
    void add(U command_args)
    {
        auto command = QSharedPointer<T>::create(command_args);
        addCommand(command);
    }

    bool canUndo()
    {
        return m_UndoStack.size() > 1;
    }

    bool canRedo()
    {
        return m_RedoStack.size() > 0;
    }

    void undo()
    {
        if (!canUndo())
            return;
        m_RedoStack.push(m_UndoStack.top());
        m_UndoStack.pop();
        m_UndoStack.top()->undo();
        emit commandHistoryChanged();
        emit refresh();
    }

    void redo()
    {
        if (!canRedo())
            return;
        m_RedoStack.top()->redo();
        m_UndoStack.push(m_RedoStack.top());
        m_RedoStack.pop();
        emit commandHistoryChanged();
        emit refresh();
    }

    QSharedPointer<TCommand> firstCommand()
    {
        if (!m_UndoStack.empty())
            return m_UndoStack.first();
        return QSharedPointer<TCommand>();
    }

    QSharedPointer<TCommand> lastCommand()
    {
        if(!m_UndoStack.empty())
            return m_UndoStack.top();
        return QSharedPointer<TCommand>();
    }

    bool empty()
    {
        return m_UndoStack.empty() && m_RedoStack.empty();
    }

    void removeLastCommand()
    {
        m_UndoStack.pop();
        emit commandHistoryChanged();
    }

    void clear()
    {
        m_UndoStack.clear();
        m_RedoStack.clear();
    }

signals:
    void commandHistoryChanged();
    void refresh();

private:
    commandStack_t m_UndoStack;
    commandStack_t m_RedoStack;
};

} // namespace common
} // namespace capture

#endif // COMMAND_MANAGER_H
