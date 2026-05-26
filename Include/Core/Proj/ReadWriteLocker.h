



#ifndef READWRITELOCKER_H
#define READWRITELOCKER_H


#include <QReadWriteLock>
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {

        
        class AI3D_API ReadWriteLocker
        {
        public:

            
            enum Mode
            {
                Read, 
                Write, 
                Unlocked 
            };

            
            ReadWriteLocker(QReadWriteLock& lock, Mode mode);

            
            void changeMode(Mode mode);

            
            void unlock();

            ~ReadWriteLocker();

        private:
#ifdef SIP_RUN
            QgsReadWriteLocker& operator=(const QgsReadWriteLocker&);
#endif

            QReadWriteLock& mLock;
            Mode mMode = Unlocked;
        };
    }
}
#endif 
