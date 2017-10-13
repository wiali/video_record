#pragma once
#ifndef WORKTOOLSLAUNCHER_H
#define WORKTOOLSLAUNCHER_H

#include <QObject>
#include "event/launch_worktool_event.h"

namespace capture {
namespace components {

class WorktoolsLauncher : public QObject
{
    Q_OBJECT
public:
    explicit WorktoolsLauncher(QObject *parent = 0);

    virtual bool eventFilter(QObject *obj, QEvent *event);

private:

    bool launchWorktool(event::LaunchWorktoolEvent::Worktool worktool, QStringList parameters);
};

} // namespace components
} // namespace capture

#endif // WORKTOOLSLAUNCHER_H
