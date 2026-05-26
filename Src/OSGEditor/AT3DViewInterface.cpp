


#include <vector>

#include <Eigen/Core>
#include <future>
#include "OSGEditor/AT3DViewInterface.h"

namespace AI3D
{
    namespace VIEWER
    {


        void AT3DViewInterface::Update()
        {
            //根据可见性来决定是否显示
        }
        void AT3DViewInterface::BuildImagesNode()
        {
           // ATData localdata = data_;
            auto blockstatus = blockstatus_;
            auto &images = data_.GetImages();
            auto cameras = data_.GetCameras();
            std::vector<ST_CAMERA_INFO> stCameras;
            for (auto& iter : images)
            {
                auto& image = iter.second;

                ST_CAMERA_INFO stCamera;
                stCamera.ID = image.GetImageId();



                auto  camid = image.GetCameraId();
                if (!cameras.count(camid))
                { 
                    continue;
                }
                if (image.HasPosition())
                {
                    auto pos = image.GetPosition();
                    stCamera.Center.set(pos(0), pos(1), pos(2));
                    
                }
                if (image.HasRotationMatrix())
                {
                    auto R = image.GetRotationMatrix();
                    stCamera.mt.set(
                        R(0, 0), R(0, 1), R(0, 2), 0,
                        R(1, 0), R(1, 1), R(1, 2), 0,
                        R(2, 0), R(2, 1), R(2, 2), 0,
                        0, 0, 0, 1);
                }
                auto cam = cameras.at(camid);
                stCamera.Image_Height = cam.GetHeight();
                stCamera.Image_Width = cam.GetWidth();
                
                double fpix = cam.GetFocalLength();
                if (fpix <= 0)
                {
                    fpix = 42.0 / 35.0 * std::max(cam.GetHeight(), cam.GetWidth());
                }
               
                stCamera.FocalPixel = fpix;
                auto depth = iter.second.GetDepth()(1);
               
                if (depth != -DBL_MAX)
                {
                    stCamera.Depth = depth;
                }
                stCamera.ImagePath = AI3D::CORE::File::EnsureUnifySlash(image.GetPath() + "/" + image.GetName());
                stCamera.mSize = engine_->image_size_;
                if (blockstatus == STATUS_COMPLETE)
                {
                    if (image.IsRegistered())
                    {
                        stCamera.aerType = Aerotriangulation_Type::AER_BACK_SUCCESS;
                    }
                    else
                    {
                        stCamera.aerType = Aerotriangulation_Type::AER_BACK_FAIL;
                    }
                }
                else
                {
                    stCamera.aerType = Aerotriangulation_Type::AER_FRONT;
                }
                stCameras.push_back(stCamera);
               
            }
            engine_->AddPhotos(stCameras);
        }



        void AT3DViewInterface::BuildTiepointsNode()
        {
            const float colortemplate = 50.0 / 255.0;
            osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
          
            std::vector<std::pair<int, std::vector<osg::Vec3> >> tmpID;
            ST_TIEPOINT stTiePoint;
         //   ATData localdata = data_;
            auto *images = &data_.GetImages();
            auto *points3D = &data_.GetPoints3D();
            {
                //添加点云
                osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
                osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

                ST_TIEPOINT stTiePoint;
                stTiePoint.ID = 1;
                uint64_t count = 0;
#if 1
                //modify by zhaobf
                //使用多线程后，性能提升300%
                std::mutex tmpMutex;
                {
                    int threadNum = 50, realThreadNum = 0;
                    std::vector<thread> threadVecs(threadNum); //最大线程
                    int pointsTotals = points3D->size();
                    int start = 0, end = 0;
                    stTiePoint.points = new osg::Vec3Array(pointsTotals);
                    stTiePoint.colors = new osg::Vec4Array(pointsTotals);
                    stTiePoint.IDRelevancyPhoto.resize(pointsTotals);

                    for (int num = 0; num < threadNum; num++)
                    {
                        end = (start + 100000) > pointsTotals ? pointsTotals: (start + 100000);    //每个线程处理10W个点的数据                     
                        realThreadNum++;
                        threadVecs[num] = thread([start, end, points3D, images ,&tmpMutex, &stTiePoint, colortemplate]() {
                            auto t1 = std::next(points3D->begin(), start);
                            auto t2 = std::next(points3D->begin(), end);
                            osg::ref_ptr<osg::Vec3Array> v1 = new osg::Vec3Array;
                            osg::ref_ptr < osg::Vec4Array> c = new osg::Vec4Array;                              

                            std::vector<std::pair<int, std::vector<osg::Vec3> >> IDRelevancyPhoto;     
                            std::map<int, std::vector<int> > PhotoRelevancyID;
                            std::map<int, int> PointIDRelevancyIndex;
                            int tmpIndex = start;

                            for (t1; t1 != t2; ++t1)
                            {
                                int pointID = t1->first;
                                
                                std::vector<osg::Vec3>  image_pos;
                                PointIDRelevancyIndex.insert(make_pair(pointID, tmpIndex));
                                for (auto& ele : t1->second.GetTrack().GetElements())
                                {
                                    auto imageid = ele.image_id;
                                    if (!images->count(imageid))
                                    {
                                        continue;
                                    }
                                    auto image_position = (images->at(imageid).GetPosition());
                                    osg::Vec3 position(image_position.x(), image_position.y(), image_position.z());
                                    image_pos.push_back(position);

                                    PhotoRelevancyID[imageid].push_back(pointID);
                                }

                                IDRelevancyPhoto.push_back({ pointID ,image_pos });

                                float x = t1->second.GetXYZ().x(); float y = t1->second.GetXYZ().y(); float z = t1->second.GetXYZ().z();
                                v1->push_back(osg::Vec3(x, y, z));

                                float r = t1->second.GetColor().x() / 255.f;
                                r = r < 0.00001 ? colortemplate : r;
                                float g = t1->second.GetColor().y() / 255.f;
                                g = g < 0.00001 ? colortemplate : g;
                                float b = t1->second.GetColor().z() / 255.f;
                                b = b < 0.00001 ? colortemplate : b;
                                c->push_back(osg::Vec4(r, g, b, 1.0));

                                tmpIndex++;
                                   
                            }//end for t1,t2;

                          

                            tmpMutex.lock();
                            for (auto it : PhotoRelevancyID)
                            {
                                stTiePoint.PhotoRelevancyID[it.first].insert(stTiePoint.PhotoRelevancyID[it.first].end(),it.second.begin(), it.second.end());                                 
                            }

                            stTiePoint.PointIDRelevancyIndex.insert(PointIDRelevancyIndex.begin(), PointIDRelevancyIndex.end());                                 

                            for (int i = 0 ; i < v1->size(); i++)
                            {
                                stTiePoint.points->at(start + i)= v1->at(i);
                                stTiePoint.colors->at(start + i) = c->at(i);
                                stTiePoint.IDRelevancyPhoto[start + i] = IDRelevancyPhoto[i];
                            }

                            tmpMutex.unlock();

                            PointIDRelevancyIndex.clear();
                            IDRelevancyPhoto.clear();
                            PhotoRelevancyID.clear();
                        });


                        start = end;
                        if (end == pointsTotals)
                        {
                            break;
                        }

                    }//end for 10;

                    for (int i=0; i< realThreadNum; i++)
                    {
                        threadVecs[i].join();
                    }                                               

                } //
#endif
#if 0

                ST_TIEPOINT stTiePoint2;
                stTiePoint2.ID = 1;
                for (auto& iter : *points3D)
                {
                    auto pointID = iter.second.GetId();                      
                    std::vector<osg::Vec3>  image_pos;
                    for (auto& ele : iter.second.GetTrack().GetElements())
                    {
                        auto imageid = ele.image_id;
                        if (!images->count(imageid))
                        {
                            //此处应该抛出异常
                            continue;
                        }
                        auto image_position = (images->at(imageid).GetPosition());
                        osg::Vec3 position(image_position.x(), image_position.y(), image_position.z());
                        image_pos.push_back(position);
                        stTiePoint2.PhotoRelevancyID[imageid].push_back(pointID);
                    }

                    
                    stTiePoint2.IDRelevancyPhoto.push_back({ pointID ,image_pos });


                    std::vector<float> res(6);
                    res[0] = iter.second.GetXYZ().x();
                    res[1] = iter.second.GetXYZ().y();
                    res[2] = iter.second.GetXYZ().z();
                    float r = iter.second.GetColor().x() / 255.f;

                    res[3] = r<0.00001 ? colortemplate : r;
                    float g = iter.second.GetColor().y() / 255.f;
                    res[4] = g < 0.00001 ? colortemplate : g;
                    float b = iter.second.GetColor().z() / 255.f;
                    res[5] = b < 0.00001 ? colortemplate : b;

                    vertices->push_back(osg::Vec3(res[0], res[1], res[2]));
                    colors->push_back(osg::Vec4(res[3], res[4], res[5], 1.f));
                    //类型转换后需要改@
                    stTiePoint2.PointIDRelevancyIndex.insert({ int(pointID),(int)count });
                    
                    
                    count++;
                }

              
                stTiePoint2.points = vertices.get();
                stTiePoint2.colors = colors.get();
              //  stTiePoint.size = 2;
#endif

                osg::ref_ptr<osg::Node> pNode = engine_->AddTiePoint(stTiePoint);

            }
        }
        void AT3DViewInterface::BuildGCPsNode()
        {
          //  ATData localdata = data_;
            auto gcps = data_.GetControlPoints();
            clock_t t11, t21, t31;

            t11 = clock();
            //添加控制点
            {
                for (auto& iter : gcps)
                {
                    std::string tmpstr = iter.second.GetName();
                    auto pos = iter.second.GetObjectPoint().GetXYZ();
                    osg::Vec3 center(pos.x(), pos.y(), pos.z());
                    auto id = iter.second.GetId();
                    if (iter.second.GetObjectPoint().GetTrack().Length() > 0)
                    {
                        
                        engine_->AddControlPoint(id, center, tmpstr, 2);
                     //   std::cout << "id======== " << id << " type========== " << 2 << std::endl;
                    }
                    else
                    {
                        engine_->AddControlPoint(id, center, tmpstr, 1);
                        //std::cout << "id======== " << id << " type========== " << 1 << std::endl;
                    }
                }
            }
            t21 = clock();
            t31 = t21 - t11;
            std::cout << " BuildGCPsNode Function " << t31 * 0.001 << std::endl;

        }
        AT3DViewInterface::AT3DViewInterface(const ATData& data, OsgEngine* osgEngine, const jobsta_e& blockstatus)
        {
            ATData* at_data = new ATData(data);
           if(1)
            {
               
                if ((at_data->HasPositionImages() ||
                    at_data->HasControlPoints() ||
                    at_data->HasTiepoints()))
                {




                    Eigen::Vector3d offset;
                    srs_s srs;
                    if (at_data->HasPositionImages())
                    {
                        at_data->RenderPoses(offset, srs);
                    }
                    else
                    {
                        if (at_data->HasControlPoints())//计算控制点的平均值
                        {
                            at_data->GetPoints3DMutual().clear();
                            if (AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(at_data->GetLocalGcpSrs()).type == GEOGRAPHIC)
                            {
                                std::string definition = BASESRS;
                                at_data->TransformControlPoints(definition);
                            }
                            if (AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(at_data->GetLocalGcpSrs()).type != LOCAL_ENU)
                            {



                                Eigen::Vector3d sum = Eigen::Vector3d::Zero();
                                Eigen::Vector3d point_first = at_data->GetControlPoints().cbegin()->second.GetGivenXYZ();

                                for (auto iter : at_data->GetControlPoints())
                                {
                                    sum += (iter.second.GetObjectPoint().GetXYZ() - point_first);
                                }
                                Eigen::Vector3d position_offset = sum / at_data->GetControlPoints().size() + point_first;

                                if (AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(at_data->GetLocalGcpSrs()).type != LOCAL)
                                {
                                    //换算到经纬度
                                    AI3D::CORE::CoordinateTransformer::Transform(1, &position_offset[0],
                                        &position_offset[1], &position_offset[2],
                                        at_data->GetLocalGcpSrs(), "EPSG:4326");
                                    position_offset[2] = 0.0;

                                    char buf[1024];
                                    sprintf(buf, "%.5f,%.5f", position_offset[1], position_offset[0]);
                                    std::string strlb(buf);
                                    std::string local_srs_definition = "ENU:" + strlb;
                                    at_data->SetLocalGcpSrs(local_srs_definition);
                                    at_data->TransformControlPoints(local_srs_definition);
                                }
                                else
                                {
                                    for (auto& iter : at_data->GetControlPointsMutual())
                                    {
                                        iter.second.GetObjectPointMutual().GetXYZMutual() -= position_offset;
                                    }
                                }
                            }

                        }

                    }

                   double scale =  at_data->GetSceneScale();
                   osgEngine->image_size_ = float(1.0/scale);
                }
            }



            data_ = *at_data;
            delete at_data;
            engine_ = osgEngine;
            blockstatus_ = blockstatus;
        }

        void AT3DViewInterface::DeleteImages()
        {

        }
        void AT3DViewInterface::DeleteTiepoints()
        {

        }
       

        void AT3DViewInterface::BuildATScene()
        {
            LOGI("Init AT image node.");
            clock_t t11, t21, t31;
            t11 = clock();
            BuildImagesNode();
            t21 = clock();
            t31 = t21 - t11;
            std::cout << t31 << " BuildImagesNode " << t31 * 0.001 << std::endl;
            t11 = clock();
            LOGI("Init Tiepoints node.");
            BuildTiepointsNode();
            t21 = clock();
            t31 = t21 - t11;
            std::cout << t31 << " tiepointsNode " << t31 * 0.001 << std::endl;
            LOGI("Init GCP  node.");
            t11 = clock();
            BuildGCPsNode();
            t21 = clock();
            t31 = t21 - t11;
            std::cout << t31 << " GCPNode " << t31 * 0.001 << std::endl;
        }
        //显示；除了要把基本元素显示出来，还要把被选择的对象也显示出来
        void AT3DViewInterface::Init()
        {
            BuildATScene();
            ATData localdata = data_;
            bool imagechanged, tiepointchanged, gcpchanged;
            localdata.GetBoundingBox(imagechanged, tiepointchanged, gcpchanged);
            engine_->BuildAxis(localdata.GetBox());
            engine_->ClearSelectElement();

            if (!images_selected_.empty())
            {
                engine_->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS, images_selected_);
            }
            else
            {
                ///engine_->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_NONE, std::vector<int>());
            }
            engine_->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS, true);
            engine_->SetSelectType(Select_Type::SELECT_ONE);
            engine_->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS);
        }
        //选择模式的切换:
    //模式切换时：1：如果当前有被选中元素的则仍然是选中状态
    //2：如果是框选，则鼠标变或者不变也可，
    //3:记录一下当前的选择模式
        void AT3DViewInterface::ResetSelectionMode(const selection_mode_e& mode)
        {
            //不刷新，但是要记录一下模式
          
            if (mode == selection_mode_e::SEL_POLYGON_MODE)
                engine_->SetSelectType(Select_Type::SELECT_POLYGON);
            else if (mode == selection_mode_e::SEL_RECT_MODE)
                engine_->SetSelectType(Select_Type::SELECT_BOX);
            else if (mode == selection_mode_e::SEL_SINGLE_MODE)
                engine_->SetSelectType(Select_Type::SELECT_ONE);
            else
                engine_->SetSelectType(Select_Type::SELECT_ONE);
        }

        //选择图层的切换
    //1：之前被选择的对象会清空，但是选择模式不变；
    //2：选择模式需要记录 ?是否有单独清空的接口
        void AT3DViewInterface::ResetSelectLayer(const selection_layer_e& layer)
        {
            ELEMENT_LAYER_TYPE oldlayer = engine_->GetCurrentElementType();
            ELEMENT_LAYER_TYPE sellayer;
            if (layer == LARYER_PHOTOS)
            {
                sellayer = ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS;
            }
            else if (layer == ELEMENT_TIEPOINTS)
            {
                sellayer = ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS;
            }

           
            if(oldlayer != sellayer)
            {
                images_selected_.clear();
                engine_->DeselectPickedNodeWithoutDeleting();
                if (layer == LARYER_PHOTOS)
                {
                   
                    engine_->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS,std::vector<int>());
                    engine_->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS);
                }
                else if (layer == LAYER_TIEPOINTS)
                {
                    engine_->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS, std::vector<int>());
                    engine_->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS);
                }
              
            }
           
        }

        void AT3DViewInterface::ResetImageLayer(const std::set<image_layer_e> &layerSet)
        {
           
                if (layerSet.count(IMAGE_LARER_PHOTOS))
                {
                    ResetElementHideOrShow(AT_element_e::AT_ELE_PHOTOS, true);
                }
                else
                {
                    ResetElementHideOrShow(AT_element_e::AT_ELE_PHOTOS, false);
                }
                if (layerSet.count(IMAGE_LAYER_TIEPOINTS))
                {
                    ResetElementHideOrShow(AT_element_e::AT_ELE_TIEPOINTS, true);
                }
                else
                {
                    ResetElementHideOrShow(AT_element_e::AT_ELE_TIEPOINTS, false);
                }
                if (layerSet.count(IMAGE_LAYER_GCP))
                {
                    ResetElementHideOrShow(AT_element_e::AT_ELE_GCP, true);
                }
                else
                {
                    ResetElementHideOrShow(AT_element_e::AT_ELE_GCP, false);
                }
        }
        
        //元素的显隐
        void AT3DViewInterface::ResetElementHideOrShow(const AT_element_e& ele, bool bvis)
        {
            if(ele == AT_element_e::AT_ELE_GCP)
                engine_->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS,bvis);
            else  if (ele == AT_element_e::AT_ELE_PHOTOS)
            {
                engine_->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS, bvis);
                //？是否需要加此句
                engine_->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS, std::vector<int>());
            }
            else if (ele == AT_element_e::AT_ELE_TIEPOINTS)
            {
                engine_->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS, bvis);
                //连线要消失，？接口待测
            }
            else if (ele == AT_element_e::AT_ELE_TILE)
            {
                engine_->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_TILE, bvis);
            }
            else if (ele == AT_element_e::AT_ELE_ROI)
            {
                engine_->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_ROI, bvis);
            }
            else if (ele == AT_element_e::AT_ELE_POLYGON)
            {
                engine_->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_POLYGON, bvis);
            }
        }


       

        void AT3DViewInterface::SetSelectedImages(const std::vector<int>& ids)
        {
            images_selected_ = ids;
        }

            //开启单选：1：左键可选，2：可多个单选
        //开始的标识是鼠标已经按下了一次，
        //1：显示被选中的目标和被选中目标的附属物体的显示如图片对点云；
        //2: 如果是影像则，需要记录被选中的对象
        //3：同时多个单选模式可以开启；
        //4：可以有删除操作
        void AT3DViewInterface::StartSingleSelect()
        {
            //？binfeng:接口在哪
        }
           

            //框选结束
        //框选可以在场景中任意一个地方选择，
        //1：选完后根据当前的图层获取相应的对象（影像和点云）
        //2：如果有影像则获取影像的id 已经影像对应的点云；以不同的样式显示；可做删除操作；删除完后需要更新显示
        // 3：如果是点：显示点对应的照片，可做删除操作，删完后更新
        //
        void AT3DViewInterface::EndWithRectSelectiton()
        {
            //GetSelectionMode();
        }
            //多边形选择结束，同框选
        void AT3DViewInterface::EndWithPolygonSelectiton()
        {
        }


        

      
        void Tile3DViewInterface::Update()
        {

        }

        Tile3DViewInterface::Tile3DViewInterface(ReconstructionObject* data, OsgEngine* osgEngine)
        {
            static int index = 2000;
            std::cout << "inside " << __FILE__ << " " << __LINE__ << " at " << (++index) << std::endl;
            LOGI("Init 01.");
            data_ = new AI3D::CORE::ReconstructionObject(*data);
            LOGI("Init 02.");
            engine_ = osgEngine;
            LOGI("Init 03.");
        }
        Tile3DViewInterface::~Tile3DViewInterface()      //modify by zhaobf
        {
             if (data_)
             {
                 delete data_;                
             }
             data_ = nullptr;
        }

        void Tile3DViewInterface::BuildScene()
        {
            engine_->RemovePickedNode();  //modify zhaobf

            clock_t t11, t21, t31;
            t11 = clock();
            BuildTilesNode();
            t21 = clock();
            t31 = t21 - t11;
            std::cout << t31 << " BuildTilesNode " << t31 * 0.001 << std::endl;
            t11 = clock();
            BuildROINode();
            t21 = clock();
            t31 = t21 - t11;
            std::cout << t31 << " BuildROINode " << t31 * 0.001 << std::endl;
            t11 = clock();
            BuildConstraintNode();
            t21 = clock();
            t31 = t21 - t11;
            std::cout << t31 << " BuildConstraintNode " << t31 * 0.001 << std::endl;
        }
        void Tile3DViewInterface::BuildTilesNode()
        {
            clock_t t11, t21, t31;  t11 = clock();
           // ReconstructionObject localdata = *data_;
            t21 = clock();
            t31 = t21 - t11;
            std::cout << t31 << " BuildTilesNodeFunction " << t31 * 0.001 << std::endl;
            auto tiles = data_->GetTilesCustom();
            if (data_->HasTiles())
            {
               
                
                int tilecnt = 0;

                auto tileset = data_->GetTilesName(data_->GetProcessingSettings().bdiscard_emptytiles_);
                std::vector<ST_BOUNDINGBOX> box(tileset.size());
                for (auto& iter : tileset)
                {
                    if (!tiles.count(iter))
                        continue;
                    auto tile = tiles.at(iter);
                    ST_BOUNDINGBOX bbtemp;
                    bbtemp.ID = tile.index_;
                 
                    bbtemp.name = tile.name_;
                    bbtemp.type = 2;

                    auto bbtile = tile.bb_;
                    bbtemp.minXYZ.x() = bbtile.min().x();
                    bbtemp.minXYZ.y() = bbtile.min().y();
                    bbtemp.minXYZ.z() = bbtile.min().z();
                    bbtemp.maxXYZ.x() = bbtile.max().x();
                    bbtemp.maxXYZ.y() = bbtile.max().y();
                    bbtemp.maxXYZ.z() = bbtile.max().z();
                    box[tilecnt] = bbtemp;
                    tilecnt++;
                }
                engine_->RenderTiles(box);
            }

           
        }
        void Tile3DViewInterface::BuildROINode()
        {
            //ReconstructionObject localdata = *data_;

            auto scenebb = data_->GetBoundingBoxCustomMutual();

            if (data_->HasBoundary())
            {
                auto boundaries = data_->GetBoundaryCustomMutual();
                std::vector<ST_POLYGON_BOX> polys(boundaries.size());
                for (int idx1 = 0; idx1 < boundaries.size(); idx1++)
                {
                    std::vector<Vec3> polypoints(boundaries[idx1].size());

                    polys[idx1].maxHeight = scenebb.max().z();
                    polys[idx1].minHeight = scenebb.min().z();
                    polys[idx1].ID = idx1;
                    for (int idx2 = 0; idx2 < boundaries[idx1].size(); idx2++)
                    {
                        Vec3 point;
                        point.x() = boundaries[idx1][idx2].x();
                        point.y() = boundaries[idx1][idx2].y();
                        point.z() = 0.;
                        polypoints[idx2] = point;
                       
                    }
                    polys[idx1].points = polypoints;

                }
                engine_->AddROIBox(polys);
            }
            else
            {
                std::vector<ST_BOUNDINGBOX> box(1);
                box[0].ID = 0;
                box[0].type = 1;
                box[0].maxXYZ.x() = scenebb.max().x();
                box[0].maxXYZ.y() = scenebb.max().y();
                box[0].maxXYZ.z() = scenebb.max().z();

                box[0].minXYZ.x() = scenebb.min().x();
                box[0].minXYZ.y() = scenebb.min().y();
                box[0].minXYZ.z() = scenebb.min().z();
                engine_->AddROIBox(box);
            }
        }
        void Tile3DViewInterface::BuildConstraintNode()
        {
            //水域文件
            //ReconstructionObject localdata = *data_;
            auto constraints = data_->GetConstraintCustom();// localdata.GetConstraintCustom();
            int index = 0;
            /*for (auto& iter : constraints)
            {
                iter.SavePolygonsDebug("D:/jiaojie/test/yueshu/rangekml/transform/");
            }*/
            for (auto& iter_c : constraints)
            {
                
                std::string name = iter_c.name_;
                
                for (auto& iter_p : iter_c.polygons_)
                {
                  
                    osg::ref_ptr<osg::Vec3Array> vecPoints = new osg::Vec3Array;
                    for (auto& iter_o : iter_p.points_)
                    {
                        double x = iter_o.x();
                        double y = iter_o.y();
                        double z = iter_o.z();
                        vecPoints->push_back(osg::Vec3( x,y,z ));
                       
                       
                    }
                   
                   
                    engine_->AddPolygon(index, name, vecPoints);
                    index++;
                  
                }
               
             
            }
            
           
        }
        void Tile3DViewInterface::InitWithOutATScene(bool bSelectTiles)
        {
            LOGI("Init 1.");
           // ReconstructionObject localdata = *data_;
            //把空三显示出来；
            LOGI("Init 2.");
            if (engine_->IsATEmpty())
            {
                AT3DViewInterface atinterface(data_->GetATData(), engine_, jobsta_e::STATUS_COMPLETE);;
                LOGI("Init AT.");
                atinterface.BuildATScene();
                LOGI("Init AT end.");
                std::cout << " empty3 " << engine_->IsATEmpty() << std::endl;
            }
            LOGI("Init Scene begin.");
            BuildScene();
            LOGI("Init Scene end.");



            engine_->BuildAxis(data_->ComputeGlobalBoxCustom());
            LOGI("Init 6.");
            if (bSelectTiles)
                engine_->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_TILE);
            else
                engine_->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_NONE);
            LOGI("Init 7.");
        }
            //显示把所有的要素显示出来
        void Tile3DViewInterface::Init(bool bSelectTiles)
        {
            LOGI("Init 1.");
          //  ReconstructionObject localdata = *data_;
            //把空三显示出来；
            LOGI("Init 2.");
            AT3DViewInterface atinterface(data_->GetATData(), engine_, jobsta_e::STATUS_COMPLETE);;
            LOGI("Init AT.");
            atinterface.BuildATScene();
            LOGI("Init AT end.");
            BuildScene();
            LOGI("Init Scene.");
           
           
          
            engine_->BuildAxis(data_->ComputeGlobalBoxCustom());
            LOGI("Init 6.");
            if(bSelectTiles)
                engine_->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_TILE);
            else
                engine_->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_NONE);
            LOGI("Init 7.");
        }
        void Tile3DViewInterface::ResetImageLayer(const std::set<reconst_element_e>& layerSet, OsgEngine* engine)
        {

            if (layerSet.count(RD_ELE_PHOTOS))
            {
                ResetElementHideOrShow(reconst_element_e::RD_ELE_PHOTOS, true, engine);
            }
            else
            {
                ResetElementHideOrShow(reconst_element_e::RD_ELE_PHOTOS, false, engine);
            }
            if (layerSet.count(RD_ELE_TIEPOINTS))
            {
                ResetElementHideOrShow(reconst_element_e::RD_ELE_TIEPOINTS, true, engine);
            }
            else
            {
                ResetElementHideOrShow(reconst_element_e::RD_ELE_TIEPOINTS, false, engine);
            }
            if (layerSet.count(RD_ELE_GCP))
            {
                ResetElementHideOrShow(reconst_element_e::RD_ELE_GCP, true, engine);
            }
            else
            {
                ResetElementHideOrShow(reconst_element_e::RD_ELE_GCP, false, engine);
            }
            ResetElementHideOrShow(reconst_element_e::RD_ELE_CONSTRAINT, layerSet.count(RD_ELE_CONSTRAINT), engine);
            ResetElementHideOrShow(reconst_element_e::RD_ELE_TILE, layerSet.count(RD_ELE_TILE), engine);
            ResetElementHideOrShow(reconst_element_e::RD_ELE_ROI, layerSet.count(RD_ELE_ROI), engine);
        }
            
            //元素的显隐
        void Tile3DViewInterface::ResetElementHideOrShow(const reconst_element_e& ele, bool bvis, OsgEngine* engine)
        {
            if (ele == reconst_element_e::RD_ELE_GCP)
                engine->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS, bvis);
            else  if (ele == reconst_element_e::RD_ELE_PHOTOS)
            {
                engine->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS, bvis);
              
            }
            else if (ele == reconst_element_e::RD_ELE_TIEPOINTS)
            {
                engine->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS, bvis);
                //连线要消失，？接口待测
            }
            else if (ele == reconst_element_e::RD_ELE_TILE)
            {
                engine->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_TILE, bvis);
            }
            else if (ele == reconst_element_e::RD_ELE_ROI)
            {
                engine->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_ROI, bvis);
            }
            else if (ele == reconst_element_e::RD_ELE_CONSTRAINT)
            {
                engine->SetElementVisible(ELEMENT_LAYER_TYPE::ELEMENT_POLYGON, bvis);
            }
         }
            //编辑模式:
        //1：设置当前图层为兴趣区；
        //2：其他区域不可编辑；
        //3：如果切换页卡则提示；
        //4：记录编辑前的状态
        void Tile3DViewInterface::StartROIEdit()
        {

        }
           

            //保存兴趣区编辑
        //1：样式恢复；
        //2：获取到最新的boundingbox然后计算结果并刷新显示；
        //3：工程为未保存状态；
        void Tile3DViewInterface::EndAndSaveWithROIEdit()
        {

        }
            //恢复到编辑前的状态
        void Tile3DViewInterface::EndAndCancelWithROIEdit()
        {

        }


      
    }//CORE
}  //AI3D


