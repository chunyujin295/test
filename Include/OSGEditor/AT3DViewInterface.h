/**
  * @file      AT3DViewInterface.h
  * @brief     空三视图的逻辑
  * @details
  * @author    
  * @attention
  */
#ifndef _AI3D_3DVIEW_AT3DVIEWINTERFACE_H_
#define _AI3D_3DVIEW_AT3DVIEWINTERFACE_H_

#include <vector>

#include <Eigen/Core>

#include "Core/Camera.h"
#include "Core/ATData.h"
#include "Core/BlockObject.h"
#include "Core/Types.h"
#include "OSGEditor/OsgEngine.h"
#include "Core/ReconstructionObject.h"
using namespace AI3D::CORE;
namespace AI3D
{
    namespace VIEWER
    {

        enum selection_layer_e
        {
            LARYER_PHOTOS,
            LAYER_TIEPOINTS,
        };

        enum image_layer_e
        {
            IMAGE_LARER_PHOTOS,
            IMAGE_LAYER_TIEPOINTS,
            IMAGE_LAYER_GCP,
            IMAGE_LAYER_TILE,
            IMAGE_LAYER_ROI,
            IMAGE_LAYER_POLYGON,
        };

        enum selection_mode_e
        {
            SEL_SINGLE_MODE,
            SEL_RECT_MODE,
            SEL_POLYGON_MODE,
        };
        
        enum AT_element_e
        {
            AT_ELE_PHOTOS,
            AT_ELE_TIEPOINTS,
            AT_ELE_GCP,
            AT_ELE_TILE,
            AT_ELE_ROI,
            AT_ELE_POLYGON,
        };
        


        
       



        class AI3D_API AT3DViewInterface
        {
        public:
            AT3DViewInterface(const ATData& data, OsgEngine* osgEngine,const jobsta_e& blockstatus) ;
           ~AT3DViewInterface() { engine_ = nullptr; }
            //显示
            void Init();
            //选择模式的切换
            void ResetSelectionMode(const selection_mode_e& mode);
            //选择图层的切换
            void ResetSelectLayer(const selection_layer_e& layer);
            void ResetImageLayer(const std::set<image_layer_e>& layerSet);
            //元素的显隐
            void ResetElementHideOrShow(const AT_element_e& ele,bool bvis);
            //开启单选：1：左键可选，2：可多个单选
            void StartSingleSelect();
            //开始框选：在框选模式下，鼠标

            //框选结束
            void EndWithRectSelectiton();
            //多边形选择结束
            void EndWithPolygonSelectiton();
            void Update();
           

            void SetSelectedImages(const std::vector<int>& ids);
            void DeleteImages();
            void DeleteTiepoints();
            void BuildATScene();
        private:
            void BuildImagesNode();
            void BuildTiepointsNode();
            void BuildGCPsNode();
        private:
            std::vector<int> images_selected_;
           
            ATData data_;
            jobsta_e blockstatus_;
            OsgEngine* engine_;
        };
        struct tile_viewer_setting_s
        {
            //bb

            //tile集合


        };
        enum reconst_element_e
        {
            RD_ELE_PHOTOS,
            RD_ELE_TIEPOINTS,
            RD_ELE_GCP,
            RD_ELE_ROI,
            RD_ELE_CONSTRAINT,
            RD_ELE_TILE,
        };

        class AI3D_API Tile3DViewInterface
        {
        public:
            Tile3DViewInterface(ReconstructionObject* data, OsgEngine* osgEngine);
            ~Tile3DViewInterface();
            //显示
            void Init(bool bSelectTiles = false);
            void InitWithOutATScene(bool bSelectTiles);
            //元素的显隐
            static void ResetElementHideOrShow(const reconst_element_e& ele, bool bvis, OsgEngine* osgEngine);
            //编辑模式
            void StartROIEdit();
            static void ResetImageLayer(const std::set<reconst_element_e>& layerSet, OsgEngine* osgEngine);
            

            //保存兴趣区编辑
            void EndAndSaveWithROIEdit();
            //
            void EndAndCancelWithROIEdit();
            void Update();

            void BuildScene();
        
            void BuildTilesNode();
            void BuildROINode();
            void BuildConstraintNode();
        private:
            
            tile_viewer_setting_s old_settings_;
            tile_viewer_setting_s now_settings_;
            /** Non-owning; lifetime must exceed this interface (block-owned ReconstructionObject). */
            ReconstructionObject* data_ = nullptr;
            OsgEngine* engine_ = nullptr;
        };

    }//CORE
}  //AI3D

#endif  // _AI3D_CORE_ALGORITHMBASE_H_
