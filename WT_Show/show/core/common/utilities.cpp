#include "utilities.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>
#include <QGuiApplication>
#include <QMediaPlayer>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>

#include <global_utilities.h>

#include "styled_message_box.h"
#include "history_manager.h"

#ifdef Q_OS_WIN
#include <tlhelp32.h>
#include <lmcons.h>
#elif
#error Not implemented for this OS
#endif


namespace capture {
namespace common {

typedef QStandardPaths::StandardLocation QStdLocation;
typedef QPair<QString, QStdLocation> Pair_QStdLocation;

const QHash<Utilities::exportFile_format, Pair_QStdLocation> lastSaveConfig = {
    {Utilities::Image,
     Pair_QStdLocation("last_image_save_dialog_path", QStdLocation::PicturesLocation)},
    {Utilities::PDF,
     Pair_QStdLocation("last_pdf_save_dialog_path", QStdLocation::DocumentsLocation)}};

const QString stageUPID = "92F6D331E4F50064BB0BB211FEAE1084";
const QString upgrade_path =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UpgradeCodes\\";
QSharedPointer<HistoryManager> common::Utilities::m_historyMgr;

QRect common::Utilities::calculateAbsoluteViewport(QRectF viewport, QSize resolution)
{
    QPointF topLeft(viewport.x() * resolution.width(), viewport.y() * resolution.height());
    QPointF bottomRight(viewport.bottomRight().x() * (resolution.width() - 1.0),
                        viewport.bottomRight().y() * (resolution.height() - 1.0));

    // Round it up to nearest pixel
    topLeft.setX(std::max(std::round(topLeft.x()), 0.0));
    topLeft.setY(std::max(std::round(topLeft.y()), 0.0));
    bottomRight.setX(std::min(std::round(bottomRight.x()), (double) resolution.width() - 1.0));
    bottomRight.setY(std::min(std::round(bottomRight.y()), (double) resolution.height() - 1.0));

    return QRect(topLeft.toPoint(), bottomRight.toPoint());
}

QString common::Utilities::monitorHardwareIdToDisplayName(QStringList hardwareIds) {
#ifdef Q_OS_WIN
  DISPLAY_DEVICE dd;

  ZeroMemory(&dd, sizeof(dd));
  dd.cb = sizeof(dd);

  for (DWORD i = 0;; i++) {
    if (EnumDisplayDevices(NULL, i, &dd, 0) == 0) {
      break;
    }

    DISPLAY_DEVICE monInfo;
    monInfo.cb = sizeof(monInfo);

    for (DWORD j = 0; EnumDisplayDevices(dd.DeviceName, j, &monInfo, 0);
         j++)  // query all monitors on the adaptor
    {
      auto deviceId = QString::fromWCharArray(monInfo.DeviceID);
      auto deviceName = QString::fromWCharArray(dd.DeviceName);

      qDebug() << "Found device hardware ID" << deviceId << "as" << deviceName;

      foreach (auto hardwareId, hardwareIds) {
        if (deviceId.contains(hardwareId)) {
          return deviceName;
        }
      }
    }
  }

#elif
#error Not implemented for this OS
#endif

  return QString();
}

#ifdef Q_OS_WIN
QString common::Utilities::getLastWin32Error() {
  LPWSTR buffer;
  DWORD errorCode = GetLastError();

  FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      errorCode,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPWSTR) &buffer,
      0, NULL);

  QString errorMessage;
  QTextStream stream(&errorMessage);
  stream << QString::fromWCharArray(buffer).trimmed() << " (" << errorCode << ")";
  LocalFree(buffer);

  return errorMessage;
}

#endif

QString common::Utilities::getLastSavePath(common::Utilities::exportFile_format format) {
  auto openDir = GlobalUtilities::applicationSettings()->value(lastSaveConfig[format].first).toString();

  QDir dir(openDir);
  if (openDir == "" || !dir.exists()) {
    openDir = QStandardPaths::writableLocation(lastSaveConfig[format].second);
    dir.mkdir(openDir);
  }

  return openDir + QDir::separator();
}

QString common::Utilities::saveDialog(exportFile_format format, const QString& title, const bool isFile,
                              const QString& filter, QString *selectedFilter, const QString& name,
                              QFileDialog::Options options) {
  playSound("qrc:/Resources/production/Sounds/popDialog.aif");

  QString openDir = getLastSavePath(format);
  QFileDialog dialog;
  QString result = "";
  if (isFile) {
    openDir += name;
    result = dialog.getSaveFileName(nullptr, title, openDir, filter, selectedFilter, options);
  } else {
    result = dialog.getExistingDirectory(nullptr, title, openDir, options);
  }

  qInfo() << "Save files from a Dialog" << result;

  if (result != "") {
    GlobalUtilities::applicationSettings()->setValue(
        lastSaveConfig[format].first,
        isFile ? QFileInfo(result).dir().absolutePath() : QDir(result).absolutePath());
  }

  return result;
}

void common::Utilities::playSound(const QString& fileUrlName) {
  static QMediaPlayer* player;
  static QMutex mutex;

  QMutexLocker locker(&mutex);

  if (player == nullptr) {
    player = new QMediaPlayer;
    player->setVolume(100);
  }

  if (player && player->isAvailable()) {
    if (GlobalUtilities::applicationSettings()->value("sound_enabled", true).toBool()) {
      player->setMedia(QUrl::fromUserInput(fileUrlName));
      player->play();
    }
  }
}

Utilities::ProcessID common::Utilities::getParentProcessId() {
#ifdef Q_OS_WIN
  ULONG_PTR pbi[6];
  ULONG ulSize = 0;
  LONG(WINAPI * NtQueryInformationProcess)
  (HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation,
   ULONG ProcessInformationLength, PULONG ReturnLength);
  *(FARPROC*)&NtQueryInformationProcess =
      GetProcAddress(LoadLibraryA("NTDLL.DLL"), "NtQueryInformationProcess");
  if (NtQueryInformationProcess) {
    if (NtQueryInformationProcess(GetCurrentProcess(), 0, &pbi, sizeof(pbi), &ulSize) >= 0 &&
        ulSize == sizeof(pbi)) {
      return pbi[5];
    }
  }
  return static_cast<ULONG_PTR>(-1);
#elif
#error Not implemented for this OS
#endif
}

QString common::Utilities::getProcessName(common::Utilities::ProcessID ProcessId) {
#ifdef Q_OS_WIN
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32)) {
      do {
        DWORD th32ProcessID = pe32.th32ProcessID;
        if (th32ProcessID == ProcessId) return QString::fromWCharArray(pe32.szExeFile);
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }
#elif
#error Not implemented for this OS
#endif
  return QString();
}

QString common::Utilities::getParentProcessName() { return getProcessName(getParentProcessId()); }

QString common::Utilities::createNonConflictingName(const QString& name) {
  // Increase the number.
  QString result = name;

  QRegExp rx(QString("\\s*\\((\\d+)\\)$"));

  if (name.contains(rx)) {
    bool isNum = false;
    int originalNum = rx.cap(1).toInt(&isNum);
    qInfo() << "originalNum is " << originalNum << isNum;
    if (isNum) {
      int newNum = originalNum + 1;
      result.replace(rx, QString(" (%1)").arg(newNum));
    }
  } else {
    result = QString("%1 (2)").arg(result);
  }

  return result;
}

QString common::Utilities::currentUserName() {
  QString userName;
#ifdef Q_OS_WIN
  wchar_t username[UNLEN+1];
  DWORD username_len = UNLEN+1;

  if (0 == GetUserName(username, &username_len)) {
      qCritical() << "Failed to read current user name, reason" << getLastWin32Error();
  } else {
      userName = QString::fromStdWString(username);
  }

#elif
#error Not implemented for this OS
#endif
  return userName;
}

bool common::Utilities::isStageWorkToolInstalled() {
  bool isStageWorkToolInstalled = false;

  QSettings settings(upgrade_path, QSettings::NativeFormat);

  if (settings.childGroups().contains(stageUPID)) isStageWorkToolInstalled = true;

  return isStageWorkToolInstalled;
}

void common::Utilities::processCloseEvent(QCloseEvent* event, QSharedPointer<model::ApplicationStateModel> model) {
  bool ignoreEvent = false;

  if (model->projects()->count() > 0) {
    auto messageBox = createMessageBox();

    messageBox->setText(capture::monitor::MonitorWindow::tr("Save images"));
    messageBox->setInformativeText(
        capture::monitor::MonitorWindow::tr("To save space, all images will be discarded when you quit this app. "
                          "Make sure you have exported your images."));

    messageBox->addStyledButton(capture::monitor::MonitorWindow::tr("Discard &&& quit"), QMessageBox::AcceptRole);
    messageBox->addStyledButton(capture::monitor::MonitorWindow::tr("Cancel"), QMessageBox::RejectRole);

    ignoreEvent = messageBox->exec() == QMessageBox::RejectRole;
    if (!ignoreEvent) {
      // Discard all projects

        model->setSelectedProject(QSharedPointer<StageProject>());
        model->projects()->clear();
    }
  }

  if (ignoreEvent) {
    event->ignore();
  } else {
    event->accept();
  }
}

QPair<Utilities::CommandLineParseResult, model::CommandLineParameters> common::Utilities::parseCommandLine(
    const QStringList& input) {
  CommandLineParseResult result = common::Utilities::Ok;
  model::CommandLineParameters parameters;

  QCommandLineParser parser;
  parser.setApplicationDescription(capture::monitor::MonitorWindow::tr("WorkTools Capture"));
  const auto helpOption = parser.addHelpOption();

  QCommandLineOption fromOption(
      QStringList() << "from",
      capture::monitor::MonitorWindow::tr("Parameter indicating from which context was this application launched"),
      capture::monitor::MonitorWindow::tr("context"));
  parser.addOption(fromOption);

  if (!parser.parse(input)) {
    parameters.userString = parser.errorText();
    result = common::Utilities::Error;
  } else if (parser.isSet(helpOption)) {
    parameters.userString = parser.helpText();
    result = common::Utilities::HelpRequested;
  } else if (parser.isSet(fromOption)) {
    parameters.launchedFrom = parser.value(fromOption);
  } else {
    for (auto& argument : input) {
      QRegExp regFrom(".*(from=)(.*)$");
      if (regFrom.indexIn(argument) > -1) {
        parameters.launchedFrom = regFrom.cap(2);
      }
    }
  }

  return qMakePair(result, parameters);
}

bool common::Utilities::bringToTopmost(WId winId) {
#ifdef Q_OS_WIN
  HWND hWnd = reinterpret_cast<HWND>(winId);
  if (hWnd) {
    ShowWindow(hWnd, IsZoomed(hWnd) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);

    WINDOWINFO wi;
    GetWindowInfo(hWnd, &wi);
    bool bTopMost = ((wi.dwExStyle & WS_EX_TOPMOST) == WS_EX_TOPMOST);

    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    if (!bTopMost) {
      SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    }

    SetForegroundWindow(hWnd);

    return true;
  }
#endif

  return false;
}

QSharedPointer<StyledMessageBox> common::Utilities::createMessageBox() {
  // Make sure that dialog box always appears on the monitor screen
  // Fix Sprout-18329 The Capture can not be closed when the Capture on Mat with post capture.

  capture::monitor::MonitorWindow* monitorWindow = getMonitorWindow();

  return QSharedPointer<StyledMessageBox>::create(monitorWindow);
}

capture::monitor::MonitorWindow* common::Utilities::getMonitorWindow() {

  capture::monitor::MonitorWindow* monitorWindow = nullptr;
  auto application = dynamic_cast<QApplication*>(QCoreApplication::instance());

  if (application != nullptr) {
    foreach (auto widget, application->topLevelWidgets()) {
      monitorWindow = dynamic_cast<capture::monitor::MonitorWindow*>(widget);

      if (monitorWindow != nullptr) {
        break;
      }
    }
  }

  return monitorWindow;
}

int common::Utilities::measureButtonTextWidth(QAbstractButton *button) {
    return measureTextWidth(button->font(), button->text());
}

int common::Utilities::measureTextWidth(const QFont& font, const QString& text) {
    return QFontMetrics(font).width(text);
}

event::ChangeMatModeEvent::MatMode common::Utilities::MatModeStateToMatMode(model::ApplicationStateModel::MatModeState matModeState)
{
    switch(matModeState)
    {
    case model::ApplicationStateModel::LampOff:
        return event::ChangeMatModeEvent::LampOff;
    case model::ApplicationStateModel::LampOn:
        return event::ChangeMatModeEvent::LampOn;
    case model::ApplicationStateModel::Desktop:
        return event::ChangeMatModeEvent::Desktop;
    case model::ApplicationStateModel::Flash:
        return event::ChangeMatModeEvent::Flash;
    case model::ApplicationStateModel::Reprojection:
        return event::ChangeMatModeEvent::Reprojection;
    }
    return event::ChangeMatModeEvent::LampOff;
}

QSharedPointer<HistoryManager> common::Utilities::getHistoryManager()
{
    if (!m_historyMgr)
        m_historyMgr = QSharedPointer<HistoryManager>::create();
    return m_historyMgr;
}

QTransform common::Utilities::transformFromViewport(QRectF* viewport, const QSizeF& sourceSize, const QRectF& targetRect) {
    QTransform result;

    // Update viewport to make sure it includes only video frame
    if (viewport->left() < 0) viewport->setLeft(0);
    if (viewport->right() > 1) viewport->setRight(1);
    if (viewport->top() < 0) viewport->setTop(0);
    if (viewport->bottom() > 1) viewport->setBottom(1);

    const QSizeF videoFrameSize(viewport->width() * sourceSize.width(),
                                viewport->height() * sourceSize.height());
    const auto scaledSize = videoFrameSize.scaled(targetRect.size(), Qt::KeepAspectRatio);

    QRectF windowRectangle(QPointF(), scaledSize);
    windowRectangle.moveCenter(targetRect.center());

    QPolygonF sourcePolygon(QRectF(QPointF(), videoFrameSize));
    sourcePolygon.removeLast();

    QPolygonF targetPolygon(windowRectangle);
    targetPolygon.removeLast();

    if (!QTransform::quadToQuad(sourcePolygon, targetPolygon, result)) {
        qWarning() << "Cannot calculate mapping for given viewport";
    }

    return result;
}

} // namespace common
} // namespace capture
