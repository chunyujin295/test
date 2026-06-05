#ifndef _AI3D_CORE_RECONSTRUCTION_H_
#define _AI3D_CORE_RECONSTRUCTION_H_
#include "Core/Image.h"
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include "Core/File.h"
#include <Constants.h>
#include <omp.h>
#include <opencv2/opencv.hpp>
#include "Core/Camera.h"
#include "Core/Point2d.h"
#include "Core/Point3d.h"
#include "Core/Track.h"
#include "Core/alignment.h"
#include "Core/Types.h"
#include "Core/ControlPoint.h"
#include "Core/MeasurePoint.h"
#include "Core/Bitmap.h"
#include "Core/ATOptions.h"

#include "Core/SimilarityTransform.h"
#include "Core/Ransac.h"
#include "Core/Loransac.h"

namespace AI3D
{
    namespace CORE
    {

        inline cv::Scalar BGR2YCrCb(const cv::Scalar& bgr)
        {
            const auto B = bgr[0];
            const auto G = bgr[1];
            const auto R = bgr[2];
            const auto delta = 128.0;
            const auto Y = 0.299 * R + 0.587 * G + 0.114 * B;
            const auto Cb = (B - Y) * 0.564 + delta;
            const auto Cr = (R - Y) * 0.713 + delta;
            return cv::Scalar(Y, Cr, Cb);
        }

        inline cv::Scalar YCrCb2BGR(const cv::Scalar& YCB)
        {
            const auto Y = YCB[0];
            const auto Cr = YCB[1];
            const auto Cb = YCB[2];
            const auto delta = 128.0;

            const auto B = (Cb - 0.5) * 1. / 0.564 + Y;
            const auto R = (Cr - 0.5) * 1. / 0.713 + Y;
            const auto  G = 1. / 0.587 * (Y - 0.299 * R - 0.114 * B);
            return cv::Scalar(B, G, R);
        }

        

        typedef std::pair<image_t, float> viewweight;
       
        
        struct Acquisition
        {
            int id = -1;
            double rms_px = -DBL_MAX;
            double error_3d = -DBL_MAX;
            std::string name;
            double error_xy = -DBL_MAX;
            double error_z = -DBL_MAX;
            int num_observations = 0;
            int num_connection = 0;
        };
        struct gcp_quality
        {
            int id = 0;
            Eigen::Vector3d error;
            Eigen::Vector3d estimated;
            int num_observations = 0;
            std::string usage;
        };

        struct ComputeSummary
        {
            int time_feature_extreation = 0;
            int time_feature_match = 0;

            int num_match_pairs = 0;
            int num_predict_pairs = 0;
            int time_Adjustment = 0;
            int time_Adjustment_Optimize = 0;
        };

        struct ConnectionPointQualityBriefing
        {
            double rms_px = NAN;
            double error_repro_px = NAN;
            int Meadium_ConnectionPoint_Perimage = 0;
            int num_observations = 0;
            int num_connections = 0;
        };
        typedef struct SingleImageQualityBriefing
        {
            double rms_px = -DBL_MAX;
            int num_images_connection = 0;
            std::string imagefullpath;
            int photogroup_id = 0;
            int num_connections = 0;
            int num_observations = 0;

        }SingleQB;
        struct Image_QualityBriefing
        {
            std::vector<SingleQB> singleQB;
            int Medium_Images_Perimage = 0;
            int Meadium_ConnectionPoint_Perimage = 0;
        };
        struct ProjectDescrip
        {
            std::string projectName;
            int num_photos = 0;
            int Calibration_failed_photos = 0;
            int Calibration_success_photos = 0;
			float Ratio_Calibration_success = 0.0;
            float average_resolution = 0.0;
            int num_cameras = 0;
        };
        struct AcquisitionReport
        {
            double rms_px_all = -DBL_MAX;
            double error_3d_all = -DBL_MAX;
            double error_xy_all = -DBL_MAX;
            double error_z_all = -DBL_MAX;
            double error_reproj = -DBL_MAX;

            int num_gcps = 0;
            std::vector<Acquisition> accuracy_vec;
            gpt_e gpt_type = gpt_e::GCP_UNKNOWN;
        };

        struct CameraCalibration
        {
            int id = 0;
            double focalinmm = UNDEFINEDVAL;
            double focalin35mm = UNDEFINEDVAL;
			std::vector<double> parameters;
        };

        struct AI3D_API ATReport
        {
            ATOptions at_optins;
            ProjectDescrip proj_desc;
            AcquisitionReport gcp_accuracy;
            AcquisitionReport checkpoint_accuracy;
            std::vector<CameraCalibration> CameraUnCalibratedParam;
            std::vector<CameraCalibration> CameraCalibratedParam;
            ComputeSummary cs; 
            ConnectionPointQualityBriefing connectionQB;
            Image_QualityBriefing imageQB;

            int all_pair_count = 0;
            int feature_match_time = 0;
            int removed_image_count = 0;
            int solved_pair_count = 0;
            int num_tasks = 0;

            
        };

        enum bb_scope_e
        {

            BB_SCOPE_VIEWS = 1,
            BB_SCOPE_VIEWS_TIEPOINTS = 1<<1,
            BB_SCOPE_VIEWFRUSTUM = 1<<2,
            BB_SCOPE_TIEPOINTS = 1 << 3,
           
        };

        
        
        enum atpoint_elements_e
        {
           
            PT_ELE_VIEWS = 1,
            PT_ELE_TIEPOINTS = 2,
            PT_ELE_CONTROPOINTS = 4,
            PT_ELE_VIEWFRUSTUM = 8,
            PT_ELE_VIEWS_TIEPOINTS = PT_ELE_VIEWS | PT_ELE_TIEPOINTS,
            PT_ELE_VIEWS_CONTROPOINTS = PT_ELE_VIEWS | PT_ELE_CONTROPOINTS,
            PT_ELE_TIEPOINTS_CONTROPOINTS = PT_ELE_TIEPOINTS | PT_ELE_CONTROPOINTS,
           
            PT_ELE_ALL = PT_ELE_VIEWS | PT_ELE_TIEPOINTS | PT_ELE_CONTROPOINTS,
        };

        class AI3D_API ATData
        {
        public:
            ATData();

            ATData(const ATData& Atdata);
            ATData& operator=(const ATData& Atdata);
            ~ATData();
            struct AI3D_API SimplifyOptions
            {
                SimplifyOptions() {}

                int min_overlap_ = 3;
                int max_overlap_ = -1;
                int max_tiepoint_count_ = 200000;
                float max_proj_error_ = 1.2f;
                bool Save(std::string fileout)
                {
                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    rapidjson::Document document;
                    document.SetObject();
                    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

                    if (min_overlap_ >0)
                    {
                        document.AddMember("max_overlap", rapidjson::Value(std::to_string(min_overlap_).c_str(), allocator), allocator);
                    }
                    if (max_overlap_ > 0)
                    {
                        document.AddMember("max_overlap", rapidjson::Value(std::to_string(max_overlap_).c_str(), allocator), allocator);
                    }
                    if (max_tiepoint_count_ > 0)
                    {
                        document.AddMember("max_tiepoint_count", rapidjson::Value(std::to_string(max_tiepoint_count_).c_str(), allocator), allocator);
                    }
                    if (max_proj_error_ > 0)
                    {
                        document.AddMember("max_proj_error", rapidjson::Value(std::to_string(max_proj_error_).c_str(), allocator), allocator);
                    }
                    document.Accept(writer);
                    std::ofstream file = File::OpenOfstreamUtf8(fileout, std::ios::out);
                    if (!file.good())
                        return false;

                    file << buffer.GetString();
                    file.close();
                    return true;
                }
                bool Load(std::string filein)
                {
                    std::string blkcontent;
                    std::ifstream in = File::OpenIfstreamUtf8(filein, std::ios::in);
                    if (!in.is_open())
                        return false;
                    std::string line;
                    std::string content;
                    while (std::getline(in, line))
                    {

                        if (line[line.size() - 1] != '\n')
                            line.append("\n");

                        content.append(line);
                    }
                    in.close();
                    rapidjson::Document doc;
                    if (doc.Parse(content.data()).HasParseError())
                    {
                        return false;
                    }
                    if (!doc.IsObject())
                    {
                        return false;
                    }

                    if (doc.HasMember("min_overlap"))
                    {
                        min_overlap_ = doc["min_overlap"].GetInt();
                    }

                    if (doc.HasMember("max_overlap"))
                    {
                        max_overlap_ = doc["max_overlap"].GetInt();
                    }
                    if (doc.HasMember("max_tiepoint_count"))
                    {
                        max_tiepoint_count_ = doc["max_tiepoint_count"].GetInt();
                    }
                    if (doc.HasMember("max_proj_error"))
                    {
                        max_proj_error_ = doc["max_proj_error"].GetDouble();
                    }
                    return true;
                }

            };

            void GeneratePointViews(std::set<image_t>& imageids, std::set<point3D_t>& point3dids);
            void GeneratePointViews();
            const std::map<point3D_t, std::vector<viewweight>>& GetPointsViews() const;
            const std::map<image_t, std::vector<point3D_t>>& GetViewPoints() const;
            
            bool  Simplify(const SimplifyOptions& opt);
            
            template <bool kEstimateScale = true>
            bool AlignRobust(const std::vector<std::string>& image_names,
                const std::vector<Eigen::Vector3d>& locations,
                const int min_common_images,
                const RANSACOptions& ransac_options,
                SimilarityTransform3* tform = nullptr);


            inline double GetMin3dPoint_x() const;
            inline double GetMin3dPoint_y() const;
            inline double GetMin3dPoint_z() const;

            inline double GetMax3dPoint_y() const;
            inline double GetMax3dPoint_z() const;
            inline double GetMax3dPoint_x() const;

            inline double GetMaxCamera_z() const;

            void GetBoundingBox(bool& imagechanged,bool& tiepointchanged,bool& gcpchanged);

            ABBox2d GetImageCenterABB();
            
            
            double GetGSD() const;

            
            float ComputeAvgResolution();
            
            bool GenerateBaseInfo(ProjectDescrip& proj_desc);
            void  EraseDuplicateImages(ATData& datatemp);

            bool GenerateATReportForCam( std::vector<CameraCalibration>& cameracalibrationvec);
            bool GenerateATReportForGCP(AcquisitionReport& gcp_accuracy, AcquisitionReport& checkpoint_accuracy);
            bool GenerateATReportForTiepoint(ConnectionPointQualityBriefing& connectionQB);
            bool GenerateATReportForImageAndTiepoint(ConnectionPointQualityBriefing& connectionQB, Image_QualityBriefing& imageQB);
            
            bool GenerateATReport( ATReport& at_report);
            
            void ComputeDepths();
            void ComputeFrustum();
            
            void ComputeTiepointError(std::vector<double>& errors);
            

            std::vector<Eigen::Vector3d> ComputePoints3dCoordinateAxis();
            
            inline size_t GetNumCameras() const;
            inline size_t GetNumImages() const;
            inline size_t GetNumRegImages() const;
            inline size_t GetNumControlPoints() const;
            

            inline  point3D_t GetNumUserPoints() const;
            inline  constraint_t GetNumConstraint() const;
            size_t GetNumValidUserPoints();
            
            inline size_t GetNumGCPElements() const;
            size_t GetNumValidControlPoints();
            size_t GetNumCheckPoints();
            
            inline size_t GetNumPoints3D() const;
            inline size_t GetNumImagePairs() const;
            void RenderPoses(Eigen::Vector3d& offset, srs_s& srs);
            bool CanPredict();
            void Predict();
            
            bool UnditortData(const std::string& path);
            bool UndistortImages(const std::string& path);
            
            void GetEpipolarLines(point2D_t img_id, point3D_t pt_id, std::map<int, std::pair<Eigen::Vector2d, Eigen::Vector2d>>& epipolarlines);
            void ComputeAvgHeight();
            
            void PredictGCPMeasurement(const point3D_t& gcp_id, std::map<image_t, Eigen::Vector2d >& estimated_xys);
            void PredictGCPMeasurement(const point3D_t& gcp_id, std::set<image_t>& imgids, bool btopredict_ = false);
            void PredictGCPMeasurement(const point3D_t& gcp_id, image_t img_id, Eigen::Vector2d& estimated_xy,bool checkborder = true, bool istopredict = false);
            
            
            inline const Camera& GetCamera(const camera_t camera_id) const;
            const ::AI3D::CORE::Image& GetImage(const image_t image_id) const;
            inline const Point3D& GetPoint3D(const point3D_t point3D_id) const;
            inline const class MeasureConstraint& GetConstraint(const constraint_t constraint_id) const;
            inline void SetPoint3D(const EIGEN_STL_UMAP(point3D_t, Point3D)& point3D);
            inline void SetUserPoint3D(const EIGEN_STL_UMAP(point3D_t, Point3D)& point3D);
            inline void SetConstraint(EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& constraints);
            inline const std::pair<size_t, size_t>& GetImagePair(
                const image_pair_t pair_id) const;
            

                                                         
            inline Camera& GetCameraMutual(const camera_t camera_id);
            AI3D::CORE::Image& GetImageMutual(const image_t image_id);
            inline Point3D& GetPoint3DMutual(const point3D_t point3D_id);
            inline class MeasureConstraint& GetConstraintMutual(const constraint_t constraint_id);
            inline std::pair<size_t, size_t>& GetImagePairMutual(const image_pair_t pair_id);
            
            std::set<image_t> GetImagesIdSet() const;
            bool GenPreviewImages(const std::string& path, std::set<image_t> vecimage);
                 
            inline const EIGEN_STL_UMAP(camera_t, Camera)& GetCameras() const;
            inline  EIGEN_STL_UMAP(camera_t, Camera)& GetCamerasMutual();
            const EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& GetImages() const;
            EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& GetImagesMutual();

            
            
            inline const std::vector<image_t> GetRegImageIds() const;
            std::vector<image_t>& GetRegImageIdsMutual();
            void SetRegImageIds(const std::vector<image_t>& reg_image_ids);
            
            std::vector<image_t> GetHasPostionImagesIds() const; 
            std::vector<image_t> GetImagesIds() const;
            std::vector<image_t> GetHasPriorPostionImagesIds() const;

            inline const EIGEN_STL_UMAP(point3D_t, Point3D)& GetPoints3D() const;
            inline EIGEN_STL_UMAP(point3D_t, Point3D)& GetPoints3DMutual();
            inline const EIGEN_STL_UMAP(point3D_t, Point3D)& GetUserPoints3D() const;
            inline EIGEN_STL_UMAP(point3D_t, Point3D)& GetUserPoints3DMutual();
            const EIGEN_STL_UMAP(point3D_t, ControlPoint)& GetControlPoints() const;
            EIGEN_STL_UMAP(point3D_t, ControlPoint)& GetControlPointsMutual();
            const EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& GetConstraints() const;
            EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& GetConstraintsMutual();
            void ClearPoints2D(image_t imageid);
            void ClearPose(const std::set<image_t>& imageids);
            void ClearControlPoints();
            inline void SetControlPoints(const EIGEN_STL_UMAP(point3D_t, ControlPoint)& controlpoints);
            void ClearConstraints();
            inline void SetConstraints(const EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& constraints);
            inline const std::unordered_map<image_pair_t, std::pair<size_t, size_t>>&
                GetImagePairs() const;

            inline image_t GetImageId(const std::string& image_path);
            
            std::unordered_set<point3D_t> GetPoint3DIds() const;
            bool IsEmpty();
            
            inline bool ExistsCamera(const camera_t camera_id) const;
            inline bool ExistsCameraBeta(const Camera camera) const;
            inline bool ExistsImage(const image_t image_id) const;
            inline bool ExistsPoint3D(const point3D_t point3D_id) const;
            inline bool ExistsGCP(const point3D_t point3D_id) const;
            inline bool ExistsUserPt(const point3D_t point3D_id) const;
            inline bool ExistsConstraint(const constraint_t constraint_id) const;
            inline bool ExistsImagePair(const image_pair_t pair_id) const;
            bool HasSurveyPoints() const;
            

            bool HasControlPoints() const ;
            bool HasConstraints() const;
            
            bool SetMetadataToCenter();
            bool HasPriorPositionImages() const;
            bool Empty() const;
            bool HasAbsPositionImages() const;
            bool HasAbsPriorPositionImages() const;
            bool HasPositionImages() const;
          
            bool HasImages() const ;
            bool HasRegImages() const;
            bool HasUserTiepoints() const;
            bool HasTiepoints() const;
            
            void TransFormATData(const std::string& dst_srs);
            
            bool TransFormTiepoints(std::string src, std::string dst);
            
            bool TransFormImages(std::string src, std::string dst);
            
            bool TransFormGCPs(std::string src, std::string dst);
            bool ComputeGCPEstimatedXYZ(point3D_t id);
            bool UpdateTiepoints();
            void TriangulateTiePoints();
            
			void UpdataGCPGlobalErrorInfo(std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >>& gcp_error_map, bool istopredict = false);
            void UpdateGCPMeasurementError(int GCP_id, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& estimate_xy, bool istopredict = false);
            void DeleteUserPtMeasurement(point3D_t gcp_id, image_t img_id, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& measurement_error_map_);
            void UpdataUserPtErrorInfo(point3D_t id, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& error_map, bool istopredict =false);
            void UpdataUserTiepointsGlobalErrorInfo(std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >>& gcp_error_map,bool istopredict = false);
            
            void UpdateGivenGCP(int GCP_id, int idx, double value, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& error_map);
            
          
            
            void UpdateGCPError(int GCP_id);
            void UpdateImageObrs(std::set<image_t> imageids);
            
            
            void UpdateGCPMeasurementError(int GCP_id, std::map<image_t, std::pair<double, double>>& error_map);
            void UpdataGCPErrorInfo(point3D_t id, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& error_map, bool istopredict = false);
            
            
           

            
            
            
            
            
            
            void TearDown();

            
            
            void AddCamera(const Camera& camera);
            camera_t ExistsCamera(const Camera& camera);
            
            void AddImage(const AI3D::CORE::Image& image);
            void DeletePoint3D(const point3D_t point3D_id);
            void DeleteImage(const image_t image_id);
            void DeleteGCP(const point3D_t gcp_id);
			void DeleteTiePoints(const std::vector<point3D_t>& ids, std::vector<image_t>& delteImageID);
            void DeleteConstraint(const constraint_t constraint_id);
																							

            
            void DeleteGCPMeasurement(point3D_t gcp_id, image_t img_id,std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& measurement_error_map_);

            bool HasUnRegisteredImages();

            void DeleteGCPs();

            void ResetTriObservations(const image_t image_id,
                const point2D_t point2D_idx,
                const bool is_deleted_point3D);
            void DeleteObservation(const image_t image_id,
                const point2D_t point2D_idx);
            const ABBox3d GetBox();
            
            
            
            
            
            void ExtractATDataByImages(std::set<image_t> ids, ATData& data);
                                                                                  
            void ExtractATDataByTiepoints(std::set<point3D_t> ids, ATData& data);
            
            
            
            

            
           

            
            
            
            
            

            
            

            
            
            
            

            
            

            
            

            
            bool IsImageRegistered(const image_t image_id) const;

            
            void ComputeTileBoundingBox(atpoint_elements_e elements,bool bremoveoutliers =false) ;
            void ComputeTileBoundingBox(bb_scope_e elements, bool bremoveoutliers =false);
            std::tuple<Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d>
                ComputeBoundsAndCentroid() ;
           
            
            
            
            
            
            
            
            
            
            
            void Normalize(const double extent = 10.0, const double p0 = 0.1,
                const double p1 = 0.9, const bool use_images = false);
            
            double GetSceneScale(const double extent = 10.0, const double p0 = 0.1,
                const double p1 = 0.9);
            void FindCommonImages(const ATData& reconstruction,
                std::unordered_set<image_t>& common_image_ids,
                std::unordered_set<image_t>& missing_image_ids);

            const AI3D::CORE::Image* FindImageWithFullName(
                const std::string& name) const;
            AI3D::CORE::Image* FindImageWithFullName(
                const std::string& name) ;
            AI3D::CORE::Image* FindImageWithFullName(
                const std::string& name, std::vector<image_t> imgs_ids);
            void Transform(const SimilarityTransform3& tform);
            
            
            
            
            
            AI3D::CORE::Image* FindImageWithName(const std::string& name, std::vector<image_t> imgs_ids);
           
            bool AddPoses(srs_s srs, std::vector<pose_s>  poses, std::vector<pose_s>& image_remain);

            void FindCommonImages(const ATData& reconstruction, std::set<image_t>& ids1, std::set<image_t>& ids2) const;
            
            
          void FindCommonRegImages(const ATData& reconstruction, std::set<image_t>& ids1, std::set<image_t>& ids2) const;
         
            std::vector<image_t> FindCommonRegImageIds(
                const ATData& reconstruction) const;
            
            srs_s GetDefaultEnuSRS();
            void DeleteUserPt(const point3D_t& point_id);
            
            
            size_t ComputeNumObservations() const;
            double ComputeMeanTrackLength() const;
            double ComputeMeanObservationsPerRegImage() const;
            
            double ComputeMeanReprojectionError() const;
            
            void ComputeSquaredReprojectionErrorForGCP(point3D_t id);
            void Compute3DErrorForGCP(point3D_t GCP_id);
            void ComputeDistErrorForGCP(point3D_t GCP_id);
           
            
            bool ComputePositionOffsetByAvgCenter(
                Eigen::Vector3d& position_offset);
            
            
            bool ComputePositionOffsetByAvgCenter(
                Eigen::Vector3d& position_offset, std::string& local_srs_definition);

            void SetOriginSrs(std::string srs);
            const std::string  GetOriginSrs()  const;
            void SetLocalSrs(std::string srs);
            const std::string  GetLocalSrs()  const;
            
            void SetLocalGcpSrs(std::string local_gcp_srs_definition);
            const std::string  GetLocalGcpSrs()  const;
            camera_t GenerateValidCameraId();
            bool HasValidPriorPositionImages() const;
            bool AreAllImagesPoseComplete() const;
            inline const std::vector<image_t> GetImageIds() const;
            const std::set<image_t> GetImageIdsSet() const;
            bool AreAllImagesRegistered() const;
            point3D_t GenerateValidGCPId();
            point3D_t GenerateValidPoint3DId();
            constraint_t GenerateValidConstraintId();

            bool GenPreviewImages(const std::string& path);
            bool GenPreviewImages(const std::string& path, std::vector<image_t> vecimage);
            
            bool TransformControlPoints(std::string& crs_definition);
            Eigen::Vector3d ComputeAvgPosition();

            void SetPoint3DStatus(image_t image_id);
            bool GetPoint3DsStatus()const;
            bool& GetPoint3DsStatusMutual();
            void SetPoint3DsStatus(bool btiepoints_changed);

            
            void SetTightBox(const ABBox3f& tight_box );
            const ABBox3f& GetTightBox() const;
            ABBox3f& GetTightBoxMutual();
            void SetTileAABBBox(const ABBox3f& box);
            const ABBox3f& GetTileAABBBox() const;
            ABBox3f& GetTileAABBBoxMutual();
            void UpdateUserPtMeasurementError(int gcp_id, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& estimate_xy, bool istopredict);
            bool ComputeUserPtEstimatedXYZ(point3D_t id);
            int WritePoints3DText(const std::string& path) const;
            int WriteImageText(const std::string& path) const;
            void PredictUserPtMeasurement(point3D_t gcp_id, image_t img_id,
                Eigen::Vector2d& estimated_xy, bool checkborder=true, bool istopredict=false);
            
            bool GetSceneUnit() const;
            void PredictUserPtMeasurement(const point3D_t& gcp_id, std::map<image_t, Eigen::Vector2d >& estimated_xys);
            void PredictUserPtMeasurement(const point3D_t& gcp_id, std::set<image_t>& imgids, bool btopredict_ =false);
            
            
            AT_complete_status_e  GetATCompleteStatus();
            point3D_t GenerateValidUserPtId();
            const bool SaveCBBin(const std::string& file) const;
            const bool ShouldCB() const;
            

            std::set<image_t> GetImageIDsTiling() 
            {
             return   imageids_tiling_;;
            }
            std::set<point3D_t> GetPointsIDsTiling() { return point3dids_tiling_; }
            const std::set<point3D_t>& GetPointsIDsTiling() const { return point3dids_tiling_; }
            
            static bool LoadViewsBin(const std::string& filename,std::set<image_t>& ids);

            
            bool IsConstraintScaleSimilarityAlreadyApplied() const;

            
            bool handleConstraint();

            bool LoadConstraint(const std::string& path);
            bool SaveConstraint(const std::string& outpath);
        private:
            std::map<point3D_t, std::vector<viewweight>> point_views_;
            std::map<image_t, std::vector<point3D_t>> view_points_;
            
            
            bool btiepoints_changed_ = false;
            double avg_height_{-DBL_MAX};
            EIGEN_STL_UMAP(camera_t, Camera) cameras_;
            EIGEN_STL_UMAP(image_t,  AI3D::CORE::Image) images_;
            
            
            EIGEN_STL_UMAP(point3D_t, Point3D) user_points3D_;
            EIGEN_STL_UMAP(point3D_t,  ControlPoint) controlpoints_;
            EIGEN_STL_UMAP(point3D_t,  Point3D) points3D_;
            EIGEN_STL_UMAP(std::string, image_t) image_path_to_id_;
            EIGEN_STL_UMAP(constraint_t, MeasureConstraint) constraintList_;
            std::unordered_map<image_pair_t, std::pair<size_t, size_t>> image_pairs_;


            std::vector<image_t> reg_image_ids_;
            
           
            std::string origin_srs_definition_ = srs_s().definition;
            
           
            std::string local_srs_definition_ = srs_s().definition;
            std::string local_gcp_srs_definition_ = srs_s().definition;;

            point3D_t num_added_points3D_;
            Eigen::Vector3d position_offset_;
            Eigen::Vector3d positon_avg_;
            bbox_s box_;
            ABBox3f tile_aabb_box_;
            ABBox3f tight_box_;
            

            std::set<image_t> imageids_tiling_; 
            std::set<point3D_t> point3dids_tiling_;

        };
        template <bool kEstimateScale>
        bool ATData::AlignRobust(const std::vector<std::string>& image_names,
            const std::vector<Eigen::Vector3d>& locations,
            const int min_common_images,
            const RANSACOptions& ransac_options,
            SimilarityTransform3* tform)
        {
            

            
            
            std::unordered_set<image_t> common_image_ids;
            std::vector<Eigen::Vector3d> src;
            std::vector<Eigen::Vector3d> dst;
            for (size_t i = 0; i < image_names.size(); ++i) 
            {
                AI3D::CORE::Image* image = FindImageWithName(image_names[i],GetRegImageIds());
            	if (image == nullptr) {
            		continue;
            	}

            	if (!IsImageRegistered(image->GetImageId())) 
            	{
            		continue;
            	}

            	
            	if (common_image_ids.count(image->GetImageId()) > 0)
            	{
            		continue;
            	}

            	common_image_ids.insert(image->GetImageId());
            	src.push_back(image->GetPosition());
            	dst.push_back(locations[i]);
            }

            
            if (common_image_ids.size() < static_cast<size_t>(min_common_images)) 
            {
            	return false;
            }

            LORANSAC<SimilarityTransformEstimator<3, kEstimateScale>,
            	SimilarityTransformEstimator<3, kEstimateScale>>
            	ransac(ransac_options);

            const auto report = ransac.Estimate(src, dst);

            if (report.support.num_inliers < static_cast<size_t>(min_common_images)) {
            	return false;
            }

            SimilarityTransform3 transform = SimilarityTransform3(report.model);
            Transform(transform);

            if (tform != nullptr) {
            	*tform = transform;
            }

            return true;
        }
    }
} 



#endif  
