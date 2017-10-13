#pragma once
#ifndef DELETE_PROJECT_WIDGET_H
#define DELETE_PROJECT_WIDGET_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class DeleteProjectWidget : public RightMenuButton
{
    Q_OBJECT

public:
    explicit DeleteProjectWidget(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();

private:
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // DELETE_PROJECT_WIDGET_H
