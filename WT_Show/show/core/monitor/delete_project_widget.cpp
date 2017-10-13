#include "delete_project_widget.h"

#include <QMessageBox>

#include "styled_message_box.h"

#include "common/utilities.h"

namespace capture {
namespace monitor {

DeleteProjectWidget::DeleteProjectWidget(QWidget *parent)
    : RightMenuButton(parent)
{
    setIconName("icon-delete");
    setText(tr("Delete"));

    connect(this, &RightMenuButton::clicked, this, &DeleteProjectWidget::onButtonClicked);
}

void DeleteProjectWidget::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
}

void DeleteProjectWidget::onButtonClicked()
{
    common::Utilities::playSound("qrc:/Resources/production/Sounds/popDialog.aif");
    auto selectedProject = m_model->selectedProject();
    Q_ASSERT(selectedProject != nullptr);

    if (selectedProject)
    {
        emit m_model->editButtonChanged(false);
        auto messageBox = common::Utilities::createMessageBox();

        messageBox->setText(tr("Delete image"));
        messageBox->setInformativeText(tr("Are you sure you want to delete this image?"));

        messageBox->addStyledButton(tr("Delete"), QMessageBox::AcceptRole);
        messageBox->addStyledButton(tr("Cancel"), QMessageBox::RejectRole);

        if (messageBox->exec() != QMessageBox::RejectRole)
        {
            m_model->projects()->remove(m_model->selectedProject());
        }
    }
    else
    {
        qCritical() << this << "Delete project clicked but no project is selected";
    }
}

} // namespace monitor
} // namespace capture
