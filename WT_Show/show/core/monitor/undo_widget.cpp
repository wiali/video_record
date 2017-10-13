#include "undo_widget.h"

#include <QDebug>
#include "common/utilities.h"
#include "common/history_manager.h"

namespace capture {
namespace monitor {

UndoWidget::UndoWidget(QWidget *parent)
    : RightMenuButton(parent)
{
    connect(this, &UndoWidget::clicked, this, &UndoWidget::onUndoButtonClicked);

    setIconName("icon-undo");
    setText(tr("Undo"));

    connect(common::Utilities::getHistoryManager().data(), &common::HistoryManager::historyChanged,
        this, &UndoWidget::onHistoryChanged);
}

void UndoWidget::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    common::Utilities::getHistoryManager()->setModel(model);
}

void UndoWidget::onHistoryChanged()
{
    setEnabled(canUndo());
}

void UndoWidget::onUndoButtonClicked()
{
    common::Utilities::getHistoryManager()->undo();
    setEnabled(canUndo());
}

bool UndoWidget::canUndo()
{
    return common::Utilities::getHistoryManager()->canUndo();
}

} // namespace monitor
} // namespace capture
