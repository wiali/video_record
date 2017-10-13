#pragma once
#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <QObject>
#include <QRectF>

#include <video_source/sourcepipeline.h>
#include "common/video_source_info.h"

namespace capture {
namespace model {

class VideoStreamSourceModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool illuminationCorrectionEnabled READ illuminationCorrectionEnabled WRITE setIlluminationCorrectionEnabled NOTIFY illuminationCorrectionEnabledChanged)
    Q_PROPERTY(bool autoWhiteBalanceEnabled READ autoWhiteBalanceEnabled WRITE setAutoWhiteBalanceEnabled NOTIFY autoWhiteBalanceEnabledChanged)
    Q_PROPERTY(model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode READ colorCorrectionMode WRITE setColorCorrectionMode NOTIFY colorCorrectionModeChanged)
    Q_PROPERTY(int skipFrameCount READ skipFrameCount WRITE setSkipFrameCount)
    Q_PROPERTY(int frameFreezeDetectionTimeout READ frameFreezeDetectionTimeout WRITE setFrameFreezeDetectionTimeout)
    Q_PROPERTY(QString pipelineName READ pipelineName WRITE setPipelineName)

public:
    enum class ColorCorrectionMode {
        None,
        LampOn,
        LampOff
    };

    Q_ENUM(ColorCorrectionMode)

    explicit VideoStreamSourceModel(capture::common::VideoSourceInfo videoSource, QObject *parent = 0);

    inline capture::common::VideoSourceInfo videoSource() const { return m_videoSource; }
    inline bool illuminationCorrectionEnabled() const { return m_illuminationCorrectionEnabled; }
    inline bool autoWhiteBalanceEnabled() const { return m_autoWhiteBalanceEnabled; }
    inline model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode() const { return m_colorCorrectionMode; }
    inline int skipFrameCount() const { return m_skipFrameCount; }
    inline int frameFreezeDetectionTimeout() const { return m_frameFreezeDetectionTimeout; }
    inline QString pipelineName() const { return m_pipelineName; }

signals:
    void illuminationCorrectionEnabledChanged(bool isEnabled);
    void autoWhiteBalanceEnabledChanged(bool isEnabled);
    void colorCorrectionModeChanged(capture::model::VideoStreamSourceModel::ColorCorrectionMode mode);

public slots:
    inline void setSkipFrameCount(int skipFrameCount) { m_skipFrameCount = skipFrameCount; }
    inline void setPipelineName(QString pipelineName) { m_pipelineName = pipelineName; }
    inline void setFrameFreezeDetectionTimeout(int frameFreezeDetectionTimeout) { m_frameFreezeDetectionTimeout = frameFreezeDetectionTimeout; }

    void setIlluminationCorrectionEnabled(bool isEnabled);
    void setAutoWhiteBalanceEnabled(bool isEnabled);
    void setColorCorrectionMode(capture::model::VideoStreamSourceModel::ColorCorrectionMode mode);
private:
    bool m_illuminationCorrectionEnabled;
    bool m_autoWhiteBalanceEnabled;
    model::VideoStreamSourceModel::ColorCorrectionMode m_colorCorrectionMode;
    capture::common::VideoSourceInfo m_videoSource;
    int m_skipFrameCount;
    int m_frameFreezeDetectionTimeout;
    QString m_pipelineName;
};

} // namespace model
} // namespace capture

#endif // CAMERAMODEL_H
