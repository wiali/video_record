#pragma once
#ifndef CAPTUREMODESWITCHWIDGET_H
#define CAPTUREMODESWITCHWIDGET_H

#include <QWidget>

#include "event/change_mat_mode_event.h"
#include "model/application_state_model.h"

namespace Ui {
class CaptureModeWidget;
}

namespace capture {
namespace monitor {

class CaptureModeWidget : public QWidget {
  Q_OBJECT

 public:
  typedef model::ApplicationStateModel::MainWindowLocation WindowLocation;
  explicit CaptureModeWidget(QWidget *parent = 0);
  ~CaptureModeWidget();

  void setModel(QSharedPointer<model::ApplicationStateModel> model);

 private slots:
  void onMatModeStateChanged(capture::model::ApplicationStateModel::MatModeState state,
                             capture::model::ApplicationStateModel::MatModeState oldState);
  void onMatEnabledChanged(WindowLocation zOrder);
  void onApplicationModeChanged(capture::model::ApplicationStateModel::Mode mode);

  void updateEnabled();
  void moveBallTo(int y, bool animate);  

  void on_desktopButton_clicked();
  void on_lampOnButton_clicked();
  void on_lampOffButton_clicked();
  void on_reprojectionButton_clicked();

 private:
  QScopedPointer<Ui::CaptureModeWidget> ui;

  void requestModeSwitch(event::ChangeMatModeEvent::MatMode mode);
  void setHeight(int height);

  QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif  // CAPTUREMODESWITCHWIDGET_H
