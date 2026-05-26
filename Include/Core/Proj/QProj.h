
#ifndef QPROJ_H
#define QPROJ_H
#include <Constants.h>
#include <QApplication>
#include <QEvent>
#include <QStringList>
#include <QColor>

#include <memory>
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {
        class CoordinateReferenceSystemRegistry;

        class AI3D_API  QProj
        {

        public:

            QProj();

            ~QProj();

            static QProj* instance();

            enum endian_t
            {
                XDR = 0,  
                NDR = 1   
            };
            static endian_t endian();

            static CoordinateReferenceSystemRegistry* coordinateReferenceSystemRegistry() ;

        private:

            struct ApplicationMembers
            {

                CoordinateReferenceSystemRegistry* mCrsRegistry = nullptr;

                ApplicationMembers();
                ~ApplicationMembers();
            };

            
            ApplicationMembers* mApplicationMembers = nullptr;
            
            static ApplicationMembers* sApplicationMembers;
            static ApplicationMembers* members();
            static void invalidateCaches();
        };

    }
}
#endif
