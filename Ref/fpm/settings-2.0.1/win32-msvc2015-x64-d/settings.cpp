#include "settings.h"

#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>

using namespace hp::fortis;

/**********************************************************************************************************************/

QString Settings::defaultPath()
{
#ifdef Q_OS_WIN
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Usually we want to use "%LocalAppData%" as data root folder
    QDir dataFolder = QDir(env.value("LocalAppData") + "/" + m_pkgName);

    // For SYSTEM account this folder is under C:\Windows\system32
    // In this case, use "%ProgramData%"
    QString systemRoot = QDir(env.value("SystemRoot")).absolutePath();

    // Change for system-wide data folder if required
    if (dataFolder.absolutePath().contains(systemRoot))
    {
        dataFolder = QDir(env.value("ProgramData") + "/" + m_pkgName);
    }

    return dataFolder.path();
#elif
#error Not supported on this platform yet
#endif
}

/**********************************************************************************************************************/

Settings::Settings(const QString& pkgName)
    : m_pkgName(pkgName)
    , m_settings(defaultPath() + "/config.ini", QSettings::IniFormat)
{}

/**********************************************************************************************************************/

Settings::Settings(const QString& pkgName, const QString& group)
    : m_pkgName(pkgName)
    , m_settings(defaultPath() + "/config.ini", QSettings::IniFormat)
{
    m_settings.beginGroup(group);
}

/**********************************************************************************************************************/

QString Settings::createDefaultKey(const QString& key)
{
    QString defaultKey = key;
    defaultKey.insert(key.lastIndexOf("/") + 1, '.');

    return defaultKey;
}

/**********************************************************************************************************************/

QVariant Settings::value(const QString & key, const QVariant & defaultValue)
{
    if (!m_settings.contains(key))
    {
        QString defaultKey = createDefaultKey(key);

        // We are only going to write to setting if the default value isn't there or is
        // not a match to what is expected. This will avoid re-writing the file
        if (!m_settings.contains(defaultKey) || m_settings.value(defaultKey) != defaultValue)
        {
            m_settings.setValue(defaultKey, defaultValue);
        }
    }
    return m_settings.value(key, defaultValue);
}

/**********************************************************************************************************************/

QStringList Settings::keys()
{
    return m_settings.childKeys();
}

/**********************************************************************************************************************/

void Settings::setValue(const QString & key, const QVariant & value)
{
    QString defaultKey = createDefaultKey(key);
    m_settings.remove(defaultKey);

    m_settings.setValue(key, value);
}

/**********************************************************************************************************************/

QStringList Settings::groups()
{
    return m_settings.childGroups();
}

/**********************************************************************************************************************/
