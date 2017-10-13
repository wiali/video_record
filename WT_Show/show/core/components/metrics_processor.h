#pragma once
#ifndef METRICS_PROCESSOR_H
#define METRICS_PROCESSOR_H

#include <QObject>
#include <single_instance.h>

#include "model/application_state_model.h"

namespace capture {
namespace components {

namespace MetricsDataType {

    // EventID, EventTitle , EventPart1 , EventPart2, EventPart3
    enum event_type
    {
        Ca_Launch = 0,
        Ca_WTOpenCa,
        Ca_Capture,
        Ca_Export
    };

    // unneccessary class but let caller clearly know what event part they belong
    struct EventPart1
    {
        enum app_guid_type
        {
            Launcher = 0,
            Stage,
            System
        };

        enum video_source_type
        {
            Downward = 0,
            Forward,
            SproutCamera,
            PrimaryDesktop,
            MatDesktop,
            Webcamera
        };

        enum export_type
        {
            All = 0,
            Single
        };
    };

    // unneccessary class but let caller clearly know what event part they belong
    struct EventPart2
    {
        enum downward_mode
        {
            None = 0,
            Lampoon,
            Lampooff,
            Desktop
        };
        enum file_format
        {
            PDF = 0,
            PNGJPG,
            Zip
        };
    };

    // unneccessary class but let caller clearly know what event part they belong
    struct EventPart3
    {
        enum flash_mode
        {
            Off = 0,
            On
        };
    };

    // unneccessary class but let caller clearly know what event part they belong
    struct EventPart4
    {
        enum auto_fix
        {
            Off = 0,
            On
        };
    };
}

class MetricsProcessor : public QObject
{
    Q_OBJECT
public:
    explicit MetricsProcessor(QSharedPointer<model::ApplicationStateModel> model,
                              QSharedPointer<SingleInstance> singleInstance,
                              QObject *parent = 0);
    virtual ~MetricsProcessor();

private slots:
    void onMetricsExport(model::ProjectsExportModel::State state);
    void onMetricsCapture(model::LiveCaptureModel::CaptureState state);
    void onSingleInstanceReceivedArguments(const QStringList &arguments);

private:
    QString getVersion();
    QString reverseProductId(const QString& pId);
    QString reverseStr(const QString& str);
    void writeMetrics(MetricsDataType::event_type type, const QVariantList& eventParts);
    QString toString(const QVariantList& varList, int index);

private:
    QSharedPointer<metrics::MetricsMgr> m_metricsMgr;
    QSharedPointer<metrics::Metrics> m_metrics;
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace components
} // namespace capture

Q_DECLARE_METATYPE(capture::components::MetricsDataType::event_type)
Q_DECLARE_METATYPE(capture::components::MetricsDataType::EventPart1::app_guid_type)
Q_DECLARE_METATYPE(capture::components::MetricsDataType::EventPart1::video_source_type)
Q_DECLARE_METATYPE(capture::components::MetricsDataType::EventPart1::export_type)
Q_DECLARE_METATYPE(capture::components::MetricsDataType::EventPart2::downward_mode)
Q_DECLARE_METATYPE(capture::components::MetricsDataType::EventPart2::file_format)
Q_DECLARE_METATYPE(capture::components::MetricsDataType::EventPart3::flash_mode)
Q_DECLARE_METATYPE(capture::components::MetricsDataType::EventPart4::auto_fix)


#endif // METRICS_PROCESSOR_H
