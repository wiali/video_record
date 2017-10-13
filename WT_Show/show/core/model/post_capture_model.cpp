#include "post_capture_model.h"

namespace capture {
namespace model {

PostCaptureModel::PostCaptureModel(QObject *parent)
    : QObject(parent){}

void PostCaptureModel::setViewport(QRectF viewport) {
    if (m_viewport != viewport) {
        m_viewport = viewport;
        emit viewportChanged(m_viewport);
    }
}

} // namespace model
} // namespace capture

