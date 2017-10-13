#include "keystone_corner_indicator_widget.h"

#include <QPainter>

#include "ui_keystone_corner_indicator_widget.h"

namespace capture {
namespace monitor {

KeystoneCornerIndicatorWidget::KeystoneCornerIndicatorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KeystoneCornerIndicatorWidget) {
    setStyleSheet("background: transparent;");
    setAttribute(Qt::WA_TranslucentBackground);

    ui->setupUi(this);
    ui->topLeftCorner->setIndex(0);
    ui->topLeftCorner->setNeighbors(ui->bottomLeftCorner, ui->topRightCorner);
    ui->topRightCorner->setIndex(1);
    ui->topRightCorner->setNeighbors(ui->topLeftCorner, ui->bottomRightCorner);
    ui->bottomRightCorner->setIndex(2);
    ui->bottomRightCorner->setNeighbors(ui->topRightCorner, ui->bottomLeftCorner);
    ui->bottomLeftCorner->setIndex(3);
    ui->bottomLeftCorner->setNeighbors(ui->bottomRightCorner, ui->topLeftCorner);

    for(auto child : children()) {
        if (auto corner = qobject_cast<DocumentSegmentationCorner*>(child)) {
            connect(corner, &DocumentSegmentationCorner::positionChanged, this, &KeystoneCornerIndicatorWidget::onCornerPositionChanged);
        }
    }
}

void KeystoneCornerIndicatorWidget::setModel(QSharedPointer<model::ApplicationStateModel> model, QSharedPointer<components::LiveVideoStreamCompositor> compositor) {
    m_model = model;
    m_compositor = compositor;

    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &KeystoneCornerIndicatorWidget::onVideoStreamStateChanged);
    onVideoStreamStateChanged(m_model->liveCapture()->videoStreamState());

    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &KeystoneCornerIndicatorWidget::updateVisibility);
    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::keystoneCorrectionModeChanged, this, &KeystoneCornerIndicatorWidget::updateVisibility);
    updateVisibility();

    auto keystoneCalibration = m_model->keystoneCalibration();

    connect(keystoneCalibration.data(), &model::KeystoneCalibrationModel::topLeftChanged, this, &KeystoneCornerIndicatorWidget::onTopLeftChanged);
    connect(keystoneCalibration.data(), &model::KeystoneCalibrationModel::topRightChanged, this, &KeystoneCornerIndicatorWidget::onTopRightChanged);
    connect(keystoneCalibration.data(), &model::KeystoneCalibrationModel::bottomLeftChanged, this, &KeystoneCornerIndicatorWidget::onBottomLeftChanged);
    connect(keystoneCalibration.data(), &model::KeystoneCalibrationModel::bottomRightChanged, this, &KeystoneCornerIndicatorWidget::onBottomRightChanged);
}

void KeystoneCornerIndicatorWidget::onVideoStreamStateChanged(model::LiveCaptureModel::VideoStreamState state) {
    if (state == model::LiveCaptureModel::VideoStreamState::Running) {
        const auto frameSize = m_compositor->frameSize();

        m_cornerOffsets[ui->topLeftCorner] = QPoint();
        m_cornerOffsets[ui->topRightCorner] = QPoint(frameSize.width(), 0);
        m_cornerOffsets[ui->bottomLeftCorner] = QPoint(0, frameSize.height());
        m_cornerOffsets[ui->bottomRightCorner] = QPoint(frameSize.width(), frameSize.height());

        recalculateTransform();
    }
}

void KeystoneCornerIndicatorWidget::updateVisibility() {
    if (m_model->mode() == model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration &&
        m_model->liveCapture()->keystoneCorrectionMode() == model::LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection) {
        recalculateTransform();
        show();
    } else {
        hide();
    }
}

void KeystoneCornerIndicatorWidget::resizeEvent(QResizeEvent *event) {
    recalculateTransform();
    QWidget::resizeEvent(event);
}

void KeystoneCornerIndicatorWidget::paintEvent(QPaintEvent *event) {
    if (m_cornerOffsets.count() > 0 &&
            m_model->liveCapture()->keystoneCorrectionMode() == model::LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection) {
        QWidget::paintEvent(event);

        const auto keystoneCalibration = m_model->keystoneCalibration();

        QPolygonF polygon;
        polygon.append(m_frameToCornersTransform.map(m_cornerOffsets[ui->topLeftCorner] + keystoneCalibration->topLeft()));
        polygon.append(m_frameToCornersTransform.map(m_cornerOffsets[ui->topRightCorner] + keystoneCalibration->topRight()));
        polygon.append(m_frameToCornersTransform.map(m_cornerOffsets[ui->bottomRightCorner] + keystoneCalibration->bottomRight()));
        polygon.append(m_frameToCornersTransform.map(m_cornerOffsets[ui->bottomLeftCorner] + keystoneCalibration->bottomLeft()));

        QPainter contour(this);
        contour.setPen(QPen(QColor(0x00, 0x96, 0xd6, 0xff), 2));
        contour.setBrush(QColor(0, 150, 214, 51));
        contour.setRenderHint(QPainter::Antialiasing);
        contour.drawPolygon(polygon, Qt::WindingFill);
    }
}

void KeystoneCornerIndicatorWidget::recalculateTransform() {
    const auto frameSize = m_compositor->frameSize();
    auto scaledSize = frameSize.scaled(size(), Qt::KeepAspectRatio);
    QRectF target(QPoint(), scaledSize);
    target.moveCenter(rect().center());

    QPolygonF source(QRectF (QPointF(), frameSize));
    source.removeLast();

    QPolygonF destination(target);
    destination.removeLast();

    QTransform::quadToQuad(source, destination, m_frameToCornersTransform);

    onTopLeftChanged();
    onTopRightChanged();
    onBottomLeftChanged();
    onBottomRightChanged();
}

QPoint KeystoneCornerIndicatorWidget::constraingCornerToFrame(const QPoint& position) {
    QPoint result(position);

    if (m_compositor) {
        const auto frameSize = m_compositor->frameSize();

        if (result.x() < 0) result.setX(0);
        if (result.y() < 0) result.setY(0);
        if (result.x() > frameSize.width()) result.setX(frameSize.width());
        if (result.y() > frameSize.height()) result.setY(frameSize.height());
    }

    return result;
}

void KeystoneCornerIndicatorWidget::onCornerPositionChanged(int cornerIndex, QRect& newCornerRect) {
    if (m_cornerOffsets.count() > 0) {
        const auto absoluteCoordinates = constraingCornerToFrame(m_frameToCornersTransform.inverted().map(newCornerRect.center()));
        auto keystoneCalibration = m_model->keystoneCalibration();

        switch (cornerIndex) {
        case 0:
            keystoneCalibration->setTopLeft(absoluteCoordinates - m_cornerOffsets[ui->topLeftCorner]);
            onTopLeftChanged();
            break;
        case 1:
            keystoneCalibration->setTopRight(absoluteCoordinates - m_cornerOffsets[ui->topRightCorner]);
            onTopRightChanged();
            break;
        case 2:
            keystoneCalibration->setBottomRight(absoluteCoordinates - m_cornerOffsets[ui->bottomRightCorner]);
            onBottomRightChanged();
            break;
        case 3:
            keystoneCalibration->setBottomLeft(absoluteCoordinates - m_cornerOffsets[ui->bottomLeftCorner]);
            onBottomLeftChanged();
            break;
        default:
            Q_UNREACHABLE();
        }

        update();
    }
}

void KeystoneCornerIndicatorWidget::moveCornerIndicator(DocumentSegmentationCorner* indicator, const QPointF& position) {
    if (m_cornerOffsets.contains(indicator)) {
        const auto absolutePosition = position + m_cornerOffsets[indicator];
        const auto transformed = m_frameToCornersTransform.map(absolutePosition).toPoint();
        const auto cornerHalfSize = QPoint(indicator->width() / 2, indicator->height() / 2);
        indicator->move(transformed - cornerHalfSize);
    }
}

void KeystoneCornerIndicatorWidget::onTopLeftChanged() {
    moveCornerIndicator(ui->topLeftCorner, m_model->keystoneCalibration()->topLeft());
}

void KeystoneCornerIndicatorWidget::onTopRightChanged() {
    moveCornerIndicator(ui->topRightCorner, m_model->keystoneCalibration()->topRight());
}

void KeystoneCornerIndicatorWidget::onBottomLeftChanged() {
    moveCornerIndicator(ui->bottomLeftCorner, m_model->keystoneCalibration()->bottomLeft());
}

void KeystoneCornerIndicatorWidget::onBottomRightChanged() {
    moveCornerIndicator(ui->bottomRightCorner, m_model->keystoneCalibration()->bottomRight());
}

} // namespace monitor
} // namespace capture
