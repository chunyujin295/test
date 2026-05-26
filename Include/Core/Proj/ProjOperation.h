


#ifndef PROJOPERATION_H
#define PROJOPERATION_H


#include <QString>
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {
        
        class AI3D_API ProjOperation
        {
        public:

            
            bool isValid() const { return mValid; }

            
            QString id() const { return mId; }

            
            QString description() const { return mDescription; }

            
            QString details() const { return mDetails; }

#ifdef SIP_RUN
            SIP_PYOBJECT __repr__();
            % MethodCode
                QString str;
            if (!sipCpp->isValid())
            {
                str = QStringLiteral("<QgsProjOperation: invalid>");
            }
            else
            {
                str = QStringLiteral("<QgsProjOperation: %1>").arg(sipCpp->id());
            }
            sipRes = PyUnicode_FromString(str.toUtf8().constData());
            % End
#endif

        private:

            bool mValid = false;
            QString mId;
            QString mDescription;
            QString mDetails;

            friend class CoordinateReferenceSystemRegistry;
            friend class CoordinateReferenceSystem;
        };
    }
}
#endif 
