


#ifndef PROJSETTINGS_H
#define PROJSETTINGS_H

#include <QSettings>
#include <QMetaEnum>

#include "Constants.h"
#include "Core/Logging.h"
namespace AI3D
{
    namespace PROJ
    {

        
        class AI3D_API CrsSettings : public QObject
        {
            Q_OBJECT
        public:

            
            enum Section
            {
                NoSection,
                Core,
                Gui,
                Server,
                Plugins,
                Auth,
                App,
                Providers,
                Expressions,
                Misc
            };

            
            explicit CrsSettings(const QString& organization,
                const QString& application = QString(), QObject* parent = nullptr);

            
            CrsSettings(QSettings::Scope scope, const QString& organization,
                const QString& application = QString(), QObject* parent = nullptr);

            
            CrsSettings(QSettings::Format format, QSettings::Scope scope, const QString& organization,
                const QString& application = QString(), QObject* parent = nullptr);

            
            CrsSettings(const QString& fileName, QSettings::Format format, QObject* parent = nullptr);

            
            explicit CrsSettings(QObject* parent = nullptr);
            ~CrsSettings() override;

            
            void beginGroup(const QString& prefix, CrsSettings::Section section = CrsSettings::NoSection);
            
            void endGroup();

            
            QString group() const;

            
            QStringList allKeys() const;
            
            QStringList childKeys() const;
            
            QStringList childGroups() const;
            
            QStringList globalChildGroups() const;
            
            static QString globalSettingsPath();
            
            static bool setGlobalSettingsPath(const QString& path);
            
            int beginReadArray(const QString& prefix);

            
            void beginWriteArray(const QString& prefix, int size = -1);
            
            void endArray();

            
            void setArrayIndex(int i);

            
            void setValue(const QString& key, const QVariant& value, CrsSettings::Section section = CrsSettings::NoSection);

            
#ifndef SIP_RUN
            QVariant value(const QString& key, const QVariant& defaultValue = QVariant(),
                Section section = NoSection) const;
#else
            SIP_PYOBJECT value(const QString& key, const QVariant& defaultValue = QVariant(),
                SIP_PYOBJECT type = 0,
                QgsSettings::Section section = QgsSettings::NoSection) const / ReleaseGIL / ;
            % MethodCode
                typedef PyObject* (*pyqt5_from_qvariant_by_type)(QVariant& value, PyObject* type);
            QVariant value;

            
            Py_BEGIN_ALLOW_THREADS
                value = sipCpp->value(*a0, *a1, a3);
            Py_END_ALLOW_THREADS

                pyqt5_from_qvariant_by_type f = (pyqt5_from_qvariant_by_type)sipImportSymbol("pyqt5_from_qvariant_by_type");
            sipRes = f(value, a2);

            sipIsErr = !sipRes;
            % End
#endif

#ifndef SIP_RUN

                
                template <class T>
            T enumValue(const QString& key, const T& defaultValue,
                const Section section = NoSection)
            {
                QMetaEnum metaEnum = QMetaEnum::fromType<T>();
                Q_ASSERT(metaEnum.isValid());
                if (!metaEnum.isValid())
                {
                    LOGI("Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration.");
                }

                T v;
                bool ok = false;

                if (metaEnum.isValid())
                {
                    
                    QByteArray ba = value(key, metaEnum.valueToKey(static_cast<int>(defaultValue)), section).toString().toUtf8();
                    const char* vs = ba.data();
                    v = static_cast<T>(metaEnum.keyToValue(vs, &ok));
                    if (ok)
                        return v;
                }

                
                
                
                v = static_cast<T>(value(key, static_cast<int>(defaultValue), section).toInt(&ok));
                if (metaEnum.isValid())
                {
                    if (!ok || !metaEnum.valueToKey(static_cast<int>(v)))
                    {
                        v = defaultValue;
                    }
                    else
                    {
                        
                        
                        setEnumValue(key, v, section);
                    }
                }

                return v;
            }

            
            template <class T>
            void setEnumValue(const QString& key, const T& value,
                const Section section = NoSection)
            {
                QMetaEnum metaEnum = QMetaEnum::fromType<T>();
                Q_ASSERT(metaEnum.isValid());
                if (metaEnum.isValid())
                {
                    setValue(key, metaEnum.valueToKey(static_cast<int>(value)), section);
                }
                else
                {
                    
                }
            }

            
            template <class T>
            T flagValue(const QString& key, const T& defaultValue,
                const Section section = NoSection)
            {
                QMetaEnum metaEnum = QMetaEnum::fromType<T>();
                Q_ASSERT(metaEnum.isValid());
                if (!metaEnum.isValid())
                {
                    LOGI("Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration.");
                }

                T v = defaultValue;
                bool ok = false;

                if (metaEnum.isValid())
                {
                    
                    QByteArray ba = value(key, metaEnum.valueToKeys(defaultValue)).toString().toUtf8();
                    const char* vs = ba.data();
                    v = static_cast<T>(metaEnum.keysToValue(vs, &ok));
                }
                if (!ok)
                {
                    
                    
                    
                    v = T(value(key, static_cast<int>(defaultValue), section).toInt(&ok));
                    if (metaEnum.isValid())
                    {
                        if (!ok || metaEnum.valueToKeys(static_cast<int>(v)).isEmpty())
                        {
                            v = defaultValue;
                        }
                        else
                        {
                            
                            
                            setFlagValue(key, v, section);
                        }
                    }
                }

                return v;
            }

            
            template <class T>
            void setFlagValue(const QString& key, const T& value,
                const Section section = NoSection)
            {
                QMetaEnum metaEnum = QMetaEnum::fromType<T>();
                Q_ASSERT(metaEnum.isValid());
                if (metaEnum.isValid())
                {
                    setValue(key, metaEnum.valueToKeys(value), section);
                }
                else
                {
                 
                }
            }
#endif

            
            bool contains(const QString& key, CrsSettings::Section section = CrsSettings::NoSection) const;
            
            QString fileName() const;

            
            void sync();
            
            void remove(const QString& key, CrsSettings::Section section = CrsSettings::NoSection);
            
            QString prefixedKey(const QString& key, CrsSettings::Section section) const;
            
            void clear();
            static QStringList GetRecentCrs();
            static void PushRecentCrs(QString crs);
            static void RemoveRecentCrs(QString crs);
            static void RemoveRecentCrsContains(QString partOfCrs);
            static QSettings* pCRSSettings;
        private:
            void init();
            QString sanitizeKey(const QString& key) const;
            QSettings* mUserSettings = nullptr;
            QSettings* mGlobalSettings = nullptr;
            bool mUsingGlobalArray = false;
            Q_DISABLE_COPY(CrsSettings)

        };
    }
}
#endif 
