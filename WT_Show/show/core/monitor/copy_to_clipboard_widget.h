#pragma once
#ifndef COPY_TO_CLIPBOARD_WIDGET_H
#define COPY_TO_CLIPBOARD_WIDGET_H

#include <QWidget>

#include "model/application_state_model.h"
#include <right_menu_button.h>

namespace capture {
namespace monitor {

class CopyToClipboardWidget : public RightMenuButton
{
    Q_OBJECT

public:
    explicit CopyToClipboardWidget(QWidget *parent = 0);
    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();

private:
     QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // COPY_TO_CLIPBOARD_WIDGET_H
