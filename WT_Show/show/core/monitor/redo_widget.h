#pragma once
#ifndef REDO_WIDGET_H
#define REDO_WIDGET_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class RedoWidget : public RightMenuButton
{
    Q_OBJECT

public:
    explicit RedoWidget(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private:
    bool canRedo();

private slots:
    void onRedoButtonClicked();
    void onHistoryChanged();
};

} // namespace monitor
} // namespace capture

#endif // REDO_WIDGET_H
