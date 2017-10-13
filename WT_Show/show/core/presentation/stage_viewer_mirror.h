#pragma once
#ifndef STAGE_VIEWER_MIRROR_H
#define STAGE_VIEWER_MIRROR_H

#include "stage_viewer.h"
#include "common/ink_layer_widget.h"

#include <QOpenGLWidget>
#include <QElapsedTimer>

namespace capture {
namespace presentation {

class StageViewerMirror : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit StageViewerMirror(QWidget *parent = 0);
    void setSources(StageViewer* viewer, common::InkLayerWidget* ink);

public Q_SLOTS:
    void renderImage(bool force = false);

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    StageViewer* m_viewer;
    common::InkLayerWidget* m_ink;
    QElapsedTimer m_timer;
    QImage m_image;
    bool m_scheduled;
};

} // namespace presentation
} // namespace capture

#endif // STAGE_VIEWER_MIRROR_H
