#pragma once
#ifndef EXPORT_WIDGET_H
#define EXPORT_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include "event/export_project_items_event.h"
#include "right_submenu_base_widget.h"
#include "model/application_state_model.h"

namespace Ui {
class ExportWidget;
}

namespace capture {
namespace monitor {

class ExportWidget : public RightSubMenuBaseWidget
{
    Q_OBJECT

public:
    explicit ExportWidget(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

    int getThisArrowTop() const override;
    void moveToTarget() override;

protected:

    void paintBorderWithArrow(QPaintEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

    void exportAll(QString location, event::ExportProjectItemsEvent::Format format);

private slots:
    void on_saveAsPNGButton_clicked();
    void on_saveAsJPGButton_clicked();
    void on_exportToPDFButton_clicked();
    void on_sendAllToStageButton_clicked();
    void on_ocrToPDFButton_clicked();

    int calculateMaxTextWidth();
private:
    QScopedPointer<Ui::ExportWidget> ui;
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture


#endif // EXPORT_WIDGET_H

