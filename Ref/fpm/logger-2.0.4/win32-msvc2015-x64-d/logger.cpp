#include "logger.h"
#include "settings.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include "windows.h"
#endif

using namespace hp::fortis;

QtMessageHandler Logger::m_previousHandler;
bool Logger::m_usePreviousHandler;
bool Logger::m_useOutputDebugString;
unsigned int Logger::m_currentLogLevel;
QString Logger::m_logFilePath;
std::shared_ptr<spdlog::logger> Logger::m_logger;

/**********************************************************************************************************************
 * Static functions
 **********************************************************************************************************************/

static QtMsgType qStringToQtMsgType(QString maxLogLevel)
{
    QString option = maxLogLevel.toLower();

    if (option == "debug")
        return QtDebugMsg;
    else if (option == "info")
        return QtInfoMsg;
    else if (option == "warning")
        return QtWarningMsg;
    else if (option == "critical")
        return QtCriticalMsg;
    else if (option == "fatal")
        return QtFatalMsg;
    else
        return QtCriticalMsg;
}

/* Unfortunately Qt messed up with log levels values.
 * According the documentation, the log levels, in decreasing verbosity order, are:
 * QtDebugMsg QtInfoMsg QtWarningMsg QtCriticalMsg QtFatalMsg. Their value are 0, 4, 1, 2 and 3, respectively.
 * To allow the use of ranges, we "normalized" these values, to be 0, 1, 2, 4, 6.
 */
static unsigned int fixQtMsgTypeIndex(QtMsgType type)
{
    return (type*2)%7;
}

/**********************************************************************************************************************
 * Methods
 **********************************************************************************************************************/

void Logger::install(const QString &pkgName, const QString& filename)
{
    hp::fortis::Settings settings(pkgName, "log");
    QString defaultPattern = "[%{time yyyy-MM-dd HH:mm:ss.zzz}] [%{type}] [pid: %{pid} tid:%{threadid}] (%{file}:%{line}) %{message}";
    qSetMessagePattern(settings.value("pattern", defaultPattern).toString());

    m_usePreviousHandler = settings.value("keep_previous_handler", true).toBool();

    QString confLogLevel = settings.value("verbosity", "debug").toString();
    m_currentLogLevel = fixQtMsgTypeIndex(qStringToQtMsgType(confLogLevel));

    int maxLogFiles = settings.value("number_of_files", 5).toInt();
    int maxLogSize = settings.value("max_log_size", 10000000).toInt();
    bool immediateFlush = settings.value("immediate_flush", false).toBool();

    m_useOutputDebugString = settings.value("output_debug_string", false).toBool();

    QDir logPath = QDir(settings.defaultPath());
    logPath.mkpath(".");

    m_logFilePath = logPath.absoluteFilePath(filename);
    m_logger = spdlog::create<spdlog::sinks::rotating_file_sink_mt>(
        filename.toStdString(),
        m_logFilePath.toStdWString(),
        SPDLOG_FILENAME_T("log"),
        maxLogSize,
        maxLogFiles,
        immediateFlush);

    m_logFilePath.append(".log");

    // We're bypassing spdlog formatter, considering the entire message as "The actual text to log".
    // Reference: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    m_logger->set_pattern("%v");
    m_previousHandler = qInstallMessageHandler(Logger::customMessageHandler);
}

/**********************************************************************************************************************/

void Logger::uninstall()
{
    qDebug() << "Logger::uninstall called.";
    m_logger->flush();
    spdlog::drop_all();
    m_logger.reset();
    qInstallMessageHandler(m_previousHandler);
}

/**********************************************************************************************************************/

void Logger::customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (m_currentLogLevel <= fixQtMsgTypeIndex(type))
    {

        char* fileName = strdup(QFileInfo(context.file).fileName().toUtf8().data());
        QMessageLogContext contextCpy(fileName,
                                  context.line,
                                  context.function,
                                  context.category);

        // format message
        QString formatedMsg = qFormatLogMessage(type, contextCpy, msg);
        free(fileName);

#ifdef Q_OS_WIN
        if (m_useOutputDebugString)
        {
            auto message = reinterpret_cast<const wchar_t*>(formatedMsg.utf16());
            OutputDebugString(message);
        }
#endif

        // Since we are formatting and filtering log verbosity using Qt,
        // we can log all messages using the same log level. In this case, info.
        m_logger->info(formatedMsg.toStdString());
    }

    // call previous handle to keep existing behavior
    if (m_usePreviousHandler)
    {
        m_previousHandler(type, context, msg);
    }
}

/**********************************************************************************************************************/

const QString& Logger::logFilePath()
{
    return m_logFilePath;
}

/**********************************************************************************************************************/
