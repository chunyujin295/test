


#ifndef EXCEPTION_H
#define EXCEPTION_H

#define SIP_NO_CREATION

#define SIP_NO_FILE

#include <QString>

#include "Constants.h"


namespace AI3D
{
    namespace PROJ
    {
        
        class AI3D_API Exception
        {
        public:

            
            Exception(const QString& message)
                : mWhat(message)
            {}

            
            virtual ~Exception() throw() = default;

            
            QString what() const throw()
            {
                return mWhat;
            }

        private:

            
            QString mWhat;

        };




        
        class AI3D_API NotSupportedException : public Exception
        {
        public:

            
            NotSupportedException(const QString& message) : Exception(message) {}
        };
    }
}
#endif
