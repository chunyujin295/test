#include "Util/Transfer.h"
#include <QSysInfo>
#include <QProcess>

#include <Windows.h>
#include <string>
#include <map>
#include "Core/File.h"
#include "Core/Logging.h"
#include "Core/TaskDef.h"
#include <Core/ATData.h>
#include <Core/Application.h>
#include <Core/BlockObject.h>
#include <Core/Tiling.h>
#include "Core/ReconstructionCommandSet.h"
#include "Core/ReconstructionObject.h"
#include "Core/ReconstructionOptions.h"
#include "Core/CoordinateSystem.h"
#include "Core/ProjectObject.h"
#include "Core/ATCommandSet.h"
#include "colmap.h"
#include <QFile>
#include <QDir>
using namespace AI3D::CORE;

int ToColmapForGS(std::string indir, std::string out, std::string atout)
{
    QFile file_xml(QString::fromUtf8(indir.c_str(), static_cast<int>(indir.size())));
    QDir dir_img(QString::fromUtf8(out.c_str(), static_cast<int>(out.size())));
    QDir dir_out(QString::fromUtf8(atout.c_str(), static_cast<int>(atout.size())));
    if (!file_xml.exists() || !dir_img.exists() || !dir_out.exists()) {
        
        return -1;
    }
    try
    {
        AI3D::CORE::BlockObject block;
        std::string file = indir;

        std::string out1 = atout;
        out1 = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(out1)));
        out = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(out)));
        AI3D::CORE::Application::Getinstance().SetProjLibENV();
        AI3D::CORE::File::CreateDirIfNotExists(out, true);
        auto atdata = std::make_shared<AI3D::CORE::ATData>();

        auto ext = AI3D::CORE::File::GetFileExtension(file);
        AI3D::CORE::String::StringToLower(&ext);
        bool loadResult = false;
        if (ext == "bin")
        {
            loadResult = block.LoadATBinary(file, atdata);
        }
        else
        {
            loadResult = block.LoadATXML(file, atdata, false);
        }
        if (!loadResult) {
            return -2;
        }
        block.SetATData(atdata);
        
        std::string undistortpath = out + "/images/";
        AI3D::CORE::File::CreateDirIfNotExists(undistortpath);
        AI3D::CORE::UndistortCameraOptions_s undistopt;
        bool handleResult = false;
        handleResult = block.UndistortBlock(undistortpath, undistopt);
        if (!handleResult) {
            return -2;
        }

        
        std::string campath = out1 + "/sparse/0/";
        AI3D::CORE::File::CreateDirIfNotExists(campath);
        colmap::Reconstruction rec(*block.GetATData().get());

        rec.Write(campath);
        std::cout << " write end " << std::endl;
        
        AI3D::CORE::BlockObject::BlockExportOptions opt;
        opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
        opt.export_tiepoint_ = true;
        int returnResult = block.ExportATXML(campath + "new.xml", opt);
        if (returnResult != 1000) {
            return -2;
        }
        return 0;
    }
    catch (std::exception& ex)
    {
        std::cout << "inside " << ",exception occured:" << ex.what() << std::endl;
        return -2;
    }

    
}

template<typename Scalar, int Dim>
using Vector = Eigen::Matrix<Scalar, Dim, 1>;
typedef Vector<uint8_t, 3> Color3b;
int ReadVpc(std::string& infile, ATData& ATdata)
{


    FILE* pf = File::FopenUtf8(infile, "rb");
    if (!pf) {
        return 1001;
    }
    int num_pts;
    fread(&num_pts, sizeof(int), 1, pf);

    std::vector<Eigen::Vector3f> points(num_pts);
    
    
    point3D_t index_point3d = 0;
    auto images = ATdata.GetImagesMutual();
    auto cameras = ATdata.GetCamerasMutual();
    std::vector<std::vector<std::pair<image_t, float> > > point_views(num_pts);
    for (int i = 0; i < num_pts; ++i)
    {

        fread(points[i].data(), sizeof(float), 3, pf);
        
        Eigen::Vector3d xyz = points[i].cast<double>();
        int numview;
        fread(&numview, sizeof(int), 1, pf);
        point_views[i].resize(numview);
        std::vector<TrackElement> vec_trackele;
        for (int j = 0; j < numview; ++j)
        {
            image_t id;
            float w;
            fread(&id, sizeof(image_t), 1, pf);
            fread(&w, sizeof(float), 1, pf);
            if (!ATdata.GetImages().count(id))
            {
                continue;
            }
            point3D_t index_point3d;
            Eigen::Vector2d uv;
            auto& image = images[id];
            auto& camera = cameras[image.GetCameraId()];
            auto estimated_xy = AlgorithmBase::ProjectPointToImage(xyz,
                image.GetProjectionMatrix(),
                camera, false);
            Image& img = ATdata.GetImageMutual(id);
            TrackElement trackelement;
            trackelement.image_id = id;
            trackelement.point2D_idx = img.AddPoints2D(uv);
            img.SetPoint3DForPoint2D(trackelement.point2D_idx, index_point3d);
            trackelement.xy = uv;
            vec_trackele.push_back(trackelement);
        }


        Point3D point3d;


        
        Track track;




        track.AddElements(vec_trackele);
        point3d.SetId(index_point3d);
        point3d.SetTrack(track);

        point3d.SetXYZ(xyz);


        ATdata.GetPoints3DMutual().insert(std::make_pair(index_point3d, point3d));
        index_point3d++;
    }
    fclose(pf);
    std::string colorfile = File::GetParentDir(infile) + "/points_rgb.bin";
    if (File::ExistsFile(colorfile))
    {
        FILE* fp = File::FopenUtf8(colorfile, "rb");
        if (!fp) {
            return 1001;
        }

        int rgb_num_pts;
        fread(&rgb_num_pts, sizeof(int), 1, fp);
        if (num_pts != rgb_num_pts)
        {
            return 1001;
        }
        index_point3d = 0;
        std::vector<Color3b> rgbpoints(rgb_num_pts);
        for (int i = 0; i < rgb_num_pts; ++i)
        {

            fread(rgbpoints[i].data(), sizeof(uint8_t), 3, fp);
            ATdata.GetPoints3DMutual().at(index_point3d).SetColor(Eigen::Vector3i{ rgbpoints[index_point3d].x(),rgbpoints[index_point3d].y(),rgbpoints[index_point3d].z() });
            index_point3d++;
        }
        fclose(fp);
    }
    return 1000;
}

int RunParsePointcloudVpc(std::string indir, std::string recout) {
    std::string xmlfile = indir + "/views.xml";
    std::string vpcfile = indir + "/point_cloud.vpc";
    std::string campath = recout;
    QFile file_xml(QString::fromUtf8(xmlfile.c_str(), static_cast<int>(xmlfile.size())));
    QFile file_vpc(QString::fromUtf8(vpcfile.c_str(), static_cast<int>(vpcfile.size())));
    QDir folder(QString::fromUtf8(campath.c_str(), static_cast<int>(campath.size())));
    if (!file_xml.exists() || !file_vpc.exists() || !folder.exists()) {
        
        return -1;
    }

    try
    {
        AI3D::CORE::BlockObject  block;
        auto atdata = std::make_shared<ATData>();
        bool loadResult = block.LoadATXML(xmlfile, atdata, false, false);
        if (!loadResult) {
            return -2;
        }
        block.SetATData(atdata);
        int readResult = ReadVpc(vpcfile, *atdata.get());
        if (readResult != 1000) {
            return -2;
        }
        AI3D::CORE::File::CreateDirIfNotExists(campath);
        colmap::Reconstruction rec(*atdata.get());
        rec.Write(campath);
        return 0;
    }
    catch (std::exception& ex)
    {
        std::cout << ",exception occured:" << ex.what() << std::endl;
        return -2;
    }

    
}