#ifndef FRONTCONTROLLER_H
#define FRONTCONTROLLER_H

#include <QMetaObject>
#include <QHash>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include "def.h"
#include "module.h"
#include "cmdevt.h"

 /**********************************************************************************************************************
 @brief: The FrontController class
 *  Front controller pattern. It accepts all command requests and invoke corresponding commands
 *  (according to the <command name>-<command class> table)
 ***********************************************************************************************************************/
class FrontController : public Module, public QThread
{
public:
    virtual ~FrontController() {}
    
    /**********************************************************************************************************************
    @brief: Filter out the command request from other request, and then it calls
    *  onCommandRequestArrive function
    ***********************************************************************************************************************/
    virtual bool fire(Event& evt);

    /**********************************************************************************************************************
    @brief: onCommandRequestArrive One command request is captured by front controller.
    *  By default, it find out the corresponding command and execute this command immediately.
    *  You can override this function to to something else.
    ***********************************************************************************************************************/
    virtual void onCommandRequestArrive(CmdEvt& evt);

    bool registerCommand(const QString& strCommandName, const QMetaObject* pMetaObject);
    void unregisterAllCommand();

    virtual void run();

protected:
    explicit FrontController();

protected:
    /// <command string>-<command class> table
    ADD_CLASS_FIELD( CONCATE(QHash<QString,QMetaObject*>), commandTable, getCommandTable, setCommandTable)
    ADD_CLASS_FIELD(int, nMaxEvtInQue, setMaxEvtInQue, getMaxEvtInQue)

    ADD_CLASS_FIELD_PRIVATE(QList<Event*>, pMainThreadEvtQue)
    ADD_CLASS_FIELD_PRIVATE(QList<Event*>, pWorkerThreadEvtQue)
    ADD_CLASS_FIELD_PRIVATE(QMutex, evtQueMutex)
    ADD_CLASS_FIELD_PRIVATE(QWaitCondition, evtQueNotEmpty)
    ADD_CLASS_FIELD_PRIVATE(QWaitCondition, evtQueNotFull)
    ADD_CLASS_FIELD_PRIVATE(QMutex, cmdExeMutex)   ///< ensure one command execution at one time

    SINGLETON_PATTERN_DECLARE(FrontController)
};

#endif // FRONTCONTROLLER_H
