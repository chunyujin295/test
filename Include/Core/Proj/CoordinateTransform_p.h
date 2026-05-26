


#ifndef COORDINATETRANSFORMPRIVATE_H
#define COORDINATETRANSFORMPRIVATE_H

#define SIP_NO_FILE













#include <QSharedData>

struct PJconsts;
typedef struct PJconsts PJ;
typedef PJ *ProjData;

#include "CoordinateReferenceSystem.h"
#include "CoordinateTransformContext.h"

#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {

        class CoordinateTransformPrivate : public QSharedData
        {

        public:

            explicit CoordinateTransformPrivate();

            CoordinateTransformPrivate(const CoordinateReferenceSystem& source,
                const CoordinateReferenceSystem& destination,
                const CoordinateTransformContext& context);

            CoordinateTransformPrivate(const CoordinateReferenceSystem& source,
                const CoordinateReferenceSystem& destination,
                int sourceDatumTransform,
                int destDatumTransform);

            CoordinateTransformPrivate(const CoordinateTransformPrivate& other);

            ~CoordinateTransformPrivate();

            bool checkValidity();

            void invalidate();

            bool initialize();

            void calculateTransforms(const CoordinateTransformContext& context);

            ProjData threadLocalProjData();

            int mAvailableOpCount = -1;
            ProjData threadLocalFallbackProjData();

            
            bool removeObjectsBelongingToCurrentThread(void* pj_context);

            
            bool mIsValid = false;

            
            bool mShortCircuit = false;

            
            bool mGeographicToWebMercator = false;

            
            CoordinateReferenceSystem mSourceCRS;

            
            CoordinateReferenceSystem mDestCRS;

            Q_DECL_DEPRECATED QString mSourceProjString;
            Q_DECL_DEPRECATED QString mDestProjString;

            Q_DECL_DEPRECATED int mSourceDatumTransform = -1;
            Q_DECL_DEPRECATED int mDestinationDatumTransform = -1;
            QString mProjCoordinateOperation;
            bool mShouldReverseCoordinateOperation = false;
            bool mAllowFallbackTransforms = true;

            bool mSourceIsDynamic = false;
            bool mDestIsDynamic = false;
            double mSourceCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();
            double mDestCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();
            double mDefaultTime = std::numeric_limits< double >::quiet_NaN();

            
            bool mIsReversed = false;

            QReadWriteLock mProjLock;
            QMap < uintptr_t, ProjData > mProjProjections;
            QMap < uintptr_t, ProjData > mProjFallbackProjections;

            
            static void setCustomMissingRequiredGridHandler(const std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const DatumTransform::GridDetails& grid)>& handler);

            
            static void setCustomMissingPreferredGridHandler(const std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const DatumTransform::TransformDetails& preferredOperation,
                const DatumTransform::TransformDetails& availableOperation)>& handler);

            
            static void setCustomCoordinateOperationCreationErrorHandler(const std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const QString& error)>& handler);

            
            static void setCustomMissingGridUsedByContextHandler(const std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const DatumTransform::TransformDetails& desiredOperation)>& handler);

            
            static void setDynamicCrsToDynamicCrsWarningHandler(const std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs)>& handler);

        private:

            void freeProj();

            static std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const DatumTransform::GridDetails& grid)> sMissingRequiredGridHandler;

            static std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const DatumTransform::TransformDetails& preferredOperation,
                const DatumTransform::TransformDetails& availableOperation)> sMissingPreferredGridHandler;

            static std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const QString& error)> sCoordinateOperationCreationErrorHandler;

            static std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const DatumTransform::TransformDetails& desiredOperation)> sMissingGridUsedByContextHandler;

            static std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs)> sDynamicCrsToDynamicCrsWarningHandler;

            CoordinateTransformPrivate& operator= (const CoordinateTransformPrivate&) = delete;
        };
    }
}


#endif 
