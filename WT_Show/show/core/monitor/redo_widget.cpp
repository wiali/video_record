#include "redo_widget.h"

#include "common/utilities.h"
#include "common/history_manager.h"

namespace capture {
namespace monitor {

RedoWidget::RedoWidget(QWidget *parent)
    : RightMenuButton(parent)
{
    connect(this, &RedoWidget::clicked, this, &RedoWidget::onRedoButtonClicked);

    setIconName("icon-redo");
    setText(tr("Redo"));

    connect(common::Utilities::getHistoryManager().data(), &common::HistoryManager::historyChanged,
        this, &RedoWidget::onHistoryChanged);
}

void RedoWidget::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    common::Utilities::getHistoryManager()->setModel(model);
}

void RedoWidget::onHistoryChanged()
{
    setEnabled(canRedo());
}

void RedoWidget::onRedoButtonClicked()
{
    common::Utilities::getHistoryManager()->redo();
    setEnabled(canRedo());
}

bool RedoWidget::canRedo()
{
    return common::Utilities::getHistoryManager()->canRedo();
}

} // namespace monitor
} // namespace capture
