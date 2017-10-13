#include "clean_environment.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

namespace capture {
namespace components {

CleanEnvironment::CleanEnvironment(QObject *parent)
    : QObject(parent) {
    QCoreApplication::instance()->installEventFilter(this);
}

bool CleanEnvironment::eventFilter(QObject *obj, QEvent *event)
{
    bool processed = false;

    if (event->type() == event::CleanEvent::type()) {
        if (auto cleanEvent = static_cast<event::CleanEvent*>(event)) {
            deleteDMPFiles();
            processed = true;
        }
    }

    return processed ? processed : QObject::eventFilter(obj, event);
}

void CleanEnvironment::deleteDMPFiles() {
    qInfo() << "Deleting DMP files";
    QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir dir(path);
    QFileInfoList list = dir.entryInfoList();

    foreach (QFileInfo info, list) {
        QRegExp rx(QString("capture.*dmp"));
        if(info.fileName().contains(rx)) {
            QFile::remove(info.filePath());
            qDebug() << "Remove file" << info.filePath();
        }
    }
}

} // namespace components
} // namespace capture
