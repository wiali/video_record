#include <QBuffer>
#include <QDataStream>
#include <QLocalSocket>

#include "single_instance.h"

/**************************************************************************************************/

SingleInstance::SingleInstance(const QString &appId, QObject *parent)
    : QObject(parent)
    , m_appId(appId)
{
}

/**************************************************************************************************/

SingleInstance::~SingleInstance()
{
    m_server.close();
}

/**************************************************************************************************/

bool SingleInstance::isRunning()
{
    QLocalSocket socket;
    socket.connectToServer(m_appId);
    bool bConn = socket.isOpen();
    socket.close();
    return bConn;
}

/**************************************************************************************************/

void SingleInstance::buildServer()
{
    m_server.setSocketOptions(QLocalServer::WorldAccessOption);
    QObject::connect(&m_server, &QLocalServer::newConnection, this, &SingleInstance::newConnection);
    m_server.listen(m_appId);
}

/**************************************************************************************************/

void SingleInstance::newConnection(){
    QLocalSocket *client = m_server.nextPendingConnection();
    if(client == nullptr){
        return;
    }

    QObject::connect(client, &QLocalSocket::readyRead,
                     [client,this](){
        QStringList arguments;
        QByteArray buf = client->readAll();

        //Converts QByteArray to QStringList
        QBuffer buffer(&buf);
        buffer.open(QIODevice::ReadWrite);
        QDataStream out(&buffer);
        out >> arguments;
        qInfo() << "Receive data from new instance of" << m_appId;
        client->deleteLater();
        Q_EMIT receivedArguments(arguments);
    });
}

/**************************************************************************************************/

void SingleInstance::sendArguments(const QStringList &arguments)
{
    QLocalSocket *socket = new QLocalSocket;
    socket->connectToServer(m_appId);

    //Converts QStringList to QByteArray
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    QDataStream out(&buffer);
    out << arguments;

    socket->write(ba);
    socket->flush();
    socket->waitForBytesWritten();
    socket->close();
    delete socket;
    Q_EMIT flushDone();
}

/**************************************************************************************************/
