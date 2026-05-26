#include <iostream>
#include <fstream>

#include "Core/ProjectObject.h"
using namespace AI3D::CORE;
//#include "Core/reconstruction.h"

//using namespace AI3D::CORE;
//需要测试的几项
//新建工程，判断是否已有该工程；并新建block；
//testNewProject();
////测试重命名，文件名以及tri中也更改;
//testRename();
////测试打开工程，打开工程时需要检测工程的有效性；
//testOpenProject();
////测试save工程,
//testSaveProject();
////test 导入block；
//testImportBlock();
//test newblock；
//test LoadProject
void TestLoadProject()
{
    std::string project_path = "D:/MyLearning/Learning_Materials/run/camera/0012/0012.tri";
    ProjectObject project;
    project.Load(project_path);
}
int main(int argc, char **argv) 
{
    TestLoadProject();
    return EXIT_SUCCESS;
}
