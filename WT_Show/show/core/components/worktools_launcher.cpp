#include "worktools_launcher.h"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>

#include "common/utilities.h"
#include "styled_message_box.h"

#ifdef Q_OS_WIN

#include <windows.h>

#endif

const QString DiscoverMessageHtmlFormat = "<html><head/><body>%1</body></html>";
const QString HpSupportHtmlFormat = "<a href=\"https://support.hp.com\"><span style=\" text-decoration: underline; color:#0096d6;\">https://support.hp.com</span></a>";

namespace capture {
namespace components {

WorktoolsLauncher::WorktoolsLauncher(QObject *parent)
    : QObject(parent)
{
    QCoreApplication::instance()->installEventFilter(this);
}

bool WorktoolsLauncher::eventFilter(QObject *obj, QEvent *event)
{
    bool processed = false;

    if (event->type() == event::LaunchWorktoolEvent::type()) {
        if (auto launchWorktoolEvent = static_cast<event::LaunchWorktoolEvent*>(event)) {
            launchWorktool(launchWorktoolEvent->worktool(), launchWorktoolEvent->parameters());

            processed = true;
        }
    }

    return processed ? processed : QObject::eventFilter(obj, event);
}

bool WorktoolsLauncher::launchWorktool(event::LaunchWorktoolEvent::Worktool worktool, QStringList parameters)
{
    qInfo() << this << "Trying to launch worktool" << worktool << "with parameters" << parameters;

    const QString captureGUID = "98F401FE-6EF3-4620-8485-C7D381C95732";

    const QString stageGUID = "133D6F29-5F4E-4600-BBB0-2B11EFEA0148";

    const QString controlGUID = "623850D5-63FC-4844-B4CB-D51EC49C7191";
    const QString controlUPID = "5D058326CF3644844BBC5DE14CC91719";

    const QString discoverGUID = "B9074B05-5FA6-4593-800F-44DEE984F086";
    const QString discoverUPID = "50B4709B6AF5395408F044ED9E480F68";

    QString upgrade_path = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UpgradeCodes\\";
    QSettings settings(upgrade_path, QSettings::NativeFormat);

    QString installGUID = captureGUID;

    switch(worktool)
    {
    case event::LaunchWorktoolEvent::Stage:
    {
        if (common::Utilities::isStageWorkToolInstalled())
        {
            QString stageReg = QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\%1\\Shell\\Open\\Command").arg(stageGUID);
            QSettings stageRegistryKey(stageReg, QSettings::NativeFormat);
            QString stage = stageRegistryKey.value(".").toString().remove(" \"%1\"");
            QString command = QString("%1 %2 from=%3").arg(stage).arg(parameters.join(" ")).arg(captureGUID);
            qInfo() << "Launch stage with command:" << command;
            return QProcess::startDetached(command);
        }
        else
        {
            installGUID = stageGUID;
        }
        break;
    }
    case event::LaunchWorktoolEvent::Control:
    {
        if (settings.childGroups().contains(controlUPID))
        {
            QString command = QString("%1://?from=%2").arg(controlGUID).arg(captureGUID);
            qInfo() << "Launch control with command:" << command;

            ShellExecute(0, 0, reinterpret_cast<const wchar_t*>(command.utf16()), 0, 0, SW_SHOWNORMAL);
            return true;
        }
        else
        {
            installGUID = controlGUID;
        }
        break;
    }
    }

    if(settings.childGroups().contains(discoverUPID))
    {
        QString command = QString("%1://?from=%2&toInstall=%3").arg(discoverGUID).arg(captureGUID).arg(installGUID);
        qInfo() << "Launch discover with command:" << command;

        ShellExecute(0, 0, reinterpret_cast<const wchar_t*>(command.utf16()), 0, 0, SW_SHOWNORMAL);
        return true;
    }
    else
    {
        auto msgBox = common::Utilities::createMessageBox();
        msgBox->setText(tr("Download Discover"));

        const auto htmlMessage = QString(tr("Please visit %1 and download the latest software.")).arg(HpSupportHtmlFormat);

        msgBox->setInformativeText(DiscoverMessageHtmlFormat.arg(htmlMessage));
        msgBox->addStyledButton(tr("OK"), QMessageBox::YesRole);
        msgBox->exec();
    }
    return false;
}

} // namespace components
} // namespace capture
