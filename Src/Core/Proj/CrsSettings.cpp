


#include <cstdlib>

#include <QFileInfo>
#include <QSettings>
#include <QDir>
#include <iostream>
#include "Core/Proj/CrsSettings.h"

namespace AI3D
{
    namespace PROJ
    {

        QSettings* CrsSettings::pCRSSettings = new QSettings("HKEY_CURRENT_USER\\Software\\MoldAI\\RecentCRS", QSettings::NativeFormat);;
        QStringList CrsSettings::GetRecentCrs()
        {
            return pCRSSettings->value(QStringLiteral("Crs/recentAuthId")).toStringList();
        }

        void CrsSettings::RemoveRecentCrs(QString crs)
        {
            QStringList recent = GetRecentCrs();
        
            recent.removeAll(crs);
            
          
            pCRSSettings->setValue(QStringLiteral("Crs/recentAuthId"), recent);
           

        }

        void CrsSettings::RemoveRecentCrsContains(QString partOfCrs)
        {
            QStringList recent = GetRecentCrs();
            QStringList newRecent;

            for (auto& t : recent)
            {
                if (!t.contains(partOfCrs,Qt::CaseInsensitive))
                    newRecent << t;
            }
            
            pCRSSettings->setValue(QStringLiteral("Crs/recentAuthId"), newRecent);
        }

        void CrsSettings::PushRecentCrs(QString crs)
        {
            QStringList recent = GetRecentCrs();
          
            recent.removeAll(crs);
            recent.insert(0, crs);

            
            recent = recent.mid(0, 8);
            QStringList authids;
            authids.reserve(recent.size());
            for (const QString& c : std::as_const(recent))
            {
                authids << crs;

            }
            pCRSSettings->setValue(QStringLiteral("Crs/recentAuthId"), authids);

            

        }


        Q_GLOBAL_STATIC(QString, sGlobalSettingsPath)

            bool CrsSettings::setGlobalSettingsPath(const QString& path)
        {
            if (QFileInfo::exists(path))
            {
                *sGlobalSettingsPath() = path;
                return true;
            }
            return false;
        }

        void CrsSettings::init()
        {
            if (!sGlobalSettingsPath()->isEmpty())
            {
                mGlobalSettings = new QSettings(*sGlobalSettingsPath(), QSettings::IniFormat);
                
            }
        }


        CrsSettings::CrsSettings(const QString& organization, const QString& application, QObject* parent)
        {
            mUserSettings = new QSettings(organization, application, parent);
            init();
        }

        CrsSettings::CrsSettings(QSettings::Scope scope, const QString& organization,
            const QString& application, QObject* parent)
        {
            mUserSettings = new QSettings(scope, organization, application, parent);
            init();
        }

        CrsSettings::CrsSettings(QSettings::Format format, QSettings::Scope scope,
            const QString& organization, const QString& application, QObject* parent)
        {
            mUserSettings = new QSettings(format, scope, organization, application, parent);
            init();
        }

        CrsSettings::CrsSettings(const QString& fileName, QSettings::Format format, QObject* parent)
        {
            mUserSettings = new QSettings(fileName, format, parent);
            init();
        }

        CrsSettings::CrsSettings(QObject* parent)
        {
            mUserSettings = new QSettings(parent);
            init();
        }

        CrsSettings::~CrsSettings()
        {
            delete mUserSettings;
            delete mGlobalSettings;
        }


        void CrsSettings::beginGroup(const QString& prefix, const CrsSettings::Section section)
        {
            QString pKey = prefixedKey(prefix, section);
            mUserSettings->beginGroup(pKey);
            if (mGlobalSettings)
            {
                mGlobalSettings->beginGroup(pKey);
            }
        }

        void CrsSettings::endGroup()
        {
            mUserSettings->endGroup();
            if (mGlobalSettings)
            {
                mGlobalSettings->endGroup();
            }
        }

        QString CrsSettings::group() const
        {
            return mUserSettings->group();
        }

        QStringList CrsSettings::allKeys() const
        {
            QStringList keys = mUserSettings->allKeys();
            if (mGlobalSettings)
            {
                for (auto& s : mGlobalSettings->allKeys())
                {
                    if (!keys.contains(s))
                    {
                        keys.append(s);
                    }
                }
            }
            return keys;
        }


        QStringList CrsSettings::childKeys() const
        {
            QStringList keys = mUserSettings->childKeys();
            if (mGlobalSettings)
            {
                for (auto& s : mGlobalSettings->childKeys())
                {
                    if (!keys.contains(s))
                    {
                        keys.append(s);
                    }
                }
            }
            return keys;
        }

        QStringList CrsSettings::childGroups() const
        {
            QStringList keys = mUserSettings->childGroups();
            if (mGlobalSettings)
            {
                for (auto& s : mGlobalSettings->childGroups())
                {
                    if (!keys.contains(s))
                    {
                        keys.append(s);
                    }
                }
            }
            return keys;
        }
        QStringList CrsSettings::globalChildGroups() const
        {
            QStringList keys;
            if (mGlobalSettings)
            {
                keys = mGlobalSettings->childGroups();
            }
            return keys;
        }

        QString CrsSettings::globalSettingsPath()
        {
            return *sGlobalSettingsPath();
        }

        QVariant CrsSettings::value(const QString& key, const QVariant& defaultValue, const CrsSettings::Section section) const
        {
            QString pKey = prefixedKey(key, section);
            if (!mUserSettings->value(pKey).isNull())
            {
                return mUserSettings->value(pKey);
            }
            if (mGlobalSettings)
            {
                return mGlobalSettings->value(pKey, defaultValue);
            }
            return defaultValue;
        }

        bool CrsSettings::contains(const QString& key, const CrsSettings::Section section) const
        {
            QString pKey = prefixedKey(key, section);
            return mUserSettings->contains(pKey) ||
                (mGlobalSettings && mGlobalSettings->contains(pKey));
        }

        QString CrsSettings::fileName() const
        {
            return mUserSettings->fileName();
        }

        void CrsSettings::sync()
        {
            mUserSettings->sync();
        }

        void CrsSettings::remove(const QString& key, const CrsSettings::Section section)
        {
            QString pKey = prefixedKey(key, section);
            mUserSettings->remove(pKey);
        }

        QString CrsSettings::prefixedKey(const QString& key, const Section section) const
        {
            QString prefix;
            switch (section)
            {
            case Section::Core:
                prefix = QStringLiteral("core");
                break;
            case Section::Server:
                prefix = QStringLiteral("server");
                break;
            case Section::Gui:
                prefix = QStringLiteral("gui");
                break;
            case Section::Plugins:
                prefix = QStringLiteral("plugins");
                break;
            case Section::Misc:
                prefix = QStringLiteral("misc");
                break;
            case Section::Auth:
                prefix = QStringLiteral("auth");
                break;
            case Section::App:
                prefix = QStringLiteral("app");
                break;
            case Section::Providers:
                prefix = QStringLiteral("providers");
                break;
            case Section::Expressions:
                prefix = QStringLiteral("expressions");
                break;
            case Section::NoSection:
                return sanitizeKey(key);
            }
            return prefix + "/" + sanitizeKey(key);
        }


        int CrsSettings::beginReadArray(const QString& prefix)
        {
            int size = mUserSettings->beginReadArray(sanitizeKey(prefix));
            if (0 == size && mGlobalSettings)
            {
                size = mGlobalSettings->beginReadArray(sanitizeKey(prefix));
                mUsingGlobalArray = (size > 0);
            }
            return size;
        }

        void CrsSettings::beginWriteArray(const QString& prefix, int size)
        {
            mUsingGlobalArray = false;
            mUserSettings->beginWriteArray(prefix, size);
        }

        void CrsSettings::endArray()
        {
            mUserSettings->endArray();
            if (mGlobalSettings)
            {
                mGlobalSettings->endArray();
            }
            mUsingGlobalArray = false;
        }

        void CrsSettings::setArrayIndex(int i)
        {
            if (mGlobalSettings && mUsingGlobalArray)
            {
                mGlobalSettings->setArrayIndex(i);
            }
            else
            {
                mUserSettings->setArrayIndex(i);
            }
        }

        void CrsSettings::setValue(const QString& key, const QVariant& value, const CrsSettings::Section section)
        {
            
            
            
            
            
            
            QVariant currentValue = CrsSettings::value(prefixedKey(key, section));
            if ((currentValue.isValid() || value.isValid()) && (currentValue != value))
            {
                mUserSettings->setValue(prefixedKey(key, section), value);
            }
            
            
            
            
            else if (mGlobalSettings && mGlobalSettings->value(prefixedKey(key, section)) == currentValue)
            {
                mUserSettings->remove(prefixedKey(key, section));
            }
        }

        
        QString CrsSettings::sanitizeKey(const QString& key) const
        {
            return QDir::cleanPath(key);
        }

        void CrsSettings::clear()
        {
            mUserSettings->clear();
        }
    }
}