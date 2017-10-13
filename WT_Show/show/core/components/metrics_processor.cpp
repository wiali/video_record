#include "metrics_processor.h"

#include <QCoreApplication>
#include <QDebug>
#include <QApplication>

#include <metrics.h>
#include <metrics_mgr.h>

#include <global_utilities.h>
#include "common/utilities.h"

namespace capture {
namespace components {

//---------------------------------------------------------------------------------------------------------------------
const QString UPGRADECODE_CAPTURE
    = "EF104F893FE6026448587C3D189C7523";
const QString REGISTRY_KEY_UNINSTALL
    = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
const QString REGISTRY_KEY_UPGRADECODES
    = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UpgradeCodes\\";

const QString EVENT_ID_TITLES[][2] =
{
    {"8d3c998f-28da-40ad-8910-33bf040a921e", "Ca_Launch"},
    {"4d1b8728-ce3e-4316-9575-fd801302f86f", "Ca_WTOpenCa"},
    {"18a7b24a-426a-422d-a08b-617e6cb679d0", "Ca_Capture"},
    {"f100f730-865b-4f1e-8224-d3bf4d565c4c", "Ca_Export"},
};

static QMap<common::VideoSourceInfo::SourceType, MetricsDataType::EventPart1::video_source_type>
    Metrics_SPT_VST_TranslationTable {
        { common::VideoSourceInfo::SourceType::DownwardFacingCamera, MetricsDataType::EventPart1::Downward },
        { common::VideoSourceInfo::SourceType::SproutCamera, MetricsDataType::EventPart1::SproutCamera},
        { common::VideoSourceInfo::SourceType::ForwardFacingCamera, MetricsDataType::EventPart1::Forward },
        { common::VideoSourceInfo::SourceType::PrimaryDesktop, MetricsDataType::EventPart1::PrimaryDesktop },
        { common::VideoSourceInfo::SourceType::MatDesktop, MetricsDataType::EventPart1::MatDesktop },
        { common::VideoSourceInfo::SourceType::Webcamera, MetricsDataType::EventPart1::Webcamera }
};

static QHash<model::LiveCaptureModel::PreCaptureMode, MetricsDataType::EventPart2::downward_mode>
    Metrics_PCM_DM_TranslationTable {
        { model::LiveCaptureModel::LampOn,  MetricsDataType::EventPart2::Lampoon},
        { model::LiveCaptureModel::LampOff, MetricsDataType::EventPart2::Lampooff },
        { model::LiveCaptureModel::Desktop, MetricsDataType::EventPart2::Desktop }
};

static QHash<model::ProjectsExportModel::Format, MetricsDataType::EventPart2::file_format>
    Metrics_EPE_FF_TranslationTable {
        { model::ProjectsExportModel::PDF,  MetricsDataType::EventPart2::PDF},
        { model::ProjectsExportModel::OCR,  MetricsDataType::EventPart2::PDF},
        { model::ProjectsExportModel::PNG, MetricsDataType::EventPart2::PNGJPG },
        { model::ProjectsExportModel::JPG, MetricsDataType::EventPart2::PNGJPG },
        { model::ProjectsExportModel::Stage, MetricsDataType::EventPart2::Zip }
};

//---------------------------------------------------------------------------------------------------------------------
MetricsProcessor::MetricsProcessor(QSharedPointer<model::ApplicationStateModel> model, QSharedPointer<SingleInstance> singleInstance,
                                   QObject *parent)
    : QObject(parent)
    , m_model(model)
{
    QString strPath;
    m_metricsMgr = QSharedPointer<metrics::MetricsMgr>::create(strPath);
    QString captureVersion = getVersion();
    if(captureVersion.isEmpty())
    {
//        captureVersion = PACKAGE_VERSION;
    }

    m_metrics = m_metricsMgr->createMetrics(
                GlobalUtilities::applicationSettings()->value("metrics_name", "WTCapture").toString(), captureVersion, "1.0");

    m_metrics->start();

    auto const launchedFrom = common::Utilities::parseCommandLine(QApplication::arguments()).second.launchedFrom;

    QVariantList launchList;
    launchList << (launchedFrom.isEmpty() ? QString("66dc3bf6-8125-4180-bf27-abbff0086734") : launchedFrom);

    writeMetrics(MetricsDataType::Ca_Launch, launchList);

    if (!launchedFrom.isEmpty())
    {
        onSingleInstanceReceivedArguments(QApplication::arguments());
    }

    connect(m_model->projectsExport().data(), &model::ProjectsExportModel::stateChanged, this, &MetricsProcessor::onMetricsExport);
    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::captureStateChanged, this, &MetricsProcessor::onMetricsCapture);
    connect(singleInstance.data(), &SingleInstance::receivedArguments, this, &MetricsProcessor::onSingleInstanceReceivedArguments);
}

//---------------------------------------------------------------------------------------------------------------------
MetricsProcessor::~MetricsProcessor()
{
    m_metrics->end();
}

//---------------------------------------------------------------------------------------------------------------------
QString MetricsProcessor::getVersion()
{
    QString productID;
    QSettings reg_ugc(REGISTRY_KEY_UPGRADECODES, QSettings::NativeFormat);
    reg_ugc.beginGroup(UPGRADECODE_CAPTURE);
    if( reg_ugc.childKeys().isEmpty() )
        return "";

    productID = reg_ugc.childKeys().at(0);
    reg_ugc.endGroup();

    QString version;
    QSettings reg_uninstall(REGISTRY_KEY_UNINSTALL, QSettings::NativeFormat);
    QString reversedId = reverseProductId(productID);
    if(reg_uninstall.childGroups().contains(reversedId))
    {
        reg_uninstall.beginGroup(reversedId);
        version = reg_uninstall.value("DisplayVersion").toString();
        reg_uninstall.endGroup();
    }
    return version;
}

//---------------------------------------------------------------------------------------------------------------------
QString MetricsProcessor::reverseProductId(const QString& pId)
{
    QString result = "{";
    result += reverseStr(pId.mid(0, 8)) + "-" +
            reverseStr(pId.mid(8, 4)) + "-" +
            reverseStr(pId.mid(12, 4)) + "-" +
            reverseStr(pId.mid(16, 2)) +
            reverseStr(pId.mid(18, 2)) + "-" +
            reverseStr(pId.mid(20, 2)) +
            reverseStr(pId.mid(22, 2)) +
            reverseStr(pId.mid(24, 2)) +
            reverseStr(pId.mid(26, 2)) +
            reverseStr(pId.mid(28, 2)) +
            reverseStr(pId.mid(30, 2)) + "}";

    qDebug() << "reverseProductId:" << result;
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
QString MetricsProcessor::reverseStr(const QString& str)
{
    int length = str.size();
    QString result(length);
    for(int i = 0; i < length; i++)
        result[i] = str[length - i - 1];

    return result;
}

QString MetricsProcessor::toString(const QVariantList& varList, int index)
{
    if( index < varList.size() )
    {
        QVariant var = varList.at(index);
        return var.canConvert<QString>() ? var.toString() : QString::number(var.toInt());
    }
    return "unknown";
}

//---------------------------------------------------------------------------------------------------------------------
void MetricsProcessor::writeMetrics(MetricsDataType::event_type type, const QVariantList& eventParts)
{
    QJsonObject eventData
    {
        {"DT", QDateTime::currentDateTime().toString("yyyyMMddThhmmss")},
        {"1", EVENT_ID_TITLES[type][0]},
        {"2", EVENT_ID_TITLES[type][1]},
        {"3", toString(eventParts, 0)},
        {"4", toString(eventParts, 1)},
        {"5", toString(eventParts, 2)},
        {"6", toString(eventParts, 3)}
    };

    qDebug() << this << "Writing metrics" << type << eventParts;

    m_metrics->trackEvent( eventData );
}

//---------------------------------------------------------------------------------------------------------------------
void MetricsProcessor::onMetricsExport(model::ProjectsExportModel::State state)
{
    if (state != model::ProjectsExportModel::State::FinalizingExport)
        return;

    auto exportModel = m_model->projectsExport();

    QVariantList eventParts;
    eventParts << ( exportModel->count() > 1 ? MetricsDataType::EventPart1::All : MetricsDataType::EventPart1::Single);
    eventParts << Metrics_EPE_FF_TranslationTable.value(exportModel->format());
    eventParts << exportModel->count();

    writeMetrics(MetricsDataType::Ca_Export, eventParts);
}

//---------------------------------------------------------------------------------------------------------------------
void MetricsProcessor::onMetricsCapture(model::LiveCaptureModel::CaptureState state)
{
    if (state != model::LiveCaptureModel::CaptureState::FinalizingCapture)
        return;

    auto captureModel = m_model->liveCapture();

    QVariantList eventParts;
    QStringList selectedStreams;

    for (auto videoStreamSource : captureModel->selectedVideoStreamSources()) {
        const auto metricsType = Metrics_SPT_VST_TranslationTable.value(videoStreamSource.type);
        selectedStreams << QString::number(static_cast<int>(metricsType));
    }

    eventParts << selectedStreams.join(",");

    if (captureModel->selectedVideoStreamSources().contains(common::VideoSourceInfo::DownwardFacingCamera()) ||
        captureModel->selectedVideoStreamSources().contains(common::VideoSourceInfo::SproutCamera()))
    {
        eventParts << Metrics_PCM_DM_TranslationTable.value(captureModel->preCaptureMode());
        eventParts << (captureModel->flashMode() ?
                           MetricsDataType::EventPart3::On : MetricsDataType::EventPart3::Off );
        eventParts << (captureModel->autoFix() ?
                           MetricsDataType::EventPart4::On : MetricsDataType::EventPart4::Off );
    }

    writeMetrics(MetricsDataType::Ca_Capture, eventParts);
}

//---------------------------------------------------------------------------------------------------------------------

void MetricsProcessor::onSingleInstanceReceivedArguments(const QStringList &arguments) {
    auto const launchedFrom = common::Utilities::parseCommandLine(arguments).second.launchedFrom;
    QVariantList openList;

    if (!launchedFrom.isEmpty()) {
        openList << launchedFrom;
    }

    writeMetrics(MetricsDataType::Ca_WTOpenCa, openList);
}

} // namespace components
} // namespace capture
