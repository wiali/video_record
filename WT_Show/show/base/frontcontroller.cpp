#include "frontcontroller.h"
#include "abstractcommand.h"

#include <QSharedPointer>
#include <QDebug>
#include <iostream>
#include "updateuievt.h"

SINGLETON_PATTERN_IMPLIMENT(FrontController)

FrontController::FrontController() : m_cmdExeMutex(QMutex::Recursive)
{
    m_nMaxEvtInQue = 1000;
    subscribeToEvtByName(_EXE_COMMAND_REQUEST_EVENT, MAKE_CALLBACK(FrontController::fire));
    this->start();
}

bool FrontController::fire( Event& evt )
{
    CmdEvt& rcCmdRequestEvt = dynamic_cast<CmdEvt&>(evt);
    QString strCommandName = rcCmdRequestEvt.getCommandName();
    QHash<QString, QMetaObject*>::iterator i = m_commandTable.find(strCommandName);

    //command not found
    if( i == m_commandTable.end() )
    {
        qWarning() << QString("No matched command name found. %1").arg(strCommandName);
        return true;
    }

    //create command
    const QMetaObject* pMetaObj = i.value();
    QObject* pObj = pMetaObj->newInstance();

    //if fail to create command class
    if(pObj == nullptr)
    {
        qCritical() << QString("Unable to create command class '%1'! Please ensure the constructor have Q_INVOKABLE macro.")
                    .arg(pMetaObj->className());
        return true;
    }

    QSharedPointer<AbstractCommand> pCmd(static_cast<AbstractCommand *>(pObj));

    /// execute it in worker thread or main(GUI) thread
    if(pCmd->getInWorkerThread() == false)
    {
        onCommandRequestArrive(rcCmdRequestEvt);
    }
    else
    {
        /// pending for worker thread execution
        m_evtQueMutex.lock();
        if( m_pWorkerThreadEvtQue.size() >= m_nMaxEvtInQue )
        {
            qWarning() << "Too Many Events Pending...Waiting...";
            m_evtQueNotFull.wait(&m_evtQueMutex);
            qWarning() << "Event Queue Not Full...Moving on...";
        }
        m_pWorkerThreadEvtQue.push_back(evt.clone());
        m_evtQueMutex.unlock();
        m_evtQueNotEmpty.wakeAll();
    }

    return true;
}

void FrontController::onCommandRequestArrive( CmdEvt& evt )
{
    QMutexLocker cCmdExeMutexLocker(&m_cmdExeMutex);
    UpdateUIEvt refreshUIEvt;
    CommandParameter& request = evt.getParameters();
    CommandParameter& respond = refreshUIEvt.getParameters();

    // find command by name
    QString strCommandName = evt.getCommandName();
    respond.setParameter("command_name", strCommandName);
    QHash<QString, QMetaObject*>::iterator iter = m_commandTable.find(strCommandName);
    if( iter != m_commandTable.end() )
    {
        //command name matched
        //create  command
        const QMetaObject* pMetaObj = iter.value();
        QObject* pObj = pMetaObj->newInstance();
        //if fail to create command class
        if(pObj == nullptr)
        {
            qCritical() << QString("Unable to create command class '%1'! Please ensure the constructor have Q_INVOKABLE macro.")
                        .arg(pMetaObj->className());
            return;
        }
        QSharedPointer<AbstractCommand> pCmd(static_cast<AbstractCommand *>(pObj));
        //execute command
        if( pCmd->execute(request, respond) == false )
        {
            qDebug() << QString("%1 Execution Failed!").arg(pMetaObj->className());
        }
        else
        {
            refreshUIEvt.dispatch();
            qDebug() << QString("%1 Execution Success!").arg(pMetaObj->className());
        }
        return;

    }
    qWarning() << QString("No matched command name found. %1").arg(strCommandName);
    return;
}

bool FrontController::registerCommand(const QString& cCommandName, const QMetaObject * pMetaObject)
{
    if(m_commandTable.contains(cCommandName))
    {
        qCritical() << QString("%1 Register Fail! Duplicated command name!").arg(cCommandName);
        return false;
    }

    QString strAbsCmdClassName = AbstractCommand::staticMetaObject.className();
    if( pMetaObject->className() == strAbsCmdClassName )
    {
        qCritical() << QString("Trying to register name '%1' for abstract class '%2'! "
                               "You may miss the Q_OBJECT macro for the derived class of '%3'")
                       .arg(cCommandName).arg(strAbsCmdClassName).arg(strAbsCmdClassName);
        return false;
    }

    QMetaObject* p = const_cast<QMetaObject*>(pMetaObject);
    m_commandTable.insert(cCommandName, p);
    qDebug() << QString("%1 Register Success!").arg(pMetaObject->className());
    return true;
}

void FrontController::unregisterAllCommand()
{
    m_commandTable.clear();
}

void FrontController::run()
{
    forever
    {
        /// get one event from the waiting queue
        m_evtQueMutex.lock();
        if( m_pWorkerThreadEvtQue.empty() )
        {
            m_evtQueNotEmpty.wait(&m_evtQueMutex);
        }
        Event* pcEvt = m_pWorkerThreadEvtQue.front();
        m_pWorkerThreadEvtQue.pop_front();
        m_evtQueMutex.unlock();
        m_evtQueNotFull.wakeAll();

        /// execute command
        CmdEvt& rcCmdRequestEvt = dynamic_cast<CmdEvt&>(*pcEvt);
        onCommandRequestArrive(rcCmdRequestEvt);
        delete pcEvt;
    }
}


