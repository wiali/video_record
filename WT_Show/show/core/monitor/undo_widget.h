#pragma once
#ifndef UNDO_WIDGET_H
#define UNDO_WIDGET_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class UndoWidget : public RightMenuButton
{
    Q_OBJECT

public:
    explicit UndoWidget(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private:
    bool canUndo();

private slots:
    void onUndoButtonClicked();
    void onHistoryChanged();
};

} // namespace monitor
} // namespace capture

#endif // UNDO_WIDGET_H
