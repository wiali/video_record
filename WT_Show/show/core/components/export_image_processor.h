#pragma once
#ifndef EXPORT_IMAGE_PROCESSOR_H
#define EXPORT_IMAGE_PROCESSOR_H

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QPolygonF>
#include <QPrinter>
#include <QQueue>
#include <QRunnable>
#include <QSharedPointer>
#include <QThreadPool>
#include <QWaitCondition>
#include <QOpenGLFunctions>

#include <ocr.h>

#include "common/offscreen_renderer.h"
#include "model/export_image_model.h"
#include "model/projects_export_model.h"

namespace capture {
namespace components {

/*!
 * \brief The ExportImageProcessor class is responsible for asynchronous export of the captured
 * images into variety of export formats.
 * \details This class monitors occurences of ExportProjectItemsEvent to start exporting. Only
 * single export operation is being executed at the same time.
 */
class ExportImageProcessor : public QObject, public QRunnable {
  Q_OBJECT

 public:
  /*!
   * \brief ExportImageProcessor constructor.
   * \param model Model object.
   * \param parent Parent object.
   */
  explicit ExportImageProcessor(QSharedPointer<model::ProjectsExportModel> model, QObject *parent = 0);
  virtual void run();

 signals:

  void clipboardUrlsReady(QList<QUrl> urls);
  void exportFailed(const QString& reason);

 protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);

 private:
  void tryProcessNextModel();

  struct ExportModel;

  void exportImages(ExportModel model);
  void exportPDF(ExportModel model);
  void performOpticCharacterRecognitionAndExportPDF(ExportModel model);

  QString handleConflictName(const QString &fileName);

  QImage waitForOffscreenImage(QSharedPointer<StageItem> item);

  QQueue<ExportModel> m_exportImageQueue;
  QMutex m_mutex;
  QMutex m_offscreenRendererMutex;
  QWaitCondition m_offscreenRendererWaitCondition;

  QScopedPointer<QThreadPool> m_threadPool;
  QScopedPointer<ocr::Ocr> m_ocr;
  QSharedPointer<model::ProjectsExportModel> m_model;
  QScopedPointer<common::OffScreenRenderer> m_offscreenRenderer;
  QImage m_lastOffscreenImage;

private slots:

  void onImageReady(QSharedPointer<StageItem> item, QImage image);

};

} // namespace components
} // namespace capture

#endif  // EXPORT_IMAGE_PROCESSOR_H
