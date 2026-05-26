



#include "Core/Proj/ReadWriteLocker.h"
namespace AI3D
{
    namespace PROJ
    {
        ReadWriteLocker::ReadWriteLocker(QReadWriteLock& lock, ReadWriteLocker::Mode mode)
            : mLock(lock)
            , mMode(mode)
        {
            if (mode == Read)
                mLock.lockForRead();
            else if (mode == Write)
                mLock.lockForWrite();
        }

        void ReadWriteLocker::changeMode(ReadWriteLocker::Mode mode)
        {
            if (mode == mMode)
                return;

            unlock();

            mMode = mode;

            if (mMode == Read)
                mLock.lockForRead();
            else if (mMode == Write)
                mLock.lockForWrite();
        }

        void ReadWriteLocker::unlock()
        {
            if (mMode != Unlocked)
                mLock.unlock();

            mMode = Unlocked;
        }

        ReadWriteLocker::~ReadWriteLocker()
        {
            unlock();
        }
    }
}
