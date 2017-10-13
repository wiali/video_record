#include "help_sub_menu_widget.h"
#include "ui_help_sub_menu_widget.h"

#include <QDebug>

#include "event/launch_worktool_event.h"
#include "monitor_window.h"
#include "common/utilities.h"
#include "global_utilities.h"

namespace capture {
namespace monitor {

HelpSubMenuWidget::HelpSubMenuWidget(QWidget* target)
    : MenuBase(target)
    , ui(new Ui::HelpSubMenuWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->versionLabel->setText(QString("%1 %2")
                              .arg(tr("Capture version:"))
                              .arg(QApplication::instance()->applicationVersion()));

    ui->copyrightLabel->setText(tr("© Copyright 2016, 2017 HP Development Company, L.P."));

    if(GlobalUtilities::isRightToLeft())
    {
        ui->copyrightLabel->setAlignment(Qt::AlignRight);
    }

    this->setContent(ui->frame_content);
    this->setHasCaption(false);
    this->setArrowWidth(15);

    if (auto monitorWindow = common::Utilities::getMonitorWindow())
    {
        connect(monitorWindow, &capture::monitor::MonitorWindow::monitorWindowResized, this, &HelpSubMenuWidget::handleResize);
    }

    auto pushButton = qobject_cast<QPushButton*>(m_target);
    auto menuButton = qobject_cast<AbstractMenuButton*>(m_target);

    if (pushButton != nullptr) {
        connect(pushButton, &QPushButton::clicked, this, &HelpSubMenuWidget::onTargetClicked);
    }
    else if (menuButton != nullptr) {
        connect(menuButton, &AbstractMenuButton::clicked, this, &HelpSubMenuWidget::onTargetClicked);
    }
    else {
        qWarning() << this << "Cannot connect to clicked signal on target" << m_target;
    }
}

HelpSubMenuWidget::~HelpSubMenuWidget()
{
    delete ui;
}

int HelpSubMenuWidget::xOffsetOfArrow() const
{
    return 1;
}

int HelpSubMenuWidget::yOffsetOfArrow() const
{
    return 50;
}

void HelpSubMenuWidget::focusOutEvent(QFocusEvent *event)
{
    hide();
    fold();

    QTimer::singleShot(200, this, [ this ] {
        if(m_target->property("checked").toBool())
        {
            m_target->setProperty("checked", false);
        }
    });

    return MenuBase::focusOutEvent(event);
}

void HelpSubMenuWidget::handleResize()
{
    hide();
    fold();
}

void HelpSubMenuWidget::onTargetClicked(bool checked)
{
    if (checked)
    {
        unfold();
    }
    else
    {
        fold();
    }
}

} // namespace monitor
} // namespace capture
