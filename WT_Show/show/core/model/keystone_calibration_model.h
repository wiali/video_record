#ifndef KEYSTONE_CALIBRATION_MODEL_H
#define KEYSTONE_CALIBRATION_MODEL_H

#include <QObject>

#include "live_capture_model.h"

namespace capture {
namespace model {

class KeystoneCalibrationModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(KeystoneCalibrationModel::Status status READ status
               WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(LiveCaptureModel::KeystoneCorrectionMode preCalibrationCorrectionMode READ preCalibrationCorrectionMode
               WRITE setPreCalibrationCorrectionMode NOTIFY preCalibrationCorrectionModeChanged)
public:
    explicit KeystoneCalibrationModel(QObject *parent = nullptr);

    enum class Status { NotCalibrating, CapturingImages, PerformingCalibration, CalibrationComplete };

    Q_ENUM(Status)

    inline KeystoneCalibrationModel::Status status() const { return m_status; }
    inline QPoint topLeft() const { return m_topLeft; }
    inline QPoint topRight() const { return m_topRight; }
    inline QPoint bottomLeft() const { return m_bottomLeft; }
    inline QPoint bottomRight() const { return m_bottomRight; }
    inline LiveCaptureModel::KeystoneCorrectionMode preCalibrationCorrectionMode() const { return m_preCalibrationCorrectionMode; }
signals:

    void statusChanged(capture::model::KeystoneCalibrationModel::Status status);
    void topLeftChanged(const QPoint& topLeft);
    void topRightChanged(const QPoint& topRight);
    void bottomLeftChanged(const QPoint& bottomLeft);
    void bottomRightChanged(const QPoint& bottomRight);
    void preCalibrationCorrectionModeChanged(capture::model::LiveCaptureModel::KeystoneCorrectionMode mode);

public slots:

    void setStatus(capture::model::KeystoneCalibrationModel::Status status);
    void setTopLeft(const QPoint& topLeft);
    void setTopRight(const QPoint& topRight);
    void setBottomLeft(const QPoint& bottomLeft);
    void setBottomRight(const QPoint& bottomRight);
    void setPreCalibrationCorrectionMode(capture::model::LiveCaptureModel::KeystoneCorrectionMode mode);

private:
    Status m_status;
    QPoint m_topLeft;
    QPoint m_topRight;
    QPoint m_bottomLeft;
    QPoint m_bottomRight;
    LiveCaptureModel::KeystoneCorrectionMode m_preCalibrationCorrectionMode;
};

} // namespace model
} // namespace capture

#endif // KEYSTONE_CALIBRATION_MODEL_H
