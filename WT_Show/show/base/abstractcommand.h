#ifndef ABSTRACTCOMMAND_H
#define ABSTRACTCOMMAND_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QDebug>
#include <QVariant>
#include "def.h"
#include "eventparam.h"

typedef EventParam CommandParameter;

 /**********************************************************************************************************************
 @brief: The AbstractCommand class
 *  All command should inherit this class and implement the execute function
 ***********************************************************************************************************************/
class AbstractCommand : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit AbstractCommand(QObject *parent = 0) : QObject(parent)
    {
        m_bInWorkerThread = false;
    }

    virtual ~AbstractCommand()
    {
    }

    /**********************************************************************************************************************
    @brief:  Execute All subclass should re implement this function.
    * \param rcInputArg this parameter is from UI(View)
    * \param rcOutputArg this parameter will be delivered to UI(View)
    ***********************************************************************************************************************/
    virtual bool execute(CommandParameter &rcInputArg, CommandParameter &rcOutputArg)
    {
        Q_UNUSED(rcInputArg)
        Q_UNUSED(rcOutputArg)
        qCritical() << "Please reimplement <AbstractCommand::execute>";
        return false;
    }

     /**********************************************************************************************************************
     @brief:  This command designed to execute in worker thread.
     * For computational intensive command, it should be set to true.
     * If it involves GUI classes creation, it MUST BE SET TO FALSE
     ***********************************************************************************************************************/
    ADD_CLASS_FIELD(bool, bInWorkerThread, getInWorkerThread, setInWorkerThread)
};

#endif // ABSTRACTCOMMAND_H
