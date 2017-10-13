#include "show_worktool.h"

#include <QStringList>
#include <QMessageBox>
#include <QTranslator>
#include <QScopedPointer>

#ifdef Q_OS_WIN
#include <crash_handler.h>
#endif

#include <single_instance.h>
#include <logger.h>
#include <hardware_info.h>
#include <hal/system.h>
#include <global_utilities.h>

#include "components/metrics_processor.h"
#include "components/system_event_monitor.h"
#include "components/projector_manager.h"
#include "components/touchmat_manager.h"
#include "components/camera_proxy_style.h"
#include "components/live_video_stream_compositor.h"
#include "components/color_correction_calibrator.h"
#include "common/utilities.h"
#include "monitor/monitor_window.h"
#include "mat/mat_window.h"
#include "presentation/presentation_mode_window.h"
#include <global_utilities.h>

namespace capture {

CaptureWorktool::CaptureWorktool()
{
#ifdef Q_OS_WIN
    //CrashHandler::HookUnhandledExceptions(PACKAGE_NAME);
#endif
}

QSharedPointer<model::ApplicationStateModel> CaptureWorktool::createModel() {
    return QSharedPointer<model::ApplicationStateModel>::create();
}

bool CaptureWorktool::singleScreenMode() {
    bool result = false;
    const auto isSprout = sprout::HardwareInfo::IsSprout();
    auto applicationMode = GlobalUtilities::applicationSettings()->value("application_mode", "autodetect").toString();

    qInfo() << this << "Hardware info reports Sprout?" << isSprout <<
                        sprout::HardwareInfo::SproutCategory() << sprout::HardwareInfo::SproutVersion();

    if (applicationMode == "autodetect") {
         result = !isSprout;
    } else if (applicationMode == "singlescreen") {
        result = true;
    } else if (applicationMode == "sprout") {
        result = false;
    }

    return result;
}

int CaptureWorktool::exec(int argc, char **argv) {
    int exitCode = 0;

    GlobalUtilities::enableDpiAwareness();

    QApplication a(argc, argv);

//    a.setApplicationVersion(PACKAGE_VERSION);
    a.setOrganizationName("HP");
    a.setApplicationName("WorkTools/Capture");

    m_model = createModel();

    if (parseCommandLine(argc, argv, &exitCode)) {
        auto singleInstanceId = QString("HP_Capture-%1").arg(common::Utilities::currentUserName());

        try
        {
            hp::fortis::Logger::install("HP/WorkTools/Capture/log", "Capture");
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            qWarning() << "Cannot install custom logging handler, reason" << ex.what();
        }

        qInfo() << this << "Checking for single instance server name" << singleInstanceId;

        auto singleInstance = QSharedPointer<SingleInstance>::create(singleInstanceId);

        // There is a instance that have been launched. So we send the arguments to it.
        if (singleInstance->isRunning())
        {
            QStringList args;

            for(int i = 0; i < argc; i++)
            {
                args << argv[i];
            }

            qInfo() << this << "Detected other instance, forwarding parameters" << args;

            singleInstance->sendArguments(args);
        }
        else
        {
            singleInstance->buildServer();
            m_model->setSingleScreenMode(singleScreenMode());

            if (!m_model->singleScreenMode())
            {
                proapi::hal::System system;
                const auto hardwareIds = system.hardwareIDs();
                GlobalUtilities::setHardwareIds(hardwareIds.sprout_touchscreen, hardwareIds.sprout_projector);
            }

            a.processEvents();

            auto metricProcessor = QSharedPointer<components::MetricsProcessor>::create(m_model, singleInstance);
            Q_UNUSED(metricProcessor);
            auto systemEventMonitor = QSharedPointer<components::SystemEventMonitor>::create(m_model, singleInstance);
            Q_UNUSED(systemEventMonitor);
            auto compositor = QSharedPointer<components::LiveVideoStreamCompositor>::create(m_model->liveCapture());
            QScopedPointer<components::ColorCorrectionCalibrator> colorCorrectionCalibrator(new components::ColorCorrectionCalibrator(m_model, compositor));

            QScopedPointer<components::ProjectorManager> projectorManager;
            QScopedPointer<components::TouchmatManager> touchmatManager;

            if (!m_model->singleScreenMode()) {
                projectorManager.reset(new components::ProjectorManager(m_model));
                touchmatManager.reset(new components::TouchmatManager(m_model));
            }

            //setStyle is used to apply specific overwrite for some widgets style/behavior
            a.setStyle(new components::CameraProxyStyle);

            QTranslator translator;

            qInfo() << "Location : " << QLocale::system().name();
            qInfo() << "Application original path: " << a.applicationFilePath();

            auto settings = GlobalUtilities::applicationSettings("locale");
            auto localeOverride = settings->value("locale_override");
            QString appLanguage;

            if (!localeOverride.isNull())
            {
                appLanguage = localeOverride.toString();
                qInfo() << this << "The app language is" << appLanguage;

                GlobalUtilities::setTranslatorLocale(QLocale(appLanguage));
                qInfo() << this << "Overriding locale for translation to" << GlobalUtilities::translatorLocale();
            }
            else
            {
                appLanguage = QLocale::system().name().split("_").first();
            }

            GlobalUtilities::setContryCode(appLanguage);

            if (translator.load(GlobalUtilities::translatorLocale(), QLatin1String("Capture"), QLatin1String("_"), QLatin1String(":/translations")))
            {
                a.installTranslator(&translator);
                qDebug() << this << "Able to install translator" << GlobalUtilities::translatorLocale() << appLanguage;
            }
            else
            {
                qWarning() << this <<  "Unable to install translator for locale" << GlobalUtilities::translatorLocale();
            }

            QStringList localeRightToLeft{"ar", "he"};
            if (localeRightToLeft.contains(appLanguage))
            {
                qDebug() << this << "The language layout changes to RightToLeft for" << appLanguage;
                GlobalUtilities::setLayoutDirection(Qt::RightToLeft);
            }

            monitor::MonitorWindow w(m_model, compositor);

            mat::MatWindow mat(m_model, compositor);

            presentation::PresentationModeWindow present(m_model, compositor);
            present.setSources(w.stageViewer(), w.inkLayerWidget());

            // Since Mat and Presentation window doesn't have MonitorWindow as parent we need to make sure that closing Monitor or Mat window will close all other windows
            QObject::connect(&w, &monitor::MonitorWindow::closing, &mat, &mat::MatWindow::close);
            QObject::connect(&w, &monitor::MonitorWindow::closing, &present, &presentation::PresentationModeWindow::close);

            QObject::connect(&mat, &mat::MatWindow::closing, &w, &monitor::MonitorWindow::close);
            QObject::connect(&mat, &mat::MatWindow::closing, &present, &presentation::PresentationModeWindow::close);

            QObject::connect(singleInstance.data(), &SingleInstance::receivedArguments, &w, &common::BaseWindow::onSingleInstanceReceivedArguments);
            QObject::connect(singleInstance.data(), &SingleInstance::receivedArguments, &present, &common::BaseWindow::onSingleInstanceReceivedArguments);

            a.processEvents();

            qDebug() << this << "Ready to show";

            // SPROUT-18517 - This should improve display sequence
            QTimer::singleShot(10, [&w, &mat, &present, this] {
                qDebug() << this << "Showing UI";

                w.show();
//                mat.show();
                present.hide();
            });

            exitCode = a.exec();
        }
    }

    return exitCode;
}


bool CaptureWorktool::parseCommandLine(int argc, char *argv[], int* exitCode)
{
    QStringList arguments;

    for(int i=0; i < argc; i++) {
        arguments << argv[i];
    }

    const auto parameters = common::Utilities::parseCommandLine(arguments);

    switch(parameters.first) {
    case common::Utilities::Ok:
        break;
    case common::Utilities::Error:
        QMessageBox::critical(nullptr, monitor::MonitorWindow::tr("WorkTools Capture"), parameters.second.userString);
        *exitCode = 1;
        break;
    case common::Utilities::HelpRequested:
        QMessageBox::information(nullptr, monitor::MonitorWindow::tr("WorkTools Capture"), parameters.second.userString);
        *exitCode = 0;
        break;
    }

    const QString stageGUID = "133D6F29-5F4E-4600-BBB0-2B11EFEA0148";

    if(parameters.first == common::Utilities::Ok && parameters.second.launchedFrom == stageGUID)
    {
        m_model->setSendToStageMode(true);
    }

    return parameters.first == common::Utilities::Ok;
}

} // namespace capture
