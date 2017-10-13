#include "global_utilities.h"

#include <QApplication>
#include <QTimer>
#include <QEventLoop>
#include <qt_windows.h>

#ifdef Q_OS_WIN
  #include <windows.h>
  #include <ShellScalingAPI.h>
#endif

const QString MONITOR = "monitor";
const QString MAT = "mat";
const QString PRESENT = "present";

QStringList GlobalUtilities::m_matHardwareIds;
QStringList GlobalUtilities::m_monitorHardwareIds;

static QHash<GlobalUtilities::ScreenType, QString> ScreenTypeTranslationTable{
    {GlobalUtilities::ScreenType::MonitorScreen, MONITOR},
    {GlobalUtilities::ScreenType::MatScreen, MAT},
    {GlobalUtilities::ScreenType::PresentScreen, PRESENT}};

bool GlobalUtilities::m_isRightToLeft = false;
QString GlobalUtilities::m_counryCode = "";
QLocale GlobalUtilities::m_translatorLocale;

QMutex GlobalUtilities::m_mutex_render;

bool GlobalUtilities::isRightToLeft()
{
    return m_isRightToLeft;
}

QLocale GlobalUtilities::translatorLocale() { return m_translatorLocale; }

void GlobalUtilities::setTranslatorLocale(const QLocale &translatorLocale) {
    m_translatorLocale = translatorLocale;
}

void GlobalUtilities::setLayoutDirection(Qt::LayoutDirection direction)
{
    auto app = qobject_cast<QGuiApplication*>(QApplication::instance());
    app->setLayoutDirection(direction);
    m_isRightToLeft = direction == Qt::RightToLeft;
}

QSharedPointer<hp::fortis::Settings> GlobalUtilities::applicationSettings()
{
    QString path = QString("HP/%1").arg(QApplication::instance()->applicationName());
    return QSharedPointer<hp::fortis::Settings>::create(path);
}

QSharedPointer<hp::fortis::Settings> GlobalUtilities::applicationSettings(QString group)
{
    QString path = QString("HP/%1").arg(QApplication::instance()->applicationName());
    return QSharedPointer<hp::fortis::Settings>::create(path, group);
}

QScreen* GlobalUtilities::findScreen(const ScreenType& type, int* index)
{
    QScreen* result = nullptr;
    QMap<QString, QString> map = getScreenNames();

    qInfo() << "The monitor screen name is" << map[MONITOR];
    qInfo() << "The mat screen name is" << map[MAT];
    qInfo() << "The present screen name is" << map[PRESENT];

    QList<QScreen*> screens = QGuiApplication::screens();

    int i = 0;
    QString key = ScreenTypeTranslationTable[type];
    foreach (auto screen, screens)
    {
        if(map.contains(key))
        {
            if (screen->name() == map[key])
            {
                if(index)
                {
                    *index = i;
                }
                result = screen;
                return result;
            }
        }
        i++;
    }

    return result;
}

void GlobalUtilities::setHardwareIds(QStringList monitorHardwareIds, QStringList matHardwareIds) {
    m_monitorHardwareIds = monitorHardwareIds;
    m_matHardwareIds = matHardwareIds;
}


QRect GlobalUtilities::findScreenGeometry(const ScreenType& type)
{
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

        for (DWORD j = 0; EnumDisplayDevices(dd.DeviceName, j, &monInfo, 0); j++)  // query all monitors on the adaptor
        {
            auto deviceId = QString::fromWCharArray(monInfo.DeviceID);
            auto deviceName = QString::fromWCharArray(dd.DeviceName);

            qDebug() << "Found device hardware ID" << deviceId << "as" << deviceName;

            DEVMODE defaultMode;

            ZeroMemory(&defaultMode, sizeof(DEVMODE));
            defaultMode.dmSize = sizeof(DEVMODE);

            if (!EnumDisplaySettings(dd.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode))
            {
                qCritical() << "Can not enumerate the monitor setting!";
                break;
            }

            QRect geometry = QRect(defaultMode.dmPosition.x, defaultMode.dmPosition.y,
                               defaultMode.dmPelsWidth, defaultMode.dmPelsHeight);

            bool bPresent = true;
            foreach (auto hardwareId, m_monitorHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    bPresent = false;
                    if(type == ScreenType::MonitorScreen)
                    {
                        qInfo() << "Find Monitor screen, the geometry is" << geometry;
                        return geometry;
                    }
                    break;
                }
            }

            foreach (auto hardwareId, m_matHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    bPresent = false;
                    if(type == ScreenType::MatScreen)
                    {
                        qInfo() << "Find Mat screen, the geometry is" << geometry;
                        return geometry;
                    }
                    break;
                }
            }

            if(type == ScreenType::PresentScreen && bPresent)
            {
                qInfo() << "Find Present screen, the geometry is" << geometry;
                return geometry;
            }
        }
    }

#elif
#error Not implemented for this OS
#endif

    return QRect();
}

QMap<QString, QString> GlobalUtilities::getScreenNames()
{
    QMap<QString, QString> screenMap;
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

        for (DWORD j = 0; EnumDisplayDevices(dd.DeviceName, j, &monInfo, 0); j++)  // query all monitors on the adaptor
        {
            auto deviceId = QString::fromWCharArray(monInfo.DeviceID);
            auto deviceName = QString::fromWCharArray(dd.DeviceName);

            qDebug() << "Found device hardware ID" << deviceId << "as" << deviceName;

            foreach (auto hardwareId, m_monitorHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    screenMap.insert(MONITOR, deviceName);
                    break;
                }
            }

            foreach (auto hardwareId, m_matHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    screenMap.insert(MAT, deviceName);
                    break;
                }
            }

            if(screenMap.contains(MONITOR) && deviceName != screenMap[MONITOR] &&
               screenMap.contains(MAT) && deviceName != screenMap[MAT]) {
                screenMap.insert(PRESENT, deviceName);
            }
        }
    }

#elif
#error Not implemented for this OS
#endif

    return screenMap;
}

QMutex& GlobalUtilities::getRenderMutex()
{
    return m_mutex_render;
}

QString GlobalUtilities::contryCode()
{
    return m_counryCode;
}

void GlobalUtilities::setContryCode(QString code)
{
    m_counryCode = code;
}

QString GlobalUtilities::convertArabicNumber(int num)
{
    QString result = "";
    QString numString = QString::number(num);
    const QMap<QString, QString> arabicNumberMap = {
        {"0","٠"},{"1","١"},{"2","٢"},{"3","٣"},{"4","٤"},
        {"5","٥"},{"6","٦"},{"7","٧"},{"8","٨"},{"9","٩"}
    };

    for(QString string : numString)
    {
        result.append(arabicNumberMap[string]);
    }
    return result;
}

void GlobalUtilities::enableDpiAwareness()
{
#ifdef Q_OS_WIN
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#else
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
}
