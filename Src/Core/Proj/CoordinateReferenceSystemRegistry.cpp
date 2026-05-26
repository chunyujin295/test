


#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/Proj/CoordinateReferenceSystem_p.h"

#include "Core/Proj/CoordinateReferenceSystemRegistry.h"


#include "Core/Proj/CoordinateTransform.h"
#include "Core/Proj/QProj.h"
#include "Core/Proj/SqliteUtils.h"
#include "Core/Proj/ProjUtils.h"
#include "Core/Proj/Exception.h"
#include "Core/Proj/ProjOperation.h"
#include <QSettings>

#include <QFileInfo>
#include <sqlite3.h>
#include <mutex>
#include <proj.h>
#include "Core/Application.h"
#include "Core/Logging.h"
#ifdef USE_AI3D_PROJ
#include "Util/Settings.h"
#include "Core/Proj/CrsSettings.h"
#endif
#define  MAXCRSITEM 30
namespace AI3D
{
    namespace PROJ
    {
        CoordinateReferenceSystemRegistry::CoordinateReferenceSystemRegistry(QObject* parent)
            : QObject(parent)
        {

        }

        CoordinateReferenceSystemRegistry::~CoordinateReferenceSystemRegistry() = default;

        QList<CoordinateReferenceSystemRegistry::UserCrsDetails> CoordinateReferenceSystemRegistry::userCrsList() const
        {
            QList<CoordinateReferenceSystemRegistry::UserCrsDetails> res;

            
            sqlite3_database_unique_ptr database;
            
            int result = database.open_v2(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()), SQLITE_OPEN_READONLY, nullptr);
            if (result != SQLITE_OK)
            {
                LOGI("Can't open database: " + database.errorMessage().toStdString());
                return res;
            }

            QString sql = QStringLiteral("select srs_id,description,parameters, wkt from tbl_srs");

            LOGI("Query to populate existing list: " + sql.toStdString());
#if 1
            sqlite3_statement_unique_ptr preparedStatement = database.prepare(sql, result);
            if (result == SQLITE_OK)
            {
                const CoordinateReferenceSystem crs;
                while (preparedStatement.step() == SQLITE_ROW)
                {
                    UserCrsDetails details;
                    details.id = preparedStatement.columnAsText(0).toLong();
                    details.name = preparedStatement.columnAsText(1);
                    details.proj = preparedStatement.columnAsText(2);
                    details.wkt = preparedStatement.columnAsText(3);
                    
                    

                    
                    {
                        
                    }

                    if (!details.wkt.isEmpty())
                        details.crs.createFromWkt(details.wkt);
                    else
                        details.crs.createFromProj(details.proj);

                    res << details;
                }
            }
#endif


            return res;
        }

        long CoordinateReferenceSystemRegistry::addUserCrs(const CoordinateReferenceSystem& crs, const QString& name, ProjCore::CrsDefinitionFormat nativeFormat)
        {
            if (!crs.isValid())
            {
                LOGI("Can't save an invalid CRS!");
                return -1;
            }

            QString mySql;

            QString proj4String = crs.d->mProj4;
            if (proj4String.isEmpty())
            {
                proj4String = crs.toProj();
            }
            const QString wktString = crs.toWkt(ProjCore::CrsWktVariant::Preferred);

            
            
            const QString quotedEllipsoidString = crs.ellipsoidAcronym().isNull() ? QStringLiteral("''") : SqliteUtils::quotedString(crs.ellipsoidAcronym());

            
            
            
            
            if (CoordinateReferenceSystem::getRecordCount() == 0)
            {
                mySql = "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
                    + QString::number(USER_CRS_START_ID)
                    + ',' + SqliteUtils::quotedString(name)
                    + ',' + (!crs.d->mProjectionAcronym.isEmpty() ? SqliteUtils::quotedString(crs.d->mProjectionAcronym) : QStringLiteral("''"))
                    + ',' + quotedEllipsoidString
                    + ',' + (!proj4String.isEmpty() ? SqliteUtils::quotedString(proj4String) : QStringLiteral("''"))
                    + ",0,"  
                    + (nativeFormat == ProjCore::CrsDefinitionFormat::Wkt ? SqliteUtils::quotedString(wktString) : QStringLiteral("''"))
                    + ')';
            }
            else
            {
                mySql = "insert into tbl_srs (description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
                    + SqliteUtils::quotedString(name)
                    + ',' + (!crs.d->mProjectionAcronym.isEmpty() ? SqliteUtils::quotedString(crs.d->mProjectionAcronym) : QStringLiteral("''"))
                    + ',' + quotedEllipsoidString
                    + ',' + (!proj4String.isEmpty() ? SqliteUtils::quotedString(proj4String) : QStringLiteral("''"))
                    + ",0,"  
                    + (nativeFormat == ProjCore::CrsDefinitionFormat::Wkt ? SqliteUtils::quotedString(wktString) : QStringLiteral("''"))
                    + ')';
            }



            sqlite3_database_unique_ptr database;
            sqlite3_statement_unique_ptr statement;
            

            int myResult = database.open(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()));
            if (myResult != SQLITE_OK)
            {
                LOGI("Can't open or create database :" +
                    (AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath() +
                        database.errorMessage().toStdString()));
                return false;
            }
            statement = database.prepare(mySql, myResult);

            qint64 returnId = -1;
            if (myResult == SQLITE_OK && statement.step() == SQLITE_DONE)
            {
                LOGI("Saved user CRS " + crs.toProj().toStdString());

                returnId = sqlite3_last_insert_rowid(database.get());
                crs.d->mSrsId = returnId;
                crs.d->mAuthId = QStringLiteral("USER:%1").arg(returnId);
                crs.d->mDescription = name;
            }

            if (returnId != -1)
            {
                
                
                
                insertProjection(crs.projectionAcronym());
            }

            CoordinateReferenceSystem::invalidateCache();
            CoordinateTransform::invalidateCache();

            if (returnId != -1)
            {
                QString  crsstr = crs.d->mAuthId;
                crsstr.toUpper();
               
                
                emit userCrsAdded(crs.d->mAuthId);
                emit crsDefinitionsChanged();
            }

            return returnId;
        }

        bool CoordinateReferenceSystemRegistry::updateUserCrs(long id, const CoordinateReferenceSystem& crs, const QString& name, ProjCore::CrsDefinitionFormat nativeFormat)
        {
            if (!crs.isValid())
            {
                LOGI(("Can't save an invalid CRS!"));
                return false;
            }

            const QString sql = "update tbl_srs set description="
                + SqliteUtils::quotedString(name)
                + ",projection_acronym=" + (!crs.projectionAcronym().isEmpty() ? SqliteUtils::quotedString(crs.projectionAcronym()) : QStringLiteral("''"))
                + ",ellipsoid_acronym=" + (!crs.ellipsoidAcronym().isEmpty() ? SqliteUtils::quotedString(crs.ellipsoidAcronym()) : QStringLiteral("''"))
                + ",parameters=" + (!crs.toProj().isEmpty() ? SqliteUtils::quotedString(crs.toProj()) : QStringLiteral("''"))
                + ",is_geo=0" 
                + ",wkt=" + (nativeFormat == ProjCore::CrsDefinitionFormat::Wkt ? SqliteUtils::quotedString(crs.toWkt(ProjCore::CrsWktVariant::Preferred, false)) : QStringLiteral("''"))
                + " where srs_id=" + SqliteUtils::quotedString(QString::number(id))
                ;

            sqlite3_database_unique_ptr database;
            
            const int myResult = database.open(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()));
            if (myResult != SQLITE_OK)
            {
                
                return false;
            }

            bool res = true;
            QString errorMessage;
            if (database.exec(sql, errorMessage) != SQLITE_OK)
            {
                
                res = false;
            }
            else
            {
                const int changed = sqlite3_changes(database.get());
                if (changed)
                {
                    
                }
                else
                {
                    
                    res = false;
                }
            }

            if (res)
            {
                
                
                
                insertProjection(crs.projectionAcronym());
            }

            CoordinateReferenceSystem::invalidateCache();
            CoordinateTransform::invalidateCache();

            if (res)
            {
                
                emit userCrsChanged(QStringLiteral("USER:%1").arg(id));
                emit crsDefinitionsChanged();
            }

            return res;
        }

        bool CoordinateReferenceSystemRegistry::removeUserCrs(long id)
        {
            sqlite3_database_unique_ptr database;

            const QString sql = "delete from tbl_srs where srs_id=" + SqliteUtils::quotedString(QString::number(id));
            LOGI(sql.toStdString());
            
            int result = database.open(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()));
            if (result != SQLITE_OK)
            {
                LOGI("Can't open database:");
                
                return false;
            }

            bool res = true;
            {
                sqlite3_statement_unique_ptr preparedStatement = database.prepare(sql, result);
                if (result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE)
                {
                    LOGI("failed to remove custom CRS from database:");
                    
                    res = false;
                }
                else
                {
                    const int changed = sqlite3_changes(database.get());
                    if (changed)
                    {
                        LOGI("Removed user CRS");
                        
                    }
                    else
                    {
                        LOGI("Error removing user CRS : No matching ID found in database");
                        
                        res = false;
                    }
                }
            }

            CoordinateReferenceSystem::invalidateCache();
            CoordinateTransform::invalidateCache();

            if (res)
            {
                emit userCrsRemoved(id);
                emit crsDefinitionsChanged();
            }

            return res;
        }


        bool CoordinateReferenceSystemRegistry::insertProjection(const QString& projectionAcronym)
        {
            sqlite3_database_unique_ptr database;
            sqlite3_database_unique_ptr srsDatabase;
            QString sql;
            

            int result = database.open(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()));
            if (result != SQLITE_OK)
            {

                
                
                return false;
            }

            int srsResult = srsDatabase.open(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath()));
            if (result != SQLITE_OK)
            {
                
               
                return false;
            }

            
            const QString srsSql = "select acronym,name,notes,parameters from tbl_projection where acronym=" + SqliteUtils::quotedString(projectionAcronym);

            sqlite3_statement_unique_ptr srsPreparedStatement = srsDatabase.prepare(srsSql, srsResult);
            if (srsResult == SQLITE_OK)
            {
                if (srsPreparedStatement.step() == SQLITE_ROW)
                {
                    
                     
                    sql = "insert into tbl_projection(acronym,name,notes,parameters) values ("
                        + SqliteUtils::quotedString(srsPreparedStatement.columnAsText(0))
                        + ',' + SqliteUtils::quotedString(srsPreparedStatement.columnAsText(1))
                        + ',' + SqliteUtils::quotedString(srsPreparedStatement.columnAsText(2))
                        + ',' + SqliteUtils::quotedString(srsPreparedStatement.columnAsText(3))
                        + ')';
                    sqlite3_statement_unique_ptr preparedStatement = database.prepare(sql, result);
                    if (result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE)
                    {
                        
                        return false;
                    }
                }
            }
            else
            {
                
                return false;
            }

            return true;
        }

        QMap<QString, ProjOperation> CoordinateReferenceSystemRegistry::projOperations() const
        {
            static std::once_flag initialized;
            std::call_once(initialized, [this]
                {


                    const PJ_OPERATIONS* operation = proj_list_operations();
                    while (operation && operation->id)
                    {
                        ProjOperation value;
                        value.mValid = true;
                        value.mId = QString(operation->id);

                        const QString description(*operation->descr);
                        const QStringList descriptionParts = description.split(QStringLiteral("\n\t"));
                        value.mDescription = descriptionParts.value(0);
                        value.mDetails = descriptionParts.mid(1).join('\n');

                        mProjOperations.insert(value.id(), value);

                        operation++;
                    }
                });

            return mProjOperations;
        }

        
        QSet<QString> CoordinateReferenceSystemRegistry::authorities() const
        {
            static std::once_flag initialized;
            std::call_once(initialized, [this]
                {


                    PJ_CONTEXT* pjContext = ProjContext::get();
                    PROJ_STRING_LIST authorities = proj_get_authorities_from_database(pjContext);

                    for (auto authIter = authorities; authIter && *authIter; ++authIter)
                    {
                        const QString authority(*authIter);
                        mKnownAuthorities.insert(authority.toLower());
                    }

                    proj_string_list_destroy(authorities);
                });

            return mKnownAuthorities;
        }

        QList<CrsDbRecord> CoordinateReferenceSystemRegistry::crsDbRecords() const
        {
            static std::once_flag initialized;
            std::call_once(initialized, [this]
                {
                    const QString srsDatabaseFileName = QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjInnerSrsFullPath());
                    if (QFileInfo::exists(srsDatabaseFileName))
                    {
                        
                        sqlite3_database_unique_ptr database;
                        int result = database.open_v2(srsDatabaseFileName, SQLITE_OPEN_READONLY, nullptr);
                        if (result == SQLITE_OK)
                        {
                            const QString sql = QStringLiteral("SELECT description, srs_id, auth_name, auth_id, projection_acronym, deprecated, srs_type FROM tbl_srs");
                            sqlite3_statement_unique_ptr preparedStatement = database.prepare(sql, result);
                            if (result == SQLITE_OK)
                            {
                                while (preparedStatement.step() == SQLITE_ROW)
                                {
                                    CrsDbRecord record;
                                    record.description = preparedStatement.columnAsText(0);
                                    record.srsId = preparedStatement.columnAsText(1);
                                    record.authName = preparedStatement.columnAsText(2);
                                    record.authId = preparedStatement.columnAsText(3);
                                    record.projectionAcronym = preparedStatement.columnAsText(4);
                                    record.deprecated = preparedStatement.columnAsText(5).toInt();
                                    record.type = EnumKeyToValue(preparedStatement.columnAsText(6), ProjCore::CrsType::Unknown);

                                    if (record.authName != QLatin1String("EPSG"))
                                    {
                                        
                                        continue;
                                    }
                                    if (record.type == ProjCore::CrsType::Compound)
                                    {
                                        
                                        QString recordstr = record.authId;
                                        recordstr.toUpper();
                                        if (recordstr.contains("5573"))
                                        {
                                            std::cout << " -* -----------*************** " << recordstr.toStdString() << std::endl;
                                        }
                                        continue;
                                    }
                                    if (record.type == ProjCore::CrsType::Geographic3d)
                                    {
                                        
                                        continue;
                                    }


                                    if (record.type == ProjCore::CrsType::Geocentric)
                                    {

                                        std::string idstr = record.authId.QString::toStdString();
                                        int id = std::atoi(idstr.c_str());
                                        if (id != 4978)
                                        {
                                            

                                            continue;
                                        }
                                        
                                        
                                            
                                        
                                       
                                    }
                                    mCrsDbRecords.append(record);

                                }
                            }
                        }
                    }
                });

            return mCrsDbRecords;
        }

      
        QList<CoordinateReferenceSystem> CoordinateReferenceSystemRegistry::GetRecentCrs()
        {
            QList<CoordinateReferenceSystem> res;

          
            CrsSettings settings;
            QStringList projectionsAuthId = CrsSettings::GetRecentCrs();;
          
            int max = projectionsAuthId.size();
            res.reserve(max);
            for (int i = 0; i < max; ++i)
            {
                const QString recentid = projectionsAuthId.value(i);
               

                CoordinateReferenceSystem crs;
                if (!recentid.isEmpty())
                {
                    crs = CoordinateReferenceSystem(recentid);
                }
               
                

                if (crs.isValid())
                {

                    res << crs;
                }
            }
            return res;
        }

        void CoordinateReferenceSystemRegistry::PushIntoRecent(const CoordinateReferenceSystem& crs)
        {

            
            if ( !crs.isValid())
                return;

          

            QStringList recent2 = CrsSettings::GetRecentCrs();
            for (const QString& c : std::as_const(recent2))
            {
              
              
            }
         
            QString currentdef = QString::fromStdString(crs.GetAuthID());
            recent2.removeAll(currentdef);
            for (const QString& c : std::as_const(recent2))
            {
            
            
            }
            
            recent2.insert(0, currentdef);
            for (const QString& c : std::as_const(recent2))
            {
            
            
            }
           
            
          
            
            
            const QList<QString> toTrim = recent2.mid(MAXCRSITEM);
            
            for (const QString& crsTrimmed : toTrim)
            {
                recent2.removeOne(crsTrimmed);
               
            }

            QStringList authids;
            authids.reserve(recent2.size());
            
            for (const QString& c : std::as_const(recent2))
            {

                authids << c;
              
            }
            CrsSettings::pCRSSettings->setValue(QStringLiteral("Crs/recentAuthId"), authids);
            for (const QString& c : (CrsSettings::GetRecentCrs()))
            {
            
            
            }
            
        }

        QList<CoordinateReferenceSystem> CoordinateReferenceSystemRegistry::recentCrs()
        {
            QList<CoordinateReferenceSystem> res;

            
            std::string globalsettingsfile = AI3D::CORE::Application::Getinstance().GetAPPPath() + "/crsconfig.ini";
            globalsettingsfile = AI3D::CORE::File::EnsureUnifySlash(globalsettingsfile);
            QString qglobalsettingsfile = QString::fromStdString(globalsettingsfile);            
            QSettings settings(qglobalsettingsfile, QSettings::IniFormat);
            QStringList projectionsProj4 = settings.value(QStringLiteral("UI/recentProjectionsProj4")).toStringList();
            QStringList projectionsWkt = settings.value(QStringLiteral("UI/recentProjectionsWkt")).toStringList();
            QStringList projectionsAuthId = settings.value(QStringLiteral("UI/recentProjectionsAuthId")).toStringList();
            
            int max = std::max(projectionsAuthId.size(), std::max(projectionsProj4.size(), projectionsWkt.size()));
            res.reserve(max);
            for (int i = 0; i < max; ++i)
            {
                const QString proj = projectionsProj4.value(i);
                const QString wkt = projectionsWkt.value(i);
                const QString authid = projectionsAuthId.value(i);

                CoordinateReferenceSystem crs;
                if (!authid.isEmpty())
                {
                    crs = CoordinateReferenceSystem(authid);
                }
                if (!crs.isValid() && !wkt.isEmpty())
                {
                    crs.createFromWkt(wkt);
                }
                if (!crs.isValid() && !proj.isEmpty())
                {
                    crs.createFromProj(wkt);
                }

                    
                if (crs.isValid())
                {
                    
                    res << crs;
                }
            }
            return res;
        }

        void CoordinateReferenceSystemRegistry::clearRecent()
        {
            std::string globalsettingsfile = AI3D::CORE::Application::Getinstance().GetAPPPath() + "/crsconfig.ini";
            globalsettingsfile = AI3D::CORE::File::EnsureUnifySlash(globalsettingsfile);
            QString qglobalsettingsfile = QString::fromStdString(globalsettingsfile);
            QSettings settings(qglobalsettingsfile, QSettings::IniFormat);
            settings.remove(QStringLiteral("UI/recentProjectionsAuthId"));
            settings.remove(QStringLiteral("UI/recentProjectionsWkt"));
            settings.remove(QStringLiteral("UI/recentProjectionsProj4"));

            emit recentCrsCleared();
        }
        
        void CoordinateReferenceSystemRegistry::InsertRecent(const CoordinateReferenceSystem& crs)
        {
            PushIntoRecent(crs);
            
            
        }
        
        long CoordinateReferenceSystemRegistry::AddUserCrsToDatabase( CoordinateReferenceSystem& crs, 
            const QString& name, ProjCore::CrsDefinitionFormat nativeFormat)
        {
            if (!crs.isValid())
            {
                LOGI("Can't save an invalid CRS!");
                return false;
            }

            QString mySql;

            QString proj4String = crs.d->mProj4;
            if (proj4String.isEmpty())
            {
                proj4String = crs.toProj();
            }
            const QString wktString = crs.toWkt(ProjCore::CrsWktVariant::Preferred);

            
            
            const QString quotedEllipsoidString = crs.ellipsoidAcronym().isNull() ? QStringLiteral("''") : SqliteUtils::quotedString(crs.ellipsoidAcronym());

            
            
            
            
            if (CoordinateReferenceSystem::getRecordCount() != 0)
            {
                std::string userdatabasefile = AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath();

               long id =  CoordinateReferenceSystem::GetUSERCrsID(QString::fromStdString(userdatabasefile));
                mySql = "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
                    + QString::number(id)
                    + ',' + SqliteUtils::quotedString(name)
                    + ',' + (!crs.d->mProjectionAcronym.isEmpty() ? SqliteUtils::quotedString(crs.d->mProjectionAcronym) : QStringLiteral("''"))
                    + ',' + quotedEllipsoidString
                    + ',' + (!proj4String.isEmpty() ? SqliteUtils::quotedString(proj4String) : QStringLiteral("''"))
                    + ",0,"  
                    + (nativeFormat == ProjCore::CrsDefinitionFormat::Wkt ? SqliteUtils::quotedString(wktString) : QStringLiteral("''"))
                    + ')';
            }
            else
            
                mySql = "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
                    + QString::number(USER_CRS_START_ID)
                    + ',' + SqliteUtils::quotedString(name)
                    + ',' + (!crs.d->mProjectionAcronym.isEmpty() ? SqliteUtils::quotedString(crs.d->mProjectionAcronym) : QStringLiteral("''"))
                    + ',' + quotedEllipsoidString
                    + ',' + (!proj4String.isEmpty() ? SqliteUtils::quotedString(proj4String) : QStringLiteral("''"))
                    + ",0,"  
                    + (nativeFormat == ProjCore::CrsDefinitionFormat::Wkt ? SqliteUtils::quotedString(wktString) : QStringLiteral("''"))
                    + ')';
            
            



            sqlite3_database_unique_ptr database;
            sqlite3_statement_unique_ptr statement;
            

            int myResult = database.open(QString::fromStdString(AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath()));
            if (myResult != SQLITE_OK)
            {
                LOGI("Can't open or create database :" +
                    (AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath() +
                        database.errorMessage().toStdString()));
                return false;
            }
            statement = database.prepare(mySql, myResult);

            qint64 returnId = -1;
            if (myResult == SQLITE_OK && statement.step() == SQLITE_DONE)
            {
                LOGI("Saved user CRS " + crs.toProj().toStdString());

                returnId = sqlite3_last_insert_rowid(database.get());
                crs.d->mSrsId = returnId;
                crs.d->mAuthId = QStringLiteral("USER:%1").arg(returnId);
                crs.d->mDescription = name;
            }

            if (returnId != -1)
            {
                
                
                
                insertProjection(crs.projectionAcronym());
            }

            CoordinateReferenceSystem::invalidateCache();
            CoordinateTransform::invalidateCache();

            if (returnId != -1)
            {
                emit userCrsAdded(crs.d->mAuthId);
                emit crsDefinitionsChanged();
            }

            return (long)returnId;
        }
        void CoordinateReferenceSystemRegistry::pushRecent(const CoordinateReferenceSystem& crs)
        {
            
            if (crs.srsid() == 0 || !crs.isValid())
                return;

            QList<CoordinateReferenceSystem> recent = recentCrs();
            recent.removeAll(crs);
            recent.insert(0, crs);

            auto hasVertical = [](const CoordinateReferenceSystem& crs)
            {
                switch (crs.type())
                {
                case ProjCore::CrsType::Unknown:
                case ProjCore::CrsType::Geodetic:
                case ProjCore::CrsType::Geocentric:
                case ProjCore::CrsType::Geographic2d:
                case ProjCore::CrsType::Projected:
                case ProjCore::CrsType::Temporal:
                case ProjCore::CrsType::Engineering:
                case ProjCore::CrsType::Bound:
                case ProjCore::CrsType::Other:
                case ProjCore::CrsType::DerivedProjected:
                    return false;

                case ProjCore::CrsType::Geographic3d:
                case ProjCore::CrsType::Vertical:
                case ProjCore::CrsType::Compound:
                    return true;
                }
                BUILTIN_UNREACHABLE
            };

            QList<CoordinateReferenceSystem> recentSameType;
            std::copy_if(recent.begin(), recent.end(), std::back_inserter(recentSameType), [crs, &hasVertical](const CoordinateReferenceSystem& it)
                {
                    return hasVertical(it) == hasVertical(crs);
                });

            
            const QList<CoordinateReferenceSystem> toTrim = recentSameType.mid(MAXCRSITEM);

            for (const CoordinateReferenceSystem& crsTrimmed : toTrim)
            {
                recent.removeOne(crsTrimmed);
                emit recentCrsRemoved(crsTrimmed);
            }

            QStringList authids;
            authids.reserve(recent.size());
            QStringList proj;
            proj.reserve(recent.size());
            QStringList wkt;
            wkt.reserve(recent.size());
            for (const CoordinateReferenceSystem& c : std::as_const(recent))
            {
                authids << c.authid();
                proj << c.toProj();
                wkt << c.toWkt(ProjCore::CrsWktVariant::Preferred);
                
                
            }
           
          
            CrsSettings settings;
            settings.setValue(QStringLiteral("UI/recentProjectionsAuthId"), authids);
            settings.setValue(QStringLiteral("UI/recentProjectionsWkt"), wkt);
            settings.setValue(QStringLiteral("UI/recentProjectionsProj4"), proj);

            emit recentCrsPushed(crs);
        }

        void CoordinateReferenceSystemRegistry::removeRecent(const CoordinateReferenceSystem& crs)
        {
            if (crs.srsid() == 0 || !crs.isValid())
                return;

            QList<CoordinateReferenceSystem> recent = recentCrs();
            recent.removeAll(crs);
            QStringList authids;
            authids.reserve(recent.size());
            QStringList proj;
            proj.reserve(recent.size());
            QStringList wkt;
            wkt.reserve(recent.size());
            for (const CoordinateReferenceSystem& c : std::as_const(recent))
            {
                authids << c.authid();
                proj << c.toProj();
                wkt << c.toWkt(ProjCore::CrsWktVariant::Preferred);
            }
            std::string globalsettingsfile = AI3D::CORE::Application::Getinstance().GetAPPPath() + "/crsconfig.ini";
            globalsettingsfile = AI3D::CORE::File::EnsureUnifySlash(globalsettingsfile);
            QString qglobalsettingsfile = QString::fromStdString(globalsettingsfile);
            QSettings settings(qglobalsettingsfile, QSettings::IniFormat);
            settings.setValue(QStringLiteral("UI/recentProjectionsAuthId"), authids);
            settings.setValue(QStringLiteral("UI/recentProjectionsWkt"), wkt);
            settings.setValue(QStringLiteral("UI/recentProjectionsProj4"), proj);

            emit recentCrsRemoved(crs);
        }
    }
}