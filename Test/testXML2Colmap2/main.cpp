#include <string>
#include "reconstruction.h"
#include "misc.h"
#include "ply.h"
#include <iostream>
#include <iomanip>
using namespace colmap;
int main(int argc, char* argv[])
{
    std::string path1 = "E:/TestData/pingdu_1/newgs/Tile_+004_+019/Tile_+004_+019-1";
    auto points = ReadPly(path1+".ply");
   // auto points = ReadPly("E:/TestData/testcolmap2gs/biao/biao.ply");
    double xmax = -DBL_MAX;
    double xmin = DBL_MAX;
    double ymax = -DBL_MAX;
    double ymin = DBL_MAX;
    double zmax = -DBL_MAX;
    double zmin = DBL_MAX;
    size_t count = points.size();

    double depth = 10000.0;
    auto remove_pos = std::remove_if(points.begin(), points.end(), [&](PlyPoint point)
        {
            if (std::fabs(point.x) > depth || std::fabs(point.y) > depth || std::fabs(point.z) > depth)
                return true;
            return false;
        });
    points.erase(remove_pos, points.end());

    //for (std::vector<PlyPoint>::iterator it = points.begin();
    //    it != points.end();)
    //
    //{
    //    /*if (std::fabs(point.x) > 10)
    //        std::cout << count << " x "<< point.x << std::endl;
    //    if (std::fabs(point.y) > 10)
    //        std::cout << count << " y " << point.y << std::endl;
    //    if (std::fabs(point.z) > 10 )
    //        std::cout << count << " z " << point.z << std::endl;*/
    //    auto point = *it;
    //    if (std::fabs(point.x) > 10 || std::fabs(point.y) > 10 || std::fabs(point.z) > 10)
    //    {

    //        it = points.erase(it);
    //    }
    //    else
    //    {
    //        it++;
    //    }

    //    /*if (point.x < xmin)
    //    {
    //        xmin = point.x;
    //    }
    //    if (point.y < ymin)
    //        ymin = point.y;
    //    if (point.z < zmin)
    //        zmin = point.z;
    //    if (point.x >xmax)
    //        xmax = point.x;
    //    if (point.y > ymax)
    //        ymax = point.y;
    //    if (point.z > zmax)
    //        zmax = point.z;*/

    //    //count++;

    //}
    //std::cout << count <<"count " /*<< points.size() - count*/ << std::endl;
   /* std::cout << std::setprecision(16) << xmin << " " << ymin << " " << zmin << " " <<
        xmax << " " << ymax << " " << zmax << std::endl;*/
        WriteTextPlyPoints(path1+"_txt.ply",points);
    Reconstruction rec;
    std::string path = argv[1], image_path=argv[2], dense_path=argv[3];
   // if (ExistsFile(argv[1]))
    {
        rec.LoadFromXMLFileNew(path);
    //    rec.Write(dense_path);
   }
  //  else
    {
        //E:\TestData\testcolmap2gs\26\at\colmap\distort/
    //E:\TestData\testcolmap2gs\26\images-raw/   E:\TestData\testcolmap2gs\26\at\colmap\cmdundisotort/
    //    rec.Read(path);
        //rec.Normalize();
        rec.Write(dense_path);
        rec.RunUndistort(rec, image_path, dense_path);
    }
    return 0;
}