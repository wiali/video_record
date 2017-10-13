#pragma once
#ifndef POSTCAPTUREMODEL_H
#define POSTCAPTUREMODEL_H

#include <QObject>
#include <QRectF>

#include <ink_data.h>

namespace capture {
namespace model {

class PostCaptureModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRectF viewport READ viewport WRITE setViewport NOTIFY viewportChanged)

public:
    typedef QMap<QSharedPointer<InkData>, QPixmap> INK_PIX_MAP;

    explicit PostCaptureModel(QObject *parent = 0);

    inline QRectF viewport() const { return m_viewport; }

signals:
    void viewportChanged(QRectF viewport);

public slots:
    void setViewport(QRectF viewport);

private:
    QRectF m_viewport;
};

} // namespace model
} // namespace capture

#endif // POSTCAPTUREMODEL_H
