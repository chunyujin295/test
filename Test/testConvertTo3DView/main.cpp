#include <string>

#include <Core/BlockObject.h>

#include <Core/Tiling.h>
#include "Core/ReconstructionCommandSet.h"
#include "Core/ReconstructionObject.h"
#include "Core/ReconstructionOptions.h"
#include "Core/CoordinateSystem.h"
#include "Core/ControlPoint.h"
#include "Core/ReconstructionOptions.h"


using namespace AI3D::CORE;
#define DEL_PATH_END(pszPath) \
{ \
	if (pszPath[lstrlen(pszPath)-1] == '\\') \
	pszPath[lstrlen(pszPath)-1] = '\0'; \

int main(int argc, char* argv[])
{

   /* constraint_info_s::polygon_info_s polygon;
    polygon.ParsePoints("C:/data/Projects/NewProjectty/Block_5/Reconstruction_1/Constraint/constaint_0_0.txt");
    std::string file1 = "D:/jiaojie/test/bugchi/guangzhou/woter-in.kml";

*/


    bool  outputpt = true;
    bool outputimage = true;
    bool outbutgcp = true;
       AI3D::CORE::BlockObject block;
       std::string file = "D:/jiaojie/test/testreconstruction/NewProject/Block_6/";
    
      


       size_t start = file.length()-1 ;
       size_t end = file.rfind("/");
       if (end == start)
       {

       }
       while (file.rfind("/") != std::string::npos)
       {
           file.erase(start,1);
           start--;
       }
       std::cout << start << std::endl;
       
       std::string atxm;// = argv[1];
        auto atdata = std::make_shared<ATData>();
        block.LoadATBinary("D:/jiaojie/test/testreconstruction/NewProject/Block_6/SCSFR.bin",atdata);
        block.LoadATXML(atxm, atdata);
        std::string basedir = File::GetParentDir(atxm);
        std::string name = File::GetFileNameWithoutExtension(atxm);
        std::string basefile = basedir + "/" + name;
        std::string ptfile = basefile + "pt.txt";
        std::string imgfile = basefile + "camera.txt";
        std::string gcpfile = basefile + "gcps.txt";
        if (outputimage)
        {
            //输出点云
            atdata->WriteImageText(imgfile);

            //输出相机
        }


        if (outputpt)
        {
            //输出点云
            atdata->WritePoints3DText(ptfile);

            //输出相机
        }

        if (outbutgcp)
        {
            //控制点D:\TestData\Images\gcptest\kzd\controlpoints.txt
            auto controlpoints = atdata->GetControlPoints();
            ControlPoints gcps;
            gcps.GetPointsMutual() = controlpoints;
            gcps.TransformPoints(atdata->GetLocalSrs());
            gcps.SaveTextFor3DView(gcpfile);

        }
        if(0)
        {
        ControlPoints gcps;
        gcps.LoadText(argv[1]);

        std::string def1 = "epsg:4547";
        std::string def2 = "ENU:37.734830,112.593190";
        srs_s srs = CoordinateDescriptor::GetSRSFromDefinition(def1);
        for (auto& iter : gcps.GetPointsMutual())
        {
            iter.second.SetSrs(srs);
        }
        gcps.TransformPoints(def2);
        gcps.SaveTextFor3DView(gcpfile);
    }

    //输出影像


    return 0;

}