#pragma once
#ifndef ZOOM_INDICATOR_FORM_H
#define ZOOM_INDICATOR_FORM_H

#include <QWidget>

#include "model/application_state_model.h"
#include "fade_out_notification.h"

namespace capture {
namespace monitor {

class ZoomIndicatorForm : public FadeOutNotification
{
    Q_OBJECT

public:
    explicit ZoomIndicatorForm(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onViewportChanged(QRectF viewport);

private:
    QString zoomText(QRectF viewport);

    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // ZOOM_INDICATOR_FORM_H
