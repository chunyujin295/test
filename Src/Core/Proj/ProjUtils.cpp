


#include "Core/Proj/ProjUtils.h"

#include "Core/Proj/CoordinateTransform.h"
#include "Core/Proj/Exception.h"
#include <QString>
#include <QSet>
#include <QRegularExpression>
#include <QDate>
#include "Core/Proj/CoordinateReferenceSystem.h"
#include <proj.h>
#include "Core/Proj/ReadWriteLocker.h"
namespace AI3D
{
    namespace PROJ
    {
#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
        thread_local ProjContext ProjContext::sProjContext;
#else
        QThreadStorage< ProjContext* > ProjContext::sProjContext;
#endif

        ProjContext::ProjContext()
        {
            mContext = proj_context_create();
        }

        ProjContext::~ProjContext()
        {
            
            
            CoordinateTransform::removeFromCacheObjectsBelongingToCurrentThread(mContext);
            CoordinateReferenceSystem::removeFromCacheObjectsBelongingToCurrentThread(mContext);
            proj_context_destroy(mContext);
        }

        PJ_CONTEXT* ProjContext::get()
        {
#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
            return sProjContext.mContext;
#else
            PJ_CONTEXT* pContext = nullptr;
            if (sProjContext.hasLocalData())
            {
                pContext = sProjContext.localData()->mContext;
            }
            else
            {
                sProjContext.setLocalData(new ProjContext());
                pContext = sProjContext.localData()->mContext;
            }
            return pContext;
#endif
        }

        void ProjUtils::ProjPJDeleter::operator()(PJ* object) const
        {
            proj_destroy(object);
        }

        bool ProjUtils::usesAngularUnit(const QString& projDef)
        {
            const QString crsDef = QStringLiteral("%1 +type=crs").arg(projDef);
            PJ_CONTEXT* context = ProjContext::get();
            ProjUtils::proj_pj_unique_ptr projSingleOperation(proj_create(context, crsDef.toUtf8().constData()));
            if (!projSingleOperation)
                return false;

            ProjUtils::proj_pj_unique_ptr coordinateSystem(proj_crs_get_coordinate_system(context, projSingleOperation.get()));
            if (!coordinateSystem)
                return false;

            const int axisCount = proj_cs_get_axis_count(context, coordinateSystem.get());
            if (axisCount > 0)
            {
                const char* outUnitAuthName = nullptr;
                const char* outUnitAuthCode = nullptr;
                
                proj_cs_get_axis_info(context, coordinateSystem.get(), 0,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    &outUnitAuthName,
                    &outUnitAuthCode);

                if (outUnitAuthName && outUnitAuthCode)
                {
                    const char* unitCategory = nullptr;
                    if (proj_uom_get_info_from_database(context, outUnitAuthName, outUnitAuthCode, nullptr, nullptr, &unitCategory))
                    {
                        return QString(unitCategory).compare(QLatin1String("angular"), Qt::CaseInsensitive) == 0;
                    }
                }
            }
            return false;
        }

        bool ProjUtils::axisOrderIsSwapped(const PJ* crs)
        {
            
            if (!crs)
                return false;

            PJ_CONTEXT* context = ProjContext::get();
            ProjUtils::proj_pj_unique_ptr pjCs(proj_crs_get_coordinate_system(context, crs));
            if (!pjCs)
                return false;

            const int axisCount = proj_cs_get_axis_count(context, pjCs.get());
            if (axisCount > 0)
            {
                const char* outDirection = nullptr;
                

                proj_cs_get_axis_info(context, pjCs.get(), 0,
                    nullptr,
                    nullptr,
                    &outDirection,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                );
                return QString(outDirection).compare(QLatin1String("north"), Qt::CaseInsensitive) == 0;
            }
            return false;
        }

        bool ProjUtils::isDynamic(const PJ* crs)
        {
            
            bool isDynamic = false;
            PJ_CONTEXT* context = ProjContext::get();

            
            proj_pj_unique_ptr candidate = crsToHorizontalCrs(crs);
            if (!crs)
                candidate = unboundCrs(crs);

            proj_pj_unique_ptr datum(candidate ? proj_crs_get_datum(context, candidate.get()) : nullptr);
            if (datum)
            {
                const PJ_TYPE type = proj_get_type(datum.get());
                isDynamic = type == PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME ||
                    type == PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME;
                if (!isDynamic)
                {
                    const QString authName(proj_get_id_auth_name(datum.get(), 0));
                    const QString code(proj_get_id_code(datum.get(), 0));
                    if (authName == QLatin1String("EPSG") && code == QLatin1String("6326"))
                    {
                        isDynamic = true;
                    }
                }
            }
            else
            {
                
             
            }
            return isDynamic;
        }

        ProjUtils::proj_pj_unique_ptr ProjUtils::crsToHorizontalCrs(const PJ* crs)
        {
            if (!crs)
                return nullptr;

            PJ_CONTEXT* context = ProjContext::get();
            switch (proj_get_type(crs))
            {
            case PJ_TYPE_COMPOUND_CRS:
            {
                int i = 0;
                ProjUtils::proj_pj_unique_ptr res(proj_crs_get_sub_crs(context, crs, i));
                while (res && (proj_get_type(res.get()) == PJ_TYPE_VERTICAL_CRS || proj_get_type(res.get()) == PJ_TYPE_TEMPORAL_CRS))
                {
                    i++;
                    res.reset(proj_crs_get_sub_crs(context, crs, i));
                }
                return res;
            }

            case PJ_TYPE_VERTICAL_CRS:
                return nullptr;

                

            default:
                return unboundCrs(crs);
            }

#ifndef _MSC_VER  
            return nullptr;
#endif
        }

        ProjUtils::proj_pj_unique_ptr ProjUtils::unboundCrs(const PJ* crs)
        {
            if (!crs)
                return nullptr;

            PJ_CONTEXT* context = ProjContext::get();
            switch (proj_get_type(crs))
            {
            case PJ_TYPE_BOUND_CRS:
                return ProjUtils::proj_pj_unique_ptr(proj_get_source_crs(context, crs));

                

            default:
                return ProjUtils::proj_pj_unique_ptr(proj_clone(context, crs));
            }

#ifndef _MSC_VER  
            return nullptr;
#endif
        }

        ProjUtils::proj_pj_unique_ptr ProjUtils::crsToDatumEnsemble(const PJ* crs)
        {
            if (!crs)
                return nullptr;

#if PROJ_VERSION_MAJOR>=8
            PJ_CONTEXT* context = ProjContext::get();
            ProjUtils::proj_pj_unique_ptr candidate = crsToHorizontalCrs(crs);
            if (!candidate) 
                candidate = unboundCrs(crs);

            if (!candidate)
                return nullptr;

            return ProjUtils::proj_pj_unique_ptr(proj_crs_get_datum_ensemble(context, candidate.get()));
#else
            throw NotSupportedException(QObject::tr("Calculating datum ensembles requires a QGIS build based on PROJ 8.0 or later"));
#endif
        }

        bool ProjUtils::identifyCrs(const PJ* crs, QString& authName, QString& authCode, IdentifyFlags flags)
        {
            authName.clear();
            authCode.clear();

            if (!crs)
                return false;

            int* confidence = nullptr;
            if (PJ_OBJ_LIST* crsList = proj_identify(ProjContext::get(), crs, nullptr, nullptr, &confidence))
            {
                const int count = proj_list_get_count(crsList);
                int bestConfidence = 0;
                ProjUtils::proj_pj_unique_ptr matchedCrs;
                for (int i = 0; i < count; ++i)
                {
                    if (confidence[i] >= bestConfidence)
                    {
                        ProjUtils::proj_pj_unique_ptr candidateCrs(proj_list_get(ProjContext::get(), crsList, i));
                        switch (proj_get_type(candidateCrs.get()))
                        {
                        case PJ_TYPE_BOUND_CRS:
                            
                            
                            if (flags & FlagMatchBoundCrsToUnderlyingSourceCrs)
                                break;
                            else
                                continue;

                        default:
                            break;
                        }

                        candidateCrs = ProjUtils::unboundCrs(candidateCrs.get());
                        const QString authName(proj_get_id_auth_name(candidateCrs.get(), 0));
                        
                        if (confidence[i] > bestConfidence || (confidence[i] == bestConfidence && authName == QLatin1String("EPSG")))
                        {
                            bestConfidence = confidence[i];
                            matchedCrs = std::move(candidateCrs);
                        }
                    }
                }
                proj_list_destroy(crsList);
                proj_int_list_destroy(confidence);
                if (matchedCrs && bestConfidence >= 70)
                {
                    authName = QString(proj_get_id_auth_name(matchedCrs.get(), 0));
                    authCode = QString(proj_get_id_code(matchedCrs.get(), 0));
                }
            }
            return !authName.isEmpty() && !authCode.isEmpty();
        }

        bool ProjUtils::coordinateOperationIsAvailable(const QString& projDef)
        {
            if (projDef.isEmpty())
                return true;

            PJ_CONTEXT* context = ProjContext::get();
            ProjUtils::proj_pj_unique_ptr coordinateOperation(proj_create(context, projDef.toUtf8().constData()));
            if (!coordinateOperation)
                return false;

            return static_cast<bool>(proj_coordoperation_is_instantiable(context, coordinateOperation.get()));
        }

        QList<DatumTransform::GridDetails> ProjUtils::gridsUsed(const QString& proj)
        {
            const thread_local QRegularExpression regex(QStringLiteral("\\+(?:nad)?grids=(.*?)\\s"));

            QList< DatumTransform::GridDetails > grids;
            QRegularExpressionMatchIterator matches = regex.globalMatch(proj);
            while (matches.hasNext())
            {
                const QRegularExpressionMatch match = matches.next();
                const QString gridName = match.captured(1);
                DatumTransform::GridDetails grid;
                grid.shortName = gridName;
                const char* fullName = nullptr;
                const char* packageName = nullptr;
                const char* url = nullptr;
                int directDownload = 0;
                int openLicense = 0;
                int available = 0;
                proj_grid_get_info_from_database(ProjContext::get(), gridName.toUtf8().constData(), &fullName, &packageName, &url, &directDownload, &openLicense, &available);
                grid.fullName = QString(fullName);
                grid.packageName = QString(packageName);
                grid.url = QString(url);
                grid.directDownload = directDownload;
                grid.openLicense = openLicense;
                grid.isAvailable = available;
                grids.append(grid);
            }
            return grids;
        }

#if 0
        QStringList ProjUtils::nonAvailableGrids(const QString& projDef)
        {
            if (projDef.isEmpty())
                return QStringList();

            PJ_CONTEXT* context = ProjContext::get();
            ProjUtils::proj_pj_unique_ptr op(proj_create(context, projDef.toUtf8().constData())); < ---- - this always fails if grids are missing
                if (!op)
                    return QStringList();

            QStringList res;
            for (int j = 0; j < proj_coordoperation_get_grid_used_count(context, op.get()); ++j)
            {
                const char* shortName = nullptr;
                int isAvailable = 0;
                proj_coordoperation_get_grid_used(context, op.get(), j, &shortName, nullptr, nullptr, nullptr, nullptr, nullptr, &isAvailable);
                if (!isAvailable)
                    res << QString(shortName);
            }
            return res;
        }
#endif

        int ProjUtils::projVersionMajor()
        {
            return PROJ_VERSION_MAJOR;
        }

        int ProjUtils::projVersionMinor()
        {
            return PROJ_VERSION_MINOR;
        }

        QString ProjUtils::epsgRegistryVersion()
        {
            PJ_CONTEXT* context = ProjContext::get();
            const char* version = proj_context_get_database_metadata(context, "EPSG.VERSION");
            return QString(version);
        }

        QDate ProjUtils::epsgRegistryDate()
        {
            PJ_CONTEXT* context = ProjContext::get();
            const char* date = proj_context_get_database_metadata(context, "EPSG.DATE");
            return QDate::fromString(date, Qt::DateFormat::ISODate);
        }

        QString ProjUtils::esriDatabaseVersion()
        {
            PJ_CONTEXT* context = ProjContext::get();
            const char* version = proj_context_get_database_metadata(context, "ESRI.VERSION");
            return QString(version);
        }

        QDate ProjUtils::esriDatabaseDate()
        {
            PJ_CONTEXT* context = ProjContext::get();
            const char* date = proj_context_get_database_metadata(context, "ESRI.DATE");
            return QDate::fromString(date, Qt::DateFormat::ISODate);
        }

        QString ProjUtils::ignfDatabaseVersion()
        {
            PJ_CONTEXT* context = ProjContext::get();
            const char* version = proj_context_get_database_metadata(context, "IGNF.VERSION");
            return QString(version);
        }

        QDate ProjUtils::ignfDatabaseDate()
        {
            PJ_CONTEXT* context = ProjContext::get();
            const char* date = proj_context_get_database_metadata(context, "IGNF.DATE");
            return QDate::fromString(date, Qt::DateFormat::ISODate);
        }

        QStringList ProjUtils::searchPaths()
        {
            const QString path(proj_info().searchpath);
            QStringList paths;
#ifdef Q_OS_WIN
            paths = path.split(';');
#else
            paths = path.split(':');
#endif

            QSet<QString> existing;
            
            QStringList res;
            res.reserve(paths.count());
            for (const QString& p : std::as_const(paths))
            {
                if (existing.contains(p))
                    continue;

                existing.insert(p);
                res << p;
            }
            return res;
        }
    }
}