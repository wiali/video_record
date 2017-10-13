#ifndef HP_FORTIS_SETTINGS_H
#define HP_FORTIS_SETTINGS_H

#include <QSettings>
#include <QVariant>

namespace hp {
namespace fortis {

/**********************************************************************************************************************/

/*! \brief Fortis settings used to configure Fortis behavior.
 *  \details The settings are stored as a INI file on all platforms.
 *           It will use default Qt's default path for QSettings::SystemScope.
 */
class Settings
{
public:

    /*! \brief Constructs settings.
     *  \param pkgName The name of the package to access settings from.
     */
    Settings(const QString& pkgName);

    /*! \brief Constructs settings for specific group name.
     * \param pkgName The name of the package to access settings from.
     * \param group INI file group name
     */
    Settings(const QString& pkgName, const QString& group);

    /*! \brief Returns the value for setting key. If the setting doesn't exist, returns defaultValue.
     *  \param key Setting's key.
     *  \param defaultValue Value to be returned if not found.
     *  \return Setting's value encapsulated in a QVariant.
     */
    QVariant value(const QString & key, const QVariant & defaultValue = QVariant());

    /*! \brief Sets the value for the specified key. If the value already exists it will be overwritten.
     *  \details If default value for this key exists it will be removed.
     *  \param key Setting's key.
     *  \param value Value to be set.
     */
    void setValue(const QString & key, const QVariant & value);

    /*! \brief Returns a list of keys for specified group or top-level keys if no group was specified.
     */
    QStringList keys();

    /*! \brief Returns a list of child groups for specified group or top-level groups if no group was specified.
     */
    QStringList groups();

    /*! \brief Returns the name of the package to access settings from. */
    inline QString packageName() { return m_pkgName; }

    /*!
     * \brief Returns the default path where settings are stored.
     * \return The path where settings are stored.
     */
    QString defaultPath();

private:

    /*! \brief The name of the package to access settings from. */
    QString m_pkgName;

    /*! \brief QSettings used to access ini file. */
    QSettings m_settings;

protected:

    /*! \brief Created default key name (prefixed with dot) for given key.
     *  \param key Setting's key.
     */
    QString createDefaultKey(const QString& key);
};

/**********************************************************************************************************************/

} // fortis
} // hp

#endif // HP_FORTIS_SETTINGS_H
