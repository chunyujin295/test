#ifndef _AI3D_CORE_BLOCK_H_
#define _AI3D_CORE_BLOCK_H_
#include <Constants.h>
#include <glog/logging.h>
#include <pugixml.hpp>
#include "Core/ATData.h"

#include "Core/Alignment.h"
#include "Core/Types.h"
#include "Core/ATGroup.h"
#include "Core/PhotoGroup.h"
#include "Core/ControlPoint.h"
#include "Core/Application.h"
#include "Core/ReturnCode.h"
#include "Core/String.h"

#include <omp.h>
#include <Core/json.h>
#include "Core/Rapidjson.h"
#include "Core/ReconstructionObject.h"
#include "Core/BlockInfo.h"

namespace AI3D
{
    namespace CORE
    {
        typedef EIGEN_STL_UMAP(image_t, pose_s)  Poses;
        typedef int (*progFunc)(void *,int,int);

        
        struct AI3D_API IndexImage
        {
            image_t id_ = kInvalidImageId;
            std::string filename_ = "";
            IndexImage();
            IndexImage(image_t id, std::string filename);

            friend bool operator==(const IndexImage& m1, const IndexImage& m2);
            
            friend bool operator!=(const IndexImage& m1, const IndexImage& m2);

            friend bool operator<(const IndexImage& m1, const IndexImage& m2);
        };

        

        class AI3D_API BlockObject
        {
        public:
            typedef enum block_type_e
            {
                TYPE_NONE = 0,
                TYPE_CLONED = 1,
                TYPE_MERGED = 2 
            }block_type_e, bt_e;           

            
            struct StatisticInfo_s
            {
                int  imagenum = 0;
                int regisimagenum = 0;
                int gcpnum = 0;
                int tiepointnum = 0;
                int usertiepointnum = 0;
            };

            enum block_status_e
            {

                BLKSTS_PENDDING = 0,
                BLKSTS_RUNNING = 1,  
                BLKSTS_COMPLETE = 2,
                BLKSTS_CANCLE = 3,
                BLKSTS_FAILURE = 4,
                BLKSTS_NEW = 5,
                BLKSTS_UNKNOWN = 6,
                BLKSTS_EMPTY = 7,
                
            };
         
            struct AI3D_API Task_Info
            {
                
                bool hasstatisinfo = false;
                bool hasatsetting = false;
                std::string blockName = "";
                std::string blockString = "";
                std::string mergedFrom = "";
                int  blockId;
                bool gotatseting = false;
                
                int AT_Num = 0;
                
                std::string Block_XML = "";
                std::string Tiepoints = "";
                std::string AT_XML = "";

                bool btiepoints_changed_ = false;
                bool isLoaded = false;
                bool isSaved = true;
              
                SaveType_e savetype_ = savetype_e::XML_SAVED;

                
                bool isFinished = false;
                bool isCancleOrDelete = 0;
                
                bool btopredict_ = false;

                std::string job_ = "";
                std::string atjson_ = "";
                std::string gcpjson_ = "";
                std::string gdal_folder_ = "";
				std::string projectfile_ = "";
                
                std::string functionname_ = "";
                ATOptions at_options;

				std::unordered_map<std::string, std::string> stageTotalTime;
                std::string ATTotalTime = "";

                int keyMaxImgNum = -1;
                int matchMaxImgNum = -1;
                
                int debug_level_ = 1;
                double focal_length_ = 42.0;
               
                
                
                bool WriteBlockInfoToJson(const std::string& blk_fullpath,bool exportatsetting=false);
                bool ReadBlockInfoJson(const std::string& file_path);
                                                                                                      

                bool WriteBlockInfoToBin(const std::string& blk_fullpath, bool exportatsetting = false);
                bool ReadBlockInfoBin(const std::string& file_path);
                int WriteToJson(std::string path);
                StatisticInfo_s statisticinfo_;

                std::vector<blk_recontruction_info_s> reconstructions_info_;
                
               

                std::map<std::string, std::string> reconstructionjobs_;

                static int WriteLocalJson(const srs_s& srs, const std::string& file);
                static int LoadLocalJson(srs_s& srs, const std::string& file);

                static int WriteLocalBin(const srs_s& srs, const std::string& file);
                static int LoadLocalBin(srs_s& srs, const std::string& file);
                
            };


           

            struct AI3D_API BlockImportOptions
            {
                BlockImportOptions() {}
                BlockImportOptions(bool load_tiepoint,bool load_images) : load_tiepoint_(load_tiepoint), load_images_(load_images)
                {

                }
                bool load_tiepoint_ = false;
                bool load_images_ = true;
                
                bool force_reload_tiepoints_from_disk_ = false;
                
                bool suppress_update_complete_at_file_on_reload_ = false;
            };
            
            struct AI3D_API BlockExportOptions
            {
                BlockExportOptions() {}
                BlockExportOptions(srs_s srs) : srs_(srs)
                {
                   
                }
                bool export_tiepoint_ = false;
                bool export_controlpoint_ = true;
                bool export_not_registered_ = true;
                
                srs_s srs_;
                rot_format_e rotformat_ = rot_format_e::ROTFORMAT_R;
                
            };

            struct DataPreprocessOption
            {
                std::string photoDir;
                double centreFocalLength;
                double obliqueFocalLength;
                std::string namePrefix;
                std::vector<std::string> photoFolderNames;
                int nameLength;
                int nameStartNo;
                std::string exportDirectory;
                std::string exportFileName;

                std::string SRS;
                std::vector<std::string> posFileFormat;
                std::vector<std::string> posDataFields;
                std::string posFile;
            };

		public:
            BlockObject();
            BlockObject(std::string path);

            BlockObject(const BlockObject& block);
            BlockObject& operator=(const BlockObject& block);
            virtual ~BlockObject();

            
            int UpdateCompleteATFile(); 
            void MakeBlockFromATData(ATData AATModel);
            int GenerateATReport();
            void Init();
            
            void SetName(const std::string& name);
            const std::string GetName() const;
            void SetPath(const std::string& path);
            std::string& GetPathMutual();
            const std::string GetPath()const;
            std::string& GetNameMutual();
            void ReName(std::string name);

            void SetDescription(std::string des);
            const std::string GetDescription() const;
            std::string GetDescriptionMutual();

            void SetType(bt_e type);
            const bt_e GetType() const;
            bt_e GetTypeMutual();

            void SetStatus(jobsta_e type);
            const jobsta_e GetStatus() const;
            jobsta_e GetStatusMutual();

            void SetId(block_t id);
            const block_t GetId() const;
            block_t& GetIdMutual();
            bool Check();

			bool Addimages_Beta(const std::vector<std::string>& images, int* cbProgress, bool* bCancle = nullptr);
            bool Addimages2_Beta(const std::vector<std::string>& images, int* cbProgress, bool* bCancle = nullptr);

            int BatchPrePare(std::string configfile,int* processValue);

            bool BatchPreProcess(const std::string& img_dir_path, int* processValue, const std::string& pref, const srs_s& blocksrs, const std::string& gcp_path = "", std::string posfilepath = "", uint8_t numLen = 4, int begin = 1);
            bool BatchPreProcess(DataPreprocessOption &option,std::string& generated_xml,const std::string& img_dir_path, int* processValue, const std::string& pref, const srs_s& blocksrs, const std::string& gcp_path = "", 
                std::string posfilepath = "", uint8_t numLen = 4, int begin = 1,progFunc funcPtr = nullptr,void *thatObj = nullptr,int taskId = -1);

            bool BatchPreProcessPosId(DataPreprocessOption& option, std::string& generated_xml, const std::string& img_dir_path, int* processValue,
                const std::string& pref, const srs_s& blocksrs, const std::string& gcp_path,
                std::string posfilepath, uint8_t numLen, int begin, progFunc funcPtr, void* thatObj, int taskId);
            

           
            
            void SearchImages(const std::string& path, std::vector<std::string>& filenames,
                std::vector<std::string> image_extension, bool bIncludeSubDir = true);
           
            
            void AddReconstruction(ReconstructionObject* object );
           
            bool ExistsReconstruction(reconstruction_t reconst_id);
            ReconstructionObject* GetReconstruction(reconstruction_t reconst_id);
            
            
            void CloneReconstruction(const reconstruction_t reconstruction_id, reconstruction_t& new_reconstruction_id);
            std::vector<std::string> GetReconstructionNames();
            bool DeleteReconstruction(reconstruction_t id);

            bool IsEmpty();

            bool AddPoses(srs_s srs, std::vector<pose_s>  poses);
            
            bool ClearPoses(const std::vector<group_t>  ids);
            bool ClearPoses(const std::set<image_t>  ids);
            
            void ChangeStatus(job_status_e status = job_status_e::STATUS_NEW);

            bool AddGCPs();
            
            
            bool PhotoGroupHasElement(group_t group_id) ;
            bool RemovePhotoGroup(std::vector<group_t> ids);
           
            bool RemoveImages(std::set<image_t> imageids);
            
            bool UnGroupImages();
            
           
            bool SetGroup(std::map<std::string, std::vector<std::shared_ptr<Image> > >& photogroup_map);
            const PhotoGroup& GetGroup(group_t id);
            EIGEN_STL_UMAP(group_t, PhotoGroup) GetPhotoGroups() const;
            EIGEN_STL_UMAP(group_t, PhotoGroup)& GetPhotoGroupsMutual() ;
            bool Clear();
     
          
         
            image_t GenerateValidImageId();

            group_t GenerateValidPhotoGroupId();

            
            reconstruction_t GenerateValidReconstructionId();

            
            
            
            void SetATData(const std::shared_ptr<ATData>& at_data);
            
            const std::shared_ptr<ATData> GetATData() const;
            std::shared_ptr<ATData> GetATDataMutual() ;
           

            bool UpdateATGroup(std::shared_ptr<ATData>& ATData, bool bgcpat = false);


            
            image_t GetNumImages();
            group_t GetNumPhotoGroup();
            
            std::set<image_t>GetImagesids();

            point3D_t GetNumControlPoints();
            point3D_t GetNumCheckPoints();
            point3D_t GetNumValidControlPoints();
            
            point3D_t GetNumUserTiePoints();
            point3D_t GetNumAutoTiePoints();
          
	        
            void MakeNewBlockForCommonImages(const BlockObject& refblock, BlockObject& newblock1,  BlockObject& newblock2,bool byname =true);
            
            void MakeNewBlockForCommonAreaImages( BlockObject& block, BlockObject& newblock);

           
             srs_s GetBlockSRS() ;
           
            
            void SetBlockSRS(std::string srsdefinition);          

            EIGEN_STL_UMAP(srsid_t, srs_s) GetSRSs() const;
			EIGEN_STL_UMAP(srsid_t, srs_s)& GetSRSsMutual();
            void SetSRSs(const EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map);
            srsid_t ExistSRS(const std::string& definition);

            std::shared_ptr<ATData>& GetOriginAT();
            const ATGroup& GetAT(group_t atid) const;
            
            void SetAT0(std::shared_ptr<ATData> ATdata);

            const ATGroup& GetAT0() const;
            ATGroup& GetAT0();
       
            
			static bool ParseSRS(EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map, const pugi::xml_node& srss);
            bool ParsePhotoGroups(std::shared_ptr<ATData> Atdata_, std::vector<PhotoGroup>& pg_,
                std::set<image_t>& images_pg, const pugi::xml_node& groups,
                EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map, std::set<srsid_t>& srs_used_ids);
            bool ParseCamera(Camera & camera,const pugi::xml_node& group, const int& temp_id)const;
            bool ParsePhotos(EIGEN_STL_UMAP(image_t, Image)& group_image,
                const pugi::xml_node& group, const int& camera_id,
                EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map, std::set<srsid_t>& srs_used_ids);

			bool ParseControlPoints(std::shared_ptr<ATData> Atdata_, EIGEN_STL_UMAP(point3D_t, ControlPoint)& cps, 
                const std::set<image_t>& images_pg, const pugi::xml_node& controlpoints, EIGEN_STL_UMAP(srsid_t, srs_s) srsmap, 
                std::set<srsid_t>& srs_used_ids);

            static bool ParseControlPoints(const pugi::xml_node& controlpoints, EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map,

                EIGEN_STL_UMAP(point3D_t, ControlPoint)& cps_map, EIGEN_STL_UMAP(image_t, std::string)& image_map);
            bool ParseTiePoints(std::shared_ptr<ATData> Atdata_, EIGEN_STL_UMAP(point3D_t, Point3D) &tps, const std::set<image_t> &images_pg, const pugi::xml_node& tiepoints, EIGEN_STL_UMAP(point3D_t, Point3D)& usertps);
           
            
            void SerializePhotoGroup(const ATData& Atdata_,const PhotoGroup& pg, pugi::xml_node node_pg, BlockExportOptions block_export_options);
            void SerializeControlPoint(const ATData& Atdata,const ControlPoint& cp, pugi::xml_node node_cp,bool forOnlyMeasureMode = false);
            
            void SerializeTiePoint(const Point3D& tp, pugi::xml_node node_tps, bool from_user_points_map = false);
            
            static void SerializePositioningConstraints(ATData& atdata, pugi::xml_node block_root);
            static void SerializeControlPoint(EIGEN_STL_UMAP(image_t, std::string)& image_map, const ControlPoint& cp,
                pugi::xml_node node_cp);

            bool LoadXLS(const std::string& xls_file_path, std::shared_ptr<ATData>ATdata);

            bool LoadXLSX(const std::string& xlsx_file_path, std::shared_ptr<ATData>ATdata);
            bool ExportXLSX(const std::string& xlsx_file_path,const std::string&srsdef,bool InRadians = false);

            bool HasSurveyPoints() const ;
            bool HasReconstruction(reconstruction_t id) const;
            bool HasReconstructions() const;

            bool LoadATXML(const std::string& xml_file_path,std::shared_ptr<ATData>ATdata,bool transformTobase = true, bool bParseTiept=true);
            int ExportATXML(const std::string& xml_file_path, BlockExportOptions block_export_options = BlockExportOptions());
            
            
            
            bool ReloadCurrentATFromPersistedFilesForExportOrReconstruction();
            
            
            void EnsureCurrentATReflectsConstraintScale();
            static int ExportGCPMeasurementsXML1(const std::string& xml_file_path,  EIGEN_STL_UMAP(image_t, std::string)& image_map,
                EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) cps_map);
            int ExportATDataToXML(const std::string& xml_file_path, BlockExportOptions block_export_options, ATData data);
            static int LoadGCPMeasurementsXML1(const std::string& xml_file_path, EIGEN_STL_UMAP(srsid_t, srs_s)& srs_map,
                EIGEN_STL_UMAP(point3D_t, ControlPoint)& cps_map, EIGEN_STL_UMAP(image_t, std::string)& image_map);
            int LoadGCPMeasurementsXML(const std::string& xml_file_path, std::shared_ptr<ATData>& ATdata);
            int ExportGCPMeasurementsXML(const std::string& xml_file_path);


            bool LoadPoseTxt(const std::string& postxt_path,std::vector<pose_s>& poses);
            
            bool LoadPoseXLSX(const std::string& posxlsx_path, std::vector<pose_s>& poses);

            bool ImagesRename(const std::string& img_dir_path,std::map<std::string,std::string>& image_old_to_new_name, int* processValue, const std::string& pref,uint8_t numLen = 4, int begin = 1);
          
           
            
           
            
            bool LoadBlockATData( std::shared_ptr<ATData>& ATdata, BlockImportOptions block_import_options = BlockImportOptions());
            bool LoadATBinaryWithoutTiepoints(const std::string& file_path, std::shared_ptr<ATData>& ATdata);    
            bool LoadTiepointsBinary(const std::string& file_path, const std::shared_ptr<ATData>& ATdata);
            bool UndistortBlock(const std::string& path, UndistortCameraOptions_s opt);
            
			bool ExportBlockATData();
           
			bool ExportATBinaryWithoutTiepoints(const std::string& file_path);
			bool ExportTiepointsBinary(const std::string& file_path);

            
            bool LoadATBinary(const std::string& AT_filepath, std::shared_ptr<ATData>ATdata);
            bool ExportATBinary(const std::string& AT_filepath);

          
            bool LoadExternalFile(const std::string& name);

         
            bool CanSubmitRecon();

            bool Load(const std::string& file_path,bool parsebin = true, BlockImportOptions block_import_options = BlockImportOptions());

			bool Save();
           
            bool UpdateCameraInfo(cam_para_e type, double value, group_t group_id);
            

            bool UpdateSensorSize( Camera& camera);

            std::shared_ptr<ATData> GetCurrentATMutual();
            const std::shared_ptr<ATData> GetCurrentAT() const;

            
            EIGEN_STL_UMAP(reconstruction_t, ReconstructionObject*) GetReconstructionsMutual()
            {
                return reconstructions_;
            }
            EIGEN_STL_UMAP(reconstruction_t, ReconstructionObject*) const GetReconstructions() const
            {
                return reconstructions_;
            }

         
            bool HasUserTiePoints() const;
            bool HasControlPoints() const;

 
            bool DeleteImages(const std::vector<image_t> ids,const group_t group_id);

            srs_s ComputeEnuSRS();

            
            srsid_t GenerateValidSrsId();
            void UpdateSRSMap(const srs_s &srs);

            
            void setModifily(bool modifyflag) { ismodify_ = modifyflag; };
            bool GetModified() { return ismodify_; };

            void SetTaskInfo(const Task_Info&taskinfo);
            Task_Info GetTaskInfo()const;
            Task_Info& GetTaskInfoMutual();
            
            void setImportFilename(std::string& importFilename) { this->importFilename_ = importFilename;  }
            std::string getImportFilename() { return importFilename_;  }

            bool GetTiepointStatus();
            void SetTiepointStatus(bool tiept_loaded);
            void LoadTiepoints();
            bool SaveCamBin(const std::string& filename);
            bool GetTiepointFullStatus();

            EIGEN_STL_UMAP(group_t, ATGroup)& GetATGroupMutual();
            bool SaveImagePosListBin(const ATOptions& options);
            bool SaveImagePosListJson(const ATOptions& options);

            void ClearImageIds();
            
            void UpdateGCPATReport(AcquisitionReport& gcp_accuracy, AcquisitionReport& chkgcp_accuracy);
			bool ParseATReport(ATReport& at_report);
           
            bool ExportATReport(const ATReport& at_report, const std::string& ATReportFilename);
            bool GenerateATReportPicture(const ATReport& at_report, const std::string& picPath);
            
            bool InterSectionAdjustment();

            std::string GetIdString();
            Task_Info block_info_;
            static bool supportTempLogs();
            static bool supportOptimization4ProductionListOverview();
            static bool isValidBlockObject(BlockObject* blockObject);
            static bool isChineseVersion();
            static void setChineseVersion();
            static std::set<BlockObject*> m_setBlockObject;
            static std::string getJobStringStatus(job_status_e& job_status);

        private:
                bool btiepoint_loaded_ = false; 
        private:

           

            bool ExistSRSId(const srsid_t& id);

            
            
            bool ApplyConstraintScaleAfterAtMergeIfAvailable(std::shared_ptr<ATData>& atdata);
            
            bool SyncPoseSidecarBinsAfterConstraintSimilarity();

            group_t temp_posgroup_id_ = kInvalidGroupId;
            
            
            EIGEN_STL_UMAP(group_t, ATGroup) ATGroups_;
            EIGEN_STL_UMAP(group_t, PhotoGroup) photogroups_;

            EIGEN_STL_UMAP(reconstruction_t, ReconstructionObject*) reconstructions_;

           
            
            std::shared_ptr<ATData> ATData_;
            block_t id_ ;
            std::string name_ = "";
            std::string path_ = "";
            Eigen::Vector3d position_offset_ = {-DBL_MAX,-DBL_MAX, -DBL_MAX};
           
            srs_s srs_enu_discription_ ;
            
            EIGEN_STL_UMAP(srsid_t, srs_s) srs_map_;
         
            srsid_t blockSRS_id_;
          
         
            std::set<image_t> image_ids_;
        
            std::string description_ ="";
            bt_e type_;
            jobsta_e status_;

            std::string importFilename_ = "";
           int blkversion_ = -1000;
           bool ismodify_ = false;
           static bool bChineseVersion;
        };


       

        };

   
}
#endif