#pragma once
#ifndef ABSTRACT_CAMERA_BUTTON_H
#define ABSTRACT_CAMERA_BUTTON_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class CameraButton : public RightMenuButton
{
    Q_OBJECT
public:
    explicit CameraButton(QWidget *parent = nullptr);

    void setModel(QVector<common::VideoSourceInfo> videoSources,
                  QSharedPointer<model::ApplicationStateModel> model);
    void setModel(const common::VideoSourceInfo& videoSource,
                  QSharedPointer<model::ApplicationStateModel> model);

protected slots:

    void updateStyle();
    void onButtonClicked();
    void onSelectedVideoStreamSourcesChanged(QVector<common::VideoSourceInfo> selectedCameras);

protected:
    QSharedPointer<model::ApplicationStateModel> m_model;
    QVector<common::VideoSourceInfo> m_videoSources;

    bool isThisLastSource();
    void updateDisabledState();
    void selectCameras(QVector<common::VideoSourceInfo> cameras);
};

} // namespace monitor
} // namespace capture

#endif // ABSTRACT_CAMERA_BUTTON_H
