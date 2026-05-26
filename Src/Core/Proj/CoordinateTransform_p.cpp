



#include "Core/Proj/CoordinateTransform.h"
#include "Core/Proj/CoordinateTransform_p.h"
#include "Core/Proj/QProj.h"
#include "Core/Proj/Exception.h"

#include "Core/Proj/ReadWriteLocker.h"
#include "Core/Proj/ProjUtils.h"
#include <proj.h>
#include <proj_experimental.h>
#include "Core/Logging.h"
#include <sqlite3.h>
#include "Core/Math.h"
#include <QStringList>


namespace AI3D
{
    namespace PROJ
    {
        std::function< void(const CoordinateReferenceSystem& sourceCrs,
            const CoordinateReferenceSystem& destinationCrs,
            const DatumTransform::GridDetails& grid)> CoordinateTransformPrivate::sMissingRequiredGridHandler = nullptr;

        std::function< void(const CoordinateReferenceSystem& sourceCrs,
            const CoordinateReferenceSystem& destinationCrs,
            const DatumTransform::TransformDetails& preferredOperation,
            const DatumTransform::TransformDetails& availableOperation)> CoordinateTransformPrivate::sMissingPreferredGridHandler = nullptr;

        std::function< void(const CoordinateReferenceSystem& sourceCrs,
            const CoordinateReferenceSystem& destinationCrs,
            const QString& error)> CoordinateTransformPrivate::sCoordinateOperationCreationErrorHandler = nullptr;

        std::function< void(const CoordinateReferenceSystem& sourceCrs,
            const CoordinateReferenceSystem& destinationCrs,
            const DatumTransform::TransformDetails& desiredOperation)> CoordinateTransformPrivate::sMissingGridUsedByContextHandler = nullptr;

        std::function< void(const CoordinateReferenceSystem& sourceCrs,
            const CoordinateReferenceSystem& destinationCrs)> CoordinateTransformPrivate::sDynamicCrsToDynamicCrsWarningHandler = nullptr;

        Q_NOWARN_DEPRECATED_PUSH 
            CoordinateTransformPrivate::CoordinateTransformPrivate()
        {
        }
        Q_NOWARN_DEPRECATED_POP

            Q_NOWARN_DEPRECATED_PUSH 
            CoordinateTransformPrivate::CoordinateTransformPrivate(const CoordinateReferenceSystem& source,
                const CoordinateReferenceSystem& destination,
                const CoordinateTransformContext& context)
            : mSourceCRS(source)
            , mDestCRS(destination)
        {
            if (mSourceCRS != mDestCRS)
                calculateTransforms(context);
        }
        Q_NOWARN_DEPRECATED_POP

            Q_NOWARN_DEPRECATED_PUSH 
            CoordinateTransformPrivate::CoordinateTransformPrivate(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination, int sourceDatumTransform, int destDatumTransform)
            : mSourceCRS(source)
            , mDestCRS(destination)
            , mSourceDatumTransform(sourceDatumTransform)
            , mDestinationDatumTransform(destDatumTransform)
        {
        }

        CoordinateTransformPrivate::CoordinateTransformPrivate(const CoordinateTransformPrivate& other)
            : QSharedData(other)
            , mAvailableOpCount(other.mAvailableOpCount)
            , mIsValid(other.mIsValid)
            , mShortCircuit(other.mShortCircuit)
            , mGeographicToWebMercator(other.mGeographicToWebMercator)
            , mSourceCRS(other.mSourceCRS)
            , mDestCRS(other.mDestCRS)
            , mSourceDatumTransform(other.mSourceDatumTransform)
            , mDestinationDatumTransform(other.mDestinationDatumTransform)
            , mProjCoordinateOperation(other.mProjCoordinateOperation)
            , mShouldReverseCoordinateOperation(other.mShouldReverseCoordinateOperation)
            , mAllowFallbackTransforms(other.mAllowFallbackTransforms)
            , mSourceIsDynamic(other.mSourceIsDynamic)
            , mDestIsDynamic(other.mDestIsDynamic)
            , mSourceCoordinateEpoch(other.mSourceCoordinateEpoch)
            , mDestCoordinateEpoch(other.mDestCoordinateEpoch)
            , mDefaultTime(other.mDefaultTime)
            , mIsReversed(other.mIsReversed)
            , mProjLock()
            , mProjProjections()
            , mProjFallbackProjections()
        {
        }
        Q_NOWARN_DEPRECATED_POP

            Q_NOWARN_DEPRECATED_PUSH
            CoordinateTransformPrivate::~CoordinateTransformPrivate()
        {
            
            freeProj();
        }
        Q_NOWARN_DEPRECATED_POP

            bool CoordinateTransformPrivate::checkValidity()
        {
            if (!mSourceCRS.isValid() || !mDestCRS.isValid())
            {
                invalidate();
                return false;
            }
            return true;
        }

        void CoordinateTransformPrivate::invalidate()
        {
            mShortCircuit = true;
            mIsValid = false;
            mAvailableOpCount = -1;
        }

        bool CoordinateTransformPrivate::initialize()
        {
            invalidate();
            if (!mSourceCRS.isValid())
            {
                
                
                LOGI(("Source CRS is invalid!"));
                return false;
            }

            if (!mDestCRS.isValid())
            {
                
                
                mDestCRS = mSourceCRS;
                LOGI(("Destination CRS is invalid!"));
                return false;
            }

            mIsValid = true;

            if (mSourceCRS == mDestCRS)
            {
                
                
                mShortCircuit = true;
                return true;
            }

            mGeographicToWebMercator =
                mSourceCRS.isGeographic() &&
                mDestCRS.authid() == QLatin1String("EPSG:3857");

            mSourceIsDynamic = mSourceCRS.isDynamic();
            mSourceCoordinateEpoch = mSourceCRS.coordinateEpoch();
            mDestIsDynamic = mDestCRS.isDynamic();
            mDestCoordinateEpoch = mDestCRS.coordinateEpoch();

            
            
            
            
            
            mDefaultTime = (mSourceIsDynamic && !std::isnan(mSourceCoordinateEpoch) && !mDestIsDynamic)
                ? mSourceCoordinateEpoch
                : (mDestIsDynamic && !std::isnan(mDestCoordinateEpoch) && !mSourceIsDynamic)
                ? mDestCoordinateEpoch : std::numeric_limits< double >::quiet_NaN();

            if (mSourceIsDynamic && mDestIsDynamic && !NanCompatibleEquals(mSourceCoordinateEpoch, mDestCoordinateEpoch))
            {
                
                if (sDynamicCrsToDynamicCrsWarningHandler)
                {
                    sDynamicCrsToDynamicCrsWarningHandler(mSourceCRS, mDestCRS);
                }
            }

            
            freeProj();

            
            ProjData res = threadLocalProjData();

#ifdef COORDINATE_TRANSFORM_VERBOSE
            QgsDebugMsgLevel("From proj : " + mSourceCRS.toProj(), 2);
            QgsDebugMsgLevel("To proj   : " + mDestCRS.toProj(), 2);
#endif

            if (!res)
                mIsValid = false;

#ifdef COORDINATE_TRANSFORM_VERBOSE
            if (mIsValid)
            {
                QgsDebugMsgLevel(QStringLiteral("------------------------------------------------------------"), 2);
                QgsDebugMsgLevel(QStringLiteral("The OGR Coordinate transformation for this layer was set to"), 2);
                QgsLogger::debug<CoordinateReferenceSystem>("Input", mSourceCRS, __FILE__, __FUNCTION__, __LINE__);
                QgsLogger::debug<CoordinateReferenceSystem>("Output", mDestCRS, __FILE__, __FUNCTION__, __LINE__);
                QgsDebugMsgLevel(QStringLiteral("------------------------------------------------------------"), 2);
            }
            else
            {
                QgsDebugError(QStringLiteral("The OGR Coordinate transformation FAILED TO INITIALIZE!"));
            }
#else
            if (!mIsValid)
            {
                LOGE(("Coordinate transformation failed to initialize!"));
            }
#endif

            
            mShortCircuit = false;

            return mIsValid;
        }

        void CoordinateTransformPrivate::calculateTransforms(const CoordinateTransformContext& context)
        {
            
            if (mSourceCRS.isValid() && mDestCRS.isValid())
            {
                mProjCoordinateOperation = context.calculateCoordinateOperation(mSourceCRS, mDestCRS);
                mShouldReverseCoordinateOperation = context.mustReverseCoordinateOperation(mSourceCRS, mDestCRS);
                mAllowFallbackTransforms = context.allowFallbackTransform(mSourceCRS, mDestCRS);
            }
            else
            {
                mProjCoordinateOperation.clear();
                mShouldReverseCoordinateOperation = false;
                mAllowFallbackTransforms = false;
            }
        }

        static void proj_collecting_logger(void* user_data, int , const char* message)
        {
            QStringList* dest = reinterpret_cast<QStringList*>(user_data);
            dest->append(QString(message));
        }

        static void proj_logger(void*, int level, const char* message)
        {
#ifndef QGISDEBUG
            Q_UNUSED(message)
#endif
                if (level == PJ_LOG_ERROR)
                {
                    const QString messageString(message);
                    if (messageString == QLatin1String("push: Invalid latitude"))
                    {
                        
                        LOGI(messageString.toStdString());
                    }
                    else
                    {
                        LOGE(messageString.toStdString());
                    }
                }
                else if (level == PJ_LOG_DEBUG)
                {
                    LOGI(message);
                    
                }
        }

        ProjData CoordinateTransformPrivate::threadLocalProjData()
        {
            ReadWriteLocker locker(mProjLock, ReadWriteLocker::Read);

            PJ_CONTEXT* context = ProjContext::get();
            const QMap < uintptr_t, ProjData >::const_iterator it = mProjProjections.constFind(reinterpret_cast<uintptr_t>(context));

            if (it != mProjProjections.constEnd())
            {
                ProjData res = it.value();
                return res;
            }

            
            locker.changeMode(ReadWriteLocker::Write);

            
            QStringList projErrors;
            proj_log_func(context, &projErrors, proj_collecting_logger);

            mIsReversed = false;

            ProjUtils::proj_pj_unique_ptr transform;
            if (!mProjCoordinateOperation.isEmpty())
            {
                transform.reset(proj_create(context, mProjCoordinateOperation.toUtf8().constData()));
                
                
                
                
                
                
                
                
               
                if (!transform)
                {
                    if (sMissingGridUsedByContextHandler)
                    {
                        DatumTransform::TransformDetails desired;
                        desired.proj = mProjCoordinateOperation;
                        desired.accuracy = -1; 
                        desired.grids = ProjUtils::gridsUsed(mProjCoordinateOperation);
                        sMissingGridUsedByContextHandler(mSourceCRS, mDestCRS, desired);
                    }
                    else
                    {
                        const QString err = QObject::tr("Could not use operation specified in project between %1 and %2. (Wanted to use: %3).").arg(mSourceCRS.authid(),
                            mDestCRS.authid(),
                            mProjCoordinateOperation);
                        
                    }

                    transform.reset();
                }
                else
                {
                    mIsReversed = mShouldReverseCoordinateOperation;
                }
            }

            QString nonAvailableError;
            if (!transform) 
            {
                if (!mSourceCRS.projObject() || !mDestCRS.projObject())
                {
                    proj_log_func(context, nullptr, nullptr);
                    return nullptr;
                }

                PJ_OPERATION_FACTORY_CONTEXT* operationContext = proj_create_operation_factory_context(context, nullptr);

                
                proj_operation_factory_context_set_grid_availability_use(context, operationContext, PROJ_GRID_AVAILABILITY_IGNORED);

                
                proj_operation_factory_context_set_spatial_criterion(context, operationContext, PROJ_SPATIAL_CRITERION_PARTIAL_INTERSECTION);

                if (PJ_OBJ_LIST* ops = proj_create_operations(context, mSourceCRS.projObject(), mDestCRS.projObject(), operationContext))
                {
                    mAvailableOpCount = proj_list_get_count(ops);
                    if (mAvailableOpCount < 1)
                    {
                        
                        const int errNo = proj_context_errno(context);
                        if (errNo && errNo != -61)
                        {
                            nonAvailableError = QString(proj_errno_string(errNo));
                        }
                        else
                        {
                            nonAvailableError = QObject::tr("No coordinate operations are available between these two reference systems");
                        }
                    }
                    else if (mAvailableOpCount == 1)
                    {
                        
                        transform.reset(proj_list_get(context, ops, 0));
                        if (transform)
                        {
                            if (!proj_coordoperation_is_instantiable(context, transform.get()))
                            {
                                
                                for (int j = 0; j < proj_coordoperation_get_grid_used_count(context, transform.get()); ++j)
                                {
                                    const char* shortName = nullptr;
                                    const char* fullName = nullptr;
                                    const char* packageName = nullptr;
                                    const char* url = nullptr;
                                    int directDownload = 0;
                                    int openLicense = 0;
                                    int isAvailable = 0;
                                    proj_coordoperation_get_grid_used(context, transform.get(), j, &shortName, &fullName, &packageName, &url, &directDownload, &openLicense, &isAvailable);
                                    if (!isAvailable)
                                    {
                                        
                                        if (sMissingRequiredGridHandler)
                                        {
                                            DatumTransform::GridDetails gridDetails;
                                            gridDetails.shortName = QString(shortName);
                                            gridDetails.fullName = QString(fullName);
                                            gridDetails.packageName = QString(packageName);
                                            gridDetails.url = QString(url);
                                            gridDetails.directDownload = directDownload;
                                            gridDetails.openLicense = openLicense;
                                            gridDetails.isAvailable = isAvailable;
                                            sMissingRequiredGridHandler(mSourceCRS, mDestCRS, gridDetails);
                                        }
                                        else
                                        {
                                            const QString err = QObject::tr("Cannot create transform between %1 and %2, missing required grid %3").arg(mSourceCRS.authid(),
                                                mDestCRS.authid(),
                                                shortName);
                                            
                                        }
                                        break;
                                    }
                                }
                            }
                            else
                            {

                                
                                transform.reset(proj_normalize_for_visualization(context, transform.get()));
                                if (!transform)
                                {
                                    const QString err = QObject::tr("Cannot normalize transform between %1 and %2").arg(mSourceCRS.authid(),
                                        mDestCRS.authid());
                                    
                                }
                            }
                        }
                    }
                    else
                    {
                        
                        DatumTransform::TransformDetails preferred;
                        bool missingPreferred = false;
                        bool stillLookingForPreferred = true;
                        for (int i = 0; i < mAvailableOpCount; ++i)
                        {
                            transform.reset(proj_list_get(context, ops, i));
                            const bool isInstantiable = transform && proj_coordoperation_is_instantiable(context, transform.get());
                            if (stillLookingForPreferred && transform && !isInstantiable)
                            {
                                
                                const DatumTransform::TransformDetails candidate = DatumTransform::transformDetailsFromPj(transform.get());
                                if (!candidate.proj.isEmpty())
                                {
                                    preferred = candidate;
                                    missingPreferred = true;
                                    stillLookingForPreferred = false;
                                }
                            }
                            if (transform && isInstantiable)
                            {
                                
                                break;
                            }
                            transform.reset();
                        }

                        if (transform && missingPreferred)
                        {
                            
                            const DatumTransform::TransformDetails available = DatumTransform::transformDetailsFromPj(transform.get());
                            if (sMissingPreferredGridHandler)
                            {
                                sMissingPreferredGridHandler(mSourceCRS, mDestCRS, preferred, available);
                            }
                            else
                            {
                                const QString err = QObject::tr("Using non-preferred coordinate operation between %1 and %2. Using %3, preferred %4.").arg(mSourceCRS.authid(),
                                    mDestCRS.authid(),
                                    available.proj,
                                    preferred.proj);
                                
                            }
                        }

                        
                        if (transform)
                            transform.reset(proj_normalize_for_visualization(context, transform.get()));
                        if (!transform)
                        {
                            const QString err = QObject::tr("Cannot normalize transform between %1 and %2").arg(mSourceCRS.authid(),
                                mDestCRS.authid());
                            
                        }
                    }
                    proj_list_destroy(ops);
                }
                proj_operation_factory_context_destroy(operationContext);
            }

            if (!transform && nonAvailableError.isEmpty())
            {
                const int errNo = proj_context_errno(context);
                if (errNo && errNo != -61)
                {
                    nonAvailableError = QString(proj_errno_string(errNo));
                }
                else if (!projErrors.empty())
                {
                    nonAvailableError = projErrors.constLast();
                }

                if (nonAvailableError.isEmpty())
                {
                    nonAvailableError = QObject::tr("No coordinate operations are available between these two reference systems");
                }
                else
                {
                    
                    nonAvailableError = nonAvailableError.remove(QStringLiteral("internal_proj_create_operations: "));
                }
            }

            if (!nonAvailableError.isEmpty())
            {
                if (sCoordinateOperationCreationErrorHandler)
                {
                    sCoordinateOperationCreationErrorHandler(mSourceCRS, mDestCRS, nonAvailableError);
                }
                else
                {
                    const QString err = QObject::tr("Cannot create transform between %1 and %2: %3").arg(mSourceCRS.authid(),
                        mDestCRS.authid(),
                        nonAvailableError);
                    
                }
            }

            
            proj_log_func(context, nullptr, proj_logger);

            if (!transform)
            {
                
                return nullptr;
            }

            ProjData res = transform.release();
            mProjProjections.insert(reinterpret_cast<uintptr_t>(context), res);
            return res;
        }

        ProjData CoordinateTransformPrivate::threadLocalFallbackProjData()
        {
            ReadWriteLocker locker(mProjLock, ReadWriteLocker::Read);

            PJ_CONTEXT* context = ProjContext::get();
            const QMap < uintptr_t, ProjData >::const_iterator it = mProjFallbackProjections.constFind(reinterpret_cast<uintptr_t>(context));

            if (it != mProjFallbackProjections.constEnd())
            {
                ProjData res = it.value();
                return res;
            }

            
            locker.changeMode(ReadWriteLocker::Write);

            ProjUtils::proj_pj_unique_ptr transform(proj_create_crs_to_crs_from_pj(context, mSourceCRS.projObject(), mDestCRS.projObject(), nullptr, nullptr));
            if (transform)
                transform.reset(proj_normalize_for_visualization(ProjContext::get(), transform.get()));

            ProjData res = transform.release();
            mProjFallbackProjections.insert(reinterpret_cast<uintptr_t>(context), res);
            return res;
        }

        void CoordinateTransformPrivate::setCustomMissingRequiredGridHandler(const std::function<void(const CoordinateReferenceSystem&, const CoordinateReferenceSystem&, const DatumTransform::GridDetails&)>& handler)
        {
            sMissingRequiredGridHandler = handler;
        }

        void CoordinateTransformPrivate::setCustomMissingPreferredGridHandler(const std::function<void(const CoordinateReferenceSystem&, const CoordinateReferenceSystem&, const DatumTransform::TransformDetails&, const DatumTransform::TransformDetails&)>& handler)
        {
            sMissingPreferredGridHandler = handler;
        }

        void CoordinateTransformPrivate::setCustomCoordinateOperationCreationErrorHandler(const std::function<void(const CoordinateReferenceSystem&, const CoordinateReferenceSystem&, const QString&)>& handler)
        {
            sCoordinateOperationCreationErrorHandler = handler;
        }

        void CoordinateTransformPrivate::setCustomMissingGridUsedByContextHandler(const std::function<void(const CoordinateReferenceSystem&, const CoordinateReferenceSystem&, const DatumTransform::TransformDetails&)>& handler)
        {
            sMissingGridUsedByContextHandler = handler;
        }

        void CoordinateTransformPrivate::setDynamicCrsToDynamicCrsWarningHandler(const std::function<void(const CoordinateReferenceSystem&, const CoordinateReferenceSystem&)>& handler)
        {
            sDynamicCrsToDynamicCrsWarningHandler = handler;
        }

        void CoordinateTransformPrivate::freeProj()
        {
            const ReadWriteLocker locker(mProjLock, ReadWriteLocker::Write);
            if (mProjProjections.isEmpty() && mProjFallbackProjections.isEmpty())
                return;
            QMap < uintptr_t, ProjData >::const_iterator it = mProjProjections.constBegin();

            
            
            
            
            
            PJ_CONTEXT* tmpContext = proj_context_create();
            for (; it != mProjProjections.constEnd(); ++it)
            {
                proj_assign_context(it.value(), tmpContext);
                proj_destroy(it.value());
            }

            it = mProjFallbackProjections.constBegin();
            for (; it != mProjFallbackProjections.constEnd(); ++it)
            {
                proj_assign_context(it.value(), tmpContext);
                proj_destroy(it.value());
            }

            proj_context_destroy(tmpContext);
            mProjProjections.clear();
            mProjFallbackProjections.clear();
        }

        bool CoordinateTransformPrivate::removeObjectsBelongingToCurrentThread(void* pj_context)
        {
            const ReadWriteLocker locker(mProjLock, ReadWriteLocker::Write);

            QMap < uintptr_t, ProjData >::iterator it = mProjProjections.find(reinterpret_cast<uintptr_t>(pj_context));
            if (it != mProjProjections.end())
            {
                proj_destroy(it.value());
                mProjProjections.erase(it);
            }

            it = mProjFallbackProjections.find(reinterpret_cast<uintptr_t>(pj_context));
            if (it != mProjFallbackProjections.end())
            {
                proj_destroy(it.value());
                mProjFallbackProjections.erase(it);
            }

            return mProjProjections.isEmpty();
        }
    }
}

