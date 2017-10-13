#pragma once
#ifndef VIDEOSOURCEINPUT_H
#define VIDEOSOURCEINPUT_H

#include <QObject>
#include <QMutex>
#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#include <QTimer>

#include <image_filters/imagefilter.h>
#include <video_source/videopipeline.h>

#include "model/application_state_model.h"
#include "live_video_stream_compositor.h"

namespace capture {
namespace components {

class VideoSourceInput : public QObject
{
    Q_OBJECT


public:
    explicit VideoSourceInput(QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                              QSharedPointer<image_filters::ImageFilter> imageFilter,
                              QSharedPointer<model::ApplicationStateModel> model,
                              QObject *parent = nullptr);
    ~VideoSourceInput();

    QSharedPointer<model::VideoStreamSourceModel> cameraModel() const;

protected:

    virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
    void isEnabledChanged(bool isEnabled);

private:
    void start(QVector<common::VideoSourceInfo> videoSources);
    void stop();

    QSharedPointer<components::LiveVideoStreamCompositor> m_compositor;
    QSharedPointer<image_filters::ImageFilter> m_imageFilter;

    QSharedPointer<model::ApplicationStateModel> m_model;

    QVector<QMetaObject::Connection> m_sourceConnections;
    QStringList m_needStreamedFrame;

private slots:

    void onPipelineStateChanged();    
    void onAppStateChanged(Qt::ApplicationState state);    
    void onStreamFrozen(capture::common::VideoSourceInfo videoSource);
    void onCompositorUpdated();
};

} // namespace components
} // namespace capture

Q_DECLARE_METATYPE(capture::components::VideoSourceInput*)

#endif // VIDEOSOURCEINPUT_H
