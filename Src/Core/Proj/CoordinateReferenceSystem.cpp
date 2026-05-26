


#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/Proj/CoordinateReferenceSystem_p.h"
#include "Core/Proj/CoordinateReferenceSystemUtils.h"
#include "Core/Proj/AuthIdToSrsIdMap.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"
#include "Core/Proj/ReadWriteLocker.h"
#include <iostream>
#include <cmath>
#include "Core/Application.h"
#include <QDir>
#include <QDomNode>
#include <QDomElement>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QFile>
#include "Core/String.h"
#include "Core/Proj/QProj.h"

#include "Core/Proj/ProjCore.h" 
#include "Core/Proj/Localec.h"
#include "Core/Proj/OgcUtils.h"
#include "Core/Proj/ProjectionFactors.h"
#include "Core/Proj/ProjOperation.h"


#include <sqlite3.h>

#include <proj.h>
#include <proj_experimental.h>


#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include <cpl_csv.h>
#include <locale.h>
namespace AI3D
{
    namespace PROJ
    {
        CUSTOM_CRS_VALIDATION CoordinateReferenceSystem::sCustomSrsValidation = nullptr;

        typedef QHash< QString, CoordinateReferenceSystem > StringCrsCacheHash;
        typedef QHash< long, CoordinateReferenceSystem > SrIdCrsCacheHash;


        Q_GLOBAL_STATIC(QReadWriteLock, sOgcLock)
            Q_GLOBAL_STATIC(StringCrsCacheHash, sOgcCache)
            bool CoordinateReferenceSystem::sDisableOgcCache = false;

        Q_GLOBAL_STATIC(QReadWriteLock, sProj4CacheLock)
            Q_GLOBAL_STATIC(StringCrsCacheHash, sProj4Cache)
            bool CoordinateReferenceSystem::sDisableProjCache = false;

        Q_GLOBAL_STATIC(QReadWriteLock, sCRSWktLock)
            Q_GLOBAL_STATIC(StringCrsCacheHash, sWktCache)
            bool CoordinateReferenceSystem::sDisableWktCache = false;

        Q_GLOBAL_STATIC(QReadWriteLock, sCRSSrsIdLock)
            Q_GLOBAL_STATIC(SrIdCrsCacheHash, sSrsIdCache)
            bool CoordinateReferenceSystem::sDisableSrsIdCache = false;

        Q_GLOBAL_STATIC(QReadWriteLock, sCrsStringLock)
            Q_GLOBAL_STATIC(StringCrsCacheHash, sStringCache)
            bool CoordinateReferenceSystem::sDisableStringCache = false;

        QString getFullProjString(PJ* obj)
        {
            
            
            ProjUtils::proj_pj_unique_ptr boundCrs(proj_crs_create_bound_crs_to_WGS84(ProjContext::get(), obj, nullptr));
            if (boundCrs)
            {
                if (const char* proj4src = proj_as_proj_string(ProjContext::get(), boundCrs.get(), PJ_PROJ_4, nullptr))
                {
                    return QString(proj4src);
                }
            }

            return QString(proj_as_proj_string(ProjContext::get(), obj, PJ_PROJ_4, nullptr));
        }
        

        CoordinateReferenceSystem::CoordinateReferenceSystem()
        {
            static CoordinateReferenceSystem nullCrs = CoordinateReferenceSystem(QString().toStdString());

            d = nullCrs.d;
        }

        bool CoordinateReferenceSystem::IsExists(std::string dbfile, std::string definition)
        {
            QString authName = QString::fromStdString(definition);

            
            QString checksql = QStringLiteral("select COUNT(*) from tbl_srs where auth_id = '%1' ").arg(authName);
            int rc;
            sqlite3_database_unique_ptr database1;
            auto db = QString::fromStdString(dbfile);
            int result1 = openDatabase(db, database1);
            if (result1 != SQLITE_OK)
            {
                
                return false;
            }

            long          myRecordCount = 0;
            auto statement1 = database1.prepare(checksql, rc);
            if (rc == SQLITE_OK)
            {
                if (statement1.step() == SQLITE_ROW)
                {
                    QString myRecordCountString = statement1.columnAsText(0);

                    myRecordCount = myRecordCountString.toLong();
                    if (myRecordCount > 0)
                    {
                       
                        return true;
                    }

                }
            }

            return false;
        }


        
            

        
        srs_s CoordinateReferenceSystem::AddCrs(std::string definition)
        {
            AI3D::PROJ::CoordinateReferenceSystem crs(definition);
            
           
            
            
            
            

            
            
            AI3D::PROJ::CoordinateReferenceSystem::InsertRecentCoordinateReferenceSystem(crs);
            srs_s srs;
            srs.definition = crs.authid().toStdString();
            srs.type = crs.GetType();
            srs.name = crs.GetDescription();
            return srs;
        }

        CoordinateReferenceSystem::CoordinateReferenceSystem(const std::string& definition)
        {
            d = new CoordinateReferenceSystemPrivate();
            QString def = QString::fromStdString(definition);
            createFromString(def);
        }
        CoordinateReferenceSystem::CoordinateReferenceSystem(const QString& definition)
        {
            d = new CoordinateReferenceSystemPrivate();
         
            createFromString(definition);
        }
        CoordinateReferenceSystem::CoordinateReferenceSystem(const long id, CrsType type)
        {
            d = new CoordinateReferenceSystemPrivate();
            Q_NOWARN_DEPRECATED_PUSH
                createFromId(id, type);
            Q_NOWARN_DEPRECATED_POP
        }

        CoordinateReferenceSystem::CoordinateReferenceSystem(const CoordinateReferenceSystem& srs)  
            : d(srs.d)
            , mValidationHint(srs.mValidationHint)
            , mNativeFormat(srs.mNativeFormat)
        {
        }

        CoordinateReferenceSystem& CoordinateReferenceSystem::operator=(const CoordinateReferenceSystem& srs)  
        {
            d = srs.d;
            mValidationHint = srs.mValidationHint;
            mNativeFormat = srs.mNativeFormat;
            return *this;
        }

        QList<long> CoordinateReferenceSystem::validSrsIds()
        {
            QList<long> results;
            
            QStringList dbs = QStringList() << QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath()) << QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath());

            const auto constDbs = dbs;
            for (const QString& db : constDbs)
            {
                QFileInfo myInfo(db);
                if (!myInfo.exists())
                {
                    
                    continue;
                }

                sqlite3_database_unique_ptr database;
                sqlite3_statement_unique_ptr statement;

                
                int result = openDatabase(db, database);
                if (result != SQLITE_OK)
                {
                    
                    continue;
                }

                QString sql = QStringLiteral("select srs_id from tbl_srs");
                int rc;
                statement = database.prepare(sql, rc);
                while (true)
                {
                    
                    int ret = statement.step();

                    if (ret == SQLITE_DONE)
                    {
                        
                        break;
                    }

                    if (ret == SQLITE_ROW)
                    {
                        results.append(statement.columnAsInt64(0));
                    }
                    else
                    {
                        
                        break;
                    }
                }
            }
            std::sort(results.begin(), results.end());
            return results;
        }
        bool CoordinateReferenceSystem::CreateFromSpecialEpsg(const QString& definition)
        {


            ReadWriteLocker locker(*sCRSSrsIdLock(), ReadWriteLocker::Read);
            
            {


                if (!sDisableStringCache)
                {
                    QHash< QString, CoordinateReferenceSystem >::const_iterator crsIt = sStringCache()->constFind(definition);
                    if (crsIt != sStringCache()->constEnd())
                    {
                        
                        *this = crsIt.value();
                        return true;
                    }
                }
                locker.unlock();
            }
            
            PJ_CONTEXT* pjContext = nullptr;;
            pjContext = proj_context_create();
            ProjUtils::proj_pj_unique_ptr crs(proj_create(pjContext, definition.toLatin1().constData()));
            
            if (crs)
            {
                d->setPj(std::move(crs));
                d->mIsValid = true;
                QString proj4String = d->mProj4;
                if (proj4String.isEmpty())
                {
                    proj4String = toProj();
                }
               
                bool result = loadFromDatabase(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()),
                    QStringLiteral("auth_id"), definition);
                
                if (!result|| d->mSrsId < USER_CRS_START_ID)
                {
                    d->mIsValid = false;
                    return false;
                }
            }
            else
            {
                d->mIsValid = false;
            }
            
            


          

            locker.changeMode(ReadWriteLocker::Write);
            if (!sDisableStringCache)
                sStringCache()->insert(definition, *this);



            return true;
        }


        long CoordinateReferenceSystem::GetUSERCrsID(QString db)
        {
            sqlite3_statement_unique_ptr statement;
            int rc;
            sqlite3_database_unique_ptr database1;

            int result1 = openDatabase(db, database1);
            if (result1 != SQLITE_OK)
            {
                
                return -1;
            }
            QString sql = QStringLiteral("select srs_id from tbl_srs where srs_id >= %1").arg(USER_CRS_START_ID);
            statement = database1.prepare(sql, rc);
            std::set<long> srsids;
            while (true)
            {
                int ret = statement.step();

                if (ret == SQLITE_DONE)
                {
                    
                    break;
                }

                if (ret == SQLITE_ROW)
                {

                    long id = statement.columnAsInt64(0);
                    srsids.insert(id);
                }
                else
                {
                   
                    
                    break;
                }
            }


            if (srsids.empty())
                return USER_CRS_START_ID;
            return *srsids.rbegin() + 1;
        }

        bool CoordinateReferenceSystem::ADDUserENUSRS(QString dbFile, std::string auth_id)
        {


            if (IsExists(dbFile.toStdString(), auth_id))
                return true;
            long id = GetUSERCrsID(dbFile);
            if (id < 0)
            {
                return false;
            }
            int nextSrsId = id;
            int nextSrId = 520000000 + nextSrsId - 60000;

            std::string srsid = "";
            std::string srId = "";
            if (srId.empty())
            {
                srId = std::to_string(nextSrId);

            }
            if (srsid.empty())
            {
                srsid = std::to_string(nextSrsId);

            }
            
            if (!srsid.empty())
            {

                std::string path = dbFile.toStdString();
                sqlite3* database = nullptr;
                int result = sqlite3_open(path.c_str(), &database);

                if (result != SQLITE_OK)
                {
                    std::cout << "Could not open .db" << std::endl;
                    LOGI("Could not open " + path +"  .db.");
                    return false;
                }



                std::string name = auth_id;
                std::string description = name;
                std::string projection_acronym = "";
                std::string ellipsoid_acronym = "WGS84";
                std::string parameters = auth_id;

                std::string auth_name = "ENU";
               
                std::string str = "INSERT INTO tbl_srs(srs_id, description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) VALUES (";
                str += srsid + ",";
                str += "'" + description + "',";
                str += "'" + projection_acronym + "',";
                str += "'" + ellipsoid_acronym + "',";
                str += "'" + parameters + "',";
                str += srId + ",";
                str += "'" + auth_name + "',";
                str += "'" + auth_id + "',0,0)";


                
                std::string  sql = str;
                char* errMsg = nullptr;
                int inserted = 0;

                if (sqlite3_exec(database, sql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK)
                {
                    inserted++;
                }
                else
                {
                    if (errMsg)
                        sqlite3_free(errMsg);
                    return  false;
                }

            }

            return true;
        }


        bool CoordinateReferenceSystem::CreateFromENUDefinition(const QString& definition)
        {

            

            ReadWriteLocker locker(*sCRSSrsIdLock(), ReadWriteLocker::Read);
           
            {
                sDisableStringCache = true; 

                if (!sDisableStringCache)
                {
                    QHash< QString, CoordinateReferenceSystem >::const_iterator crsIt = sStringCache()->constFind(definition);
                    if (crsIt != sStringCache()->constEnd())
                    {
                        
                        *this = crsIt.value();

                        return true;
                    }
                }
                locker.unlock();
            }
            std::string userdatabasefile = AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath();
            std::string databasefile = AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath();
            {
                
               
                if(0)
                {
                    ADDUserENUSRS(QString::fromStdString(userdatabasefile), definition.QString::toStdString());
                }
            }

            bool result = loadFromDatabase(QString::fromStdString(userdatabasefile),
                QStringLiteral("auth_id"), definition);
            
            if (!result  )
            {
                QString str = definition.toUpper();
                
                

                d.detach();
                d->mIsValid = false;
                d->mWktPreferred.clear();

                if (str.startsWith("ENU:"))
                {


                    


                d->mIsValid = true;
                d->mProj4 = "";
                d->mWktPreferred.clear();
              
                Eigen::Vector2d LatLon = AI3D::CORE::CoordinateDescriptor::GetLatLonFromENUDefinition(definition.QString::toStdString());
                std::string description = "Local East-North-Up (ENU); origin: " + std::to_string(LatLon(0)) + "N " + std::to_string(LatLon(1)) + "E";
                d->mDescription = QString::fromStdString(description);
                d->mAuthId = str;
                d->mIsGeographic = false;
                d->mAxisInvertedDirty = false;
                QString operation;
                QString ellipsoid;
                
                d->mProjectionAcronym = operation;
                d->mEllipsoidAcronym.clear();
                

              

                d->mMapUnits = ProjCore::DistanceUnit::Meters;
                }

                
               
            }
                  
           

            

            locker.changeMode(ReadWriteLocker::Write);
            if (!sDisableStringCache)
                sStringCache()->insert(definition, *this);

            return true;
        }

        CoordinateReferenceSystem CoordinateReferenceSystem::fromOgcWmsCrs(const QString& ogcCrs)
        {
            CoordinateReferenceSystem crs;
            crs.createFromOgcWmsCrs(ogcCrs);
            return crs;
        }
        CoordinateReferenceSystem CoordinateReferenceSystem::fromENUDefinition(const QString& definition)
        {
            CoordinateReferenceSystem crs;
            crs.CreateFromENUDefinition(definition);
            return crs;
        }
        CoordinateReferenceSystem CoordinateReferenceSystem::fromSpecialEpsg(const QString& definition)
        {
            CoordinateReferenceSystem crs;
            crs.CreateFromSpecialEpsg(definition);
            return crs;
        }

        CoordinateReferenceSystem CoordinateReferenceSystem::fromEpsgId(long epsg)
        {
            CoordinateReferenceSystem res = fromOgcWmsCrs("EPSG:" + QString::number(epsg));
            if (res.isValid())
                return res;

            
            res = fromOgcWmsCrs("ESRI:" + QString::number(epsg));
            if (res.isValid())
                return res;

            return CoordinateReferenceSystem();
        }

        CoordinateReferenceSystem CoordinateReferenceSystem::fromProj4(const QString& proj4)
        {
            return fromProj(proj4);
        }

        CoordinateReferenceSystem CoordinateReferenceSystem::fromProj(const QString& proj)
        {
            CoordinateReferenceSystem crs;
            crs.createFromProj(proj);
            return crs;
        }

        CoordinateReferenceSystem CoordinateReferenceSystem::fromWkt(const QString& wkt)
        {
            CoordinateReferenceSystem crs;
            crs.createFromWkt(wkt);
            return crs;
        }

        CoordinateReferenceSystem CoordinateReferenceSystem::fromSrsId(long srsId)
        {
            CoordinateReferenceSystem crs;
            crs.createFromSrsId(srsId);
            return crs;
        }

        CoordinateReferenceSystem::~CoordinateReferenceSystem() 
        {
        }

        bool CoordinateReferenceSystem::createFromId(const long id, CrsType type)
        {
            bool result = false;
            switch (type)
            {
            case InternalCrsId:
                result = createFromSrsId(id);
                break;

            case EpsgCrsId:
                result = createFromOgcWmsCrs(QStringLiteral("EPSG:%1").arg(id));
                break;
            default:
                
                LOGE(("Unexpected case reached!"));
            };
            return result;
        }

        bool CoordinateReferenceSystem::CreateFromLocalDefinition(const QString& definition)
        {
            d->mIsValid = true;
            d->mProj4 = "";
            d->mWktPreferred.clear();
            d->mDescription = "Local coordinate system";
            d->mAuthId = definition;
            d->mIsGeographic = false;
            d->mAxisInvertedDirty = false;
            QString operation;
            QString ellipsoid;
            
            d->mProjectionAcronym = operation;
            d->mEllipsoidAcronym.clear();
  
            d->mMapUnits = ProjCore::DistanceUnit::Meters;

            return true;
        }

        bool CoordinateReferenceSystem::createFromString(const QString& definition)
        {
            if (definition.isEmpty())
                return false;

            ReadWriteLocker locker(*sCrsStringLock(), ReadWriteLocker::Read);
            if (!sDisableStringCache)
            {
              
                QHash< QString, CoordinateReferenceSystem >::const_iterator crsIt = sStringCache()->constFind(definition);
                if (crsIt != sStringCache()->constEnd())
                {
                    
                    *this = crsIt.value();
                    return d->mIsValid;
                }
            }
            locker.unlock();

            bool result = false;

            
            
            
            
            
            
            
            
            
            
            

            const thread_local QRegularExpression reCrsId(QStringLiteral("^(epsg|esri|osgeo|ignf|ogc|nkg|zangi|iau_2015|iau2000|postgis|internal|user)\\:(\\w+)$"), QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = reCrsId.match(definition);
            if (match.capturedStart() == 0)
            {
                QString authName = match.captured(1).toLower();
                if (authName == QLatin1String("epsg"))
                {
                    result = createFromOgcWmsCrs(definition);
                }
               
                else if (authName == QLatin1String("esri")
                    || authName == QLatin1String("osgeo")
                    || authName == QLatin1String("ignf")
                    || authName == QLatin1String("zangi")
                    || authName == QLatin1String("iau2000")
                    || authName == QLatin1String("ogc")
                    || authName == QLatin1String("nkg")
                    || authName == QLatin1String("iau_2015")
                    )
                {
                    result = createFromOgcWmsCrs(definition);
                }
                else
                {
                    const long id = match.captured(2).toLong();
                    Q_NOWARN_DEPRECATED_PUSH
                        result = createFromId(id, InternalCrsId);
                    Q_NOWARN_DEPRECATED_POP
                }
            }
            else
            {
                const thread_local QRegularExpression reCrsStr(QStringLiteral("^(?:(wkt|proj4|proj)\\:)?(.+)$"), QRegularExpression::CaseInsensitiveOption);
                match = reCrsStr.match(definition);
                if (match.capturedStart() == 0)
                {
                    if (match.captured(1).startsWith(QLatin1String("proj"), Qt::CaseInsensitive))
                    {
                        result = createFromProj(match.captured(2));
                    }

                    else
                    {
                        if (match.captured(2).startsWith(QLatin1String("epsg"), Qt::CaseInsensitive))
                        {
                            result = CreateFromSpecialEpsg(match.captured(2));
                        }
                        else if (definition.contains("Local East-North-Up (ENU)", Qt::CaseInsensitive) || match.captured(2).startsWith(QLatin1String("enu"), Qt::CaseInsensitive))
                        {
                            result = CreateFromENUDefinition(match.captured(2));
                        }
                        else if (definition.contains("Local East-North-Up (ENU)", Qt::CaseInsensitive))
                        {
                            
                            result = CreateFromENUDefinition(match.captured(2));
                        }

                        else  if (match.captured(2).startsWith(QLatin1String("local"), Qt::CaseInsensitive))
                            result = CreateFromLocalDefinition(match.captured(2));
                        else
                            result = createFromWkt(match.captured(2));
                    }

                }
            }

            locker.changeMode(ReadWriteLocker::Write);
            if (!sDisableStringCache)
                sStringCache()->insert(definition, *this);
            return result;
        }
        QString OGRSpatialReferenceToWkt(OGRSpatialReferenceH srs)
        {
            if (!srs)
                return QString();

            char* pszWkt = nullptr;
            const QByteArray multiLineOption = QStringLiteral("MULTILINE=NO").toUtf8();
            const QByteArray formatOption = QStringLiteral("FORMAT=WKT2").toUtf8();
            const char* const options[] = { multiLineOption.constData(), formatOption.constData(), nullptr };
            OSRExportToWktEx(srs, &pszWkt, options);

            const QString res(pszWkt);
            CPLFree(pszWkt);
            return res;
        }
        bool CoordinateReferenceSystem::createFromUserInput(const QString& definition)
        {
            if (definition.isEmpty())
                return false;

            QString userWkt;
            OGRSpatialReferenceH crs = OSRNewSpatialReference(nullptr);

            if (OSRSetFromUserInput(crs, definition.toUtf8().constData()) == OGRERR_NONE)
            {
                userWkt = OGRSpatialReferenceToWkt(crs);
                OSRDestroySpatialReference(crs);
            }
            
            return createFromWkt(userWkt);
        }

        void CoordinateReferenceSystem::setupESRIWktFix()
        {
            
            
            const char* configOld = CPLGetConfigOption("GDAL_FIX_ESRI_WKT", "");
            const char* configNew = "GEOGCS";
            
            if (strcmp(configOld, "") == 0)
            {
                CPLSetConfigOption("GDAL_FIX_ESRI_WKT", configNew);
                if (strcmp(configNew, CPLGetConfigOption("GDAL_FIX_ESRI_WKT", "")) != 0)
                {
                    
                }
                
            }
            else
            {
                
            }
        }

        bool CoordinateReferenceSystem::createFromOgcWmsCrs(const QString& crs)
        {
            if (crs.isEmpty())
                return false;

            ReadWriteLocker locker(*sOgcLock(), ReadWriteLocker::Read);
            if (!sDisableOgcCache)
            {
                QHash< QString, CoordinateReferenceSystem >::const_iterator crsIt = sOgcCache()->constFind(crs);
                if (crsIt != sOgcCache()->constEnd())
                {
                    
                    *this = crsIt.value();
                    return d->mIsValid;
                }
            }
            locker.unlock();

            QString wmsCrs = crs;

            QString authority;
            QString code;
            const OgcCrsUtils::CRSFlavor crsFlavor = OgcCrsUtils::parseCrsName(crs, authority, code);
            const QString authorityLower = authority.toLower();
            if (crsFlavor == OgcCrsUtils::CRSFlavor::AUTH_CODE &&
                (authorityLower == QLatin1String("user") ||
                    authorityLower == QLatin1String("custom") ||
                    authorityLower == QLatin1String("qgis") 

                    ))
            {
                if (createFromSrsId(code.toInt()))
                {
                    locker.changeMode(ReadWriteLocker::Write);
                    if (!sDisableOgcCache)
                        sOgcCache()->insert(crs, *this);
                    return d->mIsValid;
                }
            }
            else if (crsFlavor != OgcCrsUtils::CRSFlavor::UNKNOWN)
            {
                wmsCrs = authority + ':' + code;
            }

            
            const QString legacyKey = wmsCrs.toLower();
            for (auto it = sAuthIdToSrsIdMap.constBegin(); it != sAuthIdToSrsIdMap.constEnd(); ++it)
            {
                if (it.key().compare(legacyKey, Qt::CaseInsensitive) == 0)
                {
                    const QStringList parts = it.key().split(':');
                    const QString auth = parts.at(0);
                    const QString code = parts.at(1);
                    if (loadFromAuthCode(auth, code))
                    {
                        locker.changeMode(ReadWriteLocker::Write);
                        if (!sDisableOgcCache)
                            sOgcCache()->insert(crs, *this);
                        return d->mIsValid;
                    }
                }
            }

            if (loadFromDatabase(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath()), QStringLiteral("lower(auth_name||':'||auth_id)"), wmsCrs.toLower()))
            {
                locker.changeMode(ReadWriteLocker::Write);
                if (!sDisableOgcCache)
                    sOgcCache()->insert(crs, *this);
                return d->mIsValid;
            }

            
            if (wmsCrs.compare(QLatin1String("CRS:27"), Qt::CaseInsensitive) == 0 ||
                wmsCrs.compare(QLatin1String("OGC:CRS27"), Qt::CaseInsensitive) == 0)
            {
                
                return createFromOgcWmsCrs(QStringLiteral("EPSG:4267"));
            }

            
            if (wmsCrs.compare(QLatin1String("CRS:83"), Qt::CaseInsensitive) == 0 ||
                wmsCrs.compare(QLatin1String("OGC:CRS83"), Qt::CaseInsensitive) == 0)
            {
                
                return createFromOgcWmsCrs(QStringLiteral("EPSG:4269"));
            }

            
            if (wmsCrs.compare(QLatin1String("CRS:84"), Qt::CaseInsensitive) == 0 ||
                wmsCrs.compare(QLatin1String("OGC:CRS84"), Qt::CaseInsensitive) == 0)
            {
                if (loadFromDatabase(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath()), QStringLiteral("lower(auth_name||':'||auth_id)"), QStringLiteral("epsg:4326")))
                {
                    d->mAxisInverted = false;
                    d->mAxisInvertedDirty = false;
                }

                locker.changeMode(ReadWriteLocker::Write);
                if (!sDisableOgcCache)
                    sOgcCache()->insert(crs, *this);

                return d->mIsValid;
            }

            
            
            if (!authority.isEmpty() && !code.isEmpty() && loadFromAuthCode(authority, code))
            {
                locker.changeMode(ReadWriteLocker::Write);
                if (!sDisableOgcCache)
                    sOgcCache()->insert(crs, *this);
                return d->mIsValid;
            }

            locker.changeMode(ReadWriteLocker::Write);
            if (!sDisableOgcCache)
                sOgcCache()->insert(crs, CoordinateReferenceSystem());
            return d->mIsValid;
        }

        


        void CoordinateReferenceSystem::validate()
        {
            if (d->mIsValid || !sCustomSrsValidation)
                return;

            
            if (sCustomSrsValidation)
                sCustomSrsValidation(*this);
        }



        bool CoordinateReferenceSystem::createFromSrsId(const long id)
        {
            ReadWriteLocker locker(*sCRSSrsIdLock(), ReadWriteLocker::Read);
            if (!sDisableSrsIdCache)
            {
                QHash< long, CoordinateReferenceSystem >::const_iterator crsIt = sSrsIdCache()->constFind(id);
                if (crsIt != sSrsIdCache()->constEnd())
                {
                    
                    *this = crsIt.value();
                    return d->mIsValid;
                }
            }
            locker.unlock();

            
            for (auto it = sAuthIdToSrsIdMap.constBegin(); it != sAuthIdToSrsIdMap.constEnd(); ++it)
            {
                if (it.value().startsWith(QString::number(id) + ','))
                {
                    const QStringList parts = it.key().split(':');
                    const QString auth = parts.at(0);
                    const QString code = parts.at(1);
                    if (loadFromAuthCode(auth, code))
                    {
                        locker.changeMode(ReadWriteLocker::Write);
                        if (!sDisableSrsIdCache)
                            sSrsIdCache()->insert(id, *this);
                        return d->mIsValid;
                    }
                }
            }

            bool result = loadFromDatabase(id < USER_CRS_START_ID ? QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath()) :
                QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()),
                QStringLiteral("srs_id"), QString::number(id));

            locker.changeMode(ReadWriteLocker::Write);
            if (!sDisableSrsIdCache)
                sSrsIdCache()->insert(id, *this);
            return result;
        }

        bool CoordinateReferenceSystem::loadFromDatabase(const QString& db, const QString& expression, const QString& value)
        {
            d.detach();

            
            d->mIsValid = false;
            d->mWktPreferred.clear();

            QFileInfo myInfo(db);
            if (!myInfo.exists())
            {
                
                return d->mIsValid;
            }

            sqlite3_database_unique_ptr database;
            sqlite3_statement_unique_ptr statement;
            int           myResult;
            
            myResult = openDatabase(db, database);
            if (myResult != SQLITE_OK)
            {
                return d->mIsValid;
            }

            

            QString mySql = "select srs_id,description,projection_acronym,"
                "ellipsoid_acronym,parameters,srid,auth_name||':'||auth_id,is_geo,wkt "
                "from tbl_srs where " + expression + '=' + SqliteUtils::quotedString(value) + " order by deprecated";
            statement = database.prepare(mySql, myResult);


            QString wkt;
            
            if (myResult == SQLITE_OK && statement.step() == SQLITE_ROW)
            {
                d->mSrsId = statement.columnAsText(0).toLong();
                d->mDescription = statement.columnAsText(1);
                d->mProjectionAcronym = statement.columnAsText(2);
                d->mEllipsoidAcronym.clear();
                d->mProj4 = statement.columnAsText(4);
                d->mWktPreferred.clear();
                d->mSRID = statement.columnAsText(5).toLong();
                d->mAuthId = statement.columnAsText(6);
                d->mIsGeographic = statement.columnAsText(7).toInt() != 0;
                wkt = statement.columnAsText(8);
                d->mAxisInvertedDirty = true;

                if (d->mSrsId >= USER_CRS_START_ID && (d->mAuthId.isEmpty() || d->mAuthId == QChar(':')))
                {
                    d->mAuthId = QStringLiteral("USER:%1").arg(d->mSrsId);
                }
                else if (!d->mAuthId.startsWith(QLatin1String("USER:"), Qt::CaseInsensitive))
                {
                    QString keystr("ENU:");
                    if (d->mAuthId.startsWith((keystr), Qt::CaseInsensitive))
                    {

                        d->mAuthId.remove(keystr);
                        d->mAuthId = (d->mAuthId.contains(keystr) ? QString() : keystr) + d->mAuthId;
                        
                        d->mIsValid = true;
                    }
                    else
                    {
                        keystr = "EPSG:";
                        if (d->mSrsId >= USER_CRS_START_ID && d->mAuthId.startsWith(keystr, Qt::CaseInsensitive))
                        {
                            d->mAuthId.remove(keystr);
                            d->mAuthId = (d->mAuthId.contains(keystr) ? QString() : keystr) + d->mAuthId;
                            

                        }

                        QStringList parts = d->mAuthId.split(':');
                        QString auth = parts.at(0);
                        QString code = parts.at(1);
                       
                        {
                            ProjUtils::proj_pj_unique_ptr crs(proj_create_from_database(ProjContext::get(), auth.toLatin1(), code.toLatin1(), PJ_CATEGORY_CRS, false, nullptr));
                            if (crs)
                            {
                                d->setPj(ProjUtils::unboundCrs(crs.get()));
                            }
                            else
                            {
                                AI3D::PROJ::ProjUtils::proj_pj_unique_ptr crs(proj_create(ProjContext::get(), d->mAuthId.toLatin1().constData()));
                                d->setPj(std::move(crs));
                            }
                            

 
                        }

                        d->mIsValid = d->hasPj();
                    }
                    setMapUnits();
                }

                if (!d->mIsValid)
                {
                    if (!wkt.isEmpty())
                    {
                        setWktString(wkt);
                        
                        
                        d->mDescription = statement.columnAsText(1);
                    }
                    else
                        setProjString(d->mProj4);
                }
            }
            else
            {
                
            }
            return d->mIsValid;
        }

        void CoordinateReferenceSystem::removeFromCacheObjectsBelongingToCurrentThread(PJ_CONTEXT* pj_context)
        {
            
            
            
            


            if (!sDisableOgcCache)
            {
                ReadWriteLocker locker(*sOgcLock(), ReadWriteLocker::Write);
                if (!sDisableOgcCache)
                {
                    for (auto it = sOgcCache()->begin(); it != sOgcCache()->end(); )
                    {
                        auto& v = it.value();
                        if (v.d->removeObjectsBelongingToCurrentThread(pj_context))
                            it = sOgcCache()->erase(it);
                        else
                            ++it;
                    }
                }
            }
            if (!sDisableProjCache)
            {
                ReadWriteLocker locker(*sProj4CacheLock(), ReadWriteLocker::Write);
                if (!sDisableProjCache)
                {
                    for (auto it = sProj4Cache()->begin(); it != sProj4Cache()->end(); )
                    {
                        auto& v = it.value();
                        if (v.d->removeObjectsBelongingToCurrentThread(pj_context))
                            it = sProj4Cache()->erase(it);
                        else
                            ++it;
                    }
                }
            }
            if (!sDisableWktCache)
            {
                ReadWriteLocker locker(*sCRSWktLock(), ReadWriteLocker::Write);
                if (!sDisableWktCache)
                {
                    for (auto it = sWktCache()->begin(); it != sWktCache()->end(); )
                    {
                        auto& v = it.value();
                        if (v.d->removeObjectsBelongingToCurrentThread(pj_context))
                            it = sWktCache()->erase(it);
                        else
                            ++it;
                    }
                }
            }
            if (!sDisableSrsIdCache)
            {
                ReadWriteLocker locker(*sCRSSrsIdLock(), ReadWriteLocker::Write);
                if (!sDisableSrsIdCache)
                {
                    for (auto it = sSrsIdCache()->begin(); it != sSrsIdCache()->end(); )
                    {
                        auto& v = it.value();
                        if (v.d->removeObjectsBelongingToCurrentThread(pj_context))
                            it = sSrsIdCache()->erase(it);
                        else
                            ++it;
                    }
                }
            }
            if (!sDisableStringCache)
            {
                ReadWriteLocker locker(*sCrsStringLock(), ReadWriteLocker::Write);
                if (!sDisableStringCache)
                {
                    for (auto it = sStringCache()->begin(); it != sStringCache()->end(); )
                    {
                        auto& v = it.value();
                        if (v.d->removeObjectsBelongingToCurrentThread(pj_context))
                            it = sStringCache()->erase(it);
                        else
                            ++it;
                    }
                }
            }
        }

        bool CoordinateReferenceSystem::hasAxisInverted() const
        {
            if (d->mAxisInvertedDirty)
            {
                d->mAxisInverted = ProjUtils::axisOrderIsSwapped(d->threadLocalProjObject());
                d->mAxisInvertedDirty = false;
            }

            return d->mAxisInverted;
        }

        QList<ProjCore::CrsAxisDirection> CoordinateReferenceSystem::axisOrdering() const
        {
            const PJ* projObject = d->threadLocalProjObject();
            if (!projObject)
                return {};

            PJ_CONTEXT* context = ProjContext::get();
            ProjUtils::proj_pj_unique_ptr pjCs(proj_crs_get_coordinate_system(context, projObject));
            if (!pjCs)
                return {};

            const thread_local QMap< ProjCore::CrsAxisDirection, QString > mapping =
            {
              { ProjCore::CrsAxisDirection::North, QStringLiteral("north") },
              { ProjCore::CrsAxisDirection::NorthNorthEast, QStringLiteral("northNorthEast") },
              { ProjCore::CrsAxisDirection::NorthEast, QStringLiteral("northEast") },
              { ProjCore::CrsAxisDirection::EastNorthEast, QStringLiteral("eastNorthEast") },
              { ProjCore::CrsAxisDirection::East, QStringLiteral("east") },
              { ProjCore::CrsAxisDirection::EastSouthEast, QStringLiteral("eastSouthEast") },
              { ProjCore::CrsAxisDirection::SouthEast, QStringLiteral("southEast") },
              { ProjCore::CrsAxisDirection::SouthSouthEast, QStringLiteral("southSouthEast") },
              { ProjCore::CrsAxisDirection::South, QStringLiteral("south") },
              { ProjCore::CrsAxisDirection::SouthSouthWest, QStringLiteral("southSouthWest") },
              { ProjCore::CrsAxisDirection::SouthWest, QStringLiteral("southWest") },
              { ProjCore::CrsAxisDirection::WestSouthWest, QStringLiteral("westSouthWest") },
              { ProjCore::CrsAxisDirection::West, QStringLiteral("west") },
              { ProjCore::CrsAxisDirection::WestNorthWest, QStringLiteral("westNorthWest") },
              { ProjCore::CrsAxisDirection::NorthWest, QStringLiteral("northWest") },
              { ProjCore::CrsAxisDirection::NorthNorthWest, QStringLiteral("northNorthWest") },
              { ProjCore::CrsAxisDirection::GeocentricX, QStringLiteral("geocentricX") },
              { ProjCore::CrsAxisDirection::GeocentricY, QStringLiteral("geocentricY") },
              { ProjCore::CrsAxisDirection::GeocentricZ, QStringLiteral("geocentricZ") },
              { ProjCore::CrsAxisDirection::Up, QStringLiteral("up") },
              { ProjCore::CrsAxisDirection::Down, QStringLiteral("down") },
              { ProjCore::CrsAxisDirection::Forward, QStringLiteral("forward") },
              { ProjCore::CrsAxisDirection::Aft, QStringLiteral("aft") },
              { ProjCore::CrsAxisDirection::Port, QStringLiteral("port") },
              { ProjCore::CrsAxisDirection::Starboard, QStringLiteral("starboard") },
              { ProjCore::CrsAxisDirection::Clockwise, QStringLiteral("clockwise") },
              { ProjCore::CrsAxisDirection::CounterClockwise, QStringLiteral("counterClockwise") },
              { ProjCore::CrsAxisDirection::ColumnPositive, QStringLiteral("columnPositive") },
              { ProjCore::CrsAxisDirection::ColumnNegative, QStringLiteral("columnNegative") },
              { ProjCore::CrsAxisDirection::RowPositive, QStringLiteral("rowPositive") },
              { ProjCore::CrsAxisDirection::RowNegative, QStringLiteral("rowNegative") },
              { ProjCore::CrsAxisDirection::DisplayRight, QStringLiteral("displayRight") },
              { ProjCore::CrsAxisDirection::DisplayLeft, QStringLiteral("displayLeft") },
              { ProjCore::CrsAxisDirection::DisplayUp, QStringLiteral("displayUp") },
              { ProjCore::CrsAxisDirection::DisplayDown, QStringLiteral("displayDown") },
              { ProjCore::CrsAxisDirection::Future, QStringLiteral("future") },
              { ProjCore::CrsAxisDirection::Past, QStringLiteral("past") },
              { ProjCore::CrsAxisDirection::Towards, QStringLiteral("towards") },
              { ProjCore::CrsAxisDirection::AwayFrom, QStringLiteral("awayFrom") },
            };

            QList< ProjCore::CrsAxisDirection > res;
            const int axisCount = proj_cs_get_axis_count(context, pjCs.get());
            if (axisCount > 0)
            {
                res.reserve(axisCount);

                for (int i = 0; i < axisCount; ++i)
                {
                    const char* outDirection = nullptr;
                    proj_cs_get_axis_info(context, pjCs.get(), i,
                        nullptr,
                        nullptr,
                        &outDirection,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr
                    );
                    
                    const thread_local QRegularExpression rx(QStringLiteral("([^\\s]+).*"));
                    const QRegularExpressionMatch match = rx.match(QString(outDirection));
                    if (!match.hasMatch())
                        continue;

                    const QString direction = match.captured(1);
                    ProjCore::CrsAxisDirection dir = ProjCore::CrsAxisDirection::Unspecified;
                    for (auto it = mapping.constBegin(); it != mapping.constEnd(); ++it)
                    {
                        if (it.value().compare(direction, Qt::CaseInsensitive) == 0)
                        {
                            dir = it.key();
                            break;
                        }
                    }

                    res.append(dir);
                }
            }
            return res;
        }

        bool CoordinateReferenceSystem::createFromWkt(const QString& wkt)
        {
            return createFromWktInternal(wkt, QString());
        }
    
        bool CoordinateReferenceSystem::createFromWktInternal(const QString& wkt, const QString& description)
        {
            if (wkt.isEmpty())
                return false;

            d.detach();

            ReadWriteLocker locker(*sCRSWktLock(), ReadWriteLocker::Read);
            if (!sDisableWktCache)
            {
                QHash< QString, CoordinateReferenceSystem >::const_iterator crsIt = sWktCache()->constFind(wkt);
                if (crsIt != sWktCache()->constEnd())
                {
                    
                    *this = crsIt.value();

                    if (!description.isEmpty() && d->mDescription.isEmpty())
                    {
                        
                        d->mDescription = description;
                        locker.changeMode(ReadWriteLocker::Write);
                        sWktCache()->insert(wkt, *this);
                    }
                    return d->mIsValid;
                }
            }
            locker.unlock();

            d->mIsValid = false;
            d->mProj4.clear();
            d->mWktPreferred.clear();
            if (wkt.isEmpty())
            {
                
                return d->mIsValid;
            }

            
            CoordinateReferenceSystem::RecordMap record = getRecord("select * from tbl_srs where wkt=" + SqliteUtils::quotedString(wkt) + " order by deprecated");
            if (!record.empty())
            {
                long srsId = record[QStringLiteral("srs_id")].toLong();
                if (srsId > 0)
                {
                    createFromSrsId(srsId);
                }
            }
            else
            {
                setWktString(wkt);
                if (!description.isEmpty())
                {
                    d->mDescription = description;
                }
                if (d->mSrsId == 0)
                {
                    
                    long id = matchToUserCrs();
                    if (id >= USER_CRS_START_ID)
                    {
                        createFromSrsId(id);
                    }
                }
            }

            locker.changeMode(ReadWriteLocker::Write);
            if (!sDisableWktCache)
                sWktCache()->insert(wkt, *this);

            return d->mIsValid;

        }

bool CoordinateReferenceSystem::isValid() const
{
  return d->mIsValid;
}

bool CoordinateReferenceSystem::createFromProj4( const QString &proj4String )
{
  return createFromProj( proj4String );
}


bool CoordinateReferenceSystem::createFromProj( const QString &projString, const bool identify )
{
  if ( projString.isEmpty() )
    return false;

  d.detach();

  if ( projString.trimmed().isEmpty() )
  {
    d->mIsValid = false;
    d->mProj4.clear();
    d->mWktPreferred.clear();
    return false;
  }

  ReadWriteLocker locker( *sProj4CacheLock(), ReadWriteLocker::Read );
  if ( !sDisableProjCache )
  {
    QHash< QString, CoordinateReferenceSystem >::const_iterator crsIt = sProj4Cache()->constFind( projString );
    if ( crsIt != sProj4Cache()->constEnd() )
    {
      
      *this = crsIt.value();
      return d->mIsValid;
    }
  }
  locker.unlock();

  
  
  
  
  
  
  
  
  QString myProj4String = projString.trimmed();
  myProj4String.remove( QStringLiteral( "+type=crs" ) );
  myProj4String = myProj4String.trimmed();

  d->mIsValid = false;
  d->mWktPreferred.clear();

  if ( identify )
  {
    
    const QString projCrsString = myProj4String + ( myProj4String.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
    ProjUtils::proj_pj_unique_ptr crs( proj_create( ProjContext::get(), projCrsString.toLatin1().constData() ) );
    if ( crs )
    {
      QString authName;
      QString authCode;
      if ( ProjUtils::identifyCrs( crs.get(), authName, authCode, ProjUtils::FlagMatchBoundCrsToUnderlyingSourceCrs ) )
      {
        const QString authid = QStringLiteral( "%1:%2" ).arg( authName, authCode );
        if ( createFromOgcWmsCrs( authid ) )
        {
          locker.changeMode( ReadWriteLocker::Write );
          if ( !sDisableProjCache )
            sProj4Cache()->insert( projString, *this );
          return d->mIsValid;
        }
      }
    }

    
    CoordinateReferenceSystem::RecordMap myRecord = getRecord( "select * from tbl_srs where parameters=" + SqliteUtils::quotedString( myProj4String ) + " order by deprecated" );
    long id = 0;
    if ( !myRecord.empty() )
    {
      id = myRecord[QStringLiteral( "srs_id" )].toLong();
      if ( id >= USER_CRS_START_ID )
      {
        createFromSrsId( id );
      }
    }
    if ( id < USER_CRS_START_ID )
    {
      
      setProjString( myProj4String );

      
      id = matchToUserCrs();
      if ( id >= USER_CRS_START_ID )
      {
        createFromSrsId( id );
      }
    }
  }
  else
  {
    setProjString( myProj4String );
  }

  locker.changeMode( ReadWriteLocker::Write );
  if ( !sDisableProjCache )
    sProj4Cache()->insert( projString, *this );

  return d->mIsValid;
}


CoordinateReferenceSystem::RecordMap CoordinateReferenceSystem::getRecord( const QString &sql )
{
  QString myDatabaseFileName;
  CoordinateReferenceSystem::RecordMap myMap;
  QString myFieldName;
  QString myFieldValue;
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int           myResult;

  
  myDatabaseFileName = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath());
  QFileInfo myInfo( myDatabaseFileName );
  if ( !myInfo.exists() )
  {
    
    return myMap;
  }

  
  myResult = openDatabase( myDatabaseFileName, database );
  if ( myResult != SQLITE_OK )
  {
    return myMap;
  }

  statement = database.prepare( sql, myResult );
  
  if ( myResult == SQLITE_OK && statement.step() == SQLITE_ROW )
  {
    int myColumnCount = statement.columnCount();
    
    for ( int myColNo = 0; myColNo < myColumnCount; myColNo++ )
    {
      myFieldName = statement.columnName( myColNo );
      myFieldValue = statement.columnAsText( myColNo );
      myMap[myFieldName] = myFieldValue;
    }
    if ( statement.step() != SQLITE_DONE )
    {
      
      
    }
  }
  else
  {
    
  }

  if ( myMap.empty() )
  {
    myDatabaseFileName = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath());
    QFileInfo myFileInfo;
    myFileInfo.setFile( myDatabaseFileName );
    if ( !myFileInfo.exists() )
    {
      
      return myMap;
    }

    
    myResult = openDatabase( myDatabaseFileName, database );
    if ( myResult != SQLITE_OK )
    {
      return myMap;
    }

    statement = database.prepare( sql, myResult );
    
    if ( myResult == SQLITE_OK && statement.step() == SQLITE_ROW )
    {
      int myColumnCount = statement.columnCount();
      
      for ( int myColNo = 0; myColNo < myColumnCount; myColNo++ )
      {
        myFieldName = statement.columnName( myColNo );
        myFieldValue = statement.columnAsText( myColNo );
        myMap[myFieldName] = myFieldValue;
      }

      if ( statement.step() != SQLITE_DONE )
      {
        
        myMap.clear();
      }
    }
    else
    {
      
    }
  }
  return myMap;
}



long CoordinateReferenceSystem::srsid() const
{
  return d->mSrsId;
}
std::string CoordinateReferenceSystem::GetAuthID() const
{
    return d->mAuthId.toStdString();
}

QString CoordinateReferenceSystem::authid() const
{
  return d->mAuthId;
}
std::string CoordinateReferenceSystem::GetDescription() const
{
   
    return description().toStdString();
}
QString CoordinateReferenceSystem::description() const
{
  if ( d->mDescription.isNull() )
  {
    return QString();
  }
  else
  {
    return d->mDescription;
  }
}

QString CoordinateReferenceSystem::userFriendlyIdentifier( ProjCore::CrsIdentifierType type ) const
{
  QString id;
  if ( !authid().isEmpty() )
  {
    if ( type != ProjCore::CrsIdentifierType::ShortString && !description().isEmpty() )
      id = QStringLiteral( "%1 - %2" ).arg( authid(), description() );
    else
      id = authid();
  }
  else if ( !description().isEmpty() )
    id = description();
  else if ( type == ProjCore::CrsIdentifierType::ShortString )
    id = isValid() ? QObject::tr( "Custom CRS" ) : QObject::tr( "Unknown CRS" );
  else if ( !toWkt( ProjCore::CrsWktVariant::Preferred ).isEmpty() )
    id = QObject::tr( "Custom CRS: %1" ).arg(
           type == ProjCore::CrsIdentifierType::MediumString ? ( toWkt( ProjCore::CrsWktVariant::Preferred ).left( 50 ) + QString( QChar( 0x2026 ) ) )
           : toWkt( ProjCore::CrsWktVariant::Preferred ) );
  else if ( !toProj().isEmpty() )
    id = QObject::tr( "Custom CRS: %1" ).arg( type == ProjCore::CrsIdentifierType::MediumString ? ( toProj().left( 50 ) + QString( QChar( 0x2026 ) ) )
         : toProj() );
  if ( !id.isEmpty() && !std::isnan( d->mCoordinateEpoch ) )
    id += QStringLiteral( " @ %1" ).arg( d->mCoordinateEpoch );

  return id;
}

QString CoordinateReferenceSystem::projectionAcronym() const
{
  if ( d->mProjectionAcronym.isNull() )
  {
    return QString();
  }
  else
  {
    return d->mProjectionAcronym;
  }
}

QString CoordinateReferenceSystem::ellipsoidAcronym() const
{
  if ( d->mEllipsoidAcronym.isNull() )
  {
    if ( PJ *obj = d->threadLocalProjObject() )
    {
      ProjUtils::proj_pj_unique_ptr ellipsoid( proj_get_ellipsoid( ProjContext::get(), obj ) );
      if ( ellipsoid )
      {
        const QString ellipsoidAuthName( proj_get_id_auth_name( ellipsoid.get(), 0 ) );
        const QString ellipsoidAuthCode( proj_get_id_code( ellipsoid.get(), 0 ) );
        if ( !ellipsoidAuthName.isEmpty() && !ellipsoidAuthCode.isEmpty() )
          d->mEllipsoidAcronym = QStringLiteral( "%1:%2" ).arg( ellipsoidAuthName, ellipsoidAuthCode );
        else
        {
          double semiMajor, semiMinor, invFlattening;
          int semiMinorComputed = 0;
          if ( proj_ellipsoid_get_parameters( ProjContext::get(), ellipsoid.get(), &semiMajor, &semiMinor, &semiMinorComputed, &invFlattening ) )
          {
            d->mEllipsoidAcronym = QStringLiteral( "PARAMETER:%1:%2" ).arg(DoubleToQString( semiMajor ),
                DoubleToQString( semiMinor ) );
          }
          else
          {
            d->mEllipsoidAcronym.clear();
          }
        }
      }
    }
    return d->mEllipsoidAcronym;
  }
  else
  {
    return d->mEllipsoidAcronym;
  }
}

QString CoordinateReferenceSystem::toProj4() const
{
  return toProj();
}

QString CoordinateReferenceSystem::toProj() const
{
  if ( !d->mIsValid )
    return QString();

  if ( d->mProj4.isEmpty() )
  {
    if ( PJ *obj = d->threadLocalProjObject() )
    {
      d->mProj4 = getFullProjString( obj );
    }
  }
  
  return d->mProj4.trimmed();
}

coord_system_type_e CoordinateReferenceSystem::GetType() const
{
    ProjCore::CrsType type = this->type();
    if (type == ProjCore::CrsType::Geodetic || type == ProjCore::CrsType::Geographic2d
        || type == ProjCore::CrsType::Geographic3d || type == ProjCore::CrsType::Compound)
    {
        return coord_system_type_e::GEOGRAPHIC;
    }
    else if (type == ProjCore::CrsType::Geocentric)
    {
        return coord_system_type_e::GEOCENTRIC;
    }
    else if (type == ProjCore::CrsType::Projected)
    {
        return coord_system_type_e::PROJECTION;
    }
    else
    {
        QString authid = this->authid().toUpper();
        if (authid.startsWith("ENU:"))
        {
            return coord_system_type_e::LOCAL_ENU;
        }
    }

    return _coord_system_type_e::LOCAL;
}

ProjCore::CrsType CoordinateReferenceSystem::type() const
{
  
  switch ( d->mProjType )
  {
    case PJ_TYPE_UNKNOWN:
      return ProjCore::CrsType::Unknown;

    case PJ_TYPE_ELLIPSOID:
    case PJ_TYPE_PRIME_MERIDIAN:
    case PJ_TYPE_GEODETIC_REFERENCE_FRAME:
    case PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME:
    case PJ_TYPE_VERTICAL_REFERENCE_FRAME:
    case PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME:
    case PJ_TYPE_DATUM_ENSEMBLE:
    case PJ_TYPE_CONVERSION:
    case PJ_TYPE_TRANSFORMATION:
    case PJ_TYPE_CONCATENATED_OPERATION:
    case PJ_TYPE_OTHER_COORDINATE_OPERATION:
  

    case PJ_TYPE_CRS:
    case PJ_TYPE_GEOGRAPHIC_CRS:
      
      return ProjCore::CrsType::Other;

    case PJ_TYPE_GEODETIC_CRS:
      return ProjCore::CrsType::Geodetic;
    case PJ_TYPE_GEOCENTRIC_CRS:
      return ProjCore::CrsType::Geocentric;
    case PJ_TYPE_GEOGRAPHIC_2D_CRS:
      return ProjCore::CrsType::Geographic2d;
    case PJ_TYPE_GEOGRAPHIC_3D_CRS:
      return ProjCore::CrsType::Geographic3d;
    case PJ_TYPE_VERTICAL_CRS:
      return ProjCore::CrsType::Vertical;
    case PJ_TYPE_PROJECTED_CRS:
      return ProjCore::CrsType::Projected;
    case PJ_TYPE_COMPOUND_CRS:
      return ProjCore::CrsType::Compound;
    case PJ_TYPE_TEMPORAL_CRS:
      return ProjCore::CrsType::Temporal;
    case PJ_TYPE_ENGINEERING_CRS:
      return ProjCore::CrsType::Engineering;
    case PJ_TYPE_BOUND_CRS:
      return ProjCore::CrsType::Bound;
    case PJ_TYPE_OTHER_CRS:
      return ProjCore::CrsType::Other;
#if PROJ_VERSION_MAJOR>9 || (PROJ_VERSION_MAJOR==9 && PROJ_VERSION_MINOR>=2)
    case PJ_TYPE_DERIVED_PROJECTED_CRS:
      return ProjCore::CrsType::DerivedProjected;
    case PJ_TYPE_COORDINATE_METADATA:
      return ProjCore::CrsType::Other;
#endif
  }
  return ProjCore::CrsType::Unknown;
  
}

bool CoordinateReferenceSystem::isDeprecated() const
{
  const PJ *pj = projObject();
  if ( !pj )
    return false;

  return proj_is_deprecated( pj );
}

bool CoordinateReferenceSystem::isGeographic() const
{
  return d->mIsGeographic;
}

bool CoordinateReferenceSystem::isDynamic() const
{
  const PJ *pj = projObject();
  if ( !pj )
    return false;

  return ProjUtils::isDynamic( pj );
}



void CoordinateReferenceSystem::setCoordinateEpoch( double epoch )
{
  if ( d->mCoordinateEpoch == epoch )
    return;

  
  ProjUtils::proj_pj_unique_ptr clone( proj_clone( ProjContext::get(), projObject() ) );
  d.detach();
  d->mCoordinateEpoch = epoch;
  d->setPj( std::move( clone ) );
}

double CoordinateReferenceSystem::coordinateEpoch() const
{
  return d->mCoordinateEpoch;
}


















































ProjectionFactors CoordinateReferenceSystem::factors( const Eigen::Vector2d &point ) const
{
  ProjectionFactors res;

  
  QString projString = toProj();
  projString.replace( QLatin1String( "+type=crs" ), QString() );

  ProjUtils::proj_pj_unique_ptr transformation( proj_create( ProjContext::get(), projString.toUtf8().constData() ) );
  if ( !transformation )
    return res;

  PJ_COORD coord = proj_coord( 0, 0, 0, HUGE_VAL );
  coord.uv.u = point.x() * M_PI / 180.0;
  coord.uv.v = point.y() * M_PI / 180.0;

  proj_errno_reset( transformation.get() );
  const PJ_FACTORS pjFactors = proj_factors( transformation.get(), coord );
  if ( proj_errno( transformation.get() ) )
  {
    return res;
  }

  res.mIsValid = true;
  res.mMeridionalScale = pjFactors.meridional_scale;
  res.mParallelScale = pjFactors.parallel_scale;
  res.mArealScale = pjFactors.areal_scale;
  res.mAngularDistortion = pjFactors.angular_distortion;
  res.mMeridianParallelAngle = pjFactors.meridian_parallel_angle * 180 / M_PI;
  res.mMeridianConvergence = pjFactors.meridian_convergence * 180 / M_PI;
  res.mTissotSemimajor = pjFactors.tissot_semimajor;
  res.mTissotSemiminor = pjFactors.tissot_semiminor;
  res.mDxDlam = pjFactors.dx_dlam;
  res.mDxDphi = pjFactors.dx_dphi;
  res.mDyDlam = pjFactors.dy_dlam;
  res.mDyDphi = pjFactors.dy_dphi;
  return res;
}

ProjOperation CoordinateReferenceSystem::operation() const
{
  if ( !d->mIsValid )
    return ProjOperation();

  ProjOperation res;

  
  QString projString = toProj();
  projString.replace( QLatin1String( "+type=crs" ), QString() );
  if ( projString.isEmpty() )
    return ProjOperation();

  ProjUtils::proj_pj_unique_ptr transformation( proj_create( ProjContext::get(), projString.toUtf8().constData() ) );
  if ( !transformation )
    return res;

  PJ_PROJ_INFO info = proj_pj_info( transformation.get() );

  if ( info.id )
  {
    return  QProj::coordinateReferenceSystemRegistry()->projOperations().value( QString( info.id ) );
  }

  return res;
}

ProjCore::DistanceUnit CoordinateReferenceSystem::mapUnits() const
{
  if ( !d->mIsValid )
    return ProjCore::DistanceUnit::Unknown;

  return d->mMapUnits;
}


QString CoordinateReferenceSystem::toOgcUri() const
{
  const auto parts { authid().split( ':' ) };
  if ( parts.length() == 2 )
  {
    if ( parts[0] == QLatin1String( "EPSG" ) )
      return  QStringLiteral( "http://www.opengis.net/def/crs/EPSG/0/%1" ).arg( parts[1] ) ;
    else if ( parts[0] == QLatin1String( "OGC" ) )
    {
      return  QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/%1" ).arg( parts[1] ) ;
    }
    else
    {
      
    }
  }
  else
  {
    
  }
  return QString();
}

void CoordinateReferenceSystem::updateDefinition()
{
  if ( !d->mIsValid )
    return;

  if ( d->mSrsId >= USER_CRS_START_ID )
  {
    
    createFromSrsId( d->mSrsId );
  }
  else
  {
    
  }
}

void CoordinateReferenceSystem::setProjString( const QString &proj4String )
{
  d.detach();
  d->mProj4 = proj4String;
  d->mWktPreferred.clear();

  LocaleNumC l;
  QString trimmed = proj4String.trimmed();

  trimmed += QLatin1String( " +type=crs" );
  PJ_CONTEXT *ctx = ProjContext::get();

  {
    d->setPj( ProjUtils::proj_pj_unique_ptr( proj_create( ctx, trimmed.toLatin1().constData() ) ) );
  }

  if ( !d->hasPj() )
  {
#ifdef QGISDEBUG
    const int errNo = proj_context_errno( ctx );
    
#endif
    d->mIsValid = false;
  }
  else
  {
    d->mEllipsoidAcronym.clear();
    d->mIsValid = true;
  }

  setMapUnits();
}

bool CoordinateReferenceSystem::setWktString( const QString &wkt )
{
  bool res = false;
  d->mIsValid = false;
  d->mWktPreferred.clear();

  PROJ_STRING_LIST warnings = nullptr;
  PROJ_STRING_LIST grammerErrors = nullptr;
  {
    d->setPj( ProjUtils::proj_pj_unique_ptr( proj_create_from_wkt( ProjContext::get(), wkt.toLatin1().constData(), nullptr, &warnings, &grammerErrors ) ) );
  }

  res = d->hasPj();
  if ( !res )
  {
    
    
    
      for (auto iter = warnings; iter && *iter; ++iter)
      {
          
          for (auto iter = grammerErrors; iter && *iter; ++iter)
          {

          }
      }
      
    
  }
  proj_string_list_destroy( warnings );
  proj_string_list_destroy( grammerErrors );

  ReadWriteLocker locker( *sProj4CacheLock(), ReadWriteLocker::Unlocked );
  if ( !res )
  {
    locker.changeMode( ReadWriteLocker::Write );
    if ( !sDisableWktCache )
      sWktCache()->insert( wkt, *this );
    return d->mIsValid;
  }

  if ( d->hasPj() )
  {
    
    QString authName( proj_get_id_auth_name( d->threadLocalProjObject(), 0 ) );
    QString authCode( proj_get_id_code( d->threadLocalProjObject(), 0 ) );

    if ( authName.isEmpty() || authCode.isEmpty() )
    {
      
      ProjUtils::identifyCrs( d->threadLocalProjObject(), authName, authCode );
    }

    if ( !authName.isEmpty() && !authCode.isEmpty() )
    {
      if ( loadFromAuthCode( authName, authCode ) )
      {
        locker.changeMode( ReadWriteLocker::Write );
        if ( !sDisableWktCache )
          sWktCache()->insert( wkt, *this );
        return d->mIsValid;
      }
    }
    else
    {
      
      d->mIsValid = true;
      d->mDescription = QString( proj_get_name( d->threadLocalProjObject() ) );
    }
    setMapUnits();
  }

  return d->mIsValid;
}

void CoordinateReferenceSystem::setMapUnits()
{
  if ( !d->mIsValid )
  {
    d->mMapUnits = ProjCore::DistanceUnit::Unknown;
    return;
  }

  if ( !d->hasPj() )
  {
    d->mMapUnits = ProjCore::DistanceUnit::Unknown;
    return;
  }

  PJ_CONTEXT *context = ProjContext::get();
  
  ProjUtils::proj_pj_unique_ptr crs( ProjUtils::crsToHorizontalCrs( d->threadLocalProjObject() ) );
  if ( !crs )
    crs = ProjUtils::unboundCrs( d->threadLocalProjObject() );

  if ( !crs )
  {
    d->mMapUnits = ProjCore::DistanceUnit::Unknown;
    return;
  }

  ProjUtils::proj_pj_unique_ptr coordinateSystem( proj_crs_get_coordinate_system( context, crs.get() ) );
  if ( !coordinateSystem )
  {
    d->mMapUnits = ProjCore::DistanceUnit::Unknown;
    return;
  }

  const int axisCount = proj_cs_get_axis_count( context, coordinateSystem.get() );
  if ( axisCount > 0 )
  {
    const char *outUnitName = nullptr;
    
    proj_cs_get_axis_info( context, coordinateSystem.get(), 0,
                           nullptr,
                           nullptr,
                           nullptr,
                           nullptr,
                           &outUnitName,
                           nullptr,
                           nullptr );

    const QString unitName( outUnitName );

    
    
    if ( unitName.compare( QLatin1String( "degree" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute second" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute second hemisphere" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree hemisphere" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute hemisphere" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "hemisphere degree" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "hemisphere degree minute" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "hemisphere degree minute second" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree (supplier to define representation)" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = ProjCore::DistanceUnit::Degrees;
    else if ( unitName.compare( QLatin1String( "metre" ), Qt::CaseInsensitive ) == 0
              || unitName.compare( QLatin1String( "m" ), Qt::CaseInsensitive ) == 0
              || unitName.compare( QLatin1String( "meter" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = ProjCore::DistanceUnit::Meters;
    
    else if ( unitName.compare( QLatin1String( "US survey foot" ), Qt::CaseInsensitive ) == 0 ||
              unitName.compare( QLatin1String( "foot" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = ProjCore::DistanceUnit::Feet;
    else if ( unitName.compare( QLatin1String( "kilometre" ), Qt::CaseInsensitive ) == 0 )  
      d->mMapUnits = ProjCore::DistanceUnit::Kilometers;
    else if ( unitName.compare( QLatin1String( "centimetre" ), Qt::CaseInsensitive ) == 0 )  
      d->mMapUnits = ProjCore::DistanceUnit::Centimeters;
    else if ( unitName.compare( QLatin1String( "millimetre" ), Qt::CaseInsensitive ) == 0 )  
      d->mMapUnits = ProjCore::DistanceUnit::Millimeters;
    else if ( unitName.compare( QLatin1String( "Statute mile" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = ProjCore::DistanceUnit::Miles;
    else if ( unitName.compare( QLatin1String( "nautical mile" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = ProjCore::DistanceUnit::NauticalMiles;
    else if ( unitName.compare( QLatin1String( "yard" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = ProjCore::DistanceUnit::Yards;
    
    else
      d->mMapUnits = ProjCore::DistanceUnit::Unknown;
    return;
  }
  else
  {
    d->mMapUnits = ProjCore::DistanceUnit::Unknown;
    return;
  }
}
static void proj_collecting_logger(void* user_data, int , const char* message)
{
    QStringList* dest = reinterpret_cast<QStringList*>(user_data);
    QString messageString(message);
    messageString.replace(QLatin1String("internal_proj_create: "), QString());
    dest->append(messageString);
}

void CoordinateReferenceSystem::validateCurrent(const QString projDef)
{
    

    PJ_CONTEXT* context = proj_context_create();

    QStringList projErrors;
    proj_log_func(context, &projErrors, proj_collecting_logger);
    ProjUtils::proj_pj_unique_ptr crs;
    QString projDeflower = projDef.toLower();
    




    bool isproj = AI3D::CORE::String::StringStartsWith(projDeflower.toStdString(), "proj");

    bool iswkt = AI3D::CORE::String::StringStartsWith(projDeflower.toStdString(), "wkt");
    ProjCore::CrsDefinitionFormat defetype ;
    if(isproj)
        defetype = ProjCore::CrsDefinitionFormat::Proj;
    if (iswkt)
        defetype =  ProjCore::CrsDefinitionFormat::Wkt ;
    
    
    switch (defetype)
    {
    case ProjCore::CrsDefinitionFormat::Wkt:
    {
        PROJ_STRING_LIST warnings = nullptr;
        PROJ_STRING_LIST grammerErrors = nullptr;
        crs.reset(proj_create_from_wkt(context, projDef.toUtf8().constData(), nullptr, &warnings, &grammerErrors));
        QStringList warningStrings;
        QStringList grammerStrings;
        for (auto iter = warnings; iter && *iter; ++iter)
            warningStrings << QString(*iter);
        for (auto iter = grammerErrors; iter && *iter; ++iter)
            grammerStrings << QString(*iter);
        proj_string_list_destroy(warnings);
        proj_string_list_destroy(grammerErrors);

        if (crs)
        {
            LOGI("Custom Coordinate Reference System: This WKT projection definition is valid.");
           
        }
        else
        {
            LOGI("Custom Coordinate Reference System: This WKT projection definition is not valid.");
            
        }
        break;
    }

    case ProjCore::CrsDefinitionFormat::Proj:
    {
        const QString projCrsString = projDef + (projDef.contains(QStringLiteral("+type=crs")) ? QString() : QStringLiteral(" +type=crs"));
        crs.reset(proj_create(context, projCrsString.toUtf8().constData()));
        if (crs)
        {
            LOGI("Custom Coordinate Reference System: This proj projection definition is valid.");
           
        }
        else
        {
            LOGI("Custom Coordinate Reference System: This proj projection definition is not valid.");
            
        }
        break;
    }
    }

    
    proj_log_func(context, nullptr, nullptr);
    proj_context_destroy(context);
    context = nullptr;
}

long CoordinateReferenceSystem::findMatchingProj()
{
  if ( d->mEllipsoidAcronym.isNull() || d->mProjectionAcronym.isNull()
       || !d->mIsValid )
  {
      LOGD("CoordinateReferenceSystem::findMatchingProj will only work if prj acr ellipsoid acr and proj4string are set and the current projection is valid!" );
    
                    
    return 0;
  }

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int myResult;

  
  
  QString mySql = QString( "select srs_id,parameters from tbl_srs where "
                           "projection_acronym=%1 and ellipsoid_acronym=%2 order by deprecated" )
                  .arg( SqliteUtils::quotedString( d->mProjectionAcronym ),
                        SqliteUtils::quotedString( d->mEllipsoidAcronym ) );
  
  QString myDatabaseFileName = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath());

  
  myResult = openDatabase( myDatabaseFileName, database );
  if ( myResult != SQLITE_OK )
  {
    return 0;
  }

  statement = database.prepare( mySql, myResult );
  if ( myResult == SQLITE_OK )
  {

    while ( statement.step() == SQLITE_ROW )
    {
      QString mySrsId = statement.columnAsText( 0 );
      QString myProj4String = statement.columnAsText( 1 );
      if ( toProj() == myProj4String.trimmed() )
      {
        return mySrsId.toLong();
      }
    }
  }

  
  
  

  myDatabaseFileName = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath());
  
  myResult = openDatabase( myDatabaseFileName, database );
  if ( myResult != SQLITE_OK )
  {
    return 0;
  }

  statement = database.prepare( mySql, myResult );

  if ( myResult == SQLITE_OK )
  {
    while ( statement.step() == SQLITE_ROW )
    {
      QString mySrsId = statement.columnAsText( 0 );
      QString myProj4String = statement.columnAsText( 1 );
      if ( toProj() == myProj4String.trimmed() )
      {
        return mySrsId.toLong();
      }
    }
  }

  return 0;
}

bool CoordinateReferenceSystem::operator==( const CoordinateReferenceSystem &srs ) const
{
  
  if ( d == srs.d )
    return true;

  if ( !d->mIsValid && !srs.d->mIsValid )
    return true;

  if ( !d->mIsValid || !srs.d->mIsValid )
    return false;

  if ( !NanCompatibleEquals( d->mCoordinateEpoch, srs.d->mCoordinateEpoch ) )
    return false;

  const bool isUser = d->mSrsId >= USER_CRS_START_ID;
  const bool otherIsUser = srs.d->mSrsId >= USER_CRS_START_ID;
  if ( isUser != otherIsUser )
    return false;

  
  if ( !isUser && ( !d->mAuthId.isEmpty() || !srs.d->mAuthId.isEmpty() ) )
    return d->mAuthId == srs.d->mAuthId;

  return toWkt( ProjCore::CrsWktVariant::Preferred ) == srs.toWkt( ProjCore::CrsWktVariant::Preferred );
}

bool CoordinateReferenceSystem::operator!=( const CoordinateReferenceSystem &srs ) const
{
  return  !( *this == srs );
}

QString CoordinateReferenceSystem::toWkt( ProjCore::CrsWktVariant variant, bool multiline, int indentationWidth ) const
{
  if ( PJ *obj = d->threadLocalProjObject() )
  {
    const bool isDefaultPreferredFormat = variant == ProjCore::CrsWktVariant::Preferred && !multiline;
    if ( isDefaultPreferredFormat && !d->mWktPreferred.isEmpty() )
    {
      
      return d->mWktPreferred;
    }

    PJ_WKT_TYPE type = PJ_WKT1_GDAL;
    switch ( variant )
    {
      case ProjCore::CrsWktVariant::Wkt1Gdal:
        type = PJ_WKT1_GDAL;
        break;
      case ProjCore::CrsWktVariant::Wkt1Esri:
        type = PJ_WKT1_ESRI;
        break;
      case ProjCore::CrsWktVariant::Wkt2_2015:
        type = PJ_WKT2_2015;
        break;
      case ProjCore::CrsWktVariant::Wkt2_2015Simplified:
        type = PJ_WKT2_2015_SIMPLIFIED;
        break;
      case ProjCore::CrsWktVariant::Wkt2_2019:
        type = PJ_WKT2_2019;
        break;
      case ProjCore::CrsWktVariant::Wkt2_2019Simplified:
        type = PJ_WKT2_2019_SIMPLIFIED;
        break;
    }

    const QByteArray multiLineOption = QStringLiteral( "MULTILINE=%1" ).arg( multiline ? QStringLiteral( "YES" ) : QStringLiteral( "NO" ) ).toUtf8();
    const QByteArray indentatationWidthOption = QStringLiteral( "INDENTATION_WIDTH=%1" ).arg( multiline ? QString::number( indentationWidth ) : QStringLiteral( "0" ) ).toUtf8();
    const char *const options[] = {multiLineOption.constData(), indentatationWidthOption.constData(), nullptr};
    QString res = proj_as_wkt( ProjContext::get(), obj, type, options );

    if ( isDefaultPreferredFormat )
    {
      
      d->mWktPreferred = res;
    }

    return res;
  }
  return QString();
}








QString CoordinateReferenceSystem::projFromSrsId( const int srsId )
{
  QString myDatabaseFileName;
  QString myProjString;
  QString mySql = QStringLiteral( "select parameters from tbl_srs where srs_id = %1 order by deprecated" ).arg( srsId );

  
  
  
  
  if ( srsId >= USER_CRS_START_ID )
  {
    myDatabaseFileName = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath());
    QFileInfo myFileInfo;
    myFileInfo.setFile( myDatabaseFileName );
    if ( !myFileInfo.exists() ) 
    {
      
      return QString();
    }
  }
  else 
  {
    myDatabaseFileName = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath());
  }

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  int rc;
  rc = openDatabase( myDatabaseFileName, database );
  if ( rc )
  {
    return QString();
  }

  statement = database.prepare( mySql, rc );

  if ( rc == SQLITE_OK )
  {
    if ( statement.step() == SQLITE_ROW )
    {
      myProjString = statement.columnAsText( 0 );
    }
  }

  return myProjString;
}

int CoordinateReferenceSystem::openDatabase( const QString &path, sqlite3_database_unique_ptr &database, bool readonly )
{
  int myResult;
  if ( readonly )
    myResult = database.open_v2( path, SQLITE_OPEN_READONLY, nullptr );
  else
    myResult = database.open( path );

  if ( myResult != SQLITE_OK )
  {
    
    
    
    
    
                              
  }
  return myResult;
}

void CoordinateReferenceSystem::setCustomCrsValidation( CUSTOM_CRS_VALIDATION f )
{
  sCustomSrsValidation = f;
}

CUSTOM_CRS_VALIDATION CoordinateReferenceSystem::customCrsValidation()
{
  return sCustomSrsValidation;
}

void CoordinateReferenceSystem::debugPrint()
{
  
  
  
  
  
  
  if ( mapUnits() == ProjCore::DistanceUnit::Meters )
  {
    
  }
  else if ( mapUnits() == ProjCore::DistanceUnit::Feet )
  {
    
  }
  else if ( mapUnits() == ProjCore::DistanceUnit::Degrees )
  {
    
  }
}

void CoordinateReferenceSystem::setValidationHint( const QString &html )
{
  mValidationHint = html;
}

QString CoordinateReferenceSystem::validationHint() const
{
  return mValidationHint;
}

long CoordinateReferenceSystem::SaveAsUsersCrs(const std::string& definition, ProjCore::CrsDefinitionFormat nativeFormat)
{
    std::string userdatabasefile = AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath();
    if (!IsExists(userdatabasefile, definition))
    {
        return QProj::coordinateReferenceSystemRegistry()->AddUserCrsToDatabase(*this, QString::fromStdString(definition), nativeFormat);
        
   }
    else
    {

    }
    return -1;
}

long CoordinateReferenceSystem::saveAsUserCrs( const QString &name, ProjCore::CrsDefinitionFormat nativeFormat )
{
    return QProj::coordinateReferenceSystemRegistry()->addUserCrs( *this, name, nativeFormat );
}


void CoordinateReferenceSystem::setNativeFormat( ProjCore::CrsDefinitionFormat format )
{
  mNativeFormat = format;
}

ProjCore::CrsDefinitionFormat CoordinateReferenceSystem::nativeFormat() const
{
  return mNativeFormat;
}

long CoordinateReferenceSystem::getRecordCount()
{
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int           myResult;
  long          myRecordCount = 0;
  
  myResult = database.open_v2( QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()), SQLITE_OPEN_READONLY, nullptr );
  if ( myResult != SQLITE_OK )
  {
    
    return 0;
  }
  
  QString mySql = QStringLiteral( "select count(*) from tbl_srs" );
  statement = database.prepare( mySql, myResult );
  if ( myResult == SQLITE_OK )
  {
    if ( statement.step() == SQLITE_ROW )
    {
      QString myRecordCountString = statement.columnAsText( 0 );
      myRecordCount = myRecordCountString.toLong();
      
    }
  }
  return myRecordCount;
}

bool testIsGeographic( PJ *crs )
{
  PJ_CONTEXT *pjContext = ProjContext::get();
  bool isGeographic = false;
  ProjUtils::proj_pj_unique_ptr coordinateSystem( proj_crs_get_coordinate_system( pjContext, crs ) );
  if ( coordinateSystem )
  {
    const int axisCount = proj_cs_get_axis_count( pjContext, coordinateSystem.get() );
    if ( axisCount > 0 )
    {
      const char *outUnitAuthName = nullptr;
      const char *outUnitAuthCode = nullptr;
      
      proj_cs_get_axis_info( pjContext, coordinateSystem.get(), 0,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             &outUnitAuthName,
                             &outUnitAuthCode );

      if ( outUnitAuthName && outUnitAuthCode )
      {
        const char *unitCategory = nullptr;
        if ( proj_uom_get_info_from_database( pjContext, outUnitAuthName, outUnitAuthCode, nullptr, nullptr, &unitCategory ) )
        {
          isGeographic = QString( unitCategory ).compare( QLatin1String( "angular" ), Qt::CaseInsensitive ) == 0;
        }
      }
    }
  }
  return isGeographic;
}

void getOperationAndEllipsoidFromProjString( const QString &proj, QString &operation, QString &ellipsoid )
{
  thread_local const QRegularExpression projRegExp( QStringLiteral( "\\+proj=(\\S+)" ) );
  const QRegularExpressionMatch projMatch = projRegExp.match( proj );
  if ( !projMatch.hasMatch() )
  {
    
    return;
  }
  operation = projMatch.captured( 1 );

  const QRegularExpressionMatch ellipseMatch = projRegExp.match( proj );
  if ( ellipseMatch.hasMatch() )
  {
    ellipsoid = ellipseMatch.captured( 1 );
  }
  else
  {
    
    
    
    
    
    ellipsoid = "";
  }
}


bool CoordinateReferenceSystem::loadFromAuthCode( const QString &auth, const QString &code )
{
  if ( !  QProj::coordinateReferenceSystemRegistry()->authorities().contains( auth.toLower() ) )
    return false;

  d.detach();
  d->mIsValid = false;
  d->mWktPreferred.clear();

  PJ_CONTEXT *pjContext = ProjContext::get();
  ProjUtils::proj_pj_unique_ptr crs( proj_create_from_database( pjContext, auth.toUtf8().constData(), code.toUtf8().constData(), PJ_CATEGORY_CRS, false, nullptr ) );
  if ( !crs )
  {
    return false;
  }

  crs = ProjUtils::unboundCrs( crs.get() );

  QString proj4 = getFullProjString( crs.get() );
  proj4.replace( QLatin1String( "+type=crs" ), QString() );
  proj4 = proj4.trimmed();

  d->mIsValid = true;
  d->mProj4 = proj4;
  d->mWktPreferred.clear();
  d->mDescription = QString( proj_get_name( crs.get() ) );
  d->mAuthId = QStringLiteral( "%1:%2" ).arg( auth, code );
  d->mIsGeographic = testIsGeographic( crs.get() );
  d->mAxisInvertedDirty = true;
  QString operation;
  QString ellipsoid;
  getOperationAndEllipsoidFromProjString( proj4, operation, ellipsoid );
  d->mProjectionAcronym = operation;
  d->mEllipsoidAcronym.clear();
  d->setPj( std::move( crs ) );

  const QString dbVals = sAuthIdToSrsIdMap.value( QStringLiteral( "%1:%2" ).arg( auth, code ).toUpper() );
  if ( !dbVals.isEmpty() )
  {
    const QStringList parts = dbVals.split( ',' );
    d->mSrsId = parts.at( 0 ).toInt();
    d->mSRID = parts.at( 1 ).toInt();
  }

  setMapUnits();

  return true;
}

QList<long> CoordinateReferenceSystem::userSrsIds()
{
  QList<long> results;
  
  const QString db = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath());

  QFileInfo myInfo( db );
  if ( !myInfo.exists() )
  {
    
    return results;
  }

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  
  int result = openDatabase( db, database );
  if ( result != SQLITE_OK )
  {
    
    return results;
  }

  QString sql = QStringLiteral( "select srs_id from tbl_srs where srs_id >= %1" ).arg( USER_CRS_START_ID );
  int rc;
  statement = database.prepare( sql, rc );
  while ( true )
  {
    int ret = statement.step();

    if ( ret == SQLITE_DONE )
    {
      
      break;
    }

    if ( ret == SQLITE_ROW )
    {
      results.append( statement.columnAsInt64( 0 ) );
    }
    else
    {
      
      break;
    }
  }

  return results;
}

long CoordinateReferenceSystem::matchToUserCrs() const
{
  PJ *obj = d->threadLocalProjObject();
  if ( !obj )
    return 0;

  const QList< long > ids = userSrsIds();
  for ( long id : ids )
  {
    CoordinateReferenceSystem candidate = CoordinateReferenceSystem::fromSrsId( id );
    if ( candidate.projObject() && proj_is_equivalent_to( obj, candidate.projObject(), PJ_COMP_EQUIVALENT ) )
    {
      return id;
    }
  }
  return 0;
}

static void sync_db_proj_logger( void * , int level, const char *message )
{
#ifndef QGISDEBUG
  Q_UNUSED( message )
#endif
  if ( level == PJ_LOG_ERROR )
  {
    
  }
  else if ( level == PJ_LOG_DEBUG )
  {
    
  }
}

int CoordinateReferenceSystem::syncDatabase()
{
  setlocale( LC_ALL, "C" );
  QString dbFilePath = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath());

  int inserted = 0, updated = 0, deleted = 0, errors = 0;

  

  sqlite3_database_unique_ptr database;
  if ( database.open( dbFilePath ) != SQLITE_OK )
  {
    
    return -1;
  }

  if ( sqlite3_exec( database.get(), "BEGIN TRANSACTION", nullptr, nullptr, nullptr ) != SQLITE_OK )
  {
    
    return -1;
  }

  sqlite3_statement_unique_ptr statement;
  int result;
  char *errMsg = nullptr;

  bool createdTypeColumn = false;
  if ( sqlite3_exec( database.get(), "ALTER TABLE tbl_srs ADD COLUMN srs_type text", nullptr, nullptr, nullptr ) == SQLITE_OK )
  {
    createdTypeColumn = true;
    if ( sqlite3_exec( database.get(), "CREATE INDEX srs_type ON tbl_srs(srs_type)", nullptr, nullptr, nullptr ) != SQLITE_OK )
    {
      
      return -1;
    }
  }

  if ( sqlite3_exec( database.get(), "create table tbl_info (proj_major INT, proj_minor INT, proj_patch INT)", nullptr, nullptr, nullptr ) == SQLITE_OK )
  {
    QString sql = QStringLiteral( "INSERT INTO tbl_info(proj_major, proj_minor, proj_patch) VALUES (%1, %2,%3)" )
                  .arg( QString::number( PROJ_VERSION_MAJOR ),
                        QString::number( PROJ_VERSION_MINOR ),
                        QString::number( PROJ_VERSION_PATCH ) );
    if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
    {
      
                      
      if ( errMsg )
        sqlite3_free( errMsg );
      return -1;
    }
  }
  else
  {
    
    QString sql = QStringLiteral( "SELECT proj_major, proj_minor, proj_patch FROM tbl_info" );
    statement = database.prepare( sql, result );
    if ( result != SQLITE_OK )
    {
      
      return -1;
    }
    if ( statement.step() == SQLITE_ROW )
    {
      int major = statement.columnAsInt64( 0 );
      int minor = statement.columnAsInt64( 1 );
      int patch = statement.columnAsInt64( 2 );
      if ( !createdTypeColumn && major == PROJ_VERSION_MAJOR && minor == PROJ_VERSION_MINOR && patch == PROJ_VERSION_PATCH )
        
        return 0;
    }
    else
    {
      
      return -1;
    }
  }

  PJ_CONTEXT *pjContext = ProjContext::get();
  
  proj_log_func( pjContext, nullptr, sync_db_proj_logger );

  PROJ_STRING_LIST authorities = proj_get_authorities_from_database( pjContext );

  int nextSrsId = 67218;
  int nextSrId = 520007218;
  for ( auto authIter = authorities; authIter && *authIter; ++authIter )
  {
    const QString authority( *authIter );
    
    PROJ_STRING_LIST codes = proj_get_codes_from_database( pjContext, *authIter, PJ_TYPE_CRS, true );

    QStringList allCodes;

    for ( auto codesIter = codes; codesIter && *codesIter; ++codesIter )
    {
      const QString code( *codesIter );
      allCodes << SqliteUtils::quotedString( code );
      
      ProjUtils::proj_pj_unique_ptr crs( proj_create_from_database( pjContext, *authIter, *codesIter, PJ_CATEGORY_CRS, false, nullptr ) );
      if ( !crs )
      {
        
        continue;
      }

      const PJ_TYPE pjType = proj_get_type( crs.get( ) );

      QString srsTypeString;
      
      switch ( pjType )
      {
        
        case PJ_TYPE_ELLIPSOID:
        case PJ_TYPE_PRIME_MERIDIAN:
        case PJ_TYPE_GEODETIC_REFERENCE_FRAME:
        case PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME:
        case PJ_TYPE_VERTICAL_REFERENCE_FRAME:
        case PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME:
        case PJ_TYPE_DATUM_ENSEMBLE:
        case PJ_TYPE_CONVERSION:
        case PJ_TYPE_TRANSFORMATION:
        case PJ_TYPE_CONCATENATED_OPERATION:
        case PJ_TYPE_OTHER_COORDINATE_OPERATION:
       
        case PJ_TYPE_UNKNOWN:
          continue;

        case PJ_TYPE_CRS:
        case PJ_TYPE_GEOGRAPHIC_CRS:
          continue; 

        case PJ_TYPE_GEODETIC_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Geodetic );
          break;

        case PJ_TYPE_GEOCENTRIC_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Geocentric );
          break;

        case PJ_TYPE_GEOGRAPHIC_2D_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Geographic2d );
          break;

        case PJ_TYPE_GEOGRAPHIC_3D_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Geographic3d );
          break;

        case PJ_TYPE_PROJECTED_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Projected );
          break;

        case PJ_TYPE_COMPOUND_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Compound );
          break;

        case PJ_TYPE_TEMPORAL_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Temporal );
          break;

        case PJ_TYPE_ENGINEERING_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Engineering );
          break;

        case PJ_TYPE_BOUND_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Bound );
          break;

        case PJ_TYPE_VERTICAL_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Vertical );
          break;

#if PROJ_VERSION_MAJOR>9 || (PROJ_VERSION_MAJOR==9 && PROJ_VERSION_MINOR>=2)
        case PJ_TYPE_DERIVED_PROJECTED_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::DerivedProjected );
          break;
        case PJ_TYPE_COORDINATE_METADATA:
          continue;
#endif
        case PJ_TYPE_OTHER_CRS:
          srsTypeString = EnumValueToKey( ProjCore::CrsType::Other );
          break;
      }
      

      crs = ProjUtils::unboundCrs( crs.get() );

      QString proj4 = getFullProjString( crs.get() );
      proj4.replace( QLatin1String( "+type=crs" ), QString() );
      proj4 = proj4.trimmed();

      if ( proj4.isEmpty() )
      {
        
        
        proj4 = "";
      }

      
      QString operation = "";
      QString ellps = "";
      getOperationAndEllipsoidFromProjString( proj4, operation, ellps );

      const QString translatedOperation = CoordinateReferenceSystemUtils::translateProjection( operation );
      if ( translatedOperation.isEmpty() && !operation.isEmpty() )
      {
        std::cout << QStringLiteral( "Operation needs translation in QgsCoordinateReferenceSystemUtils::translateProjection: %1" ).arg( operation ).toUtf8().constData() << std::endl;
        qFatal( "aborted" );
      }

      const bool deprecated = proj_is_deprecated( crs.get() );
      const QString name( proj_get_name( crs.get() ) );

      QString sql = QStringLiteral( "SELECT parameters,description,deprecated,srs_type FROM tbl_srs WHERE auth_name='%1' AND auth_id='%2'" ).arg( authority, code );
      statement = database.prepare( sql, result );
      if ( result != SQLITE_OK )
      {
        
        continue;
      }

      QString dbSrsProj4;
      QString dbSrsDesc;
      QString dbSrsType;
      bool dbSrsDeprecated = deprecated;
      if ( statement.step() == SQLITE_ROW )
      {
        dbSrsProj4 = statement.columnAsText( 0 );
        dbSrsDesc = statement.columnAsText( 1 );
        dbSrsDeprecated = statement.columnAsText( 2 ).toInt() != 0;
        dbSrsType = statement.columnAsText( 3 );
      }

      if ( !dbSrsProj4.isEmpty() || !dbSrsDesc.isEmpty() )
      {
        if ( proj4 != dbSrsProj4 || name != dbSrsDesc || deprecated != dbSrsDeprecated || dbSrsType != srsTypeString )
        {
          errMsg = nullptr;
          sql = QStringLiteral( "UPDATE tbl_srs SET parameters=%1,description=%2,deprecated=%3, srs_type=%4 WHERE auth_name=%5 AND auth_id=%6" )
                .arg( SqliteUtils::quotedString( proj4 ) )
                .arg( SqliteUtils::quotedString( name ) )
                .arg( deprecated ? 1 : 0 )
                .arg( SqliteUtils::quotedString( srsTypeString ),
                      SqliteUtils::quotedString( authority ), SqliteUtils::quotedString( code ) );

          if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
          {
           
            if ( errMsg )
              sqlite3_free( errMsg );
            errors++;
          }
          else
          {
            updated++;
          }
        }
      }
      else
      {
        const bool isGeographic = testIsGeographic( crs.get() );

        
        const QString dbVals = sAuthIdToSrsIdMap.value( QStringLiteral( "%1:%2" ).arg( authority, code ) );
        QString srsId;
        QString srId;
        if ( !dbVals.isEmpty() )
        {
          const QStringList parts = dbVals.split( ',' );
          srsId = parts.at( 0 );
          srId = parts.at( 1 );
        }
        if ( srId.isEmpty() )
        {
          srId = QString::number( nextSrId );
          nextSrId++;
        }
        if ( srsId.isEmpty() )
        {
          srsId = QString::number( nextSrsId );
          nextSrsId++;
        }

        if ( !srsId.isEmpty() )
        {
          sql = QStringLiteral( "INSERT INTO tbl_srs(srs_id, description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated,srs_type) VALUES (%1, %2,%3,%4,%5,%6,%7,%8,%9,%10,%11)" )
                .arg( srsId )
                .arg( SqliteUtils::quotedString( name ),
                      SqliteUtils::quotedString( operation ),
                      SqliteUtils::quotedString( ellps ),
                      SqliteUtils::quotedString( proj4 ) )
                .arg( srId )
                .arg( SqliteUtils::quotedString( authority ) )
                .arg( SqliteUtils::quotedString( code ) )
                .arg( isGeographic ? 1 : 0 )
                .arg( deprecated ? 1 : 0 )
                .arg( SqliteUtils::quotedString( srsTypeString ) );
        }
        else
        {
          sql = QStringLiteral( "INSERT INTO tbl_srs(description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated,srs_type) VALUES (%1,%2,%3,%4,%5,%6,%7,%8,%9,%10)" )
                .arg( SqliteUtils::quotedString( name ),
                      SqliteUtils::quotedString( operation ),
                      SqliteUtils::quotedString( ellps ),
                      SqliteUtils::quotedString( proj4 ) )
                .arg( srId )
                .arg( SqliteUtils::quotedString( authority ) )
                .arg( SqliteUtils::quotedString( code ) )
                .arg( isGeographic ? 1 : 0 )
                .arg( deprecated ? 1 : 0 )
                .arg( SqliteUtils::quotedString( srsTypeString ) );
        }

        errMsg = nullptr;
        if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) == SQLITE_OK )
        {
          inserted++;
        }
        else
        {
          qCritical( "Could not execute: %s [%s/%s]\n",
                     sql.toUtf8().constData(),
                     sqlite3_errmsg( database.get() ),
                     errMsg ? errMsg : "(unknown error)" );
          errors++;

          if ( errMsg )
            sqlite3_free( errMsg );
        }
      }
    }

    proj_string_list_destroy( codes );

    const QString sql = QStringLiteral( "DELETE FROM tbl_srs WHERE auth_name='%1' AND NOT auth_id IN (%2)" ).arg( authority, allCodes.join( ',' ) );
    if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr ) == SQLITE_OK )
    {
      deleted = sqlite3_changes( database.get() );
    }
    else
    {
      errors++;
      qCritical( "Could not execute: %s [%s]\n",
                 sql.toUtf8().constData(),
                 sqlite3_errmsg( database.get() ) );
    }

  }
  proj_string_list_destroy( authorities );

  QString sql = QStringLiteral( "UPDATE tbl_info set proj_major=%1,proj_minor=%2,proj_patch=%3" )
                .arg( QString::number( PROJ_VERSION_MAJOR ),
                      QString::number( PROJ_VERSION_MINOR ),
                      QString::number( PROJ_VERSION_PATCH ) );
  if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
  {
   
    if ( errMsg )
      sqlite3_free( errMsg );
    return -1;
  }

  if ( sqlite3_exec( database.get(), "COMMIT", nullptr, nullptr, nullptr ) != SQLITE_OK )
  {
  
    return -1;
  }

#ifdef QGISDEBUG
  
#else
  Q_UNUSED( deleted )
#endif

  if ( errors > 0 )
    return -errors;
  else
    return updated + inserted;
}

const QHash<QString, CoordinateReferenceSystem> &CoordinateReferenceSystem::stringCache()
{
  return *sStringCache();
}

const QHash<QString, CoordinateReferenceSystem> &CoordinateReferenceSystem::projCache()
{
  return *sProj4Cache();
}

const QHash<QString, CoordinateReferenceSystem> &CoordinateReferenceSystem::ogcCache()
{
  return *sOgcCache();
}

const QHash<QString, CoordinateReferenceSystem> &CoordinateReferenceSystem::wktCache()
{
  return *sWktCache();
}



const QHash<long, CoordinateReferenceSystem> &CoordinateReferenceSystem::srsIdCache()
{
  return *sSrsIdCache();
}

CoordinateReferenceSystem CoordinateReferenceSystem::toGeographicCrs() const
{
  if ( isGeographic() )
  {
    return *this;
  }

  if ( PJ *obj = d->threadLocalProjObject() )
  {
    PJ_CONTEXT *pjContext = ProjContext::get();
    ProjUtils::proj_pj_unique_ptr geoCrs( proj_crs_get_geodetic_crs( pjContext, obj ) );
    if ( !geoCrs )
      return CoordinateReferenceSystem();

    if ( !testIsGeographic( geoCrs.get() ) )
      return CoordinateReferenceSystem();

    ProjUtils::proj_pj_unique_ptr normalized( proj_normalize_for_visualization( pjContext, geoCrs.get() ) );
    if ( !normalized )
      return CoordinateReferenceSystem();

    return CoordinateReferenceSystem::fromProjObject( normalized.get() );
  }
  else
  {
    return CoordinateReferenceSystem();
  }
}

QString CoordinateReferenceSystem::geographicCrsAuthId() const
{
  if ( isGeographic() )
  {
    return d->mAuthId;
  }
  else if ( PJ *obj = d->threadLocalProjObject() )
  {
    ProjUtils::proj_pj_unique_ptr geoCrs( proj_crs_get_geodetic_crs( ProjContext::get(), obj ) );
    return geoCrs ? QStringLiteral( "%1:%2" ).arg( proj_get_id_auth_name( geoCrs.get(), 0 ), proj_get_id_code( geoCrs.get(), 0 ) ) : QString();
  }
  else
  {
    return QString();
  }
}

PJ *CoordinateReferenceSystem::projObject() const
{
  return d->threadLocalProjObject();
}

CoordinateReferenceSystem CoordinateReferenceSystem::fromProjObject( PJ *object )
{
  CoordinateReferenceSystem crs;
  crs.createFromProjObject( object );
  return crs;
}

bool CoordinateReferenceSystem::createFromProjObject( PJ *object )
{
  d.detach();
  d->mIsValid = false;
  d->mProj4.clear();
  d->mWktPreferred.clear();

  if ( !object )
  {
    return false;
  }

  switch ( proj_get_type( object ) )
  {
    case PJ_TYPE_GEODETIC_CRS:
    case PJ_TYPE_GEOCENTRIC_CRS:
    case PJ_TYPE_GEOGRAPHIC_CRS:
    case PJ_TYPE_GEOGRAPHIC_2D_CRS:
    case PJ_TYPE_GEOGRAPHIC_3D_CRS:
    case PJ_TYPE_VERTICAL_CRS:
    case PJ_TYPE_PROJECTED_CRS:
    case PJ_TYPE_COMPOUND_CRS:
    case PJ_TYPE_TEMPORAL_CRS:
    case PJ_TYPE_ENGINEERING_CRS:
    case PJ_TYPE_BOUND_CRS:
    case PJ_TYPE_OTHER_CRS:
      break;

    default:
      return false;
  }

  d->setPj( ProjUtils::unboundCrs( object ) );

  if ( !d->hasPj() )
  {
    return d->mIsValid;
  }
  else
  {
    
    const QString authName( proj_get_id_auth_name( d->threadLocalProjObject(), 0 ) );
    const QString authCode( proj_get_id_code( d->threadLocalProjObject(), 0 ) );
    if ( !authName.isEmpty() && !authCode.isEmpty() && loadFromAuthCode( authName, authCode ) )
    {
      return d->mIsValid;
    }
    else
    {
      
      d->mIsValid = true;
      d->mDescription = QString( proj_get_name( d->threadLocalProjObject() ) );
      setMapUnits();
      d->mIsGeographic = testIsGeographic( d->threadLocalProjObject() );
    }
  }

  return d->mIsValid;
}

QStringList CoordinateReferenceSystem::recentProjections()
{
  QStringList projections;
  const QList<CoordinateReferenceSystem> res =QProj::coordinateReferenceSystemRegistry()->recentCrs();
  projections.reserve( res.size() );
  for ( const CoordinateReferenceSystem &crs : res )
  {
    projections << QString::number( crs.srsid() );
  }
  return projections;
}

QList<CoordinateReferenceSystem> CoordinateReferenceSystem::recentCoordinateReferenceSystems()
{
  
    return QProj::coordinateReferenceSystemRegistry()->recentCrs();
}

void CoordinateReferenceSystem::pushRecentCoordinateReferenceSystem( const CoordinateReferenceSystem &crs )
{
   QProj::coordinateReferenceSystemRegistry()->pushRecent( crs );
}
void CoordinateReferenceSystem::InsertRecentCoordinateReferenceSystem(const CoordinateReferenceSystem& crs)
{
    QProj::coordinateReferenceSystemRegistry()->InsertRecent(crs);
}
void CoordinateReferenceSystem::removeRecentCoordinateReferenceSystem( const CoordinateReferenceSystem &crs )
{
   QProj::coordinateReferenceSystemRegistry()->removeRecent( crs );
}

void CoordinateReferenceSystem::clearRecentCoordinateReferenceSystems()
{
   QProj::coordinateReferenceSystemRegistry()->clearRecent();
}

void CoordinateReferenceSystem::invalidateCache( bool disableCache )
{
  

  sOgcLock()->lockForWrite();
  if ( !sDisableOgcCache )
  {
    if ( disableCache )
      sDisableOgcCache = true;
    sOgcCache()->clear();
  }
  sOgcLock()->unlock();

  sProj4CacheLock()->lockForWrite();
  if ( !sDisableProjCache )
  {
    if ( disableCache )
      sDisableProjCache = true;
    sProj4Cache()->clear();
  }
  sProj4CacheLock()->unlock();

  sCRSWktLock()->lockForWrite();
  if ( !sDisableWktCache )
  {
    if ( disableCache )
      sDisableWktCache = true;
    sWktCache()->clear();
  }
  sCRSWktLock()->unlock();

  sCRSSrsIdLock()->lockForWrite();
  if ( !sDisableSrsIdCache )
  {
    if ( disableCache )
      sDisableSrsIdCache = true;
    sSrsIdCache()->clear();
  }
  sCRSSrsIdLock()->unlock();

  sCrsStringLock()->lockForWrite();
  if ( !sDisableStringCache )
  {
    if ( disableCache )
      sDisableStringCache = true;
    sStringCache()->clear();
  }
  sCrsStringLock()->unlock();
}


bool operator> ( const CoordinateReferenceSystem &c1, const CoordinateReferenceSystem &c2 )
{
  if ( c1.d == c2.d )
    return false;

  if ( !c1.d->mIsValid && !c2.d->mIsValid )
    return false;

  if ( !c1.d->mIsValid && c2.d->mIsValid )
    return false;

  if ( c1.d->mIsValid && !c2.d->mIsValid )
    return true;

  const bool c1IsUser = c1.d->mSrsId >= USER_CRS_START_ID;
  const bool c2IsUser = c2.d->mSrsId >= USER_CRS_START_ID;

  if ( c1IsUser && !c2IsUser )
    return true;

  if ( !c1IsUser && c2IsUser )
    return false;

  if ( !c1IsUser && !c2IsUser && !c1.d->mAuthId.isEmpty() && !c2.d->mAuthId.isEmpty() )
  {
    if ( c1.d->mAuthId != c2.d->mAuthId )
      return c1.d->mAuthId > c2.d->mAuthId;
  }

  const QString wkt1 = c1.toWkt( ProjCore::CrsWktVariant::Preferred );
  const QString wkt2 = c2.toWkt( ProjCore::CrsWktVariant::Preferred );
  if ( wkt1 != wkt2 )
    return wkt1 > wkt2;

  if ( c1.d->mCoordinateEpoch == c2.d->mCoordinateEpoch )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && !std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( !std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return true;

  return c1.d->mCoordinateEpoch > c2.d->mCoordinateEpoch;
}

bool operator< ( const CoordinateReferenceSystem &c1, const CoordinateReferenceSystem &c2 )
{
  if ( c1.d == c2.d )
    return false;

  if ( !c1.d->mIsValid && !c2.d->mIsValid )
    return false;

  if ( c1.d->mIsValid && !c2.d->mIsValid )
    return false;

  if ( !c1.d->mIsValid && c2.d->mIsValid )
    return true;

  const bool c1IsUser = c1.d->mSrsId >= USER_CRS_START_ID;
  const bool c2IsUser = c2.d->mSrsId >= USER_CRS_START_ID;

  if ( !c1IsUser && c2IsUser )
    return true;

  if ( c1IsUser && !c2IsUser )
    return false;

  if ( !c1IsUser && !c2IsUser && !c1.d->mAuthId.isEmpty() && !c2.d->mAuthId.isEmpty() )
  {
    if ( c1.d->mAuthId != c2.d->mAuthId )
      return c1.d->mAuthId < c2.d->mAuthId;
  }

  const QString wkt1 = c1.toWkt( ProjCore::CrsWktVariant::Preferred );
  const QString wkt2 = c2.toWkt( ProjCore::CrsWktVariant::Preferred );
  if ( wkt1 != wkt2 )
    return wkt1 < wkt2;

  if ( c1.d->mCoordinateEpoch == c2.d->mCoordinateEpoch )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( !std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && !std::isnan( c2.d->mCoordinateEpoch ) )
    return true;

  return c1.d->mCoordinateEpoch < c2.d->mCoordinateEpoch;
}

bool operator>= ( const CoordinateReferenceSystem &c1, const CoordinateReferenceSystem &c2 )
{
  return !( c1 < c2 );
}
bool operator<= ( const CoordinateReferenceSystem &c1, const CoordinateReferenceSystem &c2 )
{
  return !( c1 > c2 );
}



void CoordinateReferenceSystem::TestValidSrsIds()
{
    const QList< long > ids = CoordinateReferenceSystem::validSrsIds();
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}

}
}