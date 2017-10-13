#pragma once
#ifndef EXPORT_IMAGE_EVENT_H
#define EXPORT_IMAGE_EVENT_H

#include <QEvent>
#include <QSharedPointer>

#include <stage_item.h>

#include "model/export_image_model.h"

namespace capture {
namespace event {

class ExportProjectItemsEvent : public QEvent
{
    Q_GADGET
public:
    enum Format
    {
        PNG,
        JPG,
        PDF,
        OCR,
        Clipboard
    };

    Q_ENUM(Format)

    explicit ExportProjectItemsEvent(ExportProjectItemsEvent::Format format, QString location,
                                     QVector<QSharedPointer<model::ExportImageModel>> items, bool single = false);

    inline ExportProjectItemsEvent::Format format() const { return m_format; }
    inline QString location() const { return m_location; }
    inline QVector<QSharedPointer<model::ExportImageModel>> items() const { return m_items; }
    inline bool single() const { return m_single; }

    static QEvent::Type type();
    void dispatch();

private:
    ExportProjectItemsEvent::Format m_format;
    QString m_location;
    QVector<QSharedPointer<model::ExportImageModel>> m_items;
    bool m_single;
};

} // namespace event
} // namespace capture

Q_DECLARE_METATYPE(capture::event::ExportProjectItemsEvent::Format)

#endif // EXPORT_IMAGE_EVENT_H
