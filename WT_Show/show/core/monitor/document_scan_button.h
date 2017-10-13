#pragma once
#ifndef DOCUMENT_SCAN_BUTTON_H
#define DOCUMENT_SCAN_BUTTON_H

#include <QWidget>
#include <QPointer>

#include <right_menu_button.h>

#include "model/application_state_model.h"
#include "model/segmentation_collection_model.h"
#include "../shared/capture_item_metadata.h"

namespace capture {
namespace monitor {

class DocumentScanButton : public RightMenuButton
{
    Q_OBJECT

public:
    explicit DocumentScanButton(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onSelectedProjectChanged();
    void onModelSegmentationStatusChanged(CaptureItemMetadata::SegmentationState newState);
    void onDocModeButtonClicked();
    void onDocumentScanModeChanged();

private:
    QSharedPointer<model::ApplicationStateModel> m_model;
    QMetaObject::Connection m_modelStatusConnection;
    QMetaObject::Connection m_documentModeConnection;

};

} // namespace monitor
} // namespace capture

#endif // DOCUMENT_SCAN_BUTTON_H
