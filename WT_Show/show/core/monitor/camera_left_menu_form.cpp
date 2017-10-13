#include "camera_left_menu_form.h"
#include "ui_camera_left_menu_form.h"

#include "project_item_form.h"

#include "event/change_mat_mode_event.h"
#include "export_widget.h"

#include <QAbstractItemModel>

namespace capture {
namespace monitor {

static QHash<model::LiveCaptureModel::PreCaptureMode, event::ChangeMatModeEvent::MatMode> CaptureModeTranslationTable {
    { model::LiveCaptureModel::Desktop, event::ChangeMatModeEvent::MatMode::Desktop },
    { model::LiveCaptureModel::LampOff, event::ChangeMatModeEvent::MatMode::LampOff },
    { model::LiveCaptureModel::LampOn, event::ChangeMatModeEvent::MatMode::LampOn }
};

static QHash<model::ApplicationStateModel::MatModeState, event::ChangeMatModeEvent::MatMode> MatModeTranslationTable {
    { model::ApplicationStateModel::Desktop, event::ChangeMatModeEvent::MatMode::Desktop },
    { model::ApplicationStateModel::LampOff, event::ChangeMatModeEvent::MatMode::LampOff },
    { model::ApplicationStateModel::LampOn, event::ChangeMatModeEvent::MatMode::LampOn }
};

CameraLeftMenuForm::CameraLeftMenuForm(QWidget *parent)
    : SharedLeftMenuForm(parent)
    , ui(new Ui::CameraLeftMenuForm)
    , m_mask(new QWidget(this))
    , m_isRemovingItem(false)
{
    // have Qt setup the UI Form baed on the .UI file for us
    ui->setupUi(this);

    ui->button_new->setText(tr("Add new capture"));
    ui->button_new->setIconName("icon-addnew");

    ui->button_export->setText(tr("Export all"));
    ui->button_export->setIconName("icon-export");

    addItemToBottom(ui->button_new);
    addItemToBottom(ui->button_export);

    exportWidget = new ExportWidget(ui->button_export);

    connect(list(), &QListWidget::currentRowChanged, this, &CameraLeftMenuForm::onCurrentRowChanged);
    connect(list(), &QListWidget::clicked, this, &CameraLeftMenuForm::onCurrentRowChanged);
    connect(list(), &QListWidget::currentItemChanged, this, &CameraLeftMenuForm::onCurrentItemChanged);
    connect(list()->model(), &QAbstractItemModel::rowsMoved, this, &CameraLeftMenuForm::onRowsMoved);

    m_mask->setGeometry(this->geometry());
    m_mask->setStyleSheet("background-color: rgba(0, 0, 0, 178);");
    m_mask->hide();
}

CameraLeftMenuForm::~CameraLeftMenuForm()
{
    delete ui;
}

void CameraLeftMenuForm::onRowsMoved(QModelIndex sourceParent,int sourceStart,int sourceEnd,QModelIndex destinationParent,int destinationRow){

    Q_UNUSED(sourceParent);
    Q_UNUSED(destinationParent);
    Q_UNUSED(sourceEnd);

    if(sourceStart < destinationRow) // need to decrement when moving item down
        destinationRow--;

    m_model->projects()->moveItem(sourceStart, destinationRow);
    m_model->setSelectedProject(m_model->projects()->at(destinationRow));
}

void CameraLeftMenuForm::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    auto currentWidget = dynamic_cast<ProjectItemForm*>(list()->itemWidget(current));

    if (currentWidget) {
        currentWidget->setHighlighted(true);
    }

    auto previousWidget = dynamic_cast<ProjectItemForm*>(list()->itemWidget(previous));

    if (previousWidget) {
        previousWidget->setHighlighted(false);
    }
}

void CameraLeftMenuForm::onCurrentRowChanged()
{
    // SPROUTSW-3960 - When removing items we let delete function figure out final index otherwise
    // we get two changes of index
    if (!m_isRemovingItem) {
        // We don't have any sorting so it's fine to work according to indices
        auto index = list()->currentRow();
        m_model->setSelectedProject(m_model->projects()->at(index));

        if (index >= 0) {
            m_model->setMode(model::ApplicationStateModel::Mode::Preview);
        }
    }
}

void CameraLeftMenuForm::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
    exportWidget->setModel(m_model);

    connect(model->projects().data(), &model::ObservableStageProjectCollection::added, this, &CameraLeftMenuForm::onProjectAdded);
    connect(model->projects().data(), &model::ObservableStageProjectCollection::removed, this, &CameraLeftMenuForm::onProjectRemoved);

    connect(model.data(), &model::ApplicationStateModel::selectedProjectChanged, this, &CameraLeftMenuForm::onSelectedProjectChanged);
    onSelectedProjectChanged(model->selectedProject());

    connect(model.data(), &model::ApplicationStateModel::modeChanged, this, &CameraLeftMenuForm::onApplicationModeChanged);
    onApplicationModeChanged(m_model->mode());

    connect(model->liveCapture().data(), &model::LiveCaptureModel::captureStateChanged, this, &CameraLeftMenuForm::updateDisabledState);

    updateDisabledState();

    connect(m_model.data(), &model::ApplicationStateModel::editModeChanged, this, &CameraLeftMenuForm::onEditModeChanged);
}

void CameraLeftMenuForm::updateDisabledState()
{
    bool isEnabled = m_model->projects()->count() > 0;
    isEnabled &= m_model->liveCapture()->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing;

    list()->setEnabled(isEnabled);
    ui->button_export->setEnabled(isEnabled);
}

void CameraLeftMenuForm::onApplicationModeChanged(model::ApplicationStateModel::Mode mode)
{
    switch(mode) {
    case model::ApplicationStateModel::Mode::None:
    case model::ApplicationStateModel::Mode::CameraFailedToStart:
    case model::ApplicationStateModel::Mode::NoCalibrationData:
    case model::ApplicationStateModel::Mode::NoVideoSource:
    case model::ApplicationStateModel::Mode::ColorCorrectionCalibration:
        return;
    case model::ApplicationStateModel::Mode::LiveCapture:
        ui->button_new->hide();
        ui->button_export->show();
        ensureLastProjectIsVisible();
        return;
    case model::ApplicationStateModel::Mode::Preview:
        ui->button_new->show();
        ui->button_export->show();
        ensureLastProjectIsVisible();
        return;
    }

    Q_UNREACHABLE();
}

void CameraLeftMenuForm::onProjectAdded(QSharedPointer<StageProject> project)
{
    auto newItem = new QListWidgetItem();
    newItem->setSizeHint(QSize(30, 65));

    auto itemWidget = new ProjectItemForm(list());
    itemWidget->setModel(m_model, project);

    list()->insertItem(list()->count(), newItem);
    list()->setItemWidget(newItem, itemWidget);
    list()->update();
    update();

    m_model->setSelectedProject(project);
    updateDisabledState();
}

void CameraLeftMenuForm::onProjectRemoved(QSharedPointer<StageProject> project)
{
    m_isRemovingItem = true;

    for(int i= 0; i < list()->count(); i++)
    {
        auto item = list()->item(i);
        auto itemWidget = list()->itemWidget(item);
        auto widget = dynamic_cast<ProjectItemForm*>(itemWidget);

        if (widget->projectModel() == project)
        {
            delete list()->takeItem(i);
            break;
        }
    }

    if (list()->count() == 0)
    {
        on_button_new_clicked();
    }
    else
    {
        auto index = list()->currentRow();
        m_model->setSelectedProject(m_model->projects()->at(index));
    }

    updateDisabledState();

    m_isRemovingItem = false;
}

void CameraLeftMenuForm::onSelectedProjectChanged(QSharedPointer<StageProject> selectedProject)
{
    if (m_model->selectedProject())
    {
        list()->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
        auto item = list()->item(m_model->projects()->items().indexOf(selectedProject));

        list()->setItemSelected(item, true);
        list()->scrollToItem(item);
        list()->setCurrentItem(item);
    }
    else
    {
        list()->setCurrentRow(-1);
        list()->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    }
}

void CameraLeftMenuForm::ensureLastProjectIsVisible()
{
    const auto currentRow = list()->currentRow();

    // If current project is selected and on the bottom make sure it is visible
    if (currentRow > 0 && (currentRow == m_model->projects()->count() - 1))
    {
        auto currentItem = list()->item(currentRow);

        if (list()->selectedItems().contains(currentItem))
        {
            list()->scrollToItem(currentItem);
            list()->setCurrentItem(currentItem);
        }
    }
}

void CameraLeftMenuForm::on_button_new_clicked()
{
    event::ChangeMatModeEvent::MatMode newMode;

    m_model->liveCapture()->inkData()->clear();

    // If reprojecting then revert to mode where last capture was done, otherwise honor the setting
    if (m_model->matModeState() == model::ApplicationStateModel::MatModeState::Reprojection ||
        // SPROUT-18817 - When application is becoming active the mode will be TransitioningToReprojection
        m_model->matModeState() == model::ApplicationStateModel::MatModeState::TransitioningToReprojection ||
        // Or we are returning in single screen where we don't have reprojection
        m_model->singleScreenMode())
    {
        newMode = CaptureModeTranslationTable[m_model->liveCapture()->preCaptureMode()];
    } else {
        // Force mode to cycle
        newMode = MatModeTranslationTable[m_model->matModeState()];
    }

    auto captureModeEvent = new event::ChangeMatModeEvent(newMode);
    captureModeEvent->dispatch();

    m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);
    this->unroll();
}

void CameraLeftMenuForm::on_button_export_clicked(bool checked)
{
    if (checked)
    {
        exportWidget->setFocus();
    }
}

void CameraLeftMenuForm::resizeEvent(QResizeEvent *event)
{
    SharedLeftMenuForm::resizeEvent(event);
    m_mask->setGeometry(this->geometry());
}

void CameraLeftMenuForm::onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode)
{
    m_mask->setVisible(mode == model::ApplicationStateModel::EditMenuMode::SubMenuOpen);
    m_mask->setGeometry(this->geometry());
}

} // namespace monitor
} // namespace capture
