#ifndef GLOBAL_UTILITIES_H
#define GLOBAL_UTILITIES_H

#include <QWidget>
#include <QScreen>
#include <QMutex>

#include "shared_global.h"
#include "settings.h"

class SHAREDSHARED_EXPORT GlobalUtilities
{
public:

    enum ScreenType
    {
        MonitorScreen = 0,
        MatScreen = 1,
        PresentScreen = 2
    };

    static bool isRightToLeft();

    static void setLayoutDirection(Qt::LayoutDirection);

    static QSharedPointer<hp::fortis::Settings> applicationSettings();

    static QSharedPointer<hp::fortis::Settings> applicationSettings(QString group);

    static QScreen* findScreen(const ScreenType& type, int* index = nullptr);

    static QRect findScreenGeometry(const ScreenType &type);

    static QMutex& getRenderMutex();

    static QString contryCode();

    static void setContryCode(QString code);

    static QString convertArabicNumber(int num);

    static void enableDpiAwareness();

    static void setHardwareIds(QStringList monitorHardwareIds, QStringList matHardwareIds);

    static QStringList monitorHardwareIds() { return m_monitorHardwareIds; }

    static QStringList matHardwareIds() { return m_matHardwareIds; }

    static QLocale translatorLocale();

    static void setTranslatorLocale(const QLocale& translatorLocale);

private slots:
    static QMap<QString, QString> getScreenNames();

private:
    static QStringList m_monitorHardwareIds;
    static QStringList m_matHardwareIds;
    static bool m_isRightToLeft;
    static QMutex m_mutex_render;
    static QString m_counryCode;
    static QLocale m_translatorLocale;
};

#endif // GLOBAL_UTILITIES_H
