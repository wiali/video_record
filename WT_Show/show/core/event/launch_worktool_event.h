#pragma once
#ifndef LAUNCHWORKTOOLEVENT_H
#define LAUNCHWORKTOOLEVENT_H

#include <QEvent>
#include <QStringList>

namespace capture {
namespace event {

class LaunchWorktoolEvent : public QEvent
{
    Q_GADGET
public:
    enum Worktool
    {
        Control,
        Stage,
        Capture,
        Discover
    };

    Q_ENUM(Worktool)

    explicit LaunchWorktoolEvent(LaunchWorktoolEvent::Worktool worktool, QStringList parameters = QStringList());

    inline LaunchWorktoolEvent::Worktool worktool() const { return m_worktool; }
    inline QStringList parameters() const { return m_parameters; }

    static QEvent::Type type();

    void dispatch();

private:
    LaunchWorktoolEvent::Worktool m_worktool;
    QStringList m_parameters;
};

} // namespace event
} // namespace capture

#endif // LAUNCHWORKTOOLEVENT_H
