


#ifndef COORDINATEREFERENCESYSTEM_PRIVATE_H
#define COORDINATEREFERENCESYSTEM_PRIVATE_H












#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/Proj/ProjCore.h"
#include <proj.h>
#include "Core/Proj/ProjUtils.h"
#include "Core/Proj/ReadWriteLocker.h"

#ifdef DEBUG
typedef struct OGRSpatialReferenceHS *OGRSpatialReferenceH;
#else
typedef void *OGRSpatialReferenceH;
#endif
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {
        class CoordinateReferenceSystemPrivate : public QSharedData
        {
        public:

            explicit CoordinateReferenceSystemPrivate()
            {
            }

            CoordinateReferenceSystemPrivate(const CoordinateReferenceSystemPrivate& other)
                : QSharedData(other)
                , mSrsId(other.mSrsId)
                , mDescription(other.mDescription)
                , mProjectionAcronym(other.mProjectionAcronym)
                , mEllipsoidAcronym(other.mEllipsoidAcronym)
                , mProjType(other.mProjType)
                , mIsGeographic(other.mIsGeographic)
                , mMapUnits(other.mMapUnits)
                , mAuthId(other.mAuthId)
                , mIsValid(other.mIsValid)
                , mCoordinateEpoch(other.mCoordinateEpoch)
                , mPj()
                , mPjParentContext(nullptr)
                , mProj4(other.mProj4)
                , mWktPreferred(other.mWktPreferred)
                , mAxisInvertedDirty(other.mAxisInvertedDirty)
                , mAxisInverted(other.mAxisInverted)
                , mProjLock{}
                , mProjObjects()
            {
            }

            ~CoordinateReferenceSystemPrivate()
            {
                ReadWriteLocker locker(mProjLock, ReadWriteLocker::Read);
                if (!mProjObjects.empty() || mPj)
                {
                    locker.changeMode(ReadWriteLocker::Write);
                    cleanPjObjects();
                }
            }

            
            long mSrsId = 0;

            
            QString mDescription;

            
            QString mProjectionAcronym;

            
            QString mEllipsoidAcronym;

            PJ_TYPE mProjType = PJ_TYPE::PJ_TYPE_UNKNOWN;

            
            bool mIsGeographic = false;

            
            ProjCore::DistanceUnit mMapUnits = ProjCore::DistanceUnit::Unknown;

            
            long mSRID = 0;

            
            QString mAuthId;

            
            bool mIsValid = false;

            
            double mCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();

            
            

        private:
            ProjUtils::proj_pj_unique_ptr mPj;
            PJ_CONTEXT* mPjParentContext = nullptr;

            void cleanPjObjects()
            {

                
                
                
                
                
                PJ_CONTEXT* tmpContext = proj_context_create();
                for (auto it = mProjObjects.begin(); it != mProjObjects.end(); ++it)
                {
                    proj_assign_context(it.value(), tmpContext);
                    proj_destroy(it.value());
                }
                mProjObjects.clear();
                if (mPj)
                {
                    proj_assign_context(mPj.get(), tmpContext);
                    mPj.reset();
                }
                proj_context_destroy(tmpContext);
            }

        public:

            void setPj(ProjUtils::proj_pj_unique_ptr obj)
            {
                const ReadWriteLocker locker(mProjLock, ReadWriteLocker::Write);
                cleanPjObjects();

                mPj = std::move(obj);
                mPjParentContext = ProjContext::get();

                if (mPj)
                {
                    mProjType = proj_get_type(mPj.get());
                }
                else
                {
                    mProjType = PJ_TYPE_UNKNOWN;
                }
            }

            bool hasPj() const
            {
                const ReadWriteLocker locker(mProjLock, ReadWriteLocker::Read);
                return static_cast<bool>(mPj);
            }

            mutable QString mProj4;

            mutable QString mWktPreferred;

            
            mutable bool mAxisInvertedDirty = false;

            
            mutable bool mAxisInverted = false;

        private:
            mutable QReadWriteLock mProjLock{};
            mutable QMap < PJ_CONTEXT*, PJ* > mProjObjects{};

        public:

            PJ* threadLocalProjObject() const
            {
                ReadWriteLocker locker(mProjLock, ReadWriteLocker::Read);
                if (!mPj)
                    return nullptr;

                PJ_CONTEXT* context = ProjContext::get();
                const QMap < PJ_CONTEXT*, PJ* >::const_iterator it = mProjObjects.constFind(context);

                if (it != mProjObjects.constEnd())
                {
                    return it.value();
                }

                
                locker.changeMode(ReadWriteLocker::Write);

                PJ* res = proj_clone(context, mPj.get());
                mProjObjects.insert(context, res);
                return res;
            }

            
            bool removeObjectsBelongingToCurrentThread(PJ_CONTEXT* pj_context)
            {
                const ReadWriteLocker locker(mProjLock, ReadWriteLocker::Write);

                const QMap < PJ_CONTEXT*, PJ* >::iterator it = mProjObjects.find(pj_context);
                if (it != mProjObjects.end())
                {
                    proj_destroy(it.value());
                    mProjObjects.erase(it);
                }

                if (mPjParentContext == pj_context)
                {
                    mPj.reset();
                    mPjParentContext = nullptr;
                }

                return mProjObjects.isEmpty();
            }

        private:
            CoordinateReferenceSystemPrivate& operator= (const CoordinateReferenceSystemPrivate&) = delete;

        };
    }
}


#endif 
