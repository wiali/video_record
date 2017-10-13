#pragma once
#ifndef CLEAN_ENVIRONMENT_H
#define CLEAN_ENVIRONMENT_H

#include <QObject>
#include "event/clean_event.h"

namespace capture {
namespace components {

class CleanEnvironment : public QObject
{
    Q_OBJECT
public:
    explicit CleanEnvironment(QObject *parent = 0);

    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    void deleteDMPFiles();
};

} // namespace components
} // namespace capture

#endif // CLEAN_ENVIRONMENT_H
