



#ifndef PROJCORE_H
#define PROJCORE_H

#include <QString>
#include <QMetaEnum>
#include <cfloat>
#include <memory>
#include <cmath>

#include <QVariant>
#include <QList>
#include <QSet>
#include <QVariantList>
#include "Constants.h"


#define QHASH_FOR_CLASS_ENUM(T) \
  inline uint qHash(const T &t, uint seed) { \
    return ::qHash(static_cast<typename std::underlying_type<T>::type>(t), seed); \
  }

AI3D_API uint qHash(const QVariant& variant);
#define SIP_MONKEYPATCH_SCOPEENUM
#define SIP_MONKEYPATCH_SCOPEENUM_UNNEST(OUTSIDE_CLASS,FORMERNAME)
#define SIP_MONKEYPATCH_FLAGS_UNNEST(OUTSIDE_CLASS,FORMERNAME)
#define SIP_MONKEYPATCH_COMPAT_NAME(FORMERNAME)
#define SIP_SKIP
#define SIP_DEPRECATED
#define SIP_ENUM_BASETYPE(type)

namespace AI3D
{
    namespace PROJ
    {
        template<typename To, typename From> inline To down_cast(From* f)
        {
            static_assert(
                (std::is_base_of<From,
                    typename std::remove_pointer<To>::type>::value),
                "target type not derived from source type");
            Q_ASSERT(f == nullptr || dynamic_cast<To>(f) != nullptr);
            return static_cast<To>(f);
        }
        class AI3D_API ProjCore
        {
            Q_GADGET
                Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

        public:
           

            
            enum class CrsType : int
            {
                Unknown, 
                Geodetic, 
                Geocentric, 
                Geographic2d, 
                Geographic3d, 
                Vertical, 
                Projected, 
                Compound, 
                Temporal, 
                Engineering, 
                Bound, 
                Other, 
                DerivedProjected, 
            };
            Q_ENUM(CrsType)

                
                enum class CrsAxisDirection : int
            {
                North, 
                NorthNorthEast, 
                NorthEast, 
                EastNorthEast, 
                East, 
                EastSouthEast, 
                SouthEast, 
                SouthSouthEast, 
                South, 
                SouthSouthWest, 
                SouthWest, 
                WestSouthWest, 
                West, 
                WestNorthWest, 
                NorthWest, 
                NorthNorthWest, 
                GeocentricX, 
                GeocentricY, 
                GeocentricZ, 
                Up, 
                Down, 
                Forward, 
                Aft, 
                Port, 
                Starboard, 
                Clockwise, 
                CounterClockwise, 
                ColumnPositive, 
                ColumnNegative, 
                RowPositive, 
                RowNegative, 
                DisplayRight, 
                DisplayLeft, 
                DisplayUp, 
                DisplayDown, 
                Future, 
                Past, 
                Towards, 
                AwayFrom, 
                Unspecified, 
            };
            Q_ENUM(CrsAxisDirection)

                
                enum class CoordinateOrder : int
            {
                Default, 
                XY, 
                YX, 
            };
            Q_ENUM(CoordinateOrder)

                
                enum class CrsIdentifierType SIP_MONKEYPATCH_SCOPEENUM_UNNEST(QgsCoordinateReferenceSystem, IdentifierType) : int
            {
                ShortString, 
                    MediumString, 
                    FullString, 
            };
            Q_ENUM(CrsIdentifierType)

                
                enum class CrsWktVariant SIP_MONKEYPATCH_SCOPEENUM_UNNEST(QgsCoordinateReferenceSystem, WktVariant) : int
            {
                Wkt1Gdal SIP_MONKEYPATCH_COMPAT_NAME(WKT1_GDAL), 
                    Wkt1Esri SIP_MONKEYPATCH_COMPAT_NAME(WKT1_ESRI), 
                    Wkt2_2015 SIP_MONKEYPATCH_COMPAT_NAME(WKT2_2015), 
                    Wkt2_2015Simplified SIP_MONKEYPATCH_COMPAT_NAME(WKT2_2015_SIMPLIFIED), 
                    Wkt2_2019  SIP_MONKEYPATCH_COMPAT_NAME(WKT2_2019), 
                    Wkt2_2019Simplified  SIP_MONKEYPATCH_COMPAT_NAME(WKT2_2019_SIMPLIFIED), 
                    Preferred SIP_MONKEYPATCH_COMPAT_NAME(WKT_PREFERRED) = Wkt2_2019, 
                    PreferredSimplified  SIP_MONKEYPATCH_COMPAT_NAME(WKT_PREFERRED_SIMPLIFIED) = Wkt2_2019Simplified, 
                    PreferredGdal SIP_MONKEYPATCH_COMPAT_NAME(WKT_PREFERRED_GDAL) = Wkt2_2019, 
            };
            Q_ENUM(CrsWktVariant)

                
                enum class Axis : int
            {
                X, 
                Y, 
                Z 
            };
            Q_ENUM(Axis)

                
                enum class DistanceUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST(UnitTypes, DistanceUnit) : int
            {
                Meters SIP_MONKEYPATCH_COMPAT_NAME(DistanceMeters), 
                    Kilometers SIP_MONKEYPATCH_COMPAT_NAME(DistanceKilometers), 
                    Feet SIP_MONKEYPATCH_COMPAT_NAME(DistanceFeet), 
                    NauticalMiles SIP_MONKEYPATCH_COMPAT_NAME(DistanceNauticalMiles), 
                    Yards SIP_MONKEYPATCH_COMPAT_NAME(DistanceYards), 
                    Miles SIP_MONKEYPATCH_COMPAT_NAME(DistanceMiles), 
                    Degrees SIP_MONKEYPATCH_COMPAT_NAME(DistanceDegrees), 
                    Centimeters SIP_MONKEYPATCH_COMPAT_NAME(DistanceCentimeters), 
                    Millimeters SIP_MONKEYPATCH_COMPAT_NAME(DistanceMillimeters), 
                    Inches, 
                    Unknown SIP_MONKEYPATCH_COMPAT_NAME(DistanceUnknownUnit), 
            };
            Q_ENUM(DistanceUnit)

                
                enum class CrsDefinitionFormat SIP_MONKEYPATCH_SCOPEENUM_UNNEST(QgsCoordinateReferenceSystem, Format) : int
            {
                Wkt SIP_MONKEYPATCH_COMPAT_NAME(FormatWkt), 
                    Proj SIP_MONKEYPATCH_COMPAT_NAME(FormatProj), 
                    ENU SIP_MONKEYPATCH_COMPAT_NAME(FormatENU),
            };
            Q_ENUM(CrsDefinitionFormat)
        };


        
        
        inline   QString DistanceUnittoString(ProjCore::DistanceUnit unit)
        {
            switch (unit)
            {
            case ProjCore::DistanceUnit::Meters:
                return QObject::tr("meters", "distance");

            case ProjCore::DistanceUnit::Kilometers:
                return QObject::tr("kilometers", "distance");

            case ProjCore::DistanceUnit::Feet:
                return QObject::tr("feet", "distance");

            case ProjCore::DistanceUnit::Yards:
                return QObject::tr("yards", "distance");

            case ProjCore::DistanceUnit::Miles:
                return QObject::tr("miles", "distance");

            case ProjCore::DistanceUnit::Degrees:
                return QObject::tr("degrees", "distance");

            case ProjCore::DistanceUnit::Centimeters:
                return QObject::tr("centimeters", "distance");

            case ProjCore::DistanceUnit::Millimeters:
                return QObject::tr("millimeters", "distance");

            case ProjCore::DistanceUnit::Unknown:
                return QObject::tr("<unknown>", "distance");

            case ProjCore::DistanceUnit::NauticalMiles:
                return QObject::tr("nautical miles", "distance");
            }
            return QString();
        }
        
        inline QString DoubleToQString(double a, int precision = 17)
        {
            QString str;
            if (precision)
            {
                if (precision < 0)
                {
                    const double roundFactor = std::pow(10, -precision);
                    str = QString::number(static_cast<long long>(std::round(a / roundFactor) * roundFactor));
                }
                else
                {
                    str = QString::number(a, 'f', precision);
                    if (str.contains(QLatin1Char('.')))
                    {
                        
                        int idx = str.length() - 1;
                        while (str.at(idx) == '0' && idx > 1)
                        {
                            idx--;
                        }
                        if (idx < str.length() - 1)
                            str.truncate(str.at(idx) == '.' ? idx : idx + 1);
                    }
                }
            }
            else
            {
                str = QString::number(a, 'f', precision);
            }
            
            
            if (str == QLatin1String("-0"))
            {
                return QLatin1String("0");
            }
            return str;
        }





        
        template<class T> const QMap<T, QString> EnumMap() SIP_SKIP
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
            Q_ASSERT(metaEnum.isValid());
            QMap<T, QString> enumMap;
            for (int idx = 0; idx < metaEnum.keyCount(); ++idx)
            {
                enumMap.insert(static_cast<T>(metaEnum.value(idx)), QString(metaEnum.key(idx)));
            }
            return enumMap;
        }

        
        template<class T> QString EnumValueToKey(const T& value, bool* returnOk = nullptr) SIP_SKIP
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
            Q_ASSERT(metaEnum.isValid());
            const char* key = metaEnum.valueToKey(static_cast<int>(value));
            if (returnOk)
            {
                *returnOk = key ? true : false;
            }
            return QString::fromUtf8(key);
        }

        
        template<class T> T EnumKeyToValue(const QString& key, const T& defaultValue, bool tryValueAsKey = true, bool* returnOk = nullptr) SIP_SKIP
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
            Q_ASSERT(metaEnum.isValid());
            bool ok = false;
            T v = static_cast<T>(metaEnum.keyToValue(key.toUtf8().data(), &ok));
            if (returnOk)
            {
                *returnOk = ok;
            }
            if (ok)
            {
                return v;
            }
            else
            {
                
                if (tryValueAsKey)
                {
                    bool canConvert = false;
                    const int intValue = key.toInt(&canConvert);
                    if (canConvert && metaEnum.valueToKey(intValue))
                    {
                        if (returnOk)
                        {
                            *returnOk = true;
                        }
                        return static_cast<T>(intValue);
                    }
                }
            }
            return defaultValue;
        }



        
        AI3D_API double PermissiveToDouble(QString string, bool& ok);

        
        AI3D_API int PermissiveToInt(QString string, bool& ok);

        
        AI3D_API qlonglong PermissiveToLongLong(QString string, bool& ok);

        
        AI3D_API bool VariantLessThan(const QVariant& lhs, const QVariant& rhs);

        
        AI3D_API bool VariantEqual(const QVariant& lhs, const QVariant& rhs);

        
        AI3D_API bool VariantGreaterThan(const QVariant& lhs, const QVariant& rhs);





#ifdef _MSC_VER
#define CONSTLATIN1STRING inline const QLatin1String
#else
#define CONSTLATIN1STRING constexpr QLatin1String
#endif



        
      
        
        CONSTLATIN1STRING geoWkt()
        {
            return QLatin1String(
                R"""(GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],USAGE[SCOPE["unknown"],AREA["World"],BBOX[-90,-180,90,180]],ID["EPSG",4326]] )"""
            );
        }

        
        CONSTLATIN1STRING geoProj4()
        {
            return QLatin1String("+proj=longlat +datum=WGS84 +no_defs");
        }

        
        CONSTLATIN1STRING geoEpsgCrsAuthId()
        {
            return QLatin1String("EPSG:4326");
        }

        
        CONSTLATIN1STRING geoNone()
        {
            return QLatin1String("NONE");
        }

        

        
        const int PREVIEW_JOB_DELAY_MS = 250;

        
        const int MAXIMUM_LAYER_PREVIEW_TIME_MS = 250;

        



        
        const long GEOSRID = 4326;

        
        const long GEOCRS_ID = 3452;

        
        const long GEO_EPSG_CRS_ID = 4326;

        
        const int USER_CRS_START_ID = 100000;

        
        
        

        
        const double DEFAULT_POINT_SIZE = 2.0;
        const double DEFAULT_LINE_WIDTH = 0.26;

        
        const double DEFAULT_SEGMENT_EPSILON = 1e-8;

    }
}

template<typename T> AI3D_API bool qMapLessThanKey(const T& key1, const T& key2);

#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)

#define Q_NOWARN_DEPRECATED_PUSH \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"");
#define Q_NOWARN_DEPRECATED_POP \
  _Pragma("GCC diagnostic pop");
#define Q_NOWARN_UNREACHABLE_PUSH
#define Q_NOWARN_UNREACHABLE_POP

#elif defined(_MSC_VER)

#define Q_NOWARN_DEPRECATED_PUSH \
  __pragma(warning(push)) \
  __pragma(warning(disable:4996))
#define Q_NOWARN_DEPRECATED_POP \
  __pragma(warning(pop))
#define Q_NOWARN_UNREACHABLE_PUSH \
  __pragma(warning(push)) \
  __pragma(warning(disable:4702))
#define Q_NOWARN_UNREACHABLE_POP \
  __pragma(warning(pop))

#else

#define Q_NOWARN_DEPRECATED_PUSH
#define Q_NOWARN_DEPRECATED_POP
#define Q_NOWARN_UNREACHABLE_PUSH
#define Q_NOWARN_UNREACHABLE_POP

#endif



#if __cplusplus >= 201703L
#define NODISCARD [[nodiscard]]
#elif defined(__clang__)
#define NODISCARD [[nodiscard]]
#elif defined(_MSC_VER)
#define NODISCARD 
#elif defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define NODISCARD [[nodiscard]]
#elif __has_cpp_attribute(gnu::warn_unused_result)
#define NODISCARD [[gnu::warn_unused_result]]
#else
#define NODISCARD Q_REQUIRED_RESULT
#endif
#else
#define NODISCARD Q_REQUIRED_RESULT
#endif

#if __cplusplus >= 201703L
#define MAYBE_UNUSED [[maybe_unused]]
#elif defined(__clang__)
#define MAYBE_UNUSED [[maybe_unused]]
#elif defined(_MSC_VER)
#define MAYBE_UNUSED 
#elif defined(__has_cpp_attribute)
#if __has_cpp_attribute(gnu::unused)
#define MAYBE_UNUSED [[gnu::unused]]
#else
#define MAYBE_UNUSED
#endif
#else
#define MAYBE_UNUSED
#endif

#ifndef FINAL
#define FINAL final
#endif


#ifdef _MSC_VER
#define BUILTIN_UNREACHABLE \
  __assume(false);
#elif defined(__GNUC__) && !defined(__clang__)










#define BUILTIN_UNREACHABLE \
  __builtin_unreachable();
#else
#define BUILTIN_UNREACHABLE
#endif
#endif

