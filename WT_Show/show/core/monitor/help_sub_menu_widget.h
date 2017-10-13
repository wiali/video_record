#pragma once
#ifndef HELP_SUB_MENU_WIDGET_H
#define HELP_SUB_MENU_WIDGET_H

#include <QWidget>
#include <QJsonArray>
#include <QMovie>

#include <right_submenu_base_widget.h>
#include "MenuBase.h"

namespace Ui {
class HelpSubMenuWidget;
}

namespace capture {
namespace monitor {

class HelpSubMenuWidget : public MenuBase
{
    Q_OBJECT

public:
    explicit HelpSubMenuWidget(QWidget* target);
    ~HelpSubMenuWidget();

protected:
    void focusOutEvent(QFocusEvent *event) override;
    int xOffsetOfArrow() const override;
    int yOffsetOfArrow() const override;

private slots:
    void onTargetClicked(bool checked);
    void handleResize();
private:
    Ui::HelpSubMenuWidget *ui;
};

} // namespace monitor
} // namespace capture

#endif // HELP_SUB_MENU_WIDGET_H
