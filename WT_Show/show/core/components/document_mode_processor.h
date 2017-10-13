#pragma once
#ifndef DOCUMENTMODEPROCESSOR_H
#define DOCUMENTMODEPROCESSOR_H

#include <QPolygonF>
#include <QObject>
#include <QHash>
#include <QQueue>
#include <QRunnable>
#include <QMutex>
#include <QSharedPointer>
#include <QThreadPool>

#include "model/camera_item_metadata.h"

namespace capture {
namespace components {

/*!
 * \brief The DocumentModeProcessor class is responsible for asynchronous segmenting of the captured images and providing information for DocScan mode.
 * \details This class monitors occurences of SegmentObjectEvent to start segmenting captured images. Only single segmentation is being executed at the same time.
 */
class DocumentModeProcessor : public QObject, public QRunnable {
    Q_OBJECT    

public:
    /*!
     * \brief DocumentModeProcessor constructor.
     * \param parent Parent object.
     */
    explicit DocumentModeProcessor(QObject *parent = 0);
    virtual void run() override;

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    void tryProcessNextModel();
    QQueue<QSharedPointer<model::CameraItemMetadata>> m_segmentationQueue;
    QMutex m_mutex;
    QScopedPointer<QThreadPool> m_threadPool;
};

} // namespace components
} // namespace capture

#endif // DOCUMENTMODEPROCESSOR_H
