#pragma once
#ifndef FORWARDCAMERABUTTON_H
#define FORWARDCAMERABUTTON_H

#include <QObject>
#include "camera_button.h"

namespace capture {
namespace monitor {

class ForwardFacingCameraButton : public CameraButton
{
    Q_OBJECT
public:
    explicit ForwardFacingCameraButton(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:

    void onVideoStreamSourcesChanged(QVector<QSharedPointer<capture::model::VideoStreamSourceModel>> videoStreamSources);
};

} // namespace monitor
} // namespace capture

#endif // FORWARDCAMERABUTTON_H
