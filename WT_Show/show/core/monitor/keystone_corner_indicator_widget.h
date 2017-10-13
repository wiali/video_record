#ifndef KEYSTONE_CORNER_INDICATOR_WIDGET_H
#define KEYSTONE_CORNER_INDICATOR_WIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QPaintEvent>
#include "model/application_state_model.h"
#include "components/live_video_stream_compositor.h"

#include <document_segmentation_corner.h>

namespace Ui {
class KeystoneCornerIndicatorWidget;
}

namespace capture {
namespace monitor {

class KeystoneCornerIndicatorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeystoneCornerIndicatorWidget(QWidget *parent = nullptr);

    void setModel(QSharedPointer<model::ApplicationStateModel> model, QSharedPointer<components::LiveVideoStreamCompositor> compositor);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

private slots:
    void onVideoStreamStateChanged(model::LiveCaptureModel::VideoStreamState state);
    void onCornerPositionChanged(int cornerIndex, QRect& newCornerRect);
    void updateVisibility();
    void onTopLeftChanged();
    void onTopRightChanged();
    void onBottomLeftChanged();
    void onBottomRightChanged();

private:
    void recalculateTransform();
    void moveCornerIndicator(DocumentSegmentationCorner* indicator, const QPointF& position);
    QPoint constraingCornerToFrame(const QPoint& position);

    QSharedPointer<model::ApplicationStateModel> m_model;
    QSharedPointer<components::LiveVideoStreamCompositor> m_compositor;
    QTransform m_frameToCornersTransform;
    QScopedPointer<Ui::KeystoneCornerIndicatorWidget> ui;
    QHash<DocumentSegmentationCorner*, QPoint> m_cornerOffsets;
};

} // namespace monitor
} // namespace capture


#endif // KEYSTONE_CORNER_INDICATOR_WIDGET_H
