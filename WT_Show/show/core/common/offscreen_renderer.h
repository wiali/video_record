#pragma once
#ifndef OFFSCREENRENDERER_H
#define OFFSCREENRENDERER_H

#include <QObject>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QOpenGLFunctions>
#include <QQueue>

#include "ink_data.h"
#include "stage_item.h"

namespace capture {
namespace common {

class OffScreenRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit OffScreenRenderer(bool multiSampling, QObject *parent = 0);

    QPixmap renderInkData(const QPixmap& image, const QPixmap& inkPixmap);
    QImage drawOnImage(const QImage &image, QSharedPointer<StageItem> stageItem);
signals:

    void imageReady(QSharedPointer<StageItem> item, QImage image);

public slots:
    void requestOffscreenImage(QSharedPointer<StageItem> item);

private:
    QQueue<QSharedPointer<StageItem>> m_queue;
    bool m_multiSampling;

private slots:
     void generateOffscreenImage();
};

} // namespace common
} // namespace capture

#endif // OFFSCREENRENDERER_H
