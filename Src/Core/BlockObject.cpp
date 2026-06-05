#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <Constants.h>
#include "Core/File.h"                
#include "Core/Camera.h"
#include "Core/BlockObject.h"
#include "Core/ATData.h"
#include "Core/Image.h"
#include "Core/Point2d.h"
#include "Core/Point3d.h"
#include "Core/Track.h"
#include "Core/alignment.h"
#include "Core/Types.h"
#include "Core/CoordinateSystem.h"
#include "Core/ATGroup.h"
#include "Core/PhotoGroup.h"
#include "Core/Warp.h"
#include "Core/String.h"
#include "Core/TaskDef.h"
#include "Core/Timer.h"                    
#include <filesystem>
#include <Core/Logging.h>
#include <Core/htmlDoc.hpp>
#include <opencv2/opencv.hpp>
#include <Core/Math.h>
#include <Core/DepthMap.h>
#include <freexl.h>
#include <xlnt/workbook/workbook.hpp>
#include <xlnt/workbook/worksheet_iterator.hpp>
#include <xlnt/worksheet/worksheet.hpp>
#include <xlnt/worksheet/range.hpp>
#include <xlnt/worksheet/cell_vector.hpp>
#include <xlnt/utils/path.hpp>
#include <xlnt/cell/cell.hpp>
#include <xlnt/cell/cell_reference.hpp>
#include <iomanip>
#include <sstream>
#include "Util/TaskProcess.h"
#include "Core/ReconstructionCommandSet.h"
#include "Core/ATCommandSet.h"
#include "Core/DataStruct.h"
#ifdef USE_AI3D_PROJ
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/Proj/ProjCore.h"
#endif
namespace AI3D
{
    namespace CORE
    {
        namespace {
        
        inline TrackElement* FindTrackElementByImageMutual(Track& tr, image_t img_id)
        {
            for (auto& ele : tr.GetElementsMutual()) {
                if (ele.image_id == img_id)
                    return &ele;
            }
            return nullptr;
        }
        } 

        IndexImage::IndexImage()
        {

        }
        IndexImage::IndexImage(image_t id, std::string filename) :id_(id), filename_(filename)
        {

        }
        bool operator==(const IndexImage& m1, const IndexImage& m2)
        {
            return (m1.id_ == m2.id_ && m1.filename_ == m2.filename_);
        }

        bool operator!=(const IndexImage& m1, const IndexImage& m2)
        {
            return (m1.filename_ != m2.filename_);
        }
        bool operator<(const IndexImage& m1, const IndexImage& m2)
        {
            return (m1.id_ < m2.id_);
        }

        std::set<BlockObject*> BlockObject::m_setBlockObject;
        bool BlockObject::bChineseVersion = false;

        BlockObject::BlockObject() :
            id_(kInvalidBlockId),
            description_(""),
            type_(TYPE_NONE),
            status_(STATUS_NEW),
            name_(""),
            blockSRS_id_(kInvalidSrsId)
        {
            ATData_ = std::make_shared<ATData>();
            
            {
                std::ostringstream oss;
                oss << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " constructor:"
                    << std::hex << std::showbase << this << std::dec;
            
                
            }

            m_setBlockObject.insert(this);
        }

        

        BlockObject::BlockObject(std::string path) :
            id_(kInvalidBlockId),
            description_(""),
            type_(TYPE_NONE),
            status_(STATUS_NEW),
            name_(""),
            blockSRS_id_(kInvalidSrsId)
        {
            path_ = path;
            ATData_ = std::make_shared<ATData>();

            
            {
                std::ostringstream oss;
                oss << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                    " constructor:" << std::hex << std::showbase << this << std::dec;
                
                
            }

            m_setBlockObject.insert(this);
        }

        BlockObject::BlockObject(const BlockObject& block)
        {       
            auto AT_tmp = std::make_shared<ATData>(*block.ATData_);
            if (ATData_ != nullptr)
            {
                ATData_.reset();
            }
            
            for (auto itr = block.ATGroups_.cbegin(); itr != block.ATGroups_.cend(); itr++)
            {
                ATGroup atgroup_tmp = itr->second;
                ATGroups_[itr->first] = atgroup_tmp;
            }
            photogroups_ = block.photogroups_;
            id_ = block.id_;
            name_ = block.name_;
            path_ = block.path_;
            position_offset_ = block.position_offset_;
            srs_enu_discription_ = block.srs_enu_discription_;
            srs_map_ = block.srs_map_;
            blockSRS_id_ = block.blockSRS_id_;
            image_ids_ = block.image_ids_;
            description_ = block.description_;
            type_ = block.type_;
            status_ = block.status_;
            block_info_ = block.block_info_;
            ismodify_ = block.ismodify_;
            blkversion_ = block.blkversion_;

            ATData_ = AT_tmp;

            
            {
                std::ostringstream oss;
                oss << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " constructor:"
                    << std::hex << std::showbase << this << std::dec;
                
            
            }

            m_setBlockObject.insert(this);
        }

        BlockObject::~BlockObject()
        {
            
            {
                std::ostringstream oss;
                oss << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                    " deconstructor:" << std::hex << std::showbase << this << std::dec;
                
            
            }

            m_setBlockObject.erase(this);
        }

        bool BlockObject::isValidBlockObject(BlockObject* blockObject)
        {
            if (!blockObject || m_setBlockObject.size() <= 0)
                return false;

            if (m_setBlockObject.count(blockObject) > 0)
                return true;

            return false;
        }
        
        BlockObject& BlockObject::operator=(const BlockObject& block)
        {
            if (this != &block)
            {
                auto AT_tmp = std::make_shared<ATData>(*block.ATData_);
                if (ATData_ != nullptr)
                {
                    ATData_.reset();
                }
                
                for (auto itr = block.ATGroups_.cbegin(); itr != block.ATGroups_.cend(); itr++)
                {
                    ATGroup atgroup_tmp = itr->second;
                    ATGroups_[itr->first] = atgroup_tmp;
                }
                photogroups_ = block.photogroups_;
                id_ = block.id_;
                name_ = block.name_;
                path_ = block.path_;
                position_offset_ = block.position_offset_;
                srs_enu_discription_ = block.srs_enu_discription_;
                srs_map_ = block.srs_map_;
                blockSRS_id_ = block.blockSRS_id_;
                image_ids_ = block.image_ids_;
                description_ = block.description_;
                type_ = block.type_;
                status_ = block.status_;
                block_info_ = block.block_info_;
                ismodify_ = block.ismodify_;
                blkversion_ = block.blkversion_;
                ATData_ = AT_tmp;
            }
            return *this;
        }
        void BlockObject::Init()
        {
            name_ = BLOCK_PRE + std::to_string(id_);

            
            block_info_.blockString = name_;
            block_info_.blockId = id_;
            
            path_ = path_ + PATH_SEPARATOR_STR + name_;
        }

        void BlockObject::SetName(const std::string& name)
        {
            name_ = name;
        };
        const std::string BlockObject::GetName() const
        {
            return name_;
        }

        void BlockObject::SetPath(const std::string& path)
        {
            path_ = path;
        }
        std::string& BlockObject::GetPathMutual()
        {
            return path_;
        }

        const std::string BlockObject::GetPath()const
        {
            return path_;
        }
        ;
        std::string& BlockObject::GetNameMutual()
        {
            return name_;
        }
        void BlockObject::SetId(block_t id)
        {
            id_ = id;
        };
        const block_t BlockObject::GetId() const
        {
            return id_;
        };
        block_t& BlockObject::GetIdMutual()
        {
            return id_;
        };


        point3D_t  BlockObject::GetNumControlPoints()
        {
            return GetCurrentAT()->GetNumControlPoints();
        }
        point3D_t  BlockObject::GetNumCheckPoints()
        {
            return GetCurrentAT()->GetNumCheckPoints();
        }
        point3D_t  BlockObject::GetNumValidControlPoints()
        {
            return GetCurrentAT()->GetNumValidControlPoints();
        }
        
        point3D_t  BlockObject::GetNumUserTiePoints()
        {
            return 1;
        }
        std::set<image_t> BlockObject::GetImagesids()
        {
            return image_ids_;
        }
        
        point3D_t  BlockObject::GetNumAutoTiePoints()
        {
            return GetCurrentAT()->GetNumPoints3D();
        }

        image_t BlockObject::GetNumImages()
        {
            return GetCurrentAT()->GetNumImages();
        }

        group_t BlockObject::GetNumPhotoGroup()
        {

            return photogroups_.size();


        }

        bool BlockObject::supportTempLogs()
        {
            return true;
        }

        bool BlockObject::supportOptimization4ProductionListOverview()
        {
            return true;
        }

        bool BlockObject::isChineseVersion()
        {
            
            
            return bChineseVersion;
        }

        void BlockObject::setChineseVersion()
        {
            bChineseVersion = true;
        }

        std::string BlockObject::getJobStringStatus(job_status_e& job_sta)
        {
            if (isChineseVersion())
            {

            }
            else
            {

            }
            return "";
        }

        void BlockObject::SearchImages(const std::string& path, std::vector<std::string>& filenames, std::vector<std::string> image_extension, bool bIncludeSubDir)
        {
            try
            {
                std::filesystem::path p = File::BoostPathFromUtf8(path);

                
                if (bIncludeSubDir)
                {
                    std::filesystem::recursive_directory_iterator end_itr;
                    for (std::filesystem::recursive_directory_iterator itr(p); itr != end_itr; ++itr)
                    {

                        if (std::filesystem::is_regular_file(itr->path()))
                        {
                            std::string filepath = File::BoostPathToUtf8String(itr->path());
                            filenames.push_back(filepath);
                        }
                    }
                }
                else
                {
                    std::filesystem::directory_iterator end_itr;
                    for (std::filesystem::directory_iterator itr(p); itr != end_itr; ++itr)
                    {
                        if (std::filesystem::is_regular_file(itr->path()))
                            
                        {
                            std::string filepath = File::BoostPathToUtf8String(itr->path());

                            filenames.push_back(filepath);
                        }
                    }
                }

                
                for (std::vector<std::string>::iterator it = filenames.begin();
                    it != filenames.end();)
                {
                    std::string extension, root;
                    std::string image_name = *it;
                    File::SplitFileExtension(image_name, &root, &extension);
                    String::StringToLower(&extension);
                    
                    if (std::find(image_extension.begin(), image_extension.end(), extension) == image_extension.end())
                    {

                        it = filenames.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
                
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }
        }

        

        bool BlockObject::Addimages_Beta(const std::vector<std::string>& images, int* cbProgress, bool* bCancel)
        {
            if (GetCurrentAT() == nullptr)
            {
                if (cbProgress)
                    *cbProgress = -1;

                return false;
            }

            if (images.size() <= 0)
            {
                if (cbProgress)
                    *cbProgress = -1;

                return false;
            }

            
            std::set<std::string> existingImagepath;
            for (const auto& image : GetCurrentAT()->GetImages())
            {
                std::string imageFullpath = image.second.GetPath() + PATH_SEPARATOR_STR + image.second.GetName();
                imageFullpath = File::EnsureUnifySlash(imageFullpath);
                existingImagepath.insert(imageFullpath);
            }

            int existcnt = 0;
            int addImageCount = 0;

            std::map<srsid_t, srs_s> imagesrsmap;
            for (int i_img = 0; i_img < images.size(); i_img++)
            {
                std::string imageFullpath = images[i_img];


                
                
                
                
                
                
                

                
                bool isExist = false;

                imageFullpath = File::EnsureUnifySlash(imageFullpath);
                isExist = existingImagepath.insert(imageFullpath).second;
                if (!isExist)
                {                   
                    *cbProgress = ((i_img + 1) * 100 / images.size());
                    existcnt++;


                    continue;
                }
                
                Image img;
                img.SetImageId(GenerateValidImageId());
                img.SetPath(File::GetParentDir(imageFullpath));
                img.SetName(File::GetPathBaseName(imageFullpath));
                img.SetRegistered(false);

                
                std::string extension;

                try
                {
                    extension = File::BoostPathToUtf8String(File::BoostPathFromUtf8(imageFullpath).extension());
                }
                catch (std::filesystem::filesystem_error& fse)
                {
                    std::ostringstream oss;
                    oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();


                    LOGI(oss.str());
                }
                catch (std::exception& ex)
                {
                    std::ostringstream oss;
                    oss << "exception:" << ex.what();

                    LOGI(oss.str());
                }

                String::StringToLower(&extension);

                
                Camera camera;
                camera.SetModelIdFromName("FULL_OPENCV");
                camera.SetCameraModelType(CameraModelType_e::Perspective);
                ExifInfo exif;
                std::string pathtemp = File::GetParentDir(path_);
                pathtemp = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(pathtemp));

                std::string preview_path = File::JoinPaths(pathtemp, "previews");
                File::CreateDirIfNotExists(preview_path);
                
                if (img.ParseExif(preview_path)== ERROR_IMAGE)
                {
                    LOGW(String::StringPrintf("Invalid image %s", imageFullpath.c_str()));
                    if (i_img + 1 == images.size())
                    {
                        *cbProgress = ((i_img + 1) * 100 / images.size());
                    }



                    continue;
                }           
                exif = img.GetExifinfo();
                if (exif.width > 0 && exif.height > 0)
                {
                    camera.SetWidth(exif.width);
                    camera.SetHeight(exif.height);
                }
                if (exif.sensor_width > 0)
                {
                    camera.SetSensorSize(exif.sensor_width);
                }
                camera.SetCameraOrientation("XRightYDown");
                camera.SetMake(exif.make);
                camera.SetMakeModel(exif.model);


                
                if (exif.focalLengthIn35mm >0)
                {
                    
                    camera.SetFocalLengthIn35mm(exif.focalLengthIn35mm);
                    if (camera.GetWidth() > 0 && camera.GetHeight() > 0)
                    {
                        double f_pix = camera.GetFocalLengthIn35mm() * std::max(camera.GetWidth(), camera.GetHeight()) / 36;
                        camera.SetFocalLengthX(f_pix);
                        camera.SetFocalLengthY(f_pix);

                        if (camera.GetSensorSize() > 0)
                        {
                            camera.SetFocalLengthMM(camera.GetFocalLengthIn35mm() * camera.GetWidth() / 36);
                        }
                    }
                }

                if (exif.focalLength >0 )
                {
                    camera.SetFocalLengthMM(exif.focalLength);
                    if (camera.GetSensorSize() > 0)
                    {
                        double f_35mmeq;
                        f_35mmeq = (36 * camera.GetFocalLengthMM()) / camera.GetSensorSize();
                        camera.SetFocalLengthIn35mm(f_35mmeq);

                        if (camera.GetWidth() > 0 && camera.GetHeight() > 0)
                        {
                            double f_pix;
                            f_pix = std::max(camera.GetWidth(), camera.GetHeight()) * camera.GetFocalLengthMM() / camera.GetSensorSize();
                            camera.SetFocalLengthX(f_pix);
                            camera.SetFocalLengthY(f_pix);
                        }                       
                    }
                }

                {
                    
                    if (exif.longitude != -DBL_MAX && exif.latitude != -DBL_MAX && exif.altitude != -DBL_MAX)
                    {
                        srs_s srs;
                        std::string srstemp = GEO84SRS;
                        if (img.GetXmpData().isValid)
                        {
                            srstemp = "EPSG:4326+5773";
                        }
                        
                        
                        auto pos = std::find_if(srs_map_.begin(), srs_map_.end(), [&](const std::pair<srsid_t, srs_s>& srs_temp) {return srs_temp.second.definition == srstemp; });
                        if (pos == srs_map_.end())
                        {                       
                            srs = CoordinateDescriptor::GetSRSFromDefinition(srstemp);
                            srs.ID = GenerateValidSrsId();
                        }
                        else
                        {
                            srs.ID = pos->first;
                            srs = pos->second;
                        }
                        srs_map_.insert(std::make_pair(srs.ID, srs));
                        imagesrsmap[srs.ID] = srs;
                        img.SetPriorSrs(srs);
                        
                        Eigen::Vector3d pos_prior = { exif.longitude,exif.latitude,exif.altitude };
                        img.SetPositionPrior(pos_prior);

                        
                        
                        
                        
                        
                        
                        

                        
                        
                        
                    }

                    double height = img.GetXmpDataMutual().AbsoluteAltitude;
                    if (height != -DBL_MAX && img.HasPositionPrior())
                    {
                        img.GetPositionPriorMutual().z() = height;
                    }
                    if (!exif.rotation.hasNaN())
                    {
                        img.SetRotationMatrixPrior(exif.rotation);
                    }

                    
                    if (img.HasPreCalibParams())
                    {
                        std::vector<double> pre_calib_params;
                        pre_calib_params.resize(12);
                        pre_calib_params[0] = img.GetXmpData().pre_calib_params[0];
                        pre_calib_params[1] = img.GetXmpData().pre_calib_params[1];
                        pre_calib_params[2] = img.GetXmpData().pre_calib_params[2] + exif.width / 2;
                        pre_calib_params[3] = img.GetXmpData().pre_calib_params[3] + exif.height / 2;
                        if (!img.GetXmpData().DewarpFlag)
                        {
                            
                            pre_calib_params[4] = img.GetXmpData().pre_calib_params[4];
                            pre_calib_params[5] = img.GetXmpData().pre_calib_params[5];
                            pre_calib_params[8] = img.GetXmpData().pre_calib_params[6];
                            pre_calib_params[6] = img.GetXmpData().pre_calib_params[7];
                            pre_calib_params[7] = img.GetXmpData().pre_calib_params[8];
                        }
                        camera.SetParams(pre_calib_params);
                    }
                    else
                    {
                        camera.SetParams({ camera.GetFocalLengthX(),camera.GetFocalLengthY(),double(exif.width / 2),double(exif.height / 2),0,0,0,0,0, 0, 0, 0 });
                    }

                }
                
                PhotoGroup pg;
                pg.SetGroupPath(File::GetParentDir(imageFullpath));
                pg.SetExtension(extension);
                pg.SetCamera(camera);
                bool existedPhotoGroup = false;

                addImageCount++;


                for (auto& pg_tmp : photogroups_)
                {
                    if (pg_tmp.second.IsSame_Beta(pg))
                    {
                        img.SetCameraId(pg_tmp.second.GetCamera().GetCameraId());
                        img.SetPhotoGroupID(pg_tmp.second.GetId());
                        pg_tmp.second.AddImageId(img.GetImageId());
                        GetCurrentAT()->AddImage(img);

                        existedPhotoGroup = true;
                    }
                }
                if (!existedPhotoGroup)
                {
                    group_t group_id = GenerateValidPhotoGroupId();
                    camera_t camera_id = group_id;
                    img.SetCameraId(camera_id);
                    img.SetPhotoGroupID(group_id);

                    std::string group_name = GROUPBASENAME + std::string("(") + File::GetDirName(imageFullpath, false) + std::string(")");
                    pg.SetName(group_name);
                    pg.SetId(group_id);
                    pg.AddImageId(img.GetImageId());

                    camera.SetCameraId(camera_id);
                    camera.SetCameraName(group_name);
                    pg.SetCamera(camera);

                    GetCurrentAT()->AddCamera(camera);
                    GetCurrentAT()->AddImage(img);
                    
                    
                    photogroups_[group_id] = pg;


                }
                
                *cbProgress = ((i_img + 1) * 100 / images.size());
            }



            
            if (existcnt > 0)
            {
                
                if (GetCurrentAT()->HasPriorPositionImages())
                {
                    UpdateSRSMap(CoordinateDescriptor::GetSRSFromDefinition(BASESRS));
                    blockSRS_id_ = ExistSRS(BASESRS);
                }
                GetCurrentAT()->SetMetadataToCenter();
            }



            
            if (GetCurrentAT()->HasPriorPositionImages())
            {
                UpdateSRSMap(CoordinateDescriptor::GetSRSFromDefinition(BASESRS));
                blockSRS_id_ = ExistSRS(BASESRS);
                GetCurrentAT()->SetMetadataToCenter();
            }

#ifdef USE_AI3D_PROJ
            for (auto iter : imagesrsmap)
            {
                
                
                
                AI3D::PROJ::CoordinateReferenceSystem crs(iter.second.definition);
                
                

                AI3D::PROJ::CoordinateReferenceSystem::InsertRecentCoordinateReferenceSystem((crs));
            }

#endif 
            
            if (cbProgress)
            {
                if (*cbProgress == 0)
                    *cbProgress = -1;

                if (*cbProgress == -1)
                    return false;
            }

            return true;
        }

        bool BlockObject::Addimages2_Beta(const std::vector<std::string>& images, int* cbProgress, bool* bCancel)
        {
            
            std::set<std::string> existingImagepath;







            int existcnt = 0;
            for (int i_img = 0; i_img < images.size(); i_img++)
            {


                    


                
                bool isExist = false;
                std::string imageFullpath = images[i_img];
            
                imageFullpath = File::EnsureUnifySlash(imageFullpath);








                
                Image img;
                img.SetImageId(GenerateValidImageId());
                img.SetPath(File::GetParentDir(imageFullpath));
                img.SetName(File::GetPathBaseName(imageFullpath));
                img.SetRegistered(false);

                
                std::string extension;

                try
                {
                    extension = File::BoostPathToUtf8String(File::BoostPathFromUtf8(imageFullpath).extension());
                }
                catch (std::filesystem::filesystem_error& fse)
                {
                    std::ostringstream oss;
                    oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                    LOGI(oss.str());
                }
                catch (std::exception& ex)
                {
                    std::ostringstream oss;
                    oss << "exception:" << ex.what();
                    LOGI(oss.str());
                }

                String::StringToLower(&extension);

                
                Camera camera;
                camera.SetModelIdFromName("FULL_OPENCV");
                camera.SetCameraModelType(CameraModelType_e::Perspective);
                ExifInfo exif;
                std::string pathtemp = File::GetParentDir(path_);
                pathtemp = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(pathtemp));

                std::string preview_path = File::JoinPaths(pathtemp, "previews");
                File::CreateDirIfNotExists(preview_path);
                
                if (img.ParseExif(preview_path) == ERROR_IMAGE)
                {
                    LOGW(String::StringPrintf("Invalid image %s", imageFullpath.c_str()));
                    if (i_img + 1 == images.size())
                    {
                        *cbProgress = ((i_img + 1) * 100 / images.size());
                    }
                    continue;
                }
                exif = img.GetExifinfo();
                if (exif.width > 0 && exif.height > 0)
                {
                    camera.SetWidth(exif.width);
                    camera.SetHeight(exif.height);
                }
                if (exif.sensor_width > 0)
                {
                    camera.SetSensorSize(exif.sensor_width);
                }
                camera.SetCameraOrientation("XRightYDown");
                camera.SetMake(exif.make);
                camera.SetMakeModel(exif.model);


                
                if (exif.focalLengthIn35mm > 0)
                {

                    camera.SetFocalLengthIn35mm(exif.focalLengthIn35mm);
                    if (camera.GetWidth() > 0 && camera.GetHeight() > 0)
                    {
                        double f_pix = camera.GetFocalLengthIn35mm() * std::max(camera.GetWidth(), camera.GetHeight()) / 36;
                        camera.SetFocalLengthX(f_pix);
                        camera.SetFocalLengthY(f_pix);

                        if (camera.GetSensorSize() > 0)
                        {
                            camera.SetFocalLengthMM(camera.GetFocalLengthIn35mm() * camera.GetWidth() / 36);
                        }
                    }
                }
                if (exif.focalLength > 0)
                {
                    camera.SetFocalLengthMM(exif.focalLength);
                    if (camera.GetSensorSize() > 0)
                    {
                        double f_35mmeq;
                        f_35mmeq = (36 * camera.GetFocalLengthMM()) / camera.GetSensorSize();
                        camera.SetFocalLengthIn35mm(f_35mmeq);

                        if (camera.GetWidth() > 0 && camera.GetHeight() > 0)
                        {
                            double f_pix;
                            f_pix = std::max(camera.GetWidth(), camera.GetHeight()) * camera.GetFocalLengthMM() / camera.GetSensorSize();
                            camera.SetFocalLengthX(f_pix);
                            camera.SetFocalLengthY(f_pix);
                        }

                    }
                }

                {
                    
                    if (exif.longitude != -DBL_MAX && exif.latitude != -DBL_MAX && exif.altitude != -DBL_MAX)
                    {
                        srs_s srs;
                        std::string srstemp = GEO84SRS;
                        if (img.GetXmpData().isValid)
                        {
                            srstemp = "EPSG:4326+5773";
                        }
                        
                        
                        auto pos = std::find_if(srs_map_.begin(), srs_map_.end(), [&](const std::pair<srsid_t, srs_s>& srs_temp) {return srs_temp.second.definition == srstemp; });
                        if (pos == srs_map_.end())
                        {

                            srs = CoordinateDescriptor::GetSRSFromDefinition(srstemp);
                            srs.ID = GenerateValidSrsId();
                        }
                        else
                        {
                            srs.ID = pos->first;
                            srs = pos->second;
                        }
                        srs_map_.insert(std::make_pair(srs.ID, srs));
                        img.SetPriorSrs(srs);
                        
                        Eigen::Vector3d pos_prior = { exif.longitude,exif.latitude,exif.altitude };
                        img.SetPositionPrior(pos_prior);
                        
                    }
                    
                    double height = img.GetXmpDataMutual().AbsoluteAltitude;
                    if (height != -DBL_MAX && img.HasPositionPrior())
                    {
                        img.GetPositionPriorMutual().z() = height;
                    }
                    if (!exif.rotation.hasNaN())
                    {
                        img.SetRotationMatrixPrior(exif.rotation);
                    }
                    
                    if (img.HasPreCalibParams())
                    {
                        std::vector<double> pre_calib_params;
                        pre_calib_params.resize(12);
                        pre_calib_params[0] = img.GetXmpData().pre_calib_params[0];
                        pre_calib_params[1] = img.GetXmpData().pre_calib_params[1];
                        pre_calib_params[2] = img.GetXmpData().pre_calib_params[2] + exif.width / 2;
                        pre_calib_params[3] = img.GetXmpData().pre_calib_params[3] + exif.height / 2;
                        if (!img.GetXmpData().DewarpFlag)
                        {
                            
                            pre_calib_params[4] = img.GetXmpData().pre_calib_params[4];
                            pre_calib_params[5] = img.GetXmpData().pre_calib_params[5];
                            pre_calib_params[8] = img.GetXmpData().pre_calib_params[6];
                            pre_calib_params[6] = img.GetXmpData().pre_calib_params[7];
                            pre_calib_params[7] = img.GetXmpData().pre_calib_params[8];
                        }
                        camera.SetParams(pre_calib_params);
                    }
                    else
                    {
                        camera.SetParams({ camera.GetFocalLengthX(),camera.GetFocalLengthY(),double(exif.width / 2),double(exif.height / 2),0,0,0,0,0, 0, 0, 0 });
                    }

                }
                
                PhotoGroup pg;
                pg.SetGroupPath(File::GetParentDir(imageFullpath));
                pg.SetExtension(extension);
                pg.SetCamera(camera);
                bool existedPhotoGroup = false;
                for (auto& pg_tmp : photogroups_)
                {
                    if (pg_tmp.second.IsSame_Beta(pg))
                    {
                        img.SetCameraId(pg_tmp.second.GetCamera().GetCameraId());
                        img.SetPhotoGroupID(pg_tmp.second.GetId());
                        pg_tmp.second.AddImageId(img.GetImageId());
                        GetCurrentAT()->AddImage(img);
                        existedPhotoGroup = true;
                    }
                }
                if (!existedPhotoGroup)
                {
                    group_t group_id = GenerateValidPhotoGroupId();
                    camera_t camera_id = group_id;
                    img.SetCameraId(camera_id);
                    img.SetPhotoGroupID(group_id);

                    std::string group_name = GROUPBASENAME + std::string("(") + File::GetDirName(imageFullpath, false) + std::string(")");
                    pg.SetName(group_name);
                    pg.SetId(group_id);
                    pg.AddImageId(img.GetImageId());

                    camera.SetCameraId(camera_id);
                    camera.SetCameraName(group_name);
                    pg.SetCamera(camera);

                    GetCurrentAT()->AddCamera(camera);
                    GetCurrentAT()->AddImage(img);
                    
                    
                    photogroups_[group_id] = pg;
                }
                
                *cbProgress = ((i_img + 1) * 100 / images.size());
            }



                








            return true;
        }

        reconstruction_t BlockObject::GenerateValidReconstructionId()
        {
            reconstruction_t group_id = kInvalidReconstructionId;
            std::set<reconstruction_t> resconst_ids_;
            for (auto& it : reconstructions_)
            {
                resconst_ids_.insert(it.first);
            }

            if (!resconst_ids_.empty())
            {
                group_id = *resconst_ids_.rbegin();
            }
            group_id++;
            
            return group_id;
            
        }

        bool BlockObject::ExistsReconstruction(reconstruction_t reconst_id)
        {
            return reconstructions_.find(reconst_id) != reconstructions_.end();
        }

        ReconstructionObject* BlockObject::GetReconstruction(reconstruction_t reconst_id)
        {
            if (!ExistsReconstruction(reconst_id))
                return nullptr;

            return reconstructions_[reconst_id];
        }

        
        bool BlockObject::DeleteReconstruction(reconstruction_t id)
        {
            if (!ExistsReconstruction(id))
            {
                return false;
            }
        
            std::string reconst_dir = reconstructions_.at(id)->GetPath();

            
            try
            {
                if (std::filesystem::exists(File::BoostPathFromUtf8(reconst_dir)))
                {
                    
                    CHECK_OPTION(File::Remove(reconst_dir));
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            reconstructions_.erase(id);
            
            return true;
        }

        std::vector<std::string> BlockObject::GetReconstructionNames()
        {
            std::vector<std::string> names;
            for (auto& iter : reconstructions_)
            {
                names.push_back(iter.second->GetName());
            }
            return names;
        }

        void  BlockObject::CloneReconstruction(const reconstruction_t reconstruction_id, reconstruction_t& new_reconstruction_id)
        {
            
            ReconstructionObject* object = GetReconstructionsMutual().at(reconstruction_id);
            ReconstructionObject* objectnew = new ReconstructionObject(object->GetBlockId());
            objectnew->CopyBase(*object);
            ReconstructionObject* newobject = objectnew;
            
            
            std::string newname = newobject->GetNameMutual() + "-copy";
            std::vector<std::string> names = BlockObject::GetReconstructionNames();
             
            String::MakeDuplicatedName(names, newname);
            newobject->GetNameMutual() = newname;

            newobject->SetId(GenerateValidReconstructionId());
            std::string recpath = path_ + "/" + newobject->GetIDString();
            recpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(recpath)));
            newobject->SetPath(recpath);
            AddReconstruction(newobject);
            
            std::vector<std::string> files;
            
            
            std::string currentreconstructiodir = object->GetPath();
            
            std::string srsFile = "";
            if (SRS_USE_BIN) {
                srsFile = SRSBIN;
            }
            else {
                srsFile = SRSJSON;
            }
            std::string localFile = File::EnsureUnifySlash(currentreconstructiodir+"/" + srsFile);
            if( File::ExistsFile(localFile))
                files.push_back(localFile);
            std::string viewsfile = File::EnsureUnifySlash(currentreconstructiodir + "/views.xml");
            if (File::ExistsFile(viewsfile))
                files.push_back(viewsfile);
            std::string viewsbin = File::EnsureUnifySlash(currentreconstructiodir + "/" + PRODUCTIONVIEWIDSBIN);
            if (File::ExistsFile(viewsbin))
                files.push_back(viewsbin);
            File::CreateDirIfNotExists(newobject->GetPath());
            File::CopyFiles(files, newobject->GetPath(),false);
            new_reconstruction_id = newobject->GetId();
        }

        void BlockObject::AddReconstruction(ReconstructionObject* object)
        {       
            
            if (object->GetIdMutual() == kInvalidReconstructionId)
            {
                auto id = GenerateValidReconstructionId();
                object->GetIdMutual() = id;
            }
            if(object->GetNameMutual() =="")
            {
                std::string name = RECONSTRUCT_PREFIX + std::to_string(object->GetIdMutual());
                object->SetName(name);
            }
            
            if (object->GetPath() == "" && path_!="")
            {
                std::string recpath = path_ + "/" + object->GetIDString();
                recpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(recpath)));
                object->SetPath(recpath);
            }
            reconstructions_[object->GetId()] = object;
        }

        

        group_t BlockObject::GenerateValidPhotoGroupId()
        {
            group_t group_id = kInvalidGroupId;
            std::set<group_t> photogroup_ids_;
            for (auto& it : photogroups_)
            {
                photogroup_ids_.insert(it.first);
            }

            if (!photogroup_ids_.empty())
            {
                group_id = *photogroup_ids_.rbegin();
            }
            group_id++;
            
            return group_id;
        }

        srsid_t BlockObject::GenerateValidSrsId()
        {
            srsid_t srs_id = kInvalidSrsId;

            std::set<srsid_t> srs_ids_;
            for (auto& it : srs_map_)
            {
                srs_ids_.insert(it.first);
            }
            if (!srs_ids_.empty())
            {
                srs_id = *srs_ids_.rbegin();
            }
            srs_id++;
            
            return srs_id;
        }

        image_t BlockObject::GenerateValidImageId()
        {
            image_t image_id = kInvalidImageId;
            if (!image_ids_.empty())
            {
                image_id = *image_ids_.rbegin();
            }
            image_id++;
            image_ids_.insert(image_id);
            return image_id;
        }

        srs_s BlockObject::ComputeEnuSRS()
        {
            srs_s enu_srs;
            if (blockSRS_id_ != kInvalidSrsId)
            {
                srs_s block_srs = srs_map_.at(blockSRS_id_);
                if (block_srs.type != coord_system_type_e::LOCAL_ENU)
                {
                    enu_srs = GetCurrentATMutual()->GetDefaultEnuSRS();
                    srsid_t id = ExistSRS(enu_srs.definition);
                    if (id == kInvalidSrsId)
                    {
                        enu_srs.ID = GenerateValidSrsId();
                        srs_map_.insert(std::make_pair(enu_srs.ID, enu_srs));
                    }
                    else
                    {
                        enu_srs.ID = id;
                    }
                }
                else
                {
                    enu_srs = block_srs;
                }
            }
            srs_enu_discription_ = enu_srs;
            
            return srs_enu_discription_;
        }
        
        std::shared_ptr<ATData> BlockObject::GetCurrentATMutual()
        {

            if (status_ == STATUS_COMPLETE)
            {
                if (ATGroups_.count(0))
                {
                    return ATGroups_.at(0).GetATDataMutual();
                }
                else
                {
                    LOGF("no atdata when complete.");
                    return std::shared_ptr<ATData>();
                }
            }
            else
            {
                return ATData_;

            }

        };

        const std::shared_ptr<ATData> BlockObject::GetCurrentAT()const
        {
            if (status_ == STATUS_COMPLETE)
            {
                
                return ATGroups_.at(0).GetATData();

            }
            else
            {
                return ATData_;

            }
        }
        

        bool BlockObject::SetGroup(std::map<std::string, std::vector<std::shared_ptr<Image> > >& photogroup_map)
        {
            group_t group_id = 0;
            for (auto& it : photogroup_map)
            {

                PhotoGroup photogroup;
                photogroup.SetName(it.first);
                photogroup.SetId(group_id);
                
                photogroups_[group_id] = photogroup;
                group_id++;
            }
            return true;
        }

        const PhotoGroup& BlockObject::GetGroup(group_t id)
        {
            if (!photogroups_.count(id))
            {
                return PhotoGroup();
            }
            return photogroups_.at(id);

        }


        void BlockObject::SetTaskInfo(const Task_Info& taskinfo)
        {
            block_info_ = taskinfo;
        }
        BlockObject::Task_Info BlockObject::GetTaskInfo()const
        {
            return block_info_;
        }
        BlockObject::Task_Info& BlockObject::GetTaskInfoMutual()
        {
            return block_info_;
        }
        
        bool BlockObject::UpdateSensorSize(Camera& camera)
        {
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }
            for (auto& it : photogroups_)
            {
                if (it.second.GetCameraMutual().IsMakerSame(camera))
                {
                    camera_t camera_id = GetCurrentATMutual()->GetImageMutual(*it.second.GetGroupImageIds().begin()).GetCameraId();
                    if (camera_id == camera.GetCameraId())
                    {
                        
                        continue;
                    }
                                                                       
                    Camera& cam = GetCurrentATMutual()->GetCameraMutual(camera_id);
                    
        
        
                    if ( camera.GetSensorSize() > 0)
                    {
                        it.second.GetCameraMutual().SetSensorSize(camera.GetSensorSize());
        
                        cam.SetSensorSize(camera.GetSensorSize());
                        double f_35eq = 36.0 * it.second.GetCameraMutual().GetFocalLengthMM() / camera.GetSensorSize();
                        it.second.GetCameraMutual().SetFocalLengthIn35mm(f_35eq);
                        cam.SetFocalLengthIn35mm(f_35eq);
        
                        double f_pix = it.second.GetCameraMutual().GetFocalLengthMM() * std::max(it.second.GetCamera().GetHeight(), it.second.GetCamera().GetHeight()) / camera.GetSensorSize();
                        it.second.GetCameraMutual().SetFocalLengthX(f_pix);
                        it.second.GetCameraMutual().SetFocalLengthY(f_pix);
        
                        cam.SetFocalLengthX(f_pix);
                        cam.SetFocalLengthY(f_pix);
                    }
                    else
                    {
                        double f_pix = UNDEFINEDVAL;
                        it.second.GetCameraMutual().SetSensorSize(UNDEFINEDVAL);
        
                        cam.SetSensorSize(UNDEFINEDVAL);
                        
                        it.second.GetCameraMutual().SetFocalLengthIn35mm(f_pix);
                        cam.SetFocalLengthIn35mm(f_pix);
        
                    
                        it.second.GetCameraMutual().SetFocalLengthX(f_pix);
                        it.second.GetCameraMutual().SetFocalLengthY(f_pix);
        
                        cam.SetFocalLengthX(f_pix);
                        cam.SetFocalLengthY(f_pix);
                    }
                }
            }
            return true;
        }
        
        bool BlockObject::UpdateCameraInfo(cam_para_e type, double value, group_t group_id)
        {
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }
            if (group_id < 0)
            {
                return false;
            }
            PhotoGroup& group = photogroups_[group_id];
            Camera& camera = group.GetCameraMutual();
            if (camera.GetCameraId() < 0)
            {
                return false;
            }
        
            
            for (const auto& id : group.GetGroupImageIds())
            {
                GetCurrentATMutual()->GetImageMutual(id).SetRegistered(0);
            }
        
            if (type == SENSOR_SIZE || (type == PIXEL_SIZE))
            {
                
                
                
        
                camera_t cam_id_temp = GetCurrentATMutual()->GetImageMutual(*group.GetGroupImageIds().begin()).GetCameraId();
                Camera& cam = GetCurrentATMutual()->GetCameraMutual(cam_id_temp);
                
        
                if ((value > 0))
                {
                    
                    if (type == PIXEL_SIZE)
                    {
                        camera.SetPixelSize(value);
                        cam.SetPixelSize(value);
                        double sensorvalue =  0.001*value * std::max(cam.GetHeight(), camera.GetWidth()) ;
                        camera.SetSensorSize(sensorvalue);
                        cam.SetSensorSize(sensorvalue);
                    }
                    else
                    {
                        camera.SetSensorSize(value);
                        cam.SetSensorSize(value);
                        
                        double pixlesize =value*1000 /std::max(cam.GetHeight(), camera.GetWidth()) ;
                        camera.SetPixelSize(pixlesize);
                        cam.SetPixelSize(pixlesize);
                    }
                    if (cam.GetFocalLengthMM() > 0)
                    {
                        cam.SetFocalLengthIn35mm(cam.GetFocalLengthMM() * 36.0 / value);
                        camera.SetFocalLengthIn35mm(cam.GetFocalLengthMM() * 36.0 / value);

                        
                        double f_pix = cam.GetFocalLengthMM() * std::max(cam.GetHeight(), camera.GetWidth()) / value;
                        cam.SetFocalLengthX(f_pix);
                        cam.SetFocalLengthY(f_pix);

                        camera.SetFocalLengthX(f_pix);
                        camera.SetFocalLengthY(f_pix);
                    }
                    
                }
                else
                {
                    camera.SetSensorSize(UNDEFINEDVAL);
                    cam.SetSensorSize(UNDEFINEDVAL);
                    cam.SetFocalLengthIn35mm(UNDEFINEDVAL);
                    camera.SetFocalLengthIn35mm(UNDEFINEDVAL);
                    double f_pix = UNDEFINEDVAL;
                    cam.SetFocalLengthX(f_pix);
                    cam.SetFocalLengthY(f_pix);
        
                    camera.SetFocalLengthX(f_pix);
                    camera.SetFocalLengthY(f_pix);
                    
                }
                
                
                UpdateSensorSize(camera);
            }
            
            else if (type == FOCAL)
            {
                
                
        
                camera_t cam_id_temp = GetCurrentATMutual()->GetImageMutual(*group.GetGroupImageIds().begin()).GetCameraId();
                Camera& cam = GetCurrentATMutual()->GetCameraMutual(cam_id_temp);
                
                
                if ( value >= 0)
                {
                    camera.SetFocalLengthMM(value);
                    cam.SetFocalLengthMM(value);
                    if (cam.GetSensorSize() > 0)
                    {
                        cam.SetFocalLengthIn35mm(value * 36.0 / cam.GetSensorSize());
                        camera.SetFocalLengthIn35mm(value * 36.0 / cam.GetSensorSize());

                        
                        double f_pix = value * std::max(cam.GetHeight(), camera.GetWidth()) / cam.GetSensorSize();
                        cam.SetFocalLengthX(f_pix);
                        cam.SetFocalLengthY(f_pix);

                        camera.SetFocalLengthX(f_pix);
                        camera.SetFocalLengthY(f_pix);
                    }
                }
                else
                {
                    double f_pix = UNDEFINEDVAL;
                    camera.SetFocalLengthMM(f_pix);
                    cam.SetFocalLengthMM(f_pix);
                    cam.SetFocalLengthIn35mm(f_pix);
                    camera.SetFocalLengthIn35mm(f_pix);
        
                    
                    cam.SetFocalLengthX(f_pix);
                    cam.SetFocalLengthY(f_pix);
        
                    camera.SetFocalLengthX(f_pix);
                    camera.SetFocalLengthY(f_pix);
                }
        
            }
            else if (type == FOCAL_IN35MM)
            {
                camera.SetFocalLengthIn35mm(value);
        
                camera_t cam_id_temp = GetCurrentATMutual()->GetImageMutual(*group.GetGroupImageIds().begin()).GetCameraId();
                Camera& cam = GetCurrentATMutual()->GetCameraMutual(cam_id_temp);
                cam.SetFocalLengthIn35mm(value);
            }
            else if (type == PPX)
            {
                camera.SetPrincipalPointX(value);
                for (auto& img : group.GetGroupImageIds())
                {
                    camera_t cam_id_temp = GetCurrentATMutual()->GetImageMutual(img).GetCameraId();
                    Camera& cam = GetCurrentATMutual()->GetCameraMutual(cam_id_temp);
                    cam.SetPrincipalPointX(value);
                }
            }
            else if (type == PPY)
            {
                camera.SetPrincipalPointY(value);
                for (auto& img : group.GetGroupImageIds())
                {
                    camera_t cam_id_temp = GetCurrentATMutual()->GetImageMutual(img).GetCameraId();
                    Camera& cam = GetCurrentATMutual()->GetCameraMutual(cam_id_temp);
                    cam.SetPrincipalPointY(value);
                }
            }
            else  if (type == K1 || type == K2 || type == K3 || type == P1 || type == P2)
            {
                if (value < 0)
                {
                    for (int i = 4; i < camera.GetParamsMutual().size(); i++)
                    {
                        camera.GetParamsMutual()[i] = 0;
                    }
        
                    for (auto& img : group.GetGroupImageIds())
                    {
                        camera_t cam_id_temp = GetCurrentATMutual()->GetImageMutual(img).GetCameraId();
                        Camera& cam = GetCurrentATMutual()->GetCameraMutual(cam_id_temp);
                        cam.SetPrincipalPointY(value);
                        for (int i = 4; i < cam.GetParamsMutual().size(); i++)
                        {
                            cam.GetParamsMutual()[i] = 0;
                        }
                    }
                }
                else
                {
                    int index = 4;
                    if (type == K1)
                        index = 4;
                    else if (type == K2)
                        index = 5;
                    else  if (type == K3)
                        index = 6;
                    else  if (type == P1)
                        index = 7;
                    else  if (type == P2)
                        index = 8;
                    
                    camera.GetParamsMutual()[index] = value;
                    for (auto& img : group.GetGroupImageIds())
                    {
                        camera_t cam_id_temp = GetCurrentATMutual()->GetImageMutual(img).GetCameraId();
                        Camera& cam = GetCurrentATMutual()->GetCameraMutual(cam_id_temp);
                        cam.SetPrincipalPointY(value);
                    
                        {
                            cam.GetParamsMutual()[index] = value;
                        }
                    }
                    
                }
            }
            
            status_ = job_status_e::STATUS_NEW;
            return true;
        }
        


        bool BlockObject::AddPoses(srs_s srs, std::vector<pose_s>  poses)
        {
            std::vector<pose_s> image_remain;
            bool ret = false;
            ret = GetCurrentATMutual()->AddPoses(srs, poses, image_remain);
            if (!ret)
            {
                LOGI("add pose(s) failed.");
                return ret;
            }
            if (GetCurrentATMutual()->UpdateTiepoints())
            {
                GetCurrentATMutual()->SetPoint3DsStatus(true);
            }
            std::string at_srs_definition = GetCurrentATMutual()->GetLocalSrs();
            
#ifdef USE_POS_DEBUG



            if (image_remain.size() > 0)
            {
                Camera cam;
                cam.SetCameraId(GetCurrentATMutual()->GenerateValidCameraId());
                cam.SetModelIdFromName("FULL_OPENCV");
                
                
                PhotoGroup group;
                std::string name = "00";
                name = GROUPBASENAME + name;
                group.SetName(name);
                group.SetGroupPath("");
                group.SetId(GenerateValidPhotoGroupId());
                temp_posgroup_id_ = group.GetId();
                group.SetCamera(cam);
                GetCurrentATMutual()->AddCamera(cam);

                std::vector<pose_s>  poses_srs(image_remain);
                std::vector<double> x, y, z;
                for (image_t i = 0; i < poses_srs.size(); i++)
                {
                    Eigen::Vector3d xyz = poses_srs[i].metadata_.center;
                    Eigen::Matrix3d& R = poses_srs[i].metadata_.rotation;
                    CoordinateTransformer::TransformRotation(xyz, R,
                        srs, CoordinateDescriptor::GetSRSFromDefinition(at_srs_definition));
                    x.push_back(xyz.x());
                    y.push_back(xyz.y());
                    z.push_back(xyz.z());
                }
                CoordinateTransformer::Transform(x.size(), &x[0], &y[0], &z[0], srs.definition,
                    CoordinateDescriptor::GetSRSFromDefinition(at_srs_definition).definition);
                for (image_t i = 0; i < poses_srs.size(); i++)
                {
                    poses_srs[i].metadata_.center.x() = x[i];
                    poses_srs[i].metadata_.center.y() = y[i];
                    poses_srs[i].metadata_.center.z() = z[i];
                }

                for (auto& it : image_remain)
                {
                    image_t img_id = GenerateValidImageId();

                    Image image;
                    image.SetName(it.name);
                    image.SetImageId(img_id);
                    image.SetCameraId(cam.GetCameraId());
                    image.SetPositionPrior(it.metadata_.center);
                    image.SetRotationMatrixPrior(it.metadata_.rotation);
                    image.SetPriorSrs(srs);
                    group.AddImageId(img_id);

                    image_ids_.insert(img_id);
                    Eigen::Vector3d xyz = image.GetPositionPriorMutual();
                    Eigen::Matrix3d R = image.GetRotationMatrixPriorMutual();

                      
                      
                    image.GetPositionMutual() = poses_srs[i].metadata_.center;;
                    image.GetRotationMatrixPriorMutual() = poses_srs[i].metadata_.rotation;
                    GetCurrentATMutual()->AddImage(image);
                }
                photogroups_[group.GetId()] = group;


            }
#endif 
            UpdateSRSMap(srs);

            if (at_srs_definition != "")
            {
                srs_s srs_tmp;
                srs_tmp.definition = at_srs_definition;
                UpdateSRSMap(srs_tmp);
                if (blockSRS_id_ == kInvalidSrsId)
                {
                    blockSRS_id_ = ExistSRS(at_srs_definition);
                }
            }


            return true;
        }
    
        bool BlockObject::AddGCPs()
        {
            return true;
        }
        
        bool BlockObject::ClearPoses(const std::vector<group_t>  ids)
        {
            if (ids.empty())
            {
                return false;
            }
            LOGI("Clearing Poses...");
            std::set<image_t> imageids;
            for (auto& id : ids)
            {
                for (auto& it : photogroups_[id].GetGroupImageIds())
                {
                    imageids.insert(it);
                    
                    
                    
                    
                    
                    
                    
                    
                }
            }
            if (imageids.empty())
            {
                return false;
            }
            ClearPoses(imageids);
            
            
            
            
            
            
            
            

            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            return true;
        }
        bool BlockObject::ClearPoses(const std::set<image_t>  ids)
        {
            GetCurrentATMutual()->ClearPose(ids);
            if (GetCurrentATMutual()->UpdateTiepoints())
            {
                GetCurrentATMutual()->SetPoint3DsStatus(true);
            }
            if (!GetCurrentATMutual()->HasPositionImages())
            {
                blockSRS_id_ = kInvalidSrsId;
                
            }
            
            if (!CanSubmitRecon())
            {
                ChangeStatus(jobsta_e::STATUS_NEW);
            }
            
            return true;
        }

        void BlockObject::ChangeStatus(job_status_e status)
        {
            if (status_ == status)
                return;
            
            
            if (ATGroups_.count(0))
            {
                
                ATData_ = ATGroups_.at(0).GetATDataMutual();
                ATGroups_.clear();
                status_ = status;
                Task_Info newinfo;
                newinfo.blockId = block_info_.blockId;

                newinfo.blockString = block_info_.blockString;
                newinfo.Block_XML = block_info_.Block_XML;
                newinfo.isLoaded = block_info_.isLoaded;
                newinfo.Tiepoints = block_info_.Tiepoints;
                
                block_info_ = newinfo;
            }
            else
            {
                return;
            }
        }
        bool BlockObject::HasUserTiePoints() const
        {
            return GetCurrentAT()->HasUserTiepoints();
        }
    
        bool BlockObject::HasControlPoints() const
        {
            return GetCurrentAT()->HasControlPoints();
        }
        bool BlockObject::HasSurveyPoints() const
        {
            return GetCurrentAT()->HasSurveyPoints();
        }

        bool BlockObject::HasReconstructions() const
        {
            return !reconstructions_.empty();
        }
        bool BlockObject::HasReconstruction(reconstruction_t id) const
        {
            return reconstructions_.count(id);
        }

        bool BlockObject::PhotoGroupHasElement(group_t group_id) 
        {
            bool haselement = false;
            PhotoGroup& group = photogroups_[group_id];
            auto ids = group.GetGroupImageIds();
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }
            for (auto id : ids)
            {
                Image image = GetCurrentAT()->GetImage(id);
                if (image.HasGCPs())
                {
                    haselement = true;
                    break;
                }
            }
            return haselement;
        }
        
        bool BlockObject::RemoveImages(std::set<image_t> imageids)
        {
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }
            
            if (!btiepoint_loaded_)
            {
                if (LoadTiepointsBinary(block_info_.Tiepoints, GetCurrentATMutual()))
                {
                    btiepoint_loaded_ = true;
                }
                
                
                
                
            }
            
            for (auto image_id : imageids)
            {
                if ((int)image_id < 0)
                {
                   continue;;
                }
                const auto& image = GetCurrentATMutual()->GetImage(image_id);
                auto groupids = photogroups_.at(image.GetPhotoGroupID()).GetGroupImageIds();
                groupids.erase(image_id);
                if (groupids.empty())
                {
                    photogroups_.erase(image.GetPhotoGroupID());
                    GetCurrentATMutual()->GetCamerasMutual().erase(image.GetPhotoGroupID());
                }
                else
                {
                    photogroups_.at(image.GetPhotoGroupID()).SetGroupImage(groupids);
                }
                GetCurrentATMutual()->DeleteImage(image_id);
            }
            if (GetCurrentATMutual()->UpdateTiepoints())
            {
                GetCurrentATMutual()->SetPoint3DsStatus(true);
            }
            if (!CanSubmitRecon())
            {
                ChangeStatus(jobsta_e::STATUS_NEW);
            }
            return true;
        }

        
        bool BlockObject::RemovePhotoGroup(std::vector<group_t> ids)
        {
            if (!btiepoint_loaded_)
            {
                LoadTiepointsBinary(block_info_.Tiepoints, GetCurrentATMutual());
                btiepoint_loaded_ = true;
            }
            
            for (auto id : ids)
            {
                LOGI(String::StringPrintf("Removing photogroup %d:", id));
                PhotoGroup& group = photogroups_[id];
                for (auto& it : group.GetGroupImageIds())
                {
                    GetCurrentATMutual()->DeleteImage(it);
                }

                group.ClearImage();
                GetCurrentATMutual()->GetCamerasMutual().erase(id);
                photogroups_.erase(id);
            }
            if (GetCurrentATMutual()->UpdateTiepoints())
            {
                GetCurrentATMutual()->SetPoint3DsStatus(true);
            }
            
            if (photogroups_.empty())
            {
                
                

                position_offset_ = { -DBL_MAX,-DBL_MAX, -DBL_MAX };
                srs_map_.erase(srs_enu_discription_.ID);
                srs_enu_discription_ = srs_s();

                srs_map_.erase(blockSRS_id_);

                blockSRS_id_ = kInvalidSrsId;


                image_ids_.clear();

                std::string description_ = "";
                type_ = bt_e();
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                


            }
            if (!CanSubmitRecon())
            {
                
                ChangeStatus(jobsta_e::STATUS_NEW);
            }
            return true;
        }



        EIGEN_STL_UMAP(group_t, PhotoGroup) BlockObject::GetPhotoGroups() const
        {
            return photogroups_;
        }

        EIGEN_STL_UMAP(group_t, PhotoGroup)& BlockObject::GetPhotoGroupsMutual()
        {
            return photogroups_;
        }
        
        bool BlockObject::DeleteImages(const std::vector<image_t> ids, const group_t group_id)
        {
            LOGI(String::StringPrintf("Deleting %d images.", ids.size()));
            PhotoGroup& group = photogroups_[group_id];
            for (auto& it : ids)
            {
                GetCurrentATMutual()->DeleteImage(it);
                group.GetGroupImageIds().erase(it);

            }
            if (group.GetNumImages() == 0)
            {
                std::vector<group_t> group_ids;
                group_ids.push_back(group_id);
                RemovePhotoGroup(group_ids);
            }
            return true;
        }

        
        
        
        
        bool BlockObject::UnGroupImages()
        {
            return true;
        }


        void BlockObject::ReName(std::string name)
        {
            block_info_.blockString = name;
        };
        
        void BlockObject::MakeNewBlockForCommonImages( const BlockObject& refblock,BlockObject& newblock1, BlockObject& newblock2, bool byname)
        {
            BlockObject block2(refblock);
            if (supportTempLogs())
            {
                std::ostringstream oss;
                oss << "create bo:" << std::hex << std::showbase << &block2 << std::dec;
                
            }
            LoadTiepoints();
            
            
            
            
            
            
            
            
            
            
            block2.LoadTiepoints();
            
            
            
            
            
            
            
            
            
            

            if (GetCurrentATMutual() == nullptr)
            {
                return;
            }
            if (block2.GetCurrentATMutual() == nullptr)
            {
                return;
            }
            ATData at1 = *GetCurrentATMutual();
            ATData at2 = *block2.GetCurrentATMutual();


            std::string srs_src = at1.GetLocalSrs(), srs_dst = at2.GetLocalSrs();
            if (!CoordinateTransformer::IsSame(srs_src, srs_dst))
            {
                at1.TransFormImages(srs_src, srs_dst);
                at1.TransFormTiepoints(srs_src, srs_dst);


                at1.SetLocalSrs(srs_dst);
                SetBlockSRS(srs_dst);

            }
            
            std::set<image_t> common_image_ids1, common_image_ids2;
            if (byname)
            {
                at1.FindCommonImages(at2, common_image_ids1, common_image_ids2);
            }
            else
            {
                ABBox2d bb1 = at1.GetImageCenterABB();
                ABBox2d bb2 = at2.GetImageCenterABB();
                ABBox2d bb = bb1.intersection(bb2);
                for (auto& iter : at1.GetImages())
                {
                    if (iter.second.HasPosition())
                    {
                        auto& pos = iter.second.GetPosition();
                        if (bb.contains(Eigen::Vector2d(pos.x(), pos.y())))
                        {
                            common_image_ids1.insert(iter.first);
                        }
                    }
                }
                for (auto& iter : at2.GetImages())
                {
                    if (iter.second.HasPosition())
                    {
                        auto& pos = iter.second.GetPosition();
                        if (bb.contains(Eigen::Vector2d(pos.x(), pos.y())))
                        {
                            common_image_ids2.insert(iter.first);
                        }
                    }
                }
            }

            
            std::set<image_t> imageids1, imageids2;
            for (auto& iter : at1.GetImages())
            {
                imageids1.insert(iter.first);
            }
            for (auto& iter : imageids1)
            {
                if ( !common_image_ids1.count(iter))
                {
                    at1.DeleteImage(iter);
                }

            }
            for (auto& iter : at2.GetImages())
            {
                imageids2.insert(iter.first);
            }
            for (auto& iter : imageids2)
            {
                if (!common_image_ids2.count(iter))
                {
                    at2.DeleteImage(iter);
                }
            }
            newblock1 = *this, newblock2 = block2;

            std::shared_ptr<ATData> atptr1 = std::make_shared<ATData>(at1);
            std::shared_ptr<ATData> atptr2 = std::make_shared<ATData>(at2);
            newblock1.GetCurrentATMutual() = atptr1;
            newblock2.GetCurrentATMutual() = (atptr2);
        }
        
        void BlockObject::MakeNewBlockForCommonAreaImages(BlockObject& block, BlockObject& newblock)
        {

        }

        srs_s BlockObject::GetBlockSRS()
        {
            if (srs_map_.size() <= 0
                || blockSRS_id_ == kInvalidSrsId)
            {
                srs_s srs;
                return srs;
            }
            if (srs_map_.count(blockSRS_id_) == 0)
            {
                srs_s srs;
                return srs;
            }
            
            return srs_map_.at(blockSRS_id_);
        }
        void BlockObject::SetBlockSRS(std::string srsdefinition)
        {
            if (ExistSRS(srsdefinition) == kInvalidSrsId)
            {
                srs_s srs;
                srs.definition = srsdefinition;
                UpdateSRSMap(srs);
                blockSRS_id_ = ExistSRS(srsdefinition);
                if (srs.type == coord_system_type_e::LOCAL_ENU)
                {
                    srs_enu_discription_ = srs;
                }
                else
                {
                    srs_enu_discription_ = ComputeEnuSRS();
                }
            }

              
              

              
        }

        EIGEN_STL_UMAP(srsid_t, srs_s) BlockObject::GetSRSs()const
        {
            return srs_map_;
        }
        EIGEN_STL_UMAP(srsid_t, srs_s)& BlockObject::GetSRSsMutual()
        {
            return srs_map_;
        }
        void BlockObject::SetSRSs(const EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map)
        {
            srs_map_ = srs_map;
        }

        
        bool BlockObject::ParseControlPoints(const pugi::xml_node& controlpoints, EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map,
            
            EIGEN_STL_UMAP(point3D_t, ControlPoint)& cps_map, EIGEN_STL_UMAP(image_t, std::string)& image_map)
        {
            point3D_t index_point3d = 0;

            for (pugi::xml_node cp = controlpoints.first_child(); cp != NULL; cp = cp.next_sibling())
            {
                srs_s srs;
                ControlPoint controlpoint;
                if (cp.child("SRSId"))
                {
                    std::string srstxt = cp.child("SRSId").text().as_string();
                    if (!srstxt.empty())
                    {
                        auto id = std::stoull(srstxt.c_str());
                        srs.ID = id;

                        if (srs.ID == kInvalidSrsId || !srs_map.count(id))
                        {
                            return false;
                        }
                        else
                        {
                            srs = srs_map.at(id);
                        }
                    }
                }
                else
                {
                    return false;
                   
                }
                controlpoint.SetSrs(srs);
               
                Track track;
                std::string name = "";
                std::string image_fullname = "";
                std::string category = "";
                Eigen::Vector3d position(-DBL_MAX, -DBL_MAX, -DBL_MAX);
                double horizontalaccuarcy = -1;
                double verticalaccuarcy = -1;
                bool checkpoint = false;

                if (cp.child("Name"))
                {
                    name = cp.child("Name").text().as_string();
                }
                if (cp.child("Category"))
                {
                    category = cp.child("Category").text().as_string();
                    if (category == "Full")
                    {
                        controlpoint.SetType(GCP_CONTROL_HV);
                    }
                }
                if (cp.child("CheckPoint"))
                {
                    checkpoint = cp.child("CheckPoint").text().as_bool();
                    if (checkpoint)
                    {
                        controlpoint.SetType(GCP_CHECK_HV);
                    }
                }
                if (cp.child("Position").child("x") && cp.child("Position").child("y") && cp.child("Position").child("z"))
                {
                    position(0) = cp.child("Position").child("x").text().as_double();
                    position(1) = cp.child("Position").child("y").text().as_double();
                    position(2) = cp.child("Position").child("z").text().as_double();
                }
                if (cp.child("HorizontalAccuracy"))
                {
                    horizontalaccuarcy = cp.child("HorizontalAccuracy").text().as_double();
                }
                if (cp.child("VerticalAccuracy"))
                {
                    verticalaccuarcy = cp.child("VerticalAccuracy").text().as_double();
                }
                std::vector<TrackElement> vec_trackele;
               
                for (const auto& ele : cp.children("Measurement"))
                {
                    TrackElement trackelement;
                    image_t image_id = kInvalidImageId;

                    Eigen::Vector2d uv(0.0, 0.0);
                    if (ele.child("PhotoId") && ele.child("x") && ele.child("y") && ele.child("ImagePath"))
                    {
                        image_id = ele.child("PhotoId").text().as_int();
                       
                        uv(0) = ele.child("x").text().as_double();
                        uv(1) = ele.child("y").text().as_double();
                        image_fullname = ele.child("ImagePath").text().as_string();
                        std::string imgname = AI3D::CORE::File::EnsureUnifySlash(image_fullname);
                        AI3D::CORE::String::StringToLower(&imgname);
                        
                        
                        
                        
                        
                        
                        if (image_map.count(image_id))
                        {
                            std::string nametemp = image_map.at(image_id);
                            if (nametemp != imgname)
                            {
                                return false;
                            }
                        }
                        else
                        {
                            image_map[image_id] = imgname;
                        }
                              
                    }
                   
                    

                    trackelement.image_id = image_id;
                    trackelement.xy = uv;
                    trackelement.point2D_idx = index_point3d;
                    vec_trackele.push_back(trackelement);
                }
                if (!vec_trackele.empty())
                {
                    track.AddElements(vec_trackele);
                    controlpoint.GetObjectPointMutual().SetTrack(track);
                }
                controlpoint.SetId(index_point3d);
                

                controlpoint.SetGivenXYZ(position);
                controlpoint.SetName(name);
                controlpoint.SetWeight(Eigen::Vector2d(horizontalaccuarcy, verticalaccuarcy));
                cps_map.insert(std::make_pair(index_point3d, controlpoint));
                
                index_point3d++;
            }
            return true;
           
        }

        int BlockObject::LoadGCPMeasurementsXML1(const std::string& xml_file_path, EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map,
            EIGEN_STL_UMAP(point3D_t, ControlPoint)& cps_map, EIGEN_STL_UMAP(image_t, std::string)& image_map)
        {

         
            pugi::xml_document doc;
            LOGI("*********************load_file************************");
            if (doc.load_file(xml_file_path.c_str()).status != pugi::xml_parse_status::status_ok)
            {
                LOGE(String::StringPrintf("Load XML file: %s error!", xml_file_path.c_str()));
                return AI3D_FAILURE;
            }


            
            pugi::xml_node data = doc.child("SurveysData");


            
            
           
            pugi::xml_node srss = data.child("SpatialReferenceSystems");
            if (!srss.children("SRS").empty())
            {
                if (!BlockObject::ParseSRS(srs_map, srss))
                {
                    return AI3D_FAILURE;
                }
            }

            pugi::xml_node controlpoints = data.child("ControlPoints");
          
            if (!controlpoints.children("ControlPoint").empty())
            {
               
                bool result = ParseControlPoints(controlpoints, srs_map,cps_map,   image_map);
                if (!result)
                {
                    return AI3D_FAILURE;
                }
               

            }

           
            return AI3D_SUCCESS;


        }


        int BlockObject::LoadGCPMeasurementsXML(const std::string& xml_file_path, std::shared_ptr<ATData>& ATdata)
        {

            std::vector<PhotoGroup>pg;
            std::set<image_t> images_pg;
            
            pugi::xml_document doc;
            LOGI("*********************load_file************************");
            if (doc.load_file(xml_file_path.c_str()).status != pugi::xml_parse_status::status_ok)
            {
                LOGE(String::StringPrintf("Load XML file: %s error!", xml_file_path.c_str()));
                return AI3D_FAILURE;
            }


            
            pugi::xml_node data = doc.child("SurveysData");


            
            
            EIGEN_STL_UMAP(srsid_t, srs_s) srs_map;
            pugi::xml_node srss = data.child("SpatialReferenceSystems");
            if (!srss.children("SRS").empty())
            {
                if (!BlockObject::ParseSRS(srs_map, srss))
                {
                    return AI3D_FAILURE;
                }
            }

            pugi::xml_node controlpoints = data.child("ControlPoints");
            std::set<srsid_t> srs_used_id_gcp;
            if (!controlpoints.children("ControlPoint").empty())
            {
                EIGEN_STL_UMAP(point3D_t, ControlPoint)cps_map;
                bool result = ParseControlPoints(ATdata, cps_map, images_pg, controlpoints, srs_map,srs_used_id_gcp);
                if (!result)
                {
                    return false;
                }
                for (auto& gcp : cps_map)
                {
                    auto id = (ATdata->GenerateValidGCPId());
                    gcp.second.SetId(id);
                    ATdata->GetControlPointsMutual()[id] = gcp.second;
                }

            }

            
#ifdef USE_AI3D_PROJ
            
            
            
            
            
            
            
#endif
            return AI3D_SUCCESS;


        }
        
        int BlockObject::ExportGCPMeasurementsXML1(const std::string& xml_file_path,
            EIGEN_STL_UMAP(image_t, std::string)& image_map,
            EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) gcpmap)
        {
           

            EIGEN_STL_UMAP(srsid_t, srs_s) output_srs;
            for (auto& iter : gcpmap)
            {
                srs_s src_crs = iter.second.GetSrs();
                output_srs[src_crs.ID] = src_crs;
            }

            pugi::xml_document doc;
            pugi::xml_node declaration_node = doc.append_child(pugi::node_declaration);

            declaration_node.append_attribute("version") = "1.0";
            declaration_node.append_attribute("encoding") = "utf-8";

            pugi::xml_node blocksexchange = doc.append_child("SurveysData");

            if (!output_srs.empty())
            {
                pugi::xml_node SpatialReferenceSystems = blocksexchange.append_child("SpatialReferenceSystems");


                for (const auto& srs_map : output_srs)
                {
                    pugi::xml_node srs = SpatialReferenceSystems.append_child("SRS");
                    pugi::xml_node id = srs.append_child("Id");
                    id.append_child(pugi::node_pcdata).set_value(std::to_string(srs_map.second.ID).c_str());
                    pugi::xml_node name = srs.append_child("Name");
                    name.append_child(pugi::node_pcdata).set_value(srs_map.second.name.c_str());

                    pugi::xml_node definition = srs.append_child("Definition");
                    definition.append_child(pugi::node_pcdata).set_value(srs_map.second.definition.c_str());
                }

            }
            pugi::xml_node ControlPoints;
            bool any_gcp = false;
            for (auto& cp : gcpmap)
            {
                if (!cp.second.HasGivenXYZ()) {
                    continue;
                }
                if (!any_gcp) {
                    ControlPoints = blocksexchange.append_child("ControlPoints");
                    any_gcp = true;
                }
                pugi::xml_node controlpoint = ControlPoints.append_child("ControlPoint");
                SerializeControlPoint(image_map, cp.second, controlpoint);
            }

            bool saveSucceed = doc.save_file(xml_file_path.c_str());
            if (!saveSucceed)
            {
                LOG(ERROR) << "saving" + xml_file_path + " xml failed!";
                return AI3D_FAILURE;
            }

            
            return AI3D_SUCCESS;


        }

        
        int BlockObject::ExportGCPMeasurementsXML(const std::string& xml_file_path)
        {
            if (GetCurrentAT() == nullptr)
            {
                return AI3D_FAILURE;
            }
            ControlPoints gcps;
            auto gcpmap = GetCurrentAT().get()->GetControlPoints();
            
            EIGEN_STL_UMAP(srsid_t, srs_s) output_srs;
            for (auto& iter : gcpmap)
            {
                srs_s src_crs = iter.second.GetSrs();
                output_srs[src_crs.ID] = src_crs;
            }

            pugi::xml_document doc;
            pugi::xml_node declaration_node = doc.append_child(pugi::node_declaration);

            declaration_node.append_attribute("version") = "1.0";
            declaration_node.append_attribute("encoding") = "utf-8";

            pugi::xml_node blocksexchange = doc.append_child("SurveysData");

            if (!output_srs.empty())
            {
                pugi::xml_node SpatialReferenceSystems = blocksexchange.append_child("SpatialReferenceSystems");


                for (const auto& srs_map : output_srs)
                {
                    pugi::xml_node srs = SpatialReferenceSystems.append_child("SRS");
                    pugi::xml_node id = srs.append_child("Id");
                    id.append_child(pugi::node_pcdata).set_value(std::to_string(srs_map.second.ID).c_str());
                    pugi::xml_node name = srs.append_child("Name");
                    name.append_child(pugi::node_pcdata).set_value(srs_map.second.name.c_str());

                    pugi::xml_node definition = srs.append_child("Definition");
                    definition.append_child(pugi::node_pcdata).set_value(srs_map.second.definition.c_str());
                }

            }
            pugi::xml_node ControlPoints;
            bool any_gcp = false;
            for (auto& cp : gcpmap)
            {
                if (!cp.second.HasGivenXYZ()) {
                    continue;
                }
                if (!any_gcp) {
                    ControlPoints = blocksexchange.append_child("ControlPoints");
                    any_gcp = true;
                }
                pugi::xml_node controlpoint = ControlPoints.append_child("ControlPoint");
                SerializeControlPoint(*GetCurrentATMutual().get(), cp.second, controlpoint,true);
            }

        
            return AI3D_SUCCESS;

            
        }


        bool BlockObject::ExportATReport(const ATReport& at_report, const std::string& ATReportFilename)
        {
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }

            std::string scene_coverage = path_ + PATH_SEPARATOR_STR + SCIMAG;
            File::CreateDirIfNotExists(scene_coverage);
            if (GenerateATReportPicture(at_report, File::EnsureUnifySlash(scene_coverage + PATH_SEPARATOR_STR + SCCOVERIMAG)))
            {
                return true;
            }
            else {
                return false;
            }

        }

        bool BlockObject::ExistSRSId(const srsid_t& id)
        {
            return srs_map_.find(id) != srs_map_.end();
        }

        namespace {

        bool ImwriteUtf8(const std::string& utf8Path, const cv::Mat& image)
        {
            std::string ext = File::GetFileExtension(utf8Path);
            if (ext.empty()) {
                ext = ".jpg";
            }
            std::vector<uchar> buf;
            if (!cv::imencode(ext, image, buf) || buf.empty()) {
                return false;
            }
            return File::WriteBinaryUtf8(
                utf8Path,
                reinterpret_cast<const unsigned char*>(buf.data()),
                buf.size());
        }

        } // namespace

        bool BlockObject::GenerateATReportPicture(const ATReport& at_report, const std::string& picPath)
        {
            
            
            

            
            
            

            

            
            if (GetCurrentATMutual() == nullptr)
            {
                return false; 
            }
            
            ATData atdata = *GetCurrentAT();
            if (!srs_map_.empty())
            {
                if (srs_map_.count(blockSRS_id_)&&srs_map_.at(blockSRS_id_).type != coord_system_type_e::PROJECTION && srs_map_.at(blockSRS_id_).type != coord_system_type_e::LOCAL_ENU && !atdata.GetLocalSrs().empty())
                {
                    
                    auto srs = atdata.GetDefaultEnuSRS();
                    
                    atdata.TransFormTiepoints(srs_map_.at(blockSRS_id_).definition, srs.definition);
                }
            }
            
            
            std::vector<Eigen::Vector3d>points;
            for (auto& it : atdata.GetPoints3D())
            {
                points.push_back(it.second.GetXYZ());
            }
            if (points.empty())
            {
                return false;
            }

            bbox_s box;

            for (auto& it : points)
            {

                Eigen::Vector3d point = it;
                if (point(0) > box.xmax_)
                {
                    box.xmax_ = point(0);
                }
                if (point(0) < box.xmin_)
                {
                    box.xmin_ = point(0);
                }
                if (point(1) > box.ymax_)
                {
                    box.ymax_ = point(1);
                }
                if (point(1) < box.ymin_)
                {
                    box.ymin_ = point(1);
                }
                if (point(2) > box.zmax_)
                {
                    box.zmax_ = point(2);
                }
                if (point(2) < box.zmin_)
                {
                    box.zmin_ = point(2);
                }
            }

            double xmin = box.xmin_;
            double xmax = box.xmax_;

            double ymin = box.ymin_;
            double ymax = box.ymax_;
            
            double x_len = std::abs(xmax - xmin);
            double y_len = std::abs(ymax - ymin);
            
            double dx = x_len / 600;
            
            
            double dy = y_len / int(600 * y_len / x_len);


            bool isXPan = false;
            bool isYPan = false;
            for (auto& tiepoint : atdata.GetPoints3DMutual())
            {
                if (xmin < 0)
                {
                    tiepoint.second.GetXYZMutual().x() += std::abs(xmin);
                    isXPan = true;
                }
                if (ymin < 0)
                {
                    tiepoint.second.GetXYZMutual().y() += std::abs(ymin);
                    isYPan = true;
                }
            }

            
            if (isYPan)
            {
                ymin = 0;
            }
            if (isXPan)
            {
                xmin = 0;
            }
            
            Mat<float> mat(624, int(600 * y_len / x_len)+24, 1);
            std::vector<int>PropertyValue;
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            

            std::map<std::string, int> mat_map;
            
            
            
            
            
            
            
            

            
            
            
            

            
            
            
            

            
            
            
            

            
            
            
            
            
            
            
            
            

            for (const auto& img : atdata.GetImages())
            {
                auto numtps = img.second.GetNumPoints2D();
                
                double tpsXmin = DBL_MAX;
                double tpsXmax = -DBL_MAX;
                double tpsYmin = DBL_MAX;
                double tpsYmax = -DBL_MAX;
                for (int i_points2d = 0; i_points2d < numtps; i_points2d++)
                {
                
                    
                    if (i_points2d < img.second.GetPoints2D().size())
                    {
                        auto ptid = img.second.GetPoint2D(i_points2d).GetPoint3DId();
                        if (atdata.GetPoints3DMutual().count(ptid))
                        {
                            Eigen::Vector3d xyz = atdata.GetPoint3D(img.second.GetPoint2D(i_points2d).GetPoint3DId()).GetXYZ();

                            if (xyz.x() > tpsXmax)
                            {
                                tpsXmax = xyz.x();
                            }
                            if (xyz.x() < tpsXmin)
                            {
                                tpsXmin = xyz.x();
                            }
                            if (xyz.y() > tpsYmax)
                            {
                                tpsYmax = xyz.y();
                            }
                            if (xyz.y() < tpsYmin)
                            {
                                tpsYmin = xyz.y();
                            }
                        }
                    }               
                }
                if (tpsXmin == DBL_MAX || tpsXmax == -DBL_MAX || tpsYmin == DBL_MAX || tpsYmax == -DBL_MAX)
                {
                    continue;
                }

                int row_min = int((tpsYmin - ymin) / dy);
                int row_max = int((tpsYmax - ymin) / dy);
                row_min += 12;
                row_max += 12;

                int col_min = int((tpsXmin - xmin) / dx);
                int col_max = int((tpsXmax - xmin) / dx);
                col_min += 12;
                col_max += 12;

                row_min = row_min < 12 ? 12 : row_min;
                row_max = row_max > (int(600 * y_len / x_len) + 24 - 13) ? (int(600 * y_len / x_len) + 24 - 13) : row_max;
                col_min = col_min < 12 ? 12 : col_min;
                col_max = col_max > 611 ? 611 : col_max;

                for (int i_row = row_min; i_row < row_max; i_row++)
                {
                    for (int i_col = col_min; i_col < col_max; i_col++)
                    {
                        std::string index_xy = std::to_string(int(600 * y_len / x_len) + 24 - i_row) + "," + std::to_string(i_col);
                        mat_map[index_xy]++;
                    }
                }
            }

            for (const auto& itr_mat_map : mat_map)
            {
                auto rowandcol = String::StringSplit(itr_mat_map.first, ",");
                mat.Set(std::atoi(rowandcol[0].c_str()), std::atoi(rowandcol[1].c_str()), itr_mat_map.second);
                PropertyValue.emplace_back(itr_mat_map.second);
            }

            

            
            
            
            
            
            
            
            
            
            
            
            
            

            
            
            
            
            
            
            
            
            
            
            if (PropertyValue.empty())
            {
                return false;
            }
            double mindepth = *std::min_element(PropertyValue.begin(), PropertyValue.end());
            double maxdepth = *std::max_element(PropertyValue.begin(), PropertyValue.end());
            DepthMap depthMap(mat, mindepth, maxdepth);
            Bitmap bitmap = depthMap.ToBitmap(0, 100);

            
            

            
            Mat<float> matCB(624, 72, 1);
            int numPerPixel = int((maxdepth - mindepth) / 500);
            int numPerSegment = int((maxdepth - mindepth) / 4);
            double valuePerPixel = (maxdepth - mindepth) / 500;
            for (int col = 62; col < 562; col++)
            {
                double valuePerCol = mindepth + (col - 62) * valuePerPixel;
                for (int row = 6; row < 42; row++)
                {
                    matCB.Set(row, col, valuePerCol);
                }
            }
            DepthMap depthMapColorBar(matCB, mindepth, maxdepth);
            Bitmap bitmapColorBar = depthMapColorBar.ToBitmap(0, 100);
            

            
            
            cv::Mat matOverlap(bitmap.GetHeight(), bitmap.GetWidth(), CV_8UC3, cv::Scalar(255, 255, 255));
            for (int row = 0; row < bitmap.GetHeight(); row++)
            {
                for (int col = 0; col < bitmap.GetWidth(); col++)
                {
                    BitmapColor<uint8_t> color;
                    auto pixel = bitmap.GetPixel(col, row, &color);
                    if (pixel && color != BitmapColor<uint8_t>(255, 255, 255))
                    {
                        matOverlap.at<cv::Vec3b>(row, col) =cv::Vec3b(color.b, color.g, color.r);
                    }
                }
            }
            
            cv::Point pointx0(6, int(600 * y_len / x_len) + 18);
            cv::Point pointx1(106, int(600 * y_len / x_len) + 18);
            cv::Point pointy1(6, int(600 * y_len / x_len) - 82);
            cv::arrowedLine(matOverlap, pointx0, pointx1, cv::Scalar(255, 255, 255), 2, 8);
            cv::arrowedLine(matOverlap, pointx0, pointy1, cv::Scalar(255, 255, 255), 2, 8);
            cv::putText(matOverlap, "X", cv::Point(108, int(600 * y_len / x_len) + 22), cv::FONT_HERSHEY_COMPLEX, 0.45, cv::Scalar(255,255,255), 1.8);
            cv::putText(matOverlap, "Y", cv::Point(2, int(600 * y_len / x_len) - 84), cv::FONT_HERSHEY_COMPLEX, 0.45, cv::Scalar(255, 255, 255), 1.8);

            cv::Mat matColorBar(bitmapColorBar.GetHeight(), bitmapColorBar.GetWidth(), CV_8UC3, cv::Scalar(255, 255, 255));
            for (int row = 0; row < bitmapColorBar.GetHeight(); row++)
            {
                for (int col = 0; col < bitmapColorBar.GetWidth(); col++)
                {
                    BitmapColor<uint8_t> color;
                    auto pixel = bitmapColorBar.GetPixel(col, row, &color);
                    if (pixel && color != BitmapColor<uint8_t>(255, 255, 255))
                    {
                        matColorBar.at<cv::Vec3b>(row, col) = cv::Vec3b(color.b, color.g, color.r);
                    }
                }
            }

            double range = (maxdepth - mindepth) / 4;
            for (int i = 0; i < 5; i++)
            {
                cv::putText(matColorBar, File::ToStringWithHighPrecision(mindepth + i * range, 2), cv::Point(54 + i * 125, 55), cv::FONT_HERSHEY_COMPLEX, 0.45, cv::Scalar(255,255,255), 1.8);
            }
            const std::string colorBarPath = File::EnsureUnifySlash(
                File::GetParentDir(picPath) + "/" + SCSCENEIMAG);
            if (!ImwriteUtf8(colorBarPath, matColorBar) || !ImwriteUtf8(picPath, matOverlap))
            {
                return false;
            }

            return true;
        }

        srsid_t BlockObject::ExistSRS(const std::string& definition)
        {
            
            for (auto& it : srs_map_)
            {
                if (CoordinateTransformer::IsSame(definition,it.second.definition))
                {
                    return it.first;
                }
                else if (definition.find("ENU") != std::string::npos)
                {
                    
                    Eigen::Vector2d LatLon_Srs_map = CoordinateDescriptor::GetLatLonFromENUDefinition(it.second.definition);
                    Eigen::Vector2d LatLon_srs_new = CoordinateDescriptor::GetLatLonFromENUDefinition(definition);
                    if (LatLon_Srs_map == LatLon_srs_new)
                    {
                        return it.first;
                    }
                }
            }

            return kInvalidSrsId;
        }
    
        void BlockObject::SetDescription(std::string des)
        {
            description_ = des;
        }
        const std::string BlockObject::GetDescription() const
        {
            return description_;

        }
        std::string BlockObject::GetDescriptionMutual()
        {
            return description_;
        }

        void BlockObject::SetStatus(jobsta_e status)
        {
            status_ = status;
            
        }
        const jobsta_e BlockObject::GetStatus() const
        {
            return status_;
        }
        jobsta_e BlockObject::GetStatusMutual()
        {
            return status_;
        }

        void BlockObject::SetType(bt_e type)
        {
            type_ = type;
        }
        const BlockObject::bt_e BlockObject::GetType() const
        {
            return type_;
        }
        BlockObject::bt_e BlockObject::GetTypeMutual()
        {
            return type_;
        }

        void BlockObject::SetATData(const std::shared_ptr<ATData>& at_data)
        {
            ATData_ = at_data;
            if (status_ == STATUS_COMPLETE && ATGroups_.count(0))
            {
                ATGroups_.at(0).SetATData(at_data);
            }
        }
        const std::shared_ptr<ATData> BlockObject::GetATData() const
        {
            if (status_ == STATUS_COMPLETE && ATGroups_.count(0))
            {
                return ATGroups_.at(0).GetATData();
            }
            return ATData_;
        }
        std::shared_ptr<ATData> BlockObject::GetATDataMutual()
        {
            if (status_ == STATUS_COMPLETE && ATGroups_.count(0))
            {
                return ATGroups_.at(0).GetATDataMutual();
            }
            return ATData_;
        }

        bool BlockObject::UpdateATGroup(std::shared_ptr<ATData>& data_new, bool bgcpat)
        {
            auto Atdata = ATData_;
            ATData data_old = *Atdata;


            
            
                
                
                
                
                
                
                
            
                if (data_new->GetLocalSrs() != data_old.GetLocalSrs())
                {
                    std::string msg = "Update AT result " + data_new->GetLocalSrs() + " file " + data_old.GetLocalSrs() + __FILE__ + " " + __FUNCTION__;
                    msg += __LINE__;
                    LOGI(msg);
                    srs_s newsrs = CoordinateDescriptor::GetSRSFromDefinition(data_new->GetLocalSrs());
                    srs_s oldsrs = CoordinateDescriptor::GetSRSFromDefinition(data_old.GetLocalSrs());

                    if (newsrs.type == coord_system_type_e::Unsupported || oldsrs.type == coord_system_type_e::Unsupported)
                    {
                        std::string msg = "coordinate is unsupported ";
                        msg += __LINE__;
                        LOGI(msg);
                        return false;
                    }
                    if (newsrs.type == coord_system_type_e::LOCAL && oldsrs.type != coord_system_type_e::LOCAL)
                    {
                        
                        for (auto& iter : data_old.GetImagesMutual())
                        {
                            iter.second.GetPositionMutual().setConstant(-DBL_MAX);
                            iter.second.GetRotationMatrixMutual().setConstant(0);
                            data_old.SetLocalSrs(srs_s().definition);
                            data_old.GetPoints3DMutual().clear();
                        }
                    }
                    else
                    {
                        data_old.TransFormImages(data_old.GetLocalSrs(), data_new->GetLocalSrs());
                        data_old.TransFormTiepoints(data_old.GetLocalSrs(), data_new->GetLocalSrs());
                        data_old.TransFormGCPs(data_old.GetLocalSrs(), data_new->GetLocalSrs());
                        data_old.SetLocalSrs(data_new->GetLocalSrs());
                        
                    }
                    UpdateSRSMap(CoordinateDescriptor::GetSRSFromDefinition(data_new->GetLocalSrs()));
                    blockSRS_id_ = ExistSRS(data_new->GetLocalSrs());
                }

            for (const auto& image : data_new->GetImages())
            {
                if (image.second.IsRegistered())
                {
                    if (!data_old.GetImages().count(image.first))
                    {

                        continue;
                    }
                    Image& img = data_old.GetImageMutual(image.first);

                    img.GetPositionMutual() = image.second.GetPosition();
                    img.GetRotationMatrixMutual() = image.second.GetRotationMatrix();
                    img.GetColorParamMutual() = image.second.GetColorParam();
                    img.GetPoints2DMutual().clear();
                    img.SetPoints2D(image.second.GetPoints2D());


                    
                    
                    img.SetNumPoints3D(image.second.GetNumPoints3D());
                    img.SetRegistered(true);
                }
            }



            

            data_old.GetPoints3DMutual() = data_new->GetPoints3D();
            
            for (const auto& camera : data_new->GetCameras())
            {
                
                if (photogroups_.count(camera.first))
                {
                    int imageid = *photogroups_[camera.first].GetGroupImageIds().begin();
                    if (data_old.ExistsImage(imageid))
                    {
                        Camera& cam = data_old.GetCameraMutual(data_old.GetImage(imageid).GetCameraId());
                        cam.GetParamsMutual() = camera.second.GetParams();
                        if (cam.GetSensorSize() != kInvalideNum)
                        {
                            double f_mm = camera.second.GetFocalLengthX() * cam.GetSensorSize() / std::max(cam.GetWidth(), cam.GetHeight());
                            cam.SetFocalLengthMM(f_mm);
                        }
                        
                        double f_35eq = camera.second.GetFocalLengthX() * 36 / std::max(cam.GetWidth(), cam.GetHeight());
                        cam.SetFocalLengthIn35mm(f_35eq);

                        photogroups_[camera.first].GetCameraMutual() = cam;
                    }
                }
                
                
            }

            
            std::set<image_t> reg_image_ids;
            for (const auto& image : data_old.GetImages())
            {
                if (!image.second.IsRegistered())
                {
                    auto groupimages = photogroups_[image.second.GetPhotoGroupID()].GetGroupImageIds();
                    groupimages.insert(image.first);
                    photogroups_[image.second.GetPhotoGroupID()].SetGroupImage(groupimages);
                }
                else if (data_new->GetImages().count(image.second.GetImageId())
                    && data_new->GetImages().at(image.second.GetImageId()).IsRegistered())
                {
                    reg_image_ids.insert(image.second.GetImageId());
                }
            }
            
            
            std::vector<image_t> reg_image_ids_vec;
            reg_image_ids_vec.assign(reg_image_ids.begin(), reg_image_ids.end());
            data_old.SetRegImageIds(reg_image_ids_vec);

            data_new = std::make_shared<ATData>(data_old);
            if (reg_image_ids_vec.empty())
            {
                return false;
            }
            return true;
        }

        std::shared_ptr<ATData>& BlockObject::GetOriginAT()
        {
            return ATData_;
        };
        const ATGroup& BlockObject::GetAT(group_t atid) const
        {
            return ATGroups_.at(atid);
        };


        const ATGroup& BlockObject::GetAT0() const
        {
            return ATGroups_.at(0);
        }
        ATGroup& BlockObject::GetAT0()
        {
            if (ATGroups_.size() <= 0)
            {
                ATGroup* atgroup = new ATGroup;
                std::string str = "undefine";
                atgroup->SetName(str);
                return *atgroup;
            }
            return ATGroups_.at(0);
        }
        void BlockObject::SetAT0(std::shared_ptr<ATData>ATdata)
        {
            
            
            
            ATData_ = ATdata;
            ATGroups_[0].SetATData(ATdata);
        }


        bool BlockObject::Check()
        {
            return true;
        }

        

        

        
        

        bool BlockObject::ParseSRS(EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map, const pugi::xml_node& srss)
        {
            srs_s srs;
            for (const auto& node : srss.children("SRS"))
            {
                srsid_t id = kInvalidSrsId;
                std::string name = "";
                std::string definition = "";
                coord_system_type_e type;

                if (node.child("Id"))
                {
                    id = node.child("Id").text().as_int();
                    srs.ID = id;
                }

                if (node.child("Name"))
                {
                    name = node.child("Name").text().as_string();
                    srs.name = name;
                }

                if (node.child("Definition"))
                {
                    definition = node.child("Definition").text().as_string();
                    srs.definition = definition;
                }

                if (node.child("type"))
                {
                    type = coord_system_type_e(node.child("type").text().as_int());
                    srs.type = type;
                }
                else
                {
#ifdef USE_AI3D_PROJ
                    AI3D::PROJ::CoordinateReferenceSystem crs(definition);
                    if (!crs.isValid())
                    {
                        String::StringToLower(&definition);
                        if (String::StringContains(definition, "wgs84"))
                        {
                            definition = GEO84SRS;
                            crs = AI3D::PROJ::CoordinateReferenceSystem(definition);
                            if (!crs.isValid())
                            {
                                if (isChineseVersion())
                                {
                                    LOGI("无效的坐标系统.");
                                }
                                else
                                {
                                    LOGI("invalid srs.");
                                }
                                continue;
                            }
                        }


                    }
                    srs.definition = crs.GetAuthID();
                    srs.name = crs.GetDescription();
                    srs.type = crs.GetType();

#else


                    size_t index = definition.find_first_of(":");
                    
                    if (index != std::string::npos)
                    {
                        std::string coord_name = definition.substr(0, index);
                        if (coord_name == std::string("ENU"))
                        {
                            srs.type = coord_system_type_e::LOCAL_ENU;
                        }
                        else
                        {
                            size_t index_plus = definition.find("+");
                            if (index_plus != std::string::npos)
                            {
                                definition = definition.substr(0, index_plus);
                            }
                            srs.type = CoordinateDescriptor::GetSRSFromDefinition(definition).type;

                        }
                    }
                    else
                    {
                        
                        
                        OGRSpatialReference sr;
                        if (OGRERR_NONE == sr.importFromWkt(definition.c_str()))
                        {

                            std::string codestr(sr.GetAuthorityCode(NULL));

                            definition = "epsg:" + codestr;

                            auto srs_tmp = CoordinateDescriptor::GetSRSFromDefinition(definition);


                            
                            srs.definition = srs_tmp.definition;
                            srs.name = srs_tmp.name;
                            srs.type = srs_tmp.type;
                        }
                    }
                
#endif 

                }

                if (id == kInvalidSrsId || definition.empty())
                {
                    LOGE(String::StringPrintf("Invalid SRS  id:%d name:%s definition:%s ", id, name.c_str(), definition.c_str()));
                    return false;
                }
                srs_map.insert(std::make_pair(id, srs));
            }
            return true;
        }

        bool BlockObject::Clear()
        {

            ATGroups_.clear();
            photogroups_.clear();


            id_ = kInvalidBlockId;
            name_ = "";
            path_ = "";

            position_offset_ = { -DBL_MAX,-DBL_MAX, -DBL_MAX };

            srs_enu_discription_ = srs_s();

            srs_map_.clear();

            blockSRS_id_ = kInvalidSrsId;


            image_ids_.clear();

            std::string description_ = "";
            return true;
        }

        bool BlockObject::ParsePhotoGroups(std::shared_ptr<ATData> Atdata_, std::vector<PhotoGroup>& pg_, 
            std::set<image_t>& images_pg, const pugi::xml_node& groups,
            EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map,std::set<srsid_t>& srs_used_ids)
        {
            group_t temp_id = 0;
            std::vector<image_t> reg_image_ids;
            for (pugi::xml_node group = groups.first_child(); group != NULL; group = group.next_sibling())
            {
                PhotoGroup pg;
                
                std::set<image_t> images_pg_temp;
                std::string name;
                if (group.child("Name"))
                {
                    name = group.child("Name").text().as_string();
                }
                else
                {
                    name = GROUPBASENAME + std::to_string(temp_id);
                }
                pg.SetName(name);

                
                Camera camera;
                bool result = ParseCamera(camera, group, temp_id);
                if (!result)
                {
                    return false;
                }


                
                if (group.children("Photo").empty())
                {
                    continue;
                }
                
                EIGEN_STL_UMAP(image_t, Image) image_map;
                result = ParsePhotos(image_map, group, temp_id, srs_map, srs_used_ids);;
                if (!result)
                {
                    return false;
                }

                
                std::string make;
                make = (*image_map.begin()).second.GetExifinfo().make;
                std::string make_model;
                make_model = (*image_map.begin()).second.GetExifinfo().model;

                if (!make.empty() && !make_model.empty())
                {
                    camera.SetMake(make);
                    camera.SetMakeModel(make_model);
                    double f_35eq = camera.GetFocalLengthIn35mm() != kInvalideNum ? camera.GetFocalLengthIn35mm() : \
                        (*image_map.begin()).second.GetExifinfo().focalLengthIn35mm;
                    camera.SetFocalLengthIn35mm(f_35eq);
                }
                camera.SetCameraName(name);
                pg.SetCamera(camera);

                if (Atdata_->GetCameras().find(camera.GetCameraId()) == Atdata_->GetCameras().end())
                {
                    Atdata_->AddCamera(camera);
                }


                for (auto& image : image_map)
                {
                    
                    if(image.second.IsRegistered())
      
                                       
                        reg_image_ids.push_back(image.first);
                    images_pg_temp.insert(image.first);
                    images_pg.insert(image.first);
                    if (Atdata_->GetImages().find(image.first) == Atdata_->GetImages().end())
                    {
                        image.second.GetExifinfoMutual().width = camera.GetWidth();
                        image.second.GetExifinfoMutual().height = camera.GetHeight();
                        image.second.SetUp(camera);
                        Atdata_->AddImage(image.second);
                    }
                }
                pg.SetGroupImage(images_pg_temp);
                pg.SetId(temp_id);

                pg_.push_back(pg);
                temp_id++;
            }
            Atdata_->SetRegImageIds(reg_image_ids);
            return true;
        }

        bool BlockObject::ParseCamera(Camera& camera, const pugi::xml_node& group, const int& temp_id) const
        {
            
            std::string camera_name = "";
            std::string CameraOrientation = "XRightYDown";
            int  cameramodeltype = 0;
            int camera_id = temp_id;
            int image_width = -1;
            int image_height = -1;
            
            
            double focal_length = NAN;
            
            double focal_length_pixel_x = NAN;
            
            double focal_length_pixel_y = NAN;
            double sensor_size = NAN;
            double focal_lengthin35mm = NAN;
            double cx = NAN;
            double cy= NAN;
            double ratio = 1.;
            double k1, k2, k3, k4, k5, k6, p1, p2, p3;
            cx = cy = k1 = k2 = k3 = k4 = k5 = k6 = p1 = p2 = p3 = 0.0;
            std::vector<int> fixedindexes;

            
            pugi::xml_node image_dimension = group.child("ImageDimensions");
            if (image_dimension.child("Width"))
            {
                image_width = image_dimension.child("Width").text().as_int();
            }
            else
            {
                LOG(ERROR) << "camera has no width!";
                return false;
            }
            if (image_dimension.child("Height"))
            {
                image_height = image_dimension.child("Height").text().as_int();
            }
            else
            {
                LOG(ERROR) << "camera has no height!";
                return false;
            }
            
            if (group.child("CameraModelType"))
            {
                std::string temp_string;
                temp_string = group.child("CameraModelType").text().as_string();
                cameramodeltype = (temp_string == "Perspective" ? 0 : 1);
            }

            
            if (group.child("Fixed"))
            {
                std::string temp_string;
                temp_string = group.child("Fixed").text().as_string();
                const auto& fixedindexstrs = String::StringSplit(temp_string,",");
                
                for (int i = 0; i < fixedindexstrs.size(); i++)
                {
                    fixedindexes.push_back(std::atoi(fixedindexstrs[i].c_str()));
                }
                
            }
            
            if (group.child("CameraOrientation"))
            {
                CameraOrientation = group.child("CameraOrientation").text().as_string();
            }
            
            
            
            
            
            
            if (group.child("FocalLength"))
            {
                focal_length = group.child("FocalLength").text().as_double();
            }

            
            if (group.child("FocalLengthPixels"))
            {
                focal_length_pixel_x = group.child("FocalLengthPixels").text().as_double();
                focal_length_pixel_y = focal_length_pixel_x * 1.;
                
            }

            if (group.child("AspectRatio"))
            {
                ratio = group.child("AspectRatio").text().as_double();
                focal_length_pixel_y = focal_length_pixel_x * ratio;
            }
            

            
            if (group.child("SensorSize"))
            {
                sensor_size = group.child("SensorSize").text().as_double();
            }
            
            if (group.child("PrincipalPoint"))
            {
                if (group.child("PrincipalPoint").child("x"))
                {
                    cx = group.child("PrincipalPoint").child("x").text().as_double();
                }
                if (group.child("PrincipalPoint").child("y"))
                {
                    cy = group.child("PrincipalPoint").child("y").text().as_double();
                }
            }
            else
            {
                
                cx = image_width / 2;
                cy = image_height / 2;
            }

            
            if (group.child("Distortion").child("K1"))
            {
                k1 = group.child("Distortion").child("K1").text().as_double();
            }
            if (group.child("Distortion").child("K2"))
            {
                k2 = group.child("Distortion").child("K2").text().as_double();
            }
            if (group.child("Distortion").child("K3"))
            {
                k3 = group.child("Distortion").child("K3").text().as_double();
            }
            if (group.child("Distortion").child("P1"))
            {
                p1 = group.child("Distortion").child("P1").text().as_double();
            }
            if (group.child("Distortion").child("P2"))
            {
                p2 = group.child("Distortion").child("P2").text().as_double();
            }
            if (group.child("Distortion").child("P3"))
            {
                p3 = group.child("Distortion").child("P3").text().as_double();
            }
            
            
            
            
            
            
            
            
            
            

            
            
            
            
            
            
            
            
            
            
            
            

            
            if (focal_length >0 && sensor_size >0 
                && std::isnan(focal_length_pixel_x )
                && image_width >0 && image_height >0)
            {
                focal_length_pixel_x = focal_length * std::max(image_width, image_height) / sensor_size;

                focal_length_pixel_y = focal_length_pixel_x;
            }

            
            if (focal_length_pixel_x >0 && image_width >0 && image_height >0)
            {
                focal_lengthin35mm = focal_length_pixel_x * 36 / std::max(image_width, image_height);
            }
            
            if (std::isnan(focal_length) && sensor_size >0 && image_width >0 && image_height >0)
            {
                focal_length = focal_length_pixel_x * sensor_size / std::max(image_width, image_height);
            }

            if (!std::isnan(focal_length_pixel_x) && !std::isnan(cx) && !std::isnan(cy))
            {
                camera.InitializeWithName("FULL_OPENCV", focal_length_pixel_x, image_width, image_height);
                camera.SetParams({ focal_length_pixel_x,focal_length_pixel_y,cx,cy,k1,k2,p1,p2,k3, k4, k5, k6 });
            }

            camera.SetModelIdFromName("FULL_OPENCV");
            camera.SetWidth(image_width);
            camera.SetHeight(image_height);
            camera.SetCameraId(camera_id);
            camera.SetCameraModelType(CameraModelType_e(cameramodeltype));
            camera.SetFocalLengthMM(focal_length);
            camera.SetSensorSize(sensor_size);
            camera.SetFocalLengthIn35mm(focal_lengthin35mm);
            camera.SetCameraOrientation(CameraOrientation);
            camera.SetFixed(fixedindexes);
            return true;
        }

        bool BlockObject::ParsePhotos(EIGEN_STL_UMAP(image_t, Image)& group_image, 
            const pugi::xml_node& group, const int& camera_id,
            EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map,std::set<srsid_t>& srs_used_ids)
        {
            int invalidcount = 0;

            for (auto& photo : group.children("Photo"))
            {
                Image image;
                
                int photo_id = -1;
                int component = -1;
                fix_e fixedstatus = fix_e::EOE_FREE;
                std::string image_path = "";
                std::string image_name = "";
                double near_depth = -DBL_MAX;
                double median_depth = -DBL_MAX;
                double far_depth = -DBL_MAX;


                if (photo.child("Id"))
                {
                    std::string idtxt = photo.child("Id").text().as_string();
                                           
                    
                    if (!idtxt.empty())
                    {
                        auto id = std::stoull(idtxt.c_str());

                        photo_id = id;
                    }
                }
                if (photo_id == -1)
                {
                    LOGI("NO photo id.");
                    return false;
                }
                

                
                
                
                
                
                if (photo.child("Component"))
                    component = photo.child("Component").text().as_int();
                if (photo.child("Fixed"))
                {
                    fixedstatus = (fix_e)photo.child("Fixed").text().as_int();
                    image.SetFixStatus(fixedstatus);
                }
                if (photo.child("ImagePath"))
                    image_path = photo.child("ImagePath").text().as_string();
                if (photo.child("NearDepth"))
                    near_depth = photo.child("NearDepth").text().as_double();
                if (photo.child("MedianDepth"))
                    median_depth = photo.child("MedianDepth").text().as_double();
                if (photo.child("FarDepth"))
                    far_depth = photo.child("FarDepth").text().as_double();

                try
                {
                    image_name = File::BoostPathToUtf8String(File::BoostPathFromUtf8(image_path).filename());
                    image_path = File::BoostPathToUtf8String(File::BoostPathFromUtf8(image_path).parent_path());
                }
                catch (std::filesystem::filesystem_error& fse)
                {
                    std::ostringstream oss;
                    oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                    LOGI(oss.str());
                }
                catch (std::exception& ex)
                {
                    std::ostringstream oss;
                    oss << "exception:" << ex.what();
                    LOGI(oss.str());
                }

                image_path = File::EnsureUnifySlash(image_path);
#ifdef _MSC_VER
                image_path = String::StringReplace(image_path, PATH_SEPARATOR_STR, REVERSE_PATH_SEPARATOR_STR);
#endif 


                
                
                Eigen::Vector3d center(-DBL_MAX, -DBL_MAX, DBL_MAX);
                Eigen::Vector3d center_prior(-DBL_MAX, -DBL_MAX, DBL_MAX);
                Eigen::Matrix3d rotation = Eigen::Matrix3d::Zero();
                Eigen::Matrix3d rotation_prior = Eigen::Matrix3d::Zero();

                srs_s prior_srs;
                
                if (photo.child("Pose"))
                {
                    pugi::xml_node node_pose = photo.child("Pose");
                    
                    pugi::xml_node node = node_pose.child("Rotation");
                    if (node)
                    {
                        
                        if (node.child("Omega")|| node.child("Phi") || node.child("Kappa"))
                        {

                            double omga = node.child("Omega").text().as_double();
                            double phi = node.child("Phi").text().as_double();
                            double kappa = node.child("Kappa").text().as_double();
                            AlgorithmBase::ConvertOPK2Rotmat(omga* RAD_PER_DEG,phi * RAD_PER_DEG,kappa * RAD_PER_DEG, rotation);
                        }
                        else if (node.child("Yaw") || node.child("Pitch") || node.child("Roll"))
                        {

                            double yaw = node.child("Yaw").text().as_double();
                            double pitch = node.child("Pitch").text().as_double();
                            double roll = node.child("Roll").text().as_double();
                            Eigen::Vector3d ypr = Eigen::Vector3d(FD2R(yaw), FD2R(pitch), FD2R(roll));
                        
                            rotation= AI3D::CORE::AlgorithmBase::YPRToRotationInner(ypr);
                            
                        }
                        else
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                for (int j = 0; j < 3; ++j)
                                {
                                    std::string flag = std::string("M_") + std::to_string(i) + std::to_string(j);
                                    if (node.child(flag.c_str()))
                                    {
                                        rotation(i, j) = node.child(flag.c_str()).text().as_double();
                                    }
                                }
                            }
                        }
                    }
                    


                    
                    pugi::xml_node node_center = node_pose.child("Center");
                    if (node_center)
                    {

                        if (node_center.child("x") && node_center.child("y") && node_center.child("z")) {
                            center(0) = node_center.child("x").text().as_double();
                            center(1) = node_center.child("y").text().as_double();
                            center(2) = node_center.child("z").text().as_double();

                        }
                    }

                    
                    if (node_pose.child("Metadata"))
                    {
                        pugi::xml_node node_meta = node_pose.child("Metadata");
                        if (node_meta.child("SRSId"))
                        {
                            
                            std::string srstxt = node_meta.child("SRSId").text().as_string();
                            if (!srstxt.empty())
                            

                                            
                            {
                                auto id = std::stoull(srstxt.c_str());
                                prior_srs.ID = id;

                                if (prior_srs.ID != kInvalidSrsId)
                                {

                                    
                                    if (srs_map.count(prior_srs.ID))
                                    {
                                        prior_srs.ID = ExistSRS(srs_map.at(prior_srs.ID).definition);
                                    }
                                    else {
                                        prior_srs.ID = kInvalidSrsId;
                                    }
                                    
                                }
                            }
                        }

                        if (node_meta.child("Center").child("x") && node_meta.child("Center").child("y") && node_meta.child("Center").child("z"))
                        {

                            center_prior(0) = node_meta.child("Center").child("x").text().as_double();
                            center_prior(1) = node_meta.child("Center").child("y").text().as_double();
                            center_prior(2) = node_meta.child("Center").child("z").text().as_double();
                        }
                        
                        pugi::xml_node node_raotation = node_meta.child("Rotation");
                        if (node_raotation)
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                for (int j = 0; j < 3; ++j)
                                {
                                    std::string flag = std::string("M_") + std::to_string(i) + std::to_string(j);
                                    if (node_raotation && node_raotation.child(flag.c_str()))
                                    {
                                        rotation_prior(i, j) = node_raotation.child(flag.c_str()).text().as_double();
                                    }
                                }
                            }
                        }
                    }
                    
                    
                    
                    
                    
                    
                    
                    
                    
                }

                Eigen::Vector3d color{1.0,1.0,1.0};
                
                pugi::xml_node node_color = photo.child("ColorParameter");
                if (node_color)
                {
                    if (node_color.child("P0") && node_color.child("P1") && node_color.child("P2"))
                    {
                        color[0] = node_color.child("P0").text().as_double();
                        color[1] = node_color.child("P1").text().as_double();
                        color[2] = node_color.child("P2").text().as_double();
                    }
                }
                
                std::string make = "";
                std::string model = "";
                std::string lensmodel = "";
                std::string datetimeoriginal = "";
                double FocalLength, FocalLength35mmEq, Latitude, Longitude, Altitude;
                FocalLength = FocalLength35mmEq = 0.0;
                Latitude = Longitude = Altitude = -DBL_MAX;
                if (photo.child("ExifData"))
                {
                    pugi::xml_node exif_data = photo.child("ExifData");
                    if (exif_data.child("Make"))
                    {
                        make = exif_data.child("Make").text().as_string();
                    }
                    if (exif_data.child("Model"))
                    {
                        model = exif_data.child("Model").text().as_string();
                    }
                    if (exif_data.child("LensModel"))
                    {
                        lensmodel = exif_data.child("LensModel").text().as_string();
                    }
                    if (exif_data.child("DateTimeOriginal"))
                    {
                        datetimeoriginal = exif_data.child("DateTimeOriginal").text().as_string();
                    }
                    if (exif_data.child("FocalLength"))
                    {
                        FocalLength = exif_data.child("FocalLength").text().as_double();
                    }
                    if (exif_data.child("FocalLength35mmEq"))
                    {
                        FocalLength35mmEq = exif_data.child("FocalLength35mmEq").text().as_double();
                    }
                    if (exif_data.child("GPS").child("Latitude"))
                    {
                        Latitude = exif_data.child("GPS").child("Latitude").text().as_double();
                    }
                    if (exif_data.child("GPS").child("Longitude"))
                    {
                        Longitude = exif_data.child("GPS").child("Longitude").text().as_double();
                    }
                    if (exif_data.child("GPS").child("Altitude"))
                    {
                        Altitude = exif_data.child("GPS").child("Altitude").text().as_double();
                    }
                    if (center_prior.x() == -DBL_MAX && Latitude != -DBL_MAX)
                    {
                        center_prior.x() = Longitude;
                        center_prior.y() = Latitude;
                        center_prior.z() = Altitude;
                        prior_srs.definition = GEO84SRS;
                        UpdateSRSMap(prior_srs);                        
                        prior_srs.ID = ExistSRS(prior_srs.definition);
                        prior_srs = srs_map_.at(prior_srs.ID);
                        
                    }
                }
                if (component == 0 && center.x()!=-DBL_MAX&& center_prior.x() == -DBL_MAX)
                {
                    center_prior = center;
                    rotation_prior = rotation;
                    prior_srs = srs_map_.at(blockSRS_id_);
                }
                ExifInfo exifinfo;
               
                exifinfo.make = make;
                exifinfo.model = model;
                exifinfo.dateTime = datetimeoriginal;
                exifinfo.latitude = Latitude;
                exifinfo.longitude = Longitude;
                exifinfo.altitude = Altitude;
                exifinfo.focalLength = FocalLength;
                exifinfo.focalLengthIn35mm = FocalLength35mmEq;
               
                image.SetPhotoGroupID(camera_id);
                image.SetCameraId(camera_id);

                if (rotation != Eigen::Matrix3d::Zero())
                {
                    image.SetRotationMatrix(rotation);
                }
                if (rotation_prior != Eigen::Matrix3d::Zero())
                {
                    image.SetRotationMatrixPrior(rotation_prior);
                }
                if (center.x() != -DBL_MAX)
                {
                    image.SetPosition(center);
                }
                if (center_prior.x() != -DBL_MAX)
                {
                    image.SetPositionPrior(center_prior);
                }
                if (prior_srs.ID != kInvalidSrsId && srs_map_.count(prior_srs.ID))
                {
                    prior_srs = srs_map_.at(prior_srs.ID);
                    image.SetPriorSrs(prior_srs);
                    srs_used_ids.insert(prior_srs.ID);
                }
                
                
                
                image.SetImageId(photo_id);
                image.SetExifinfo(exifinfo);
                image.SetPath(image_path);
                image.SetName(image_name);
                image.SetColorParam(color);
                image.SetRegistered(component);
                if ((near_depth != -DBL_MAX) && (median_depth != -DBL_MAX) && (far_depth != -DBL_MAX))
                {
                    image.SetDepth(Eigen::Vector3d(near_depth, median_depth, far_depth));
                }
                

                auto ret = image_ids_.insert(photo_id);
                

                if (!ret.second)
                {
                    LOG(ERROR) << "image id不唯一";
                    invalidcount++;
                    LOG(ERROR) << photo_id << " " << image_name<< " "<< invalidcount;
                    
                    return  false;
                }
                else
                {
                    group_image.insert(std::make_pair(photo_id, image));
                }
            }


            return true;
        }

        bool BlockObject::ParseControlPoints(std::shared_ptr<ATData> Atdata_, EIGEN_STL_UMAP(point3D_t, ControlPoint)& cps,
            const std::set<image_t>& images_pg, const pugi::xml_node& controlpoints, 
            EIGEN_STL_UMAP(srsid_t, srs_s) srsmap,std::set<srsid_t>& srs_used_ids)
        {
            point3D_t index_point3d = 0;
            
            for (pugi::xml_node cp = controlpoints.first_child(); cp != NULL; cp = cp.next_sibling())
            {
                srs_s srs;
                ControlPoint controlpoint;
                if (cp.child("SRSId"))
                {
                    std::string srstxt = cp.child("SRSId").text().as_string();
                    if (!srstxt.empty())         
                    {
                        auto id = std::stoull(srstxt.c_str());
                        srs.ID = id;

                        if (srs.ID != kInvalidSrsId)
                        {
                            srs = srs_map_[ExistSRS(srsmap.at(srs.ID).definition)];
                        }
                    }
                }
                else
                {
                    
                    srs = srs_map_[blockSRS_id_];
                }
                controlpoint.SetSrs(srs);
                srs_used_ids.insert(srs.ID);
                Track track;
                std::string name = "";
                std::string category = "";
                Eigen::Vector3d position(-DBL_MAX, -DBL_MAX, -DBL_MAX);
                double horizontalaccuarcy = -1;
                double verticalaccuarcy = -1;
                bool checkpoint = false;

                if (cp.child("Name"))
                {
                    name = cp.child("Name").text().as_string();
                }
                if (cp.child("Category"))
                {
                    category = cp.child("Category").text().as_string();
                    if (category == "Full")
                    {
                        controlpoint.SetType(GCP_CONTROL_HV);
                    }
                }
                if (cp.child("CheckPoint"))
                {
                    checkpoint = cp.child("CheckPoint").text().as_bool();
                    if (checkpoint)
                    {
                        controlpoint.SetType(GCP_CHECK_HV);
                    }
                }
                if (cp.child("Position").child("x") && cp.child("Position").child("y") && cp.child("Position").child("z"))
                {
                    position(0) = cp.child("Position").child("x").text().as_double();
                    position(1) = cp.child("Position").child("y").text().as_double();
                    position(2) = cp.child("Position").child("z").text().as_double();
                }
                if (cp.child("HorizontalAccuracy"))
                {
                    horizontalaccuarcy = cp.child("HorizontalAccuracy").text().as_double();
                }
                if (cp.child("VerticalAccuracy"))
                {
                    verticalaccuarcy = cp.child("VerticalAccuracy").text().as_double();
                }
                std::vector<TrackElement> vec_trackele;
                std::set<image_t> imageids;
                for (const auto& ele : cp.children("Measurement"))
                {
                    TrackElement trackelement;
                    image_t image_id = kInvalidImageId;

                    Eigen::Vector2d uv(0.0, 0.0);
                    if (ele.child("PhotoId") && ele.child("x") && ele.child("y"))
                    {
                        image_id = ele.child("PhotoId").text().as_int();
                        if (images_pg.find(image_id) == images_pg.end())
                        {
                            LOGE(String::StringPrintf("Parsing GCP: invalid measurement id %d", image_id));
                            return false;
                        }

                        if(imageids.count(image_id))
                        {
                            LOGE(String::StringPrintf("Parsing GCP: duplicate image id %d for GCP %s", image_id, name.c_str()));
                            return false;
                        }
                        uv(0) = ele.child("x").text().as_double();
                        uv(1) = ele.child("y").text().as_double();
                    }
                    imageids.insert(image_id);
                    Image& img = Atdata_->GetImageMutual(image_id);

                    img.SetPoints2DGCP(index_point3d, uv);
                    

                    trackelement.image_id = image_id;
                    trackelement.xy = uv;
                    trackelement.point2D_idx = index_point3d;
                    vec_trackele.push_back(trackelement);
                }
                if (!vec_trackele.empty())
                {
                    track.AddElements(vec_trackele);
                    controlpoint.GetObjectPointMutual().SetTrack(track);
                }
                controlpoint.SetId(index_point3d);
                

                controlpoint.SetGivenXYZ(position);
                controlpoint.SetName(name);
                controlpoint.SetWeight(Eigen::Vector2d(horizontalaccuarcy, verticalaccuarcy));
                cps.insert(std::make_pair(index_point3d, controlpoint));

               
                
                index_point3d++;
            }
            return true;
        }

        bool BlockObject::ParseTiePoints(std::shared_ptr<ATData> Atdata_, EIGEN_STL_UMAP(point3D_t, Point3D)& tps, const std::set<image_t>& images_pg, 
            const pugi::xml_node& tiepoints,EIGEN_STL_UMAP(point3D_t, Point3D)& usertps)
        {
            point3D_t index_point3d = 0;
            point3D_t index_userpoint3d = 0;

            for (auto& tp : tiepoints.children("TiePoint"))
            {
                
                Point3D point3d;
                Track track;
                Eigen::Vector3d xyz;
                Eigen::Vector3i rgb;
                Eigen::Vector3d color3d;

                pugi::xml_node typenode = tp.child("Type");
                if (typenode )
                {
                    std::string typestr = typenode.text().as_string();
                    String::StringToLower(&typestr);
                    if (typestr == "user")
                    {
                        point3d.SetType(ptt_e::PT_USER);
                        if (tp.child("Name"))
                        {
                            std::string namept = tp.child("Name").text().as_string();
                            point3d.SetName(namept);
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    pugi::xml_node Position = tp.child("Position");
                    if (Position.child("x") && Position.child("y") && Position.child("z"))
                    {
                        xyz(0) = Position.child("x").text().as_double();
                        xyz(1) = Position.child("y").text().as_double();
                        xyz(2) = Position.child("z").text().as_double();
                        point3d.SetXYZ(xyz);
                    }
                    
                    pugi::xml_node Color = tp.child("Color");
                    if (Color.child("Red") && Color.child("Green") && Color.child("Blue"))
                    {
                        color3d(0) = Color.child("Red").text().as_double();
                        color3d(1) = Color.child("Green").text().as_double();
                        color3d(2) = Color.child("Blue").text().as_double();
                        
                        rgb(0) = std::max(std::min(255, int(color3d(0) * 255)), 0);
                        rgb(1) = std::max(std::min(255, int(color3d(1) * 255)), 0);
                        rgb(2) = std::max(std::min(255, int(color3d(2) * 255)), 0);
                        point3d.SetColor(rgb);
                    }
                    
                    
                }
                std::vector<TrackElement> vec_trackele;
                if (point3d.GetType() != ptt_e::PT_USER)
                {
                    for (auto& ele : tp.children("Measurement"))
                    {
                        TrackElement trackelement;
                        image_t image_id = kInvalidImageId;
                        Eigen::Vector2d uv(0.0, 0.0);
                        if (ele.child("PhotoId") && ele.child("x") && ele.child("y"))
                        {
                            image_id = ele.child("PhotoId").text().as_int();
                            if (images_pg.find(image_id) == images_pg.end())
                            {
                                LOGE(String::StringPrintf("Parsing Tiepoints: invalid measurement id %d", image_id));
                                
                                continue;
                            }
                            uv(0) = ele.child("x").text().as_double();
                            uv(1) = ele.child("y").text().as_double();
                            
                            
                            
                        }
                        
                        Image& img = Atdata_->GetImageMutual(image_id);
                        trackelement.image_id = image_id;

                        

                        
                        auto ret = std::find_if(vec_trackele.begin(), vec_trackele.end(), [&](const TrackElement ele) {
                            return ele.image_id == image_id;
                            }
                        );
                        if (ret == vec_trackele.end())
                        {
                            
                            trackelement.point2D_idx = img.AddPoints2D(uv);

                            img.SetPoint3DForPoint2D(trackelement.point2D_idx, index_point3d);
                            trackelement.xy = uv;
                            vec_trackele.push_back(trackelement);
                        }
                    }

                    

                    if (vec_trackele.size() < 2)
                    {
                        for (auto& iter : vec_trackele)
                        {
                            Image& img = Atdata_->GetImageMutual(iter.image_id);
                            img.ResetPoint3DForPoint2D(iter.point2D_idx);
                        }
                        continue;
                    }
                    track.AddElements(vec_trackele);
                    point3d.SetId(index_point3d);
                    point3d.SetTrack(track);

                    {
                        tps.insert(std::make_pair(index_point3d, point3d));
                        index_point3d++;
                    }

                }
                else
                {
                    for (auto& ele : tp.children("Measurement"))
                    {
                        TrackElement trackelement;
                        image_t image_id = kInvalidImageId;
                        Eigen::Vector2d uv(0.0, 0.0);
                        if (ele.child("PhotoId") && ele.child("x") && ele.child("y"))
                        {
                            image_id = ele.child("PhotoId").text().as_int();
                            if (images_pg.find(image_id) == images_pg.end())
                            {
                                LOGE(String::StringPrintf("Parsing Tiepoints: invalid measurement id %d", image_id));
                                return false;
                            }
                            uv(0) = ele.child("x").text().as_double();
                            uv(1) = ele.child("y").text().as_double();
                            
                        }
                        
                        Image& img = Atdata_->GetImageMutual(image_id);
                        trackelement.image_id = image_id;
                        trackelement.point2D_idx = index_userpoint3d;

                        img.SetPoints2DUserPt(index_userpoint3d, uv);
                        trackelement.xy = uv;
                        auto ret = std::find_if(vec_trackele.begin(), vec_trackele.end(), [&](const TrackElement ele) {
                            return ele.image_id == image_id;
                            }
                        );
                        if (ret == vec_trackele.end())
                            vec_trackele.push_back(trackelement);
                    }
                    if (vec_trackele.size() < 1)
                    {
                        continue;
                    }
                    point3d.image_for_userptguide_ = vec_trackele.front().image_id;
                    track.AddElements(vec_trackele);
                    point3d.SetId(index_userpoint3d);
                    point3d.SetTrack(track);
                    usertps.insert(std::make_pair(index_userpoint3d, point3d));
                    index_userpoint3d++;
                }
                
                
            }
            return true;
        }
        
        bool BlockObject::UndistortBlock(const std::string& path, UndistortCameraOptions_s options)
        {
            std::map<camera_t, Camera> undistrotcams;
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }
            const auto& atdat = GetCurrentATMutual();
            auto& reg_image_ids = atdat->GetRegImageIds();
            auto& images = atdat->GetImagesMutual();
            auto& cameras = atdat->GetCamerasMutual();
            auto& points3d = atdat->GetPoints3DMutual();
            auto& gcps = atdat->GetControlPointsMutual();
            auto& usertiepts = atdat->GetUserPoints3DMutual();
            for (auto& id : reg_image_ids)
            {
                Image img = images.at(id);
                
                Camera cam = cameras.at(img.GetCameraId());
                if (undistrotcams.count(cam.GetCameraId()))
                {
                    continue;
                }
                Camera undiscam;
                undiscam.SetModelIdFromName("PINHOLE");
                undiscam.SetCameraModelType(CameraModelType_e::Perspective);
                if (0)
                {
                    cam.GenUndistortCamera(undiscam);

                    if (options.max_image_size > 0)
                    {
                        const double max_image_scale_x =
                            options.max_image_size /
                            static_cast<double>(undiscam.GetWidth());
                        const double max_image_scale_y =
                            options.max_image_size /
                            static_cast<double>(undiscam.GetHeight());
                        const double max_image_scale =
                            std::min(max_image_scale_x, max_image_scale_y);
                        if (max_image_scale < 1.0) {
                            undiscam.Rescale(max_image_scale);
                        }
                    }

                }
                else
                {

                    cam.UndistortCamera(options, undiscam);

                }
                undistrotcams[cam.GetCameraId()] = undiscam;
            }
            std::set<image_t> ids;
#ifdef USE_OPENMP
#pragma omp parallel  for
#endif
            
            for (int i = 0; i < reg_image_ids.size(); i++)
            {
                bool bsec = false;
                int id = reg_image_ids[i];
                Image& image = images.at(id);

                
                
                std::string output_image_path1 = path + "/" + std::to_string(id) + ".jpg";
                
                
                
                
                if ( File::ExistsFile(output_image_path1))
                {
                    bsec = true;

                }
                else
                {
                    Bitmap distorted_bitmap, undistorted_bitmap;
                    std::string imagepath = image.GetPath();
                    imagepath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(imagepath));

                    const std::string input_image_path = File::JoinPaths(imagepath, image.GetName());
                    if (!distorted_bitmap.Read(input_image_path))
                    {
                        LOGE("Cannot read image at path " + input_image_path);
                        continue;
                         
                    }
                    Camera undistorted_camera = undistrotcams.at(image.GetCameraId());
                    undistorted_bitmap.Allocate(static_cast<int>(undistorted_camera.GetWidth()),
                        static_cast<int>(undistorted_camera.GetHeight()),
                        distorted_bitmap.IsRGB());
                    distorted_bitmap.CloneMetadata(&undistorted_bitmap);

                    WarpImageBetweenCameras(cameras.at(image.GetCameraId()), undistorted_camera, distorted_bitmap, &undistorted_bitmap);

                    bsec = undistorted_bitmap.Write(output_image_path1);
                }
#ifdef USE_OPENMP
#pragma omp critical
#endif
                {
                    if (bsec)
                    {
                        ids.insert(id);
                    }
                }

            }

            auto imageids = atdat->GetImagesIds();
            std::set<camera_t> camids;
            std::set<image_t> imgids;
            for (auto& id : imageids)
            {
                if (!images.count(id))
                {
                    continue;
                }
                auto& image = images.at(id);
                if (!ids.count(id))
                {
                    imgids.insert(id);


                }
                else
                {
                    image.SetPath(path);
                    std::string imagename =  image.GetName();
                    std::string output_image_path = path + "/" + std::to_string(image.GetImageId()) + "_" + image.GetName();
                    if (File::ExistsFile(output_image_path))
                    {
                        imagename = std::to_string(image.GetImageId()) + "_" + image.GetName();

                    }






                    std::string output_image_path2 = path + "/" + std::to_string(id) + ".jpg";
                    if (File::ExistsFile(output_image_path2))
                    {
                        imagename = std::to_string(id) + ".jpg";

                    }
                    std::string output_image_path1 = path + "/" + image.GetName();
                    if (File::ExistsFile(output_image_path1))
                    {
                        imagename = image.GetName();

                    }

                    image.SetName(imagename);

                    
                    for (point2D_t idx = 0; idx < image.GetNumPoints2D(); idx++)
                    {
                        auto& point2d = image.GetPoint2DMutual(idx);
                        auto pt3did = point2d.GetPoint3DId();





                        if (point2d.HasPoint3D())
                        {


                            if (points3d.count(pt3did))
                            {
                                auto newxy = undistrotcams.at(image.GetCameraId()).WorldToImage(cameras.at(image.GetCameraId()).ImageToWorld(point2d.GetXY()));
                                bool insideimg = newxy.x() >= 0 &&
                                    newxy.x() < undistrotcams.at(image.GetCameraId()).GetWidth()
                                    && newxy.y() >= 0 &&
                                    newxy.y() < undistrotcams.at(image.GetCameraId()).GetHeight();
                                if (insideimg)
                                {
                                    if (TrackElement* te = FindTrackElementByImageMutual(points3d.at(pt3did).GetTrackMutual(), id))
                                    {
                                        te->xy = newxy;
                                    }

                                    point2d.SetXY(newxy);
                                }
                                else
                                {
                                    points3d.at(pt3did).GetTrackMutual().DeleteElementByImageId(id);
                                }
                            }

                        }
                        else
                        {

                        }

                    }
                    for (auto& iter : image.GetPoints2DGCPMap())
                    {
                        if (gcps.count(iter.first))
                        {


                            auto newxy = undistrotcams.at(image.GetCameraId()).WorldToImage(cameras.at(image.GetCameraId()).ImageToWorld(iter.second));

                            bool insideimg = newxy.x() >= 0 &&
                                newxy.x() < undistrotcams.at(image.GetCameraId()).GetWidth()
                                && newxy.y() >= 0 &&
                                newxy.y() < undistrotcams.at(image.GetCameraId()).GetHeight();
                            if (insideimg)
                            {
                                iter.second = newxy;
                                if (TrackElement* te = FindTrackElementByImageMutual(gcps.at(iter.first).GetObjectPointMutual().GetTrackMutual(), id))
                                {
                                    te->xy = newxy;
                                }
                            }
                            else
                            {
                                gcps.at(iter.first).GetObjectPointMutual().GetTrackMutual().DeleteElementByImageId(id);
                            }
                        }

                    }
                    for (auto& iter : image.GetUserPtsPoint2DMutual())
                    {
                        if (usertiepts.count(iter.first))
                        {
                            auto newxy = undistrotcams.at(image.GetCameraId()).WorldToImage(cameras.at(image.GetCameraId()).ImageToWorld(iter.second));
                            bool insideimg = newxy.x() >= 0 &&
                                newxy.x() < undistrotcams.at(image.GetCameraId()).GetWidth()
                                && newxy.y() >= 0 &&
                                newxy.y() < undistrotcams.at(image.GetCameraId()).GetHeight();
                            if (insideimg)
                            {
                                iter.second = newxy;
                                if (TrackElement* te = FindTrackElementByImageMutual(usertiepts.at(iter.first).GetTrackMutual(), id))
                                {
                                    te->xy = newxy;
                                }
                            }
                            else
                            {
                                usertiepts.at(iter.first).GetTrackMutual().DeleteElementByImageId(id);
                            }
                        }
                    }
                    camids.insert(image.GetCameraId());
                }

            }
            RemoveImages(imgids);
            cameras.clear();
            for (auto& id : camids)
            {
                cameras[id] = undistrotcams.at(id);
            }

        }


        void BlockObject::SerializePhotoGroup(const ATData& Atdata_, const PhotoGroup& pg, 
            pugi::xml_node node_pg, BlockExportOptions block_export_options)
        {
            
            if (!pg.GetName().empty())
            {
                pugi::xml_node pg_name = node_pg.append_child("Name");
                pg_name.append_child(pugi::node_pcdata).set_value(pg.GetName().c_str());
            }
            auto camid = pg.GetCamera().GetCameraId();
            if (!Atdata_.GetCameras().count(camid))
            {
                LOGI("cam is not exists,pg id is "+ std::to_string(pg.GetId()));
                return;
            }
            
            Camera camera = Atdata_.GetCamera(camid);
            
            if (camera.GetWidth() > 0 && camera.GetHeight() > 0)
            {
                pugi::xml_node image_dimension = node_pg.append_child("ImageDimensions");
                pugi::xml_node width = image_dimension.append_child("Width");
                pugi::xml_node height = image_dimension.append_child("Height");
                width.append_child(pugi::node_pcdata).set_value(std::to_string(camera.GetWidth()).c_str());
                height.append_child(pugi::node_pcdata).set_value(std::to_string(camera.GetHeight()).c_str());
            }

            
            if (camera.GetCameraModelType() == CameraModelType_e::Fisheye || camera.GetCameraModelType() == CameraModelType_e::Perspective)
            {
                pugi::xml_node CameraModelType = node_pg.append_child("CameraModelType");
                CameraModelType.append_child(pugi::node_pcdata).set_value(camera.GetCameraModelType() == 0 ? "Perspective" : "Fisheye");
            }

            
            {
                if (camera.GetSensorSize() > 0.0 && camera.GetFocalLengthMM() > 0.0)
                {
                    pugi::xml_node SensorSize = node_pg.append_child("SensorSize");
                    SensorSize.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetSensorSize()).c_str());
                    pugi::xml_node FocalLength = node_pg.append_child("FocalLength");
                    FocalLength.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetFocalLengthMM()).c_str());
                }
                else
                {
                    if (camera.GetMeanFocalLength() > 0.0)
                    {
                        pugi::xml_node FocalLengthPixels = node_pg.append_child("FocalLengthPixels");
                        FocalLengthPixels.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetMeanFocalLength()).c_str());
                        double fx = camera.GetFocalLengthX();
                        double fy = camera.GetFocalLengthY();
                        if (fx > 0. && fy > 0.)
                        {
                            double ratio = fx / fy;
                            if (fabs(ratio - 1.0) < 1e-6)
                            {
                            }
                            else
                            {
                                pugi::xml_node rationode = node_pg.append_child("AspectRatio");
                                rationode.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(ratio).c_str());
                            }
                        }
                    }
                }
            }
            
            
            
            
            
            
            
            


            
            if (camera.GetPrincipalPointX() > 0.0 && camera.GetPrincipalPointY() > 0.0)
            {
                pugi::xml_node pp = node_pg.append_child("PrincipalPoint");
                pugi::xml_node cx = pp.append_child("x");
                cx.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetPrincipalPointX()).c_str());
                pugi::xml_node cy = pp.append_child("y");
                cy.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetPrincipalPointY()).c_str());
            }

            
            if (!camera.GetCameraOrientation().empty())
            {
                pugi::xml_node CameraOrientation = node_pg.append_child("CameraOrientation");
                CameraOrientation.append_child(pugi::node_pcdata).set_value(camera.GetCameraOrientation().c_str());
            }
            
            
            if (camera.HasDistortion())
            {
                pugi::xml_node distortion = node_pg.append_child("Distortion");
                pugi::xml_node K1 = distortion.append_child("K1");
                pugi::xml_node K2 = distortion.append_child("K2");
                pugi::xml_node K3 = distortion.append_child("K3");
                pugi::xml_node P1 = distortion.append_child("P1");
                pugi::xml_node P2 = distortion.append_child("P2");
                
                
                

                K1.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetParams()[4]).c_str());
                K2.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetParams()[5]).c_str());
                P1.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetParams()[6]).c_str());
                P2.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetParams()[7]).c_str());
                K3.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(camera.GetParams()[8]).c_str());
                
                
                
            }

            
            
            
            if (!camera.GetFixed().empty())
            {
                pugi::xml_node fixednode = node_pg.append_child("Fixed");
                std::string fixedstr = "";
                for (auto& index : camera.GetFixed())
                {
                    fixedstr += std::to_string(index)+",";

                }
                fixednode.append_child(pugi::node_pcdata).set_value(fixedstr.c_str());
            }
            
            for (const auto& image_id : pg.GetGroupImageIds())
            {

                
                
                
                
                
                if(!Atdata_.GetImages().count(image_id))
                {
                    continue;
                }
                pugi::xml_node node_photos = node_pg.append_child("Photo");
                if (image_id >= 0)
                {
                    pugi::xml_node Id = node_photos.append_child("Id");
                    Id.append_child(pugi::node_pcdata).set_value(std::to_string(image_id).c_str());
                }
                else
                {
                    continue;
                }
                
                
                
                
                
                
                std::string image_path = Atdata_.GetImage(image_id).GetPath() + "/" + Atdata_.GetImage(image_id).GetName();
                image_path = File::EnsureUnifySlash(image_path);
                if (!image_path.empty())
                {
                    pugi::xml_node ImagePath = node_photos.append_child("ImagePath");
                    ImagePath.append_child(pugi::node_pcdata).set_value(image_path.c_str());
                }

                
                {
                    
                    pugi::xml_node Component = node_photos.append_child("Component");
                    Component.append_child(pugi::node_pcdata).set_value(std::to_string(Atdata_.GetImage(image_id).IsRegistered()).c_str());
                }
                fix_e fixedstatus = Atdata_.GetImage(image_id).GetFixStatus();
                if (fixedstatus == fix_e::EOE_FIXED)
                {
                    pugi::xml_node fixedstatusnode = node_photos.append_child("Fixed");
                    fixedstatusnode.append_child(pugi::node_pcdata).set_value(std::to_string((int)fixedstatus).c_str());
                }
                pugi::xml_node pose = node_photos.append_child("Pose");
                
                Eigen::Matrix3d R = Atdata_.GetImage(image_id).GetRotationMatrix();
                
                if (Atdata_.GetImage(image_id).HasRotationMatrix())
                {
                    pugi::xml_node rotation = pose.append_child("Rotation");



                    if (block_export_options.rotformat_ == rot_format_e::ROTFORMAT_OMK)
                    {
                        double omga, phi, kappa;
                        AlgorithmBase::ConvertRotmat2OPK(R,omga, phi, kappa );
                        omga = R2FD(omga);
                        phi = R2FD(phi);
                        kappa = R2FD(kappa);
                        pugi::xml_node anglue1 = rotation.append_child("Omega");
                        anglue1.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(omga).c_str());
                        pugi::xml_node anglue2 = rotation.append_child("Phi");
                        anglue2.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(phi).c_str());
                        pugi::xml_node anglue3 = rotation.append_child("Kappa");
                        anglue3.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(kappa).c_str());
                    }
                    else if (block_export_options.rotformat_ == rot_format_e::ROTFORMAT_YPR)
                    {
                        
                        auto ypr = AlgorithmBase::RotationInnerToYPR(R);
                        
                        pugi::xml_node anglue1 = rotation.append_child("Yaw");
                        anglue1.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(ypr.x()).c_str());
                        pugi::xml_node anglue2 = rotation.append_child("Pitch");
                        anglue2.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(ypr.y()).c_str());
                        pugi::xml_node anglue3 = rotation.append_child("Roll");
                        anglue3.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(ypr.z()).c_str());
                    }
                    else
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            for (int j = 0; j < 3; j++)
                            {
                                std::string R_value = "M_" + std::to_string(i) + std::to_string(j);
                                pugi::xml_node node = rotation.append_child(R_value.c_str());
                                node.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(R(i, j)).c_str());
                            }
                        }
                    }
                }
                
                Eigen::Vector3d center = Atdata_.GetImage(image_id).GetPosition();

                if (Atdata_.GetImage(image_id).HasPosition())
                {
                    Eigen::Vector3d dst_center = center;

                    pugi::xml_node c = pose.append_child("Center");
                    pugi::xml_node x = c.append_child("x");
                    pugi::xml_node y = c.append_child("y");
                    pugi::xml_node z = c.append_child("z");

                    x.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(dst_center(0)).c_str());
                    y.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(dst_center(1)).c_str());
                    z.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(dst_center(2)).c_str());
                }

                
                {
                    Eigen::Vector3d center_prior = Atdata_.GetImage(image_id).GetPositionPrior();
                    if (Atdata_.GetImage(image_id).GetPriorSrs().ID != kInvalidSrsId && Atdata_.GetImage(image_id).HasPositionPrior())
                    {
                        pugi::xml_node metadata = pose.append_child("Metadata");
                        pugi::xml_node SRSId = metadata.append_child("SRSId");
                        SRSId.append_child(pugi::node_pcdata).set_value(std::to_string(Atdata_.GetImage(image_id).GetPriorSrs().ID).c_str());

                        pugi::xml_node c_prior = metadata.append_child("Center");

                        {
                            pugi::xml_node x_prior = c_prior.append_child("x");
                            pugi::xml_node y_prior = c_prior.append_child("y");
                            pugi::xml_node z_prior = c_prior.append_child("z");


                            x_prior.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(center_prior(0)).c_str());
                            y_prior.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(center_prior(1)).c_str());
                            z_prior.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(center_prior(2)).c_str());
                        }
                        
                        Eigen::Matrix3d R_prior = Atdata_.GetImage(image_id).GetRotationMatrixPrior();
                        
                        if (Atdata_.GetImage(image_id).HasRotationMatrixPrior())
                        {
                            pugi::xml_node rotation_prior = metadata.append_child("Rotation");
                            for (int i = 0; i < 3; i++)
                            {
                                for (int j = 0; j < 3; j++)
                                {
                                    std::string R_value = "M_" + std::to_string(i) + std::to_string(j);
                                    pugi::xml_node node = rotation_prior.append_child(R_value.c_str());
                                    node.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(R_prior(i, j)).c_str());
                                }
                            }
                        }


                    }
                }
                if (Atdata_.GetImage(image_id).HasColorParams())
                {
                    pugi::xml_node ColorParameter = node_photos.append_child("ColorParameter");
                    Eigen::Vector3d colorpara = Atdata_.GetImage(image_id).GetColorParam();
                    pugi::xml_node P0 = ColorParameter.append_child("P0");
                    P0.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(colorpara[0], 20).c_str());
                    pugi::xml_node P1 = ColorParameter.append_child("P1");
                    P1.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(colorpara[1], 20).c_str());
                    pugi::xml_node P2 = ColorParameter.append_child("P2");
                    P2.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(colorpara[2], 20).c_str());
                }

                Eigen::Vector3d depth = Atdata_.GetImage(image_id).GetDepth();
                if (Atdata_.GetImage(image_id).HasDepth())
                {
                    pugi::xml_node NearDepth = node_photos.append_child("NearDepth");
                    NearDepth.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(depth(0)).c_str());

                    pugi::xml_node MedianDepth = node_photos.append_child("MedianDepth");
                    MedianDepth.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(depth(1)).c_str());

                    pugi::xml_node FarDepth = node_photos.append_child("FarDepth");
                    FarDepth.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(depth(2)).c_str());
                }

                
                {
                    
                    ExifInfo exifinfo = Atdata_.GetImage(image_id).GetExifinfo();
                    pugi::xml_node ExifData = node_photos.append_child("ExifData");
                    pugi::xml_node GPS;

                    if (exifinfo.latitude != -DBL_MAX)
                    {
                        GPS = ExifData.append_child("GPS");
                        pugi::xml_node Latitude = GPS.append_child("Latitude");
                        Latitude.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(exifinfo.latitude).c_str());
                    }
                    if (exifinfo.longitude != -DBL_MAX)
                    {
                        pugi::xml_node Longitude = GPS.append_child("Longitude");
                        Longitude.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(exifinfo.longitude).c_str());
                    }
                    if (exifinfo.altitude != -DBL_MAX)
                    {
                        pugi::xml_node Altitude = GPS.append_child("Altitude");
                        Altitude.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(exifinfo.altitude).c_str());
                    }
                    if (!exifinfo.make.empty())
                    {
                        pugi::xml_node Make = ExifData.append_child("Make");
                        Make.append_child(pugi::node_pcdata).set_value(exifinfo.make.c_str());
                    }
                    if (!exifinfo.model.empty())
                    {
                        pugi::xml_node Model = ExifData.append_child("Model");
                        Model.append_child(pugi::node_pcdata).set_value(exifinfo.model.c_str());
                    }
                    if (!exifinfo.dateTime.empty())
                    {
                        pugi::xml_node DateTimeOriginal = ExifData.append_child("DateTimeOriginal");
                        DateTimeOriginal.append_child(pugi::node_pcdata).set_value(exifinfo.dateTime.c_str());
                    }
                    if (exifinfo.focalLength != 0.0)
                    {
                        pugi::xml_node FocalLength = ExifData.append_child("FocalLength");
                        FocalLength.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(exifinfo.focalLength).c_str());
                    }
                    if (exifinfo.focalLengthIn35mm != 0.0)
                    {
                        pugi::xml_node FocalLength35mmEq = ExifData.append_child("FocalLength35mmEq");
                        FocalLength35mmEq.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(exifinfo.focalLengthIn35mm).c_str());
                    }
                }
            }
        }

        void BlockObject::SerializeControlPoint(EIGEN_STL_UMAP(image_t, std::string)& image_map, const ControlPoint& cp, 
            pugi::xml_node node_cp)
        {
            
            pugi::xml_node idnode = node_cp.append_child("Id");
            idnode.append_child(pugi::node_pcdata).set_value(std::to_string(cp.GetId()).c_str());
            pugi::xml_node typenode = node_cp.append_child("PointType");
            typenode.append_child(pugi::node_pcdata).set_value(std::to_string(0).c_str());
            pugi::xml_node SRSId = node_cp.append_child("SRSId");
            srsid_t id = cp.GetSrs().ID;
            SRSId.append_child(pugi::node_pcdata).set_value(std::to_string(id).c_str());

            pugi::xml_node Name = node_cp.append_child("Name");
            Name.append_child(pugi::node_pcdata).set_value(cp.GetName().c_str());
            std::string category = "";
            pugi::xml_node Position = node_cp.append_child("Position");
            pugi::xml_node x = Position.append_child("x");
            pugi::xml_node y = Position.append_child("y");
            pugi::xml_node z = Position.append_child("z");
            Eigen::Vector3d xyz = cp.GetGivenXYZ();

            x.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(0)).c_str());
            y.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(1)).c_str());
            z.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(2)).c_str());

            pugi::xml_node HorizontalAccuracy = node_cp.append_child("HorizontalAccuracy");
            pugi::xml_node VerticalAccuracy = node_cp.append_child("VerticalAccuracy");
            HorizontalAccuracy.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(cp.GetWeight()(0)).c_str());
            VerticalAccuracy.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(cp.GetWeight()(1)).c_str());

            gpt_e gpte;

            bool checkpoint = false;
            switch (cp.GetType())
            {
            case 1:
                category = "Horizontal";
                break;
            case 2:
                category = "Vertical";
                break;
            case 3:
                category = "Full";
                break;
            case 4:
                checkpoint = true;
                break;
            case 5:
                checkpoint = true;
                break;
            case 6:
                checkpoint = true;
                break;
            }


            pugi::xml_node Category = node_cp.append_child("Category");
            Category.append_child(pugi::node_pcdata).set_value(category.c_str());
            pugi::xml_node CheckPoint = node_cp.append_child("CheckPoint");
            CheckPoint.append_child(pugi::node_pcdata).set_value(checkpoint ? "true" : "false");
            std::vector<TrackElement> vector = cp.GetObjectPoint().GetTrack().GetElements();
            std::set<point3D_t> elesidsets;
            std::vector<point3D_t> elesidvecs;
            for (const auto& it : vector)
            {
                if (!image_map.count(it.image_id))
                {
                    continue;
                }

                pugi::xml_node Measurement = node_cp.append_child("Measurement");
                pugi::xml_node photo_id = Measurement.append_child("PhotoId");
                photo_id.append_child(pugi::node_pcdata).set_value(std::to_string(it.image_id).c_str());
                elesidsets.insert(it.image_id);
                elesidvecs.push_back(it.image_id);
                {
                    pugi::xml_node imagepathnode = Measurement.append_child("ImagePath");
                   
                    std::string imagepath = image_map.at(it.image_id);
                    imagepathnode.append_child(pugi::node_pcdata).set_value(imagepath.c_str());
                }
                pugi::xml_node x = Measurement.append_child("x");
                x.append_child(pugi::node_pcdata).set_value(std::to_string(it.xy.x()).c_str());
                pugi::xml_node y = Measurement.append_child("y");
                y.append_child(pugi::node_pcdata).set_value(std::to_string(it.xy.y()).c_str());
            }
            if (elesidvecs.size() != elesidsets.size())
            {
                std::cout << cp.GetName() << std::endl;
            }
        }


        void BlockObject::SerializeControlPoint(const ATData& Atdata,const ControlPoint& cp, pugi::xml_node node_cp, bool forOnlyMeasureMode)
        {
            pugi::xml_node SRSId = node_cp.append_child("SRSId");
            srsid_t id = cp.GetSrs().ID;
            SRSId.append_child(pugi::node_pcdata).set_value(std::to_string(id).c_str());

            pugi::xml_node Name = node_cp.append_child("Name");
            Name.append_child(pugi::node_pcdata).set_value(cp.GetName().c_str());
            std::string category = "";
            pugi::xml_node Position = node_cp.append_child("Position");
            pugi::xml_node x = Position.append_child("x");
            pugi::xml_node y = Position.append_child("y");
            pugi::xml_node z = Position.append_child("z");
            Eigen::Vector3d xyz = cp.GetGivenXYZ();

            x.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(0)).c_str());
            y.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(1)).c_str());
            z.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(2)).c_str());

            pugi::xml_node HorizontalAccuracy = node_cp.append_child("HorizontalAccuracy");
            pugi::xml_node VerticalAccuracy = node_cp.append_child("VerticalAccuracy");
            HorizontalAccuracy.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(cp.GetWeight()(0)).c_str());
            VerticalAccuracy.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(cp.GetWeight()(1)).c_str());

            gpt_e gpte;

            bool checkpoint = false;
            switch (cp.GetType())
            {
            case 1:
                category = "Horizontal";
                break;
            case 2:
                category = "Vertical";
                break;
            case 3:
                category = "Full";
                break;
            case 4:
                checkpoint = true;
                break;
            case 5:
                checkpoint = true;
                break;
            case 6:
                checkpoint = true;
                break;
            }


            pugi::xml_node Category = node_cp.append_child("Category");
            Category.append_child(pugi::node_pcdata).set_value(category.c_str());
            pugi::xml_node CheckPoint = node_cp.append_child("CheckPoint");
            CheckPoint.append_child(pugi::node_pcdata).set_value(checkpoint ? "true" : "false");
            std::vector<TrackElement> vector = cp.GetObjectPoint().GetTrack().GetElements();
            for (const auto& it : vector)
            {
                pugi::xml_node Measurement = node_cp.append_child("Measurement");
                pugi::xml_node photo_id = Measurement.append_child("PhotoId");
                photo_id.append_child(pugi::node_pcdata).set_value(std::to_string(it.image_id).c_str());

                if (forOnlyMeasureMode)
                {
                    pugi::xml_node imagepathnode = Measurement.append_child("ImagePath");
                    std::string imagepath = Atdata.GetImage(it.image_id).GetPath() + "/" + Atdata.GetImage(it.image_id).GetName();
                    imagepathnode.append_child(pugi::node_pcdata).set_value(imagepath.c_str());
                }
                pugi::xml_node x = Measurement.append_child("x");
                x.append_child(pugi::node_pcdata).set_value(std::to_string(it.xy.x()).c_str());
                pugi::xml_node y = Measurement.append_child("y");
                y.append_child(pugi::node_pcdata).set_value(std::to_string(it.xy.y()).c_str());
            }

        }

        void BlockObject::SerializeTiePoint(const Point3D& tp, pugi::xml_node node_tps, bool from_user_points_map)
        {
            pugi::xml_node TiePoint = node_tps.append_child("TiePoint");
            if (from_user_points_map || tp.GetType() == ptt_e::PT_USER)
            {
                
                const point3D_t id = tp.GetId();
                pugi::xml_node idnode = TiePoint.append_child("Id");
                idnode.append_child(pugi::node_pcdata).set_value(
                    id != kInvalidPoint3DId ? std::to_string(id).c_str() : "0");
                pugi::xml_node name_node = TiePoint.append_child("Name");
                name_node.append_child(pugi::node_pcdata).set_value(tp.GetName().c_str());
                pugi::xml_node type_node = TiePoint.append_child("Type");
                type_node.append_child(pugi::node_pcdata).set_value("User");
                for (const auto& ele : tp.GetTrack().GetElements())
                {
                    pugi::xml_node Measurement = TiePoint.append_child("Measurement");
                    pugi::xml_node PhotoId = Measurement.append_child("PhotoId");
                    PhotoId.append_child(pugi::node_pcdata).set_value(std::to_string(ele.image_id).c_str());
                    pugi::xml_node x_ele = Measurement.append_child("x");
                    pugi::xml_node y_ele = Measurement.append_child("y");
                    x_ele.append_child(pugi::node_pcdata).set_value(
                        File::ToStringWithHighPrecision(ele.xy(0)).c_str());
                    y_ele.append_child(pugi::node_pcdata).set_value(
                        File::ToStringWithHighPrecision(ele.xy(1)).c_str());
                }
                return;
            }

            
            {
                
                Eigen::Vector3d tiepoint;
                bool have_world_xyz = false;
                if (tp.HasXYZ()) {
                    tiepoint = tp.GetXYZ();
                    have_world_xyz = true;
                } else {
                    const Eigen::Vector3d est = tp.GetEstimatedXYZ();
                    if (est.x() != -DBL_MAX && est.y() != -DBL_MAX && est.z() != -DBL_MAX) {
                        tiepoint = est;
                        have_world_xyz = true;
                    }
                }
                if (have_world_xyz) {
                    pugi::xml_node Position = TiePoint.append_child("Position");
                    pugi::xml_node x = Position.append_child("x");
                    pugi::xml_node y = Position.append_child("y");
                    pugi::xml_node z = Position.append_child("z");
                    x.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(tiepoint[0]).c_str());
                    y.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(tiepoint[1]).c_str());
                    z.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(tiepoint[2]).c_str());
                }

                pugi::xml_node Color = TiePoint.append_child("Color");
                pugi::xml_node Red = Color.append_child("Red");
                pugi::xml_node Green = Color.append_child("Green");
                pugi::xml_node Blue = Color.append_child("Blue");

                Red.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(1.0 * tp.GetColor()(0) / 255).c_str());
                Green.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(1.0 * tp.GetColor()(1) / 255).c_str());
                Blue.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(1.0 * tp.GetColor()(2) / 255).c_str());
            }
            for (const auto& ele : tp.GetTrack().GetElements())
            {
                pugi::xml_node Measurement = TiePoint.append_child("Measurement");
                pugi::xml_node PhotoId = Measurement.append_child("PhotoId");
                PhotoId.append_child(pugi::node_pcdata).set_value(std::to_string(ele.image_id).c_str());

                pugi::xml_node x_ele = Measurement.append_child("x");
                pugi::xml_node y_ele = Measurement.append_child("y");
                x_ele.append_child(pugi::node_pcdata).set_value(
                    File::ToStringWithHighPrecision(ele.xy(0), 2).c_str());
                y_ele.append_child(pugi::node_pcdata).set_value(
                    File::ToStringWithHighPrecision(ele.xy(1), 2).c_str());
            }
        }


        void BlockObject::UpdateSRSMap(const srs_s& srs)
        {
            std::string definition = srs.definition;
            if (ExistSRS(definition) == kInvalidSrsId)
            {
                srs_s srs_temp;
                srs_temp.ID = GenerateValidSrsId();
                srs_temp.definition = definition;
                
                size_t index_colon = definition.find_first_of(":");
                size_t index_comma = definition.find_first_of(",");
                std::string coord_name = definition.substr(0, index_colon);
                std::string lat = definition.substr(index_colon + 1, index_comma - index_colon - 1);
                std::string lon = definition.substr(index_comma + 1);
                if (coord_name == "ENU")
                {
                    srs_temp.name = "Local East-North-Up (ENU); origin: " + lat + "N " + lon + "E";
                    srs_temp.type = coord_system_type_e::LOCAL_ENU;
                }
                else
                {
                    srs_s srs_temp1 = CoordinateDescriptor::GetSRSFromDefinition(definition);
                    if (srs_temp1.type == coord_system_type_e::LOCAL)
                    {
                        
                        srs_temp.name = srs.name;
                        srs_temp.definition = srs.definition;
                        srs_temp.type = srs.type;
                    }
                    else
                    {
                        
                        srs_temp.name = srs_temp1.name;
                        srs_temp.type = srs_temp1.type;
                    }
                }
                srs_map_.insert(std::make_pair(srs_temp.ID, srs_temp));
            }
        }

        bool BlockObject::GetTiepointStatus()
        {
            return btiepoint_loaded_;
        }

        
        bool BlockObject::LoadBlockATData(std::shared_ptr<ATData>& ATdata, BlockImportOptions block_import_options)
        {
            std::string file_path = block_info_.Block_XML;

            
            std::string tiepoints_file_path = block_info_.Tiepoints;
            tiepoints_file_path = File::EnsureUnifySlash(tiepoints_file_path);
    
            if (!File::ExistsFile(tiepoints_file_path) && File::ExistsFile(File::EnsureUnifySlash(path_ + "/" + TIEPOINTS)))
            {
                tiepoints_file_path = File::EnsureUnifySlash(path_ + "/" + TIEPOINTS);
            }

            if (!File::ExistsFile(tiepoints_file_path))
            {
                tiepoints_file_path = "";
            }

            
#if 0       
            if (!File::ExistsFile(block_info_.Tiepoints) && File::ExistsFile(File::EnsureUnifySlash(path_+"/" + TIEPOINTS)))
            {

                block_info_.Tiepoints = File::EnsureUnifySlash(path_ + "/" + TIEPOINTS);
            }

            std::string tiepoints_file_path = block_info_.Tiepoints;
#endif

            
            if (block_import_options.load_images_)
            {
                
                if (!LoadATBinaryWithoutTiepoints(file_path, ATdata))
                {
                    
                    LOGE("Load Block without tiepoints error!");
                    return false;
                }
                bool shoudupdate = false;

                auto images = ATdata->GetImages();
                auto points3D = ATdata->GetPoints3D();

                
                
                
                if ((!block_info_.isFinished && status_ == STATUS_COMPLETE))
                {
                    shoudupdate = true;
                    if ((ATGroups_.count(0)) && CanSubmitRecon())
                    {
                        shoudupdate = false;
                        block_info_.isFinished = true;
                    }

                }
                if (ATdata->GetRegImageIds().empty() && status_ == jobsta_e::STATUS_COMPLETE)
                {
                    shoudupdate = true;
                    block_info_.isFinished = false;

                }

                if (block_info_.isFinished && status_ == STATUS_COMPLETE && ATGroups_.empty())
                {
                    shoudupdate = true;
                    block_info_.isFinished = false;
                }


                
                if (shoudupdate && !block_import_options.suppress_update_complete_at_file_on_reload_)
                {
                    {
                        
                        if (AI3D_SUCCESS != UpdateCompleteATFile())
                        {
                            status_ = STATUS_NEW;
                            block_info_.isFinished = false;
                        }
                            
                    }
                }
            }

            
            if (block_import_options.load_tiepoint_ && !tiepoints_file_path.empty())
            {

                if (!GetTiepointStatus() || block_import_options.force_reload_tiepoints_from_disk_)
                {




#if 0
                    if (!LoadTiepointsBinary(tiepoints_file_path, ATdata))
                    {

                        LOGE("LoadTiepoints Binary error!");
                        return false;
                    }
                    else
                    {

                        GetCurrentATMutual() = ATdata;
                    }

                    auto images = ATdata->GetImages();
                    auto points3D = ATdata->GetPoints3D();




#else
                    if (!LoadTiepointsBinary(tiepoints_file_path, ATdata))
                    {

                        LOGE("LoadTiepoints Binary error!");
                        
                    }
                    else
                    {


                        GetCurrentATMutual() = ATdata;

                        auto images = ATdata->GetImages();
                        auto points3D = ATdata->GetPoints3D();




                    }
#endif
                }
            }

            
            
            {
                const std::string constraint_path =
                    File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + CONSTRAINTFILE);
                if (File::ExistsFile(constraint_path)) {
                    if (!ATdata->LoadConstraint(constraint_path)) {
                        LOGE(std::string("LoadConstraint failed: ") + constraint_path);
                    }
                }
            }

            
            
            
            if (status_ == jobsta_e::STATUS_COMPLETE && ATdata) {
                if (!ATGroups_.count(0) ||
                    ATGroups_.at(0).GetATDataMutual().get() != ATdata.get()) {
                    ATGroups_[0].SetATData(ATdata);
                }
            }
            if (status_ == jobsta_e::STATUS_COMPLETE && ATGroups_.count(0)) {
                ATData_ = ATGroups_.at(0).GetATDataMutual();
            }


            return true;
        }

        bool BlockObject::ReloadCurrentATFromPersistedFilesForExportOrReconstruction()
        {
            if (status_ != jobsta_e::STATUS_COMPLETE || !block_info_.isFinished) {
                return true;
            }
            if (GetCurrentAT() == nullptr) {
                return false;
            }
            
            std::shared_ptr<ATData> holder = std::make_shared<ATData>();
            BlockImportOptions opts;
            opts.load_tiepoint_ = true;
            opts.load_images_ = true;
            opts.force_reload_tiepoints_from_disk_ = true;
            opts.suppress_update_complete_at_file_on_reload_ = true;
            if (!LoadBlockATData(holder, opts)) {
                LOGE(std::string(
                    "ReloadCurrentATFromPersistedFilesForExportOrReconstruction: LoadBlockATData failed block=") +
                     GetIdString());
                return false;
            }
            if (!holder) {
                LOGE(std::string("ReloadCurrentATFromPersistedFilesForExportOrReconstruction: holder null after load block=") +
                     GetIdString());
                return false;
            }
            
            {
                const std::shared_ptr<ATData> cur = GetCurrentAT();
                if (!cur) {
                    LOGE(std::string(
                        "ReloadCurrentATFromPersistedFilesForExportOrReconstruction: GetCurrentAT() null after load block=") +
                         GetIdString());
                    return false;
                }
                if (cur.get() != holder.get()) {
                    LOGW(std::string(
                        "ReloadCurrentATFromPersistedFilesForExportOrReconstruction: GetCurrentAT ptr != loaded holder, "
                        "SetAT0 sync block=") +
                         GetIdString());
                    SetAT0(holder);
                }
                if (GetCurrentAT().get() != holder.get()) {
                    LOGE(std::string(
                        "ReloadCurrentATFromPersistedFilesForExportOrReconstruction: sync failed, GetCurrentAT still "
                        "!= holder block=") +
                         GetIdString());
                    return false;
                }
            }
            if (GetCurrentAT() && GetCurrentAT()->HasTiepoints()) {
                SetTiepointStatus(true);
            }
            
            return true;
        }

        void BlockObject::SetTiepointStatus(bool tiept_loaded)
        {
            btiepoint_loaded_ = tiept_loaded;
        }


        bool BlockObject::LoadATBinaryWithoutTiepoints(const std::string& file_path, std::shared_ptr<ATData>& ATdataCurrent)
        {
            
            
            if (!File::IsFileExistent(file_path))
            {
                LOGE(String::StringPrintf("%s is not exist", file_path.c_str()));
                return false;
            }
            std::ifstream in = File::OpenIfstreamUtf8(file_path, std::ios::binary);
            
            if (!in.is_open())
            {
                LOGE(String::StringPrintf("Reading %s failed!", file_path.c_str()));
                return false;
            }

            ATBlockBinFile atBlockBinFile;
            atBlockBinFile.Deserialize(in);
            std::set<srsid_t> srs_used_ids;
            try
            {
            
                int num_atdata = atBlockBinFile.atdatasize;
                blkversion_ = atBlockBinFile.version;
                
                if (blkversion_ <= -1000)
                {
                    if (num_atdata <= 0)
                        return false;
                }
                else
                {
                    num_atdata = 1;
                }
                {                   
                    for (int data_idx = 0; data_idx < num_atdata; data_idx++)
                    {
                        ATItemData atItemData = atBlockBinFile.atList[data_idx];
                        std::shared_ptr<ATData> ATdata = std::make_shared<ATData>();
                        int num_srs;
                        int atdataid;
                        
                        if (blkversion_ <= -1000)
                        {
                            num_srs = atItemData.num_srs;
                            atdataid = atItemData.data_idx;
                        }
                        else
                        {
                            atdataid = 0;
                            num_srs = blkversion_;
                        }
                        
                        for (int srs_idx = 0; srs_idx < num_srs; srs_idx++)
                        {
                            SRSItemData srsItemData = atItemData.srsVec[srs_idx];
                            srs_s srs_tmp;
                            srsid_t id = srsItemData.id;
                            srs_tmp.ID = id;
                            srs_tmp.name = srsItemData.name;
                            srs_tmp.type = (coord_system_type_e)srsItemData.type;
                            srs_tmp.definition = srsItemData.definition;
                            srs_map_.insert(std::pair<srsid_t, srs_s>(id, srs_tmp));
                        }

                        
                        std::string name = atItemData.block_name;
                        std::string description = atItemData.block_description;
#ifdef WIN32
                        // name = UTF82GBK(name);
                        // description = UTF82GBK(description);
#endif 
                        name_ = name;
                        description_ = description;
                        blockSRS_id_ = atItemData.blockSRS_id;
                        srs_used_ids.insert(blockSRS_id_);
                        std::string local_srs_definition = atItemData.local_srs_definition;
                        ATdata->SetLocalSrs(local_srs_definition);

                        
                        int num_photogroups = atItemData.num_photogroups;
                        std::vector<image_t> reg_image_ids;                       
                        for (int pg_idx = 0; pg_idx < num_photogroups; pg_idx++)
                        {
                            ATCameraData atCameraData = atItemData.photoGroups[pg_idx];
                            PhotoGroup pg;
                            group_t group_id = atCameraData.cameraData.group_id;
                            pg.SetId(group_id);
                            pg.SetName(atCameraData.cameraData.group_name);

                            Camera camera;
                            camera_t camera_id = atCameraData.cameraData.id;
                            camera.SetCameraId(camera_id);
                            camera.SetModelId(atCameraData.cameraData.cameraModelid);
                            camera.SetCameraName(atCameraData.cameraData.camera_name);
                            camera.SetFocalLengthMM(atCameraData.cameraData.prior_focal);
                            camera.SetFocalLengthIn35mm(atCameraData.cameraData.f_35eq);
                            camera.SetSensorSize(atCameraData.cameraData.sensorsize);
                            camera.SetMake(atCameraData.cameraData.num_make);
                            camera.SetMakeModel(atCameraData.cameraData.num_model);
                            camera.SetWidth(atCameraData.cameraData.width);
                            camera.SetHeight(atCameraData.cameraData.height);
                            CameraModelType_e cameramodeltype = (CameraModelType_e)(atCameraData.cameraData.projection_model);
                            camera.SetCameraModelType(cameramodeltype);
                            camera.SetCameraOrientation(atCameraData.cameraData.num_cameraorientation);

                            std::vector<double>params;
                            
                            
                            
                            for (int p_idx = 0; p_idx < 12; p_idx++)
                            {
                                params.push_back(atCameraData.cameraData.params[p_idx]);
                            }
                            camera.SetParams(params);
                           
                            if (blkversion_ <= -1010)
                            {
                                int fix_num = atCameraData.cameraData.fix_num;
                                if (fix_num > 0)
                                {
                                    std::vector<int> params_0;
                                    params_0.resize(fix_num);
                                    for (int fix_idx = 0; fix_idx < fix_num; fix_idx++)
                                    {
                                        params_0[fix_idx] = atCameraData.cameraData.fix_param[fix_idx];
                                    }
                                    camera.SetFixed(params_0);
                                }
                            }
                            ATdata->AddCamera(camera);
                            pg.SetCamera(camera);

                            int num_images = atCameraData.num_images;
                            
                            for (int img_idx = 0; img_idx < num_images; img_idx++)
                            {
                                ImageData imageData = atCameraData.images[img_idx];
                                Image image;
                                image_t image_id = imageData.image_id;
                                image.SetCameraId(camera_id);
                                image.SetPhotoGroupID(group_id);
                                
                                
                                
                                
                                image.SetImageId(image_id);
                                image_ids_.insert(image_id);
                                image.SetRegistered(imageData.isregis);

                                if (blkversion_ <= -1010)
                                {
                                    int status = imageData.status;
                                    image.SetFixStatus(fix_e(status));
                                }
                                std::string imgpath = imageData.path;
                                std::string imgname = imageData.name;
#ifdef WIN32
                                // imgpath = UTF82GBK(imgpath);
                                // imgname = UTF82GBK(imgname);
#endif 
                                image.SetPath(imgpath);
                                image.SetName(imgname);
                                image.SetPriviewFileFullName(imageData.num_preview_name_str);
                                
                                
                                
                                Eigen::Matrix3d R;
                                for (int i = 0; i < 3; ++i) {
                                    for (int j = 0; j < 3; ++j) {
                                        R(i, j) = imageData.rotation[i][j];
                                    }
                                }
                                image.SetRotationMatrix(R);
                                Eigen::Vector3d center;
                                for (int i = 0; i < 3; ++i) {
                                    center[i] = imageData.center[i];
                                }
                                image.SetPosition(center);
                                std::string tmep = image.GetName();
                                String::StringToLower(&tmep);
                                
                                
                                bool HasPrior = imageData.hasPrior;
                                if (HasPrior)
                                {
                                    srsid_t srs_id = imageData.srs_id_prior;
                                    srs_s srs_prior = srs_map_.at(srs_id);
                                    srs_used_ids.insert(srs_id);
                                    image.SetPriorSrs(srs_prior);
                                    Eigen::Matrix3d R_prior;
                                    for (int i = 0; i < 3; ++i) {
                                        for (int j = 0; j < 3; ++j) {
                                            R_prior(i, j) = imageData.R_prior[i][j];
                                        }
                                    }
                                    image.SetRotationMatrixPrior(R_prior);
                                    Eigen::Vector3d center_prior;
                                    for (int i = 0; i < 3; ++i) {
                                        center_prior[i] = imageData.center_prior[i];
                                    }
                                    image.SetPositionPrior(center_prior);
                                }                                
                                Eigen::Vector3d colorparam;
                                for (int i = 0; i < 3; ++i) {
                                    colorparam[i] = imageData.color_param[i];
                                }
                                image.SetColorParam(colorparam);
                                Eigen::Vector3d depth;
                                for (int i = 0; i < 3; ++i) {
                                    depth[i] = imageData.depth[i];
                                }
                                image.SetDepth(depth);                           
                                
                                ExifInfo exif;
                                ExifData exifData = imageData.exifData;
                                exif.make = exifData.make;
                                exif.model = exifData.model;
                                exif.dateTime = exifData.dateTime;
                                exif.focalLength = exifData.focalLength;
                                exif.focalLengthIn35mm = exifData.focalLengthIn35mm;
                                
                                exif.longitude = exifData.longitude;
                                exif.latitude = exifData.latitude;
                                exif.altitude = exifData.altitude;
                                image.SetExifinfo(exif);
                                bool dewrapflag_ = exifData.dewrapflag;
                                image.SetDewrapFlag(dewrapflag_);
                                if (image.IsRegistered())
                                {                                   
                                    reg_image_ids.push_back(image.GetImageId());
                                }
                                image.SetUp(camera);
                                ATdata->AddImage(image);
                                pg.AddImageId(image_id);
                            }
                            photogroups_.insert(std::pair<group_t, PhotoGroup>(group_id, pg));
                        }

                        
                        EIGEN_STL_UMAP(point3D_t, class ControlPoint) cps;
                        point3D_t num_controlpoints = atItemData.num_controlpoints;                       
                        for (point3D_t gcp_idx = 0; gcp_idx < num_controlpoints; gcp_idx++)
                        {
                            GCPData gcpData = atItemData.gcpVec[gcp_idx];
                            ControlPoint cp;
                            Track track;
                            point3D_t point3d_id = gcpData.pointid;
                            cp.SetId(point3d_id);
                            Eigen::Vector3d cp_pos;
                            for (int i = 0; i < 3; ++i) {
                                cp_pos[i] = gcpData.cp_pos[i];
                            }
                            cp.SetGivenXYZ(cp_pos);
                            Eigen::Vector2d weight;
                            for (int i = 0; i < 2; ++i) {
                                weight[i] = gcpData.weight[i];
                            }
                            cp.SetWeight(weight);
                            srsid_t srs_id = gcpData.srsid;
                            srs_used_ids.insert(srs_id);
                            cp.SetSrs(srs_map_[srs_id]);
                            cp.SetName(gcpData.name);
                            gpt_e category = (gpt_e)(gcpData.category);
                            cp.SetType(category);

                            
                            int num_eles = gcpData.num_eles;
                            std::vector<TrackElement> vec_trackele;
                            for (int ele_idx = 0; ele_idx < num_eles; ele_idx++)
                            {
                                GCPItem gcpItem = gcpData.elesVec[ele_idx];
                                TrackElement trackelement;
                                image_t image_id = kInvalidImageId;
                                image_id = gcpItem.imageid;
                                Eigen::Vector2d xy;
                                for (int i = 0; i < 2; ++i) {
                                    xy[i] = gcpItem.xy[i];
                                }
                                Image& image = ATdata->GetImageMutual(image_id);
                                image.SetPoints2DGCP(point3d_id, xy);
                                trackelement.xy = xy;
                                trackelement.image_id = image_id;
                                trackelement.point2D_idx = point3d_id;
                                vec_trackele.push_back(trackelement);
                            }
                            if (!vec_trackele.empty())
                            {
                                track.AddElements(vec_trackele);
                                cp.GetObjectPointMutual().SetTrack(track);
                            }
                            cps.insert(std::make_pair(point3d_id, cp));
                        }
                        ATdata->SetControlPoints(cps);
                        
                        
                        
                        {
                            EIGEN_STL_UMAP(point3D_t, class Point3D) cps;
                            point3D_t num_points = atItemData.num_userpoints;                            
                            if (num_points > 0)
                            {
                                for (point3D_t upt_idx = 0; upt_idx < num_points; upt_idx++)
                                {
                                    Point3D cp;
                                    Track track;
                                    UserPointVecData userPointVecData = atItemData.usedPointVec[upt_idx];
                                    
                                    point3D_t id = userPointVecData.id;
                                    cp.SetId(id);
                                    cp.SetName(userPointVecData.name);
                                    
                                    int num_eles = userPointVecData.num_ele;
                                    std::vector<TrackElement> vec_trackele;
                                    for (int ele_idx = 0; ele_idx < num_eles; ele_idx++)
                                    {
                                        UsedPointData usedPointData = userPointVecData.usedPointVec[ele_idx];
                                        TrackElement trackelement;
                                        image_t image_id = kInvalidImageId;
                                        image_id = usedPointData.imageid;
                                        Eigen::Vector2d xy;
                                        for (int i = 0; i < 2; ++i) {
                                            xy[i] = usedPointData.xy[i];
                                        }

                                        Image& image = ATdata->GetImageMutual(image_id);
                                        image.SetPoints2DUserPt(id, xy);
                                        trackelement.xy = xy;
                                        trackelement.image_id = image_id;
                                        trackelement.point2D_idx = id;                                        

                                        vec_trackele.push_back(trackelement);
                                    }
                                    if (!vec_trackele.empty())
                                    {
                                        track.AddElements(vec_trackele);
                                        cp.SetTrack(track);
                                    }
                                    cps.insert(std::make_pair(upt_idx, cp));
                                    bool hasguideimage = userPointVecData.hasguideimage;
                                    if (hasguideimage)
                                    {
                                        image_t image_id_forguide = userPointVecData.image_id_forguide;
                                        cp.image_for_userptguide_ = image_id_forguide;
                                    }
                                    else
                                    {
                                        cp.image_for_userptguide_ = kInvalidImageId;
                                    }
                                    ATdata->SetUserPoint3D(cps);
                                }
                            }
                        }
                        
                        
                        std::string dst_definition = BASESRS;
                        if (ExistSRSId(blockSRS_id_))
                        {
                            
                            
                            
                            
                            
                            
                            srs_s origin_srs = srs_map_[blockSRS_id_];
                            if (origin_srs.type == coord_system_type_e::LOCAL_ENU)
                            {
                                srs_enu_discription_ = origin_srs;
                            }
                            ATdata->SetOriginSrs(origin_srs.definition);

                            
                            if (origin_srs.type == GEOGRAPHIC)
                            {
                                ATdata->TransFormImages(origin_srs.definition, dst_definition);
                                ATdata->TransFormTiepoints(origin_srs.definition, dst_definition);
                            }
                            else
                            {
                                dst_definition = origin_srs.definition;
                            }
                            ATdata->TransformControlPoints(dst_definition);
                            ATdata->SetLocalGcpSrs(dst_definition);
                            ATdata->SetLocalSrs(dst_definition);
                            UpdateSRSMap(CoordinateDescriptor::GetSRSFromDefinition(dst_definition));
                            blockSRS_id_ = ExistSRS(dst_definition);
                        }

                        ATdata->SetRegImageIds(reg_image_ids);

                        
                        
                        
                        if (blkversion_ <= -1000)
                        {

                            if (atdataid == 0 )
                            {
                                
                                    ATData_ = ATdata;
                                
                                

                            }
                            else
                            {
                                
                                {
                                    ATGroups_[atdataid - 1].SetATData(ATdata);

                                    status_ = jobsta_e::STATUS_COMPLETE;
                                }
                            }
                            
                        }
                        else
                        {
                            if (status_ == jobsta_e::STATUS_COMPLETE && block_info_.isFinished)
                            {
                                ATGroups_[atdataid].SetATData(ATdata);
                                
                                ATData_ = ATdata;
                            }
                            else
                                ATData_ = ATdata;
                        }
                        ATdataCurrent = ATdata;
                    }
                }
        
                in.close();
                
            }
            catch (const std::exception& err)
            {
                LOGE(err.what());
                in.close();
                return false;
            }
#ifdef USE_AI3D_PROJ
            for (auto& srs : srs_map_)
            {
                if (srs_used_ids.count(srs.first))
                {


                    AI3D::PROJ::CoordinateReferenceSystem::AddCrs(srs.second.definition);
                }
            }
#endif

            return true;
        }


        

        bool BlockObject::LoadTiepointsBinary(const std::string& file_path, const std::shared_ptr<ATData>& ATdata)
        {
            if (!File::IsFileExistent(file_path))
            {
                std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " no tiepoints file:" << file_path << std::endl;
                return false;
            }

            std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " has tiepoints file." << std::endl;

            
            std::set<image_t> images_pg = image_ids_;
            if (images_pg.empty())
            {
                return false;
            }
            
            std::ifstream in = File::OpenIfstreamUtf8(file_path, std::ios::binary);
            
            if (!in.is_open())
            {
                LOG(ERROR) << "Error opening file! " << file_path;
                return false;
            }
            
            
            
            
            
            
            try
            {
                TiePointsFile tiePointsFile;
                tiePointsFile.Deserialize(in);

                
                point3D_t num_tiepoints = tiePointsFile.num_tiepoints;
                
                for (point3D_t tp_idx = 0; tp_idx < num_tiepoints; tp_idx++)
                {
                    PointItemData pointItemData = tiePointsFile.pointVec[tp_idx];
                    Point3D point3d;
                    point3D_t index_point3d = pointItemData.index_point3d;

                    
                    Track track;
                    Eigen::Vector3d xyz;
                    Eigen::Vector3i rgb;
                    
                    
                    
                    
                    for (int i = 0; i < pointItemData.xyz.size(); i++) {
                        xyz(i) = pointItemData.xyz[i];
                    }
                    for (int i = 0; i < pointItemData.rgb.size(); i++) {
                        rgb(i) = pointItemData.rgb[i];
                    }
                    
                    
                    int num_elements = pointItemData.num_elements;
                    

                    std::vector<TrackElement> vec_trackele;
                    for (int i_ele = 0; i_ele < num_elements; i_ele++)
                    {
                    
                        TrackElement trackelement;
                        TrackItemData trackItemData = pointItemData.vec_trackele[i_ele];
                        image_t image_id = trackItemData.image_id;
                        Eigen::Vector2d uv;
                        
                        if (!ATdata->GetImages().count(image_id))
                        {
                            continue;
                        }
                        
                        for (int j = 0; j < 2; j++) {
                            uv(j) = trackItemData.uv[j];
                        }
                        Image& img = ATdata->GetImageMutual(image_id);
                        trackelement.image_id = image_id;
                        trackelement.point2D_idx = img.AddPoints2D(uv);
                        img.SetPoint3DForPoint2D(trackelement.point2D_idx, index_point3d);
                        trackelement.xy = uv;
                        vec_trackele.push_back(trackelement);
                    }
                    track.AddElements(vec_trackele);
                    point3d.SetId(index_point3d);
                    point3d.SetTrack(track);
                    point3d.SetColor(rgb);
                    point3d.SetXYZ(xyz);

                    btiepoint_loaded_ = true;
                    
                    
                    
                    

                    

                    ATdata->GetPoints3DMutual().insert(std::make_pair(index_point3d, point3d));
                }

                
                in.close();
            }
            catch (const std::exception& err)
            {
                LOGE(err.what());
                in.close();
                
                return false;
            }
            return true;
        }

        bool BlockObject::ExportBlockATData()
        {
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }
            
            
            std::string block_path = block_info_.Block_XML;
            std::string tiepoints_path = block_info_.Tiepoints;
            Timer time;
            time.Start();
            std::string block_path_bak = block_path + BAK;
            std::string tiepoints_path_bak = tiepoints_path + BAK;

            try
            {
                if (File::IsFileExistent(block_path))
                {
                    std::filesystem::copy_file(File::BoostPathFromUtf8(block_path), File::BoostPathFromUtf8(block_path_bak), std::filesystem::copy_options::overwrite_existing);
                }
                if (File::IsFileExistent(tiepoints_path))
                {
                    std::filesystem::copy_file(File::BoostPathFromUtf8(tiepoints_path), File::BoostPathFromUtf8(tiepoints_path_bak), std::filesystem::copy_options::overwrite_existing);
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }
            
            
            if (!ExportATBinaryWithoutTiepoints(block_path))
            {
                return false;
            }
            
            
            if (GetCurrentATMutual()->UpdateTiepoints())
            {
                LOGW(String::StringPrintf("%s already has been updated!", block_info_.Tiepoints.c_str()));
                GetCurrentATMutual()->SetPoint3DsStatus(true);
            }
            
            
            
            
            
            bool shouldsavetiepoints = ((GetCurrentAT()->GetPoint3DsStatus()) || (GetCurrentAT()->HasTiepoints() && (!File::ExistsFile(tiepoints_path))));

            if (shouldsavetiepoints)
            {
                
                
                LOGI(std::string("save tiepoint file  " + tiepoints_path));
                if (!ExportTiepointsBinary(tiepoints_path))
                {
                    
                    LOGI(std::string("save tiepoint file  " + tiepoints_path + " failed"));
                    
                    return false;
                }
            }

            try
            {
                if (File::IsFileExistent(block_path_bak))
                {
                    std::filesystem::remove(File::BoostPathFromUtf8(block_path_bak));
                }
                if (File::IsFileExistent(tiepoints_path_bak))
                {
                    std::filesystem::remove(File::BoostPathFromUtf8(tiepoints_path_bak));
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            LOGI(String::StringPrintf("Saving block_%d(%s) spends %f s", id_, block_info_.blockString.c_str(), time.ElapsedSeconds()));
            return true;
        }

        bool BlockObject::ExportATBinaryWithoutTiepoints(const std::string& file_path)
        {
            
            
            std::ofstream out = File::OpenOfstreamUtf8(file_path, std::ios::binary);
            if (!out.is_open()) {
                LOGE(String::StringPrintf("Writing %s failed!", file_path.c_str()));
                return false;
            }
            ATBlockBinFile atBlockBinFile;
            
            
            try
            {
                if (ATData_->HasUserTiepoints() || (ATGroups_.count(0) && ATGroups_.at(0).GetATData()->HasUserTiepoints()))
                {
                    blkversion_ = -1001;
                }
                atBlockBinFile.version = blkversion_;
                

                int atdatasize = ATGroups_.size() + 1;
                atBlockBinFile.atdatasize = atdatasize;
                atBlockBinFile.atList.clear();
                for (int data_idx = 0; data_idx < atdatasize; data_idx++)
                {
                    ATItemData atItemData;
                    ATData Atdata = (data_idx == 0 ? *ATData_ : *ATGroups_[data_idx - 1].GetATDataMutual());
                    atItemData.data_idx = data_idx;
                    int num_srs = srs_map_.size();
                    atItemData.num_srs = num_srs;
                    atItemData.srsVec.clear();
                    for ( auto& srs : srs_map_)
                    {
                        SRSItemData srsItemData;
                        srsItemData.id = srs.first;
                        srsItemData.name = srs.second.name;
                        srsItemData.type = srs.second.type;
                        srsItemData.definition = srs.second.definition;
                        int definition_length = srs.second.definition.length();
                        atItemData.srsVec.push_back(srsItemData);
                    }

                    
                    std::string name = name_;
                    std::string description = description_;
#ifdef WIN32
                    // name = GBK2UTF8(name);
                    // description = GBK2UTF8(description);
#endif 
                    atItemData.block_name = name;
                    atItemData.block_description = description;
                    atItemData.blockSRS_id = blockSRS_id_;                   
                    atItemData.local_srs_definition = Atdata.GetLocalSrs();

                    
                    atItemData.num_photogroups = photogroups_.size();
                    atItemData.photoGroups.clear();
                    for (const auto& pg : photogroups_)
                    {
                        ATCameraData atCameraData;
                        CameraData cameraData;
                        cameraData.hasExtraParam = true;
                        cameraData.group_id = pg.first;
                        cameraData.group_name = pg.second.GetName();

                        Camera camera = Atdata.GetCamera(pg.second.GetCamera().GetCameraId());
                        cameraData.id = camera.GetCameraId();
                        cameraData.cameraModelid = camera.GetModelId();
                        cameraData.camera_name = camera.GetCameraName();
                        cameraData.prior_focal = camera.GetFocalLengthMM();
                        cameraData.f_35eq = camera.GetFocalLengthIn35mm();
                        cameraData.sensorsize = camera.GetSensorSize();
                        cameraData.num_make = camera.GetMake();
                        cameraData.num_model = camera.GetMakeModel();
                        cameraData.width = camera.GetWidth();
                        cameraData.height = camera.GetHeight();
                        CameraModelType_e cameramodeltype = camera.GetCameraModelType();
                        cameraData.projection_model = (int)cameramodeltype;
                        cameraData.num_cameraorientation = camera.GetCameraOrientation();

                        
                        
                        
                        std::vector<double>params_0 = camera.GetParams();
                        for (int p_idx = 0; p_idx < 12; p_idx++)
                        {
                            cameraData.params[p_idx] = params_0[p_idx];
                        }

                        

                        if (blkversion_ <= -1010)
                        {
                            int fix_num = camera.GetFixed().size();
                            cameraData.fix_num = fix_num;
                            std::vector<int> params_0 = camera.GetFixed();                            
                            for (int fix_idx = 0; fix_idx < fix_num; fix_idx++)
                            {
                                cameraData.fix_param[fix_idx] = params_0[fix_idx];
                            }                           
                        }
                        atCameraData.cameraData = cameraData;
                        
                        atCameraData.num_images = pg.second.GetGroupImageIds().size();
                        atCameraData.images.clear();
                        for (const auto& img_id : pg.second.GetGroupImageIds())
                        {
                            if (!Atdata.GetImages().count(img_id))
                            {                               
                                continue;
                            }
                            ImageData imageData;
                            imageData.hasExtraParam = true;
                            Image image = Atdata.GetImage(img_id);
                            imageData.image_id = image.GetImageId();
                            imageData.isregis = image.IsRegistered();
                            if (blkversion_ <= -1010)
                            {
                                imageData.status = (int)image.GetFixStatus();
                            }
                            std::string imgpath = image.GetPath();
                            std::string imgname = image.GetName();
#ifdef WIN32
                            // imgpath = GBK2UTF8(imgpath);
                            // imgname = GBK2UTF8(imgname);
#endif 
                            imageData.path = imgpath;
                            imageData.name = imgname;
                            imageData.num_preview_name_str = image.GetPriviewFileFullName();
                            
                            imageData.hasRotaiton = true;
                            Eigen::Matrix3d R = image.GetRotationMatrix();
                            for (int i = 0; i < 3; ++i) {
                                for (int j = 0; j < 3; ++j) {
                                    imageData.rotation[i][j] = R(i, j);
                                }
                            }
                            imageData.hasCenter = true;
                            Eigen::Vector3d center = image.GetPosition();
                            for (int i = 0; i < 3; ++i) {
                                imageData.center[i] = center[i];
                            }
                            
                            
                            
                            bool HasPrior = image.GetPriorSrs().ID != kInvalidSrsId;   
                            imageData.hasPrior = HasPrior;
                            if (HasPrior)
                            {
                                imageData.srs_id_prior = image.GetPriorSrs().ID;
                                Eigen::Matrix3d R_prior = image.GetRotationMatrixPrior();
                                for (int i = 0; i < 3; ++i) {
                                    for (int j = 0; j < 3; ++j) {
                                        imageData.R_prior[i][j] = R_prior(i, j);
                                    }
                                }
                                Eigen::Vector3d center_prior = image.GetPositionPrior();
                                for (int i = 0; i < 3; ++i) {
                                    imageData.center_prior[i] = center_prior[i];
                                }
                                
                                
                                
                                
                                
                            } 
                            imageData.hasColorParam = true;
                            Eigen::Vector3d colorparam = image.GetColorParam();
                            for (int i = 0; i < 3; ++i) {
                                imageData.color_param[i] = colorparam[i];
                            }
                            Eigen::Vector3d depth = image.GetDepth();
                            for (int i = 0; i < 3; ++i) {
                                imageData.depth[i] = depth[i];
                            }

                            
                            ExifData exifData;
                            ExifInfo exif = image.GetExifinfo();
                            
                            exifData.make = exif.imagePath;
                            exifData.model = exif.model;
                            exifData.dateTime = exif.dateTime;
                            exifData.focalLength = exif.focalLength;
                            exifData.focalLengthIn35mm = exif.focalLengthIn35mm;
                            
                            exifData.longitude = exif.longitude;
                            exifData.latitude = exif.latitude;
                            exifData.altitude = exif.altitude;
                            exifData.dewrapflag = image.GetDewrapFlag();
                            imageData.exifData = exifData;
                            atCameraData.images.push_back(imageData);
                        }
                        atItemData.photoGroups.push_back(atCameraData);
                    }

                    
                    atItemData.num_controlpoints = Atdata.GetControlPoints().size();
                    atItemData.gcpVec.clear();
                    for (const auto& cp_pair : Atdata.GetControlPoints())
                    {
                        GCPData gcpData;
                        ControlPoint cp = cp_pair.second;
                        
                        gcpData.pointid = cp_pair.first;
                        Eigen::Vector3d cp_pos = cp.GetGivenXYZ();
                        for (int i = 0; i < 3; ++i) {
                            gcpData.cp_pos[i] = cp_pos[i];
                        }
                        Eigen::Vector2d weight = cp.GetWeight();
                        for (int i = 0; i < 2; ++i) {
                            gcpData.weight[i] = weight[i];
                        }
                        gcpData.hasExtraParam = true;
                        gcpData.srsid = cp.GetSrs().ID;
                        gcpData.name = cp.GetName();
                        gpt_e category = cp.GetType();
                        gcpData.category = (int)category;
                        
                        std::vector<TrackElement> elements = cp.GetObjectPoint().GetTrack().GetElements();
                        gcpData.num_eles = elements.size();
                        gcpData.elesVec.clear();
                        for (const auto& ele : elements)
                        {
                            GCPItem gcpItem;
                            gcpItem.imageid = ele.image_id;
                            Eigen::Vector2d xy = ele.xy;
                            for (int i = 0; i < 2; ++i) {
                                gcpItem.xy[i] = xy[i];
                            }
                            gcpData.elesVec.push_back(gcpItem);
                        }
                        atItemData.gcpVec.push_back(gcpData);
                    }

                    
                
                    {
                        point3D_t points_num = Atdata.GetNumUserPoints();
                        atItemData.num_userpoints = points_num;
                        atItemData.usedPointVec.clear();
                        {
                            
                            if (points_num > 0)
                            {
                                for (const auto& cp : Atdata.GetUserPoints3D())
                                {
                                    UserPointVecData userPointVecData;
                                    userPointVecData.id = cp.second.GetId();
                                    userPointVecData.name = cp.second.GetName();
                                    
                                    std::vector<TrackElement> elements = cp.second.GetTrack().GetElements();
                                    userPointVecData.num_ele = elements.size();
                                    userPointVecData.usedPointVec.clear();
                                    for (const auto& ele : elements)
                                    {
                                        UsedPointData usedPointData;
                                        usedPointData.imageid = ele.image_id;
                                        Eigen::Vector2d xy = ele.xy;
                                        for (int i = 0; i < 2; ++i) {
                                            usedPointData.xy[i] = xy[i];
                                        }
                                        userPointVecData.usedPointVec.push_back(usedPointData);
                                    }
                                    bool hasguideimage = (cp.second.image_for_userptguide_ != kInvalidImageId);
                                    userPointVecData.hasguideimage = hasguideimage;
                                    if (hasguideimage)
                                    {
                                        userPointVecData.image_id_forguide = cp.second.image_for_userptguide_;
                                    }
                                    atItemData.usedPointVec.push_back(userPointVecData);
                                }
                                
                            }
                        }
                    }
                    atBlockBinFile.atList.push_back(atItemData);
                }
                atBlockBinFile.Serialize(out);
                out.close();
            }
            catch (const std::exception& err)
            {
                LOGE(String::StringPrintf("Saving export images bin: %s failed! Msg: %s", file_path.c_str(), err.what()));
                out.close();
                return false;
            }
            return true;
        }
        bool BlockObject::ExportTiepointsBinary(const std::string& file_path)
        {
            
            
            
            
            
            
            std::ofstream out = File::OpenOfstreamUtf8(file_path, std::ios::binary);
            
            if (!out.is_open()) {
                LOGE(String::StringPrintf("Writing file %s error!", file_path.c_str()));
                return false;
            }
            TiePointsFile tiePointsFile;          

            try
            {
                
                
                point3D_t num_tiepoints = GetCurrentAT()->GetPoints3D().size();
                LOGI(__FUNCTION__ + file_path +" " + std::to_string(num_tiepoints));
            
                
                tiePointsFile.num_tiepoints = num_tiepoints;
                tiePointsFile.pointVec.clear();
                for (const auto& tp : GetCurrentAT()->GetPoints3D())
                {
                    PointItemData pointItemData;
                    
                    pointItemData.index_point3d = tp.first;

                    Eigen::Vector3d xyz = tp.second.GetXYZ();
                    
                    
                    for (int i = 0; i < xyz.size(); i++) {
                        pointItemData.xyz[i] = xyz(i);
                    }

                    Eigen::Vector3i color = tp.second.GetColor();
                    
                    for (int i = 0; i < color.size(); i++) {
                        pointItemData.rgb[i] = color(i);
                    }

                    
                    int num_elements = tp.second.GetTrack().GetElements().size();
                    
                    pointItemData.num_elements = num_elements;
                    pointItemData.vec_trackele.clear();
                    for (const auto& ele : tp.second.GetTrack().GetElements())
                    {
                        TrackItemData trackItemData;
                        image_t photo_id = ele.image_id;
                        
                        trackItemData.image_id = photo_id;
                        Eigen::Vector2d xy = ele.xy;
                        
                        for (int j = 0; j < 2; j++) {
                            trackItemData.uv[j] = xy(j);
                        }
                        pointItemData.vec_trackele.push_back(trackItemData);
                    }
                    tiePointsFile.pointVec.push_back(pointItemData);

                }
                
                tiePointsFile.Serialize(out);
                out.close();
            }
            catch (const std::exception& err)
            {
                LOGE(String::StringPrintf("Saving export images bin: %s failed! Msg: %s", file_path.c_str(), err.what()));
                
                out.close();
                return false;
            }
            return true;
        }



        bool BlockObject::LoadATBinary(const std::string& AT_filepath, std::shared_ptr<ATData>ATdata)
        {
            std::ifstream in = File::OpenIfstreamUtf8(AT_filepath, std::ios::binary);
            
            if (!in.is_open())
            {
                LOGE(String::StringPrintf("Reading %s failed!", AT_filepath.c_str()));
                return false;
            }

            ATBinFile atBinFile;
            atBinFile.Deserialize(in);

            
            {
                
                int version = atBinFile.version;
                std::string definition = atBinFile.definition;

                ATdata->SetLocalSrs(definition);
           

                
                
                std::vector<PhotoGroup>pgs;
                int num_photogroups = atBinFile.num_photogroups;
                if (num_photogroups > std::numeric_limits<uint8_t>::max())
                {
                    LOGW(String::StringPrintf("Invalid SCSFR.bin(%s),too many photogroups!", AT_filepath.c_str()));
                    return false;
                }
                for (int pg_idx = 0; pg_idx < num_photogroups; pg_idx++)
                {
                    ATCameraData atCameraData = atBinFile.photoGroups[pg_idx];
                    PhotoGroup pg;
                    Camera camera;
                    pg.SetName(atCameraData.cameraData.camera_name);
                    camera.SetCameraId(atCameraData.cameraData.id);
                    pg.SetId(atCameraData.cameraData.id);
                    camera.SetModelId(atCameraData.cameraData.cameraModelid);
                    camera.SetWidth(atCameraData.cameraData.width);
                    camera.SetHeight(atCameraData.cameraData.height);
                    CameraModelType_e cameramodeltype = (cameramodeltype_e)(atCameraData.cameraData.projection_model);
                    camera.SetCameraModelType(cameramodeltype);

                    std::vector<double>params;
                    
                    
                    
                    
                    
                    for (int i_param = 0; i_param < 12; i_param++)
                    {
                        params.push_back(atCameraData.cameraData.params[i_param]);
                    }
                    camera.SetParams(params);

                    if (version <= -1010)
                    {
                        int fix_num = atCameraData.cameraData.fix_num;
                        if (fix_num > 0)
                        {
                            std::vector<int> params_0 = atCameraData.cameraData.fix_param;
                            camera.SetFixed(params_0);
                        }
                    }
                    ATdata->AddCamera(camera);
                    pg.SetCamera(camera);

                    
                    int num_images = atCameraData.num_images;
                    for (int num_idx = 0; num_idx < num_images; num_idx++)
                    {
                        ImageData imageData = atCameraData.images[num_idx];
                        Image image;
                        image_t image_id = imageData.image_id;
                        image.SetCameraId(atCameraData.cameraData.id);
                        image.SetPhotoGroupID(atCameraData.cameraData.id);
                        image.SetImageId(image_id);
                        image_ids_.insert(image_id);
                        bool isRegistered = imageData.isregis;
                        image.SetRegistered(imageData.isregis);

                        if (isRegistered)
                        {
                            ATdata->GetRegImageIdsMutual().emplace_back(image_id);
                        }
                        if (version <= -1010)
                        {
                            image.SetFixStatus(fix_e(imageData.status));
                        }

                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        std::string path = imageData.path;
                        std::string name = imageData.name;
#ifdef WIN32
                        // path = UTF82GBK(path);
                        // name = UTF82GBK(name);
#endif 
                        image.SetPath(path);
                        image.SetName(name);

                        Eigen::Matrix3d R;
                        for (int i = 0; i < 3; ++i) {
                            for (int j = 0; j < 3; ++j) {
                                R(i, j) = imageData.rotation[i][j];
                            }
                        }
                        image.SetRotationMatrix(R);

                        Eigen::Vector3d center;
                        for (int i = 0; i < 3; ++i) {
                            center[i] = imageData.center[i];
                        }
                        image.SetPosition(center);

                        Eigen::Vector3d colorparam;
                        for (int i = 0; i < 3; ++i) {
                            colorparam[i] = imageData.color_param[i];
                        }
                        image.SetColorParam(colorparam);
                        
                        image.SetUp(camera);
                        ATdata->AddImage(image);
                        pg.AddImageId(image_id);
                        
                    }
                    
                    
                        pgs.push_back(pg);
                }

                
                point3D_t num_tiepoints = atBinFile.num_tiepoints;;
                for (point3D_t pt_idx = 0; pt_idx < num_tiepoints; pt_idx++)
                {
                    PointItemData pointItemData = atBinFile.pointVec[pt_idx];
                    Point3D point3d;
                    point3D_t index_point3d = pointItemData.index_point3d;

                    
                    Track track;
                    Eigen::Vector3d xyz;
                    for (int i = 0; i < 3; ++i) {
                        xyz[i] = pointItemData.xyz[i];
                    }
                    Eigen::Vector3i rgb;
                    for (int i = 0; i < 3; ++i) {
                        rgb[i] = pointItemData.rgb[i];
                    }

                    
                    int num_elements = pointItemData.num_elements;
                    std::vector<TrackElement> vec_trackele;
                    for (int i_ele = 0; i_ele < num_elements; i_ele++)
                    {
                        TrackItemData trackItemData = pointItemData.vec_trackele[i_ele];
                        TrackElement trackelement;
                        image_t image_id = trackItemData.image_id;
                        Eigen::Vector2d uv;
                        for (int i = 0; i < 2; ++i) {
                            uv[i] = trackItemData.uv[i];
                        }
                        if (!image_ids_.count(image_id))
                        {
                            LOGE("Invalid image id or image ids!");
                            continue;
                        }
                        
                        
                        
                        
                        
                        
                        
                        Image& img = ATdata->GetImageMutual(image_id);
                        trackelement.image_id = image_id;
                        trackelement.point2D_idx = img.AddPoints2D(uv);
                        img.SetPoint3DForPoint2D(trackelement.point2D_idx, index_point3d);
                        trackelement.xy = uv;
                        vec_trackele.push_back(trackelement);
                    }
                    track.AddElements(vec_trackele);
                    point3d.SetId(index_point3d);
                    point3d.SetTrack(track);
                    point3d.SetColor(rgb);
                    point3d.SetXYZ(xyz);

                    ATdata->GetPoints3DMutual().insert(std::make_pair(index_point3d, point3d));

                }
                
                {
                    uint64_t num_controlpoints = atBinFile.num_controlpoints;
                    if (num_controlpoints > 0)
                    {
                        std::string gcpdefstr = atBinFile.gcpDefine;
                        
                        srs_s gcpsrs;
                        
                        gcpsrs = CoordinateDescriptor::GetSRSFromDefinition(gcpdefstr);
                        this->UpdateSRSMap(gcpsrs);
                        gcpsrs.ID = this->ExistSRS(gcpdefstr);                       
                        EIGEN_STL_UMAP(point3D_t, class ControlPoint) cps;

                        for (point3D_t gcp_idx = 0; gcp_idx < num_controlpoints; gcp_idx++)
                        {
                            GCPData gcpData = atBinFile.gcpVec[gcp_idx];
                            ControlPoint cp;
                            Track track;
                            point3D_t point3d_id = gcpData.pointid;
                            cp.SetId(point3d_id);
                            cp.SetSrs(gcpsrs);
                            Eigen::Vector3d cp_pos;
                            for (int i = 0; i < 3; ++i) {
                                cp_pos[i] = gcpData.cp_pos[i];
                            }
                            cp.SetGivenXYZ(cp_pos);
                            Eigen::Vector2d weight;
                            for (int i = 0; i < 2; ++i) {
                                weight[i] = gcpData.weight[i];
                            }
                            cp.SetWeight(weight);
                            
                            
                            cp.SetSrs(gcpsrs);
                            cp.SetName(gcpData.name);
                            gpt_e category = (gpt_e)(gcpData.category);
                            cp.SetType(category);
                            std::cout << category << " " << cp.GetName() << std::endl;
                            
                            int num_eles = gcpData.num_eles;
                            std::vector<TrackElement> vec_trackele;
                            for (int ele_idx = 0; ele_idx < num_eles; ele_idx++)
                            {
                                GCPItem gcpItem = gcpData.elesVec[ele_idx];
                                TrackElement trackelement;
                                image_t image_id = kInvalidImageId;
                                image_id = gcpItem.imageid;
                                Eigen::Vector2d xy;
                                for (int i = 0; i < 2; ++i) {
                                    xy[i] = gcpItem.xy[i];
                                }
                                if (!ATdata->GetImagesMutual().count(image_id))
                                {
                                    LOGI("image id " + image_id);
                                    continue;
                                }
                                Image& image = ATdata->GetImageMutual(image_id);
                                image.SetPoints2DGCP(point3d_id, xy);
                                trackelement.xy = xy;
                                trackelement.image_id = image_id;
                                trackelement.point2D_idx = point3d_id;
                                vec_trackele.push_back(trackelement);
                            }
                            if (!vec_trackele.empty())
                            {
                                track.AddElements(vec_trackele);
                                cp.GetObjectPointMutual().SetTrack(track);
                            }
                            cps.insert(std::make_pair(point3d_id, cp));
                        }
                        ATdata->SetControlPoints(cps);
                    }
                    

                    EIGEN_STL_UMAP(point3D_t, class Point3D) userpts;
                    point3D_t num_points = atBinFile.num_userpoints;
                    for (point3D_t pt_idx = 0; pt_idx < num_points; pt_idx++)
                    {
                        Point3D cp;
                        Track track;
                        cp.SetId(point3D_t(pt_idx));
                        
                        int num_eles = atBinFile.usedPointVec[pt_idx].size();;
                        std::vector<TrackElement> vec_trackele;
                        for (int ele_idx = 0; ele_idx < num_eles; ele_idx++)
                        {
                            UsedPointData usedPointData = atBinFile.usedPointVec[pt_idx][ele_idx];
                            TrackElement trackelement;
                            image_t image_id = usedPointData.imageid;
                            Eigen::Vector2d xy;
                            for (int i = 0; i < 2; ++i) {
                                trackelement.xy[i] = usedPointData.xy[i];
                            }
                            trackelement.image_id = image_id;
                            vec_trackele.push_back(trackelement);
                        }
                        if (!vec_trackele.empty())
                        {
                            track.AddElements(vec_trackele);
                            cp.SetTrack(track);
                        }
                        userpts.insert(std::make_pair(pt_idx, cp));
                    }
                    ATdata->SetUserPoint3D(userpts);
                }
                in.close();
                
                for (auto& it : pgs)
                {
                    if (photogroups_.empty())
                    {
                        continue;
                    }
                    bool IsContain = false;
                    for (auto& it_pg : photogroups_)
                    {
                        
                        if (it_pg.second.PhotoGroupContain(it))
                        {
                            
                            Camera cam = ATdata->GetCamera(it.GetId());
                            cam.SetCameraId(it_pg.first);
                            ATdata->GetCamerasMutual().erase(it.GetId());
                            ATdata->GetCamerasMutual().insert(std::make_pair(it_pg.first, cam));
                            it.SetId(it_pg.first);
                            it.GetCameraMutual().SetCameraId(it_pg.first);
                            
                            for (const auto& img_id : it.GetGroupImageIds())
                            {
                                ATdata->GetImageMutual(img_id).SetCameraId(it_pg.first);
                                ATdata->GetImageMutual(img_id).SetPhotoGroupID(it_pg.first);
                            }
                            it.SetName(it_pg.second.GetName());
                            
                            IsContain = true;
                            break;
                        }
                    }
                    if (!IsContain)
                    {
                        if (it.GetNumImages() != 0)
                        {
                            LOGE("bad group.");
                        }
                    }
                }
            }
          
          
          
          
          
          
            return true;
        }
        bool BlockObject::ExportATBinary(const std::string& AT_filepath)
        {
            
            std::ofstream out = File::OpenOfstreamUtf8(AT_filepath, std::ios::binary);
            if (!out.is_open()) {
                LOGE(String::StringPrintf("Writing %s failed!", AT_filepath.c_str()));
                return false;
            }
            ATBinFile atBinFile;
            try
            {
                ATData* ATdata_tmp = new AI3D::CORE::ATData();
                if (GetCurrentAT() == nullptr)
                {
                    return false;
                }
                *ATdata_tmp = *GetCurrentAT().get();

                std::string local_srs_def = ATdata_tmp->GetLocalSrs();
                
                if (ATdata_tmp->GetLocalSrs().empty())
                {
                    local_srs_def = LOCALSRS;
                }
                
                
                int version = 0;
                atBinFile.version = version;

                
                atBinFile.definition = local_srs_def;

                std::string msg = "-----save=----- " + local_srs_def + " file " + AT_filepath + __FILE__ + " " + __FUNCTION__;
                msg += __LINE__;
                LOGI(msg);

                
                

                
                
                int num_photogroups = photogroups_.size();
                atBinFile.num_photogroups = num_photogroups;
                atBinFile.photoGroups.clear();
                for (const auto& pg : photogroups_)
                {
                    ATCameraData atCameraData;
                    CameraData cameraData;
                    std::string image_name = pg.second.GetName();
                    int photogroupname_length = image_name.size();
                    cameraData.camera_name = image_name;
                    Camera camera = ATdata_tmp->GetCamera(pg.second.GetCamera().GetCameraId());
                    cameraData.id = camera.GetCameraId();
                    cameraData.cameraModelid = camera.GetModelId();
                    cameraData.width = camera.GetWidth();
                    cameraData.height = camera.GetHeight();
                    CameraModelType_e cameramodeltype = camera.GetCameraModelType();
                    cameraData.projection_model = (int)cameramodeltype;
                    
                    
                    
                    std::vector<double>params_0 = camera.GetParams();
                    
                    for (int p_idx = 0; p_idx < 12; p_idx++)
                    {
                        cameraData.params[p_idx] = params_0[p_idx];
                    }
                    if (version <= -1010)
                    {
                        int fix_num = camera.GetFixed().size();
                        cameraData.fix_num = fix_num;
                        std::vector<int> params_0 = camera.GetFixed();
                        for (int fix_idx = 0; fix_idx < fix_num; fix_idx++)
                        {
                            cameraData.fix_param[fix_idx] = params_0[fix_idx];
                        }
                    }
                    atCameraData.cameraData = cameraData;
                    
                    int num_images = pg.second.GetGroupImageIds().size();
                    atCameraData.num_images = num_images;
                    atCameraData.images.clear();
                    for (const auto& img_id : pg.second.GetGroupImageIds())
                    {
                        ImageData imageData;
                        Image image = ATdata_tmp->GetImage(img_id);
                        image_t image_id = image.GetImageId();
                        imageData.image_id = image_id;
                        imageData.isregis = image.IsRegistered();
                        if (version <= -1010)
                        {
                            imageData.status = (int)image.GetFixStatus();
                        }
                        std::string path = image.GetPath();
                        std::string name = image.GetName();
#ifdef WIN32
                        // path = GBK2UTF8(path);
                        // name = GBK2UTF8(name);
#endif 
                        imageData.path = path;
                        imageData.name = name;
                        Eigen::Matrix3d R = image.GetRotationMatrix();
                        imageData.hasRotaiton = true;
                        for (int i = 0; i < 3; ++i) {
                            for (int j = 0; j < 3; ++j) {
                                imageData.rotation[i][j] = R(i, j);
                            }
                        }
                        Eigen::Vector3d center = image.GetPosition();
                        imageData.hasCenter = true;
                        for (int j = 0; j < 3; ++j) {
                            imageData.center[j] = center[j];
                        }
                        Eigen::Vector3d colorparam = image.GetColorParam();
                        imageData.hasColorParam = true;
                        for (int j = 0; j < 3; ++j) {
                            imageData.color_param[j] = colorparam[j];
                        }
                        std::string tmep = image.GetName();
                        String::StringToLower(&tmep);
                        atCameraData.images.push_back(imageData);
                    }
                    atBinFile.photoGroups.push_back(atCameraData);
                }

                
                
                point3D_t num_tiepoints = ATdata_tmp->GetPoints3D().size();
                atBinFile.num_tiepoints = num_tiepoints;
                atBinFile.pointVec.clear();
                for (const auto& tp : ATdata_tmp->GetPoints3D())
                {
                    PointItemData pointItemData;
                    
                    pointItemData.index_point3d = tp.first;
                    Eigen::Vector3d xyz = tp.second.GetXYZ();
                    for (int i = 0; i < 3; i++) {
                        pointItemData.xyz[i] = xyz[i];
                    }

                    Eigen::Vector3i rgb = tp.second.GetColor();
                    for (int i = 0; i < 3; i++) {
                        pointItemData.rgb[i] = rgb[i];
                    }

                    
                    int num_elements = tp.second.GetTrack().GetElements().size();
                    pointItemData.num_elements = num_elements;
                    pointItemData.vec_trackele.clear();
                    for (const auto& ele : tp.second.GetTrack().GetElements())
                    {
                        TrackItemData trackItemData;
                        image_t photo_id = ele.image_id;
                        trackItemData.image_id = photo_id;
                        Eigen::Vector2d xy = ele.xy;
                        for (int i = 0; i < 2; i++) {
                            trackItemData.uv[i] = xy[i];
                        }
                        pointItemData.vec_trackele.push_back(trackItemData);
                    }
                    atBinFile.pointVec.push_back(pointItemData);
                }
            
                {
                    uint64_t num_controlpoints = ATdata_tmp->GetNumControlPoints();
                    atBinFile.num_controlpoints = num_controlpoints;
                    if (num_controlpoints > 0)
                    {
                        
                        ControlPoints gcps;
                        for (auto& it : ATdata_tmp->GetControlPoints())
                        {
                            gcps.ADDPoint(it.second);
                        }
                        
                        point3D_t basecoorgcpid = -1;
                        for (auto& cp : gcps.GetPointsMutual())
                        {
                            if (CoordinateDescriptor::IsGeode(cp.second.GetSrs().type))
                            {
                                basecoorgcpid = cp.second.GetId();
                            }
                        }
                        srs_s srs = ATdata_tmp->GetControlPoints().at(basecoorgcpid).GetSrs();
                        std::string definition = srs.definition;                          
                        {
                            gcps.TransformPointsToBaseCoordinate(definition);
                        }
                        atBinFile.gcpDefine = definition;
                        for (auto& cp_pair : gcps.GetPointsMutual())
                        {
                            GCPData gcpData;
                            ControlPoint cp = cp_pair.second;
                            
                            gcpData.pointid = cp_pair.first;
                            Eigen::Vector3d cp_pos = cp.GetGivenXYZ();
                            for (int i = 0; i < 3; i++)
                            {
                                gcpData.cp_pos[i] = cp_pos[i];
                            }
                            Eigen::Vector2d weight = cp.GetWeight();
                            for (int i = 0; i < 2; i++)
                            {
                                gcpData.weight[i] = weight[i];
                            }
                            gcpData.name = cp.GetName();
                            gpt_e category = cp.GetType();
                            int icategory = int(category);
                            gcpData.category = icategory;
                            std::cout << cp.GetName()<<" " << category << std::endl;
                            
                            std::vector<TrackElement> elements = cp.GetObjectPoint().GetTrack().GetElements();
                            int num_ele = elements.size();
                            gcpData.num_eles = num_ele;
                            gcpData.elesVec.clear();
                            for (const auto& ele : elements)
                            {
                                GCPItem gcpItem;
                                gcpItem.imageid = ele.image_id;
                                Eigen::Vector2d xy = ele.xy;
                                for (int i = 0; i < 2; i++)
                                {
                                    gcpItem.xy[i] = xy[i];
                                }
                                gcpData.elesVec.push_back(gcpItem);
                            }
                            atBinFile.gcpVec.push_back(gcpData);
                        }
                    }
                        

                    {
                        
                        {
                            point3D_t points_num = ATdata_tmp->GetNumUserPoints();
                            atBinFile.num_userpoints = points_num;
                            atBinFile.usedPointVec.clear();
                            if (points_num > 0)
                            {
                                for (const auto& cp : ATdata_tmp->GetUserPoints3D())
                                {
                                    std::vector<UsedPointData> level2;
                                    
                                    std::vector<TrackElement> elements = cp.second.GetTrack().GetElements();
                                    for (const auto& ele : elements)
                                    {
                                        UsedPointData usedPointData;
                                        usedPointData.imageid = ele.image_id;
                                        Eigen::Vector2d xy = ele.xy;
                                        for (int i = 0; i < 2; i++)
                                        {
                                            usedPointData.xy[i] = xy[i];
                                        }
                                        level2.push_back(usedPointData);
                                    }
                                    atBinFile.usedPointVec.push_back(level2);
                                }
                            }
                        }
                    }

                }
                atBinFile.Serialize(out);
                out.close();
            }
            catch (const std::exception& err)
            {
                LOGE(String::StringPrintf("Saving: %s failed! Msg: %s", AT_filepath.c_str(), err.what()));
                out.close();
                return false;
            }
            return true;
        }

        bool BlockObject::LoadXLS(const std::string& xls_file_path, std::shared_ptr<ATData>ATdata)
        {
            const void* handle;
            const char* utf8_string;
            int ret;
            unsigned int info;
            unsigned int fat_count;
            unsigned int sst_count;
            unsigned int worksheet_count;
            unsigned int format_count;
            unsigned int xf_count;
            unsigned int idx;
            unsigned int next_sector;
            try
            {
                 
                ret = freexl_open(xls_file_path.c_str(), &handle);
                if (ret != FREEXL_OK)
                {
                    LOGE(String::StringPrintf("OPEN ERROR: %d", ret));
                    return false;
                }
                 
                ret = freexl_get_info(handle, FREEXL_BIFF_SHEET_COUNT, &worksheet_count);
                if (ret != FREEXL_OK)
                {
                    LOGE(String::StringPrintf("GET-INFO [FREEXL_BIFF_SHEET_COUNT] Error: %d", ret));
                    return false;
                }

                for (idx = 0; idx < worksheet_count; idx++)
                {
                     
                    unsigned short active;
                    unsigned int rows;
                    unsigned short columns;

                    ret = freexl_get_worksheet_name(handle, idx, &utf8_string);


                    if (ret != FREEXL_OK)
                    {
                        LOGE(String::StringPrintf("GET-WORKSHEET-NAME Error: %d", ret));
                        return false;
                    }
                    if (utf8_string == NULL)
                    {
                        LOGW(String::StringPrintf("%3u NULL (unnamed)", idx));
                        continue;
                    }
                    else
                    {
                        LOGI(String::StringPrintf("idx = %d; worksheet = %s", idx, utf8_string));
                    }
                    ret = freexl_select_active_worksheet(handle, idx);
                    if (ret != FREEXL_OK)
                    {
                        LOGE(String::StringPrintf("SELECT-ACTIVE_WORKSHEET Error: %d", ret));
                        return false;
                    }
                    ret = freexl_get_active_worksheet(handle, &active);
                    if (ret != FREEXL_OK)
                    {
                        LOGE(String::StringPrintf("GET-ACTIVE_WORKSHEET Error: %d", ret));
                        return false;
                    }

                    ret = freexl_worksheet_dimensions(handle, &rows, &columns);
                    if (ret != FREEXL_OK)
                    {
                        LOGE(String::StringPrintf("WORKSHEET-DIMENSIONS Error: %d\n", ret));
                        return false;
                    }

                    
                    if (std::strcmp(utf8_string, "Photogroups") == 0)
                    {
                        
                        std::map<std::string, uint8_t> headNames;
                        std::vector<std::string>mandatory = { "Name","Width","Height","FocalLength" };


                        for (uint8_t i_col = 0; i_col < columns; i_col++)
                        {
                            FreeXL_CellValue val;
                            ret = freexl_get_cell_value(handle, 0, i_col, &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell Photogroups value failed :%d  %d",0, i_col));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                headNames.insert(std::make_pair(val.value.text_value, i_col));
                            }
                        }
                        if (headNames.empty())
                        {
                            LOGE("Empty A1!");
                            return false;
                        }
                        
                        for (int i = 0; i < mandatory.size(); i++)
                        {
                            if (headNames.find(mandatory[i]) == headNames.end())
                            {
                                LOGE(String::StringPrintf("xls error no: %s", mandatory[i].c_str()));
                                return false;
                            }
                        }

                        bool hasSensorSize = headNames.find("SensorSize") != headNames.end();
                        bool hasPixelSize = headNames.find("PixelSize") != headNames.end();
                        
                        if (!hasSensorSize && !hasPixelSize)
                        {
                            LOGE("Neither SensorSize nor PixelSize!");
                            return false;
                        }

                        bool hasPPX = headNames.find("PrincipalPointX") != headNames.end();
                        bool hasPPY = headNames.find("PrincipalPointY") != headNames.end();
                        bool hasPPXMM = headNames.find("PrincipalPointXmm") != headNames.end();
                        bool hasPPYMM = headNames.find("PrincipalPointYmm") != headNames.end();

                        if (!(hasPPX && hasPPY) && !(hasPPXMM && hasPPYMM))
                        {
                            LOGE("Neither PrincipalPointX PrincipalPointY nor PrincipalPointXmm PrincipalPointYmm!");
                            return false;
                        }
                        
                        for (int i_row = 1; i_row < rows; i_row++)
                        {
                            PhotoGroup pg;
                            Camera camera;
                            camera.SetCameraId(i_row - 1);
                            pg.SetId(i_row - 1);
                            camera.SetModelId(6);

                            FreeXL_CellValue val;
                            
                            ret = freexl_get_cell_value(handle, i_row, headNames["Name"], &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell Name value failed: %d %d ", i_row, headNames["Name"]));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_NULL)
                            {
                                continue;
                            }
                            if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                camera.SetCameraName(val.value.text_value);
                                pg.SetName(val.value.text_value);
                            }
                            else if (val.type == FREEXL_CELL_DOUBLE)
                            {
                                camera.SetCameraName(std::to_string(int(val.value.double_value)));
                                pg.SetName(std::to_string(int(val.value.double_value)));
                            }
                            else if (val.type == FREEXL_CELL_INT)
                            {
                                camera.SetCameraName(std::to_string(val.value.int_value));
                                pg.SetName(std::to_string(val.value.int_value));
                            }
                            else
                            {
                                LOGE(String::StringPrintf("Cell Name type error in % dX % d", i_row, headNames["Name"]));
                                return false;
                            }


                            
                            ret = freexl_get_cell_value(handle, i_row, headNames["Width"], &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Width"]));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_DOUBLE)
                            {
                                camera.SetWidth(val.value.double_value);
                            }
                            else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                camera.SetWidth(std::atof(val.value.text_value));
                            }
                            else if (val.type == FREEXL_CELL_INT)
                            {
                                camera.SetWidth(val.value.int_value);
                            }
                            else
                            {
                                LOGE(String::StringPrintf("Error Photogroup Width in %dX%d:", i_row + 1, headNames["Width"] + 1));
                                return false;
                            }


                            
                            ret = freexl_get_cell_value(handle, i_row, headNames["Height"], &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Height"]));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_DOUBLE)
                            {
                                camera.SetHeight(val.value.double_value);
                            }
                            else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                camera.SetHeight(std::atof(val.value.text_value));
                            }
                            else if (val.type == FREEXL_CELL_INT)
                            {
                                camera.SetHeight(val.value.int_value);
                            }
                            else
                            {
                                LOGE(String::StringPrintf("Error Photogroup Height in %dX%d:", i_row + 1, headNames["Height"] + 1));
                                return false;
                            }

                            
                            ret = freexl_get_cell_value(handle, i_row, headNames["FocalLength"], &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["FocalLength"]));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_DOUBLE)
                            {
                                camera.SetFocalLengthMM(val.value.double_value);
                            }
                            else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                camera.SetFocalLengthMM(std::atof(val.value.text_value));
                            }
                            else if (val.type == FREEXL_CELL_INT)
                            {
                                camera.SetFocalLengthMM(val.value.int_value);
                            }
                            else
                            {
                                LOGE(String::StringPrintf("Error Photogroup FocalLength in %dX%d:", i_row + 1, headNames["FocalLength"] + 1));
                                return false;
                            }

                            
                            if (hasSensorSize)
                            {
                                ret = freexl_get_cell_value(handle, i_row, headNames["SensorSize"], &val);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d  %d", i_row, headNames["SensorSize"]));
                                    return false;
                                }
                                if (val.type == FREEXL_CELL_DOUBLE)
                                {
                                    camera.SetSensorSize(val.value.double_value);
                                }
                                else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                {
                                    camera.SetSensorSize(std::atof(val.value.text_value));
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Photogroup Sensorsize in %dX%d:", i_row + 1, headNames["SensorSize"] + 1));
                                    return false;
                                }
                            }

                            
                            if (hasPixelSize)
                            {
                                ret = freexl_get_cell_value(handle, i_row, headNames["PixelSize"], &val);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell  PixelSize value failed: %d %d", i_row, headNames["PixelSize"]));
                                    return false;
                                }
                                if (val.type == FREEXL_CELL_DOUBLE)
                                {
                                    camera.SetSensorSize(std::max(camera.GetHeight(), camera.GetWidth()) * val.value.double_value);
                                }
                                else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                {
                                    camera.SetSensorSize(std::max(camera.GetHeight(), camera.GetWidth()) * std::atof(val.value.text_value));
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Photogroup PixelSize in %dX%d:", i_row + 1, headNames["PixelSize"] + 1));
                                    return false;
                                }

                            }

                            
                            camera.SetFocalLengthIn35mm(36 * camera.GetFocalLengthMM() / camera.GetSensorSize());

                            
                            if (hasPPX && hasPPY)
                            {
                                ret = freexl_get_cell_value(handle, i_row, headNames["PrincipalPointX"], &val);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell PrincipalPointX value failed: %d %d", i_row, headNames["PrincipalPointX"]));
                                    return false;
                                }
                                if (val.type == FREEXL_CELL_DOUBLE)
                                {
                                    camera.SetPrincipalPointX(val.value.double_value);
                                }
                                else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                {
                                    camera.SetPrincipalPointX(std::atof(val.value.text_value));
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Photogroup PrincipalPointX in %dX%d:", i_row + 1, headNames["PrincipalPointX"] + 1));
                                    return false;
                                }

                                ret = freexl_get_cell_value(handle, i_row, headNames["PrincipalPointY"], &val);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell PrincipalPointY value failed: %d %d", i_row, headNames["PrincipalPointY"]));
                                    return false;
                                }
                                if (val.type == FREEXL_CELL_DOUBLE)
                                {
                                    camera.SetPrincipalPointY(val.value.double_value);
                                }
                                else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                {
                                    camera.SetPrincipalPointY(std::atof(val.value.text_value));
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Photogroup PrincipalPointY in %dX%d:", i_row + 1, headNames["PrincipalPointY"] + 1));
                                    return false;
                                }
                            }

                            
                            if (hasPPXMM && hasPPYMM)
                            {
                                ret = freexl_get_cell_value(handle, i_row, headNames["PrincipalPointXmm"], &val);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell PrincipalPointXmm value failed: %d %d", i_row, headNames["PrincipalPointXmm"]));
                                    return false;
                                }
                                double ccd_width = camera.GetSensorSize() / std::max(camera.GetHeight(), camera.GetWidth());
                                if (val.type == FREEXL_CELL_DOUBLE)
                                {
                                    camera.SetPrincipalPointX(val.value.double_value / ccd_width);
                                }
                                else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                {
                                    camera.SetPrincipalPointX(std::atof(val.value.text_value) / ccd_width);
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Photogroup PrincipalPointXmm in %dX%d:", i_row + 1, headNames["PrincipalPointXmm"] + 1));
                                    return false;
                                }


                                ret = freexl_get_cell_value(handle, i_row, headNames["PrincipalPointYmm"], &val);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["PrincipalPointYmm"]));
                                    return false;
                                }
                                if (val.type == FREEXL_CELL_DOUBLE)
                                {
                                    camera.SetPrincipalPointY(val.value.double_value / ccd_width);
                                }
                                else if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                {
                                    camera.SetPrincipalPointY(std::atof(val.value.text_value) / ccd_width);
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Photogroup PrincipalPointYmm in %dX%d:", i_row + 1, headNames["PrincipalPointYmm"] + 1));
                                    return false;
                                }

                            }

                            double f_pix = std::max(camera.GetHeight(), camera.GetWidth()) * camera.GetFocalLengthMM() / camera.GetSensorSize();
                            camera.InitializeWithId(6, f_pix, camera.GetWidth(), camera.GetHeight());

                            

                            pg.SetCamera(camera);
                            ATdata->AddCamera(camera);
                            photogroups_.insert(std::make_pair(pg.GetId(), pg));
                        }

                    }


                    
                    if (std::strcmp(utf8_string, "Photos") == 0)
                    {
                        
                        std::map<std::string, uint8_t> headNames;
                        std::vector<std::string>mandatory = { "Name","PhotogroupName" };


                        for (uint8_t i_col = 0; i_col < columns; i_col++)
                        {
                            FreeXL_CellValue val;
                            ret = freexl_get_cell_value(handle, 0, i_col, &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell Name PhotogroupNamevalue failed: %d %d", 0, i_col));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                headNames.insert(std::make_pair(val.value.text_value, i_col));
                            }
                        }
                        if (headNames.empty())
                        {
                            LOGE("Empty A1!");
                            return false;
                        
                        }
                        
                        for (int i = 0; i < mandatory.size(); i++)
                        {
                            if (headNames.find(mandatory[i]) == headNames.end())
                            {
                                LOGE(String::StringPrintf("Get cell Name PhotogroupName value failed: %s", mandatory[i].c_str()));
                                return false;
                            }
                        }
                        
                        bool hasDirectory = headNames.find("Directory") != headNames.end();
                        bool hasExtesion = headNames.find("Extesion") != headNames.end();

                        bool hasLongitude = headNames.find("Longitude") != headNames.end();
                        bool hasLatitude = headNames.find("Latitude") != headNames.end();
                        bool hasHeight = headNames.find("Height") != headNames.end();

                        bool hasEasting = headNames.find("Easting") != headNames.end();
                        bool hasNorthing = headNames.find("Northing") != headNames.end();

                        bool hasOmega = headNames.find("Omega") != headNames.end();
                        bool hasPhi = headNames.find("Phi") != headNames.end();
                        bool hasKappa = headNames.find("Kappa") != headNames.end();

                        bool hasHeading = headNames.find("Heading") != headNames.end();
                        bool hasPitch = headNames.find("Pitch") != headNames.end();
                        bool hasRoll = headNames.find("Roll") != headNames.end();

                        bool hasYaw = headNames.find("Yaw") != headNames.end();

                        bool hasRotation00 = headNames.find("Rotation00") != headNames.end();
                        bool hasRotation01 = headNames.find("Rotation01") != headNames.end();
                        bool hasRotation02 = headNames.find("Rotation02") != headNames.end();
                        bool hasRotation10 = headNames.find("Rotation10") != headNames.end();
                        bool hasRotation11 = headNames.find("Rotation11") != headNames.end();
                        bool hasRotation12 = headNames.find("Rotation12") != headNames.end();
                        bool hasRotation20 = headNames.find("Rotation20") != headNames.end();
                        bool hasRotation21 = headNames.find("Rotation21") != headNames.end();
                        bool hasRotation22 = headNames.find("Rotation22") != headNames.end();

                        
                        if (hasLongitude && hasLatitude)
                        {
                            if (hasEasting || hasNorthing)
                            {
                                LOGE("Either LLH or ENH!");
                                return false;
                            }
                            if (!hasHeight)
                            {
                                LOGE("No Height in LLH(Longitude Latitude Height)!");
                                return false;
                            }
                        }
                        if (hasEasting && hasNorthing)
                        {
                            if (hasLongitude || hasLatitude)
                            {
                                LOGE("Either LLH or ENH");
                                return false;
                            }
                            if (!hasHeight)
                            {
                                LOGE("No Height in ENH(Easting Northing Height)");
                                return false;
                            }
                        }

                        int img_index = 0;
                        for (int i_row = 1; i_row < rows; i_row++)
                        {
                            Image img;
                            img.SetImageId(img_index);
                            
                            FreeXL_CellValue value;
                            ret = freexl_get_cell_value(handle, i_row, headNames["Name"], &value);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Name"]));
                                return false;
                            }
                            if (value.type == FREEXL_CELL_NULL)
                            {
                                continue;
                            }
                            if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                            {
                                img.SetName(value.value.text_value);
                            }
                            else if (value.type == FREEXL_CELL_DOUBLE)
                            {
                                img.SetName(std::to_string(value.value.double_value));
                            }
                            else if (value.type == FREEXL_CELL_INT)
                            {
                                img.SetName(std::to_string(value.value.int_value));
                            }
                            else
                            {
                                LOGE(String::StringPrintf("Error Photo Name in %dX%d:", i_row, headNames["Name"]));
                                return false;
                            }

                            if (hasDirectory)
                            {
                                ret = freexl_get_cell_value(handle, i_row, headNames["Directory"], &value);
                                if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                {
                                    img.SetPath(value.value.text_value);
                                }
                            }
                            
                            ret = freexl_get_cell_value(handle, i_row, headNames["PhotogroupName"], &value);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["PhotogroupName"]));
                                return false;
                            }
                            if (value.type == FREEXL_CELL_NULL)
                            {
                                continue;
                            }
                            if (value.type != FREEXL_CELL_TEXT && value.type != FREEXL_CELL_SST_TEXT)
                            {
                                LOGE(String::StringPrintf("Error PhotogroupName in %dX%d:", i_row, headNames["PhotogroupName"]));
                                return false;
                            }
                            auto pos = std::find_if(photogroups_.begin(), photogroups_.end(), [value](const std::pair<group_t, PhotoGroup>& pg) {
                                return std::strcmp(value.value.text_value, pg.second.GetName().c_str()) == 0; });
                            if (pos == photogroups_.end())
                            {
                                LOGE(String::StringPrintf("Invalid camera name: %s", value.value.text_value));
                                return false;
                            }
                            pos->second.AddImageId(img_index);
                            img.SetPhotoGroupID(pos->first);
                            img.SetCameraId(pos->first);
                            img.SetHeight(pos->second.GetCamera().GetHeight());
                            img.SetWidth(pos->second.GetCamera().GetWidth());

                            if (hasLongitude && hasLatitude && hasHeight)
                            {
                                Eigen::Vector3d position = Eigen::Vector3d::Zero();
                                ret = freexl_get_cell_value(handle, i_row, headNames["Longitude"], &value);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Longitude"]));
                                    return false;
                                }
                                if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                {
                                    position[0] = std::atof(value.value.text_value);
                                }
                                else if (value.type == FREEXL_CELL_DOUBLE)
                                {
                                    position[1] = value.value.double_value;
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Longitude in %dX%d:", i_row, headNames["Longitude"]));
                                    return false;
                                }


                                ret = freexl_get_cell_value(handle, i_row, headNames["Latitude"], &value);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Latitude"]));
                                    return false;
                                }
                                if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                {
                                    position[1] = std::atof(value.value.text_value);
                                }
                                else if (value.type == FREEXL_CELL_DOUBLE)
                                {
                                    position[1] = value.value.double_value;
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Latitude in %dX%d:", i_row, headNames["Latitude"]));
                                    return false;
                                }


                                ret = freexl_get_cell_value(handle, i_row, headNames["Height"], &value);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Height"]));
                                    return false;
                                }
                                if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                {
                                    position[2] = std::atof(value.value.text_value);
                                }
                                else if (value.type == FREEXL_CELL_DOUBLE)
                                {
                                    position[2] = value.value.double_value;
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Height in %dX%d:", i_row, headNames["Height"]));
                                    return false;
                                }
                                
                                srs_s srs;
                                std::string srstemp = GEO84SRS;
                                if (img.GetXmpData().isValid)
                                {
                                    srstemp = "EPSG:4326+5773";
                                }
                                
                                
                                auto pos = std::find_if(srs_map_.begin(), srs_map_.end(), [&](const std::pair<srsid_t, srs_s>& srs_temp) {return srs_temp.second.definition == srstemp; });
                                if (pos == srs_map_.end())
                                {

                                    srs = CoordinateDescriptor::GetSRSFromDefinition(srstemp);
                                    srs.ID = GenerateValidSrsId();
                                }
                                else
                                {
                                    srs.ID = pos->first;
                                    srs = pos->second;
                                }
                                srs_map_.insert(std::make_pair(srs.ID, srs));
                                img.SetPriorSrs(srs);
                                img.SetPositionPrior(position);
                            }

                            if (hasEasting && hasNorthing && hasHeight)
                            {
                                Eigen::Vector3d position = Eigen::Vector3d::Zero();
                                ret = freexl_get_cell_value(handle, i_row, headNames["Easting"], &value);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Easting"]));
                                    return false;
                                }
                                if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                {
                                    position[0] = std::atof(value.value.text_value);
                                }
                                else if (value.type == FREEXL_CELL_DOUBLE)
                                {
                                    position[1] = value.value.double_value;
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Easting in %dX%d:", i_row, headNames["Easting"]));
                                    return false;
                                }


                                ret = freexl_get_cell_value(handle, i_row, headNames["Northing"], &value);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Northing"]));
                                    return false;
                                }
                                if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                {
                                    position[1] = std::atof(value.value.text_value);
                                }
                                else if (value.type == FREEXL_CELL_DOUBLE)
                                {
                                    position[1] = value.value.double_value;
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Northing in %dX%d:", i_row, headNames["Northing"]));
                                    return false;
                                }


                                ret = freexl_get_cell_value(handle, i_row, headNames["Height"], &value);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Height"]));
                                    return false;
                                }
                                if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                {
                                    position[2] = std::atof(value.value.text_value);
                                }
                                else if (value.type == FREEXL_CELL_DOUBLE)
                                {
                                    position[2] = value.value.double_value;
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error Height in %dX%d:", i_row, headNames["Height"]));
                                    return false;
                                }
                                srs_s srs;
                                
                                
                                std::string srstemp = GEO84SRS;
                                if (img.GetXmpData().isValid)
                                {
                                    srstemp = "EPSG:4326+5773";
                                }
                                
                                
                                auto pos = std::find_if(srs_map_.begin(), srs_map_.end(), [&](const std::pair<srsid_t, srs_s>& srs_temp) {return srs_temp.second.definition == srstemp; });
                                if (pos == srs_map_.end())
                                {

                                    srs = CoordinateDescriptor::GetSRSFromDefinition(srstemp);
                                    srs.ID = GenerateValidSrsId();
                                }
                                {
                                    srs.ID = pos->first;
                                    srs = pos->second;
                                }
                                srs_map_.insert(std::make_pair(srs.ID, srs));
                                img.SetPriorSrs(srs);
                                img.SetPositionPrior(position);
                            }

                            
                            if (hasRotation00 && hasRotation01 && hasRotation02 && hasRotation10 && \
                                hasRotation11 && hasRotation12 && hasRotation20 && hasRotation21 && hasRotation22)
                            {
                                Eigen::Matrix3d R = Eigen::Matrix3d::Zero();
                                for (int i = 0; i < 3; i++)
                                {
                                    for (int j = 0; j < 3; j++)
                                    {
                                        std::string Rotation = "Rotation" + std::to_string(i) + std::to_string(j);
                                        ret = freexl_get_cell_value(handle, i_row, headNames[Rotation], &value);
                                        if (ret != FREEXL_OK)
                                        {
                                            LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames[Rotation]));
                                            return false;
                                        }
                                        if (value.type == FREEXL_CELL_TEXT || value.type == FREEXL_CELL_SST_TEXT)
                                        {
                                            R(i, j) = std::atof(value.value.text_value);
                                        }
                                        else if (value.type == FREEXL_CELL_DOUBLE)
                                        {
                                            R(i, j) = value.value.double_value;
                                        }
                                        else
                                        {
                                            LOGE(String::StringPrintf("Error %s in %dX%d:", Rotation.c_str(), i_row, headNames["Rotation"]));
                                            return false;
                                        }

                                    }
                                }
                                
                                img.SetRotationMatrixPrior(R);
                            }
                            ATdata->AddImage(img);
                            img_index++;
                        }
                    }
                    
                    if (std::strcmp(utf8_string, "ControlPoints") == 0)
                    {
                        std::map<std::string, uint8_t> headNames;

                        for (uint8_t i_col = 0; i_col < columns; i_col++)
                        {
                            FreeXL_CellValue val;
                            ret = freexl_get_cell_value(handle, 0, i_col, &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell value failed: %d %d", 0, i_col));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                headNames.insert(std::make_pair(val.value.text_value, i_col));
                            }
                        }

                        bool hasName = headNames.find("Name") != headNames.end();
                        bool hasLongitude = headNames.find("Longitude") != headNames.end();
                        bool hasLatitude = headNames.find("Latitude") != headNames.end();
                        bool hasEasting = headNames.find("Easting") != headNames.end();
                        bool hasNorthing = headNames.find("Northing") != headNames.end();
                        bool hasHeight = headNames.find("Height") != headNames.end();

                        if (hasName)
                        {
                            EIGEN_STL_UMAP(point3D_t, ControlPoint) cps;
                            int index_cp = 0;
                            std::string name;
                            for (int i_row = 1; i_row < rows; i_row++)
                            {
                                ControlPoint cp;
                                FreeXL_CellValue val;
                                ret = freexl_get_cell_value(handle, i_row, headNames["Name"], &val);
                                if (ret != FREEXL_OK)
                                {
                                    LOGE(String::StringPrintf("Get cell value failed: %d %d" , i_row, headNames["Name"]));
                                    return false;
                                }
                                if (val.type == FREEXL_CELL_NULL)
                                {
                                    continue;
                                }
                                if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                {
                                    name = val.value.text_value;
                                }
                                else if (val.type == FREEXL_CELL_DOUBLE)
                                {
                                    name = std::to_string(val.value.double_value);
                                }
                                else if (val.type == FREEXL_CELL_INT)
                                {
                                    name = std::to_string(val.value.int_value);
                                }
                                else
                                {
                                    LOGE(String::StringPrintf("Error GCP name in %dX%d:", i_row, headNames["Name"]));
                                    return false;
                                }
                                cp.SetName(name);
                                cp.SetId(index_cp);

                                if (hasEasting && hasNorthing && hasHeight)
                                {
                                    Eigen::Vector3d position = Eigen::Vector3d::Zero();
                                    ret = freexl_get_cell_value(handle, i_row, headNames["Easting"], &val);
                                    if (ret != FREEXL_OK)
                                    {
                                        LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Easting"]));
                                        return false;
                                    }
                                    if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                    {
                                        position[0] = std::atof(val.value.text_value);
                                    }
                                    else if (val.type == FREEXL_CELL_DOUBLE)
                                    {
                                        position[1] = val.value.double_value;
                                    }
                                    else
                                    {
                                        LOGE(String::StringPrintf("Error Easting in %dX%d:", i_row, headNames["Easting"]));
                                        return false;
                                    }


                                    ret = freexl_get_cell_value(handle, i_row, headNames["Northing"], &val);
                                    if (ret != FREEXL_OK)
                                    {
                                        LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Northing"]));
                                        return false;
                                    }
                                    if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                    {
                                        position[1] = std::atof(val.value.text_value);
                                    }
                                    else if (val.type == FREEXL_CELL_DOUBLE)
                                    {
                                        position[1] = val.value.double_value;
                                    }
                                    else
                                    {
                                        LOGE(String::StringPrintf("Error Northing in %dX%d:", i_row, headNames["Northing"]));
                                        return false;
                                    }


                                    ret = freexl_get_cell_value(handle, i_row, headNames["Height"], &val);
                                    if (ret != FREEXL_OK)
                                    {
                                        LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Height"]));
                                        return false;
                                    }
                                    if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                    {
                                        position[2] = std::atof(val.value.text_value);
                                    }
                                    else if (val.type == FREEXL_CELL_DOUBLE)
                                    {
                                        position[2] = val.value.double_value;
                                    }
                                    else
                                    {
                                        LOGE(String::StringPrintf("Error Height in %dX%d:", i_row, headNames["Height"]));
                                        return false;
                                    }
                                    cp.SetGivenXYZ(position);
                                }
                                if (hasLongitude && hasLatitude && hasHeight)
                                {
                                    Eigen::Vector3d position = Eigen::Vector3d::Zero();
                                    ret = freexl_get_cell_value(handle, i_row, headNames["Longitude"], &val);
                                    if (ret != FREEXL_OK)
                                    {
                                        LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Longitude"]));
                                        return false;
                                    }
                                    if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                    {
                                        position[0] = std::atof(val.value.text_value);
                                    }
                                    else if (val.type == FREEXL_CELL_DOUBLE)
                                    {
                                        position[1] = val.value.double_value;
                                    }
                                    else
                                    {
                                        LOGE(String::StringPrintf("Error Longitude in %dX%d:", i_row, headNames["Longitude"]));
                                        return false;
                                    }


                                    ret = freexl_get_cell_value(handle, i_row, headNames["Latitude"], &val);
                                    if (ret != FREEXL_OK)
                                    {
                                        LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Latitude"]));
                                        return false;
                                    }
                                    if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                    {
                                        position[1] = std::atof(val.value.text_value);
                                    }
                                    else if (val.type == FREEXL_CELL_DOUBLE)
                                    {
                                        position[1] = val.value.double_value;
                                    }
                                    else
                                    {
                                        LOGE(String::StringPrintf("Error Latitude in %dX%d:", i_row, headNames["Latitude"]));
                                        return false;
                                    }


                                    ret = freexl_get_cell_value(handle, i_row, headNames["Height"], &val);
                                    if (ret != FREEXL_OK)
                                    {
                                        LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, headNames["Height"]));
                                        return false;
                                    }
                                    if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                                    {
                                        position[2] = std::atof(val.value.text_value);
                                    }
                                    else if (val.type == FREEXL_CELL_DOUBLE)
                                    {
                                        position[2] = val.value.double_value;
                                    }
                                    else
                                    {
                                        LOGE(String::StringPrintf("Error Height in %dX%d:", i_row, headNames["Height"]));
                                        return false;
                                    }
                                    cp.SetGivenXYZ(position);
                                }
                                cps.insert(std::make_pair(index_cp, cp));
                                index_cp++;
                            }
                            ATdata->SetControlPoints(cps);
                        }
                        else
                        {
                            continue;
                        }

                    }
                    
                    if (std::strcmp(utf8_string, "Options") == 0)
                    {
                        std::map<std::string, uint8_t> headNames;

                        for (uint8_t i_row = 0; i_row < rows; i_row++)
                        {
                            FreeXL_CellValue val;
                            ret = freexl_get_cell_value(handle, i_row, 0, &val);
                            if (ret != FREEXL_OK)
                            {
                                LOGE(String::StringPrintf("Get cell value failed: %d %d", i_row, 0));
                                return false;
                            }
                            if (val.type == FREEXL_CELL_NULL)
                            {
                                continue;
                            }
                            if (val.type == FREEXL_CELL_TEXT || val.type == FREEXL_CELL_SST_TEXT)
                            {
                                headNames.insert(std::make_pair(val.value.text_value, i_row));
                            }
                        }

                        if (headNames.empty())
                        {
                            LOGE("Empty A1!");
                            return false;
                        }

                        FreeXL_CellValue val;
                        ret = freexl_get_cell_value(handle, headNames["SRS"], 1, &val);
                        if (ret != FREEXL_OK)
                        {
                            LOGE(String::StringPrintf("Get cell value failed: %d %d", headNames["SRS"], 1));
                            return false;
                        }
                        srs_s blockSRS;
                        blockSRS.definition = GEO84SRS;
                        if (val.type != FREEXL_CELL_NULL)
                        {
#ifdef USE_AI3D_PROJ
                            
                            std::string deftext = val.value.text_value;
                            AI3D::PROJ::CoordinateReferenceSystem crs(deftext);
                            
                            
                            srs_s newsrs;
                            newsrs.definition = crs.authid().toStdString();
                            newsrs.type = crs.GetType();
                            newsrs.name = crs.GetDescription();
                            blockSRS.definition = newsrs.definition;

#else
                            blockSRS.definition = val.value.text_value;
#endif 
                            
                        }
                        blockSRS.ID = GenerateValidSrsId();
                        auto tmpSrs = CoordinateDescriptor::GetSRSFromDefinition(blockSRS.definition);
                        blockSRS.name = tmpSrs.name;
                        blockSRS.type = tmpSrs.type;
                        srs_map_.insert(std::make_pair(blockSRS.ID, blockSRS));
                        blockSRS_id_ = blockSRS.ID;

                        
                        
                        
                        
                        
                        
                    }
                }

                if (ATdata->HasPriorPositionImages())
                {
                    UpdateSRSMap(CoordinateDescriptor::GetSRSFromDefinition("EPSG:4978"));
                    blockSRS_id_ = ExistSRS("EPSG:4978");
                    ATdata->SetMetadataToCenter();
                }

                
              
              
              
              
              
              
              

                
                std::string dst_definition = BASESRS;
                
                
                
                
                
                



                
                
                
                
                
                
                
                
                
                
                ATdata->TransformControlPoints(dst_definition);
                ATdata->SetLocalGcpSrs(dst_definition);
                
                
            

                ret = freexl_close(handle);
                if (ret != FREEXL_OK)
                {
                    LOGE("freexl_close error!");
                    return false;
                }
            }
            catch (const std::exception& err)
            {
                LOGE(err.what());
                ret = freexl_close(handle);
                if (ret != FREEXL_OK)
                {
                    LOGE("freexl_close error!");
                    return false;
                }
                return false;
            }
            return true;
        }
        
        bool BlockObject::LoadXLSX(const std::string& xlsx_file_path, std::shared_ptr<ATData>ATdata)
        {
            try
            {
                 
                xlnt::workbook wb;
                wb.load(xlsx_file_path);

                for (auto ws_itr = wb.begin(); ws_itr != wb.end(); ++ws_itr)
                {
                     
                    unsigned short active;
                    unsigned int rows;
                    unsigned short columns;
                    xlnt::worksheet ws = *ws_itr;
                    
                    LOGI(String::StringPrintf("idx = %d; worksheet = %s", wb.index(ws), ws.title().c_str()));
                    
                    auto range = ws.calculate_dimension();
                    rows = range.height();
                    columns = range.width();

                    const xlnt::range& xlrange = ws.rows();
                    
                    if (std::strcmp(ws.title().c_str(), "Photogroups") == 0)
                    {
                        
                        std::map<std::string, uint8_t> headNames;
                        std::vector<std::string>mandatory = { "Name","Width","Height","FocalLength" };


                        for (uint8_t i_col = 0; i_col < columns; i_col++)
                        {
                            auto tmpCell = xlrange[0][i_col].value<std::string>();
                            if (tmpCell.empty())
                            {
                                continue;
                            }
                            headNames.insert(std::make_pair(tmpCell, i_col));
                        }
                        if (headNames.empty())
                        {
                            LOGE("Empty A1!");
                            return false;
                        }
                        
                        for (int i = 0; i < mandatory.size(); i++)
                        {
                            if (headNames.find(mandatory[i]) == headNames.end())
                            {
                                LOGE(String::StringPrintf("xls error no: %s", mandatory[i].c_str()));
                                return false;
                            }
                        }

                        bool hasSensorSize = headNames.find("SensorSize") != headNames.end();
                        bool hasPixelSize = headNames.find("PixelSize") != headNames.end();
                        
                        if (!hasSensorSize && !hasPixelSize)
                        {
                            LOGE("Neither SensorSize nor PixelSize!");
                            return false;
                        }

                        bool hasPPX = headNames.find("PrincipalPointX") != headNames.end();
                        bool hasPPY = headNames.find("PrincipalPointY") != headNames.end();
                        bool hasPPXMM = headNames.find("PrincipalPointXmm") != headNames.end();
                        bool hasPPYMM = headNames.find("PrincipalPointYmm") != headNames.end();

                        if (!(hasPPX && hasPPY) && !(hasPPXMM && hasPPYMM))
                        {
                            LOGE("Neither PrincipalPointX PrincipalPointY nor PrincipalPointXmm PrincipalPointYmm!");
                            return false;
                        }
                        
                        for (int i_row = 1; i_row < rows; i_row++)
                        {
                            PhotoGroup pg;
                            Camera camera;
                            camera.SetCameraId(i_row - 1);
                            pg.SetId(i_row - 1);
                            camera.SetModelId(6);

                            
                            std::string name = xlrange[i_row][headNames["Name"]].to_string();
                            if (name.empty())
                            {
                                continue;
                            }
                            camera.SetCameraName(name);
                            pg.SetName(name);

                            
                            std::string width;
                            width = xlrange[i_row][headNames["Width"]].to_string();
                            camera.SetWidth(std::atof(width.c_str()));

                            
                            std::string height = xlrange[i_row][headNames["Height"]].to_string();
                            camera.SetHeight(std::atof(height.c_str()));

                            
                            std::string  focallength = xlrange[i_row][headNames["FocalLength"]].to_string();
                            camera.SetFocalLengthMM(std::atof(focallength.c_str()));

                            
                            if (hasSensorSize)
                            {
                                std::string sensorsize = xlrange[i_row][headNames["SensorSize"]].to_string();
                                camera.SetSensorSize(std::atof(sensorsize.c_str()));
                            }

                            
                            if (hasPixelSize)
                            {
                                std::string pixelsize = xlrange[i_row][headNames["PixelSize"]].to_string();
                                camera.SetSensorSize(std::max(camera.GetHeight(), camera.GetWidth()) * std::atof(pixelsize.c_str()));
                            }

                            
                            camera.SetFocalLengthIn35mm(36 * camera.GetFocalLengthMM() / camera.GetSensorSize());

                            
                            if (hasPPX && hasPPY)
                            {
                                std::string ppx = xlrange[i_row][headNames["PrincipalPointX"]].to_string();
                                camera.SetPrincipalPointX(std::atof(ppx.c_str()));

                                std::string ppy = xlrange[i_row][headNames["PrincipalPointY"]].to_string();
                                camera.SetPrincipalPointY(std::atof(ppy.c_str()));
                            }

                            
                            if (hasPPXMM && hasPPYMM)
                            {
                                std::string ppxmm = xlrange[i_row][headNames["PrincipalPointXmm"]].to_string();
                                double ccd_width = camera.GetSensorSize() / std::max(camera.GetHeight(), camera.GetWidth());
                                camera.SetPrincipalPointX(std::atof(ppxmm.c_str()) / ccd_width);

                                std::string ppymm = xlrange[i_row][headNames["PrincipalPointYmm"]].to_string();
                                camera.SetPrincipalPointY(std::atof(ppymm.c_str()) / ccd_width);
                            }

                            double f_pix = std::max(camera.GetHeight(), camera.GetWidth()) * camera.GetFocalLengthMM() / camera.GetSensorSize();
                            camera.InitializeWithId(6, f_pix, camera.GetWidth(), camera.GetHeight());

                            

                            pg.SetCamera(camera);
                            ATdata->AddCamera(camera);
                            photogroups_.insert(std::make_pair(pg.GetId(), pg));
                        }

                    }


                    
                    if (std::strcmp(ws.title().c_str(), "Photos") == 0)
                    {
                        
                        std::map<std::string, uint8_t> headNames;
                        std::vector<std::string>mandatory = { "Name","PhotogroupName" };

                        for (uint8_t i_col = 0; i_col < columns; i_col++)
                        {
                            std::string headTitles = xlrange[0][i_col].value<std::string>();
                            headNames.insert(std::make_pair(headTitles, i_col));
                        }
                        
                        for (int i = 0; i < mandatory.size(); i++)
                        {
                            if (headNames.find(mandatory[i]) == headNames.end())
                            {
                                LOGE(String::StringPrintf("%s is mandatory!", mandatory[i].c_str()));
                                return false;
                            }
                        }
                        
                        bool hasDirectory = headNames.find("Directory") != headNames.end();
                        bool hasExtesion = headNames.find("Extesion") != headNames.end();

                        bool hasLongitude = headNames.find("Longitude") != headNames.end();
                        bool hasLatitude = headNames.find("Latitude") != headNames.end();
                        bool hasHeight = headNames.find("Height") != headNames.end();

                        bool hasEasting = headNames.find("Easting") != headNames.end();
                        bool hasNorthing = headNames.find("Northing") != headNames.end();

                        bool hasOmega = headNames.find("Omega") != headNames.end();
                        bool hasPhi = headNames.find("Phi") != headNames.end();
                        bool hasKappa = headNames.find("Kappa") != headNames.end();

                        bool hasHeading = headNames.find("Heading") != headNames.end();
                        bool hasPitch = headNames.find("Pitch") != headNames.end();
                        bool hasRoll = headNames.find("Roll") != headNames.end();

                        bool hasYaw = headNames.find("Yaw") != headNames.end();

                        bool hasRotation00 = headNames.find("Rotation00") != headNames.end();
                        bool hasRotation01 = headNames.find("Rotation01") != headNames.end();
                        bool hasRotation02 = headNames.find("Rotation02") != headNames.end();
                        bool hasRotation10 = headNames.find("Rotation10") != headNames.end();
                        bool hasRotation11 = headNames.find("Rotation11") != headNames.end();
                        bool hasRotation12 = headNames.find("Rotation12") != headNames.end();
                        bool hasRotation20 = headNames.find("Rotation20") != headNames.end();
                        bool hasRotation21 = headNames.find("Rotation21") != headNames.end();
                        bool hasRotation22 = headNames.find("Rotation22") != headNames.end();

                        
                        if (hasLongitude && hasLatitude)
                        {
                            if (hasEasting || hasNorthing)
                            {
                                LOGE("Either LLH or ENH!");
                                return false;
                            }
                            if (!hasHeight)
                            {
                                LOGE("No Height in LLH(Longitude Latitude Height)!");
                                return false;
                            }
                        }
                        if (hasEasting && hasNorthing)
                        {
                            if (hasLongitude || hasLatitude)
                            {
                                LOGE("Either LLH or ENH");
                                return false;
                            }
                            if (!hasHeight)
                            {
                                LOGE("No Height in ENH(Easting Northing Height)");
                                return false;
                            }
                        }

                        int img_index = 0;
                        for (int i_row = 1; i_row < rows; i_row++)
                        {
                            Image img;
                            img.SetImageId(img_index);
                            
                            std::string name = xlrange[i_row][headNames["Name"]].to_string();
                            if (name.empty())
                            {
                                continue;
                            }
                            img.SetName(name);

                            if (hasDirectory)
                            {
                                std::string path = xlrange[i_row][headNames["Directory"]].to_string();
                                img.SetPath(path);
                            }

                            
                            std::string photogroupName = xlrange[i_row][headNames["PhotogroupName"]].to_string();
                            if (photogroupName.empty())
                            {
                                LOGE("Empty photogroupName");
                                return false;
                            }
                            auto pos = std::find_if(photogroups_.begin(), photogroups_.end(), [photogroupName](const std::pair<group_t, PhotoGroup>& pg) {
                                return std::strcmp(photogroupName.c_str(), pg.second.GetName().c_str()) == 0; });
                            pos->second.AddImageId(img_index);
                            img.SetPhotoGroupID(pos->first);
                            img.SetCameraId(pos->first);
                            img.SetHeight(pos->second.GetCamera().GetHeight());
                            img.SetWidth(pos->second.GetCamera().GetWidth());

                            if (hasLongitude && hasLatitude && hasHeight)
                            {
                                Eigen::Vector3d position;
                                std::string x = xlrange[i_row][headNames["Longitude"]].to_string();
                                position[0] = std::atof(x.c_str());

                                std::string y = xlrange[i_row][headNames["Latitude"]].to_string();
                                position[1] = std::atof(y.c_str());

                                std::string z = xlrange[i_row][headNames["Height"]].to_string();
                                position[2] = std::atof(z.c_str());
                                img.SetPosition(position);
                            }

                            if (hasEasting && hasNorthing && hasHeight)
                            {
                                Eigen::Vector3d position = Eigen::Vector3d::Zero();
                                std::string easting = xlrange[i_row][headNames["Easting"]].to_string();
                                position[0] = std::atof(easting.c_str());

                                std::string northing = xlrange[i_row][headNames["Northing"]].to_string();
                                position[1] = std::atof(northing.c_str());

                                std::string height = xlrange[i_row][headNames["Height"]].to_string();
                                position[2] = std::atof(height.c_str());
                                img.SetPosition(position);
                            }

                            
                            if (hasRotation00 && hasRotation01 && hasRotation02 && hasRotation10 && \
                                hasRotation11 && hasRotation12 && hasRotation20 && hasRotation21 && hasRotation22)
                            {
                                Eigen::Matrix3d R = Eigen::Matrix3d::Zero();
                                for (int i = 0; i < 3; i++)
                                {
                                    for (int j = 0; j < 3; j++)
                                    {
                                        std::string Rotation = "Rotation" + std::to_string(i) + std::to_string(j);
                                        std::string R_Tmp = xlrange[i_row][headNames[Rotation]].to_string();
                                        R(i, j) = std::atof(R_Tmp.c_str());
                                    }
                                }
                                img.SetRotationMatrix(R);
                            }
                            ATdata->AddImage(img);
                            img_index++;
                        }
                    }
                    
                    if (std::strcmp(ws.title().c_str(), "ControlPoints") == 0)
                    {
                        std::map<std::string, uint8_t> headNames;

                        for (uint8_t i_col = 0; i_col < columns; i_col++)
                        {
                            std::string headTitles = xlrange[0][i_col].value<std::string>();
                            headNames.insert(std::make_pair(headTitles, i_col));
                        }
                        bool hasName = headNames.find("Name") != headNames.end();
                        bool hasLongitude = headNames.find("Longitude") != headNames.end();
                        bool hasLatitude = headNames.find("Latitude") != headNames.end();
                        bool hasEasting = headNames.find("Easting") != headNames.end();
                        bool hasNorthing = headNames.find("Northing") != headNames.end();
                        bool hasHeight = headNames.find("Height") != headNames.end();

                        if (hasName)
                        {
                            EIGEN_STL_UMAP(point3D_t,ControlPoint) cps;
                            int index_cp = 0;
                            for (int i_row = 1; i_row < rows; i_row++)
                            {
                                ControlPoint cp;
                                std::string name = xlrange[i_row][headNames["Name"]].to_string();
                                if (name.empty())
                                {
                                    continue;
                                }
                                cp.SetName(name);
                                cp.SetId(index_cp);
                                
                                if (hasEasting && hasNorthing && hasHeight)
                                {
                                    Eigen::Vector3d gcp_pos;
                                    std::string easting = xlrange[i_row][headNames["Easting"]].to_string();
                                    gcp_pos[0] = std::atof(easting.c_str());

                                    std::string northing = xlrange[i_row][headNames["Northing"]].to_string();
                                    gcp_pos[1] = std::atof(northing.c_str());

                                    std::string height = xlrange[i_row][headNames["Height"]].to_string();
                                    gcp_pos[2] = std::atof(height.c_str());
                                    cp.SetGivenXYZ(gcp_pos);
                                }
                                if (hasLongitude && hasLatitude && hasHeight)
                                {
                                    Eigen::Vector3d gcp_pos;
                                    std::string lon = xlrange[i_row][headNames["Longitude"]].to_string();
                                    gcp_pos[0] = std::atof(lon.c_str());

                                    std::string lat = xlrange[i_row][headNames["Latitude"]].to_string();
                                    gcp_pos[1] = std::atof(lat.c_str());

                                    std::string height = xlrange[i_row][headNames["Height"]].to_string();
                                    gcp_pos[2] = std::atof(height.c_str());
                                    cp.SetGivenXYZ(gcp_pos);
                                }
                                cps.insert(std::make_pair(index_cp, cp));
                                index_cp++;
                            }
                            ATdata->SetControlPoints(cps);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    
                    if (std::strcmp(ws.title().c_str(), "Options") == 0)
                    {
                        std::map<std::string, uint8_t> headNames;

                        for (uint8_t i_row = 0; i_row < rows; i_row++)
                        {
                            std::string colName = xlrange[i_row][0].value<std::string>();
                            headNames.insert(std::make_pair(colName, i_row));
                        }



                        std::string SRS = xlrange[headNames["SRS"]][1].value<std::string>();
                        srs_s blockSRS;
                        blockSRS.definition = GEO84SRS;
                        if (!SRS.empty())
                        {
                            
#ifdef USE_AI3D_PROJ
                            
                        
                            AI3D::PROJ::CoordinateReferenceSystem crs(SRS);


                            srs_s newsrs;
                            newsrs.definition = crs.authid().toStdString();
                            newsrs.type = crs.GetType();
                            newsrs.name = crs.GetDescription();
                            blockSRS.definition = newsrs.definition;

#else
                            blockSRS.definition = SRS;
#endif 

                            
                        }
                        blockSRS.ID = GenerateValidSrsId();
                        auto tmpSRS = CoordinateDescriptor::GetSRSFromDefinition(blockSRS.definition);
                        blockSRS.name = tmpSRS.name;
                        blockSRS.type = tmpSRS.type;
                        srs_map_.insert(std::make_pair(blockSRS.ID, blockSRS));
                        blockSRS_id_ = blockSRS.ID;
                    }
                }

                
                if (!ATdata->GetControlPoints().empty())
                {
                    for (auto& cp : ATdata->GetControlPointsMutual())
                    {
                        cp.second.SetSrs(srs_map_[blockSRS_id_]);
                    }
                }

                
                std::string dst_definition = BASESRS;
                srs_s origin_srs = srs_map_[blockSRS_id_];
                if (origin_srs.type == coord_system_type_e::LOCAL_ENU)
                {
                    srs_enu_discription_ = origin_srs;
                }
                ATdata->SetOriginSrs(origin_srs.definition);

                
                if (origin_srs.type == GEOGRAPHIC)
                {
                    ATdata->TransFormImages(origin_srs.definition, dst_definition);
                    ATdata->TransFormTiepoints(origin_srs.definition, dst_definition);
                }
                else
                {
                    dst_definition = origin_srs.definition;
                }
                ATdata->TransformControlPoints(dst_definition);
                ATdata->SetLocalGcpSrs(dst_definition);
                ATdata->SetLocalSrs(dst_definition);
                UpdateSRSMap(CoordinateDescriptor::GetSRSFromDefinition(dst_definition));
                blockSRS_id_ = ExistSRS(dst_definition);
            }
            catch(const std::exception&err)
            {
                LOGE(err.what());
                return false;
            }
            return true;
        }


        bool BlockObject::ExportXLSX(const std::string& xlsx_file_path,const std::string& srsdef, bool InRadians)
        {
            try
            {
                if (GetCurrentAT() == nullptr)
                {
                    return false;
                }
                auto Atdata = GetCurrentAT();
                xlnt::workbook wb;
                
                wb.create_sheet(0).title("Photogroups");
                const auto& ws_photogroups = wb.sheet_by_index(0);
                auto xlRange = ws_photogroups.rows();
                xlRange[0][0].value("Name");
                xlRange[0][1].value("Width");
                xlRange[0][2].value("Height");
                xlRange[0][3].value("FocalLength");
                xlRange[0][4].value("SensorSize");
                xlRange[0][5].value("PixelSize");
                xlRange[0][6].value("PrincipalPointX");
                xlRange[0][7].value("PrincipalPointY");
                xlRange[0][8].value("CameraOrientation");
                

                int i_row = 1;
                for (auto itr_pg = photogroups_.begin(); itr_pg !=photogroups_.end() ; ++itr_pg)
                {
                    Camera cam = itr_pg->second.GetCamera();
                    xlRange[i_row][0].value(cam.GetCameraName().c_str());
                    xlRange[i_row][1].value(cam.GetWidth());
                    xlRange[i_row][2].value(cam.GetHeight());
                    xlRange[i_row][3].value(std::to_string(cam.GetFocalLengthMM()));
                    xlRange[i_row][4].value(std::to_string(cam.GetSensorSize()));
                    xlRange[i_row][5].value(std::to_string(cam.GetSensorSize() / std::max(cam.GetHeight(), cam.GetWidth())));
                    xlRange[i_row][6].value(cam.GetPrincipalPointX());
                    xlRange[i_row][7].value(cam.GetPrincipalPointY());
                    xlRange[i_row][8].value(cam.GetCameraOrientation());
                    i_row++;
                }
                
                wb.create_sheet(1).title("Photos");
                const auto& ws_photos = wb.sheet_by_index(1);
                auto xlRangePhoto = ws_photos.rows();
                xlRangePhoto[0][0].value("Name");
                xlRangePhoto[0][1].value("PhotogroupName");
                xlRangePhoto[0][2].value("Directory");
                xlRangePhoto[0][3].value("Longitude");
                xlRangePhoto[0][4].value("Latitude");
                xlRangePhoto[0][5].value("Height");
                if (Atdata->GetImages().begin()->second.HasRotationMatrix())
                {
                    xlRangePhoto[0][6].value("Rotation00");
                    xlRangePhoto[0][7].value("Rotation01");
                    xlRangePhoto[0][8].value("Rotation02");
                    xlRangePhoto[0][9].value("Rotation10");
                    xlRangePhoto[0][10].value("Rotation11");
                    xlRangePhoto[0][11].value("Rotation12");
                    xlRangePhoto[0][12].value("Rotation20");
                    xlRangePhoto[0][13].value("Rotation21");
                    xlRangePhoto[0][14].value("Rotation22");
                }
                i_row = 1;
                Atdata->TransFormImages(Atdata->GetLocalSrs(), srsdef);
                std::string dstdef = srsdef;
                Atdata->TransformControlPoints(dstdef);
                Atdata->SetLocalGcpSrs(dstdef);
                for (auto itr_photo = Atdata->GetImages().begin(); itr_photo != Atdata->GetImages().end(); ++itr_photo)
                {
                    Image img = itr_photo->second;
                    xlRangePhoto[i_row][0].value(img.GetName());
                    xlRangePhoto[i_row][1].value(photogroups_[img.GetPhotoGroupID()].GetName());
                    xlRangePhoto[i_row][2].value(img.GetPath());
                    xlRangePhoto[i_row][3].value(img.GetPosition().x());
                    xlRangePhoto[i_row][4].value(img.GetPosition().y());
                    xlRangePhoto[i_row][5].value(img.GetPosition().z());
                    if (img.HasRotationMatrix())
                    {
                        int count = 6;
                        for (int i = 0; i < 3; ++i)
                        {
                            for (int j = 0; j < 3; ++j)
                            {
                                xlRangePhoto[i_row][count].value(img.GetRotationMatrix()(i, j));
                                count++;
                            }
                        }
                    }
                    i_row++;
                }
                
                auto ws_gcp = wb.create_sheet(2);
                ws_gcp.title("ControlPOints");
                if (Atdata->HasControlPoints())
                {
                    auto xlRangeGCP = ws_gcp.rows();
                    xlRangeGCP[0][0].value("Name");
                    xlRangeGCP[0][1].value("Longitude");
                    xlRangeGCP[0][2].value("Latitude");
                    xlRangeGCP[0][3].value("Height");
                    i_row = 1;
                    for (auto itr_gcp = Atdata->GetControlPoints().begin(); itr_gcp != Atdata->GetControlPoints().end(); ++itr_gcp)
                    {
                        xlRangeGCP[i_row][0].value(itr_gcp->second.GetName());
                        xlRangeGCP[i_row][1].value(itr_gcp->second.GetObjectPoint().GetXYZ().x());
                        xlRangeGCP[i_row][2].value(itr_gcp->second.GetObjectPoint().GetXYZ().y());
                        xlRangeGCP[i_row][3].value(itr_gcp->second.GetObjectPoint().GetXYZ().z());
                        i_row++;
                    }
                }
                
                auto ws_options = wb.create_sheet(3);
                ws_options.title("Options");
                auto xlRangeOptions = ws_options.rows();
                xlRangeOptions[0][0].value("OptionName");
                xlRangeOptions[0][1].value("Value");
                xlRangeOptions[1][0].value("SRS");
                xlRangeOptions[1][1].value(srsdef);
                xlRangeOptions[2][0].value("InRadians");
                xlRangeOptions[2][3].value(InRadians);
                xlRangeOptions[3][0].value("BaseImagePath");
                xlRangeOptions[3][1].value(File::GetParentDir(Atdata->GetImages().begin()->second.GetPath()));
                xlRangeOptions[4][0].value("BlockType");
                xlRangeOptions[4][1].value("Aerial");

                wb.save(xlsx_file_path);
            }
            catch (const std::exception& error)
            {
                LOGE(error.what());
                return false;
            }
            return true;
        }

        
        
        bool BlockObject::LoadPoseTxt(const std::string& postxt_path, std::vector<pose_s>& poses)
        {
            std::ifstream in = File::OpenIfstreamUtf8(postxt_path, std::ios::in);
            try
            {
                if (!in.is_open())
                    return false;
                std::string line;
                
                std::string item;
                std::string name;
                double x, y, z;
                while (std::getline(in, line))
                {
                    if (line.empty())
                    {
                        continue;
                    }
                    std::stringstream ss;
                    name.clear();
                    ss.clear();
                    
                    ss.str(line);
                    ss >> name >> x >> y >> z;
                    pose_s poseinfo;
                    poseinfo.name = name;
                    poseinfo.metadata_.center.x() = x;
                    poseinfo.metadata_.center.y() = y;
                    poseinfo.metadata_.center.z() = z;
                    poses.push_back(poseinfo);
                }
                in.close();
            }
            catch (const std::exception& err)
            {
                LOGE(err.what());
                in.close();
                return false;
            }
            return true;
        }
        bool BlockObject::LoadPoseXLSX(const std::string& posxlsx_path, std::vector<pose_s>&poses)
        {
            try
            {
                 
                xlnt::workbook wb;
                wb.load(posxlsx_path);

                xlnt::worksheet ws = wb.active_sheet();
                auto range = ws.calculate_dimension();
                int rows = range.height();
                int columns = range.width();
                xlnt::range xlrange = ws.rows();
                for (int i_row = 0; i_row < rows; i_row++)
                {
                    std::string name = xlrange[i_row][0].to_string();
                    std::string x = xlrange[i_row][1].to_string();
                    std::string y = xlrange[i_row][2].to_string();
                    std::string z = xlrange[i_row][3].to_string();

                    if (name.empty()||x.empty()||y.empty()||z.empty())
                    {
                        continue;
                    }
                    pose_s poseinfo;
                    poseinfo.name = name;
                    poseinfo.metadata_.center.x() = std::atof(x.c_str());
                    poseinfo.metadata_.center.y() = std::atof(y.c_str());
                    poseinfo.metadata_.center.z() = std::atof(z.c_str());
                    poses.push_back(poseinfo);
                }
            }
            catch (const std::exception& error)
            {
                LOGE(error.what());
                return false;
            }
            return true;
        }

        
        bool BlockObject::ImagesRename(const std::string& img_dir_path, std::map<std::string, std::string>& image_old_to_new_name, int* processValue, const std::string& pref,uint8_t numLen, int begin)
        {
            *processValue = 0;

            try
            {
                if (!std::filesystem::is_directory(File::BoostPathFromUtf8(img_dir_path)))
                {

                    std::ostringstream oss;
                    oss << "==== Data Preprocess failed:Invalid directory path " << img_dir_path;
                    LOGE(oss.str());
                    *processValue = -1;
                    return false;
                }

            

                
                std::map<std::string, std::vector<std::string>>DirectorFilesMap;
                const std::filesystem::path path = File::BoostPathFromUtf8(img_dir_path);
                std::filesystem::recursive_directory_iterator end_itr;
                

                for (std::filesystem::recursive_directory_iterator itr(path); itr != end_itr; ++itr)
                {
                

                    if (std::filesystem::is_directory(itr->path()))
                    {
                        std::string filepath = File::BoostPathToUtf8String(itr->path());

                        std::vector<std::string> extensions = { ".arw" , "*.raw" , "*.rw2" , ".jpg" , ".jpeg" , ".png" , ".tiff" , ".tif" };
                        std::vector<std::string> filenames;
                        SearchImages(filepath, filenames, extensions, false);
                        if (!filenames.empty())
                        {
                        
                            DirectorFilesMap.insert(std::make_pair(File::GetDirName(filepath), filenames));
                        }
                        else
                        {
                        
                        }
                    }
                    else
                    {
                    
                    }
                }

                

                if (DirectorFilesMap.empty())
                {
                    *processValue = -1;
                    std::ostringstream oss;
                    oss << "==== Data Preprocess failed:Photos path(" << img_dir_path << ") has some errors inside it. =====";
                    LOGI(oss.str());

                    return false;
                }

                
                *processValue = 20;

                int filecount = DirectorFilesMap.size();
                if (filecount > 0)
                {
                    int PercentageOfPerPair = 80 / filecount;
                    
                    for (const auto& dirfiles : DirectorFilesMap)
                    {
                        begin = 1;
                        *processValue = *processValue + PercentageOfPerPair;
                        std::string FirstCharInDir = dirfiles.first;
                        FirstCharInDir = FirstCharInDir.substr(0, 1);

                        for (const auto& imgpath : dirfiles.second)
                        {
                            std::stringstream osr;
                            osr << std::setfill('0') << std::setw(numLen) << begin;
                            begin++;
                            std::string oldName = imgpath;
                            std::string newName = File::GetParentDir(imgpath) + PATH_SEPARATOR_STR + pref + FirstCharInDir + osr.str() + File::BoostPathToUtf8String(File::BoostPathFromUtf8(imgpath).extension());
                            std::filesystem::rename(File::BoostPathFromUtf8(oldName), File::BoostPathFromUtf8(newName));
                            std::string oldimagename = File::GetFileNameWithoutExtension(oldName);
                            
                            std::string newimagename = File::GetFileNameWithoutExtension(newName);
                            
                            image_old_to_new_name[oldimagename] = newimagename;
                            
                        }
                    }
                }

                *processValue = 100;
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "==== Data Preprocess failed:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                *processValue = -1;
                return false;
            }
            catch (const std::exception& error)
            {
                std::ostringstream oss;
                oss << "==== Data Preprocess failed:" << error.what();

                LOGE(oss.str());
                *processValue = -1;
                return false;
            }
            return true;
        }

        bool BlockObject::IsEmpty()
        {
            return GetCurrentAT()->IsEmpty();
        }
        int BlockObject::BatchPrePare(std::string configfile, int* processValue)
        {
            
            
            preparetaskinfo_s info;
            info.load(configfile);
            std::string img_dir_path = info.ImagePath;
            std::string  pref = info.Prefix;
            srs_s blocksrs= CoordinateDescriptor::GetSRSFromDefinition(info.SRS);
            std::string gcp_path= info.GcpPath;
            std::string posfilepath = info.PosfilePath;
            uint8_t numLen = info.NumLength; int begin = info.NumStart;

            return BatchPreProcess(img_dir_path, processValue, pref, blocksrs, gcp_path, posfilepath, numLen, begin);
        }

        
        
        bool BlockObject::BatchPreProcess(const std::string& img_dir_path, int* processValue,
            const std::string& pref, const srs_s& blocksrs, const std::string& gcp_path,std::string posfilepath,uint8_t numLen , int begin)
        {
            
            if (!processValue)
                return false;

            *processValue = 0;
            bool brenameimg = true;
            int* imgRenameProcessValue = new int(0);
            int lastValue = 0;
            if (pref == "" && numLen == 0 && begin == 0)
            {
                brenameimg = false;
            }
            std::map<std::string, std::string> image_old_to_new_name;
            if (brenameimg)
            {
                
                
                std::thread imagesRenameThread(std::bind(&BlockObject::ImagesRename, this, img_dir_path, image_old_to_new_name, std::ref(imgRenameProcessValue), pref, numLen, begin));
                imagesRenameThread.detach();
                
                while (true)
                {
                    if (lastValue != *imgRenameProcessValue)
                    {
                        lastValue = *imgRenameProcessValue;
                        LOGI(*processValue);
                    }
                    *processValue = *(imgRenameProcessValue) * 20 / 100;
                    if (*imgRenameProcessValue == 100)
                    {
                        *processValue = *(imgRenameProcessValue) * 20 / 100;
                        LOGI("Renaming Images successfully");
                        break;
                    }
                }
            }
            
            
            std::vector<std::string> imgfilenames;
            std::vector<std::string> extension = { ".arw" , "*.raw" , "*.rw2" , ".jpg" , ".jpeg" , ".png" , ".tiff" , ".tif" };
            SearchImages(img_dir_path,imgfilenames, extension);
            
            
            
            
            
            
            
            

            *imgRenameProcessValue = 0;
            lastValue = 0;
            
            bool* bCancel = new bool;
            *bCancel = false;
            std::thread addImagesThread(std::bind(&BlockObject::Addimages_Beta,this, imgfilenames, std::ref(imgRenameProcessValue),std::ref(bCancel)));
            addImagesThread.detach();
            while (true)
            {
                if (lastValue != *imgRenameProcessValue)
                {
                    *processValue = (*(imgRenameProcessValue) * 50 / 100 + 20);
                    LOGI(*processValue);
                }
                if (*imgRenameProcessValue == 100)
                {
                    *processValue = (*(imgRenameProcessValue) * 50 / 100 + 20);
                    LOGI("Add Images successfully");
                    break;
                }
                lastValue = *imgRenameProcessValue;
            }

            
            

            if (imgfilenames.empty())
            {
                LOGI("Add Images failed");
                return false;
            }

            



            bool  baddpose = false;
            if (posfilepath != "" )
            {
                baddpose = true;
            }
            if (baddpose)
            {
                
                
                if (File::IsFileExistent(posfilepath))
                {
                    std::vector<pose_s> poses;
                    bool bLoadError = false;
                    try
                    {
                        if (File::BoostPathToUtf8String(File::BoostPathFromUtf8(posfilepath).extension()) == ".txt")
                        {
                            if (!LoadPoseTxt(posfilepath, poses))
                            {
                                LOGE("Load pose files error!");
                                bLoadError = true;
                                
                            }
                        }
                        else if (File::BoostPathToUtf8String(File::BoostPathFromUtf8(posfilepath).extension()) == ".xlsx")
                        {
                            if (!Allow2LoadXLS())
                            {
                                bLoadError = true;
                            }
                            else if (!LoadPoseXLSX(posfilepath, poses))
                            {
                                LOGE("Load pose files error!");
                                bLoadError = true;
                                
                            }
                        }
                    }
                    catch (std::filesystem::filesystem_error& fse)
                    {
                        std::ostringstream oss;
                        oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                        LOGI(oss.str());
                        bLoadError = true;
                        
                    }
                    catch (std::exception& ex)
                    {
                        std::ostringstream oss;
                        oss << "exception:" << ex.what();
                        LOGI(oss.str());
                        bLoadError = true;
                        
                    }

                    
                    if (!bLoadError)
                    {
                        
                        AddPoses(blocksrs, poses);
                    }
                }
            }
            
            *processValue = 90;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            bool  baddgcp = false;
            if (gcp_path != "" )
            {
                baddgcp = true;
            }
            if (baddgcp)
            {
                
                if (File::IsFileExistent(gcp_path))
                {
                    ControlPoints cps;
                    bool bLoadError = false;
                    if (!cps.LoadText(gcp_path))
                    {
                        LOGE("Load GCP file error!");
                        bLoadError = true;
                        
                    }

                    if(!bLoadError)
                        GetCurrentATMutual()->SetControlPoints(cps.GetPoints());
                }
            }
            BlockExportOptions opt;

            std::string outfile = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(path_)) + "preprocess.xml";
            ExportATXML(outfile, opt);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            *processValue = 100;
            
            block_info_.isSaved = false;
            Save();
            return true;
        }


        bool BlockObject::BatchPreProcessPosId(DataPreprocessOption& option, std::string& generated_xml, const std::string& img_dir_path, int* processValue,
            const std::string& pref, const srs_s& blocksrs, const std::string& gcp_path,
            std::string posfilepath, uint8_t numLen, int begin, progFunc funcPtr, void* thatObj, int taskId)
        {

            
            std::map<std::string, std::set<std::string >> images_groupbydir;
            
            bool is_subdir_fit_policy;
            
            bool is_dirname_fit_policy;
            
            int subdircnt = images_groupbydir.size();
            std::map<std::string, std::string> firstimage_eachdir;
            
            
            
            return true;

        }

        bool BlockObject::BatchPreProcess(DataPreprocessOption &option,std::string& generated_xml,const std::string& img_dir_path, int* processValue,
            const std::string& pref, const srs_s& blocksrs, const std::string& gcp_path, std::string posfilepath, uint8_t numLen, int begin, progFunc funcPtr,void *thatObj,int taskId)
        {
            
            if (!processValue)
                return false;

            *processValue = 0;
            bool brenameimg = true;
            int* imgRenameProcessValue = new int(0);
            int lastValue = 0;
            if (pref == "" && numLen == 0 && begin == 0)
            {
                brenameimg = false;
            }
            std::map<std::string, std::string> image_old_to_new_name;
            
            if (brenameimg)
            {
                

                std::thread imagesRenameThread(std::bind(&BlockObject::ImagesRename, this, img_dir_path, std::ref(image_old_to_new_name), std::ref(imgRenameProcessValue), pref, numLen, begin));
                imagesRenameThread.detach();

                while (true)
                {
                    if (*imgRenameProcessValue == -1)
                    {
                        
                        return false;
                    }

                    if (lastValue != *imgRenameProcessValue)
                    {
                        lastValue = *imgRenameProcessValue;
                        LOGI(*processValue);

                        if (funcPtr && thatObj)
                            funcPtr(thatObj, taskId, *processValue);
                    }
                    *processValue = *(imgRenameProcessValue) * 20 / 100;
                    if (*imgRenameProcessValue == 100)
                    {
                        *processValue = *(imgRenameProcessValue) * 20 / 100;
                        if (funcPtr && thatObj)
                            funcPtr(thatObj, taskId, *processValue);

                        LOGI("Renaming Images successfully");
                        break;
                    }

                    
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            
            std::vector<std::string> imgfilenames;
            std::vector<std::string> extension = { ".arw" , "*.raw" , "*.rw2" , ".jpg" , ".jpeg" , ".png" , ".tiff" , ".tif"};
            SearchImages(img_dir_path, imgfilenames, extension);
            *imgRenameProcessValue = 0;
            lastValue = 0;
            
            bool* bCancel = new bool;
            *bCancel = false;

            int* imgRenameProcessValue2 = new int(0);
            *imgRenameProcessValue2 = 0;

            std::thread addImagesThread(std::bind(&BlockObject::Addimages2_Beta, this, imgfilenames, std::ref(imgRenameProcessValue2), std::ref(bCancel)));
            addImagesThread.detach();
            while (true)
            {
                
                if (lastValue != *imgRenameProcessValue2)
                {
                    *processValue = (*(imgRenameProcessValue2) * 50 / 100 + 20);
                    LOGI(*processValue);
                    if (funcPtr && thatObj)
                        funcPtr(thatObj, taskId, *processValue);
                }
                if (*imgRenameProcessValue2 == 100)
                {
                    *processValue = (*(imgRenameProcessValue2) * 50 / 100 + 20);
                    if (funcPtr && thatObj)
                        funcPtr(thatObj, taskId, *processValue);

                    LOGI("Add Images successfully");
                    break;
                }

                lastValue = *imgRenameProcessValue2;
            }

            
            

            std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;

            if (imgfilenames.empty())
            {
                LOGI("Add Images failed");
                std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            
            {
                
                
                for (auto& itergroup : photogroups_)
                {
                    std::string basestr = itergroup.second.GetName();
                    const char* tofindstr = "0";
                    UpdateCameraInfo(cam_para_e::SENSOR_SIZE, 23.4, itergroup.second.GetId());
                    if (strstr(basestr.c_str(), tofindstr) != NULL)
                    {
                        
                        UpdateCameraInfo(cam_para_e::FOCAL, option.centreFocalLength, itergroup.second.GetId());
                    }
                    else
                    {
                        UpdateCameraInfo(cam_para_e::FOCAL, option.obliqueFocalLength, itergroup.second.GetId());
                        
                    }
                    
                    
                }

            }


            bool  baddpose = false;
            if (posfilepath != "")
            {
                baddpose = true;
            }
            if (baddpose)
            {
                
                
                if (File::IsFileExistent(posfilepath))
                {
                    std::vector<pose_s> poses;
                    bool bLoadError = false;
                    try
                    {
                        if (File::BoostPathToUtf8String(File::BoostPathFromUtf8(posfilepath).extension()) == ".txt")
                        {
                            
                        
                            if (!LoadPoseTxt(posfilepath, poses))
                            {
                                LOGE("Load pose files error!");
                                bLoadError = true;
                                
                            }
                        }
                        else if (File::BoostPathToUtf8String(File::BoostPathFromUtf8(posfilepath).extension()) == ".xlsx")
                        {
                            if (!Allow2LoadXLS())
                            {
                                bLoadError = true;
                            }
                            else if (!LoadPoseXLSX(posfilepath, poses))
                            {
                                LOGE("Load pose files error!");
                                bLoadError = true;
                                
                            }
                        }
                    }
                    catch (std::filesystem::filesystem_error& fse)
                    {
                        std::ostringstream oss;
                        oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                        LOGI(oss.str());
                        bLoadError = true;
                        
                    }
                    catch (std::exception& ex)
                    {
                        std::ostringstream oss;
                        oss << "exception:" << ex.what();
                        LOGI(oss.str());
                        bLoadError = true;
                        
                    }

                    
                    if (!bLoadError)
                    {
                        
                        if (brenameimg)
                        {
                            for (auto& iter : poses)
                            {
                                std::string shortname = iter.name;
                                std::string shortnamewithoutext = File::GetFileNameWithoutExtension(shortname);
                                
                                if (image_old_to_new_name.count(shortnamewithoutext))
                                {
                                    iter.name = String::StringReplace(iter.name, shortnamewithoutext, image_old_to_new_name.at(shortnamewithoutext));

                                }
                            }
                        }
                        AddPoses(blocksrs, poses);
                    }
                }
            }
            
            std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;

            
            *processValue = 90;
            if (funcPtr && thatObj)
                funcPtr(thatObj, taskId, *processValue);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            bool  baddgcp = false;
            if (gcp_path != "")
            {
                baddgcp = true;
            }
            if (baddgcp)
            {
                
                if (File::IsFileExistent(gcp_path))
                {
                    ControlPoints cps;
                    bool bLoadError = false;
                    if (!cps.LoadText(gcp_path))
                    {
                        LOGE("Load GCP file error!");
                        bLoadError = true;
                        
                    }

                    if (!bLoadError)
                        GetCurrentATMutual()->SetControlPoints(cps.GetPoints());
                }
            }

            BlockExportOptions opt;
#ifdef USE_AI3D_PROJ
            opt.srs_ = CoordinateDescriptor::GetSRSFromDefinition(option.SRS);
#else
            opt.srs_ = CoordinateDescriptor::GetSRSFromName(option.SRS);
#endif
            opt.srs_.ID = ExistSRS(opt.srs_.definition);
            if (opt.srs_.ID == kInvalidSrsId)
            {
                UpdateSRSMap(opt.srs_);
                opt.srs_.ID = ExistSRS(opt.srs_.definition);
            }
            std::string outfile;

            std::string outfile_filename;
            std::string outfile_path;

            
            std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (!option.exportFileName.empty())
                outfile_filename = option.exportFileName;
            else
                outfile_filename = "preprocess.xml";

            if(!option.exportDirectory.empty())
                outfile = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(option.exportDirectory)) + outfile_filename;
            else
                outfile = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(path_)) + outfile_filename;

            std::cout << "export:" << outfile;

            std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
            ExportATXML(outfile, opt);

            *processValue = 100;
            if (funcPtr && thatObj)
                funcPtr(thatObj, taskId, *processValue);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
            



            generated_xml = outfile;

            return true;
        }

        bool  BlockObject::LoadATXML(const std::string& xml_file_path, std::shared_ptr<ATData> ATdata, bool transformTobase, bool bParseTiept)
        {
            
            std::vector<PhotoGroup>pg;
            std::set<image_t> images_pg;
            
            pugi::xml_document doc;
            LOGI("*********************load_file************************");
            if (doc.load_file(xml_file_path.c_str()).status != pugi::xml_parse_status::status_ok)
            {
                LOGE(String::StringPrintf("Load XML file: %s error!", xml_file_path.c_str()));
                return false;
            }
            
            if (!doc.child("BlocksExchange") || !doc.child("BlocksExchange").child("Block"))
            {
                LOGE("No BlocksExchange Root or No Block!");
                return false;
            }

            
            pugi::xml_node block = doc.child("BlocksExchange").child("Block");
            if (!block)
            {
                LOGE("No Block Element!");
                return false;
            }
            std::set<srsid_t> srs_used_ids;
            
            
            EIGEN_STL_UMAP(srsid_t, srs_s) srs_map;
            pugi::xml_node srss = doc.child("BlocksExchange").child("SpatialReferenceSystems");
            if (!srss.children("SRS").empty())
            {
                if (!ParseSRS(srs_map, srss))
                {
                    return false;
                }
            }
            
            for (const auto& srs : srs_map)
            {
                UpdateSRSMap(srs.second);
            }

            
            if (block.child("Name") )
            {
                name_ = block.child("Name").text().as_string();
                // name_ = UTF82GBK(name_);
            }
            
            
            if ( !srss.children("SRS").empty())
            {
                if (block.child("SRSId"))
                {
                    blockSRS_id_ = block.child("SRSId").text().as_int();
                    if (srs_map.find(blockSRS_id_) == srs_map.end())
                    {
                        LOGE(String::StringPrintf("Bad block: unknown SRS id in:\n <Name> = %s\n<SRSId> = %s", name_.c_str(), blockSRS_id_));
                        return false;
                    }
                    else
                    {
                        blockSRS_id_ = ExistSRS(srs_map.at(blockSRS_id_).definition);
                        srs_used_ids.insert(blockSRS_id_);
                    }
                }
                
            }
            

            
            if (block.child("Description"))
            {
                description_ = block.child("Description").text().as_string();
            }
            
            
            LOGI("*********************ParsePhotoGroups************************");
            pugi::xml_node photogroups = block.child("Photogroups");
            
            if (photogroups)
            {
                bool result = ParsePhotoGroups(ATdata, pg, images_pg, photogroups, srs_map, srs_used_ids);
                if (!result)
                {
                    return false;
                }
            }

            
            LOGI("*********************ParseControlPoints************************");
            pugi::xml_node controlpoints = block.child("ControlPoints");
            
            if (!controlpoints.children("ControlPoint").empty())
            {
                EIGEN_STL_UMAP(point3D_t, ControlPoint)cps_map;
                bool result = ParseControlPoints(ATdata, cps_map, images_pg, controlpoints, srs_map, srs_used_ids);
                if (!result)
                {
                    return false;
                }
                ATdata->SetControlPoints(cps_map);
            }
            if (bParseTiept)
            {
                LOGI("*********************ParseTiePoints************************");
                
                pugi::xml_node tiepoints = block.child("TiePoints");
                if (!tiepoints.children("TiePoint").empty())
                {
                    EIGEN_STL_UMAP(point3D_t, Point3D) tps, usertiepts;
                    bool result = ParseTiePoints(ATdata, tps, images_pg, tiepoints, usertiepts);
                    if (!result)
                    {
                        return false;
                    }
                    if (!tps.empty())
                    {
                        ATdata->SetPoint3D(tps);
                    }
                    if (!usertiepts.empty())
                    {
                        ATdata->SetUserPoint3D(usertiepts);
                    }
                }
            }
            
            std::set<camera_t> camids_added;
            for (auto& it : pg)
            {
                
                bool IsContain = false;
              
                for (auto& it_pg : photogroups_)
                {
                    
                   
                    if (it_pg.second.PhotoGroupContain(it))
                    {
                        auto pgtemp = it;
                        
                        auto idsss = it_pg.second.GetGroupImageIds();
                        auto idsss1 = it.GetGroupImageIds();
                        Camera cam = it.GetCamera();
                        cam.SetCameraId(it_pg.first);
                        if(!camids_added.count(it.GetId()))
                            ATdata->GetCamerasMutual().erase(it.GetId());
                        if (ATdata->GetCamerasMutual().count(it_pg.first))
                        {
                            ATdata->GetCamerasMutual().erase(it_pg.first);
                        }
                       auto retss= ATdata->GetCamerasMutual().insert(std::make_pair(it_pg.first, cam));
                       if (retss.second)
                       {
                           camids_added.insert(it_pg.first);
                       }
                       pgtemp.SetId(it_pg.first);
                       pgtemp.GetCameraMutual().SetCameraId(it_pg.first);
                        
                        for (const auto& img_id : it.GetGroupImageIds())
                        {
                            ATdata->GetImageMutual(img_id).SetCameraId(it_pg.first);
                            ATdata->GetImageMutual(img_id).SetPhotoGroupID(it_pg.first);
                        }
                        pgtemp.SetName(it_pg.second.GetName());
                        photogroups_[it_pg.first] = pgtemp;
                        IsContain = true;
                        break;
                    }
                }
                if (!IsContain)
                {
                    photogroups_[it.GetId()] = it;
                }
            }
            
            {
                LOGI("*********************TransFormImages************************");
                if (ExistSRSId(blockSRS_id_))
                {
                    srs_s origin_srs = srs_map_.at(blockSRS_id_);
                    if (origin_srs.type == coord_system_type_e::LOCAL_ENU)
                    {
                        srs_enu_discription_ = origin_srs;
                    }
                    ATdata->SetOriginSrs(origin_srs.definition);
                    ATdata->SetLocalSrs(origin_srs.definition);
                    if (transformTobase)
                    {
                        std::string dst_definition = BASESRS;
                        if (origin_srs.type != LOCAL)
                        {
                            
                            if (origin_srs.type == GEOGRAPHIC)
                            {
                                ATdata->TransFormImages(origin_srs.definition, dst_definition);
                                ATdata->TransFormTiepoints(origin_srs.definition, dst_definition);
                            }
                            else
                            {
                                dst_definition = origin_srs.definition;
                            }
                            ATdata->TransformControlPoints(dst_definition);
                            ATdata->SetLocalGcpSrs(dst_definition);
                            ATdata->SetLocalSrs(dst_definition);
                            UpdateSRSMap(CoordinateDescriptor::GetSRSFromDefinition(dst_definition));
                            blockSRS_id_ = ExistSRS(dst_definition);
                        }
                    }
                }
                ATdata->ComputeDepths();
            }
            
#ifdef USE_AI3D_PROJ
            for (auto& srs : srs_map_)
            {
                if (srs_used_ids.count(srs.first))
                {
                    

                    AI3D::PROJ::CoordinateReferenceSystem::AddCrs(srs.second.definition);
                }
            }
#endif
            LOGI("*********************load xml finished************************");
            return true;
        }


        int BlockObject::ExportATDataToXML(const std::string& xml_file_path, BlockExportOptions block_export_options, ATData ATdata)
        {
            pugi::xml_document doc;
            pugi::xml_node declaration_node = doc.append_child(pugi::node_declaration);

            declaration_node.append_attribute("version") = "1.0";
            declaration_node.append_attribute("encoding") = "utf-8";

            pugi::xml_node blocksexchange = doc.append_child("BlocksExchange");
            blocksexchange.append_attribute("version") = "3.2";
            
            if (block_export_options.srs_.type == LOCAL)
            {
                if (srs_enu_discription_.type != LOCAL)
                {
                    block_export_options.srs_ = srs_enu_discription_;
                }
                else
                {
                    if (ATdata.GetLocalSrs() == "" || ATdata.GetLocalSrs() == LOCALSRS)
                    {

                    }
                    else
                    {
                        block_export_options.srs_ = ATdata.GetDefaultEnuSRS();
                        
                    }
                }
                srsid_t id = ExistSRS(block_export_options.srs_.definition);
                if (id == kInvalidSrsId)
                {
                    block_export_options.srs_.ID = GenerateValidSrsId();
                    srs_map_.insert(std::make_pair(block_export_options.srs_.ID, block_export_options.srs_));
                }
                else
                {
                    block_export_options.srs_.ID = id;
                }
            }

            
            EIGEN_STL_UMAP(srsid_t, srs_s) output_srs;
            if (!srs_map_.empty())
            {
                
                
                output_srs = srs_map_;
            }

            if (block_export_options.srs_.type != coord_system_type_e::LOCAL)
            {
                bool hassrs = false;
                srs_s srs_found;
                for (auto& iter : output_srs)
                {
                    if (CoordinateTransformer::IsSame(iter.second.definition, block_export_options.srs_.definition))
                    {
                        hassrs = true;
                        srs_found = iter.second;
                        break;
                    }
                }
                if (!hassrs)
                {
                    srsid_t idtemp = GenerateValidSrsId();
                    block_export_options.srs_.ID = idtemp;
                    output_srs.insert(std::make_pair(idtemp, block_export_options.srs_));
                }
                else
                {
                    block_export_options.srs_ = srs_found;
                }
            }

            const bool use_constraint_local_meter_srs = ATdata.HasConstraints();
            if (use_constraint_local_meter_srs) {
                block_export_options.srs_.ID = 0;
                block_export_options.srs_.name = "Local coordinate system (meter)";
                block_export_options.srs_.definition = "Local:unit=meter";
                block_export_options.srs_.type = coord_system_type_e::LOCAL;
            }

            if (use_constraint_local_meter_srs)
            {
                pugi::xml_node SpatialReferenceSystems = blocksexchange.append_child("SpatialReferenceSystems");
                pugi::xml_node srs = SpatialReferenceSystems.append_child("SRS");
                pugi::xml_node id = srs.append_child("Id");
                id.append_child(pugi::node_pcdata).set_value("0");
                pugi::xml_node srs_name = srs.append_child("Name");
                srs_name.append_child(pugi::node_pcdata).set_value("Local coordinate system (meter)");
                pugi::xml_node definition = srs.append_child("Definition");
                definition.append_child(pugi::node_pcdata).set_value("Local:unit=meter");
            }
            else if (!output_srs.empty())
            {
                pugi::xml_node SpatialReferenceSystems = blocksexchange.append_child("SpatialReferenceSystems");
                {
                    
                    
                    {
                        for (const auto& srs_map : output_srs)
                        {
                            pugi::xml_node srs = SpatialReferenceSystems.append_child("SRS");
                            pugi::xml_node id = srs.append_child("Id");
                            id.append_child(pugi::node_pcdata).set_value(std::to_string(srs_map.first).c_str());
                            pugi::xml_node name = srs.append_child("Name");
                            name.append_child(pugi::node_pcdata).set_value(srs_map.second.name.c_str());
                            pugi::xml_node definition = srs.append_child("Definition");
                            definition.append_child(pugi::node_pcdata).set_value(srs_map.second.definition.c_str());
                        }
                    }
                }
            }


            
            pugi::xml_node Block = blocksexchange.append_child("Block");

            if (!name_.empty())
            {
                pugi::xml_node name = Block.append_child("Name");
                name.append_child(pugi::node_pcdata).set_value(name_.c_str());
            }

            if (!description_.empty())
            {
                pugi::xml_node description = Block.append_child("Description");
                description.append_child(pugi::node_pcdata).set_value(description_.c_str());
            }

            if (block_export_options.srs_.ID != kInvalidSrsId)
            {
                pugi::xml_node SRSId = Block.append_child("SRSId");
                SRSId.append_child(pugi::node_pcdata).set_value(std::to_string(block_export_options.srs_.ID).c_str());
            }



            if (!use_constraint_local_meter_srs &&
                !(block_export_options.srs_.definition == "" || block_export_options.srs_.definition == LOCALSRS))
            {
                
                
                
                std::string src_srs_definition = ATdata.GetLocalSrs();
                if (!(src_srs_definition.find("ENU") != std::string::npos && block_export_options.srs_.definition.find("ENU") != std::string::npos))
                {
                    if (ATdata.HasTiepoints())
                    {
                        ATdata.TransFormTiepoints(src_srs_definition, block_export_options.srs_.definition);
                    }
                    ATdata.TransFormImages(src_srs_definition, block_export_options.srs_.definition);
                    ATdata.TransFormGCPs(src_srs_definition, block_export_options.srs_.definition);

                }
                

            }
            
            pugi::xml_node Photogroups = Block.append_child("Photogroups");
            std::set<std::pair<image_t, group_t>> ids;
            for (const auto& pg : photogroups_)
            {
                image_t min_image_id = *pg.second.GetGroupImageIds().begin();
                std::pair<int, int> id = std::make_pair(min_image_id, pg.first);
                ids.insert(id);
            }



            
            for (const auto& id : ids)
            {
                auto& pg = photogroups_[id.second];
                pugi::xml_node photogroup = Photogroups.append_child("Photogroup");
                SerializePhotoGroup(ATdata, pg, photogroup, block_export_options);
            }

            
            if (block_export_options.export_controlpoint_ && !ATdata.GetControlPoints().empty())
            {
                pugi::xml_node ControlPoints;
                bool any_gcp = false;
                for (auto& cp : ATdata.GetControlPoints())
                {
                    if (!cp.second.HasGivenXYZ()) {
                        continue;
                    }
                    if (!any_gcp) {
                        ControlPoints = Block.append_child("ControlPoints");
                        any_gcp = true;
                    }
                    pugi::xml_node controlpoint = ControlPoints.append_child("ControlPoint");
                    SerializeControlPoint(ATdata, cp.second, controlpoint);
                }
            }


            
            bool bexportAutoTiepoints = (block_export_options.export_tiepoint_ && !ATdata.GetPoints3D().empty());
            bool baddtiepointnode = (bexportAutoTiepoints || ATdata.HasUserTiepoints());
            if (baddtiepointnode)
            {
                pugi::xml_node TiePoints = Block.append_child("TiePoints");
                if (bexportAutoTiepoints)
                {
                    for (const auto& tp : ATdata.GetPoints3D())
                    {
                        SerializeTiePoint(tp.second, TiePoints, false);
                    }
                }
                if (ATdata.HasUserTiepoints())
                {
                    for (const auto& tp : ATdata.GetUserPoints3D())
                    {
                        SerializeTiePoint(tp.second, TiePoints, true);
                    }
                }

            }

            SerializePositioningConstraints(ATdata, Block);
            
            bool saveSucceed = doc.save_file(xml_file_path.c_str());
            if (!saveSucceed)
            {
                LOG(ERROR) << "saving" + xml_file_path +" xml failed!";
                return AI3D_FAILURE;
            }

            return AI3D_SUCCESS;
        }

        void BlockObject::SerializePositioningConstraints(ATData& atdata, pugi::xml_node block_root)
        {
            pugi::xml_node positioning_root;
            bool have_root = false;
            for (auto& kv : atdata.GetConstraintsMutual()) {
                MeasureConstraint& mc = kv.second;
                if (mc.GetType() != CONSTRAINT_TYPE::CONSTRAINT_SCALE) {
                    continue;
                }
                auto& pl = mc.GetPointList();
                if (pl.size() < 2) {
                    continue;
                }
                std::vector<point3D_t> point_ids;
                point_ids.reserve(pl.size());
                for (const auto& pit : pl) {
                    point_ids.push_back(pit.first);
                }
                std::vector<ConstraintKV>& kvs = mc.GetConstraintItemList();
                if (kvs.size() < 2) {
                    continue;
                }
                double dist_m = kvs[0].getDoubleValue();
                const SCALE_UNIT_TYPE unit = static_cast<SCALE_UNIT_TYPE>(kvs[1].getIntValue());
                if (unit == SCALE_UNIT_TYPE::UNIT_CENTI) {
                    dist_m /= 100.0;
                } else if (unit == SCALE_UNIT_TYPE::UNIT_KILOMETER) {
                    dist_m *= 1000.0;
                }
                if (!have_root) {
                    positioning_root = block_root.append_child("PositioningConstraints");
                    have_root = true;
                }
                pugi::xml_node sc = positioning_root.append_child("ScaleConstraint");
                pugi::xml_node node_a = sc.append_child("A");
                node_a.append_child(pugi::node_pcdata).set_value(std::to_string(point_ids[0]).c_str());
                pugi::xml_node node_b = sc.append_child("B");
                node_b.append_child(pugi::node_pcdata).set_value(std::to_string(point_ids[1]).c_str());
                pugi::xml_node node_d = sc.append_child("DistanceAB");
                node_d.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(dist_m).c_str());
                pugi::xml_node node_u = sc.append_child("Unit");
                node_u.append_child(pugi::node_pcdata).set_value("meter");
            }
        }

        void BlockObject::EnsureCurrentATReflectsConstraintScale()
        {
            if (GetCurrentAT() == nullptr) {
                return;
            }
            
            if (status_ != jobsta_e::STATUS_COMPLETE) {
                return;
            }
            const std::string constraint_path =
                File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + CONSTRAINTFILE);
            if (!File::ExistsFile(constraint_path)) {
                return;
            }
            std::shared_ptr<ATData> at_mut = GetCurrentATMutual();
            if (!at_mut) {
                return;
            }
            if (!at_mut->LoadConstraint(constraint_path)) {
                LOGE(std::string("EnsureCurrentATReflectsConstraintScale LoadConstraint failed: ") +
                     constraint_path);
                return;
            }
            if (!at_mut->HasConstraints()) {
                return;
            }
            
            if (at_mut->IsConstraintScaleSimilarityAlreadyApplied()) {
                LOGI(std::string("EnsureCurrentATReflectsConstraintScale: block=") + GetIdString() +
                     " CONSTRAINT_SCALE already satisfied (same as post-AT merge), skip handleConstraint");
                if (status_ == jobsta_e::STATUS_COMPLETE && ATGroups_.count(0)) {
                    ATData_ = ATGroups_.at(0).GetATDataMutual();
                }
                return;
            }
            LOGI(std::string("EnsureCurrentATReflectsConstraintScale: block=") + GetIdString() +
                 " handleConstraint");
            if (at_mut->handleConstraint()) {
                if (!SyncPoseSidecarBinsAfterConstraintSimilarity()) {
                    LOGW("EnsureCurrentATReflectsConstraintScale: SyncPoseSidecarBinsAfterConstraintSimilarity failed.");
                }
            }
            if (status_ == jobsta_e::STATUS_COMPLETE && ATGroups_.count(0)) {
                ATData_ = ATGroups_.at(0).GetATDataMutual();
            }
        }

        
        int BlockObject::ExportATXML(const std::string& xml_file_path, BlockExportOptions block_export_options)
        {
            if (GetCurrentAT() == nullptr)
            {
                return AI3D_FAILURE;
            }
            
            if (!ReloadCurrentATFromPersistedFilesForExportOrReconstruction()) {
                return AI3D_FAILURE;
            }

            auto Atdata = GetCurrentAT();
            {
                std::ostringstream oss;
                oss << "ExportATXML[block]: id=" << GetIdString() << " name=" << GetName()
                    << " path=" << GetPath() << " out=" << xml_file_path
                    << " status=" << static_cast<int>(status_)
                    << " GetCurrentAT*=" << static_cast<const void*>(Atdata.get())
                    << " ATData_*=" << static_cast<const void*>(GetATData().get())
                    << " ATGroups[0]*=";
                if (ATGroups_.count(0)) {
                    oss << static_cast<const void*>(ATGroups_.at(0).GetATData().get());
                } else {
                    oss << "null";
                }
                LOGI(oss.str());
            }

            ATData ATdata = *Atdata;

            return ExportATDataToXML(xml_file_path, block_export_options, ATdata); 
        }

        bool BlockObject::LoadExternalFile(const std::string& name)
        {
            std::string prjFileType;
            
            try
            {
                prjFileType = File::BoostPathFromUtf8(name).extension().string();
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            String::StringToLower(&prjFileType);
            bool ret = false;

            
            
            
            
            
            if (prjFileType == ".xls" && !Allow2LoadXLS())
            {
                auto atdata = std::make_shared<ATData>();
                ret = LoadXLS(name, atdata);
                if (!ret)
                {
                    return false;
                }
                SetATData(atdata);
                GetTaskInfoMutual().isSaved = false;            
            }
            else if (prjFileType == ".xlsx" && !Allow2LoadXLS())
            {
                auto atdata = std::make_shared<ATData>();
                ret = LoadXLSX(name, atdata);
                if (!ret)
                {
                    return false;
                }
                SetATData(atdata);
                GetTaskInfoMutual().isSaved = false;            
            }
            else if (prjFileType == ".xml")
            {           
                auto atdata = std::make_shared<ATData>();
                ret = LoadATXML(name, atdata);
                if (!ret)
                {
                    return false;
                }
                SetATData(atdata);
                if (atdata->HasTiepoints())
                {
                    SetTiepointStatus(true);
                    GetCurrentATMutual()->SetPoint3DsStatus(true);
                }
                GetTaskInfoMutual().isSaved = false;            
            }

            return ret;
        }

        
        bool BlockObject::Task_Info::ReadBlockInfoJson(const std::string& file_path)
        {
            std::string blkcontent;

            bool ret = RapidJsonCore::ReadFile(File::EnsureUnifySlash(file_path), blkcontent);
            if (!ret)
            {
                LOGE(String::StringPrintf("File: %s was Read Error", file_path.c_str()));
                return false;
            }

            rapidjson::Document doc_blk;

            if (doc_blk.Parse(blkcontent.data()).HasParseError())
            {
                LOGE(String::StringPrintf("%s :parse block file  error!", file_path.c_str()));
                return false;
            }

            if (!doc_blk.IsObject())
            {
                LOGE("Parse block file error!");
                return false;
            }

            if (!doc_blk.HasMember("block"))
            {
                LOGE("Blocks file error");
                return false;
            }
            rapidjson::Value& block_blk = doc_blk["block"];
            bool bhassettings = false;
            if (doc_blk.HasMember("settings"))
            {
                rapidjson::Value& settings = doc_blk["settings"];
                if (!settings.HasMember("feature") || !settings.HasMember("sfm"))
                {
                    LOGE("Block file error!");
                    return false;
                }
                else
                {
                    at_options.ParseJson(settings);
                    bhassettings = true;
                }


            }

            if (doc_blk.HasMember("block_task_category"))
            {
                block_task_category = doc_blk["block_task_category"].GetInt();
            }
            if (doc_blk.HasMember("gen_options"))
            {
                gen_options.ParseJson(doc_blk["gen_options"]);
            }
            if (doc_blk.HasMember("generations_info"))
            {
                const rapidjson::Value& genArr = doc_blk["generations_info"];
                for (rapidjson::SizeType i = 0; i < genArr.Size(); i++)
                {
                    blk_generation_info_s info;
                    info.ParseJson(genArr[i]);
                    generations_info_.push_back(info);
                }
            }
            if (doc_blk.HasMember("GenJobs"))
            {
                const rapidjson::Value& genJobsArr = doc_blk["GenJobs"];
                for (rapidjson::SizeType i = 0; i < genJobsArr.Size(); i++)
                {
                    std::string combined = genJobsArr[i].GetString();
                    size_t colonPos = combined.find(":");
                    if (colonPos != std::string::npos)
                    {
                        generationjobs_[combined.substr(0, colonPos)] = combined.substr(colonPos + 1);
                    }
                }
            }

            if (block_blk.HasMember("blockStatistics") )
            {
                hasstatisinfo = true;
                rapidjson::Value& staticsinfo = block_blk["blockStatistics"];
                
                if (staticsinfo.HasMember("tiepointNum"))
                {
                    statisticinfo_.tiepointnum = staticsinfo["tiepointNum"].GetInt();
                    std::string msg = std::to_string(blockId) + __FUNCTION__ + " ******** ";
                    
                    msg += std::to_string(statisticinfo_.tiepointnum);
                    
                }

            }
            else
            {
                hasstatisinfo = false;
                
            }


            if (1)
            {
                if (block_blk.HasMember("ATsettings"))
                {
                    rapidjson::Value& settings1 = block_blk["ATsettings"];
                    at_options.ParseJson(settings1);
                    
                    
                   hasatsetting = true;
                }
            }

            
            if (block_blk.HasMember("ATJson"))
            {
                atjson_ = block_blk["ATJson"].GetString();
            }
            
            if (block_blk.HasMember("GCPJson"))
            {
                gcpjson_ = block_blk["GCPJson"].GetString();
            }

            
            if (block_blk.HasMember("blkString"))
            {
                blockString = block_blk["blkString"].GetString();
            }
            if (block_blk.HasMember("mergedFrom"))
            {
                mergedFrom = block_blk["mergedFrom"].GetString();
            }
            if (block_blk.HasMember("blkId"))
            {
                blockId = block_blk["blkId"].GetInt();
                
            }

            if (block_blk.HasMember("job"))
            {
                job_ = block_blk["job"].GetString();
            }

            if (block_blk.HasMember("isFinished"))
            {
                isFinished = block_blk["isFinished"].GetBool();
            }
            if (block_blk.HasMember("btopredict_"))
            {
                btopredict_ = block_blk["btopredict_"].GetBool();
            }

            if (block_blk.HasMember("AT_Num"))
            {
                AT_Num = block_blk["AT_Num"].GetInt();
            }

            std::string BlockXML;
            if (block_blk.HasMember("BlockXML"))
            {
                
                BlockXML = block_blk["BlockXML"].GetString();
                
                BlockXML = File::GetParentDir(file_path) + BlockXML.substr(1);
               Block_XML = BlockXML;
            }
        
            if (block_blk.HasMember("Tiepoints"))
            {
                
                Tiepoints = block_blk["Tiepoints"].GetString();
                
                Tiepoints = File::GetParentDir(file_path) + Tiepoints.substr(1);
                
            }

            

            if (block_blk.HasMember("recontructions"))
            {
                auto recontructionsjson = block_blk["recontructions"].GetArray();
                for (int index = 0; index < recontructionsjson.Size(); index++)
                {
                    blk_recontruction_info_s info;
                    info.ParseJson(recontructionsjson[index]);
                    reconstructions_info_.push_back(info);
                }
                
            }
            if (block_blk.HasMember("BRPJobs"))
            {
                rapidjson::Value& jobjson= block_blk["BRPJobs"].GetArray();
                for (int i = 0; i < jobjson.Size(); i++)
                {
                    std::string jobstr = jobjson[i].GetString();
                    auto strs = String::StringSplit(jobstr, ":");
                    reconstructionjobs_[strs[0]] = strs[1];
                }
            }


            if (bhassettings)
            {
                jobtype_e jobtype = GetJobType(job_);
                if (jobtype == JOB_AT)
                {
                    hasatsetting = true;
                    if (BLK_USE_BIN) {
                        WriteBlockInfoToBin(file_path, true);
                    }
                    else {
                        WriteBlockInfoToJson(file_path, true);
                    }
                    
                }
                else
                {
                    if (BLK_USE_BIN) {
                        WriteBlockInfoToBin(file_path, false);
                    }
                    else {
                        WriteBlockInfoToJson(file_path, false);
                    }
                }
                
            }
            return true;
        }

        bool BlockObject::Task_Info::ReadBlockInfoBin(const std::string& file_path)
        {
            std::ifstream in = File::OpenIfstreamUtf8(file_path, std::ios::binary);
            
            if (!in.is_open())
                return false;

            BLKBinFile bLKBinFile;
            bool result = bLKBinFile.Deserialize(in);
            if (!result) {
                LOGE("load block file error");
                return false;
            }

            blockString = bLKBinFile.blkString;
            mergedFrom = bLKBinFile.mergedFrom;
            blockId = bLKBinFile.blkId;
            job_ = bLKBinFile.job;
            isFinished = bLKBinFile.isFinished;
            btopredict_ = bLKBinFile.btoPredict;
            AT_Num = bLKBinFile.AT_Num;
            std::string blkxmlpath = bLKBinFile.BlockXML;
            std::string tilePointPath = bLKBinFile.Tiepoints;
#ifdef WIN32
            // blkxmlpath = UTF82GBK(blkxmlpath);
            // tilePointPath = UTF82GBK(tilePointPath);
#endif 
            blkxmlpath = File::GetParentDir(file_path) + blkxmlpath.substr(1);
            Block_XML = blkxmlpath;           
            tilePointPath = File::GetParentDir(file_path) + tilePointPath.substr(1);
            Tiepoints = tilePointPath;
            if (bLKBinFile.hasAT) {
                atjson_ = bLKBinFile.ATJson;
#ifdef WIN32
                // atjson_ = UTF82GBK(atjson_);
#endif 
            }
            if (bLKBinFile.hasGCP) {
                gcpjson_ = bLKBinFile.GCPJson;
#ifdef WIN32
                // gcpjson_ = UTF82GBK(gcpjson_);
#endif 
            }

            bool bhassettings = false;
            
            
            
            
            
            
            
            
            
            
            
            
            


            
            hasstatisinfo = true;
            statisticinfo_.tiepointnum = bLKBinFile.tiepointNum;

            block_task_category = bLKBinFile.gen_block_task_category;
            gen_options.gen_params = GenTaskParams::CreateFromJsonString(bLKBinFile.gen_params_json);
            generations_info_.clear();
            if (!bLKBinFile.gen_info_json.empty()) {
                rapidjson::Document doc;
                if (!doc.Parse(bLKBinFile.gen_info_json.c_str()).HasParseError() && doc.IsArray()) {
                    for (rapidjson::SizeType i = 0; i < doc.Size(); i++) {
                        blk_generation_info_s info;
                        info.ParseJson(doc[i]);
                        generations_info_.push_back(info);
                    }
                }
            }
            generationjobs_.clear();
            for (auto& job : bLKBinFile.genJobVec) {
                size_t colonPos = job.find(":");
                if (colonPos != std::string::npos)
                    generationjobs_[job.substr(0, colonPos)] = job.substr(colonPos + 1);
            }

            at_options.feature_num = bLKBinFile.atSetting.keyNum;
            at_options.maxthreads_num = bLKBinFile.atSetting.maxthreads_num;
            at_options.saveoptions.min_overlap = bLKBinFile.atSetting.minOverlap;
            at_options.saveoptions.max_overlap = bLKBinFile.atSetting.maxOverlap;
            at_options.saveoptions.max_tiepoint_num = bLKBinFile.atSetting.maxTieptNum;
            at_options.sfmsettings.sfm_mode = (sfm_mode_e)bLKBinFile.atSetting.mode;
            at_options.sfmsettings.grid_count_1 = bLKBinFile.atSetting.ba1_grid_count;
            at_options.sfmsettings.grid_count_2 = bLKBinFile.atSetting.ba2_grid_count;           
            at_options.sfmsettings.max_feature_count_1 = bLKBinFile.atSetting.max_feature_count_1;
            at_options.sfmsettings.max_feature_count_2 = bLKBinFile.atSetting.max_feature_count_2;
            at_options.saveoptions.boutput_tiepoint = bLKBinFile.atSetting.output_tiepoint;
            at_options.saveoptions.max_projection_error = bLKBinFile.atSetting.max_projection_error;
            at_options.reconstruct_mode = (pair_selection_mode_e)bLKBinFile.atSetting.reconstruct_mode;
            at_options.saveoptions.output_rawxml = bLKBinFile.atSetting.output_rawxml;
            at_options.sfmsettings.bapolicies.use_user_tiepoints_ = bLKBinFile.atSetting.use_user_tiepoints;
            if (bLKBinFile.atSetting.use_user_tiepoints) {
                std::string usertiepoints_path_ = bLKBinFile.atSetting.usertiepoints_path_;
#ifdef WIN32
                // usertiepoints_path_ = UTF82GBK(usertiepoints_path_);
#endif 
                at_options.sfmsettings.bapolicies.usertiepoints_path_ = usertiepoints_path_;
            }           
            at_options.sfmsettings.bapolicies.use_gcp_ = bLKBinFile.atSetting.use_gcp;
            if (bLKBinFile.atSetting.use_gcp) {
                std::string control_point_path = bLKBinFile.atSetting.control_point_path;
#ifdef WIN32
                // control_point_path = UTF82GBK(control_point_path);
#endif 
                at_options.sfmsettings.bapolicies.gcp_path_ = control_point_path;
            } 
            at_options.sfmsettings.bapolicies.use_constraints_ = bLKBinFile.atSetting.use_constraint;
            if (bLKBinFile.atSetting.use_constraint) {
                std::string cosntraint_path = bLKBinFile.atSetting.constraint_path;
#ifdef WIN32
                // cosntraint_path = UTF82GBK(cosntraint_path);
#endif 
                at_options.sfmsettings.bapolicies.constraint_path_ = cosntraint_path;
            }
            at_options.sfmsettings.bapolicies.tiepoints_policy_ = (AI3D::CORE::policies_e)bLKBinFile.atSetting.tiepoints_policy;
            at_options.sfmsettings.bapolicies.pos_policy_ = (AI3D::CORE::policies_e)bLKBinFile.atSetting.pos_policy;
            at_options.sfmsettings.bapolicies.f_policy_ = (AI3D::CORE::policies_e)bLKBinFile.atSetting.ppa_policy;
            at_options.sfmsettings.bapolicies.rdis_policy_ = (AI3D::CORE::policies_e)bLKBinFile.atSetting.rdis_policy;
            at_options.sfmsettings.bapolicies.tdis_policy_ = (AI3D::CORE::policies_e)bLKBinFile.atSetting.tdis_policy;
            at_options.sfmsettings.bapolicies.use_image_position_ = bLKBinFile.atSetting.use_image_position_;
            if (bLKBinFile.atSetting.use_image_position_) {
                std::string image_pos_list = bLKBinFile.atSetting.image_pos_list;
#ifdef WIN32
                // image_pos_list = UTF82GBK(image_pos_list);
#endif 
                at_options.sfmsettings.bapolicies.pos_path_ = image_pos_list;
            }
            if (bLKBinFile.atSetting.hasATPath) {
                std::string at_path_ = bLKBinFile.atSetting.at_path;
#ifdef WIN32
                // at_path_ = UTF82GBK(at_path_);
#endif 
                at_options.sfmsettings.bapolicies.at_path_ = at_path_;
            }
            hasatsetting = true;
           
            
            reconstructions_info_.clear();
            for(ReconstrutionData  reconData : bLKBinFile.reconstrutionDataVec)
            {
                blk_recontruction_info_s info;
                info.id_ = reconData.id;
                info.name_ = reconData.name;
                if (reconData.hasCoord) {
                    info.srs_custom_.type = static_cast<coord_system_type_e>(reconData.coordinateData.type);
                    if (info.srs_custom_.type == coord_system_type_e::LOCAL_ENU) {
                        std::string lat = std::to_string(reconData.coordinateData.ori[0]);
                        std::string lon = std::to_string(reconData.coordinateData.ori[1]);
                        info.srs_custom_.definition = "LAT:" + lat + ",LON:" + lon;
                    }
                    else {
                        info.srs_custom_.definition = reconData.coordinateData.espgStr;
                    }
                }
                bbox_s bb;
                bb.xmax_ = reconData.boundingbox_custom.max[0];
                bb.ymax_ = reconData.boundingbox_custom.max[1];
                bb.zmax_ = reconData.boundingbox_custom.max[2];
                bb.xmin_ = reconData.boundingbox_custom.min[0];
                bb.ymin_ = reconData.boundingbox_custom.min[1];
                bb.zmin_ = reconData.boundingbox_custom.min[2];
                info.boundingbox_custom_ = bb.toABBox3d();

                if(reconData.hasBoundary){
                    tiling_param_s param;
                    info.boundary_custom_.clear();
                    for (auto& iter1 : reconData.boundary_custom_)
                    {
                        auto& boundarylevel2 = iter1;
                        std::vector<Eigen::Vector2d> bd(boundarylevel2.size());
                        for (unsigned index = 0; index < boundarylevel2.size(); index++)
                        {
                            bd[index][0] = boundarylevel2[index][0];
                            bd[index][1] = boundarylevel2[index][1];
                        }

                        info.boundary_custom_.push_back(bd);
                    }
                }
                
                tiling_mode_e tile_model = (tiling_mode_e)reconData.tillData.tiling_mode;
                info.tile_params_.mode_ = tile_model;
                info.tile_params_.expected_max_ram_used_ = reconData.tillData.expected_max_ram_used;
                if (tile_model == tiling_mode_e::TILE_NONE)
                {

                }
                else if (tile_model == tiling_mode_e::TILE_PALNAR_GRID || tile_model == tiling_mode_e::TILE_VOL_GRID || tile_model == tiling_mode_e::TILE_ADAPTIVE)
                {
                    info.tile_params_.regular_params_.automatic_origin_.x() = reconData.tillData.automatic_origin[0];
                    info.tile_params_.regular_params_.automatic_origin_.y() = reconData.tillData.automatic_origin[1];
                    info.tile_params_.regular_params_.automatic_origin_.z() = reconData.tillData.automatic_origin[2];

                    info.tile_params_.regular_params_.custom_origin_.x() = reconData.tillData.custom_origin[0];
                    info.tile_params_.regular_params_.custom_origin_.y() = reconData.tillData.custom_origin[1];
                    info.tile_params_.regular_params_.custom_origin_.z() = reconData.tillData.custom_origin[2];

                    info.tile_params_.regular_params_.tilesize_ = reconData.tillData.tileSize;
                }               

                
                info.tiles_.clear();
                for (const auto& tileData : reconData.tileVec) {
                    tile_info_s tile;
                    tile.index_ = tileData.index;
                    tile.name_ = tileData.name;
                    tile.reference_model_status_ = static_cast<tile_info_s::reconst_status_e>(tileData.status);
                    tile.isempty = tileData.isempty;

                    if (tileData.hasBBbox) {
                        bbox_s bb;
                        bb.xmin_ = tileData.bbox.min[0]; 
                        bb.ymin_ = tileData.bbox.min[1];
                        bb.zmin_ = tileData.bbox.min[2];

                        bb.xmax_ = tileData.bbox.max[0];
                        bb.ymax_ = tileData.bbox.max[1];
                        bb.zmax_ = tileData.bbox.max[2];
                        tile.bb_ = bb.toABBox3d().cast<float>();
                    }

                    for (auto& iter : tileData.imageids)
                    {
                        auto id = iter;
                        tile.image_ids_.insert(id);
                    }

                    info.tiles_[tile.name_] = tile;
                }

                
                info.production_infos_.clear();
                for (const auto& productionData : reconData.productionVec) {
                    blk_reconst_production_info_s pinfo;
                    
                    pinfo.id_ = productionData.id;
                    pinfo.name_ = productionData.name;
                    std::string settings_str_ = productionData.modelingsettings;
                    pinfo.options_.settings_str_ = settings_str_;
                    
                    
                    
                    
                    
                    
                    
                    

                    rapidjson::Document doc;
                    if (doc.Parse(settings_str_.data()).HasParseError())
                    {
                        LOGE("Parse production setting ERROR!");
                        return false;
                    }
                    
                    if (doc.HasMember("srs_definition"))
                    {
                        auto value = doc["srs_definition"].GetString();

                        pinfo.options_.cs_.definition_ = value;
                    }
                    if (doc.HasMember("coordinate_origin"))
                    {
                        auto value = doc["coordinate_origin"].GetArray();
                        if (value.Size() != 3)
                        {
                            return false;
                        }
                        Eigen::Vector3d xyz;
                        xyz.x() = value[0].GetDouble();
                        xyz.y() = value[1].GetDouble();
                        xyz.z() = value[2].GetDouble();
                        pinfo.options_.cs_.origin_ = xyz;

                    }
                                     
                    auto formatstr = productionData.production_format;
                    if (ProductionFormatStringFromProcessing.count(formatstr))
                    {
                        pinfo.options_.production_format_ = ProductionFormatStringFromProcessing.at(formatstr);
                    }

                    std::string tmpDestin = productionData.destination;
#ifdef WIN32
                    // tmpDestin = UTF82GBK(tmpDestin);
#endif 
                    if (!tmpDestin.empty()) {
                        pinfo.options_.destination_ = tmpDestin;
                    }
                    
                    std::string tmpName = productionData.name;
#ifdef WIN32
                    // tmpName = UTF82GBK(tmpName);
#endif 
                    if (!tmpName.empty()) {
                        pinfo.options_.name_ = tmpName;
                    }
                    

                    pinfo.options_.tiles_ = productionData.tiles;

                    info.production_infos_.push_back(pinfo);
                }
                
                
                info.processing_settings_.level_ = static_cast<geometric_level_e>(reconData.Geometric_Level);
                info.processing_settings_.bcolorbalance_ = reconData.ColorBalanced;
                info.processing_settings_.untex_policy_ = static_cast<untexture_policy_e>(reconData.Untexture_Fill_Mode);
                if (info.processing_settings_.untex_policy_ == untexture_policy_e::UNTEX_COLOR_FILLED)
                {
                    for (int n = 0; n < 3; n++)
                    {
                        info.processing_settings_.texture_fill_color_[n] = reconData.Texture_Fill_Color[n];
                    }
                }
                info.processing_settings_.bdiscard_emptytiles_ = reconData.DiscardEmptyTiles;
                info.processing_settings_.hollfilling_ = static_cast<holefilling_policy_e>(reconData.HoleFillingMode);
                reconstructions_info_.push_back(info);
            }
            
            reconstructionjobs_.clear();
            for (auto& job : bLKBinFile.jobVec) {
                size_t colonPos = job.find(":");
                if (colonPos != std::string::npos) {
                    std::string firstPart = job.substr(0, colonPos);
                    std::string secondPart = job.substr(colonPos + 1);
                    reconstructionjobs_[firstPart] = secondPart;
                }
            }

            if (bhassettings)
            {
                jobtype_e jobtype = GetJobType(job_);
                if (jobtype == JOB_AT)
                {
                    hasatsetting = true;
                    if (BLK_USE_BIN) {
                        WriteBlockInfoToBin(file_path, true);
                    }
                    else {
                        WriteBlockInfoToJson(file_path, true);
                    }
                    
                }
                else
                {
                    if (BLK_USE_BIN) {
                        WriteBlockInfoToBin(file_path, false);
                    }
                    else {
                        WriteBlockInfoToJson(file_path, false);
                    }
                    
                }

            }

            in.close();
            return true;
        }

        void BlockObject::MakeBlockFromATData(ATData AATModel)
        {       
            for (auto& iter : AATModel.GetCameras())
            {
                PhotoGroup pg;
                
                std::set<image_t> images_pg_temp;
                std::string name;
                int temp_id = iter.first;
                name = GROUPBASENAME + std::to_string(temp_id);
                pg.SetName(name);
                
                Camera camera = iter.second;
                pg.SetCamera(camera);
                pg.SetId(temp_id);
                for (auto& image : AATModel.GetImagesMutual())
                {
                
                    if (image.second.GetCameraId() == iter.first)
                    {
                        images_pg_temp.insert(image.first);
                        image.second.SetPhotoGroupID(temp_id);
                    }

                }
                pg.SetGroupImage(images_pg_temp);
                
                GetPhotoGroupsMutual()[temp_id] = pg;
                
            }
            auto atsrs = AATModel.GetLocalSrs();

            srs_map_.clear();
            int idx = 0;
            std::map<std::string, srsid_t> srs_mapid;
            for (auto& gcp : AATModel.GetControlPoints())
            {
                
                srs_s srs = gcp.second.GetSrs(); 
                std::string def = srs.definition;
                String::StringToLower(&def);
                if (!srs_mapid.count(def))
                {
                    srs_mapid[def] = idx;
                    idx++;
                }
            }
            if (!srs_mapid.count(atsrs))
            {
                srs_mapid[atsrs] = idx;
            }
          
                blockSRS_id_ = srs_mapid.at(atsrs);
            
            for (auto& iter : srs_mapid)
            {
                srs_s srs;
                
                srs = CoordinateDescriptor::GetSRSFromDefinition(iter.first);
                srs.ID = iter.second;
                
                srs_map_[iter.second] = srs;
            }
            for (auto& gcp : AATModel.GetControlPointsMutual())
            {
                srs_s srs = gcp.second.GetSrs();
                std::string def = srs.definition;
                String::StringToLower(&def);
                srs.ID = srs_mapid.at(def);
                gcp.second.SetSrs(srs);
            }
            SetATData(std::make_shared<ATData>(AATModel));

            return;
        }

        bool BlockObject::ApplyConstraintScaleAfterAtMergeIfAvailable(std::shared_ptr<ATData>& atdata)
        {
            if (!atdata) {
                return false;
            }
            const std::string constraint_path =
                File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + CONSTRAINTFILE);
            if (!File::ExistsFile(constraint_path)) {
                if (atdata->HasConstraints()) {
                    LOGW(std::string("ApplyConstraintScaleAfterAtMergeIfAvailable: block=") + GetIdString() +
                         " memory HasConstraints but no CON.bin at " + constraint_path +
                         " — skip handleConstraint (disk CON is required for post-AT scale).");
                }
                return false;
            }
            if (!atdata->LoadConstraint(constraint_path)) {
                LOGE(std::string(
                    "ApplyConstraintScaleAfterAtMergeIfAvailable: LoadConstraint failed, skip handleConstraint: ") +
                     constraint_path);
                return false;
            }
            if (!atdata->HasConstraints()) {
                return false;
            }
            {
                std::ostringstream oss;
                oss << "handleConstraint[block]: id=" << GetIdString()
                    << " name=" << GetName() << " path=" << GetPath()
                    << " ATData*=" << static_cast<const void*>(atdata.get());
                LOGI(oss.str());
            }
            const bool applied_similarity = atdata->handleConstraint();
            return applied_similarity;
        }

        bool BlockObject::SyncPoseSidecarBinsAfterConstraintSimilarity()
        {
            if (!GetCurrentAT()) {
                return true;
            }
            const std::string dir = File::EnsureUnifySlash(path_);
            bool ok = true;

#if SOURCEDATA_USE_BIN
            const std::string od_bin = File::EnsureUnifySlash(dir + PATH_SEPARATOR_STR + ORIDATABIN);
            if (File::ExistsFile(od_bin)) {
                const Eigen::Vector3d possigma(10., 10., 10.);
                if (!ATCommandSet::SaveSourceDataBinary(*GetCurrentATMutual(), od_bin, possigma)) {
                    LOGW(std::string("SyncPoseSidecars: SaveSourceDataBinary failed: ") + od_bin);
                    ok = false;
                } else {
                    LOGI(std::string("SyncPoseSidecars: updated ") + od_bin);
                }
            }
#else
            const std::string od_json = File::EnsureUnifySlash(dir + PATH_SEPARATOR_STR + ORIDATAJSON);
            if (File::ExistsFile(od_json)) {
                const Eigen::Vector3d possigma(10., 10., 10.);
                if (!ATCommandSet::SaveSourceDataJson(*GetCurrentATMutual(), od_json, possigma)) {
                    LOGW(std::string("SyncPoseSidecars: SaveSourceDataJson failed: ") + od_json);
                    ok = false;
                } else {
                    LOGI(std::string("SyncPoseSidecars: updated ") + od_json);
                }
            }
#endif

            const std::string scsfr = File::EnsureUnifySlash(dir + PATH_SEPARATOR_STR + SCBINFILE);
            if (File::ExistsFile(scsfr)) {
                if (!ExportATBinary(scsfr)) {
                    LOGW(std::string("SyncPoseSidecars: ExportATBinary failed: ") + scsfr);
                    ok = false;
                } else {
                    LOGI(std::string("SyncPoseSidecars: updated ") + scsfr);
                }
            }

            const std::string pl_bin = File::EnsureUnifySlash(dir + PATH_SEPARATOR_STR + POSBIN);
            if (File::ExistsFile(pl_bin)) {
                if (!SaveImagePosListBin(block_info_.at_options)) {
                    LOGW(std::string("SyncPoseSidecars: SaveImagePosListBin failed: ") + pl_bin);
                    ok = false;
                } else {
                    LOGI(std::string("SyncPoseSidecars: updated ") + pl_bin);
                }
            }
            const std::string pl_json = File::EnsureUnifySlash(dir + PATH_SEPARATOR_STR + POSJSON);
            if (File::ExistsFile(pl_json)) {
                if (!SaveImagePosListJson(block_info_.at_options)) {
                    LOGW(std::string("SyncPoseSidecars: SaveImagePosListJson failed: ") + pl_json);
                    ok = false;
                } else {
                    LOGI(std::string("SyncPoseSidecars: updated ") + pl_json);
                }
            }
            return ok;
        }

        int BlockObject::UpdateCompleteATFile()
        {
            try
            {
                
                
                if (!block_info_.isFinished)
                {

                    bool bgcpat = false;
                    std::string atxml_0, atxml_1, atxml, atbin;
                    
                    
                    
                    
                    
                    if (ATData_->HasControlPoints() && (std::filesystem::exists(File::BoostPathFromUtf8(File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT_absolute"))) || std::filesystem::exists(File::BoostPathFromUtf8(path_ + PATH_SEPARATOR_STR + "block_AT_absolute.xml"))) && (block_info_.statisticinfo_.tiepointnum > 0 || ATData_->HasTiepoints()))
                    {
                        
                        atxml_0 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT_absolute.xml");
                        atxml_1 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT_absolute");
                        atxml = File::IsFileExistent(atxml_0) ? atxml_0 : atxml_1;

                        atbin = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + SCBINFILE);
                        bgcpat = true;
                    }
                    else
                    {
                        atxml_0 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT.xml");
                        atxml_1 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT");
                        atxml = File::IsFileExistent(atxml_0) ? atxml_0 : atxml_1;

                        atbin = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + SCBINFILE);
                    }
                    
                    
                    std::shared_ptr<ATData> ATdata = std::make_shared<ATData>();
                    ClearImageIds();
                    
                    if (!LoadATBinary(atbin, ATdata))
                    {
                        LOGW(String::StringPrintf("Load %s error", atbin.c_str()));
                        ATdata = std::make_shared<ATData>();
                        if (!LoadATXML(atxml, ATdata))
                            return AI3D_FAILURE;
                    }
                    
                    
                    if (ATdata->GetImages().empty())
                    {
                        LOGE("ATData has no images!");
                        return AI3D_FAILURE;
                    }

                    srs_s newsrs = CoordinateDescriptor::GetSRSFromDefinition(ATdata->GetLocalSrs());
                    srs_s oldsrs = CoordinateDescriptor::GetSRSFromDefinition(ATData_->GetLocalSrs());

                    if (newsrs.type == coord_system_type_e::Unsupported || oldsrs.type == coord_system_type_e::Unsupported)
                    {
                        std::string msg = "coordinate is unsupported ";
                        msg += __LINE__;
                        LOGI(msg);
                        return AI3D_FAILURE;
                    }
                    
                    
                    

                    
                    
                    {

                        if (!UpdateATGroup(ATdata))
                        {
                            LOGD("block_data_ UpdateATGroup  Failed!");
                            return AI3D_FAILURE;
                        }
                    }
                    ATdata->ComputeDepths();
                    SetAT0(ATdata);
                    
                    GetCurrentATMutual()->SetPoint3DsStatus(true);
                    SetTiepointStatus(true);

                    
                    const bool constraint_similarity_applied =
                        ApplyConstraintScaleAfterAtMergeIfAvailable(ATdata);
                    if (constraint_similarity_applied) {
                        
                        if (!SyncPoseSidecarBinsAfterConstraintSimilarity()) {
                            LOGW("SyncPoseSidecarBinsAfterConstraintSimilarity returned false (see prior warnings).");
                        }
                    }

                    
                    block_info_.isSaved = false;
                    block_info_.isFinished = true;
                    if (!Save())
                    {
                        block_info_.isSaved = false;
                        block_info_.isFinished = false;

                        return AI3D_FAILURE;
                    }
                    auto timeconvert = [&](std::string starttimestr, std::string endtimestr)
                    {

                        std::tm time_start, time_end;
                        time_start.tm_year = std::atoi(starttimestr.substr(0, 4).c_str()) - 1900;
                        time_start.tm_mon = std::atoi(starttimestr.substr(4, 2).c_str()) - 1;
                        time_start.tm_mday = std::atoi(starttimestr.substr(6, 2).c_str());
                        time_start.tm_hour = std::atoi(starttimestr.substr(8, 2).c_str()) - 8;
                        time_start.tm_min = std::atoi(starttimestr.substr(10, 2).c_str());
                        time_start.tm_sec = std::atoi(starttimestr.substr(12, 2).c_str());

                        time_end.tm_year = std::atoi(endtimestr.substr(0, 4).c_str()) - 1900;
                        time_end.tm_mon = std::atoi(endtimestr.substr(4, 2).c_str()) - 1;
                        time_end.tm_mday = std::atoi(endtimestr.substr(6, 2).c_str());
                        time_end.tm_hour = std::atoi(endtimestr.substr(8, 2).c_str()) - 8;
                        time_end.tm_min = std::atoi(endtimestr.substr(10, 2).c_str());
                        time_end.tm_sec = std::atoi(endtimestr.substr(12, 2).c_str());

                        time_t start, end;
                        start = mktime(&time_start);
                        end = mktime(&time_end);
                        double esplasetime = difftime(end, start);

                        
                        esplasetime < 0 ? 0 : esplasetime;
                        int hours = esplasetime / 3600;
                        hours < 0 ? 0 : hours;
                        int left_secs = esplasetime - hours * 3600;
                        left_secs < 0 ? 0 : left_secs;
                        int mins = left_secs / 60;
                        mins < 0 ? 0 : mins;
                        int secs = left_secs % 60;
                        secs < 0 ? 0 : secs;
                        std::string hourstr = (hours < 10) ? ("0" + std::to_string(hours)) : std::to_string(hours);
                        std::string minstr = (mins < 10) ? ("0" + std::to_string(mins)) : std::to_string(mins);
                        std::string secstr = (secs < 10) ? ("0" + std::to_string(secs)) : std::to_string(secs);
                        std::string timestr = hourstr + ":" + minstr + ":" + secstr;
                        return timestr;
                    };
                    
                    std::string timefile = "";
                    if (JOB_FEEDBACK_USE_BIN) {
                        timefile = MAKE_TIMESUM_BIN_FILE(File::EnsureUnifySlash(GetPath()), block_info_.job_);
                    }
                    else {
                        timefile = MAKE_TIMESUM_JSON_FILE(File::EnsureUnifySlash(GetPath()), block_info_.job_);
                        
                    }
                    ATTimeSummary_s attimesum;
                    if (std::filesystem::exists(File::BoostPathFromUtf8(timefile)) && (attimesum.load(timefile)))
                    {

                        std::string starttime = attimesum.runinfo.runninginfo.StartTime.c_str();
                        std::string endtime = attimesum.runinfo.runninginfo.EndTime.c_str();
                        if (starttime == "" || endtime == "")
                        {
                            LOGI(" starttime or endtime is empty. ");
                            return AI3D_FAILURE;
                        }

                          
                          

                        block_info_.ATTotalTime = timeconvert(starttime, endtime);
                        
                        for (auto iter : attimesum.tasksmap)
                        {
                            int idex = iter.first;
                            std::string  starttime = (attimesum.tasksmap.at(idex).StartTime.c_str());
                            std::string  endtime = (attimesum.tasksmap.at(idex).EndTime.c_str());
                            if (attimesum.tasksmap.at(idex).FunctionName == "")
                            {
                                continue;
                            }
                            block_info_.stageTotalTime[attimesum.tasksmap.at(idex).FunctionName] = timeconvert(starttime, endtime);
                        }
                    }
                    else
                    {
                        return AI3D_FAILURE;
                    }
                    GenerateATReport();
                    
                    
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return AI3D_FAILURE;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return AI3D_FAILURE;
            }
            return AI3D_SUCCESS;
        }
        int BlockObject::GenerateATReport()
        {
            
            ATReport at_report;
            ParseATReport(at_report);
            std::string ATReportHtml = GetPath() + "/ATReport.html";
            bool bretoutput = ExportATReport(at_report, ATReportHtml);
            if (!bretoutput)
            {
                return AI3D_FAILURE;
            }
            
            std::string icons_path = GetPath() + PATH_SEPARATOR_STR + "icons";
            File::CreateDirIfNotExists(icons_path);
            

            

            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            

            return AI3D_SUCCESS;
        }
            
        bool BlockObject::CanSubmitRecon()
        {
            if (GetCurrentATMutual()!=nullptr)
            {
                bool tiepointstatus = GetTiepointFullStatus();

                
                

                if (GetCurrentATMutual()->GetNumRegImages() > 2 && tiepointstatus)
                {
                

                
                

                    return true;
                }

                
                
            }

            
            
            return false;
        }
        std::string BlockObject::GetIdString()
        {
            return "Block_" + std::to_string(id_);
        }

        
        bool BlockObject::Load(const std::string& file_path, bool parsebin, BlockImportOptions block_import_options)
        {
            
            LOGI(std::string("loading file  " + file_path));
            std::string ext = File::GetFileExtension(file_path);
            String::StringToLower(&ext);

            if (ext == BLOCKBINFILE)
            {
                block_info_.ReadBlockInfoBin(file_path);
            }
            else if (ext == BLOCKFILE) {
                block_info_.ReadBlockInfoJson(file_path);
            }
            else {
                return false;
            }
                     
            
            try
            {

                id_ = block_info_.blockId;
                std::string jobname = block_info_.job_;
                std::string jobpath = path_ + "/" + jobname + "/";
                if (jobname != "" && GetJobType(block_info_.job_) == JOB_AT)
                {
                    
                    
                    std::string feedbackfile = "";
                    std::string timefile = "";
                    if (JOB_FEEDBACK_USE_BIN) {
                        
                        feedbackfile = MAKE_FEEDBAK_BIN_FILE(path_, jobname);
                        timefile = MAKE_TIMESUM_BIN_FILE(path_, jobname);
                    }
                    else {
                        feedbackfile = MAKE_FEEDBAK_JSON_FILE(path_, jobname);
                        timefile = MAKE_TIMESUM_BIN_FILE(path_, jobname);
                    }
                    
                    JobFeedBack_s feadback;
                    std::string context;
                    bool isfeedbackok = true;
                    if (!std::filesystem::exists(File::BoostPathFromUtf8(feedbackfile)))
                    {

                        LOGW(String::StringPrintf("File: %s is not exists", feedbackfile.c_str()));
                        isfeedbackok = false;
                        
                        
                    }
                    else
                    {
                        if (JOB_FEEDBACK_USE_BIN) {
                            std::ifstream in = File::OpenIfstreamUtf8(feedbackfile, std::ios::binary);
                            
                            if (!in.is_open())
                                isfeedbackok = false;
                            FeedBackFile feedBackFile;
                            feedBackFile.Deserialize(in);

                            feadback.Status = (jobsta_e)feedBackFile.feedBackData.status;
                            status_ = (feadback.Status);
                            feadback.Percent = feedBackFile.feedBackData.percent;
                            std::string msg = feedBackFile.feedBackData.msg;
#ifdef WIN32
                            // msg = UTF82GBK(msg);
#endif 
                            feadback.Msg = msg;

                            
                            isfeedbackok = true;
                            in.close();
                        }
                        else {
                            bool ret = RapidJsonCore::ReadFile(feedbackfile, context);

                            if (ret)
                            {
                                if (context.empty())
                                {
                                    isfeedbackok = false;
                                }

                                
                                rapidjson::Document doc;
                                if (doc.Parse(context.data()).HasParseError())
                                {
                                    isfeedbackok = false;
                                }

                                if (!doc.IsObject())
                                {
                                    isfeedbackok = false;
                                }

                                if (!doc.HasMember("Status"))
                                {
                                    isfeedbackok = false;
                                }
                                rapidjson::Value& status = doc["Status"];
                                feadback.Status = (jobsta_e)status.GetInt();

                                status_ = (feadback.Status);

                                
                            }
                            else
                            {
                                
                                
                                isfeedbackok = false;
                            }
                        }
                        
                        

                    }
                    if (!isfeedbackok)
                    {
                        
                        
                        
                        BlockObject blocktemp(*this);
                        if (supportTempLogs())
                        {
                            std::ostringstream oss;
                            oss << "create bo:" << std::hex << std::showbase << &blocktemp << std::dec;
                        
                        }
                        auto Atdata = std::make_shared<ATData>();
                        bool recoversucess = false;
                        
                        if (LoadBlockATData(Atdata, block_import_options))
                        {
                            if (status_ == STATUS_COMPLETE)
                            {
                                recoversucess = true;
                            }
                            else if (status_ <= STATUS_NEW  )
                            {
                                
                                
                                
                                
                                
                                
                                
                                
                                
                                SetATData(Atdata);
                                block_info_.isFinished = false;
                                block_info_.isSaved = false;
                                
                                status_ = STATUS_NEW;
                                recoversucess = true;
                                ExportATBinaryWithoutTiepoints(block_info_.Block_XML);
                                
                                
                                std::vector<std::string> files = File::GetFileList(path_);
                                
                                for (std::vector<std::string>::iterator iter = files.begin();
                                    iter != files.end();)
                                {
                                    std::string filenow = *iter;
                                    filenow = File::EnsureUnifySlash(filenow);
                                    std::string filebin = File::EnsureUnifySlash(block_info_.Block_XML);
                                    std::string fileblk = File::EnsureUnifySlash(file_path);
                                    if (filenow == filebin || filenow == fileblk)
                                    {
                                        iter = files.erase(iter);
                                    }
                                    else
                                    {
                                        iter++;
                                        
                                    }

                                }
                                
                                File::RemoveFiles(files);
                                if(File::ExistsDir(jobpath))
                                    File::Remove(jobpath);
                                
                                return true;
                            }                               
                        }

                        
                        
                        
                        if (!recoversucess)
                        {
                            status_ = jobsta_e::STATUS_UNKNOWN;
                            return false;
                        }
                    }
                }
                else
                {
                    status_ = (jobsta_e::STATUS_NEW);

                }

                if (!block_info_.isFinished && status_ == STATUS_COMPLETE)
                {
                    parsebin = true;

                }
                

                if (parsebin)
                {
                    auto Atdata = std::make_shared<ATData>();
                      
                    if (!LoadBlockATData(Atdata, block_import_options))
                        return false;
                    


                    
                    if(0)
                    {
                        if (status_ == STATUS_COMPLETE)
                        {
                            if (!ATGroups_.count(0))
                            {
                                UpdateCompleteATFile();
                                if(0)
                                {

                                    bool bgcpat = false, hasat = false;
                                    std::string atxml_0 = "", atxml_1 = "", atxml = "", atbin = "";


                                    atxml_0 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT_absolute.xml");
                                    atxml_1 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT_absolute");
                                    bgcpat = File::IsFileExistent(atxml_0) || File::IsFileExistent(atxml_1);
                                    if (bgcpat)
                                    {
                                        atxml = File::IsFileExistent(atxml_0) ? atxml_0 : atxml_1;
                                    }

                                    if (!bgcpat)
                                    {
                                        atxml_0 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT.xml");
                                        atxml_1 = File::EnsureUnifySlash(path_ + PATH_SEPARATOR_STR + "block_AT");
                                        hasat = File::IsFileExistent(atxml_0) || File::IsFileExistent(atxml_1);
                                        if (hasat)
                                        {
                                            atxml = File::IsFileExistent(atxml_0) ? atxml_0 : atxml_1;
                                        }
                                    }
                                    if (atxml != "")
                                    {
                                        auto Atdatatemp = std::make_shared<ATData>();
                                        BlockObject blocknew;
                                        blocknew.LoadATXML(atxml, Atdatatemp);
                                        if (supportTempLogs())
                                        {
                                            std::ostringstream oss;
                                            oss << "create bo:" << std::hex << std::showbase << &blocknew << std::dec;
                                        
                                        }
                                        ATGroups_[0].SetATData(Atdatatemp);
                                        
                                        ExportATBinaryWithoutTiepoints(block_info_.Block_XML);
                                        ExportTiepointsBinary(block_info_.Tiepoints);
                                        block_info_.isFinished = true;
                                    }
                                }
                            }
                        }
                    }

                    
                }
                
                if (!block_info_.reconstructions_info_.empty())
                {
                    struct rpt_s
                    {
                        reconstruction_t r_id;
                        production_t p_id;
                        std::string tile_name;
                        
                    };
                    std::vector<rpt_s> tiles_to_update;
                    
                        

                    for (auto& info : block_info_.reconstructions_info_)
                    {


                        
                        

                        

                        ReconstructionObject* object  =  new ReconstructionObject(id_);
                        
                        object->SetId(info.id_);
                        std::string recpath = path_ + "/" + object->GetIDString();
                        recpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(recpath)));
                        object->SetPath(recpath);
                        
                        object->SetName(info.name_);
                        object->SetProcessingSettings(info.processing_settings_);
                        
                        
                        object->SetBoundaryCustom(info.boundary_custom_);
                        
                        
                        
                        object->SetBoundingBoxCustom(info.boundingbox_custom_);
                        object->SetCustomSrs(info.srs_custom_);
                        
                        Tiling* discriptor = TilingGenaratorFactory(info.tile_params_);
                        object->SetTilingDisriptor(discriptor);
                        object->SetTilesCustom(info.tiles_);
                        if (GetCurrentAT() == nullptr)
                        {
                            continue;
                        }
                        ATData atdata = *GetCurrentAT();
                        
                        srs_s  srs;
                        

                        std::string reconstructionpath = path_ + "/" + object->GetIDString();
                        std::string srsFile = "";
                        if (SRS_USE_BIN) {
                            srsFile = SRSBIN;
                        }
                        else {
                            srsFile = SRSJSON;
                        }
                        std::string localfile = reconstructionpath + "/" + srsFile;
                        localfile = File::EnsureUnifySlash(localfile);
                        LOGI("localfile:"+ localfile);
                        bool ret = false;
                        if (SRS_USE_BIN) {
                            ret = Task_Info::LoadLocalBin(srs, localfile);
                            if (!ret)
                            {
                                srs = atdata.GetDefaultEnuSRS();
                                Task_Info::WriteLocalBin(srs, localfile);
                            }
                        }
                        else {
                            ret = Task_Info::LoadLocalJson(srs, localfile);
                            if (!ret)
                            {
                                srs = atdata.GetDefaultEnuSRS();
                                Task_Info::WriteLocalJson(srs, localfile);
                            }
                        }

                        object->GetBaseSrsMutual() = srs;
                        
                        

                        atdata.TransFormATData(srs.definition);
                            
        
                            object->GetATDataMutual() = atdata;
                            
                            
                            auto ATdataCustom = atdata;
                            ATdataCustom.TransFormATData(info.srs_custom_.definition);
                            object->GetATDataCustomMutual() = ATdataCustom;

                            
                            
                            std::string constraintpath = recpath + "/Constraint";
                            constraintpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(constraintpath)));
                            std::string constraintfile = constraintpath + "constraint.xml";
                            if (File::ExistsPath(constraintpath) && File::ExistsDir(constraintpath)&& File::ExistsFile(constraintfile))
                            {
                                
                                object->LoadGlobalConstraintFile(constraintfile);
                            }
                           


                        
                        
                            
                        
                            

                        for (auto& iterproduction : info.production_infos_)
                        {
                            production_option_s options = iterproduction.options_;
                            options.id_ = iterproduction.id_;
                            ProductionObject* productionobj = new ProductionObject(options,object);

                            productionobj->SetName(iterproduction.options_.name_);
                            productionobj->SetId(iterproduction.id_);
                            std::string recpath = object->GetPath();

                            recpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(recpath)));
                            recpath += PRODUCTION_DIR;
                            recpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(recpath)));
                            recpath += productionobj->GetIDString();
                            productionobj->SetPath(recpath);
                            EIGEN_STL_UMAP(std::string, production_tileinfo_s) tilestemp;
                            const std::string brp_prefix = "B" + std::to_string(id_) + "R"
                                + std::to_string(object->GetId()) + "P" + std::to_string(iterproduction.id_);

                            for (auto iter : iterproduction.options_.tiles_)
                            {
                                tilestemp[iter].name_ = iter;
                                const std::string job_key = brp_prefix + iter;
                                if (block_info_.reconstructionjobs_.count(job_key))
                                {
                                    tilestemp[iter].jobstr_ = block_info_.reconstructionjobs_.at(job_key);
                                }
                            }

                            productionobj->SetTiles(tilestemp);
                            ReconstructionCommandSet::SyncProductionTileJobStrs(this, object, productionobj);
                            object->AddProduction(productionobj);
                            
                            
                            for (auto itertile : iterproduction.options_.tiles_)
                            {
                                auto tilestring = itertile;
                                if (info.tiles_.count(tilestring))
                                {
                                    bool bshouldupate = info.tiles_.at(tilestring).reference_model_status_ != tile_info_s::reconst_status_e::RE_STA_COMPLETED;
                                    if (bshouldupate)
                                    {
                                        
                                        rpt_s  rpt;
                                        rpt.p_id = iterproduction.id_;
                                        rpt.r_id = info.id_;
                                        rpt.tile_name = tilestring;
                                        tiles_to_update.push_back(rpt);
                                        
                                    }
                                }
                            }
                            
                        }
                        
                        AddReconstruction(object);


                    }


                    
                    
                    



                    for (int i = 0; i < tiles_to_update.size(); i++)
                    {
                        
                        rpt_s rpt = tiles_to_update[i];
                        ReconstructionObject* object = reconstructions_[rpt.r_id];
                        std::string rpath = object->GetIDString();
                        ProductionObject* pro = object->GetProduction(rpt.p_id);
                        std::string ppath = "Productions/" + pro->GetIDString();
                        std::string tpath = rpt.tile_name;
                        
                        std::string rptstring = "B" + std::to_string(id_) + "R" + std::to_string(rpt.r_id) + "P" + std::to_string(rpt.p_id);
                        std::string rptjobstring = rptstring + tpath;
                        
                        if (!block_info_.reconstructionjobs_.count(rptjobstring))
                        {
                            continue;
                        }
                        std::string jobstring = block_info_.reconstructionjobs_.at(rptjobstring);
                        pro->GetTilesMutual().at(tpath).jobstr_ = jobstring;
                        std::string blockitembase_path = path_ + "/" + rpath + "/" + ppath+"/" + tpath+"/";
                        std::string feedbackName = "";
                        if (JOB_FEEDBACK_USE_BIN) {
                            feedbackName = MAKE_FEEDBAK_BIN_FILE(blockitembase_path, jobstring);
                        }
                        else {
                            feedbackName = MAKE_FEEDBAK_JSON_FILE(blockitembase_path, jobstring);
                            
                        }
                        std::string feedbackpath = File::EnsureUnifySlash(feedbackName);
                        
                        if (!File::ExistsFile(feedbackpath))
                        {
                            
                            pro->GetTilesMutual().at(tpath).status_ = jobsta_e::STATUS_UNKNOWN;
                            object->GetTilesCustomMutual().at(tpath).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_UNPROCESSED;
                            continue;
                        }
                        

                        JobFeedBack_s feedback;
                        bool ret = feedback.load_with_retry(feedbackpath);
                        LOGI("1==========feedbackpath===== " + feedbackpath);
                        if(!ret  )
                        {
                            pro->GetTilesMutual().at(tpath).status_ = jobsta_e::STATUS_UNKNOWN;
                            object->GetTilesCustomMutual().at(tpath).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_UNPROCESSED;
                            continue;
                             
                        }
                        
                        
                        pro->GetTilesMutual().at(tpath).status_ = feedback.Status;
                        if (pro->GetTilesMutual().at(tpath).status_ == jobsta_e::STATUS_COMPLETE)
                        {
                            object->GetTilesCustomMutual().at(tpath).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_COMPLETED;
                        }

                    }
                    

                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return true;
        }
        bool BlockObject::Save()
        {
            
            if (block_info_.isSaved)
            {
                return true;
            }
            
            
            std::string propath = path_;

            try
            {
                
                if (!std::filesystem::exists(File::BoostPathFromUtf8(propath)))
                {
                    if (!std::filesystem::create_directory(File::BoostPathFromUtf8(propath)))
                    {
                        LOGI("create" + propath + " failed when save block" +  std::to_string(id_));
                        return false;
                    }
                }
                if (GetCurrentAT()->HasConstraints()) {
                    std::string constraint_path = File::EnsureUnifySlash(propath + PATH_SEPARATOR_STR + CONSTRAINTFILE);
                    GetCurrentAT()->SaveConstraint(constraint_path);
                    Task_Info taskinfo = GetTaskInfoMutual();
                    taskinfo.at_options.sfmsettings.bapolicies.use_constraints_ = true;
                    taskinfo.at_options.sfmsettings.bapolicies.constraint_path_ = constraint_path;
                }
                

                
                std::string file_path = "";
                std::string bak_path = "";
                if (BLK_USE_BIN) {
                    
                    
                    file_path = File::EnsureUnifySlash(propath + PATH_SEPARATOR_STR + name_ + BLOCKBINFILE);
                    bak_path = File::EnsureUnifySlash(propath + PATH_SEPARATOR_STR + name_ + BLOCKBINBAKFILE);
                }
                else {
                    
                    
                    file_path = File::EnsureUnifySlash(propath + PATH_SEPARATOR_STR + name_ + BLOCKFILE);
                    bak_path = File::EnsureUnifySlash(propath + PATH_SEPARATOR_STR + name_ + BLOCKBAKFILE);
                }
                std::string file_path_tmp = file_path + "_tmp";
                if (File::IsFileExistent(file_path))
                {
                    std::filesystem::copy_file(File::BoostPathFromUtf8(file_path), File::BoostPathFromUtf8(bak_path), std::filesystem::copy_options::overwrite_existing);
                }

                
                if (!ExportBlockATData())
                {
                    LOGE(std::string("Save: ExportBlockATData failed block=") + GetIdString());
                    return false;
                }
                if (GetCurrentAT() == nullptr)
                {
                    return false;
                }
                
                if (GetTiepointStatus())
                {

                    GetTaskInfoMutual().statisticinfo_.tiepointnum = GetCurrentAT()->GetPoint3DIds().size();

                }
                bool  singleblock = false;
                if (singleblock)
                {
                    block_info_.statisticinfo_.imagenum = GetCurrentAT()->GetImagesIds().size();
                    block_info_.statisticinfo_.gcpnum = GetCurrentAT()->GetControlPoints().size();
                    block_info_.statisticinfo_.regisimagenum = GetCurrentAT()->GetRegImageIds().size();
                }
                if (block_info_.job_ != "") {
                    if (BLK_USE_BIN) {
                        block_info_.WriteBlockInfoToBin(file_path_tmp, true);
                    }
                    else {
                        block_info_.WriteBlockInfoToJson(file_path_tmp, true);
                    }
                }
                else {
                    if (BLK_USE_BIN) {
                        block_info_.WriteBlockInfoToBin(file_path_tmp, false);
                    }
                    else {
                        block_info_.WriteBlockInfoToJson(file_path_tmp, false);
                    }
                    
                }
                

                
                std::filesystem::copy_file(File::BoostPathFromUtf8(file_path_tmp), File::BoostPathFromUtf8(file_path), std::filesystem::copy_options::overwrite_existing);
                std::filesystem::remove(File::BoostPathFromUtf8(file_path_tmp));
                if (std::filesystem::exists(File::BoostPathFromUtf8(bak_path)))
                    std::filesystem::remove(File::BoostPathFromUtf8(bak_path));


                {
                    if (!GetReconstructions().empty())
                    {
                        
                        
                        for (auto& iter : GetReconstructions())
                        {
                            std::string productiontile_dir = path_ + "/" + iter.second->GetIDString() + "/";
                            productiontile_dir =AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(productiontile_dir));
                                

                                iter.second->WriteTiles(productiontile_dir);
                            
                        }
                    }
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }
            

            block_info_.isSaved = true;
            return true;
        }

        
        bool BlockObject::Task_Info::WriteBlockInfoToJson(const std::string& blk_fullpath,bool exportatsetting)
        {
            
            
            rapidjson::Document document;
            document.SetObject();

            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

            rapidjson::Value root(rapidjson::kObjectType);
            root.AddMember("blkString", rapidjson::Value(blockString.c_str(), allocator), allocator);
            root.AddMember("mergedFrom", rapidjson::Value(mergedFrom.c_str(), allocator), allocator);
            
            
            
            
            
            root.AddMember("blkId", rapidjson::Value(blockId), allocator);
            root.AddMember("job", rapidjson::Value(job_.c_str(), allocator), allocator);
            root.AddMember("isFinished", rapidjson::Value(isFinished), allocator);
            root.AddMember("btopredict_", rapidjson::Value(btopredict_), allocator);
            
            root.AddMember("AT_Num", rapidjson::Value(AT_Num), allocator);
            std::string block_bin = "./" + File::GetPathBaseName(Block_XML);
            
            root.AddMember("BlockXML", rapidjson::Value(block_bin.c_str(), allocator), allocator);
            std::string tiepoints_bin = "./" + File::GetPathBaseName(Tiepoints);
            
            root.AddMember("Tiepoints", rapidjson::Value(tiepoints_bin.c_str(), allocator), allocator);


            // std::string atjson2_ = GBK2UTF8(atjson_);
            std::string atjson2_ = atjson_;
            // std::string gcpjson2_ = GBK2UTF8(gcpjson_);
            std::string gcpjson2_ = gcpjson_;

            if (atjson_ != "")
                root.AddMember("ATJson", rapidjson::Value(atjson2_.c_str(), allocator), allocator);
            if (gcpjson_ != "")
                root.AddMember("GCPJson", rapidjson::Value(gcpjson2_.c_str(), allocator), allocator);


            rapidjson::Value statisticinfo(rapidjson::kObjectType);
            bool  singleblock = false;
            
            if (singleblock)
            {
                    statisticinfo.AddMember("imageNum", rapidjson::Value(statisticinfo_.imagenum), allocator);
                    statisticinfo.AddMember("gcpNum", rapidjson::Value(statisticinfo_.gcpnum), allocator);
                    statisticinfo.AddMember("registerdimageNum", rapidjson::Value(statisticinfo_.regisimagenum), allocator);
            }
            std::string msg = std::to_string(blockId) + __FUNCTION__ + " ******** ";
            msg+=std::to_string(statisticinfo_.tiepointnum);
            LOGI(msg);
            statisticinfo.AddMember("tiepointNum", rapidjson::Value(statisticinfo_.tiepointnum), allocator);
            root.AddMember("blockStatistics", statisticinfo, allocator);

            // genration -begin
            root.AddMember("block_task_category", rapidjson::Value(block_task_category), allocator);
            rapidjson::Value genOptionsJson(rapidjson::kObjectType);
            gen_options.WriteToJson(genOptionsJson, document);
            root.AddMember("gen_options", genOptionsJson, allocator);

            rapidjson::Value generationsInfo(rapidjson::kArrayType);
            for (auto& gen : generations_info_)
            {
                rapidjson::Value genJson(rapidjson::kObjectType);
                gen.CreateJson(genJson, document);
                generationsInfo.PushBack(genJson, allocator);
            }
            root.AddMember("generations_info", generationsInfo, allocator);

            rapidjson::Value genJobs(rapidjson::kArrayType);
            for (auto& jobstr : generationjobs_)
            {
                std::string combined = jobstr.first + ":" + jobstr.second;
                genJobs.PushBack(rapidjson::Value(combined.c_str(), allocator), allocator);
            }
            root.AddMember("GenJobs", genJobs, allocator);

            // generation -end
            rapidjson::Value settings(rapidjson::kObjectType);
            at_options.WriteToJson(settings, document);                                                                                    
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            if (exportatsetting)
            {

                root.AddMember("ATsettings", settings, allocator);
                
            }
            
            if (reconstructions_info_.size() > 0)
            {
                rapidjson::Value reconstructions(rapidjson::kArrayType);

                for (auto iter : reconstructions_info_)
                {
                    rapidjson::Value reconstructionjson(rapidjson::kObjectType);
                    iter.CreateJson(reconstructionjson, document);

                    reconstructions.PushBack(reconstructionjson, allocator);
                }
                root.AddMember("recontructions", reconstructions, allocator);

                rapidjson::Value reconstructionjobs(rapidjson::kArrayType);
                for (auto& jobstr : reconstructionjobs_)
                {
                    std::string combined = jobstr.first + ":" + jobstr.second;
                    reconstructionjobs.PushBack(rapidjson::Value(combined.c_str(), allocator), allocator);

                }
                root.AddMember("BRPJobs", reconstructionjobs, allocator);
                

            }

            document.AddMember("block", root, allocator);
         
            if (AI3D_SUCCESS!=RapidJsonCore::SaveFile(blk_fullpath, document))
            {
                LOGE(String::StringPrintf("Saving Block_%d .blk failed.", blockId));
                return false;
            }
            return true;
        }

        bool BlockObject::Task_Info::WriteBlockInfoToBin(const std::string& blk_fullpath, bool exportatsetting)
        {
            std::ofstream out = File::OpenOfstreamUtf8(blk_fullpath, std::ios::binary);
            
            if (!out.is_open()) {
                LOGE("Save blk bin failed!");
                return false;
            }
            BLKBinFile bLKBinFile;
            bLKBinFile.blkString = blockString;
            bLKBinFile.mergedFrom = mergedFrom;
            bLKBinFile.blkId = blockId;
            bLKBinFile.job = job_;
            bLKBinFile.isFinished = isFinished;
            bLKBinFile.btoPredict = btopredict_;
            bLKBinFile.AT_Num = AT_Num;
            std::string block_bin = "./" + File::GetPathBaseName(Block_XML);            
            std::string tiepoints_bin = "./" + File::GetPathBaseName(Tiepoints);
#ifdef WIN32
            // block_bin = GBK2UTF8(block_bin);
            // tiepoints_bin = GBK2UTF8(tiepoints_bin);
#endif 
            bLKBinFile.BlockXML = block_bin;
            bLKBinFile.Tiepoints = tiepoints_bin;
            bLKBinFile.hasAT = false;
            bLKBinFile.hasGCP = false;

            std::string atjson2_ = atjson_;
            std::string gcpjson2_ = gcpjson_;
#ifdef WIN32
            // atjson2_ = GBK2UTF8(atjson2_);
            // gcpjson2_ = GBK2UTF8(gcpjson2_);
#endif 

            if (atjson_ != ""){
                bLKBinFile.hasAT = true;
                bLKBinFile.ATJson = atjson2_;
            }
            if (gcpjson_ != "") {
                bLKBinFile.hasGCP = true;
                bLKBinFile.GCPJson = gcpjson2_;
            }

            bool  singleblock = false;
            
            if (singleblock)
            {
                
                
                
            }
            std::string msg = std::to_string(blockId) + __FUNCTION__ + " ******** ";
            msg += std::to_string(statisticinfo_.tiepointnum);
            LOGI(msg);
            bLKBinFile.tiepointNum = statisticinfo_.tiepointnum;

            bLKBinFile.gen_block_task_category = block_task_category;
            bLKBinFile.gen_params_json = gen_options.gen_params.ToJsonString();
            {
                rapidjson::Document doc;
                doc.SetArray();
                auto& allocator = doc.GetAllocator();
                for (auto& gen : generations_info_) {
                    rapidjson::Value genJson(rapidjson::kObjectType);
                    gen.CreateJson(genJson, doc);
                    doc.PushBack(genJson, allocator);
                }
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                doc.Accept(writer);
                bLKBinFile.gen_info_json = buffer.GetString();
            }
            bLKBinFile.genJobNum = generationjobs_.size();
            for (auto& jobstr : generationjobs_)
                bLKBinFile.genJobVec.push_back(jobstr.first + ":" + jobstr.second);
            
            bLKBinFile.atSetting.keyNum = at_options.feature_num;
            bLKBinFile.atSetting.maxthreads_num = at_options.maxthreads_num;
            bLKBinFile.atSetting.minOverlap = at_options.saveoptions.min_overlap;
            bLKBinFile.atSetting.maxOverlap = at_options.saveoptions.max_overlap;
            bLKBinFile.atSetting.maxTieptNum = at_options.saveoptions.max_tiepoint_num;
            bLKBinFile.atSetting.mode = at_options.sfmsettings.sfm_mode;
            bLKBinFile.atSetting.ba1_grid_count = at_options.sfmsettings.grid_count_1;
            bLKBinFile.atSetting.ba2_grid_count = at_options.sfmsettings.grid_count_2;
            bLKBinFile.atSetting.max_feature_count_1 = at_options.sfmsettings.max_feature_count_1;
            bLKBinFile.atSetting.max_feature_count_2 = at_options.sfmsettings.max_feature_count_2;
            bLKBinFile.atSetting.output_tiepoint = at_options.saveoptions.boutput_tiepoint;
            bLKBinFile.atSetting.max_projection_error = at_options.saveoptions.max_projection_error;
            bLKBinFile.atSetting.reconstruct_mode = at_options.reconstruct_mode;
            bLKBinFile.atSetting.output_rawxml = at_options.saveoptions.output_rawxml;
            bLKBinFile.atSetting.use_user_tiepoints = at_options.sfmsettings.bapolicies.use_user_tiepoints_;
            
            std::string tmpTilePointPath = at_options.sfmsettings.bapolicies.usertiepoints_path_;
            if (bLKBinFile.atSetting.use_user_tiepoints && tmpTilePointPath != "") {
                bLKBinFile.atSetting.use_user_tiepoints = true;
#ifdef WIN32
                // tmpTilePointPath = GBK2UTF8(tmpTilePointPath);
#endif 
                bLKBinFile.atSetting.usertiepoints_path_ = tmpTilePointPath;
            }
            else {
                bLKBinFile.atSetting.use_user_tiepoints = false;
            }
            bLKBinFile.atSetting.use_gcp = at_options.sfmsettings.bapolicies.use_gcp_;
            std::string tmpGcpPath = at_options.sfmsettings.bapolicies.gcp_path_;
            if (bLKBinFile.atSetting.use_gcp && tmpGcpPath != "") {
                bLKBinFile.atSetting.use_gcp = true;
#ifdef WIN32
                // tmpGcpPath = GBK2UTF8(tmpGcpPath);
#endif 
                bLKBinFile.atSetting.control_point_path = tmpGcpPath;
            }
            else {
                bLKBinFile.atSetting.use_gcp = false;
            }
            bLKBinFile.atSetting.use_constraint = at_options.sfmsettings.bapolicies.use_constraints_;
            std::string tmpConstraintPath = at_options.sfmsettings.bapolicies.constraint_path_;
            if (bLKBinFile.atSetting.use_constraint && tmpConstraintPath != "") {
                bLKBinFile.atSetting.use_constraint = true;
#ifdef WIN32
                // tmpConstraintPath = GBK2UTF8(tmpConstraintPath);
#endif 
                bLKBinFile.atSetting.constraint_path = tmpConstraintPath;
            }
            else {
                bLKBinFile.atSetting.use_constraint = false;
            }
            bLKBinFile.atSetting.tiepoints_policy = at_options.sfmsettings.bapolicies.tiepoints_policy_;
            bLKBinFile.atSetting.pos_policy = at_options.sfmsettings.bapolicies.pos_policy_;
            bLKBinFile.atSetting.ppa_policy = at_options.sfmsettings.bapolicies.f_policy_;
            bLKBinFile.atSetting.rdis_policy = at_options.sfmsettings.bapolicies.ppa_policy_;
            bLKBinFile.atSetting.f_policy = at_options.sfmsettings.bapolicies.rdis_policy_;
            bLKBinFile.atSetting.tdis_policy = at_options.sfmsettings.bapolicies.tdis_policy_;
            bLKBinFile.atSetting.use_image_position_ = at_options.sfmsettings.bapolicies.use_image_position_;
            std::string tmp_image_pos_list = at_options.sfmsettings.bapolicies.pos_path_;
            if (bLKBinFile.atSetting.use_image_position_ && tmp_image_pos_list != "") {
                bLKBinFile.atSetting.use_image_position_ = true;
#ifdef WIN32
                // tmp_image_pos_list = GBK2UTF8(tmp_image_pos_list);
#endif 
                bLKBinFile.atSetting.image_pos_list = tmp_image_pos_list;
            }
            else {
                bLKBinFile.atSetting.use_image_position_ = false;
            }
            bLKBinFile.atSetting.hasATPath = false;
            std::string tmpATPath = at_options.sfmsettings.bapolicies.at_path_;
            if (tmpATPath != "") {
                bLKBinFile.atSetting.hasATPath = true;
#ifdef WIN32
                // tmpATPath = GBK2UTF8(tmpATPath);
#endif 
                bLKBinFile.atSetting.at_path = tmpATPath;
            }

            bLKBinFile.reconstructionNum = reconstructions_info_.size();
            bLKBinFile.jobNum = 0;
            
            if (reconstructions_info_.size() > 0)
            {
                for (auto iter : reconstructions_info_)
                {
                    ReconstrutionData reconstrutionData;
                    if (iter.id_ == kInvalidReconstructionId) {
                        return false;
                    }
                    reconstrutionData.id = iter.id_;
                    if (iter.name_ == "") {
                        return false;
                    }
                    reconstrutionData.name = iter.name_;
                    reconstrutionData.hasCoord = iter.srs_custom_.isValid();
                    if (reconstrutionData.hasCoord) {
                        coord_system_type_e type = iter.srs_custom_.type;
                        reconstrutionData.coordinateData.type = type;
                        if (type == coord_system_type_e::LOCAL_ENU)
                        {
                            std::string latlon_tmp = AI3D::CORE::String::StringSplit(iter.srs_custom_.definition, ":")[1];
                            std::string lat = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[0];
                            std::string lon = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[1];
                            reconstrutionData.coordinateData.ori[0] = std::atof(lat.c_str());
                            reconstrutionData.coordinateData.ori[1] = std::atof(lon.c_str());
                            reconstrutionData.coordinateData.ori[2] = 0.0;
                        }
                        else if (type == coord_system_type_e::LOCAL)
                        {

                        }
                        else
                        {
                            auto definitionstrs = AI3D::CORE::String::StringSplit(iter.srs_custom_.definition, ":");
                            std::string codeflag = definitionstrs[0];
                            std::string codestr = definitionstrs[1];
                            AI3D::CORE::String::StringToLower(&codeflag);
                            if (codeflag == "epsg")
                            {
                                reconstrutionData.coordinateData.espgStr = codestr;
                            }
                            else
                            {
                                OGRSpatialReference sr;
                                if (OGRERR_NONE == sr.importFromWkt(iter.srs_custom_.definition.c_str()))
                                {
                                    std::string codestr(sr.GetAuthorityCode(NULL));
                                    reconstrutionData.coordinateData.espgStr = codestr;
                                }

                            }

                        }
                    }
                    bbox_s bb(iter.boundingbox_custom_);
                    if (bb.isValid()) {
                         reconstrutionData.boundingbox_custom.hasBBbox = true;
                         reconstrutionData.boundingbox_custom.max[0] = bb.xmax_;
                         reconstrutionData.boundingbox_custom.max[1] = bb.ymax_;
                         reconstrutionData.boundingbox_custom.max[2] = bb.zmax_;
                         reconstrutionData.boundingbox_custom.min[0] = bb.xmin_;
                         reconstrutionData.boundingbox_custom.min[1] = bb.ymin_;
                         reconstrutionData.boundingbox_custom.min[2] = bb.zmin_;                      
                    }
                    reconstrutionData.hasBoundary = false;
                    if (!(iter.boundary_custom_.empty())) {
                        reconstrutionData.hasBoundary = true;
                        reconstrutionData.boundary_custom_.clear();
                        reconstrutionData.boundary_level1_size = iter.boundary_custom_.size();
                        for (int index = 0; index < iter.boundary_custom_.size(); index++)
                        {
                            std::vector< std::vector<double> > leve2;
                            reconstrutionData.boundary_level2_size = iter.boundary_custom_[index].size();
                            for (int indexj = 0; indexj < iter.boundary_custom_[index].size(); indexj++)
                            {
                                auto boundary = iter.boundary_custom_[index][indexj];
                                std::vector<double> leve3;
                                leve3.push_back(boundary.x());
                                leve3.push_back(boundary.y());
                                leve2.push_back(leve3);
                            }
                            reconstrutionData.boundary_custom_.push_back(leve2);
                        }
                    }
                    
                    tiling_mode_e tile_model = iter.tile_params_.mode_;
                    reconstrutionData.tillData.tiling_mode = tile_model;
                    reconstrutionData.tillData.expected_max_ram_used = iter.tile_params_.expected_max_ram_used_;
                    if (tile_model == tiling_mode_e::TILE_NONE)
                    {

                    }
                    else if (tile_model == tiling_mode_e::TILE_PALNAR_GRID || tile_model == tiling_mode_e::TILE_VOL_GRID || tile_model == tiling_mode_e::TILE_ADAPTIVE)
                    {
                        reconstrutionData.tillData.automatic_origin[0] = iter.tile_params_.regular_params_.automatic_origin_.x();
                        reconstrutionData.tillData.automatic_origin[1] = iter.tile_params_.regular_params_.automatic_origin_.y();
                        reconstrutionData.tillData.automatic_origin[2] = iter.tile_params_.regular_params_.automatic_origin_.z();

                        reconstrutionData.tillData.custom_origin[0] = iter.tile_params_.regular_params_.custom_origin_.x();
                        reconstrutionData.tillData.custom_origin[1] = iter.tile_params_.regular_params_.custom_origin_.y();
                        reconstrutionData.tillData.custom_origin[2] = iter.tile_params_.regular_params_.custom_origin_.z();

                        reconstrutionData.tillData.tileSize = iter.tile_params_.regular_params_.tilesize_;
                    }

                    reconstrutionData.tileNum = iter.tiles_.size();
                    for (auto& tile : iter.tiles_)
                    {
                        TileData tmpTileData;
                        tile_info_s tmpInfo = tile.second;
                        tmpTileData.index = tmpInfo.index_;
                        tmpTileData.name = tmpInfo.name_;
                        tmpTileData.status = (int)(tmpInfo.reference_model_status_);
                        tmpTileData.isempty = tmpInfo.isempty;
                        tmpTileData.hasBBbox = false;
                        bbox_s bb(tmpInfo.bb_.cast<double>());
                        if (bb.isValid())
                        {
                            tmpTileData.hasBBbox = true;
                            tmpTileData.bbox.hasBBbox = true;
                            tmpTileData.bbox.max[0] = bb.xmax_;
                            tmpTileData.bbox.max[1] = bb.ymax_;
                            tmpTileData.bbox.max[2] = bb.zmax_;
                            tmpTileData.bbox.min[0] = bb.xmin_;
                            tmpTileData.bbox.min[1] = bb.ymin_;
                            tmpTileData.bbox.min[2] = bb.zmin_;
                        }
                        tmpTileData.imageNum = tmpInfo.image_ids_.size();
                        tmpTileData.imageids.clear();
                        for (auto& iter : tmpInfo.image_ids_)
                        {
                            tmpTileData.imageids.push_back(iter);
                        }
                        reconstrutionData.tileVec.push_back(tmpTileData);
                    }
                    reconstrutionData.productionNum = iter.production_infos_.size();
                    for (auto& production : iter.production_infos_)
                    {
                        ProductionData tmpProductData;
                        
                        tmpProductData.id = production.id_;
                        std::string tmpName = production.options_.name_;
#ifdef WIN32
                        // tmpName = GBK2UTF8(tmpName);
#endif 
                        tmpProductData.name = tmpName;
                        tmpProductData.modelingsettings = production.options_.settings_str_;
                        tmpProductData.production_format = ProductionFormatStringToProcessing.at(production.options_.production_format_);
                        SRSData srsData;
                        std::string tmpDestin = production.options_.destination_;
#ifdef WIN32
                        // tmpDestin = GBK2UTF8(tmpDestin);
#endif 
                        if (tmpDestin != "") {
                            tmpProductData.destination = tmpDestin;
                        }
                        else {
                            return false;
                        }
                        
                        tmpProductData.tileSize = production.options_.tiles_.size();
                        tmpProductData.tiles.clear();
                        for (auto& iter : production.options_.tiles_)
                        {
                            tmpProductData.tiles.push_back(iter);
                        }

                        reconstrutionData.productionVec.push_back(tmpProductData);

                    }
                    reconstrutionData.Geometric_Level = (int)(iter.processing_settings_.level_);
                    reconstrutionData.ColorBalanced = iter.processing_settings_.bcolorbalance_;
                    int untexPolicy = (int)(iter.processing_settings_.untex_policy_);
                    reconstrutionData.Untexture_Fill_Mode = untexPolicy;
                    
                    if ((untexture_policy_e)untexPolicy == untexture_policy_e::UNTEX_COLOR_FILLED)
                    {
                        for (int n = 0; n < 3; n++)
                        {
                            reconstrutionData.Texture_Fill_Color[n] = iter.processing_settings_.texture_fill_color_[n];
                        }

                    }
                    reconstrutionData.DiscardEmptyTiles = iter.processing_settings_.bdiscard_emptytiles_;
                    reconstrutionData.HoleFillingMode = (int)(iter.processing_settings_.hollfilling_);

                    bLKBinFile.reconstrutionDataVec.push_back(reconstrutionData);
                }

                bLKBinFile.jobNum = reconstructionjobs_.size();
                for (auto& jobstr : reconstructionjobs_)
                {
                    std::string combined = jobstr.first + ":" + jobstr.second;
                    bLKBinFile.jobVec.push_back(combined);
                }
            }

            bLKBinFile.Serialize(out);

            out.close();
            return true;
        }

        EIGEN_STL_UMAP(group_t, ATGroup)& BlockObject::GetATGroupMutual()
        {
            return ATGroups_;
        }

        bool BlockObject::GetTiepointFullStatus()
        {
            if (GetCurrentAT() == nullptr)
            {
                return false;
            }
            bool tiepointstatus = (GetTiepointStatus()) ? GetCurrentAT()->HasTiepoints() : GetTaskInfoMutual().statisticinfo_.tiepointnum > 0;
            return tiepointstatus;
        }

        bool BlockObject::SaveCamBin(const std::string& filename)
        {
            std::ofstream ofile = File::OpenOfstreamUtf8(filename, std::ios::out | std::ios::binary);
            if (!ofile)
            {
                LOGE(String::StringPrintf("Writing %s failed!", filename.c_str()));
                return false;
            }
            try
            {
                int num_cams = photogroups_.size();
                ofile.write((char*)&num_cams, sizeof(int));
                for (const auto& iter : photogroups_)
                {
                    auto camera = iter.second.GetCamera();
                    camera_t camera_id = camera.GetCameraId();
                    
                    ofile.write((char*)&camera_id, sizeof(camera_t));

                    int model_id = camera.GetModelId();
                    ofile.write((char*)&model_id, sizeof(int));

                    size_t width, height;
                    width = camera.GetWidth();
                    height = camera.GetHeight();
                    ofile.write((char*)&width, sizeof(size_t));
                    ofile.write((char*)&height, sizeof(size_t));

                    CameraModelType_e cameramodeltype = camera.GetCameraModelType();
                    ofile.write((char*)&cameramodeltype, sizeof(CameraModelType_e));

                    
                    int num_cam_param = camera.GetParams().size();
                    ofile.write((char*)&num_cam_param, sizeof(int));
                    std::vector<double> params_0 = camera.GetParams();
                    if (num_cam_param > 0)
                    {

                         
                         
                         
                         
                         
                        ofile.write((char*)&params_0[0], sizeof(double) * num_cam_param);

                    }
                    int imgnum = iter.second.GetNumImages();
                    ofile.write((char*)&imgnum, sizeof(int));
                    for (auto& iterimgid : iter.second.GetGroupImageIds())
                    {
                        ofile.write((char*)&iterimgid, sizeof(image_t));
                    }
                }
                ofile.close();
            }
            catch (const std::exception& err)
            {
                LOGE(String::StringPrintf("Saving: %s failed! Msg: %s", filename.c_str(), err.what()));
                ofile.close();
                return false;
            }
            return true;
        }

        void BlockObject::LoadTiepoints()
        {
            if (GetCurrentATMutual() == nullptr)
            {
                return;
            }
            if (!GetTiepointStatus())
            {
                
                if (GetCurrentATMutual()->HasRegImages())
                {
                    if (LoadTiepointsBinary(GetTaskInfo().Tiepoints, GetCurrentATMutual()))
                    {
                        SetTiepointStatus(true);
                        GetTaskInfoMutual().statisticinfo_.tiepointnum = GetCurrentAT()->GetNumPoints3D();
                        GetCurrentATMutual()->ComputeDepths();
                    }
                }
            }
        }

        bool BlockObject::SaveImagePosListBin(const ATOptions& options)
        {
            const std::shared_ptr<ATData> src = GetCurrentAT() ? GetCurrentAT() : ATData_;
            ATData ATdata = *src;
            std::string file = path_ + "/" + POSBIN;
            std::ofstream out = File::OpenOfstreamUtf8(file, std::ios::binary);
            if (!out.is_open()) {
                LOGE("Writing POS file failed!");
                return AI3D_FAILURE;
            }
            POSFile posFile;

            std::string definition;
            srs_s srs;
            if (ATdata.GetLocalSrs().find("ENU") != std::string::npos)
            {
                definition = ATdata.GetLocalSrs();
                srs.type = coord_system_type_e::LOCAL_ENU;
            }
            else
            {
                srs = ATdata.GetDefaultEnuSRS();
                definition = srs.definition;
                {
                    
                    ATdata.TransFormImages(ATdata.GetLocalSrs(), srs.definition);
                }
            }
            posFile.type = srs.type;
            rapidjson::Value origin_point(rapidjson::kArrayType);
            std::string latlon_tmp = String::StringSplit(definition, ":")[1];
            std::string lat = String::StringSplit(latlon_tmp, ",")[0];
            std::string lon = String::StringSplit(latlon_tmp, ",")[1];
            posFile.ori[0] = atof(lat.c_str());
            posFile.ori[1] = atof(lon.c_str());
            posFile.ori[2] = 0.0;

            int posNum = ATdata.GetImages().size();
            posFile.posNum = posNum;
            posFile.posList.clear();
            for (const auto& img : ATdata.GetImages())
            {
                POSItem posItem;
                posItem.accuracy[0] = options.sfmsettings.pos_sigma[0];
                posItem.accuracy[1] = options.sfmsettings.pos_sigma[1];
                posItem.accuracy[2] = options.sfmsettings.pos_sigma[2];

                posItem.position[0] = img.second.GetPosition()[0];
                posItem.position[1] = img.second.GetPosition()[1];
                posItem.position[2] = img.second.GetPosition()[2];

                posItem.id = img.second.GetImageId();
                posFile.posList.push_back(posItem);
            }
            int fixSize = 0;
            posFile.fixNum = fixSize;
            posFile.fixIdList.clear();

            posFile.Serialize(out);
            out.close(); 
            return true;
        }
        bool BlockObject::SaveImagePosListJson(const ATOptions& options)
        {
            const std::shared_ptr<ATData> src = GetCurrentAT() ? GetCurrentAT() : ATData_;
            ATData ATdata = *src;

            rapidjson::Document document;
            
            
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

            std::string definition;
            srs_s srs;
            if (ATdata.GetLocalSrs().find("ENU") != std::string::npos)
            {
                definition = ATdata.GetLocalSrs();
                srs.type = coord_system_type_e::LOCAL_ENU;
            }
            else
            {
                srs = ATdata.GetDefaultEnuSRS();
                definition = srs.definition;
                {
                    
                    ATdata.TransFormImages(ATdata.GetLocalSrs(), srs.definition);
                }
            }


            rapidjson::Value origin_point(rapidjson::kArrayType);
            std::string latlon_tmp = String::StringSplit(definition, ":")[1];
            std::string lat = String::StringSplit(latlon_tmp, ",")[0];
            std::string lon = String::StringSplit(latlon_tmp, ",")[1];
            origin_point.PushBack(std::atof(lat.c_str()), allocator);
            origin_point.PushBack(std::atof(lon.c_str()), allocator);
            origin_point.PushBack(0, allocator);

            rapidjson::Value coordinate(rapidjson::kObjectType);
            coordinate.AddMember("origin_point", origin_point, allocator);
            coordinate.AddMember("type", rapidjson::Value(srs.type), allocator);

            rapidjson::Value items(rapidjson::kArrayType);
            for (const auto& img : ATdata.GetImages())
            {
                rapidjson::Value item(rapidjson::kObjectType);
                rapidjson::Value accuracy(rapidjson::kArrayType);
                rapidjson::Value pos(rapidjson::kArrayType);

                accuracy.PushBack(options.sfmsettings.pos_sigma[0], allocator);
                accuracy.PushBack(options.sfmsettings.pos_sigma[1], allocator);
                accuracy.PushBack(options.sfmsettings.pos_sigma[2], allocator);

                pos.PushBack(img.second.GetPosition()[0], allocator);
                pos.PushBack(img.second.GetPosition()[1], allocator);
                pos.PushBack(img.second.GetPosition()[2], allocator);

                item.AddMember("accuracy", accuracy, allocator);
                item.AddMember("id", rapidjson::Value(img.second.GetImageId()), allocator);
                item.AddMember("position", pos, allocator);

                items.PushBack(item, allocator);
            }

            document.AddMember("coordinate_system", coordinate, allocator);
            document.AddMember("items", items, allocator);

            

            if (RapidJsonCore::SaveFile((path_ + "/" + POSJSON), document ) != AI3D_SUCCESS)
            {
                LOG(ERROR) << "Save imageposlist.josn failed!";
                return false;
            }
            return true;
        }
        void BlockObject::ClearImageIds()
        {
            image_ids_.clear();
        }

        void BlockObject::UpdateGCPATReport(AcquisitionReport& gcp_accuracy, AcquisitionReport& checkpoint_accuracy)
        {
            if (!GetCurrentATMutual()->HasControlPoints())
            {
                return;
            }
            gcp_accuracy.gpt_type = gpt_e::GCP_CONTROL_HV;
            checkpoint_accuracy.gpt_type = gpt_e::GCP_CHECK_HV;         
            std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> measurement_error_map;
            GetCurrentATMutual()->UpdataGCPGlobalErrorInfo(measurement_error_map);
            std::vector<double> gcplist_pix, chklist_pix;
            double gcprms_pix = 0,chksrms_pix = 0;
            Eigen::Vector3d gcp_3derror, chkgcp_3derror;
            int gcpcount = 0, chkcount = 0;
            for (auto it : GetCurrentATMutual()->GetControlPoints())
            {
                Acquisition gcp_acq;
                gcp_acq.id = it.first;
                gcp_acq.rms_px = it.second.GetObjectPoint().GetPixelRMS();
                if ((gcp_acq.rms_px == kInvalidError || gcp_acq.rms_px == -DBL_MAX))
                    continue;
                double squarerms = gcp_acq.rms_px * gcp_acq.rms_px;
                double error3d_xx = it.second.GetX3DError() * it.second.GetX3DError();
                double error3d_yy = it.second.GetY3DError() * it.second.GetY3DError();
                double error3d_zz = it.second.GetZ3DError() * it.second.GetZ3DError();
                bool bvalidxyz = (it.second.GetGivenXYZ().x() != -DBL_MAX);
                bool bvalidetimatedxyz = (it.second.GetEstimatedXYZ().x() != -DBL_MAX);
                if (!(bvalidxyz && bvalidetimatedxyz))
                {
                    continue;
                }
                gcp_acq.error_z = it.second.GetZ3DError();
                gcp_acq.error_3d = it.second.Get3DError();
                gcp_acq.error_xy = it.second.GetXY3DError();
                gcp_acq.name = it.second.GetName();
                gcp_acq.num_observations = it.second.GetObjectPoint().GetTrack().Length();
                
                if (it.second.GetType() == gpt_e::GCP_CONTROL_HV)
                {
                    gcprms_pix += squarerms;
                    gcp_3derror.x() += error3d_xx;
                    gcp_3derror.y() += error3d_yy;
                    gcp_3derror.z() += error3d_zz;
                    gcpcount++;

                    gcp_accuracy.accuracy_vec.push_back(gcp_acq);
                    gcplist_pix.push_back(gcp_acq.rms_px);
                }
                else if (it.second.GetType() == gpt_e::GCP_CHECK_HV)
                {
                    chksrms_pix += squarerms;
                    chklist_pix.push_back(gcp_acq.rms_px);
                    chkcount++;
                    chkgcp_3derror.x() += error3d_xx;
                    chkgcp_3derror.y() += error3d_yy;
                    chkgcp_3derror.z() += error3d_zz;
                    
                    checkpoint_accuracy.accuracy_vec.push_back(gcp_acq);
                }
                
            }
            std::sort(gcplist_pix.begin(), gcplist_pix.end());
            std::sort(chklist_pix.begin(), chklist_pix.end());
            
            if (gcpcount > 0)
            {
                double gcp_rms_px_all = std::sqrt(gcprms_pix / gcpcount);
                gcp_accuracy.rms_px_all = gcp_rms_px_all;
                gcp_3derror.x() = std::sqrt(gcp_3derror.x() / gcpcount);
                gcp_3derror.y() = std::sqrt(gcp_3derror.y() / gcpcount);
                gcp_3derror.z() = std::sqrt(gcp_3derror.z() / gcpcount);
                gcp_accuracy.error_3d_all = gcp_3derror.norm();
                gcp_accuracy.error_xy_all = std::sqrt(gcp_3derror.x() * gcp_3derror.x() + gcp_3derror.y() * gcp_3derror.y());
                gcp_accuracy.error_z_all = gcp_3derror.z();
                gcp_accuracy.error_reproj = gcplist_pix[gcplist_pix.size()/2];
                gcp_accuracy.num_gcps = gcpcount; 

            }
            if (chkcount > 0)
            {
                double gcpchk_rms_px_all = std::sqrt(chksrms_pix / chkcount);
                checkpoint_accuracy.rms_px_all = gcpchk_rms_px_all;
                chkgcp_3derror.x() = std::sqrt(chkgcp_3derror.x() / chkcount);
                chkgcp_3derror.y() = std::sqrt(chkgcp_3derror.y() / chkcount);
                chkgcp_3derror.z() = std::sqrt(chkgcp_3derror.z() / chkcount);

                checkpoint_accuracy.error_3d_all = chkgcp_3derror.norm();
                checkpoint_accuracy.error_xy_all = std::sqrt(chkgcp_3derror.x() * chkgcp_3derror.x() + chkgcp_3derror.y() * chkgcp_3derror.y());
                checkpoint_accuracy.error_z_all = chkgcp_3derror.z();
                checkpoint_accuracy.error_reproj = chklist_pix[chklist_pix.size() / 2];;
                checkpoint_accuracy.num_gcps = chkcount;
            }
        }

        bool BlockObject::InterSectionAdjustment()
        {
            
            
            
            
            
            
            
            
            
            
            return true;
        }

        int BlockObject::Task_Info::LoadLocalJson(srs_s& srs, const std::string& file)
        {
            std::string context;
            bool ret = String::ReadFileToString(file, context);
            if (!ret)
            {
                LOGE("ReadFile Error");
                return AI3D_FAILURE;
            }
            if (context.empty())
            {
                LOGE("Json file is empty!");
                return AI3D_FAILURE;
            }

            
            rapidjson::Document doc;
            if (doc.Parse(context.data()).HasParseError())
            {
                LOGE("Parse Block Json file ERROR!");
                return AI3D_FAILURE;
            }

            if (!doc.IsObject())
            {
                LOGE("Json ilegal ERROR!");
                return AI3D_FAILURE;
            }

            if (!doc.HasMember("local_coordinate_system"))
            {
                LOGE("Json Memeber was lost!");
                return AI3D_FAILURE;
            }


            if (doc.HasMember("local_coordinate_system"))
            {
                srs.ParseJson(doc["local_coordinate_system"]);
            }
            return AI3D_SUCCESS;
        }

        int BlockObject::Task_Info::WriteLocalJson(const srs_s& srs, const std::string& file)
        {
            srs_s srs_temp = srs;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            rapidjson::Document documnet;
            documnet.SetObject();
            rapidjson::Document::AllocatorType& allocator = documnet.GetAllocator();

            rapidjson::Value coordinate(rapidjson::kObjectType);
            srs_temp.CreateJson(coordinate, documnet);
            documnet.AddMember("local_coordinate_system", coordinate, allocator);
            documnet.Accept(writer);

            std::ofstream fileout = File::OpenOfstreamUtf8(file, std::ios::out);
            if (!fileout.good())
                return AI3D_FAILURE;


            fileout << buffer.GetString();
            fileout.close();
            return AI3D_SUCCESS;
        }

        int BlockObject::Task_Info::WriteLocalBin(const srs_s& srs, const std::string& file)
        {
            srs_s srs_temp = srs;
            std::ofstream out = File::OpenOfstreamUtf8(file, std::ios::binary);
            if (!out.is_open()) {
                LOGE("Writing SRS file failed!");
                return AI3D_FAILURE;
            }
            SRSFile srsFile;
            coord_system_type_e type = srs_temp.type;
            srsFile.type = (int)type;

            if (type == coord_system_type_e::LOCAL_ENU)
            {
                std::string latlon_tmp = AI3D::CORE::String::StringSplit(srs_temp.definition, ":")[1];
                std::string lat = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[0];
                std::string lon = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[1];
                srsFile.ori[0] = std::stod(lat);
                srsFile.ori[1] = std::stod(lon);
                srsFile.ori[2] = 0.0;

            }
            else if (type == coord_system_type_e::LOCAL)
            {

            }
            else
            {
                auto definitionstrs = AI3D::CORE::String::StringSplit(srs_temp.definition, ":");
                std::string codeflag = definitionstrs[0];
                std::string codestr = definitionstrs[1];
                AI3D::CORE::String::StringToLower(&codeflag);
                if (codeflag == "epsg")
                {
                    srsFile.espgCode = codestr;
                }
                else
                {
                    OGRSpatialReference sr;
                    if (OGRERR_NONE == sr.importFromWkt(srs_temp.definition.c_str()))
                    {
                        std::string codestr(sr.GetAuthorityCode(NULL));
                        srsFile.espgCode = codestr;
                    }
                }
            }

            srsFile.Serialize(out);
            out.close();
                      
            return AI3D_SUCCESS;
        }

        int BlockObject::Task_Info::LoadLocalBin(srs_s& srs, const std::string& file)
        {
            std::ifstream in = File::OpenIfstreamUtf8(file, std::ios::binary);
            if (!in.is_open())
            {
                LOGE("Load SRS file failed!");
                return false;
            }

            SRSFile srsFile;
            srsFile.Deserialize(in);

            
            
            
            
            
            coord_system_type_e type = (coord_system_type_e)srsFile.type;
            srs.type = type;
     
            if (type == coord_system_type_e::LOCAL_ENU)
            {
                std::array<double, 3> ori = srsFile.ori;
                double lat = ori[0];
                double lon = ori[1];
                double alt = ori[2];
                srs.definition = "ENU:" + std::to_string(lat) + "," + std::to_string(lon);
            }
            else if (type == GEOGRAPHIC || type == PROJECTION || type == GEOCENTRIC)
            {
                std::string espCode = srsFile.espgCode;

                int code = std::stoi(espCode);
                OGRSpatialReference sr;
                if (OGRERR_NONE == sr.importFromEPSG(code))
                {
                    srs.definition = "EPSG:" + std::to_string(code);
                }

                
                
                
                
                
                
                
                
                
                
            }

            in.close();
            return AI3D_SUCCESS;
        }
        
        bool BlockObject::ParseATReport(ATReport& at_report)
        {
            LoadTiepoints();
            at_report.at_optins = GetTaskInfoMutual().at_options;
            GetCurrentATMutual()->GenerateATReport(at_report);
            std::vector<CameraCalibration> CameraUnCalibrationVec, CameraCalibrationVec;
            GetATData()->GenerateATReportForCam( CameraUnCalibrationVec);
            at_report.CameraUnCalibratedParam = CameraUnCalibrationVec;
            GetCurrentATMutual()->GenerateATReportForCam(CameraCalibrationVec);
            at_report.CameraCalibratedParam = CameraCalibrationVec;
            return true;
            
        }
    }
}




