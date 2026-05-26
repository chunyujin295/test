


#ifndef COORDINATETRANSFORM_H
#define COORDINATETRANSFORM_H

#include <QExplicitlySharedDataPointer>


#include "CoordinateReferenceSystem.h"
#include "CoordinateTransformContext.h"
#include "CoordinateTransform_p.h"
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {
        

        class AI3D_API CoordinateTransform
        {

        public:

            
            CoordinateTransform();
            ~CoordinateTransform() {};

            static void invalidateCache(bool disableCache = false);


            static void removeFromCacheObjectsBelongingToCurrentThread(void* pj_context);

        private:



            mutable QExplicitlySharedDataPointer<CoordinateTransformPrivate> d;

            
            CoordinateTransformContext mContext;

            
            static QReadWriteLock sCacheLock;

            
            static QMultiHash< QPair< QString, QString >, CoordinateTransform > sTransforms;
            static bool sDisableCache;


            static std::function< void(const CoordinateReferenceSystem& sourceCrs,
                const CoordinateReferenceSystem& destinationCrs,
                const QString& desiredOperation)> sFallbackOperationOccurredHandler;

            friend class TestQgsCoordinateTransform;

        };
    }
}

#endif 
