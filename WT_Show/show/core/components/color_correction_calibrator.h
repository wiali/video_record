#pragma once
#ifndef COLORCORRECTIONCALIBRATOR_H
#define COLORCORRECTIONCALIBRATOR_H

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <QMutex>

#include <video_source/videopipeline.h>

#include "components/live_video_stream_compositor.h"
#include "model/application_state_model.h"

namespace capture {
namespace components {

class ColorCorrectionCalibrator : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ColorCorrectionCalibrator(QSharedPointer<model::ApplicationStateModel> model, QSharedPointer<components::LiveVideoStreamCompositor> compositor, QObject *parent = 0);
    virtual void run() override;

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:

    QMutex m_mutex;
    QScopedPointer<QThreadPool> m_threadPool;
    QSharedPointer<components::LiveVideoStreamCompositor> m_compositor;
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace components
} // namespace capture

#endif // COLORCORRECTIONCALIBRATOR_H
