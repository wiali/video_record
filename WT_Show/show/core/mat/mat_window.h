#pragma once
#ifndef MATWINDOW_H
#define MATWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>
#include <QScreen>

#include "model/application_state_model.h"
#include "ink_button.h"
#include "components/video_source_input.h"
#include "common/base_window.h"

#include <qt_windows.h>

namespace Ui {
class MatWindow;
}

namespace capture {
namespace mat {

class MatWindow : public capture::common::BaseWindow
{
    Q_OBJECT

public:
    explicit MatWindow(QSharedPointer<model::ApplicationStateModel> model,
                       QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                       QWidget *parent = 0);
    ~MatWindow();

signals:

    void closing();

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, long* result);
    void closeEvent(QCloseEvent *event);
    virtual QScreen* findOwnScreen() override;
    virtual StageViewer::Options stageViewerOptions() override;
    virtual void applySelectedProject(QSharedPointer<StageProject> selectedProject) override;

signals:
    void penPressDown(const POINTER_PEN_INFO& penInfo);
    void penPressUp(const POINTER_PEN_INFO& penInfo);
    void penHoverEntered(const POINTER_PEN_INFO& penInfo);
    void penHoverExited(const POINTER_PEN_INFO& penInfo);
    void penMove(const POINTER_PEN_INFO& penInfo);

protected slots:
    bool eventFilter(QObject *obj, QEvent *event);
    virtual void onDisplayCountChanged(int screenCount) override;

private slots:
    void onScreenGeometryChanged(const QRect &geometry);
    void onViewportChanged();
    void onModeChanged();
    void onMatModeStateChanged(capture::model::ApplicationStateModel::MatModeState state);
    void onMonitorWindowMinimizedChanged(bool monitorWindowMinimized);
    void onCaptureStateChanged();
    void onPerspectiveQuadChanged();
    void onInkWidgetChanged();    
    void onMetadataChanged();
    void onSelectedVideoStreamSourcesChanged();
    void showInk();
    void onInkPointAdded(const QPoint& point, double width);

private:
    void MatWindow::calcInkWidgetSize(const QSize& scaledImageSize);

private:
    Ui::MatWindow *ui;

    QSharedPointer<model::ApplicationStateModel> m_model;

    QHash<QString, Qt::GlobalColor> m_colorTranslationTable;
    QSharedPointer<InkWidget> m_inkWidget;
    QScopedPointer<InkButton> m_inkButton;
    bool m_inkStart;
    QVector<QMetaObject::Connection> m_itemConnections;
    QTransform m_transform;
    QSharedPointer<StageProject> m_stageProject;
    QSharedPointer<components::LiveVideoStreamCompositor> m_compositor;
    QSharedPointer<InkData> m_inkData;
};

} // namespace mat
} // namespace capture

#endif // MATWINDOW_H
