#include "export_image_processor.h"

#include <QCoreApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QtConcurrent>

#include <global_utilities.h>

#include "common/utilities.h"
#include "common/measured_block.h"
#include <global_utilities.h>

#include "event/export_project_items_event.h"

namespace capture {
namespace components {

static QHash<event::ExportProjectItemsEvent::Format, model::ProjectsExportModel::Format> FormatTranslationTable{
    {event::ExportProjectItemsEvent::Format::JPG, model::ProjectsExportModel::Format::JPG},
    {event::ExportProjectItemsEvent::Format::PNG, model::ProjectsExportModel::Format::PNG},
    {event::ExportProjectItemsEvent::Format::PDF, model::ProjectsExportModel::Format::PDF},
    {event::ExportProjectItemsEvent::Format::OCR, model::ProjectsExportModel::Format::OCR},
    {event::ExportProjectItemsEvent::Format::Clipboard, model::ProjectsExportModel::Format::Clipboard}};

struct ExportImageProcessor::ExportModel {
 public:
  event::ExportProjectItemsEvent::Format format;
  QString location;
  QVector<QSharedPointer<model::ExportImageModel>> items;
  bool single;
};

ExportImageProcessor::ExportImageProcessor(QSharedPointer<model::ProjectsExportModel> model,
                                           QObject *parent)
    : QObject(parent), m_threadPool(new QThreadPool), m_model(model), m_offscreenRenderer(new common::OffScreenRenderer(true)) {
  setAutoDelete(false);
  connect(m_offscreenRenderer.data(), &common::OffScreenRenderer::imageReady, this, &ExportImageProcessor::onImageReady);
  QCoreApplication::instance()->installEventFilter(this);
}

bool ExportImageProcessor::eventFilter(QObject *obj, QEvent *event) {
  bool processed = false;

  if (event->type() == event::ExportProjectItemsEvent::type()) {
    auto exportImageEvent = static_cast<event::ExportProjectItemsEvent *>(event);

    if (exportImageEvent != nullptr) {
      {
        QMutexLocker locker(&m_mutex);
        m_exportImageQueue.enqueue({exportImageEvent->format(), exportImageEvent->location(),
                                    exportImageEvent->items(), exportImageEvent->single()});
      }

      tryProcessNextModel();
      processed = true;
    }
  }

  return processed ? processed : QObject::eventFilter(obj, event);
}

void ExportImageProcessor::tryProcessNextModel() {
  QMutexLocker locker(&m_mutex);

  if (!m_exportImageQueue.isEmpty()) {
    m_threadPool->start(this);
  }
}

QImage ExportImageProcessor::waitForOffscreenImage(QSharedPointer<StageItem> item) {
    QMutexLocker locker(&m_offscreenRendererMutex);

    m_offscreenRenderer->requestOffscreenImage(item);
    m_offscreenRendererWaitCondition.wait(&m_offscreenRendererMutex);

    return m_lastOffscreenImage;
}

void ExportImageProcessor::onImageReady(QSharedPointer<StageItem> item, QImage image) {
    m_lastOffscreenImage = image;
    m_offscreenRendererWaitCondition.wakeAll();
}

void ExportImageProcessor::run() {
  ExportModel model;
  bool hasExportItem = false;

  {
    QMutexLocker locker(&m_mutex);

    if (!m_exportImageQueue.isEmpty()) {
      model = m_exportImageQueue.dequeue();
      hasExportItem = true;
    }
  }

  if (hasExportItem) {
    MEASURED_BLOCK

    qInfo() << this << "Exporting" << model.items.count() << "items to" << model.format;

    m_model->setState(model::ProjectsExportModel::State::PrepairingToExport);
    m_model->setFormat(FormatTranslationTable[model.format]);

    m_model->setCount(model.items.count());
    m_model->setIndex(0);

    m_model->setState(model::ProjectsExportModel::State::Exporting);

    try {
      switch (model.format) {
        case event::ExportProjectItemsEvent::Format::PNG:
        case event::ExportProjectItemsEvent::Format::JPG:
          exportImages(model);
          break;
        case event::ExportProjectItemsEvent::Format::OCR:
          performOpticCharacterRecognitionAndExportPDF(model);
          break;
        case event::ExportProjectItemsEvent::Format::PDF:
          exportPDF(model);
          break;
        case event::ExportProjectItemsEvent::Format::Clipboard: {
          Q_ASSERT(model.items.count() == 1);

          auto exportItem = model.items.first();
          const auto image = waitForOffscreenImage(exportItem->item());

          // Due to bug https://bugreports.qt.io/browse/QTBUG-11463,
          // we can't directly call QClipboard::setImage or transparency will be lost.
          // Use same approach from Stage WT by saving the image in the temp folder and setting the URL in the clipboard.

          QTemporaryFile tempFile(QDir::tempPath() + "/CaptureWT_ClipboardXXXXXX.png");

          // The name is constructed only when it is successfully opened.
          if (tempFile.open())
          {
              tempFile.close();

              // We don't want the file to be removed.
              tempFile.setAutoRemove(false);

              qDebug() << "Saving file to" << tempFile.fileName();
              if (image.save(tempFile.fileName())) {
                  QList<QUrl> urls;
                  urls << QUrl::fromLocalFile(tempFile.fileName());
                  emit clipboardUrlsReady(urls);
              }
              else {
                  throw std::exception("Failed to save copied image.");
              }
          }
          else
          {
              throw std::exception("Failed to create temporary file to hold copied image.");
          }

          break;
      }
      default:
          Q_UNREACHABLE();
      }
    } catch (std::exception &ex) {
      emit exportFailed(ex.what());
    }

    m_model->setState(model::ProjectsExportModel::State::FinalizingExport);

    m_lastOffscreenImage = QImage();

    m_model->setState(model::ProjectsExportModel::State::NotExporting);

    tryProcessNextModel();
  }
}

void ExportImageProcessor::exportImages(ExportModel model) {
  int index = 0;
  common::OffScreenRenderer renderer(true);

  for (auto exportItem : model.items) {
    m_model->setIndex(index++);

    QString fileName = exportItem->name();

    auto metaEnum = QMetaEnum::fromType<event::ExportProjectItemsEvent::Format>();

    QString postfix = metaEnum.valueToKey(model.format);
    QString pathName = QString("%1/%2.%3").arg(model.location).arg(fileName).arg(postfix.toLower());

    if (!model.single) {
      pathName = handleConflictName(pathName);
    }

    auto settings = GlobalUtilities::applicationSettings("image_export");
    auto quality = -1;

    switch(model.format) {
    case event::ExportProjectItemsEvent::Format::Clipboard:
    case event::ExportProjectItemsEvent::Format::PDF:
        // Not covered here
        break;
    case event::ExportProjectItemsEvent::Format::PNG:
        // SPROUTSW-3065 - 50 is a good compromise between speed of compression (3-4x faster than default value)
        // while adding only ~5% to the final file size
        quality = settings->value(QString("png_quality"), 50).toInt();
        break;
    case event::ExportProjectItemsEvent::Format::JPG:
        quality = settings->value(QString("jpg_quality"), -1).toInt();
        break;
    }

    qInfo() << this << "Exporting" << fileName << "to" << pathName << "with quality" << quality;

    if (!waitForOffscreenImage(exportItem->item()).save(pathName, Q_NULLPTR, quality)) {
      QFile file(pathName);
      if (file.exists()) {
        file.remove();
      }
      throw std::exception("Failed to export image");
    }
  }
}

void ExportImageProcessor::performOpticCharacterRecognitionAndExportPDF(ExportModel model) {
    qDebug() << this << "Peforming OCR and exporting to PDF";
    QVector<QImage> images;

    int index = 0;

    for (auto exportItem : model.items) {
        images << waitForOffscreenImage(exportItem->item());
        m_model->setIndex(index++);
    }

    if (m_ocr.isNull()) {
        m_ocr.reset(new ocr::Ocr);
    }

    auto settings = GlobalUtilities::applicationSettings("ocr");

    const static QStringList defaultRecognizedLanguages { "en-us", "zh-cn", "zh-tw" };
    const auto recognizedLanguages = settings->value("recognized_languages", defaultRecognizedLanguages).toStringList();
    const auto localeLanguageName = GlobalUtilities::translatorLocale().name().toLower();

    ocr::OcrOptions options;
    options.language = recognizedLanguages.contains(localeLanguageName) ? localeLanguageName : recognizedLanguages.first();
    options.workDepth = settings->value("work_depth", 128).toInt();

    qInfo() << this << "Running OCR with language" << options.language << "and work depth" << options.workDepth;

    m_model->setState(model::ProjectsExportModel::State::PerformingOcr);

    m_ocr->extractDocument(images, model.location, options);
}

void ExportImageProcessor::exportPDF(ExportModel model) {
  qDebug() << this << "Exporting to PDF";

  auto settings = GlobalUtilities::applicationSettings("pdf_export");

  auto pdfLayoutLandscape = settings->value("layout_landscape", true).toBool();
  auto pdfDpi = settings->value("dpi", 600).toInt();

  QPrinter printer(QPrinter::HighResolution);

  // PDF files are always single files so we don't need to handle conflict names
  qInfo() << this << "Exporting to" << model.location;

  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(model.location);
  printer.setResolution(pdfDpi);

  // must be called before QPainter
  if (pdfLayoutLandscape) {
    printer.setOrientation(QPrinter::Landscape);
  } else {
    printer.setOrientation(QPrinter::Portrait);
  }
  QPainter painter;

  try {
    if (!painter.begin(&printer)) {
      throw std::exception("Failed to begin painting on PDF export");
    }

    int exportCount = 0;
    int exportTotal = model.items.size();

    if (exportTotal == 0) {
      qWarning() << this << "No items to export to PDF.";
      return;
    }
    qInfo() << this << "PDF Resolution: " << printer.resolution();

    // we will navigate and add Image by image to the pdf
    for (auto exportItem : model.items) {
      // default size of page is defined
      // need to scale image to fit in the specified layout
      auto sourceImage = waitForOffscreenImage(exportItem->item());
      auto scaledImage = sourceImage.scaled(printer.pageRect().width(), printer.pageRect().height(),
                                            Qt::KeepAspectRatio);

      painter.drawImage((printer.pageRect().width() - scaledImage.width()) / 2, 0, scaledImage);

      exportCount++;
      m_model->setIndex(exportCount);

      if (exportCount < exportTotal) {
        // add new page to printer
        if (!printer.newPage()) {
          throw std::exception("Failed to add new page to PDF file");
        }
      }
    }

    if (!painter.end()) {
      throw std::exception("Failed to end painting on PDF export");
    }
  } catch (std::exception &ex) {
    Q_UNUSED(ex);
    QFile file(printer.outputFileName());
    if (file.exists()) {
      file.remove();
    }
    throw;
  }
}

QString ExportImageProcessor::handleConflictName(const QString &fileName) {
  auto result = fileName;
  bool exists = false;

  do {
    QFileInfo info(result);
    exists = info.exists();

    result = exists
                 ? QString("%1/%2.%3")
                       .arg(info.path())
                       .arg(common::Utilities::createNonConflictingName(info.completeBaseName()))
                       .arg(info.suffix())
                 : result;
  } while (exists);

  return result;
}

} // namespace components
} // namespace capture
