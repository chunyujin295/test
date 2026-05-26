#if 000
#ifndef _AI3D_S3DPROJECTFILE_H_
#define _AI3D_S3DPROJECTFILE_H_
#include "Core/BlockObject.h"
#include "Core/Camera.h"
#include "Core/ATData.h"
#include "Core/Image.h"
#include "Core/Point2d.h"
#include "Core/Point3d.h"
#include "Core/Track.h"
#include "Core/alignment.h"
#include "Core/Types.h"
#include "Core/ControlPoint.h"
#include "Core/Bitmap.h"
#include "Core/ATOptions.h"
#include <Core/json.h>
using namespace AI3D::CORE;
namespace AI3D
{
    namespace SMT3D
    {
        struct extvalue_s
        {
            extvalue_s() {};
            Eigen::Matrix3d rotation_;
            Eigen::Vector3d center_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            }
            extvalue_s(nlohmann::json json_str)
            {
                rotation_.setConstant(NAN);
                center_.setConstant(NAN);
                auto it = json_str.find("rotation");
                if (it != json_str.end())
                {
                    auto matrix = json_str.at("rotation");
                  
                    for (int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j < 3; j++)
                        {
                            rotation_(i, j) = matrix[i][j];
                        }
                        
                    }
                    
                }

                if (json_str.find("center") != json_str.end())
                {
                    nlohmann::json centerstr = json_str.at("center");
                 
                    center_.x() = centerstr[0];
                    center_.y() = centerstr[1];
                    center_.z() = centerstr[2];
                   
                }
            };
        };
        struct value0_s
        {
            value0_s() {};
            int width_;
            int height_;
            double focal_length_;
            Eigen::Vector2d principal_point_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            }
            value0_s(nlohmann::json json_str)
            {
                principal_point_.setConstant(NAN);
                width_ = json_str.at("width").get<int>();
                height_ = json_str.at("height").get<int>();
                focal_length_ = json_str.at("focal_length");
                principal_point_[0] = json_str.at("principal_point")[0];
                principal_point_[1] = json_str.at("principal_point")[1];

            };
        };
        struct innerptrdata_s
        {
            innerptrdata_s() {};

            std::vector<double> disto_t2_;
            value0_s value_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            }
            innerptrdata_s(nlohmann::json json_str)
            {

                value_ = json_str.at("value0");
                auto it = json_str.find("disto_t2");
                if (it != json_str.end())
                {
                    nlohmann::json disto_t2str = json_str.at("disto_t2");
                    for (auto d = 0; d < disto_t2str.size(); d++)
                    {
                        disto_t2_.push_back(disto_t2str[d]);
                    }
                }
            };

        };
        struct innerptr_wrapper_s
        {
            innerptr_wrapper_s() {};
            int id_;
            innerptrdata_s innerdata_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            }
            innerptr_wrapper_s(nlohmann::json json_str)
            {
                id_ = json_str.at("id").get<int>();
                innerdata_ = json_str.at("data");
            };

        };
        struct innervaluejson_s
        {
            innervaluejson_s() {};
            int polymorphic_id_;
            std::string polymorphic_name_;
            innerptr_wrapper_s inptr_wrapper_;
            innervaluejson_s(nlohmann::json json_str)
            {
                polymorphic_id_ = json_str.at("polymorphic_id").get<int>();
                if (json_str.find("polymorphic_name") != json_str.end())
                {
                    polymorphic_name_ = json_str.at("polymorphic_name").get<std::string>();
                }
                if (json_str.find("ptr_wrapper") != json_str.end())
                {
                    inptr_wrapper_ = json_str.at("ptr_wrapper");
                }
            }


        };

        struct ptr_wrapperdata_s
        {
            ptr_wrapperdata_s() {};
            std::string local_path_;
            std::string filename_;
            int width_, height_, id_view_, id_intrinsic_, id_pose_;
            bool use_pose_center_prior_ = true;
            Eigen::Vector3d center_weight_{ 1.0,1.0,1.0 };
            Eigen::Vector3d center_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            }
            ptr_wrapperdata_s(nlohmann::json json_str)
            {
                center_.setConstant(NAN);
                local_path_ = json_str.at("local_path").get<std::string>();
                filename_ = json_str.at("filename").get<std::string>();
                width_ = json_str.at("width").get<int>();
                height_ = json_str.at("height").get<int>();
                id_view_ = json_str.at("id_view").get<int>();
                id_intrinsic_ = json_str.at("id_intrinsic").get<int>();
                id_pose_ = json_str.at("id_pose").get<int>();
                if(json_str.find("use_pose_center_prior")!= json_str.end())
                use_pose_center_prior_ = json_str.at("use_pose_center_prior").get<bool>();
                if (json_str.find("center_weight") != json_str.end())
                {
                    center_weight_[0] = json_str.at("center_weight")[0];
                    center_weight_[1] = json_str.at("center_weight")[1];
                    center_weight_[2] = json_str.at("center_weight")[2];
                }
                if (json_str.find("center") != json_str.end())
                {
                    center_[0] = json_str.at("center")[0];
                    center_[1] = json_str.at("center")[1];
                    center_[2] = json_str.at("center")[2];
                }
            };
        };
        struct viewptr_wrapper_s
        {
            viewptr_wrapper_s() {};
            int id_;
            ptr_wrapperdata_s viewdata_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            };
            viewptr_wrapper_s(nlohmann::json json_str)
            {
                id_ = json_str.at("id").get<int>();

                if (json_str.find("data") != json_str.end())
                {
                    viewdata_ = json_str.at("data");
                }
            };

        };

        struct viewvaluejson_s
        {
            viewvaluejson_s() {};
            int polymorphic_id_;
            std::string polymorphic_name_ = "view_priors";
            viewptr_wrapper_s ptr_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            }
            viewvaluejson_s(nlohmann::json json_str)
            {

                polymorphic_id_ = json_str.at("polymorphic_id").get<int>();

                if (json_str.find("ptr_wrapper") != json_str.end())
                {
                    ptr_ = json_str.at("ptr_wrapper");
                }

            };

        };
        struct S3DJSON
        {
            S3DJSON() {};
            std::string sfm_data_version_ = "0.3";
            std::string root_path_ = "";
            std::map<int, viewvaluejson_s> viewsjson_;
            std::map<int, innervaluejson_s> innersjson_;
            std::map<int, extvalue_s> exnersjson_;
            nlohmann::json WriteToJson()
            {
                nlohmann::json jsonstr;
            }
            bool load(const std::string& file)
            {
                std::ifstream ifs(file);
                if (ifs.fail())
                    return false;

                std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                if (str == "")
                    return false;
                S3DJSON v(nlohmann::json::parse(str.begin(), str.end()));
                exnersjson_ = v.exnersjson_;
                innersjson_ = v.innersjson_;
                sfm_data_version_ = v.sfm_data_version_;
                viewsjson_ = v.viewsjson_;
                root_path_ = v.root_path_;
                
                ifs.close();

                return true;
            }


            S3DJSON(nlohmann::json json_str)
            {
                if (json_str.find("views") != json_str.end())
                {
                    nlohmann::json viewsss = json_str.at("views");
                    for (auto& j : viewsss)
                    {
                        int key = j.at("key");
                        if (j.find("value") != j.end())
                        {
                            viewsjson_[key] = j.at("value");
                        }
                    }

                }
                if (json_str.find("extrinsics") != json_str.end())
                {
                    nlohmann::json viewsss = json_str.at("extrinsics");
                    for (const auto& j : viewsss)
                    {
                        int key = j.at("key");
                        if (j.find("value") != j.end())
                        {
                            exnersjson_[key] = j.at("value");
                        }
                    }

                }
                if (json_str.find("intrinsics") != json_str.end())
                {
                    nlohmann::json viewsss = json_str.at("intrinsics");
                    for (const auto& j : viewsss)
                    {
                        int key = j.at("key");
                        if (j.find("value") != j.end())
                        {
                            innersjson_[key] = j.at("value");
                        }
                    }

                }
                if (json_str.find("sfm_data_version") != json_str.end())
                {
                    sfm_data_version_ = json_str.at("sfm_data_version").get<std::string>();
                }
                if (json_str.find("root_path") != json_str.end())
                {
                    root_path_ = json_str.at("root_path").get<std::string>();
                }

            };

            
            void ConvertSMT3DToATData(AI3D::CORE::BlockObject& block)
            {
               
                block.SetId(0);
                ATData data;
                EIGEN_STL_UMAP(group_t, PhotoGroup) groups;
                EIGEN_STL_UMAP(camera_t, Camera) cameras;
               
                EIGEN_STL_UMAP(point3D_t, Point3D) user_points3D;
                EIGEN_STL_UMAP(point3D_t, ControlPoint) controlpoints;
                EIGEN_STL_UMAP(point3D_t, Point3D) points3D;
                EIGEN_STL_UMAP(std::string, image_t) image_path_to_id;
                for (auto& iter : innersjson_)
                {
                    auto camstr = iter.second;
                    Camera camera;
                    camera.SetCameraId(iter.first);
                    camera.SetCameraName(std::to_string(iter.first));
                    camera.SetModelIdFromName("FULL_OPENCV");
                    camera.SetWidth(camstr.inptr_wrapper_.innerdata_.value_.width_);
                    camera.SetHeight(camstr.inptr_wrapper_.innerdata_.value_.height_);

                    camera.SetCameraModelType(CameraModelType_e::Perspective);
                    camera.SetFocalLength(camstr.inptr_wrapper_.innerdata_.value_.focal_length_);
                    camera.SetPrincipalPointX(camstr.inptr_wrapper_.innerdata_.value_.principal_point_.x());
                    camera.SetPrincipalPointY(camstr.inptr_wrapper_.innerdata_.value_.principal_point_.y());

                    cameras[iter.first] = camera;
                    PhotoGroup pg;
                    pg.SetCamera(camera);
                    pg.SetId(camera.GetCameraId());
                    pg.SetName(camera.GetCameraName());
                    groups[iter.first] = pg;     
                 
                }
                data.GetCamerasMutual() = cameras;
                EIGEN_STL_UMAP(group_t, std::set<image_t> ) imageidforgroup;
                
                
                    block.SetBlockSRS(BASESRS);
                    srs_s srs_temp = CoordinateDescriptor::GetSRSFromDefinition(BASESRS);
                    srs_temp.ID = block.ExistSRS(BASESRS);
                for (auto& iter : viewsjson_)
                {
                    auto imgstr = iter.second;
                    Image image;
                    image.SetImageId(iter.first);
                    image.SetCameraId(imgstr.ptr_.viewdata_.id_intrinsic_);
                    image.SetPhotoGroupID(imgstr.ptr_.viewdata_.id_intrinsic_);
                   
                    image.SetPath(imgstr.ptr_.viewdata_.local_path_);
                    image.SetName(imgstr.ptr_.viewdata_.filename_);
                    image.SetPriorSrs(srs_temp);
                    groups.at(imgstr.ptr_.viewdata_.id_intrinsic_).SetGroupPath(imgstr.ptr_.viewdata_.local_path_);
                    imageidforgroup[imgstr.ptr_.viewdata_.id_intrinsic_].insert(iter.first);
                    auto& pricenter = imgstr.ptr_.viewdata_.center_;
                    image.SetPositionPrior(pricenter);
                   
                    int id_pose = imgstr.ptr_.viewdata_.id_pose_;
                    if (exnersjson_.count(id_pose)>0)
                    {

                        
                        auto& rotation = exnersjson_.at(id_pose).rotation_;
                        image.SetRotationMatrix(rotation);
                        image.SetPosition(exnersjson_.at(id_pose).center_);
                       
                        image.SetRegistered(1);
                        data.GetRegImageIdsMutual().push_back(image.GetImageId());
                    }
                    else
                    {
                        image.SetPosition(pricenter);
                    }
                    data.AddImage(image);
                    
                   
                }
                for (auto& iter : imageidforgroup)
                {
                    groups.at(iter.first).SetGroupImage(iter.second);
                }
                data.SetLocalSrs(srs_temp.definition);
                std::shared_ptr<ATData> dataptr = std::make_shared<ATData>(data);
                block.GetPhotoGroupsMutual()=groups;
                if (data.HasRegImages())
                {
                    block.SetStatus(jobsta_e::STATUS_COMPLETE);
                    block.SetAT0(dataptr);
                }
                else
                {
                    block.SetATData(dataptr);
                }
                
                
            }
            
        };
    }
   
}
#endif
#endif