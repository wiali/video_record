#include "stage_viewer_mirror.h"
#include "stage_viewer.h"

#include <QPainter>

namespace capture {
namespace presentation {

StageViewerMirror::StageViewerMirror(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_viewer(nullptr)
    , m_ink(nullptr)
    , m_scheduled(false)
{
    setStyleSheet(QStringLiteral("background-color: black;"));
}

void StageViewerMirror::setSources(StageViewer* viewer, common::InkLayerWidget* ink)
{
    if (m_viewer) {
        disconnect(m_viewer, 0, this, 0);
    }
    m_viewer = viewer;
    m_ink = ink;

    connect(m_viewer->renderer(), SIGNAL(frameSwapped()), this, SLOT(renderImage()));
    renderImage();
}

void StageViewerMirror::renderImage(bool force)
{
    if (!m_viewer) {
        m_image = QImage();
        update();
        return;
    }

    // Avoid updating image all the time.
    // This will cause a delay in the updates of presentation window but it enables inking.
    // TODO: Render image directly and not through m_image
    if (force || !m_timer.isValid() || m_timer.elapsed() > 200 && isVisible()) {
        m_timer.start();
        m_scheduled = false;
        m_image = m_viewer->renderer()->grabFramebuffer();
        m_ink->render(&m_image);
        update();
    }
    else {
        if (!m_scheduled) {
            QMetaObject::invokeMethod(this, "renderImage", Qt::QueuedConnection);
            m_scheduled = true;
        }
    }
}

void StageViewerMirror::paintEvent(QPaintEvent*)
{
    if (m_image.isNull()) {
        QPainter painter(this);
        painter.fillRect(this->rect(), Qt::black);
        return;
    }

    QRect target(QPoint(0, 0), m_image.size().scaled(this->size(), Qt::KeepAspectRatioByExpanding));
    target.moveCenter(rect().center());

    double scaleX = target.width() / (double) m_image.width();
    double scaleY = target.height() / (double) m_image.height();
    double scale = scaleX < scaleY ? scaleX : scaleY;

    QPainter painter(this);
    painter.scale(scale, scale);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawImage(target.topLeft(), m_image);
}

} // namespace presentation
} // namespace capture
