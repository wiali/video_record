#include "project_item_form.h"
#include "ui_project_item_form.h"

#include <QMutex>
#include <QGestureEvent>
#include <QGesture>
#include <QTapAndHoldGesture>
#include <QTimer>

#include "styled_message_box.h"
#include "common/utilities.h"
#include "model/camera_item_metadata.h"
#include "event/change_invalid_project_name_visibility_event.h"
#include "global_utilities.h"
#include "common/history_manager.h"

namespace capture {
namespace monitor {

const QString SelectedStylesheet = "border: 2px solid #0096d6;";

ProjectItemForm::ProjectItemForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProjectItemForm)        
    , m_projectNameValidator(new components::ProjectNameValidator)
    , m_highligted(false)
    , mIsCollisionDialogOpen(false)
    , m_offscreenRenderer(new common::OffScreenRenderer(false))
{
    ui->setupUi(this);
    grabGesture(Qt::TapAndHoldGesture);

    connect(ui->projectName, &QLineEdit::editingFinished, this, &ProjectItemForm::onProjectNameTextConfirmed);
    connect(m_projectNameValidator.data(), &components::ProjectNameValidator::invalidInputDetected, this, &ProjectItemForm::onInvalidCharacterEntered);
    connect(m_offscreenRenderer.data(), &common::OffScreenRenderer::imageReady, this, &ProjectItemForm::onImageReady);

    // Use negative look-ahead to match invalid characters
    ui->projectName->setValidator(m_projectNameValidator.data());
    ui->projectName->installEventFilter(this);

    ui->deleteButton->hide();
}

ProjectItemForm::~ProjectItemForm()
{
    delete ui;
}

void ProjectItemForm::setModel(QSharedPointer<model::ApplicationStateModel> applicationStateModel, QSharedPointer<StageProject> projectModel)
{
    m_projectModel = projectModel;
    m_applicationStateModel = applicationStateModel;

    connect(m_projectModel.data(), &StageProject::thumbnailChanged, this, &ProjectItemForm::onThumbnailChanged);
    onThumbnailChanged(m_projectModel->thumbnail());

    connect(m_projectModel.data(), &StageProject::nameChanged, this, &ProjectItemForm::onProjectNameChanged);
    onProjectNameChanged(m_projectModel->name());

    auto item = m_projectModel->items().first();
    auto metadata = item->metadata().dynamicCast<model::CameraItemMetadata>();

    connect(metadata.data(), &model::CameraItemMetadata::selectedChanged, this, &ProjectItemForm::onSelectedChanged);
    onSelectedChanged(metadata->selected());

    connect(this, &ProjectItemForm::highligtedChanged, this, &ProjectItemForm::onHighlightedChanged);
    onHighlightedChanged(highligted());

    connect(applicationStateModel->liveCapture().data(), &model::LiveCaptureModel::captureStateChanged, this, &ProjectItemForm::onCaptureStateChanged);
    onCaptureStateChanged(applicationStateModel->liveCapture()->captureState());

    connect(m_applicationStateModel.data(), &model::ApplicationStateModel::inkPixMapChanged, this, &ProjectItemForm::onInkDataChanged);
    connect(m_applicationStateModel.data(), &model::ApplicationStateModel::modeChanged, this, &ProjectItemForm::onModeChanged);
    connect(m_applicationStateModel.data(), &model::ApplicationStateModel::editModeChanged, this, &ProjectItemForm::onEditModeChanged);

    connect(m_applicationStateModel.data(), &model::ApplicationStateModel::inkWidgetChanged, this, &ProjectItemForm::onUndoRedoChanged);
    ui->thumbnailLabel->setPixmap(m_renderedImage);
    onInkDataChanged();
}

void ProjectItemForm::onCaptureStateChanged(model::LiveCaptureModel::CaptureState captureState)
{
    onProjectNameTextConfirmed();
    disableNameEdit();

    ui->projectName->setEnabled(captureState == model::LiveCaptureModel::CaptureState::NotCapturing);
}

bool ProjectItemForm::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
    {
        auto gestureEvent = static_cast<QGestureEvent*>(event);

        if (QGesture *gesture = gestureEvent->gesture(Qt::TapAndHoldGesture))
        {
            auto tapAndHold = static_cast<QTapAndHoldGesture*>(gesture);
            auto localPosition = mapFromGlobal(tapAndHold->position().toPoint());

            if (ui->projectName->isReadOnly() && ui->projectName->geometry().contains(localPosition))
            {
                enableNameEdit();
            }
        }

        return true;
    }
    return QWidget::event(event);
}

bool ProjectItemForm::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->projectName) {
        switch(event->type()) {
            case QEvent::MouseButtonDblClick:
                if (ui->projectName->isReadOnly()) {
                    enableNameEdit();
                }
                break;
            case QEvent::FocusOut:
                onProjectNameTextConfirmed();
                disableNameEdit();
                return true;
                break;
        }
    }
    return false;
}

QSharedPointer<StageProject> ProjectItemForm::projectModel() const { return m_projectModel; }

void ProjectItemForm::onProjectNameChanged(QString name) {
    QFontMetrics metrics(ui->projectName->font());
    QString ellipsisText = metrics.elidedText(name, Qt::ElideRight, 260);
    ui->projectName->setText(ellipsisText);
}

void ProjectItemForm::onInvalidCharacterEntered() {
    auto position = ui->thumbnailLabel->mapToGlobal(ui->thumbnailLabel->geometry().bottomLeft());
    auto notificationVisibilityEvent = new event::ChangeInvalidProjectNameVisibilityEvent(true, position);

    notificationVisibilityEvent->dispatch();
}

void ProjectItemForm::enableNameEdit() {
    ui->projectName->setText(m_projectModel->name());
    ui->projectName->setStyleSheet(ui->projectName->styleSheet() + SelectedStylesheet);
    ui->projectName->setReadOnly(false);
    ui->projectName->setFocus();
    ui->projectName->selectAll();
}

void ProjectItemForm::disableNameEdit() {
    ui->projectName->setReadOnly(true);
    ui->projectName->setStyleSheet(ui->projectName->styleSheet().replace(SelectedStylesheet, ""));
    onProjectNameChanged(m_projectModel->name());
}

void ProjectItemForm::onProjectNameTextConfirmed()
{
    // Qt sends *two* editingFinished singals when Enter is pressed because why not
    if (!mIsCollisionDialogOpen) {
        mIsCollisionDialogOpen = true;

        auto projectName = ui->projectName->text().trimmed();

        if (projectName.length() == 0) {
            ui->projectName->setText(m_projectModel->name());
        } else {
            bool collision;
            bool hadCollision = false;

            do {
                collision = false;

                foreach(auto project, m_applicationStateModel->projects()->items())
                {
                    if (project != m_projectModel)
                    {
                        collision = collision ? collision : project->name() == projectName;
                    }
                }

                hadCollision = collision ? true : hadCollision;
                projectName = collision ? common::Utilities::createNonConflictingName(projectName) : projectName;

            } while (collision);

            if (hadCollision) {
                auto messageBox = common::Utilities::createMessageBox();

                QString title = tr("Rename image to %1?").arg(projectName);
                if (common::Utilities::measureTextWidth(messageBox->labelFont(), title) > 408) {
                    const auto font = messageBox->labelFont();
                    QFontMetrics metrics(font);
                    int projectNameWidth = 408 - common::Utilities::measureTextWidth(font, tr("Rename image to %1?").remove("%1"));
                    QString ellipsisText = metrics.elidedText(projectName, Qt::ElideMiddle, projectNameWidth);
                    title = tr("Rename image to %1?").arg(ellipsisText);
                    qDebug() << "The new name is too long so that elide the title to" << title;
                }

                messageBox->setTextFormat(Qt::RichText);
                messageBox->setText(title);
                messageBox->setInformativeText(tr("There is already an image with the same name in the camera roll."));

                messageBox->addStyledButton(tr("OK"), QMessageBox::AcceptRole);
                messageBox->addStyledButton(tr("Cancel"), QMessageBox::RejectRole);

                if (messageBox->exec() == QMessageBox::AcceptRole)
                {
                    m_projectModel->setName(projectName);
                }
            }
            else
            {
                m_projectModel->setName(ui->projectName->text());
            }

            auto notificationVisibilityEvent = new event::ChangeInvalidProjectNameVisibilityEvent(false, QPoint());
            notificationVisibilityEvent->dispatch();
        }

        setFocus();
        mIsCollisionDialogOpen = false;
    }
}

void ProjectItemForm::onThumbnailChanged(QImage thumbnail)
{
    // SPROUT-15718 - Scale thumbnail from original image
    m_renderedImage = QPixmap::fromImage(thumbnail.scaled(ui->thumbnailLabel->size(), Qt::KeepAspectRatio));
    onInkDataChanged();
}

bool ProjectItemForm::highligted() const
{
    return m_highligted;
}

void ProjectItemForm::onSelectedChanged(bool selected)
{
    ui->selectedLabel->setVisible(selected);
}

void ProjectItemForm::onHighlightedChanged(bool highligted)
{
    ui->deleteButton->setVisible(highligted && m_applicationStateModel->mode() == model::ApplicationStateModel::Mode::Preview);
    if (highligted)
    {
        ui->thumbnailLabel->setStyleSheet("QFrame {  border: 4px solid #0096d6; }");
    }
    else
    {
        ui->thumbnailLabel->setStyleSheet("QFrame {  border: 4px solid #7d7d7d; }");
    }
}

void ProjectItemForm::onModeChanged(model::ApplicationStateModel::Mode mode)
{
    ui->deleteButton->setVisible(m_highligted && mode == model::ApplicationStateModel::Mode::Preview);
}

void ProjectItemForm::mouseDoubleClickEvent(QMouseEvent* event)
{
    // Reserved for future use
    /*
    event->accept();

    auto item = m_model->items().first();
    auto metadata = item->metadata().dynamicCast<model::CameraItemMetadata>();
    metadata->setSelected(!metadata->selected());*/

    QWidget::mouseDoubleClickEvent(event);
}

void ProjectItemForm::setHighlighted(bool highligted)
{
    if (m_highligted != highligted)
    {
        m_highligted = highligted;

        emit highligtedChanged(m_highligted);
    }
}

void ProjectItemForm::on_deleteButton_clicked()
{
    common::Utilities::playSound("qrc:/Resources/production/Sounds/popDialog.aif");

    auto selectedProject = m_applicationStateModel->selectedProject();
    Q_ASSERT(selectedProject != nullptr);

    if (selectedProject)
    {
        emit m_applicationStateModel->editButtonChanged(false);
        auto messageBox = common::Utilities::createMessageBox();

        messageBox->setText(tr("Delete image"));
        messageBox->setInformativeText(tr("Are you sure you want to delete this image?"));

        messageBox->addStyledButton(tr("Delete"), QMessageBox::AcceptRole);
        messageBox->addStyledButton(tr("Cancel"), QMessageBox::RejectRole);

        if (messageBox->exec() != QMessageBox::RejectRole)
        {
            m_applicationStateModel->projects()->remove(m_projectModel);
        }
    }
    else
    {
        qCritical() << this << "Delete project clicked but no project is selected";
    }
}

void ProjectItemForm::onEditModeChanged(model::ApplicationStateModel::EditMenuMode editMode)
{
    //only exit the edit mode then update the thumbnail 
    //by new metadata change(doc mode, crop/rotate, adjustment)
    if (editMode == model::ApplicationStateModel::EditMenuMode::MainMenuClose)
    {
        if (m_projectModel == m_applicationStateModel->selectedProject() ||
            m_projectModel == m_currentProject)
        {
            QtConcurrent::run(this, &ProjectItemForm::updateThumbnail, true);
        }
    }
    else if (editMode == model::ApplicationStateModel::EditMenuMode::MainMenuOpen)
    {
        m_currentProject = m_applicationStateModel->selectedProject();
    }
}

void ProjectItemForm::updateThumbnail(bool forceUpdate)
{
    if (!forceUpdate && m_projectModel != m_applicationStateModel->selectedProject())
        return;

    QMutexLocker locker(&m_offscreenRendererMutex);
    auto item = m_projectModel->items().first();
    if (item)
    {
        m_offscreenRenderer->requestOffscreenImage(item);
        m_offscreenRendererWaitCondition.wait(&m_offscreenRendererMutex);
        m_projectModel->setThumbnail(m_lastOffscreenImage);
        m_renderedImage = QPixmap::fromImage(m_lastOffscreenImage.scaled(ui->thumbnailLabel->size(), Qt::KeepAspectRatio));
        updateInkData(true);
    }
}

void ProjectItemForm::onImageReady(QSharedPointer<StageItem> item, QImage image)
{
    m_lastOffscreenImage = image;
    m_offscreenRendererWaitCondition.wakeAll();
}

void ProjectItemForm::updateInkData(bool withInk)
{
    QPixmap imageWithInking = m_renderedImage;
    if (!withInk)
    {
        QPixmap inkPix = QPixmap::fromImage(m_offscreenRenderer->drawOnImage(m_renderedImage.toImage(), m_projectModel->items().first()));
        imageWithInking = m_offscreenRenderer->renderInkData(m_renderedImage, inkPix);
    }

    ui->thumbnailLabel->setPixmap(imageWithInking);
}

void ProjectItemForm::onInkDataChanged()
{
    updateInkData(false);
}

void ProjectItemForm::onUndoRedoChanged()
{
    QtConcurrent::run(this, &ProjectItemForm::updateThumbnail, false);
}

} // namespace monitor
} // namespace capture
