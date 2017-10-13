#pragma once
#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>

#include "right_menu_button.h"
#include "help_sub_menu_widget.h"

namespace capture {
namespace monitor {


class HelpWidget : public RightMenuButton
{
    Q_OBJECT

public:
    explicit HelpWidget(QWidget *parent = 0);

protected:
    QPointer<HelpSubMenuWidget> m_helpSubMenuWidget;

private slots:
    void on_helpButton_clicked(bool checked);

};

} // namespace monitor
} // namespace capture

#endif // HELPWIDGET_H
