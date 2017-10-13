#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#include <QRect>
#include <QStringList>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSharedDataPointer>

#include <settings.h>
#include <qt_windows.h>
#include <styled_message_box.h>

#include "model/application_state_model.h"
#include "model/command_line_parameters.h"
#include "monitor/monitor_window.h"

namespace capture {
namespace common {

/*!
 * \brief The Utilities class contains code that is used from various classes but it's too small to be extracted to own class.
 */
class HistoryManager;
class Utilities
{
public:
    enum exportFile_format
    {
        Image=0,
        PDF
    };

    enum CommandLineParseResult {
        Ok,
        Error,
        HelpRequested
    };

#ifdef Q_OS_WIN
    typedef ULONG_PTR ProcessID;
#endif

    /*!
     * \brief Calculates viewport in absolute units (pixels) from relative units based on given resolution.
     * \details This method rounds the absolute units to closest integer value.
     * \param viewport Relative viewport between (0;0) and (1;1)
     * \param resolution Resolution in pixels.
     * \return Viewport in pixels.
     */
    static QRect calculateAbsoluteViewport(QRectF viewport, QSize resolution);

    /*!
     * \brief Find screen name in `QScreen` format based on given hardware ID.
     * \param hardwareIds List of hardware IDs to look into.
     * \return Screen name.
     */
    static QString monitorHardwareIdToDisplayName(QStringList hardwareIds);

    static QString saveDialog( exportFile_format format, const QString& title, const bool isFile = true, const QString& filter = "",
                               QString *selectedFilter = Q_NULLPTR, const QString& name = "", QFileDialog::Options options = QFileDialog::Options());

    static void playSound(const QString& fileUrlName);

    static QString getParentProcessName();    

    static QString createNonConflictingName(const QString& name);

    static QString currentUserName();

    static bool isStageWorkToolInstalled();

    static void processCloseEvent(QCloseEvent* event, QSharedPointer<model::ApplicationStateModel> model);

    static QPair<CommandLineParseResult, model::CommandLineParameters> parseCommandLine(const QStringList& input);

    static bool bringToTopmost(WId winId);

    static QSharedPointer<StyledMessageBox> createMessageBox();

    static capture::monitor::MonitorWindow *getMonitorWindow();

    static QTransform transformFromViewport(QRectF *viewport, const QSizeF &sourceSize, const QRectF &targetRect);

#ifdef Q_OS_WIN
    static QString getLastWin32Error();
#endif

    static int measureButtonTextWidth(QAbstractButton *button);

    static int measureTextWidth(const QFont& font, const QString& text);

    static event::ChangeMatModeEvent::MatMode MatModeStateToMatMode(model::ApplicationStateModel::MatModeState matModeState);

    static QSharedPointer<HistoryManager> getHistoryManager();  

private:
    static ProcessID getParentProcessId();
    static QString getProcessName(ProcessID ProcessId);
    static QString getLastSavePath(exportFile_format format);

private:
    static QSharedPointer<HistoryManager> m_historyMgr;
};

} // namespace common
} // namespace capture

#endif // UTILITIES_H
