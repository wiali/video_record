#include "stage_project_exporter.h"

#include <QJsonDocument>
#include <QQuaternion>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QUuid>
#include <QImageWriter>
#include <QBuffer>

#include <quazipfile.h>
#include <system_error>

#include <global_utilities.h>

#include "event/export_projects_event.h"
#include "event/launch_worktool_event.h"
#include "event/change_mat_mode_event.h"
#include "common/utilities.h"
#include "model/camera_item_metadata.h"
#include "common/measured_block.h"

namespace capture {
namespace components {

// XML schema
namespace xmlElement {
static const QString scene = "scene";
namespace sceneAttribute {
static const QString scale = "scale";
static const QString inkData = "inkData";
}

static const QString uuidgen = "uuidgen";
namespace uuidgenAttribute {
static const QString invalidId = "invalidID";
static const QString lastId = "lastID";
}

static const QString image = "image";
namespace imageAttribute {
static const QString uid = "uid";
static const QString width = "width";
static const QString height = "height";
static const QString rect = "rect";
static const QString fileName = "fileName";
}

static const QString model = "model";
static const QString capture_model_uid = "-1";
namespace modelAttribute {
static const QString uid = "uid";
static const QString scale = "scale";
static const QString x = "x";
static const QString y = "y";
static const QString rotation = "rotation";
}

static const QString meta = "meta";
namespace metaAttribute {
static const QString metadata = "metadata";
}

static const QString sensorData = "sensorData";
namespace sensorDataAttribute {
static const QString uid = "uid";
static const QString captureDevice = "captureDevice";
static const QString type = "type";
}
}

namespace fileName {
static const QString objectData = "objectData.xml";
static const QString sensorDataJsonFormat = "%1_sensordata.json";
static const QString sensorDataImageFormat = "%1_sensordata";
}

// Conversion helpers
template<typename T>
QString toString(const T& input);

inline QString toString(const int& input)         { return QString::number(input); }
inline QString toString(const float& input)       { return QString::number(input); }
inline QString toString(const bool& input)        { return QString::number(input ? 1 : 0); }
inline QString toString(const QVector2D& input)   { return QString("%1 %2").arg(input.x()).arg(input.y()); }
inline QString toString(const QVector3D& input)   { return QString("%1 %2 %3").arg(input.x()).arg(input.y()).arg(input.z()); }
inline QString toString(const QQuaternion& input) { return QString("%1 %2 %3 %4").arg(input.x()).arg(input.y()).arg(input.z()).arg(input.scalar()); }
inline QString toString(const QRect& input)       { return QString("%1 %2 %3 %4").arg(input.x()).arg(input.y()).arg(input.width()).arg(input.height()); }
inline QString toString(const QSize& input)       { return QString("%1 %2").arg(input.width()).arg(input.height()); }
inline QString toString(const QPoint& input)      { return QString("%1 %2").arg(input.x()).arg(input.y()); }

struct StageProjectExporter::StageItemExportData
{
public:
    QSharedPointer<StageItem> stageItem;
    int id;
    QUuid uuid;
};

StageProjectExporter::StageProjectExporter(QSharedPointer<model::ProjectsExportModel> model, QObject *parent)
    : QObject(parent)
    , m_threadPool(new QThreadPool)
    , m_model(model) {
    setAutoDelete(false);
    QCoreApplication::instance()->installEventFilter(this);
}

bool StageProjectExporter::eventFilter(QObject *obj, QEvent *event) {
    bool processed = false;

    if (event->type() == event::ExportProjectsEvent::type()) {
        if (auto exportEvent = static_cast<event::ExportProjectsEvent*>(event))
        {
            {
                QMutexLocker locker(&m_mutex);
                m_exportQueue.enqueue(exportEvent->projects());
            }

            tryExportNextProject();
            processed = true;
        }
    }

    return processed ? processed : QObject::eventFilter(obj, event);
}

void StageProjectExporter::tryExportNextProject() {
    QMutexLocker locker(&m_mutex);

    if (!m_exportQueue.isEmpty()) {
        m_threadPool->start(this);
    }
}

void StageProjectExporter::run() {
    QVector<QSharedPointer<StageProject>> projects;

    {
        QMutexLocker locker(&m_mutex);

        if (!m_exportQueue.isEmpty()) {
            projects = m_exportQueue.dequeue();
        }
    }

    if (projects.count() > 0) {
        qInfo() << this << "Exporting" << projects.count() << "projects";

        m_model->setState(model::ProjectsExportModel::State::PrepairingToExport);
        m_model->setFormat(model::ProjectsExportModel::Format::Stage);

        m_model->setCount(projects.count());
        m_model->setIndex(0);

        m_model->setState(model::ProjectsExportModel::State::Exporting);

        exportToStage(projects);

        m_model->setState(model::ProjectsExportModel::State::FinalizingExport);

        // ToDO - cleanup temporary images

        common::Utilities::playSound("qrc:/Resources/production/Sounds/sendToStage.wav");

        m_model->setState(model::ProjectsExportModel::State::NotExporting);
    }
}


void StageProjectExporter::exportToStage(QVector<QSharedPointer<StageProject>> projects)
{    
    QStringList parameters("--import");
    QVector<QString> fileNames;

    try
    {
        if(common::Utilities::isStageWorkToolInstalled())
        {
            int index = 0;

            for(auto project : projects)
            {
                MEASURED_BLOCK

                m_model->setIndex(index++);
                QTemporaryFile temporaryFile;
                temporaryFile.setAutoRemove(false);

                QString uuid = QUuid::createUuid().toString().remove("{").remove("}");
                QString tempName = QString("%1/%2.%3").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).arg(uuid).arg("stage");
                temporaryFile.setFileName(tempName);

                fileNames.push_back(tempName);

                if (!temporaryFile.open()) {
                    throw std::exception(QString("Cannot create temporary file %1").arg(tempName).toLocal8Bit());
                }

                QuaZip stageZip(&temporaryFile);

                if (!stageZip.open(QuaZip::mdAdd)) throw std::exception("Cannot open ZIP file in Add mode");

                FlatExportList flatItemList;

                zipObjectsDataFile(stageZip, flatItemList, project);
                zipImageFiles(stageZip, flatItemList);
                zipSensorDataFiles(stageZip, flatItemList);

                parameters << QString("\"%1\"").arg(temporaryFile.fileName());
            }

            // Change to Desktop mode if Stage is installed so it will be visible
            auto matModeChangeEvent = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode::Desktop);
            matModeChangeEvent->dispatch();
        }

        auto launchWorktoolEvent = new event::LaunchWorktoolEvent(event::LaunchWorktoolEvent::Worktool::Stage, parameters);
        launchWorktoolEvent->dispatch();
    }
    catch(std::exception& ex)
    {
        cleanupTemporaryFiles(fileNames);
        emit exportFailed(ex.what());
    }
}

void StageProjectExporter::flattenItemList(FlatExportList &flatItemList, const QList<QSharedPointer<StageItem>>& stageItems)
{
    int id = 1;

    for(auto item : stageItems)
    {
        flatItemList << StageItemExportData { item, id++, QUuid::createUuid() };
    }
}

void StageProjectExporter::writeUUIDGenerator(QXmlStreamWriter &xmlWriter, int itemCount)
{
    qDebug() << this << "Writing UUID generator tag with" << itemCount << "items";

    xmlWriter.writeStartElement(xmlElement::uuidgen);

    xmlWriter.writeAttribute(xmlElement::uuidgenAttribute::lastId, toString(itemCount));
    xmlWriter.writeAttribute(xmlElement::uuidgenAttribute::invalidId, toString(0));

    xmlWriter.writeEndElement(); // xmlElement::uuidgen
}

void StageProjectExporter::zipObjectsDataFile(QuaZip &zipFile, FlatExportList& flatItemList, QSharedPointer<StageProject>& project)
{
    QuaZipFile file(&zipFile);

    if (!file.open(QIODevice::WriteOnly, QuaZipNewInfo(fileName::objectData), NULL, 0, Z_DEFLATED, 1)) {
        throw std::exception("Failed to add object data entry to ZIP file");
    }

    QXmlStreamWriter xmlWriter(&file);

    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement(xmlElement::scene);

    // ToDo: Where to get Scene scale from?
    QVector3D sceneScale(1,1,1);
    xmlWriter.writeAttribute(xmlElement::sceneAttribute::scale, toString(sceneScale.toVector2D()));

    flattenItemList(flatItemList, project->items());

    writeUUIDGenerator(xmlWriter, flatItemList.count());

    for (auto stageItem : project->items())
    {
        writeStageItem(xmlWriter, flatItemList, stageItem);
    }

    xmlWriter.writeEndElement(); // xmlElement::scene
    xmlWriter.writeEndDocument();
}

void StageProjectExporter::zipImageFiles(QuaZip &zipFile, FlatExportList &flatItemList)
{
    for(auto item : flatItemList)
    {
        zipImageFile(zipFile, item.uuid.toString(), item.stageItem->image());
    }
}

void StageProjectExporter::zipImageFile(QuaZip &zipFile, const QString & fileName, const QImage& image)
{
    QuaZipFile file(&zipFile);

    qDebug() << this << "Writing image file" << fileName << "with format" << imageFormat();

    QBuffer imageBuffer;

    // We want bit more control over the quality settings
    QImageWriter writer(&imageBuffer, imageFormat().toLocal8Bit());

    // Lossless compression
    writer.setQuality(GlobalUtilities::applicationSettings("stage_export")->value("quality", 100).toInt());
    writer.setCompression(GlobalUtilities::applicationSettings("stage_export")->value("compression", 100).toInt());

    // QuaZip doesn't support seek operation so we need to write to intermediate buffer first
    if (!writer.write(image.convertToFormat(QImage::Format_ARGB32))) {
        auto errorString = QString("Failed to convert image file; reason: %1").arg(writer.errorString());
        qCritical() << this << errorString;
        throw std::exception(errorString.toLocal8Bit());
    }

    if (!imageBuffer.seek(0)) {
        throw std::exception("Failed to seek to start of the stream");
    }

    auto zipCompression = GlobalUtilities::applicationSettings("stage_export")->value("zip_compression", 0).toInt();
    auto fullFileName = QString("%1.%2").arg(fileName).arg(imageFormat());

    if (!file.open(QIODevice::WriteOnly, fullFileName, NULL, 0, Z_DEFLATED, zipCompression)) {
        throw std::exception(QString("Failed to add image entry %1 to ZIP file").arg(fullFileName).toLocal8Bit());
    }

    const auto buffer = imageBuffer.buffer();
    if (file.write(buffer) != buffer.size())
    {
        throw std::exception("Failed to write complete image buffer");
    }
}

void StageProjectExporter::zipSensorDataFiles(QuaZip &zipFile, FlatExportList &flatItemList)
{
    for(auto item : flatItemList)
    {
        auto uid = item.id;
        auto metadata = item.stageItem->metadata().dynamicCast<model::CameraItemMetadata>();

        if (metadata == nullptr)
        {
            qWarning() << this << "Item with UUID" << uid << "doesn't contain camera metadata";
        }
        else
        {
            qDebug() << this << "Writing sensor data with UUID" << uid;

            for (auto sensorData : *metadata->sensorData())
            {
                auto prefix = QString("%1_%2_%3").arg(uid).arg(sensorData.captureDevice).arg(sensorData.type);

                {
                    QuaZipFile file(&zipFile);

                    if (!file.open(QIODevice::WriteOnly, QuaZipNewInfo(fileName::sensorDataJsonFormat.arg(prefix)), NULL, 0, Z_DEFLATED, 1)) {
                        throw std::exception("Failed to create sensor data JSON entry");
                    }

                    QJsonDocument doc(sensorData.toJson());
                    const auto buffer = doc.toJson();

                    if (file.write(buffer) != buffer.length()) {
                        throw std::exception("Failed to write complete sensor data buffer");
                    }
                }

                zipImageFile(zipFile, fileName::sensorDataImageFormat.arg(prefix), sensorData.image);
            }
        }
    }
}

QString StageProjectExporter::imageFormat()
{
    return GlobalUtilities::applicationSettings("stage_export")->value("image_format", "bmp").toString();
}

void StageProjectExporter::writeStageItem(QXmlStreamWriter &xmlWriter, FlatExportList &flatItemList, QSharedPointer<StageItem> stageItem)
{
    xmlWriter.writeStartElement(xmlElement::model);

    auto metadata = stageItem->metadata();
    auto id = 0;
    QUuid uuid;

    for(auto item : flatItemList)
    {
        if (item.stageItem == stageItem)
        {
            id =  item.id;
            uuid = item.uuid;
            break;
        }
    }

    xmlWriter.writeAttribute(xmlElement::modelAttribute::uid, xmlElement::capture_model_uid);

    qDebug() << this << "Writing Stage item " << id << uuid;

    QVector3D scale(1, 1, 1);
    xmlWriter.writeAttribute(xmlElement::modelAttribute::scale, toString(scale));

    xmlWriter.writeAttribute(xmlElement::modelAttribute::x, toString(id));
    xmlWriter.writeAttribute(xmlElement::modelAttribute::y, toString(id));
    xmlWriter.writeAttribute(xmlElement::modelAttribute::rotation, toString(QQuaternion::fromEulerAngles(metadata->rotation())));

    xmlWriter.writeEndElement(); // xmlElement::modelAttribute

    if (stageItem->children().count() == 0)
    {
        xmlWriter.writeStartElement(xmlElement::image);

        const auto imageSize = stageItem->imageRect().size();
        auto metaData = stageItem->metadata();
        auto editableMetaData = metaData.dynamicCast<EditableItemMetadata>();

        xmlWriter.writeAttribute(xmlElement::imageAttribute::uid, toString(id));
        xmlWriter.writeAttribute(xmlElement::imageAttribute::fileName, QString("%1.%2").arg(uuid.toString()).arg(imageFormat()));
        xmlWriter.writeAttribute(xmlElement::imageAttribute::width, toString(imageSize.width()));
        xmlWriter.writeAttribute(xmlElement::imageAttribute::height, toString(imageSize.height()));
        //      Stage need the 0,0,0,0 when no crop rect. Update here temporarily.
        //        xmlWriter.writeAttribute(xmlElement::imageAttribute::rect, toString(QRect(QPoint(), image.size())));
        xmlWriter.writeAttribute(xmlElement::imageAttribute::rect, toString(QRect(0,0,0,0)));

        if (editableMetaData == nullptr)
        {
            qWarning() << this << "This model doesn't contain editable metadata, skipping properties";
        }
        else
        {
            xmlWriter.writeAttribute(xmlElement::metaAttribute::metadata, 
                editableMetaData->toJsonString().toStdString().c_str());
        }

        xmlWriter.writeEndElement(); // xmlElement::meta

        auto cameraMetadata = metaData.dynamicCast<CaptureItemMetadata>();

        if (cameraMetadata == nullptr)
        {
            qWarning() << this << "This model doesn't contain camera metadata, skipping properties";
        }
        else
        {
            for (auto sensorData: *cameraMetadata->sensorData())
            {
                xmlWriter.writeStartElement(xmlElement::sensorData);

                xmlWriter.writeAttribute(xmlElement::sensorDataAttribute::uid, toString(id));
                xmlWriter.writeAttribute(xmlElement::sensorDataAttribute::captureDevice, sensorData.captureDevice);
                xmlWriter.writeAttribute(xmlElement::sensorDataAttribute::type, sensorData.type);

                xmlWriter.writeEndElement(); // xmlElement::sensorData
            }
        }

        xmlWriter.writeEndElement(); //xmlElement::image
    }
    else
    {
        for(auto child : stageItem->children())
        {
            writeStageItem(xmlWriter, flatItemList, child);
        }
    }
}

void StageProjectExporter::cleanupTemporaryFiles(const QVector<QString>& fileNames)
{    
     m_model->setState(model::ProjectsExportModel::State::FinalizingExport);

    for (auto filename : fileNames)
    {
        QFile tempfile(filename);
        if (tempfile.exists())
        {
            if (!tempfile.remove()) {
                qWarning() << this << "Failed to remove temporary file" << filename;
            }
        }
    }

    m_model->setState(model::ProjectsExportModel::State::NotExporting);
}

} // namespace components
} // namespace capture
