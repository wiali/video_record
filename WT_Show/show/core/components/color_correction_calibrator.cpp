#include "color_correction_calibrator.h"

#include <QCoreApplication>

#include "common/measured_block.h"
#include "event/start_color_calibration_event.h"

namespace capture {
namespace components {

ColorCorrectionCalibrator::ColorCorrectionCalibrator(QSharedPointer<model::ApplicationStateModel> model,
                                                     QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                                                     QObject *parent)
    : QObject(parent)
    , m_threadPool(new QThreadPool)
    , m_model(model)
    , m_compositor(compositor) {
    setAutoDelete(false);
    QCoreApplication::instance()->installEventFilter(this);
}

bool ColorCorrectionCalibrator::eventFilter(QObject *obj, QEvent *event) {
    bool processed = false;

    if (event->type() == event::StartColorCalibrationEvent::type()) {
        if (auto startColorCalibrationEvent = static_cast<event::StartColorCalibrationEvent*>(event)) {
            m_threadPool->start(this);
            processed = true;
        }
    }

    return processed ? processed : QObject::eventFilter(obj, event);
}

void ColorCorrectionCalibrator::run() {
    QMutexLocker locker(&m_mutex);
    MEASURED_BLOCK

    m_model->setColorCalibrationStatus(model::ApplicationStateModel::ColorCalibrationStatus::CapturingImages);

    // Simulate some action
    QThread::sleep(2);

    m_model->setColorCalibrationStatus(model::ApplicationStateModel::ColorCalibrationStatus::PerformingCalibration);

    // Simulate some action
    QThread::sleep(2);

    m_model->setColorCalibrationStatus(model::ApplicationStateModel::ColorCalibrationStatus::CalibrationComplete);
}

} // namespace components
} // namespace capture

