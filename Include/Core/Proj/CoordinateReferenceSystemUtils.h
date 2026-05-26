


#ifndef COORDINATEREFERENCESYSTEMUTILS_H
#define COORDINATEREFERENCESYSTEMUTILS_H

#include "ProjCore.h"
#include "Constants.h"

namespace AI3D
{
    namespace PROJ
    {
        class CoordinateReferenceSystem;
        
        class AI3D_API CoordinateReferenceSystemUtils
        {
        public:

            
            static ProjCore::CoordinateOrder defaultCoordinateOrderForCrs(const CoordinateReferenceSystem& crs);

            
            static QString axisDirectionToAbbreviatedString(ProjCore::CrsAxisDirection axis);

            
            static QString crsTypeToString(ProjCore::CrsType type);

            
             
            static QString translateProjection(const QString& projection);
        };
    }
}
#endif 
