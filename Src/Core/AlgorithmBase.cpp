#include "Core/AlgorithmBase.h"
#include "Core/Camera.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include "Core/CameraModels.h"

 Eigen::Matrix3d MatrixCross(const Eigen::Vector3d& v)
{
    Eigen::Matrix3d M = Eigen::Matrix3d::Zero();
    M(0, 1) = -v[2];
    M(1, 0) = v[2];
    M(0, 2) = v[1];
    M(2, 0) = -v[1];
    M(1, 2) = -v[0];
    M(2, 1) = v[0];
    return M;
}

 Eigen::Matrix3d MatrixReg()
 {
     Eigen::Matrix3d M = Eigen::Matrix3d::Identity();
     M(1, 1) = -1;
  
     M(2, 2) = -1;
     return M;
 }

namespace AI3D
{
    namespace CORE
    {


        Similarity3::Similarity3()
            : pose_(posemetadata_s()),
            scale_(1.0)
        {

        }

        Similarity3::Similarity3(const posemetadata_s& pose, const double scale)
            : pose_(pose),
            scale_(scale)
        {

        }

        Eigen::Matrix<double, 3, Eigen::Dynamic> Similarity3::operator () (const Eigen::Matrix<double, 3, Eigen::Dynamic> & point) const
        {
            
            return scale_ * pose_(point);
        }

        posemetadata_s Similarity3::operator () (const posemetadata_s& pose) const
        {
            return posemetadata_s(pose.rotation * pose_.rotation.transpose(), this->operator()(pose.center));
        }

        Similarity3 Similarity3::inverse() const
        {
            return Similarity3(pose_.inverse(), 1.0 / scale_);
        }
        bool AlgorithmBase::FindRTS
        (
            const Eigen::MatrixXd& x1,
            const Eigen::MatrixXd& x2,
            double* S,
            Eigen::Vector3d* t,
            Eigen::Matrix<double, 3, 3>* R
        )
        {
            if (x1.cols() < 3 || x2.cols() < 3)
            {
                return false;
            }

            assert(3 == x1.rows());
            assert(3 <= x1.cols());
            assert(x1.rows() == x2.rows());
            assert(x1.cols() == x2.cols());

            
            
            
            
            
            const Eigen::Matrix4d transform = Eigen::umeyama(x1, x2, true);

            
            *R = transform.topLeftCorner<3, 3>();
            if (R->determinant() < 0)
            {
                return false;
            }
            *S = pow(R->determinant(), 1.0 / 3.0);
            
            if (*S < std::numeric_limits<double>::epsilon())
            {
                return false;
            }

            
            *S = pow(R->determinant(), 1.0 / 3.0);
            *R /= *S;
            *t = transform.topRightCorner<3, 1>();

            return true;
        }

 

        lm_SRTRefine_functor::lm_SRTRefine_functor(int inputs, int values,
            const Eigen::MatrixXd& x1, const Eigen::MatrixXd& x2,
            const double& S, const Eigen::Matrix3d& R, const Eigen::Vector3d& t) : Functor<double>(inputs, values),
            x1_(x1), x2_(x2), t_(t), R_(R), S_(S) { }

        int lm_SRTRefine_functor::operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const
        {
            
            
            Eigen::Vector3d transAdd = x.block<3, 1>(0, 0);
            Eigen::Vector3d rot = x.block<3, 1>(3, 0);
            double Sadd = x(6);

            
            Eigen::Matrix3d Rcor =
                (Eigen::AngleAxis<double>(rot(0), Eigen::Vector3d::UnitX())
                    * Eigen::AngleAxis<double>(rot(1), Eigen::Vector3d::UnitY())
                    * Eigen::AngleAxis<double>(rot(2), Eigen::Vector3d::UnitZ())).toRotationMatrix();

            const Eigen::Matrix3d nR = R_ * Rcor;
            const Eigen::Vector3d nt = t_ + transAdd;
            const double nS = S_ + Sadd;

            
            Eigen::Vector3d proj;
            for (Eigen::MatrixXd::Index i = 0; i < x1_.cols(); ++i)
            {
                proj = x2_.col(i) - (nS * nR * (x1_.col(i)) + nt);
                fvec[i * 3] = proj(0);
                fvec[i * 3 + 1] = proj(1);
                fvec[i * 3 + 2] = proj(2);
            }
            return 0;
        }


        lm_RRefine_functor::lm_RRefine_functor(int inputs, int values,
            const Eigen::MatrixXd& x1, const Eigen::MatrixXd& x2,
            const double& S, const Eigen::Matrix3d& R, const Eigen::VectorXd& t) : Functor<double>(inputs, values),
            x1_(x1), x2_(x2), t_(t), R_(R), S_(S) { }

        int lm_RRefine_functor::operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const
        {
            
            
            Eigen::Vector3d rot = x.block<3, 1>(0, 0);

            
            Eigen::Matrix3d Rcor =
                (Eigen::AngleAxis<double>(rot(0), Eigen::Vector3d::UnitX())
                    * Eigen::AngleAxis<double>(rot(1), Eigen::Vector3d::UnitY())
                    * Eigen::AngleAxis<double>(rot(2), Eigen::Vector3d::UnitZ())).toRotationMatrix();

            const Eigen::Matrix3d nR = R_ * Rcor;
            const Eigen::Vector3d nt = t_;
            const double nS = S_;

            
            Eigen::Vector3d proj;
            for (Eigen::MatrixXd::Index i = 0; i < x1_.cols(); ++i)
            {
                proj = x2_.col(i) - (nS * nR * (x1_.col(i)) + nt);
                fvec[i * 3] = proj(0);
                fvec[i * 3 + 1] = proj(1);
                fvec[i * 3 + 2] = proj(2);
            }
            return 0;
        }


        void AlgorithmBase::Refine_RTS
        (
            const Eigen::MatrixXd& x1,
            const Eigen::MatrixXd& x2,
            double* S,
            Eigen::Vector3d* t,
            Eigen::Matrix3d* R
        )
        {
            {
                lm_SRTRefine_functor functor(7, 3 * x1.cols(), x1, x2, *S, *R, *t);

                Eigen::NumericalDiff<lm_SRTRefine_functor> numDiff(functor);

                Eigen::LevenbergMarquardt<Eigen::NumericalDiff<lm_SRTRefine_functor>> lm(numDiff);
                lm.parameters.maxfev = 1000;
                Eigen::VectorXd xlm = Eigen::VectorXd::Zero(7); 

                lm.minimize(xlm);

                const Eigen::Vector3d transAdd = xlm.block<3, 1>(0, 0);
                const Eigen::Vector3d rot = xlm.block<3, 1>(3, 0);
                const double SAdd = xlm(6);

                
                const Eigen::Matrix<double, 3, 3> Rcor =
                    (Eigen::AngleAxis<double>(rot(0), Eigen::Vector3d::UnitX())
                        * Eigen::AngleAxis<double>(rot(1), Eigen::Vector3d::UnitY())
                        * Eigen::AngleAxis<double>(rot(2), Eigen::Vector3d::UnitZ())).toRotationMatrix();

                *R = (*R) * Rcor;
                *t = (*t) + transAdd;
                *S = (*S) + SAdd;
            }

             
            {
                lm_RRefine_functor functor(3, 3 * x1.cols(), x1, x2, *S, *R, *t);

                Eigen::NumericalDiff<lm_RRefine_functor> numDiff(functor);

                Eigen::LevenbergMarquardt<Eigen::NumericalDiff<lm_RRefine_functor>> lm(numDiff);
                lm.parameters.maxfev = 1000;
                Eigen::VectorXd xlm = Eigen::VectorXd::Zero(3); 

                lm.minimize(xlm);

                const  Eigen::Vector3d rot = xlm.block<3, 1>(0, 0);

                
                const  Eigen::Matrix3d  Rcor =
                    (Eigen::AngleAxis<double>(rot(0), Eigen::Vector3d::UnitX())
                        * Eigen::AngleAxis<double>(rot(1), Eigen::Vector3d::UnitY())
                        * Eigen::AngleAxis<double>(rot(2), Eigen::Vector3d::UnitZ())).toRotationMatrix();

                *R = (*R) * Rcor;
            }
        }


        void AlgorithmBase::ConvertOPK2Rotmat(double omega, double phi, double kappa, Eigen::Matrix3d& matrix)
        {

             double& a1 = matrix(0, 0);
             double& a2 = matrix(0, 1);
             double& a3 = matrix(0, 2);
             double& b1 = matrix(1, 0);
             double& b2 = matrix(1, 1);
             double& b3 = matrix(1, 2);
             double& c1 = matrix(2, 0);
             double& c2 = matrix(2, 1);
             double& c3 = matrix(2, 2);

             a1 = cos(phi) * cos(kappa);
             b1 = cos(omega) * sin(kappa) + sin(omega) * sin(phi) * cos(kappa);
             c1 = sin(omega) * sin(kappa) - cos(omega) * sin(phi) * cos(kappa);
             a2 = -cos(phi) * sin(kappa);
             b2 = cos(omega) * cos(kappa) - sin(omega) * sin(phi) * sin(kappa);
             c2 = sin(omega) * cos(kappa) + cos(omega) * sin(phi) * sin(kappa);
             a3 = sin(phi);
             b3 = -sin(omega) * cos(phi);
             c3 = cos(omega) * cos(phi);
        }


        void AlgorithmBase::ConvertRotmat2OPK(Eigen::Matrix3d matrix, double& omega, double& phi, double& kappa)
        {

             double a1 = matrix(0, 0);
             double a2 = matrix(0, 1);
             double a3 = matrix(0, 2);
             double b1 = matrix(1, 0);
             double b2 = matrix(1, 1);
             double b3 = matrix(1, 2);
             double c1 = matrix(2, 0);
             double c2 = matrix(2, 1);
             double c3 = matrix(2, 2);
             phi = asin(a3);
             omega = atan(-b3 / c3);
             kappa = atan(-a2 / a1);



             double sinphi = -a3;
             double cosphi = cos(phi);

             double sinomega = -b3 / cosphi;
             double cosomega = c3 / cosphi;

             double sinkappa = -a2 / cosphi;
             double coskappa = a1 / cosphi;

             if (cosomega < 0)
             {
                 if (sinomega < 0)
                 {
                     omega -= M_PI;
                 }
                 else if (sinomega > 0)
                 {
                     omega += M_PI;
                 }
                 else
                 {
                     omega = M_PI;
                 }
             }

             if (coskappa < 0)
             {
                 if (sinkappa < 0)
                 {
                     kappa -= M_PI;
                 }
                 else if (sinkappa > 0)
                 {
                     kappa += M_PI;
                 }
                 else
                 {
                     kappa = M_PI;
                 }
             }
        }

        void AlgorithmBase::ConvertXYZToBLH(Eigen::Matrix3d& rotation, double L, double B, double L0)
        {
            Eigen::Matrix3d rotmatXYZ2BLH;

            Eigen::Matrix3d rotmatXYZ = rotation;
            Eigen::Matrix3d tempR1;
            tempR1 << 1., -(L0-L)*sin(B), 0., (L0 - L)* sin(B), 1, 0, 0., 0, 1;
           
            rotation = tempR1  * rotmatXYZ;
            
        }
        
        void AlgorithmBase::ConvertXYZToBLH(Eigen::Matrix3d& rotation, double dL, double dB)
        {
            Eigen::Matrix3d rotmatXYZ2BLH;

            Eigen::Matrix3d rotmatXYZ = rotation.transpose() * MatrixReg();
            Eigen::Matrix3d tempR1;
            tempR1<<1., 0., 0., 0., sin(dB), cos(dB), 0., -cos(dB), sin(dB);
            Eigen::Matrix3d tempR2;
            tempR2<<-sin(dL), cos(dL), 0., -cos(dL), -sin(dL), 0., 0., 0., 1.;
            rotmatXYZ2BLH = tempR1 * tempR2 * rotmatXYZ;
            rotation = (rotmatXYZ2BLH * MatrixReg()).transpose();


        }

        
        void AlgorithmBase::ConvertBLHToXYZ(Eigen::Matrix3d& rotation, double dL, double dB)
        {

           
        Eigen::Matrix3d rotmat;


        Eigen::Matrix3d rotmatXYZ = rotation.transpose() * MatrixReg();


            Eigen::Matrix3d rotmatBLH2XYZ;
            Eigen::Matrix3d tempR1;
            tempR1 << -sin(dL), -cos(dL), 0., cos(dL), -sin(dL), 0., 0., 0., 1.;
            Eigen::Matrix3d tempR2;
            tempR2<< 1., 0., 0., 0., sin(dB), -cos(dB), 0., cos(dB), sin(dB);
            rotmatBLH2XYZ = tempR1 * tempR2 * rotmatXYZ;
            rotation =(rotmatBLH2XYZ * MatrixReg()).transpose();

        }

        Eigen::Matrix3x4d AlgorithmBase::InvertProjectionMatrix(const Eigen::Matrix3x4d& proj_matrix) 
        {
            Eigen::Matrix3x4d inv_proj_matrix;
            inv_proj_matrix.leftCols<3>() = proj_matrix.leftCols<3>().transpose();
            inv_proj_matrix.rightCols<1>() = ProjectionCenterFromMatrix(proj_matrix);
            return inv_proj_matrix;
        };

        double AlgorithmBase::CalculateNormalizedAngularError(const Eigen::Vector2d& point2D,
            const Eigen::Vector3d& point3D,
            const Eigen::Matrix3x4d& proj_matrix) 
        {
            const Eigen::Vector3d ray1 = point2D.homogeneous();
            const Eigen::Vector3d ray2 = proj_matrix * point3D.homogeneous();
            return std::acos(ray1.normalized().transpose() * ray2.normalized());
        }

        Eigen::Vector3d AlgorithmBase::RotationInnerToYPR(const Eigen::Matrix3d& R)
        {

            double pitch = asin(R(2, 2));						
            double yaw = atan(R(2, 0) / R(2, 1));			
            double roll = atan(-R(0, 2) / R(1, 2));		
            if (1)
            {
                double sinpitch = R(2, 2);
                double cospitch = cos(pitch);

                double sinrolling = R(0, 2) / cospitch;
                double cosrolling = -R(1, 2) / cospitch;

                double sinyaw = R(2, 0) / cospitch;
                double cosyaw = R(2, 1) / cospitch;


                

              
                {

                    if (cosrolling < 0)
                    {
                        if (sinrolling < 0)
                        {
                            roll -= M_PI;
                        }
                        else if (sinrolling > 0)
                        {
                            roll += M_PI;
                        }
                        else
                        {
                            roll = M_PI;
                        }
                    }
                    
                    
                    
                    
                    
                    
                    
                }

                if (cosyaw < 0)
                {
                    if (sinyaw < 0)
                    {
                        yaw -= M_PI;
                    }
                      else if (sinyaw > 0)
                      {
                          yaw += M_PI;
                      }
                    else
                    {
                        yaw = M_PI;
                    }
                }
            }
            yaw = R2FD(yaw);
            pitch = R2FD(pitch);
            roll = R2FD(roll);


            
            
            

            
            
            
            

            
            
            
            
            
            

            
            

            
            

            
            

            
            
            
            
            
            
            
            
            
            
            
            
            
            
            

            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
           
            return Eigen::Vector3d{ yaw,pitch,roll };

        }

      
        Eigen::Matrix3d AlgorithmBase::YPRToRotationInner(const Eigen::Vector3d& ypr)
        {
          if(1)
            {
                double yaw = ypr.x();
                double pitch = ypr.y();
                double roll = ypr.z();
                Eigen::Matrix3d R1;
                R1(0, 0) = cos(yaw) * cos(roll) - sin(yaw) * sin(pitch) * sin(roll);
                R1(0, 1) = -sin(yaw) * cos(roll) - cos(yaw) * sin(pitch) * sin(roll);
                R1(0, 2) = cos(pitch) * sin(roll);
                R1(2, 0) = sin(yaw) * cos(pitch);
                R1(2, 1) = cos(yaw) * cos(pitch);
                R1(2, 2) = sin(pitch);
                R1(1, 0) = cos(yaw) * sin(roll) + sin(yaw) * sin(pitch) * cos(roll);
                R1(1, 1) = -sin(yaw) * sin(roll) + cos(yaw) * sin(pitch) * cos(roll);
                R1(1, 2) = -cos(pitch) * cos(roll);
                return R1;
            }
          
            Eigen::Matrix3d Enu2Ned,Ned2Cam;
             Enu2Ned << 0, 1, 0, 1, 0, 0, 0, 0, -1;
            Ned2Cam << 0, 1, 0, 0, 0, 1, 1, 0, 0;

            
            Eigen::Matrix3d rota = Eigen::AngleAxisd(ypr[0], Eigen::Vector3d::UnitZ()) *
                Eigen::AngleAxisd(ypr[1], Eigen::Vector3d::UnitY()) *
                Eigen::AngleAxisd(ypr[2], Eigen::Vector3d::UnitX()).toRotationMatrix();
            return Ned2Cam * rota.transpose() * Enu2Ned;
            
        }
       

        Eigen::Vector2d AlgorithmBase::ProjectPointToImage(const Eigen::Vector3d& point3D,
            const Eigen::Matrix3x4d& proj_matrix,
            const Camera& camera, bool checkboder )
        {
            const double proj_z = proj_matrix.row(2).dot(point3D.homogeneous());          
            const Eigen::Vector3d world_point = proj_matrix * point3D.homogeneous();          
            Eigen::Vector2d xy=  (camera.GetCalibrationMatrix() * world_point).hnormalized();       
            
            
            
            
            {
                Camera rawcamera = camera;
                if (checkboder)
                {
                    double *undistbord = rawcamera.GetUndistortBorder();
                    double left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y;
                    left_min_x = undistbord[0];
                    left_max_x = undistbord[1];
                    right_min_x = undistbord[2];
                    right_max_x = undistbord[3];
                    top_min_y = undistbord[4];
                    top_max_y = undistbord[5];
                    bottom_min_y = undistbord[6];
                    bottom_max_y = undistbord[7];

                    
                    
                    
					double min_x = left_min_x < right_min_x ? left_min_x : right_min_x;
					double max_x = left_max_x > right_max_x ? left_max_x : right_max_x;
					double min_y = top_min_y < top_max_y ? top_min_y : top_max_y;
					double max_y = bottom_min_y > bottom_max_y ? bottom_min_y : bottom_max_y;
					if (((xy.x() >= min_x - 0.5) && (xy.x() <= max_x + 0.5) && (xy.y() >= min_y - 0.5) && (xy.y() <= max_y + 0.5)))
					{
						Eigen::Vector2d  xy_src, xyresult;
						xy_src.x() = world_point.hnormalized().x();
						xy_src.y() = world_point.hnormalized().y();
						xyresult = camera.WorldToImage(xy_src);

						bool status1 = (camera.GetWidth()) >= xyresult(0) && xyresult(0) >= (0);
						bool status2 = (camera.GetHeight()) >= xyresult(1) && xyresult(1) >= (0);
						if ((status1 && status2))
						{
							return xyresult;
						}
					}
                    
                }
                else
                {
                    Eigen::Vector2d  xy_src, xyresult;
                    xy_src.x() = world_point.hnormalized().x();
                    xy_src.y() = world_point.hnormalized().y();
                    return camera.WorldToImage(xy_src);
                }
            }
                   
                return  Eigen::Vector2d(-DBL_MAX, -DBL_MAX);      
        }


        Eigen::Vector4d AlgorithmBase::NormalizeQuaternion(const Eigen::Vector4d& qvec)
        {
            const double norm = qvec.norm();
            if (norm == 0) {
                
                
                return Eigen::Vector4d(1.0, qvec(1), qvec(2), qvec(3));
            }
            else {
                return qvec / norm;
            }
        }
        Eigen::Matrix3d AlgorithmBase::QuaternionToRotationMatrix(const Eigen::Vector4d& qvec) {
            const Eigen::Vector4d normalized_qvec = NormalizeQuaternion(qvec);
            const Eigen::Quaterniond quat(normalized_qvec(0), normalized_qvec(1),
                normalized_qvec(2), normalized_qvec(3));
            return quat.toRotationMatrix();
        }
        Eigen::Matrix3x4d AlgorithmBase::ComposeProjectionMatrix(const Eigen::Vector4d& qvec,
            const Eigen::Vector3d& tvec) {
            Eigen::Matrix3x4d proj_matrix;
            proj_matrix.leftCols<3>() = QuaternionToRotationMatrix(qvec);
            proj_matrix.rightCols<1>() = tvec;
            return proj_matrix;
        }
    
        double AlgorithmBase::CalculateReprojectionError(const Eigen::Vector2d& point2D,
            const Eigen::Vector3d& point3D,
            const Eigen::Matrix3x4d& proj_matrix,
            const Camera& camera) {
            const auto image_point = ProjectPointToImage(point3D, proj_matrix, camera,false);
            return (image_point - point2D).norm();
        }

        

        Eigen::Matrix3x4d AlgorithmBase::ComposeProjectionMatrix(const Eigen::Matrix3d& R,
            const Eigen::Vector3d& C) 
        {
            Eigen::Matrix3x4d proj_matrix;
            proj_matrix.leftCols<3>() = R;
            proj_matrix.rightCols<1>() = -R * C;
           
            return proj_matrix;
        }

         Eigen::Vector3d AlgorithmBase::ProjectionCenterFromMatrix(
            const Eigen::Matrix3x4d& proj_matrix)
        {
            return -proj_matrix.leftCols<3>().transpose() * proj_matrix.rightCols<1>();
        }

         Eigen::Matrix3d AlgorithmBase::RotationCenterFromMatrix(
            const Eigen::Matrix3x4d& proj_matrix)
        {
            
            
            return proj_matrix.leftCols<3>();
        }



         bool AlgorithmBase::TriangulateNViewAlgebraic
         (
             const Eigen::Matrix<double, 3, Eigen::Dynamic>& points,
             const std::vector< Eigen::Matrix<double, 3, 4>>& poses,
             Eigen::Vector4d* X
         )
         {
             assert(poses.size() == points.cols());

             Eigen::Matrix<double, 4, 4> AtA =  Eigen::Matrix<double, 4, 4>::Zero();
            for (Eigen::Matrix<double, 3, Eigen::Dynamic>::Index i = 0; i < points.cols(); ++i)
             {
                 
                 const Eigen::Vector3d point_norm = points.col(i).normalized();
                 const  Eigen::Matrix<double, 3, 4> cost =
                     poses[i] -
                     point_norm * point_norm.transpose() * poses[i];
                 AtA += cost.transpose() * cost;
             }

             Eigen::SelfAdjointEigenSolver< Eigen::Matrix<double, 4, 4>> eigen_solver(AtA);
            
             return eigen_solver.info() == Eigen::Success;
         }


         Eigen::Vector3d TriangulatePoint(const Eigen::Matrix3x4d& proj_matrix1,
             const Eigen::Matrix3x4d& proj_matrix2,
             const Eigen::Vector2d& point1,
             const Eigen::Vector2d& point2) {
             Eigen::Matrix4d A;

             A.row(0) = point1(0) * proj_matrix1.row(2) - proj_matrix1.row(0);
             A.row(1) = point1(1) * proj_matrix1.row(2) - proj_matrix1.row(1);
             A.row(2) = point2(0) * proj_matrix2.row(2) - proj_matrix2.row(0);
             A.row(3) = point2(1) * proj_matrix2.row(2) - proj_matrix2.row(1);

             Eigen::JacobiSVD<Eigen::Matrix4d> svd(A, Eigen::ComputeFullV);

             return svd.matrixV().col(3).hnormalized();
         }

         Eigen::Vector3d AlgorithmBase::TriangulatePointd(const std::vector<Eigen::Matrix<double, 3, 4>>& proj_matrix,
             const std::vector<Eigen::Vector2d>& point)
         {
             CHECK_OPTION_GE(point.size(), 2);
             CHECK_OPTION_EQ(point.size(), proj_matrix.size());

             
             {
                 int size = proj_matrix.size();

                 Eigen::MatrixX4d A(size * 2, 4);
                 for (int i = 0; i < size; i++)
                 {
                     
                     A.row(2 * i + 0) = point[i](0) * proj_matrix[i].row(2) - proj_matrix[i].row(0);
                     A.row(2 * i + 1) = point[i](1) * proj_matrix[i].row(2) - proj_matrix[i].row(1);
                 }
                 
                 Eigen::JacobiSVD<Eigen::MatrixX4d> svd(A, Eigen::ComputeFullV);

                 return svd.matrixV().col(3).hnormalized();
             }
           
         }



        Eigen::Vector3f AlgorithmBase::TriangulatePoint(const std::vector<Eigen::Matrix<float, 3, 4>>& proj_matrix,
            const std::vector<Eigen::Vector2f>& point)
        {
            CHECK_OPTION_GE(point.size(), 2);
            CHECK_OPTION_EQ(point.size(), proj_matrix.size());
           
          
            {
                int size = proj_matrix.size();
                
                Eigen::MatrixX4f A(size * 2, 4);
                for (int i = 0; i < size; i++)
                {
                   
                    A.row(2 * i + 0) = point[i](0) * proj_matrix[i].row(2) - proj_matrix[i].row(0);
                    A.row(2 * i + 1) = point[i](1) * proj_matrix[i].row(2) - proj_matrix[i].row(1);
                }
               
                Eigen::JacobiSVD<Eigen::MatrixX4f> svd(A, Eigen::ComputeFullV);

                return svd.matrixV().col(3).hnormalized();
            }
           
        }

        


        Eigen::Vector3d AlgorithmBase::TriangulatePoint(const Eigen::Matrix3x4d& proj_matrix1,
            const Eigen::Matrix3x4d& proj_matrix2,
            const Eigen::Vector2d& point1,
            const Eigen::Vector2d& point2) 
        {
            Eigen::Matrix4d A;

            A.row(0) = point1(0) * proj_matrix1.row(2) - proj_matrix1.row(0);
            A.row(1) = point1(1) * proj_matrix1.row(2) - proj_matrix1.row(1);
            A.row(2) = point2(0) * proj_matrix2.row(2) - proj_matrix2.row(0);
            A.row(3) = point2(1) * proj_matrix2.row(2) - proj_matrix2.row(1);

            Eigen::JacobiSVD<Eigen::Matrix4d> svd(A, Eigen::ComputeFullV);

            return svd.matrixV().col(3).hnormalized();
        }

        std::vector<Eigen::Vector3d> AlgorithmBase::TriangulatePoints(
            const Eigen::Matrix3x4d& proj_matrix1,
            const Eigen::Matrix3x4d& proj_matrix2,
            const std::vector<Eigen::Vector2d>& points1,
            const std::vector<Eigen::Vector2d>& points2)
        {
            CHECK_OPTION_EQ(points1.size(), points2.size());

            std::vector<Eigen::Vector3d> points3D(points1.size());

            for (size_t i = 0; i < points3D.size(); ++i) 
            {
                points3D[i] =
                    TriangulatePoint(proj_matrix1, proj_matrix2, points1[i], points2[i]);
            }

            return points3D;
        }
        Eigen::Vector4d AlgorithmBase::ComposeIdentityQuaternion() {
            return Eigen::Vector4d(1, 0, 0, 0);
        }
        Eigen::Vector4d AlgorithmBase::RotationMatrixToQuaternion(const Eigen::Matrix3d& rot_mat) 
        {
            const Eigen::Quaterniond quat(rot_mat);
            return Eigen::Vector4d(quat.w(), quat.x(), quat.y(), quat.z());
        }

        double AlgorithmBase::CalculateSquaredReprojectionError(const Eigen::Vector2d& point2D,
            const Eigen::Vector3d& point3D,
            const Eigen::Matrix3x4d& proj_matrix,
            const Camera& camera) {
            const double proj_z = proj_matrix.row(2).dot(point3D.homogeneous());

            
            
            
            

            const double proj_x = proj_matrix.row(0).dot(point3D.homogeneous());
            const double proj_y = proj_matrix.row(1).dot(point3D.homogeneous());
            const double inv_proj_z = 1.0 / proj_z;

            const Eigen::Vector2d proj_point2D = camera.WorldToImage(
                Eigen::Vector2d(inv_proj_z * proj_x, inv_proj_z * proj_y));

            return (proj_point2D - point2D).squaredNorm();
        }

        Eigen::Vector3d AlgorithmBase::TriangulateMultiViewPoint(
            const std::vector<Eigen::Matrix3x4d>& proj_matrices,
            const std::vector<Eigen::Vector2d>& points) 
        {
            CHECK_OPTION_EQ(proj_matrices.size(), points.size());

            Eigen::Matrix4d A = Eigen::Matrix4d::Zero();

            for (size_t i = 0; i < points.size(); i++)
            {
                const Eigen::Vector3d point = points[i].homogeneous().normalized();
                const Eigen::Matrix3x4d term =
                    proj_matrices[i] - point * point.transpose() * proj_matrices[i];
                A += term.transpose() * term;
            }

            Eigen::SelfAdjointEigenSolver<Eigen::Matrix4d> eigen_solver(A);

            return eigen_solver.eigenvectors().col(0).hnormalized();
        }



        double AlgorithmBase::CalculateTriangulationAngle(const Eigen::Vector3d& proj_center1,
            const Eigen::Vector3d& proj_center2,
            const Eigen::Vector3d& point3D)
        {
            const double baseline_length_squared =
                (proj_center1 - proj_center2).squaredNorm();

            const double ray_length_squared1 = (point3D - proj_center1).squaredNorm();
            const double ray_length_squared2 = (point3D - proj_center2).squaredNorm();

            
            const double denominator =
                2.0 * std::sqrt(ray_length_squared1 * ray_length_squared2);
            if (denominator == 0.0)
            {
                return 0.0;
            }
            const double nominator =
                ray_length_squared1 + ray_length_squared2 - baseline_length_squared;
            const double angle = std::abs(std::acos(nominator / denominator));

            
            
            
            return std::min(angle, M_PI - angle);
        }

        std::vector<double> AlgorithmBase::CalculateTriangulationAngles(
            const Eigen::Vector3d& proj_center1, const Eigen::Vector3d& proj_center2,
            const std::vector<Eigen::Vector3d>& points3D) 
        {
            
            const double baseline_length_squared =
                (proj_center1 - proj_center2).squaredNorm();

            std::vector<double> angles(points3D.size());

            for (size_t i = 0; i < points3D.size(); ++i) 
            {
                
                const double ray_length_squared1 =
                    (points3D[i] - proj_center1).squaredNorm();
                const double ray_length_squared2 =
                    (points3D[i] - proj_center2).squaredNorm();

                
                const double denominator =
                    2.0 * std::sqrt(ray_length_squared1 * ray_length_squared2);
                if (denominator == 0.0) {
                    angles[i] = 0.0;
                    continue;
                }
                const double nominator =
                    ray_length_squared1 + ray_length_squared2 - baseline_length_squared;
                const double angle = std::abs(std::acos(nominator / denominator));

                
                
                
                angles[i] = std::min(angle, M_PI - angle);
            }

            return angles;
        }

        Eigen::Matrix3d AlgorithmBase::F_from_P(const Eigen::Matrix3x4d& P1, const Eigen::Matrix3x4d& P2)
        {
            Eigen::Matrix3d F12;

            using Mat24 = Eigen::Matrix<double, 2, 4>;
            Mat24 X1 = P1.block<2, 4>(1, 0);
            Mat24 X2;  X2 << P1.row(2), P1.row(0);
            Mat24 X3 = P1.block<2, 4>(0, 0);
            Mat24 Y1 = P2.block<2, 4>(1, 0);
            Mat24 Y2;  Y2 << P2.row(2), P2.row(0);
            Mat24 Y3 = P2.block<2, 4>(0, 0);


            Eigen::Matrix4d X1Y1, X2Y1, X3Y1, X1Y2, X2Y2, X3Y2, X1Y3, X2Y3, X3Y3;
            X1Y1 << X1, Y1;  X2Y1 << X2, Y1;  X3Y1 << X3, Y1;
            X1Y2 << X1, Y2;  X2Y2 << X2, Y2;  X3Y2 << X3, Y2;
            X1Y3 << X1, Y3;  X2Y3 << X2, Y3;  X3Y3 << X3, Y3;


            F12 <<
                X1Y1.determinant(), X2Y1.determinant(), X3Y1.determinant(),
                X1Y2.determinant(), X2Y2.determinant(), X3Y2.determinant(),
                X1Y3.determinant(), X2Y3.determinant(), X3Y3.determinant();

            return F12;
        }


       
        Eigen::Matrix3d CrossProductMatrix(const Eigen::Vector3d& vector) {
            Eigen::Matrix3d matrix;
            matrix << 0, -vector(2), vector(1), vector(2), 0, -vector(0), -vector(1),
                vector(0), 0;
            return matrix;
        }
        Eigen::Matrix3d EssentialMatrixFromPose(const Eigen::Matrix3d& R,
            const Eigen::Vector3d& t) {
            return CrossProductMatrix(t.normalized()) * R;
        }


        void AlgorithmBase::GetEpipolarLine(Eigen::Vector2d xy, const Eigen::Matrix3x4d& p1, const Eigen::Matrix3x4d& p2,
             Camera& K1,Camera& K2, std::pair< Eigen::Vector2d, Eigen::Vector2d >& line)
        {
            
         
            
            Eigen::Vector2d undistxy = K1.UndistortPixel(xy);
            Eigen::Vector3d xyz = CalcEpipolarLine(p1, p2,
                K1.GetCalibrationMatrix(),
                K2.GetCalibrationMatrix(),
                undistxy.x(), undistxy.y());
            
            Eigen::Vector2d x0;
            Eigen::Vector2d y0;
            if (!AlgorithmBase::LineToEndPoints(xyz,
                K2.GetWidth(),
                K2.GetHeight(), x0, y0))
                return;
          
            Eigen::Matrix3d scale_K_inverse = K2.GetCalibrationMatrix().inverse();
            Eigen::Vector3d x01 = scale_K_inverse * x0.homogeneous();
            Eigen::Vector3d y01 = scale_K_inverse * y0.homogeneous();
            Eigen::Vector2d l0, l1;
            l0 = K2.WorldToImage(x01.hnormalized());
            l1 = K2.WorldToImage(y01.hnormalized());
          
           
            double a = l1.y() - l0.y();
            double b = l0.x() - l1.x();
            double c = l1.x() * l0.y() - l0.x() * l1.y();
            Eigen::Vector2d x011;
            Eigen::Vector2d y011;
            Eigen::Vector3d undisline{ a,b,c };
           
            AlgorithmBase::LineToEndPoints(undisline,
                K2.GetWidth(),
                K2.GetHeight(), x011, y011);
            line.first = x011;
            line.second = y011;
           
     
            
        }

        Eigen::Matrix3d EssentialMatrixFromAbsolutePoses(
            const Eigen::Matrix3x4d& proj_matrix1,
            const Eigen::Matrix3x4d& proj_matrix2) 
        {
            const Eigen::Matrix3d R1 = proj_matrix1.leftCols<3>();
            const Eigen::Matrix3d R2 = proj_matrix2.leftCols<3>();
            const Eigen::Vector3d t1 = proj_matrix1.rightCols<1>();
            const Eigen::Vector3d t2 = proj_matrix2.rightCols<1>();

            
            const Eigen::Matrix3d R = R2 * R1.transpose();
            const Eigen::Vector3d t = t2 - R * t1;

            return EssentialMatrixFromPose(R, t);
        }
        Eigen::Vector3d AlgorithmBase::CalcEpipolarLine(const Eigen::Matrix3x4d& p1, const Eigen::Matrix3x4d& p2, const Eigen::Matrix3d& K1,
            const Eigen::Matrix3d& K2, const double x1, const double y1)
        {

            
            const Eigen::Matrix3d R1 = p1.block(0, 0, 3, 3);
            const Eigen::Matrix3d R2 = p2.block(0, 0, 3, 3);
            const Eigen::Vector3d T2 = p2.rightCols<1>();
            const Eigen::Vector3d C2 = -R2.transpose() * T2;
            const Eigen::Vector3d T1 = p1.block(0, 3, 3, 1);
            const Eigen::Matrix3d E = R2 * R1.transpose() * MatrixCross(R1 * C2 + T1);
         
            Eigen::Matrix3d E1 = EssentialMatrixFromAbsolutePoses(p1, p2);
           

            const Eigen::Matrix3d F = K2.transpose().inverse() * E * K1.inverse();

            Eigen::Vector3d v1(x1, y1, 1.);
            return  F * v1;

        }

      

        inline bool AlgorithmBase::LineToEndPoints(const Eigen::Vector3d& line, int W, int H, Eigen::Vector2d& x0, Eigen::Vector2d& x1)
        {
            const double a = line(0), b = line(1), c = line(2);
         
            float r1, r2;
            
            if (b != 0)
            {
                double x = (b < 0) ? 0 : W - 1;
                double y = -(a * x + c) / b;
                if (y < 0) y = 0.;
                else if (y >= H) y = H - 1;
                r1 = std::abs(a * x + b * y + c);
                x0 << x, y;
            }
            else {
                return false;
            }

            
            if (a != 0)
            {
                double y =  (a < 0) ? H - 1 : 0;
               
                double x = -(b * y + c) / a;
              
                if (x < 0) x = 0.;
                else if (x >= W) x = W - 1;
                r2 = std::abs(a * x + b * y + c);
                x1 << x, y;
            }
            else {
                return false;
            }

            
            if (r1 > r2)
                std::swap(x0, x1);
            
            bool x1_inconner = false;
            Eigen::Vector2i lt{ 0,0 };
            Eigen::Vector2i rt{ W-1,0 };
            Eigen::Vector2i lb{ 0 ,H - 1};
            Eigen::Vector2i rb{ W - 1,H - 1 };
            Eigen::Vector2i bxpt = x1.cast<int>();
            if (bxpt == lt || bxpt == rt || bxpt == lb || bxpt == rb)
            {
                x1_inconner = true;
            }
           
            
            if (x1_inconner)
            {
                
                int flag = 0;
                if (x0(1) == 0)
                {
                    flag = 1;
                    
                    double y =  H - 1 ;
                    double x = -(b * y + c) / a;
                    if (x < 0) x = 0.;
                    else if (x >= W) x = W - 1;                  
                    x1 << x, y;
                }
                if (x0(0) == W - 1)
                {
                    flag = 2;
                    
                    double x =0 ;
                    double y = -(a * x + c) / b;
                    if (y < 0) y = 0.;
                    else if (y >= H) y = H - 1;
                  
                    x1 << x, y;
                }
                if (x0(1) == H - 1)
                {
                    flag = 3; 
                    
                    double y = 0;
                    double x = -(b * y + c) / a;
                    if (x < 0) x = 0.;
                    else if (x >= W) x = W - 1;
                    x1 << x, y;
                }

                if (flag == 0)
                {
                    
                    double x = W-1;
                    double y = -(a * x + c) / b;
                    if (y < 0) y = 0.;
                    else if (y >= H) y = H - 1;

                    x1 << x, y;
                }
            }
            

            return true;
        }

      
        


    }
}  
