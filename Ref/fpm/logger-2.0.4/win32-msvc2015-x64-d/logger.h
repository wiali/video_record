#ifndef HP_FORTIS_LOGGER_H
#define HP_FORTIS_LOGGER_H

#define SPDLOG_WCHAR_FILENAMES

#include <QtCore>
#include <memory>
#include <spdlog/spdlog.h>

namespace hp {
namespace fortis {

class Logger
{
public:
    /*! \brief Install the logger on the current process.
     *  \param pkgName The name of the package which is using the Logger. This argument will be used to decide where
     *                 the log will be stored inside the common data folder.
     *  \param filename The name of the file, without extension, where the log will written to.
     */
    static void install(const QString &pkgName, const QString& filename);

    /*! \brief Uninstall the logger, closing all related files.
     */
    static void uninstall();

    /*! \brief Custom message hanlder used to write log to files.
     */
    static void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    /*! \brief Gets file path for currently installed log.
     */
    static const QString& logFilePath();

protected:

    /*! \brief Protect constructor to force using functionality with the install method.
     */
    Logger();

    /*! \brief Logger used to write messages to file.
     */
    static std::shared_ptr<spdlog::logger> m_logger;

    /*! \brief Store previous message handler to be still called after saving content to file.
     */
    static QtMessageHandler m_previousHandler;

    /*! \brief Store information if we should use the previous handler (i.e. log to stdout).
     *  \details Default: true.
     */
    static bool m_usePreviousHandler;

    /*! \brief Store information of the configured log level, to define which messages should processed by the log.
     * Since Qt log level values aren't continuous, this value is fixed to turn the processing easier.
     *  \details Default: info.
     */
    static unsigned int m_currentLogLevel;

    /*! \brief Stores the current path of the log file.
     *  \details Default: %LocalAppData%\<filename>.log
     */
    static QString m_logFilePath;

    /*!
     * \brief Stores the configuration if the log should be writen also using OutputDebugString.
     * \details Default: false.
     */
    static bool m_useOutputDebugString;
};

} }

#endif // HP_FORTIS_LOGGER_H
