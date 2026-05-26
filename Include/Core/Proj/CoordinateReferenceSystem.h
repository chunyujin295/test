
#ifndef PROJ_COORDINATEREFERENCESYSTEM_H
#define PROJ_COORDINATEREFERENCESYSTEM_H


#include "Core/Proj/ProjCore.h"
#include <ostream>
#include "Core/Proj/SqliteUtils.h"

#include <QString>
#include <QMap>
#include <QHash>
#include <QReadWriteLock>
#include <QExplicitlySharedDataPointer>
#include <QObject>


#include <Eigen/Core>
#include "Core/Types.h"
#include "Constants.h"


#ifndef SIP_RUN

typedef struct PJconsts PJ;

#if PROJ_VERSION_MAJOR>=8
struct pj_ctx;
typedef struct pj_ctx PJ_CONTEXT;
#else

typedef struct projCtx_t PJ_CONTEXT;
#endif
#endif



typedef struct sqlite3 sqlite3;

#ifdef DEBUG
typedef struct OGRSpatialReferenceHS *OGRSpatialReferenceH SIP_SKIP;
#else
typedef void *OGRSpatialReferenceH SIP_SKIP;
#endif
namespace AI3D
{
    namespace PROJ
    {
        class CoordinateReferenceSystem;
        typedef void (*CUSTOM_CRS_VALIDATION)(CoordinateReferenceSystem&) SIP_SKIP;



        class CoordinateReferenceSystemPrivate;

        class ProjectionFactors;
        class ProjOperation;
        class AI3D_API CoordinateReferenceSystem
        {
            Q_GADGET


                Q_PROPERTY(bool isGeographic READ isGeographic)
                Q_PROPERTY(QString authid READ authid)
                Q_PROPERTY(QString description READ description)

        public:

            
            enum CrsType
            {
                InternalCrsId,  
               
               EpsgCrsId       
            };
            enum Format
            {
                FormatWkt = 0, 
                FormatProj, 
                FormatENU,
            };
            
            CoordinateReferenceSystem();

            ~CoordinateReferenceSystem();

            

            
             
            explicit CoordinateReferenceSystem(const std::string& definition);
            explicit CoordinateReferenceSystem(const QString& definition);
            

            
            Q_DECL_DEPRECATED explicit CoordinateReferenceSystem(long id, CrsType type = EpsgCrsId) SIP_DEPRECATED;

            
            CoordinateReferenceSystem(const CoordinateReferenceSystem& srs);

            
            CoordinateReferenceSystem& operator=(const CoordinateReferenceSystem& srs);

            
            operator QVariant() const
            {
                return QVariant::fromValue(*this);
            }

            
             
            static QList< long > validSrsIds();
            void TestValidSrsIds();
            

            
            static CoordinateReferenceSystem fromOgcWmsCrs(const QString& ogcCrs);
            
            void validateCurrent(const QString projDef);
            
            
            Q_INVOKABLE static CoordinateReferenceSystem fromEpsgId(long epsg);

            
            
            Q_DECL_DEPRECATED static CoordinateReferenceSystem fromProj4(const QString& proj4) SIP_DEPRECATED;

            
            static CoordinateReferenceSystem fromProj(const QString& proj);

            
            static CoordinateReferenceSystem fromWkt(const QString& wkt);

            
            
            static CoordinateReferenceSystem fromSrsId(long srsId);

            

            

            
            Q_DECL_DEPRECATED bool createFromId(long id, CrsType type = EpsgCrsId) SIP_DEPRECATED;

            

            
            bool createFromOgcWmsCrs(const QString& crs);

            




            
            bool createFromWkt(const QString& wkt);

            
            bool createFromSrsId(long srsId);

            
            Q_DECL_DEPRECATED bool createFromProj4(const QString& projString) SIP_DEPRECATED;

            
#ifndef SIP_RUN
            bool createFromProj(const QString& projString, bool identify = true);
#else
            bool createFromProj(const QString& projString);
#endif

            
            bool createFromString(const QString& definition);
            static CoordinateReferenceSystem fromENUDefinition(const QString& definition);
            bool CreateFromLocalDefinition(const QString& definition);
            bool CreateFromENUDefinition(const QString& definition);
            bool CreateFromSpecialEpsg(const QString& definition);
            
            Q_INVOKABLE static  CoordinateReferenceSystem fromSpecialEpsg(const QString& definition);
            
          
            

            
            
            bool createFromUserInput(const QString& definition);

            
            Q_DECL_DEPRECATED static void setupESRIWktFix() SIP_DEPRECATED;

            
            bool isValid() const;

            
            void validate();

            

            
            
            Q_DECL_DEPRECATED long findMatchingProj() SIP_DEPRECATED;

            
            bool operator==(const CoordinateReferenceSystem& srs) const;

            
            bool operator!=(const CoordinateReferenceSystem& srs) const;



            
            static void setCustomCrsValidation(CUSTOM_CRS_VALIDATION f) SIP_SKIP;

            
            static CUSTOM_CRS_VALIDATION customCrsValidation() SIP_SKIP;

            

            
            long srsid() const;



            
             
            QString authid() const;
            std::string GetAuthID() const;
            
             
            QString description() const;
            std::string GetDescription() const;
            
            
             
            QString userFriendlyIdentifier(ProjCore::CrsIdentifierType type = ProjCore::CrsIdentifierType::MediumString) const;

            
            QString projectionAcronym() const;

            
            QString ellipsoidAcronym() const;

            
             
            QString toWkt(ProjCore::CrsWktVariant variant = ProjCore::CrsWktVariant::Wkt1Gdal, bool multiline = false, int indentationWidth = 4) const;

            
            Q_DECL_DEPRECATED QString toProj4() const SIP_DEPRECATED;

            
            QString toProj() const;

            
            ProjCore::CrsType type() const;
            coord_system_type_e GetType() const;
            
            bool isDeprecated() const;

            
            bool isGeographic() const;

            
            bool isDynamic() const;



            
             
            void setCoordinateEpoch(double epoch);

            
            double coordinateEpoch() const;

            
            ProjectionFactors factors(const Eigen::Vector2d& point) const;

            
            ProjOperation operation() const;

            
            bool hasAxisInverted() const;

            
#ifndef SIP_RUN
            QList< ProjCore::CrsAxisDirection > axisOrdering() const;
#else
            SIP_PYOBJECT axisOrdering() const SIP_TYPEHINT(List[Qgis.CrsAxisDirection]);
            % MethodCode
                

                const QList< Qgis::CrsAxisDirection > cppRes = sipCpp->axisOrdering();

            PyObject* l = PyList_New(cppRes.size());

            if (!l)
                sipIsErr = 1;
            else
            {
                for (int i = 0; i < cppRes.size(); ++i)
                {
                    PyObject* eobj = sipConvertFromEnum(static_cast<int>(cppRes.at(i)),
                        sipType_Qgis_CrsAxisDirection);

                    if (!eobj)
                    {
                        sipIsErr = 1;
                    }

                    PyList_SetItem(l, i, eobj);
                }

                if (!sipIsErr)
                {
                    sipRes = l;
                }
                else
                {
                    Py_DECREF(l);
                }
            }
            % End
#endif

                
                ProjCore::DistanceUnit mapUnits() const;

            
             
             

             
            QString toOgcUri() const;

            

            
             
            void updateDefinition();

            
            void setValidationHint(const QString& html);
            
            
            QString validationHint() const;
           static bool IsExists(std::string dbfile, std::string definition);
            bool ADDUserENUSRS(QString dbFile, std::string auth_id);
            static long GetUSERCrsID(QString db);
            
            static int syncDatabase();
            static int AddToUserDatabase(const CoordinateReferenceSystem& crs);
            
            static srs_s AddCrs(std::string definition);
            
            long SaveAsUsersCrs(const std::string& name, ProjCore::CrsDefinitionFormat nativeFormat = ProjCore::CrsDefinitionFormat::Proj);

            long saveAsUserCrs(const QString& name, ProjCore::CrsDefinitionFormat nativeFormat = ProjCore::CrsDefinitionFormat::Wkt);

            
             
            void setNativeFormat(ProjCore::CrsDefinitionFormat format);

            
            ProjCore::CrsDefinitionFormat nativeFormat() const;

            
            CoordinateReferenceSystem toGeographicCrs() const;

            
            QString geographicCrsAuthId() const;



#ifndef SIP_RUN

            
            PJ* projObject() const;

            
            static CoordinateReferenceSystem fromProjObject(PJ* object);

            
            bool createFromProjObject(PJ* object);
#endif

            
            Q_DECL_DEPRECATED static QStringList recentProjections() SIP_DEPRECATED;

            
            Q_DECL_DEPRECATED static QList< CoordinateReferenceSystem > recentCoordinateReferenceSystems() SIP_DEPRECATED;

            
            Q_DECL_DEPRECATED static void pushRecentCoordinateReferenceSystem(const CoordinateReferenceSystem& crs) SIP_DEPRECATED;
            static void InsertRecentCoordinateReferenceSystem(const CoordinateReferenceSystem& crs);
            
            Q_DECL_DEPRECATED static void removeRecentCoordinateReferenceSystem(const CoordinateReferenceSystem& crs) SIP_DEPRECATED;

            
            Q_DECL_DEPRECATED static void clearRecentCoordinateReferenceSystems() SIP_DEPRECATED;

#ifndef SIP_RUN

            
            static void invalidateCache(bool disableCache = false);
#else

            
            static void invalidateCache(bool disableCache SIP_PYARGREMOVE = false);
#endif

            
            
            
          

            
            static QString projFromSrsId(int srsId);

            
            void setProjString(const QString& projString);

            
            bool setWktString(const QString& wkt);

            
            void debugPrint();

            
            typedef QMap<QString, QString> RecordMap;

            
            RecordMap getRecord(const QString& sql);

            
            static int openDatabase(const QString& path, sqlite3_database_unique_ptr& database, bool readonly = true);

            
            void setMapUnits();

            
            static long getRecordCount();

            bool loadFromAuthCode(const QString& auth, const QString& code);

            
            static QList< long > userSrsIds();

            
            long matchToUserCrs() const;

            
            bool loadFromDatabase(const QString& db, const QString& expression, const QString& value);

            bool createFromWktInternal(const QString& wkt, const QString& description);

            QExplicitlySharedDataPointer<CoordinateReferenceSystemPrivate> d;

            QString mValidationHint;

            ProjCore::CrsDefinitionFormat mNativeFormat = ProjCore::CrsDefinitionFormat::Wkt;

            friend class QgsProjContext;

            
            static void removeFromCacheObjectsBelongingToCurrentThread(PJ_CONTEXT* pj_context);

            
            static CUSTOM_CRS_VALIDATION sCustomSrsValidation;

            

            static bool sDisableOgcCache;
            static bool sDisableProjCache;
            static bool sDisableWktCache;
            static bool sDisableSrsIdCache;
            static bool sDisableStringCache;

            
            static const QHash< QString, CoordinateReferenceSystem >& stringCache();
            static const QHash< QString, CoordinateReferenceSystem >& projCache();
            static const QHash< QString, CoordinateReferenceSystem >& ogcCache();
            static const QHash< QString, CoordinateReferenceSystem >& wktCache();
            static const QHash< long, CoordinateReferenceSystem >& srsIdCache();



            friend class QgsCoordinateReferenceSystemRegistry;
            friend bool AI3D_API operator> (const CoordinateReferenceSystem& c1, const CoordinateReferenceSystem& c2);
            friend bool AI3D_API operator< (const CoordinateReferenceSystem& c1, const CoordinateReferenceSystem& c2);
            friend bool AI3D_API operator>= (const CoordinateReferenceSystem& c1, const CoordinateReferenceSystem& c2);
            friend bool AI3D_API operator<= (const CoordinateReferenceSystem& c1, const CoordinateReferenceSystem& c2);

        };

        Q_DECLARE_METATYPE(CoordinateReferenceSystem)
    }
}

#ifndef SIP_RUN
inline std::ostream &operator << ( std::ostream &os, const AI3D::PROJ::CoordinateReferenceSystem &r )
{
  QString mySummary( QStringLiteral( "\n\tSpatial Reference System:" ) );
  mySummary += QLatin1String( "\n\t\tDescription : " );
  if ( !r.description().isNull() )
  {
    mySummary += r.description();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }
  mySummary += QLatin1String( "\n\t\tProjection  : " );
  if ( !r.projectionAcronym().isNull() )
  {
    mySummary += r.projectionAcronym();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }

  mySummary += QLatin1String( "\n\t\tEllipsoid   : " );
  if ( !r.ellipsoidAcronym().isNull() )
  {
    mySummary += r.ellipsoidAcronym();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }

  mySummary += QLatin1String( "\n\t\tProjString  : " );
  if ( !r.toProj().isNull() )
  {
    mySummary += r.toProj();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }
  
  return os << mySummary.toUtf8().data() << std::endl;
}

bool AI3D_API operator> ( const AI3D::PROJ::CoordinateReferenceSystem &c1, const AI3D::PROJ::CoordinateReferenceSystem &c2 );
bool AI3D_API operator< ( const AI3D::PROJ::CoordinateReferenceSystem &c1, const AI3D::PROJ::CoordinateReferenceSystem &c2 );
bool AI3D_API operator>= ( const AI3D::PROJ::CoordinateReferenceSystem &c1, const AI3D::PROJ::CoordinateReferenceSystem &c2 );
bool AI3D_API operator<= ( const AI3D::PROJ::CoordinateReferenceSystem &c1, const AI3D::PROJ::CoordinateReferenceSystem &c2 );
#endif


#endif 
