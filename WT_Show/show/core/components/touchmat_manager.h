#pragma once
#ifndef TOUCHMATMANAGER_H
#define TOUCHMATMANAGER_H

#include <QObject>

#include <touchmat/touchmat.h>
#include "model/application_state_model.h"

namespace capture {
namespace components {

class TouchmatManager : public QObject {
  Q_OBJECT
 public:
  explicit TouchmatManager(QSharedPointer<model::ApplicationStateModel> model, QObject *parent = 0);
  ~TouchmatManager();

  void onApplicationStateChanged(Qt::ApplicationState state);

 private slots:
  void updateTouchmatState();

 private:
  QSharedPointer<proapi::touchmat::Touchmat> m_touchmat;
  QSharedPointer<model::ApplicationStateModel> m_model;
  bool m_originalTouchState;
};

} // namespace components
} // namespace capture

#endif  // TOUCHMATMANAGER_H
