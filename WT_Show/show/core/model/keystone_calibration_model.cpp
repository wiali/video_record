#include "keystone_calibration_model.h"

#include <QDebug>

namespace capture {
namespace model {

KeystoneCalibrationModel::KeystoneCalibrationModel(QObject *parent)
    : QObject(parent)
    , m_preCalibrationCorrectionMode(LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::model::KeystoneCalibrationModel::Status>();
        }
    } initialize;
}

void KeystoneCalibrationModel::setStatus(KeystoneCalibrationModel::Status status) {
    if (m_status != status) {
        m_status = status;

        qInfo() << this << "Status changed to" << m_status;

        emit statusChanged(m_status);
    }
}

void KeystoneCalibrationModel::setTopLeft(const QPoint& topLeft) {
    if (m_topLeft != topLeft) {
        m_topLeft = topLeft;

        emit topLeftChanged(m_topLeft);
    }
}

void KeystoneCalibrationModel::setTopRight(const QPoint& topRight) {
    if (m_topRight != topRight) {
        m_topRight = topRight;

        emit topRightChanged(m_topRight);
    }
}

void KeystoneCalibrationModel::setBottomLeft(const QPoint& bottomLeft) {
    if (m_bottomLeft != bottomLeft) {
        m_bottomLeft = bottomLeft;

        emit bottomLeftChanged(m_bottomLeft);
    }
}

void KeystoneCalibrationModel::setBottomRight(const QPoint& bottomRight) {
    if (m_bottomRight != bottomRight) {
        m_bottomRight = bottomRight;

        emit bottomRightChanged(m_bottomRight);
    }
}

void KeystoneCalibrationModel::setPreCalibrationCorrectionMode(LiveCaptureModel::KeystoneCorrectionMode mode) {
    if (m_preCalibrationCorrectionMode != mode) {
        m_preCalibrationCorrectionMode = mode;

        qInfo() << this << "Pre-calibration calibration correction mode changed to" << m_preCalibrationCorrectionMode;

        emit preCalibrationCorrectionModeChanged(m_preCalibrationCorrectionMode);
    }
}

} // namespace model
} // namespace capture
