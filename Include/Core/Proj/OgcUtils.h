
#ifndef OGCUTILS_H
#define OGCUTILS_H



class QString;


#include <list>
#include <QVector>



#include "Core/Proj/CoordinateTransformContext.h"


#ifndef SIP_RUN

namespace AI3D
{
    namespace PROJ
    {
        
        class AI3D_API OgcCrsUtils
        {
        public:

            
            enum class CRSFlavor
            {
                UNKNOWN, 
                AUTH_CODE, 
                HTTP_EPSG_DOT_XML, 
                OGC_URN, 
                X_OGC_URN, 
                OGC_HTTP_URI, 
            };

            
            static CRSFlavor parseCrsName(const QString& crsName, QString& authority, QString& code);
        };

        Q_DECLARE_METATYPE(OgcCrsUtils::CRSFlavor)

#endif 
    }
}
#endif 
