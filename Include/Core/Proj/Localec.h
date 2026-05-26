



#ifndef LOCALENUMC_H
#define LOCALENUMC_H

#define SIP_NO_FILE

#include <QMutex>
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {

        
        class AI3D_API LocaleNumC
        {
            char* mOldlocale = nullptr;
            static QMutex sLocaleLock;

        public:
            LocaleNumC();
            ~LocaleNumC();

            
            LocaleNumC(const LocaleNumC& rh) = delete;
            
            LocaleNumC& operator=(const LocaleNumC& rh) = delete;

        };
    }
}
#endif 
