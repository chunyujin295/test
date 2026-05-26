
#include <osgUtil/Optimizer>
//read image
#include <osg/DrawPixels>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osg/Node>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgUtil/Optimizer>




#include <osgUtil/Optimizer>

#include "Core/File.h"



using namespace AI3D::CORE;
int main(int argc, char* argv[])
{

    auto path = argv[1];
    auto dir_list = File::GetDirList(path);
    if (dir_list.empty())
    {
        std::cout << " empty dir " << path<< std::endl;
    }
    std::vector<std::string> osgpaths(dir_list.size());
    osg::ref_ptr<osg::Group> root = new osg::Group;
    for (int i = 0; i < dir_list.size(); ++i)
    {
        std::string  tilename = File::GetDirName(dir_list[i]);
        osgpaths[i] = dir_list[i] + "/" + tilename + ".osgb";
        if (!File::ExistsFile(osgpaths[i]))
        {
            continue;
        }
       
        osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(osgpaths[i]);
        if (node == nullptr)
        {
            std::cout << "failed in read " << osgpaths[i] << std::endl;
            continue;
        }
        auto bound = node->computeBound();
        osg::ref_ptr<osg::PagedLOD> paged_lod = new osg::PagedLOD;
        paged_lod->setCenterMode(osg::LOD::USER_DEFINED_CENTER);
        paged_lod->setCenter(bound.center());
        paged_lod->setRadius(bound.radius());
        paged_lod->setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
        paged_lod->setRange(0,0,std::numeric_limits<float>::max());
        paged_lod->setFileName(0, tilename+"/"+ tilename + ".osgb");
        root->addChild(paged_lod);   
    }
    std::string outfile = argv[2];
    osg::ref_ptr<osgDB::Options> option = new osgDB::Options();
    bool ret = osgDB::writeNodeFile(*root, outfile, option);
    if (ret)
    {
        std::cout << "success " << outfile << std::endl;
    }
    else
    {
        std::cout << "failed " << outfile << std::endl;
    }
	return 0;
	

}

