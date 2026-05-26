


#ifndef PROJECTIONFACTORS_H
#define PROJECTIONFACTORS_H


#include <QString>
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {

        
        class AI3D_API ProjectionFactors
        {
        public:

            
            bool isValid() const { return mIsValid; }

            
            double meridionalScale() const { return mMeridionalScale; }

            
            double parallelScale() const { return mParallelScale; }

            
            double arealScale() const { return mArealScale; }

            
            double angularDistortion() const { return mAngularDistortion; }

            
            double meridianParallelAngle() const { return mMeridianParallelAngle; }

            
            double meridianConvergence() const { return mMeridianConvergence; }

            
            double tissotSemimajor() const { return mTissotSemimajor; }

            
            double tissotSemiminor() const { return mTissotSemiminor; }

            
            double dxDlam() const { return mDxDlam; }

            
            double dxDphi() const { return mDxDphi; }

            
            double dyDlam() const { return mDyDlam; }

            
            double dyDphi() const { return mDyDphi; }

#ifdef SIP_RUN
            SIP_PYOBJECT __repr__();
            % MethodCode
                QString str;
            if (!sipCpp->isValid())
            {
                str = QStringLiteral("<QgsProjectionFactors: invalid>");
            }
            else
            {
                str = QStringLiteral("<QgsProjectionFactors>");
            }
            sipRes = PyUnicode_FromString(str.toUtf8().constData());
            % End
#endif

        private:

            bool mIsValid = false;
            double mMeridionalScale = 0;
            double mParallelScale = 0;
            double mArealScale = 0;
            double mAngularDistortion = 0;
            double mMeridianParallelAngle = 0;
            double mMeridianConvergence = 0;
            double mTissotSemimajor = 0;
            double mTissotSemiminor = 0;
            double mDxDlam = 0;
            double mDxDphi = 0;
            double mDyDlam = 0;
            double mDyDphi = 0;

            friend class CoordinateReferenceSystem;
        };
    }
}
#endif 
