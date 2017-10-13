#include "offscreen_renderer.h"

#include <QThread>

#include "stage_renderer.h"
#include "global_utilities.h"
#include "scalable_rect.h"
#include "ink_manger.h"

namespace capture {
namespace common {

struct OffscreenSurface
{
    OffscreenSurface()
    {
        context.create();
        surface.setFormat(context.format());
        surface.create();
        context.makeCurrent(&surface);
    }

    ~OffscreenSurface()
    {
        context.doneCurrent();
    }

private:
    QOffscreenSurface surface;
    QOpenGLContext context;
};


OffScreenRenderer::OffScreenRenderer(bool multiSampling, QObject *parent) 
    : m_multiSampling(multiSampling), QObject(parent)
{

}

// This method MUST run on main thread because it is trying to create new context
void OffScreenRenderer::generateOffscreenImage() {
    if (!m_queue.isEmpty()) {
        auto stageItem = m_queue.dequeue();

        // First create an empty StageItem;
        auto metadata = QSharedPointer<EditableItemMetadata>::create();
        QSharedPointer<StageItem> exportItem = QSharedPointer<StageItem>::create(QImage(), metadata);

        // Clone all changes but override image mode, geometry and viewport to ensure complete image is exported.
        exportItem->clone(*stageItem);
        QRect region = exportItem->imageRect();
        exportItem->setImageMode(StageItem::ImageMode::Transparent);
        metadata->setGeometry(QRect(QPoint(0, 0), region.size()));
        metadata->setViewport(QRectF(0, 0, 1, 1));

        QImage image;
        {
            OffscreenSurface surface;
            // Create the new StageRenderer and configure it.
            StageRenderer offScreenStageRenderer;
            offScreenStageRenderer.initialize();
            offScreenStageRenderer.updateStageSize(region.size());
            offScreenStageRenderer.getRenderer()->setViewport(A3D::Viewport(0, 0, region.width(), region.height()));
            offScreenStageRenderer.getCamera()->setRealVpSize(region.size());

            // Add the model
            QList<QSharedPointer<StageItem>> items;
            items << exportItem;
            offScreenStageRenderer.addItems(items);

            //create the off screen frame buffer
            QOpenGLFramebufferObjectFormat format;
            //for export image we need to do multiple sampling, but if just for thumbnail we don`t need.
            //for 4K image, with multiple sampling, it takes about 1s, but only need 200ms without it.
            if (m_multiSampling)
            {
                format.setSamples(16);
                format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            }

            QOpenGLFramebufferObject renderFbo(region.width(), region.height(), format);
            renderFbo.bind();

            //render scene to paint device for Image
            QOpenGLPaintDevice paintFbo(region.width(), region.height());
            offScreenStageRenderer.renderScene();

            //get the image from frame buffer
            image = renderFbo.toImage();

            renderFbo.release();

            // We must release textures before OpenGL context is lost
            offScreenStageRenderer.reset();
        }


        auto meta = stageItem->metadata().dynamicCast<EditableItemMetadata>();
        QSizeF perspectiveSize = ScalableRect::perspectiveSize(meta->correctedPerspectiveQuad());
        QSizeF scalePerspectiveSize = perspectiveSize.scaled(QSizeF(1.0, 1.0), Qt::KeepAspectRatio);
        if(perspectiveSize != QSizeF(1.0, 1.0))
        {
            QSize imageSize = stageItem->image().size();
            QSize realSize(meta->geometry().width(), meta->geometry().height());
            realSize = realSize.scaled(imageSize, Qt::KeepAspectRatio);
            double ratio = perspectiveSize.width() / scalePerspectiveSize.width();
            QSize dstSize(static_cast<int>(realSize.width() * ratio),
                          static_cast<int>(realSize.height() * ratio));
            image = image.scaled(dstSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        image = drawOnImage(image, stageItem);

        emit imageReady(stageItem, image);
    }
}

void OffScreenRenderer::requestOffscreenImage(QSharedPointer<StageItem> item)
{
    m_queue.enqueue(item);
    QMetaObject::invokeMethod(this, "generateOffscreenImage", Qt::QueuedConnection);
}

QImage OffScreenRenderer::drawOnImage(const QImage &image, QSharedPointer<StageItem> stageItem)
{
    EditableItemMetadata* metadata = (EditableItemMetadata*)stageItem->metadata().data();
    QJsonArray inkStrokes = metadata->inkStrokes();
    if(inkStrokes.empty())
        return image;

    QTransform tran;
    QPolygonF src(InkManger::normalizeRect(stageItem));
    src.remove(4);
    QPolygonF dst(QRectF(0, 0, image.width(), image.height()));
    dst.remove(4);
    if(!QTransform::quadToQuad(src, dst, tran))
    {
        return image;
    }

    QPixmap pixmap(image.width(), image.height());
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPointF imagePt = tran.map(QPointF(0, 0));
    painter.drawImage(imagePt, image);
    painter.setTransform(tran);
    inkStrokes = InkManger::applyPerspective(inkStrokes, stageItem);
    if(!inkStrokes.isEmpty())
    {
        QPolygonF perspectiveQuad = metadata->correctedPerspectiveQuad();
        double penScale = 1.0 / ScalableRect::perspectiveSize(perspectiveQuad).width();
        for(auto strokeJson : inkStrokes)
        {
            QJsonObject strokeObj = strokeJson.toObject();
            ObjectInkStroke inkStroke(strokeObj);
            inkStroke.draw(painter, penScale);
        }
    }

    painter.end();

    return pixmap.toImage();
}

QPixmap OffScreenRenderer::renderInkData(const QPixmap& image, const QPixmap& inkPixmap)
{
    if (inkPixmap.isNull())
        return image;

    QPixmap imageRet = image;
    QPainter painter;
    painter.begin(&imageRet);

    painter.drawPixmap(QRect(0, 0, imageRet.width(), imageRet.height()), inkPixmap);

    painter.end();

    return imageRet;
}

} // namespace common
} // namespace capture
