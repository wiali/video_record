#include "export_progress_dialog.h"
#include "ui_export_progress_dialog.h"

#include <global_utilities.h>

#include "common/utilities.h"

namespace capture {
namespace monitor {

ExportProgressDialog::ExportProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportProgressDialog)
  , m_opacityEffect(new QGraphicsOpacityEffect(this))
  , m_fadeOutAnimation(new QPropertyAnimation(this))
{
    ui->setupUi(this);

    m_opacityEffect->setOpacity(1);
    setGraphicsEffect(m_opacityEffect.data());

    m_fadeOutAnimation->setTargetObject(m_opacityEffect.data());
    m_fadeOutAnimation->setPropertyName("opacity");

    auto settings = GlobalUtilities::applicationSettings("export_progress_dialog");

    m_fadeOutAnimation->setDuration(settings->value("fade_out_timeout_ms", 1000).toInt());
    m_fadeOutAnimation->setStartValue(1);
    m_fadeOutAnimation->setEndValue(0);
    m_fadeOutAnimation->setKeyValueAt(0.5, 1);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::InQuint);

    connect(m_fadeOutAnimation.data(), &QPropertyAnimation::finished, this, &QDialog::close);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    if(GlobalUtilities::isRightToLeft())
    {
        ui->progressBar->setLayoutDirection(Qt::RightToLeft);
    }
}

ExportProgressDialog::~ExportProgressDialog()
{
    delete ui;
}

void ExportProgressDialog::setModel(QSharedPointer<model::ProjectsExportModel> model)
{
    m_model = model;

    connect(m_model.data(), &model::ProjectsExportModel::stateChanged, this, &ExportProgressDialog::onStateChanged);
    onStateChanged(m_model->state());

    connect(m_model.data(), &model::ProjectsExportModel::indexChanged, this, &ExportProgressDialog::onIndexChanged);
    connect(m_model.data(), &model::ProjectsExportModel::countChanged, this, &ExportProgressDialog::onIndexChanged);

    onIndexChanged();
}

void ExportProgressDialog::onIndexChanged()
{
    ui->progressBar->setMaximum(m_model->count() + 1);
    ui->progressBar->setValue(m_model->index() + 1);

    if(m_model->index() != m_model->count())
    {
        if(GlobalUtilities::contryCode() == "ar")
        {
            ui->progressLabel->setText(QString("%1/%2")
                                       .arg(GlobalUtilities::convertArabicNumber(m_model->count()))
                                       .arg(GlobalUtilities::convertArabicNumber(m_model->index() + 1)));
        }
        else
        {
            ui->progressLabel->setText(tr("%1/%2").arg(m_model->index() + 1).arg(m_model->count()));
        }
    }
}

void ExportProgressDialog::onStateChanged(model::ProjectsExportModel::State state)
{
    switch(state) {
    case model::ProjectsExportModel::State::NotExporting:
        return;
    case model::ProjectsExportModel::State::PrepairingToExport:
        ui->progressLabel->setVisible(false);

        // Show Busy indicator while preparing
        ui->progressBar->setMaximum(0);
        ui->progressBar->setMinimum(0);
        ui->progressBar->setValue(0);
        return;
    case model::ProjectsExportModel::State::Exporting:
        onIndexChanged();
        ui->progressLabel->setVisible(true);
        setVisible(true);
        return;
    case model::ProjectsExportModel::State::PerformingOcr: {
        ui->progressBar->setMaximum(0);
        ui->progressBar->setMinimum(0);
        ui->progressBar->setValue(0);

        ui->progressLabel->setText(tr("Performing optical character recognition"));
        return;
    }
    case model::ProjectsExportModel::State::FinalizingExport:
        m_fadeOutAnimation->start();
        common::Utilities::playSound("qrc:/Resources/production/Sounds/exportFinished.wav");
        return;
    }

    Q_UNREACHABLE();
}

} // namespace monitor
} // namespace capture
