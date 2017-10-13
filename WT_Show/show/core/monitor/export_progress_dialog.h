#pragma once
#ifndef EXPORT_PROGRESS_DIALOG_H
#define EXPORT_PROGRESS_DIALOG_H

#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "model/projects_export_model.h"

namespace Ui {
class ExportProgressDialog;
}

namespace capture {
namespace monitor {

class ExportProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportProgressDialog(QWidget *parent = 0);
    ~ExportProgressDialog();

    void setModel(QSharedPointer<model::ProjectsExportModel> model);

private slots:
    void onStateChanged(model::ProjectsExportModel::State state);
    void onIndexChanged();

private:
    Ui::ExportProgressDialog *ui;
    QScopedPointer<QGraphicsOpacityEffect> m_opacityEffect;
    QScopedPointer<QPropertyAnimation> m_fadeOutAnimation;

    QSharedPointer<model::ProjectsExportModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif  // EXPORT_PROGRESS_DIALOG_H
