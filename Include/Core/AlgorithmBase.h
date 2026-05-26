#ifndef _AI3D_CORE_ALGORITHMBASE_H_
#define _AI3D_CORE_ALGORITHMBASE_H_

#include <vector>

#include <Eigen/Core>

#include "Core/Camera.h"
#include "Core/Alignment.h"
#include "Core/Math.h"
#include "Core/Types.h"

namespace AI3D
{
    namespace CORE
    {

        struct Similarity3
        {
            
            posemetadata_s pose_;

            
            double scale_;

            
            Similarity3();

            
            Similarity3(const posemetadata_s& pose, const double scale);

            
            Eigen::Matrix<double, 3, Eigen::Dynamic> operator () (const Eigen::Matrix<double, 3, Eigen::Dynamic>& point) const;

            
            posemetadata_s operator () (const posemetadata_s& pose) const;

            
            Similarity3 inverse() const;

        };



        template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
        struct Functor
        {
            using Scalar = _Scalar;
            enum
            {
                InputsAtCompileTime = NX,
                ValuesAtCompileTime = NY
            };
            using InputType = Eigen::Matrix<Scalar, InputsAtCompileTime, 1>;
            using ValueType = Eigen::Matrix<Scalar, ValuesAtCompileTime, 1>;
            using JacobianType = Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime>;


            
            const int m_inputs;

            
            const int m_values;

            
            Functor()
                : m_inputs(InputsAtCompileTime),
                m_values(ValuesAtCompileTime)
            {

            }
            Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

            
            int inputs() const
            {
                return m_inputs;
            }

            
            int values() const
            {
                return m_values;
            }
        };

        struct lm_SRTRefine_functor : Functor<double>
        {
            
            lm_SRTRefine_functor(int inputs, int values,
                const Eigen::MatrixXd& x1, const Eigen::MatrixXd& x2,
                const double& S, const  Eigen::Matrix3d& R, const  Eigen::Vector3d& t);

            
            int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const;

            Eigen::MatrixXd x1_, x2_;
            Eigen::Vector3d t_;
            Eigen::Matrix3d R_;
            double S_;
        };

        struct lm_RRefine_functor : Functor<double>
        {
            
            lm_RRefine_functor(int inputs, int values,
                const Eigen::MatrixXd& x1, const Eigen::MatrixXd& x2,
                const double& S, const  Eigen::Matrix3d& R, const Eigen::VectorXd& t);

            
            int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const;

            Eigen::MatrixXd x1_, x2_;
            Eigen::Vector3d  t_;
            Eigen::Matrix3d R_;
            double S_;
        };


        class AI3D_API AlgorithmBase
        {
        public:
          
            static Eigen::Vector3d TransformPointW2Iz(const Eigen::Matrix3x4d& proj_matrix,
                const Eigen::Matrix3d& cam_mat, const Eigen::Vector3d& pt)
            {
                Eigen::Vector3d tmp = TransformPointW2C(proj_matrix,pt);
                Eigen::Vector2d obr = TransformPointC2I(cam_mat, tmp);
                return  Eigen::Vector3d(obr(0),obr(1),tmp(2));
              
            }
            static Eigen::Vector2d TransformPointC2I(const Eigen::Matrix3d& cam_matrix,
                const Eigen::Vector3d& point) 
            {
               
                return (cam_matrix * point).hnormalized();
            };

            static Eigen::Vector3d TransformPointW2C(const Eigen::Matrix3x4d& proj_matrix,
                const Eigen::Vector3d& point) 
            {
               
                return proj_matrix * point.homogeneous();
            };

            static void ConvertOPK2Rotmat(double omega, double phi, double kappa, Eigen::Matrix3d& matrix);
            static void ConvertRotmat2OPK(Eigen::Matrix3d rotmat, double& omega, double& phi, double& kappa);
            static void ConvertXYZToBLH(Eigen::Matrix3d& rotation, double L, double B,double L0);
            static void ConvertXYZToBLH(Eigen::Matrix3d& rotation, double L, double B);
            static void ConvertBLHToXYZ(Eigen::Matrix3d& rotation, double L, double B);
            static Eigen::Vector3d ProjectionCenterFromMatrix(
                const Eigen::Matrix3x4d& proj_matrix);
            static Eigen::Matrix3d QuaternionToRotationMatrix(const Eigen::Vector4d& qvec);
            static Eigen::Matrix3x4d ComposeProjectionMatrix(const Eigen::Vector4d& qvec,
                const Eigen::Vector3d& tvec);
            static Eigen::Matrix3x4d InvertProjectionMatrix(const Eigen::Matrix3x4d& proj_matrix);
            static Eigen::Vector4d NormalizeQuaternion(const Eigen::Vector4d& qvec);
            static Eigen::Vector2d ProjectPointToImage(const Eigen::Vector3d& point3D,
                const Eigen::Matrix3x4d& proj_matrix,
                const Camera& camera, bool checkboder = true);
           
            static double CalculateReprojectionError(const Eigen::Vector2d& point2D,
                const Eigen::Vector3d& point3D,
                const Eigen::Matrix3x4d& proj_matrix,
                const Camera& camera);
            static double CalculateNormalizedAngularError(const Eigen::Vector2d& point2D,
                const Eigen::Vector3d& point3D,
                const Eigen::Matrix3x4d& proj_matrix);

            static Eigen::Matrix3d YPRToRotationInner(const Eigen::Vector3d& ypr);
            
            static  Eigen::Vector3d RotationInnerToYPR(const Eigen::Matrix3d& R);

            static void Refine_RTS
            (
                const Eigen::MatrixXd& x1,
                const Eigen::MatrixXd& x2,
                double* S,
                Eigen::Vector3d* t,
                Eigen::Matrix<double, 3, 3>* R
            );

            static bool FindRTS(
                const Eigen::MatrixXd& x1,
                const Eigen::MatrixXd& x2,
                double* S,
                Eigen::Vector3d* t,
                Eigen::Matrix<double, 3, 3>* R
            );

            
            static Eigen::Matrix3x4d ComposeProjectionMatrix(const Eigen::Matrix3d& R,
                const Eigen::Vector3d& C);
           
         
            static Eigen::Matrix3d RotationCenterFromMatrix(
                const Eigen::Matrix3x4d& proj_matrix);
            static bool TriangulateNViewAlgebraic(
                const Eigen::Matrix<double, 3, Eigen::Dynamic>& points,
                const std::vector< Eigen::Matrix<double, 3, 4>>& poses,
                Eigen::Vector4d* X    );

            static Eigen::Vector3d TriangulatePointd(const std::vector<Eigen::Matrix<double, 3, 4>>& proj_matrix,
                const std::vector<Eigen::Vector2d>& point);
            static Eigen::Vector3f TriangulatePoint(const std::vector<Eigen::Matrix<float, 3, 4>>& proj_matrix,
                const std::vector<Eigen::Vector2f>& point);
             static Eigen::Vector3d TriangulatePoint(const Eigen::Matrix3x4d& proj_matrix1,
                const Eigen::Matrix3x4d& proj_matrix2,
                const Eigen::Vector2d& point1,
                const Eigen::Vector2d& point2);
             static void GetEpipolarLine(Eigen::Vector2d xy, const Eigen::Matrix3x4d& p1, const Eigen::Matrix3x4d& p2,
                 Camera& K1,Camera& K2, std::pair< Eigen::Vector2d, Eigen::Vector2d >& line);

             static Eigen::Vector3d CalcEpipolarLine(const Eigen::Matrix3x4d& p1, const Eigen::Matrix3x4d& p2,
                 const Eigen::Matrix3d& K1,
                const Eigen::Matrix3d& K2, const double x1,const double y1);
            
            static Eigen::Vector3d TriangulatePointCalcEpipolarLine(Eigen::Matrix3x4d& p1, Eigen::Matrix3x4d& p2, Eigen::Matrix3d& K1,
                Eigen::Matrix3d& K2, double x1, double y1);
            static inline bool LineToEndPoints(const Eigen::Vector3d& line, int W, int H, Eigen::Vector2d& x0, Eigen::Vector2d& x1);
            static Eigen::Matrix3d F_from_P(const Eigen::Matrix3x4d& P1, const Eigen::Matrix3x4d& P2);

            static std::vector<Eigen::Vector3d> TriangulatePoints(
                const Eigen::Matrix3x4d& proj_matrix1,
                const Eigen::Matrix3x4d& proj_matrix2,
                const std::vector<Eigen::Vector2d>& points1,
                const std::vector<Eigen::Vector2d>& points2);


            static Eigen::Vector3d TriangulateMultiViewPoint(
                const std::vector<Eigen::Matrix3x4d>& proj_matrices,
                const std::vector<Eigen::Vector2d>& points);
            static Eigen::Vector4d RotationMatrixToQuaternion(const Eigen::Matrix3d& rot_mat);
            static Eigen::Vector4d ComposeIdentityQuaternion();
            
            static double CalculateTriangulationAngle(const Eigen::Vector3d& proj_center1,
                const Eigen::Vector3d& proj_center2,
                const Eigen::Vector3d& point3D);
            std::vector<double> CalculateTriangulationAngles(
                const Eigen::Vector3d& proj_center1, const Eigen::Vector3d& proj_center2,
                const std::vector<Eigen::Vector3d>& points3D);

            
            Eigen::Vector3d CalcEpipolarLine(Eigen::Matrix3x4d& p1, Eigen::Matrix3x4d& p2, double x1, double y1);


            static double CalculateSquaredReprojectionError(const Eigen::Vector2d& point2D,
                const Eigen::Vector3d& point3D,
                const Eigen::Matrix3x4d& proj_matrix,
                const Camera& camera);


        };
    }
}  

#endif  
