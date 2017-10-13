/*! \file single_instance.h
 *  \brief SingleInstance class is responsible to provide functionality to allow an application to make sure just one
 *         instance exists at the same time, check if another instance is already running and communicate the parameters
 *         a new instance of the same app was launched with.
 *  \date April, 2016
 *  \copyright 2016 HP Inc.
 */

#ifndef FORTIS_CONTAINER_DESKTOP_SINGLE_INSTANCE_H
#define FORTIS_CONTAINER_DESKTOP_SINGLE_INSTANCE_H

#include "shared_global.h"

#include <QObject>
#include <QLocalServer>

/*! \brief SingleInstance class is responsible to provide functionality to allow an application to make sure just one
 *         instance exists at the same time, check if another instance is already running and communicate the parameters
 *         a new instance of the same app was launched with.
 */
class SHAREDSHARED_EXPORT SingleInstance : public QObject
{
    Q_OBJECT
public:

    /*! \brief Initializes internal structures
     */
    explicit SingleInstance(const QString &appId, QObject *parent = 0);

    /*! \brief Disconnects the server
     */
    ~SingleInstance();

    /*! \brief Tries to connect to the server to check if there is an instance of the same app already running
     */
    bool isRunning();

    /*! \brief Connects to newConnection signal and makes the server starts listening for connections
     */
    void buildServer();

    /*! \brief Creates a socket, connects to the other app already running server and sends the arguments the
     *         app was launched to the other instance
     */
    void sendArguments(const QStringList &arguments);

Q_SIGNALS:

    /*! \brief Signal emitted when new arguments was received by another instance of the same app
     *  \param arguments The arguments that another instance of the same app was launched with
     */
    void receivedArguments(const QStringList &arguments);

    /*! \brief Signal emitted when the sendArguments data flush is done
     */
    void flushDone();

private Q_SLOTS:

    /*! \brief Slot to handle new local server connections
     */
    void newConnection();
private:

    /*! \brief The local server to receive arguments that a new instance of the same app was launched with
     */
    QLocalServer m_server;

    /*! \brief The app id
     */
    QString m_appId;
};

#endif // FORTIS_CONTAINER_DESKTOP_SINGLE_INSTANCE_H
