#include <string>
#include "Core/Types.h"
//#include <Core/BlockObject.h>
#include "Core/KML.h"
//#include <Core/Tiling.h>
//#include "Core/ReconstructionCommandSet.h"
//#include "Core/ReconstructionObject.h"

int main(int argc, char* argv[])
{

   /* std::vector<std::vector<Eigen::Vector3d> > points = ReadKML(file);;
    std::vector<std::vector<Eigen::Vector2d> > boundary_custom_(points.size());
    std::string def = "ENU:23.09953,113.34051";
    CoordinateTransformer::TransformPoints(points, GEO84SRS, def);*/
   
    std::vector<std::vector<Eigen::Vector2d> > boundary_custom_(1);
    boundary_custom_[0].push_back(Eigen::Vector2d(0.,0.));
    boundary_custom_[0].push_back(Eigen::Vector2d(0., 1.));
    boundary_custom_[0].push_back(Eigen::Vector2d(1., 1.));
    boundary_custom_[0].push_back(Eigen::Vector2d(1., 0.));
    /*for (int i = 0; i < points.size(); i++)
    {
        std::vector<Eigen::Vector2d> boundary(points[i].size());
        for (int j = 0; j < points[i].size(); j++)
        {

            
            boundary[j] = (Eigen::Vector2d{ points[i][j].x(),points[i][j].y() });
        }
        boundary_custom_[i] = boundary;
    }*/
    ABBox2d bb_;
    bb_.min() = Eigen::Vector2d(0.2, 0.2);
    bb_.max() = Eigen::Vector2d(0.5, 0.5);
    std::vector<OGRPoint> cornors;
    OGRPoint cornor1 = OGRPoint(bb_.min().x(), bb_.min().y());
    cornors.push_back(cornor1);
    OGRPoint cornor2 = OGRPoint(bb_.min().x(), bb_.max().y());
    OGRPoint cornor3 = OGRPoint(bb_.max().x(), bb_.min().y());
    OGRPoint cornor4 = OGRPoint(bb_.max().x(), bb_.max().y());
    cornors.push_back(cornor2); cornors.push_back(cornor3); cornors.push_back(cornor4);
    OGRLinearRing* tilebox_geo_metry = nullptr;


    tilebox_geo_metry = static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing));
    for (int i = 0; i < 4; i++)
    {


        tilebox_geo_metry->addPoint(cornors[i].getX(), cornors[i].getY());

        tilebox_geo_metry->closeRings();

    }

    std::vector<OGRLinearRing*> boundaries_geo_metry(boundary_custom_.size());
  
        for (int i = 0; i < boundary_custom_.size(); i++)
        {

            OGRGeometry* geo = ToPolygon(boundary_custom_[i]);
            std::vector<OGRGeometry*> geoms;
            geoms.push_back(geo);
            OGRLinearRing* boundary_geometry = nullptr;
            if (boundary_custom_[i].size() > 2)
            {
                boundary_geometry = static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing));
                for (int j = 0; j < boundary_custom_[i].size(); j++)
                {
                    boundary_geometry->addPoint(boundary_custom_[i][j][0], boundary_custom_[i][j][1]);


                }
                boundary_geometry->closeRings();
            }
            boundaries_geo_metry[i] = boundary_geometry;
        }

       
        bool a = boundaries_geo_metry[0]->Intersect(tilebox_geo_metry);

        std::cout << a << std::endl;

        std::vector<Eigen::Vector2d> poins2d;
     
        
        //{
        //    std::vector<OGRPoint> cornors;
        //    OGRPoint cornor1 = OGRPoint(bb_.min().x(), bb_.min().y());
        //    cornors.push_back(cornor1);
        //    OGRPoint cornor2 = OGRPoint(bb_.min().x(), bb_.max().y());
        //    OGRPoint cornor3 = OGRPoint(bb_.max().x(), bb_.min().y());
        //    OGRPoint cornor4 = OGRPoint(bb_.max().x(), bb_.max().y());
        //    cornors.push_back(cornor2); cornors.push_back(cornor3); cornors.push_back(cornor4);
        //    for (int j = 0; j < boundaries_geo_metry.size(); j++)
        //    {
        //        auto geobound = boundaries_geo_metry[j];
        //        std::cout << "ring " << std::endl;
        //        for (int bj = 0; bj < geobound->getNumPoints(); bj++)
        //        {
        //            OGRPoint pt;
        //            geobound->getPoint(bj, &pt);
        //            std::cout << std::setprecision(16) << bj << " " << pt.getX() << " " << pt.getY()
        //                << " " << std::endl;
        //        }

        //        for (int ci = 0; ci < 4; ci++)
        //        {
        //            std::cout << std::setprecision(16) << "cor " << cornors[ci].getX() << " " << cornors[ci].getY()
        //                << " " << std::endl;
        //            if (geobound->isPointInRing(&cornors[ci]))
        //            {
        //                std::cout << "name "  << std::endl;
        //            }
        //        }


        //        /*if (std::any_of(cornors.begin(), cornors.end(), [&](const OGRPoint& pt)
        //            {  return (geobound->isPointInRing(&pt));
        //                  }))
        //        {
        //            tiles_tobeused[iter.first] = iter.second;
        //        }*/
        //    }
        //}


    
   /* ABBox3d a,aa;
    a.min().x() = 1.;
    a.min().y() = 1.;
    a.min().z() = 1.;
    a.max().x() = 0.;
    a.max().y() = 2.;
    a.max().z() = 2.;
   MakeBoundingBoxValid(a);
   
    auto dig = a.diagonal();
   
    std::cout << a.min().x()<< " ===" << dig.x()<< " "<<dig.y() << " "<<dig.z() << std::endl;
    
    if (dig.norm() > (0.))
    {
        std::cout << a.min().x() << " =++==" << aa.diagonal() << std::endl;
    }
    auto b = a.corner(Eigen::AlignedBox<double, 3>::CornerType::BottomLeftFloor);*/
    //测试blk
    
   // AI3D::CORE::BlockObject block("C:/data/Projects/NewProject88/Block_7/");
   // AI3D::CORE::BlockObject::BlockImportOptions opt;
   // opt.load_tiepoint_ = true;

   // std::string atxm = "C:/data/Projects/NewProject88/Block_7/Block_7.blk";
   // 
   // block.Load(atxm,true, opt);
   // auto ptnum = block.GetCurrentAT()->GetNumPoints3D();
   // auto imgnum = block.GetCurrentAT()->GetNumPoints3D();
   // ReconstructionCommandSet::SubmitReconstruction(&block);
   // //ReconstructionCommandSet::SubmitReconstruction(&block);
   // ReconstructionCommandSet::ResetTilingMode(&block,1, tiling_mode_e::TILE_ADAPTIVE);
   //
   // //对第一个reconstruction提交生产
   // ReconstructionObject* rec1 = block.GetReconstructionsMutual().at(1);
   // production_option_s opts;
   // auto bbbase = rec1->GetBoundingBoxBase();
   // 
   // std::cout << bbbase.min() << " raw " << bbbase.max() << std::endl;
   // ABBox3f tile1bb = bbbase.cast<float>();
   // tile1bb.max() = tile1bb.max() * 0.5;
   // //std::cout << tile1bb.min() << " " << tile1bb.max() << std::endl;
   // ABBox3f tile2bb = bbbase.cast<float>();
   // tile2bb.min().x() = tile1bb.max().x();
   // tile2bb.min().y() = tile1bb.max().y();
   // //std::cout << tile2bb.min() << " " << tile2bb.max() << std::endl;
   // auto& tiles = rec1->GetTilesMutual();
   ///* std::cout << rec1->GetTilesMutual()["Tile_1"].name_ <<" "<<rec1->GetTilesMutual().begin()->first << " "<< rec1->GetTilesMutual()["Tile_1"].bb_.min() << " "<<
   //     tile1bb.min() << std::endl;*/
   // tiles["Tile_1"].bb_ =  (tile1bb);
   // tiles["Tile_2"].bb_ = (tile2bb);

   // std::cout << tiles["Tile_1"].bb_.min() << "tile1=== " << tiles["Tile_1"].bb_.max() << std::endl;
   // std::cout << tiles["Tile_2"].bb_.min() << "tile2=== " << tiles["Tile_2"].bb_.max() << std::endl;
   // /*std::cout << rec1->GetTilesMutual()["Tile_1"].bb_.min() << " +== " <<
   //     tile1bb.min() << std::endl;*/
   // /*rec1->GetTilesMutual().clear();
   // rec1->SetTiles(tiles);*/
   // //rec1->GetTilesMutual()["Tile_1"].bb_.max() = (tile1bb.max());
   ///* std::cout << rec1->GetTilesMutual()["Tile_1"].bb_.min() << " == " <<
   //     tile1bb.min() << std::endl;*/
   // //rec1->GetTilesMutual()["Tile_2"].bb_ = tile1bb;
   // opts.tiles_.push_back("Tile_1");
   // opts.tiles_.push_back("Tile_2");
   // 
   // block.GetTaskInfoMutual().reconstructions_info_[0].tiles_ = tiles;

   // std::string blockitem = block.GetIdString() + "/" + rec1->GetIDString() + "/" + "Productions/";
   // ReconstructionCommandSet::SubmitProduction("chy","9999","C:/data/Projects/NewProject88/", blockitem, &block, 1,opts);
   // std::cout << "tile1=== " << rec1->GetTilesMutual()["Tile_1"].bb_.min() << rec1->GetTilesMutual()["Tile_1"].bb_.max() << std::endl;
   // std::cout << "tile2++++=== " << rec1->GetTilesMutual()["Tile_2"].bb_.min() << rec1->GetTilesMutual()["Tile_2"].bb_.max() << std::endl;
   // block.Save();
  // /* auto& taskinfo = block.GetTaskInfoMutual();
  //  BlockObject::blk_recontruction_info_s rinfo;
  //  rinfo.id_ = 1;
  //  rinfo.name_ = "first";
  //  srs_s basesrs;
  //
  //  basesrs = CoordinateDescriptor::GetSRSFromDefinition("epsg:4326");*/

  // 
  // 


  //  //测试分块
  //  if (0)
  //  {
  //      tiling_param_s paras(tiling_mode_e::TILE_PALNAR_GRID);
  //      Tiling* pTilingGenarator = TilingGenaratorFactory(paras);
  //      auto atdata = std::make_shared<ATData>();
  //      const std::vector<Eigen::Vector3d> points;
  //      pTilingGenarator->Run(*atdata.get(), points);
  //      int a = pTilingGenarator->GetNumTiles();
  //      std::cout << a << std::endl;
  //  }
  //  if(0)
  //  {
  //      Exiv2::Image::AutoPtr image_;
  //      image_ = Exiv2::ImageFactory::open("D:/jiaojie/test/nurf/images/building/add/DJI_202305271351_004/DJI_20230527135303_0001.JPG");
  //      image_->readMetadata();
  //      Exiv2::ExifData ed = image_->exifData();
  //      Exiv2::XmpData xmpData = image_->xmpData();
  //      
  //      std::string  lonstr = xmpData["Xmp.drone-dji.GpsLongitude"].toString();
  //      float lon = xmpData["Xmp.drone-dji.GpsLongitude"].toFloat();
  //     /* Exiv2::ExifThumbC thumb(ed);
  //      Action::Print print;
  //      print.printMetadata(image_.get());*/
  //  }
    //if (1)
    //{

    //    AI3D::CORE::BlockObject block;
    //    tiling_param_s paras(tiling_mode_e::TILE_ADAPTIVE);

    //    Tiling* pTilingGenarator = TilingGenaratorFactory(paras);

    //    std::string atxm = "D:/TestData/S3DResult/5w.xml";
    //    auto atdata = std::make_shared<ATData>();
    //    block.LoadATXML(atxm, atdata);
    //    tiling_mode_e mode = tiling_mode_e::TILE_ADAPTIVE;
    // /*   ReconstructionObject* recobj = new ReconstructionObject(*atdata.get(),1);
    //    tiling_mode_e mode = tiling_mode_e::TILE_ADAPTIVE;
    //    ReconstructionCommandSet::ResetTilingMode(&block,1, mode);*/

    //    /*const std::vector<Eigen::Vector2d> points;*/
    //    atdata->ComputeTileBoundingBox(atpoint_elements_e::PT_ELE_VIEWS_TIEPOINTS);
    //    auto bb = atdata->GetTileAABBBox();
    //    pTilingGenarator->Run(*atdata.get(), bb.cast<double>());

    //    //将tile bb输出到 txt文件

    //    std::ofstream file("D:/TestData/S3DResult/5wbb.txt", std::ios::trunc);
    //  
    //    file.precision(17);
    //    auto tileresult = pTilingGenarator->GetTilesInfo();
    //    for (auto& tile : tileresult) 
    //    {
    //        auto xmin = tile.second.bb_.min().x();
    //        auto ymin = tile.second.bb_.min().y();
    //        auto zmin = tile.second.bb_.min().z();
    //        auto xmax = tile.second.bb_.max().x();
    //        auto ymax = tile.second.bb_.max().y();
    //        auto zmax = tile.second.bb_.max().z();
    //        
    //        Eigen::Vector3d left_buttom_floor{xmin,ymin,zmin};
    //        Eigen::Vector3d right_buttom_floor{ xmin,ymax,zmin };
    //        Eigen::Vector3d left_top_floor{ xmin,ymax,zmin };
    //        Eigen::Vector3d right_top_floor{ xmax,ymax,zmin };

    //        Eigen::Vector3d left_buttom_ceil{ xmin,ymin,zmax };
    //        Eigen::Vector3d right_buttom_ceil{ xmin,ymax,zmax };
    //        Eigen::Vector3d left_top_ceil{ xmin,ymax,zmax };
    //        Eigen::Vector3d right_top_ceil{ xmax,ymax,zmax };

    //       
    //        file << left_buttom_floor.x() << " "<< left_buttom_floor.y() << " "<< left_buttom_floor.z()  << std::endl;
    //        file << right_buttom_floor.x() << " " << right_buttom_floor.y() << " " << right_buttom_floor.z() << std::endl;
    //        file << left_top_floor.x() << " " << left_top_floor.y() << " " << left_top_floor.z() << std::endl;
    //        file << right_top_floor.x() << " " << right_top_floor.y() << " " << right_top_floor.z() << std::endl;
    //        file << left_buttom_ceil.x() << " " << left_buttom_ceil.y() << " " << left_buttom_ceil.z() << std::endl;
    //        file << right_buttom_ceil.x() << " " << right_buttom_ceil.y() << " " << right_buttom_ceil.z() << std::endl;
    //        file << left_top_ceil.x() << " " << left_top_ceil.y() << " " << left_top_ceil.z() << std::endl;
    //        file << right_top_ceil.x() << " " << right_top_ceil.y() << " " << right_top_ceil.z() << std::endl;
    //       
    //    }
    //    file.close();
    //}
        return 0;
    }