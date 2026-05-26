#include "OSGEditor/LodTreeProcessor.h"
#include "Core/Application.h"
#include "Core/File.h"
//
//test 
int main(int argc, char* argv[])
{
	//
	/*std::string path1 = "//10.10.90.22/HY_Text/Proj/NewProject/Block_2/R1/";
	std::string path2 = "\\10.10.90.22/HY_Text/Proj/NewProject/Block_2/";
	bool a = AI3D::CORE::File::ExistsPath(path1);
	bool b = AI3D::CORE::File::ExistsDir(path1);

	
	std::cout << " " << a << " " << b << std::endl;

	AI3D::CORE::File::CreateDirIfNotExists(path1,true);*/

	std::string path = argv[1];
	std::cout << "path " << path << std::endl;;
	std::vector<std::string> result;
	LodTreeProcessor::GetTileDirCoarseLevelTrees(path, result);
	std::cout << "find tiles " << result.size() << std::endl;;
	//???????las??????
	for (std::vector<std::string>::iterator it = result.begin();
		it != result.end();/*it++*/)	
	{
		std::string temp = *it;
		AI3D::CORE::String::StringToLower(&temp);
		std::string b = "las";
		string::size_type idx = temp.find(b); //??a??????b.
		if (idx != string::npos) //?????
			{
				it = result.erase(it);
		}
		else //
		{
				it++;
		}
		
	}
	std::cout << "all tiles " << result.size()<<std::endl;;

	{
		std::string fileshort = path + "/list.txt";
		//fileshort = AI3D::CORE::String::LocaleToUtf8(fileshort);
		std::ofstream out2 = AI3D::CORE::File::OpenOfstreamUtf8(fileshort, std::ios::trunc);
		if (!out2.is_open())
		{

			std::cout << "file " << fileshort << " open failed ." << std::endl;
			return 1;
		}

		for (std::string iter : result)
		{
			//  iter = AI3D::CORE::String::LocaleToUtf8(iter);
			out2 << iter << std::endl;
		}
		out2.close();
	}

	mergeoptions_s mergeopts;

	if (argc > 2)
	{
		mergeopts.outfiletypes_ = argv[2];
	}
	std::cout << "start merge " << std::endl;;
	int ret = LodTreeProcessor::MergeMeshImpl(result, path, "Model");
	std::cout << "merged " << ret << std::endl;;
	//??????????
	std::string apppath = AI3D::CORE::Application::Getinstance().GetAPPPath();
	std::ofstream file = AI3D::CORE::File::OpenOfstreamUtf8(path+"/run.bat", std::ios::trunc);

	if (!file.is_open())
	{
		return 1;
	}
	std::cout << "out " << std::endl;;
	file << apppath + "/osgconv.exe";
	for (auto& iter : result)
	{
		file << " "<< iter;
	}
	file<<" "<< path+"/show.osgb" << std::endl;
	file << "pause" << std::endl;;
	file << apppath + "/osgviewer.exe --window 100 100 1500 1500 "  << path + "/show.osgb" << std::endl;
	file << "pause" << std::endl;;
	file.close();
	
    return 0;
}

