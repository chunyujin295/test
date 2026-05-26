

#ifndef VARIANTUTILS_H
#define VARIANTUTILS_H
#include "Constants.h"
#include <QVariant>
namespace AI3D
{
    namespace PROJ
    {

        
        class AI3D_API VariantUtils
        {
        public:

            
            static QString typeToDisplayString(QVariant::Type type, QVariant::Type subType = QVariant::Type::Invalid);

            
            static bool isNull(const QVariant& variant);

            
            static QMetaType::Type variantTypeToMetaType(QVariant::Type variantType);

            
            static QVariant::Type metaTypeToVariantType(QMetaType::Type metaType) ;

        };
    }
}
#endif 
