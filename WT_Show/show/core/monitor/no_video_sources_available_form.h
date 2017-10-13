#pragma once
#ifndef NO_VIDEO_SOURCES_AVAILABLE_WIDGET_H
#define NO_VIDEO_SOURCES_AVAILABLE_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

namespace Ui {
class NoVideoSourcesAvailableForm;
}

namespace capture {
namespace monitor {

class NoVideoSourcesAvailableForm : public QWidget
{
    Q_OBJECT

public:
    explicit NoVideoSourcesAvailableForm(QWidget *parent = 0);
    ~NoVideoSourcesAvailableForm();

private:
    QScopedPointer<Ui::NoVideoSourcesAvailableForm> ui;
};

} // namespace monitor
} // namespace capture

#endif // NO_VIDEO_SOURCES_AVAILABLE_WIDGET_H
