



#ifndef COORDINATETRANSFORMCONTEXT_PRIVATE_H
#define COORDINATETRANSFORMCONTEXT_PRIVATE_H














#define SIP_NO_FILE

#include "CoordinateReferenceSystem.h"
#include "DatumTransform.h"
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {
        class CoordinateTransformContextPrivate : public QSharedData
        {

        public:

            CoordinateTransformContextPrivate() = default;

            CoordinateTransformContextPrivate(const CoordinateTransformContextPrivate& other)
                : QSharedData(other)
                , mLock{}
            {
                other.mLock.lockForRead();
                mSourceDestDatumTransforms = other.mSourceDestDatumTransforms;
                other.mLock.unlock();
            }

            
            class OperationDetails
            {
            public:
                QString operation;
                bool allowFallback = true;

                
                bool operator==(const OperationDetails& other) const
                {
                    return operation == other.operation && allowFallback == other.allowFallback;
                }
            };
            QMap< QPair< CoordinateReferenceSystem, CoordinateReferenceSystem >, OperationDetails > mSourceDestDatumTransforms;

            
            mutable QReadWriteLock mLock{};

        private:
            CoordinateTransformContextPrivate& operator= (const CoordinateTransformContextPrivate&) = delete;
        };


        

    }
}
#endif 




