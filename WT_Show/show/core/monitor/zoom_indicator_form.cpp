#include "zoom_indicator_form.h"

#include <global_utilities.h>

namespace capture {
namespace monitor {

ZoomIndicatorForm::ZoomIndicatorForm(QWidget *parent)
    : FadeOutNotification(parent) {
    auto settings = GlobalUtilities::applicationSettings("zoom_indicator");
    setDuration(settings->value("fade_out_timeout_ms", 3000).toInt());
}

void ZoomIndicatorForm::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;
    const auto liveCapture = m_model->liveCapture();

    setText(zoomText(QRectF(0, 0, 1, 1)));

    connect(liveCapture.data(), &model::LiveCaptureModel::viewportChanged, this, &ZoomIndicatorForm::onViewportChanged);
    connect(m_model->postCapture().data(), &model::PostCaptureModel::viewportChanged, this, &ZoomIndicatorForm::onViewportChanged);
}

QString ZoomIndicatorForm::zoomText(QRectF viewport) {
    auto zoom = 1.0 / std::min(viewport.width(), viewport.height());
    zoom = qRound(zoom * 100.0) / 100.0;

    return zoom >= 1.0 ? tr("%1x").arg(QString::number(zoom, 'f', 1)) : QString();
}

void ZoomIndicatorForm::onViewportChanged(QRectF viewport) {
    const auto newText = zoomText(viewport);

    if (!newText.isNull() && newText != text()) {
        m_fadeOutAnimation->stop();
        m_fadeOutAnimation->start();

        setText(newText);
    }
}

} // namespace monitor
} // namespace capture
