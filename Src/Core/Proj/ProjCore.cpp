


#include "Core/Proj/ProjCore.h"

#include <QCoreApplication>
#include <QColor>
#include <QDate>
#include <QTime>
#include <QLocale>
#include <QDateTime>

#include "Core/Logging.h"

#include <QVariant>
#include <gdal.h>
#include <geos_c.h>
#include <ogr_api.h>

#define xstr(x) str(x)
#define str(x) #x
namespace AI3D
{
    namespace PROJ
    {
        bool VariantEqual(const QVariant& lhs, const QVariant& rhs)
        {
            return (lhs.isNull() == rhs.isNull() && lhs == rhs) || (lhs.isNull() && rhs.isNull() && lhs.isValid() && rhs.isValid());
        }
        double PermissiveToDouble(QString string, bool& ok)
        {
            
            string.remove(QLocale().groupSeparator());
            return QLocale().toDouble(string, &ok);
        }

        int PermissiveToInt(QString string, bool& ok)
        {
            
            string.remove(QLocale().groupSeparator());
            return QLocale().toInt(string, &ok);
        }

        qlonglong PermissiveToLongLong(QString string, bool& ok)
        {
            
            string.remove(QLocale().groupSeparator());
            return QLocale().toLongLong(string, &ok);
        }



        bool VariantLessThan(const QVariant& lhs, const QVariant& rhs)
        {
            
            if (!lhs.isValid())
                return rhs.isValid();
            else if (lhs.isNull())
                return rhs.isValid() && !rhs.isNull();
            else if (!rhs.isValid() || rhs.isNull())
                return false;

            switch (lhs.type())
            {
            case QVariant::Int:
                return lhs.toInt() < rhs.toInt();
            case QVariant::UInt:
                return lhs.toUInt() < rhs.toUInt();
            case QVariant::LongLong:
                return lhs.toLongLong() < rhs.toLongLong();
            case QVariant::ULongLong:
                return lhs.toULongLong() < rhs.toULongLong();
            case QVariant::Double:
                return lhs.toDouble() < rhs.toDouble();
            case QVariant::Char:
                return lhs.toChar() < rhs.toChar();
            case QVariant::Date:
                return lhs.toDate() < rhs.toDate();
            case QVariant::Time:
                return lhs.toTime() < rhs.toTime();
            case QVariant::DateTime:
                return lhs.toDateTime() < rhs.toDateTime();
            case QVariant::Bool:
                return lhs.toBool() < rhs.toBool();

            case QVariant::List:
            {
                const QList<QVariant>& lhsl = lhs.toList();
                const QList<QVariant>& rhsl = rhs.toList();

                int i, n = std::min(lhsl.size(), rhsl.size());
                for (i = 0; i < n && lhsl[i].type() == rhsl[i].type() && VariantEqual(lhsl[i], rhsl[i]); i++)
                    ;

                if (i == n)
                    return lhsl.size() < rhsl.size();
                else
                    return VariantLessThan(lhsl[i], rhsl[i]);
            }

            case QVariant::StringList:
            {
                const QStringList& lhsl = lhs.toStringList();
                const QStringList& rhsl = rhs.toStringList();

                int i, n = std::min(lhsl.size(), rhsl.size());
                for (i = 0; i < n && lhsl[i] == rhsl[i]; i++)
                    ;

                if (i == n)
                    return lhsl.size() < rhsl.size();
                else
                    return lhsl[i] < rhsl[i];
            }

            default:
                return QString::localeAwareCompare(lhs.toString(), rhs.toString()) < 0;
            }
        }

        bool VariantGreaterThan(const QVariant& lhs, const QVariant& rhs)
        {
            return !VariantLessThan(lhs, rhs);
        }



        


    }
}
uint qHash(const QVariant& variant)
{
    if (!variant.isValid() || variant.isNull())
        return std::numeric_limits<uint>::max();

    switch (variant.type())
    {
    case QVariant::Int:
        return qHash(variant.toInt());
    case QVariant::UInt:
        return qHash(variant.toUInt());
    case QVariant::Bool:
        return qHash(variant.toBool());
    case QVariant::Double:
        return qHash(variant.toDouble());
    case QVariant::LongLong:
        return qHash(variant.toLongLong());
    case QVariant::ULongLong:
        return qHash(variant.toULongLong());
    case QVariant::String:
        return qHash(variant.toString());
    case QVariant::Char:
        return qHash(variant.toChar());
    case QVariant::List:
        return qHash(variant.toList());
    case QVariant::StringList:
        return qHash(variant.toStringList());
    case QVariant::ByteArray:
        return qHash(variant.toByteArray());
    case QVariant::Date:
        return qHash(variant.toDate());
    case QVariant::Time:
        return qHash(variant.toTime());
    case QVariant::DateTime:
        return qHash(variant.toDateTime());
    case QVariant::Url:
    case QVariant::Locale:
    case QVariant::RegularExpression:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case QVariant::RegExp:
#endif
        return qHash(variant.toString());
    default:
        break;
    }

    return std::numeric_limits<uint>::max();
}
        template<>
        bool qMapLessThanKey<QVariantList>(const QVariantList& key1, const QVariantList& key2)
        {
            
            
            return AI3D::PROJ::VariantGreaterThan(key1, key2) && key1 != key2;
        }

    
