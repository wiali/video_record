#include "help_widget.h"

namespace capture {
namespace monitor {

HelpWidget::HelpWidget(QWidget *parent)
    : RightMenuButton(parent)
{
    setIconName("icon-help");
    setText(tr("Help"));
    setCheckable(true);

    connect(this, &RightMenuButton::clicked, this, &HelpWidget::on_helpButton_clicked);

    m_helpSubMenuWidget = new HelpSubMenuWidget(this);
}

void HelpWidget::on_helpButton_clicked(bool checked)
{
    if (checked)
    {
        m_helpSubMenuWidget->setFocus();
    }
}

} // namespace monitor
} // namespace capture
