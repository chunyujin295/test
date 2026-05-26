


#ifndef DATUMTRANSFORM_H
#define DATUMTRANSFORM_H



#include <QString>
#include <QList>
#include "Constants.h"

struct PJconsts;
typedef struct PJconsts PJ;

namespace AI3D
{
    namespace PROJ
    {
        class CoordinateReferenceSystem;



        
        class AI3D_API DatumTransform
        {

        public:

            
            struct TransformPair
            {

                
                TransformPair(int sourceTransformId = -1, int destinationTransformId = -1)
                    : sourceTransformId(sourceTransformId)
                    , destinationTransformId(destinationTransformId)
                {}

                
                int sourceTransformId = -1;

                
                int destinationTransformId = -1;

                
                bool operator==(DatumTransform::TransformPair other) const
                {
                    return other.sourceTransformId == sourceTransformId && other.destinationTransformId == destinationTransformId;
                }

                bool operator!=(DatumTransform::TransformPair other) const
                {
                    return other.sourceTransformId != sourceTransformId || other.destinationTransformId != destinationTransformId;
                }

            };

            
            struct TransformInfo
            {
                
                int datumTransformId = -1;

                
                int epsgCode = 0;

                
                QString sourceCrsAuthId;

                
                QString destinationCrsAuthId;

                
                QString sourceCrsDescription;

                
                QString destinationCrsDescription;

                
                QString remarks;

                
                QString scope;

                
                bool preferred = false;

                
                bool deprecated = false;

            };


            
            struct GridDetails
            {
                
                QString shortName;
                
                QString fullName;
                
                QString packageName;
                
                QString url;
                
                bool directDownload = false;
                
                bool openLicense = false;
                
                bool isAvailable = false;
            };

            
            struct SingleOperationDetails
            {
                
                QString scope;

                
                QString remarks;

                
                QString areaOfUse;

                
                QString authority;

                
                QString code;
            };

            
            struct TransformDetails
            {
                
                QString proj;
                
                QString name;
                
                double accuracy = 0;

                
                QString authority;

                
                QString code;

                
                QString scope;

                
                QString remarks;

                
                bool isAvailable = false;

                
                QString areaOfUse;

                
                 

                 
                QList< DatumTransform::GridDetails > grids;

                
                QList< DatumTransform::SingleOperationDetails > operationDetails;
            };

            
            static QList< DatumTransform::TransformDetails > operations(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination, bool includeSuperseded = false);

            
            Q_DECL_DEPRECATED static QList< DatumTransform::TransformPair > datumTransformations(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) ;

            
            Q_DECL_DEPRECATED static QString datumTransformToProj(int datumTransformId) ;

            
            Q_DECL_DEPRECATED static int projStringToDatumTransformId(const QString& string) ;

            
            Q_DECL_DEPRECATED static DatumTransform::TransformInfo datumTransformInfo(int datumTransformId) ;

#ifndef SIP_RUN

            
            static DatumTransform::TransformDetails transformDetailsFromPj(PJ* op);
#endif

        private:

            static void searchDatumTransform(const QString& sql, QList< int >& transforms);


        };
    }
}

#endif 
