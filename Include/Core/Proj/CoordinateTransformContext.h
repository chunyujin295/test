



#ifndef COORDINATETRANSFORMCONTEXT_H
#define COORDINATETRANSFORMCONTEXT_H


#include "DatumTransform.h"

#include <QMetaType>
#include <QExplicitlySharedDataPointer>

#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {
        class CoordinateReferenceSystem;
        class ReadWriteContext;
        class CoordinateTransformContextPrivate;


        

         

        class AI3D_API CoordinateTransformContext
        {
        public:

            
            CoordinateTransformContext();

            ~CoordinateTransformContext();

            
            CoordinateTransformContext(const CoordinateTransformContext& rhs);

            
            CoordinateTransformContext& operator=(const CoordinateTransformContext& rhs) ;

            bool operator==(const CoordinateTransformContext& rhs) const;

            
            void clear();

            
            Q_DECL_DEPRECATED QMap< QPair< QString, QString>, DatumTransform::TransformPair > sourceDestinationDatumTransforms() const ;

            
            QMap< QPair< QString, QString>, QString > coordinateOperations() const;

            
            Q_DECL_DEPRECATED bool addSourceDestinationDatumTransform(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs, int sourceTransformId, int destinationTransformId) ;

            
            bool addCoordinateOperation(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs, const QString& coordinateOperationProjString, bool allowFallback = true);

            
            Q_DECL_DEPRECATED void removeSourceDestinationDatumTransform(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs) ;

            
            void removeCoordinateOperation(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs);

            
            bool hasTransform(const CoordinateReferenceSystem& source,
                const CoordinateReferenceSystem& destination) const;

            
            Q_DECL_DEPRECATED DatumTransform::TransformPair calculateDatumTransforms(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const ;

            
            QString calculateCoordinateOperation(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const;

            
            bool allowFallbackTransform(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const;

            
            bool mustReverseCoordinateOperation(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const;

            



            
            void readSettings();

            
            void writeSettings();


        private:

            QExplicitlySharedDataPointer<CoordinateTransformContextPrivate> d;

        };

        Q_DECLARE_METATYPE(CoordinateTransformContext)
    }
}
#endif 




