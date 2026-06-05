

#include "OSGEditor/LodTreeProcessor.h"
#include "Core/String.h"
#include "Core/File.h"
#include <filesystem>
#include <osgUtil/Optimizer>
#include "Core/Application.h"
#include "OSGEditor/TModelIO.h"

std::string levelmaskstr[2] = { "L","Level" };
//LodTreeProcessor::LodTreeProcessor(/*std::string filename,*/ LODTree* tree)
//{
//	level_count_ = 0;
//	tree_ = tree;
//}
//LodTreeProcessor::LodTreeProcessor(std::vector<std::string>& filenamelist, LODTree* tree)
//{
//	level_count_ = 0;
//	tree_ = tree;
//}





LodTreeProcessor::LodTreeProcessor()
{
	level_count_ = 0;
	tree_ = NULL;
}

LodTreeProcessor::~LodTreeProcessor()
{
	

}
bool isnumber(std::string str)
{
	for (int chi = 0; chi < str.length(); chi++)
	{
		if (isdigit(str[chi]))
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
};



void LodTreeProcessor::GetTileDirHighestLevelTreesByL(std::string dir,  std::vector<std::string>& trees, std::string extension)
{
	AI3D::CORE::String::StringToLower(&extension);

	dir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(dir));
	std::vector<std::string> deepsetdirs;
	AI3D::CORE::File::FindDeepsestDir(dir, deepsetdirs);


	//找出L后边数字最大然后再找上一级



	if (!deepsetdirs.empty())
	{



		std::string level_str = "";
		for (int i = 0; i < deepsetdirs.size(); i++)
		{
			int levelnum = 0;
			std::vector<std::string> files = AI3D::CORE::File::GetFileList(deepsetdirs[i], extension);
			if (files.empty())
				continue;
			if (level_str == "")
			{
				auto getlevelstr = [&files]()->std::string
				{

					std::string level_str = "";
					for (int fileidx = 0; fileidx < files.size(); fileidx++)
					{
						std::string nametemp = AI3D::CORE::File::GetFileNameWithoutExtension(files[fileidx]);
						/*names.push_back(name); */
						//提取L后面的数字
						std::vector<std::string> strstemp = AI3D::CORE::String::StringSplit(nametemp, "_");
						for (int fileidxvec = 0; fileidxvec < strstemp.size(); fileidxvec++)
						{
							//获取基础的level字符串
							for (int sizel = 0; sizel < 2; sizel++)
							{
								std::string lstr = levelmaskstr[sizel];
								AI3D::CORE::String::StringToLower(&lstr);
								AI3D::CORE::String::StringToLower(&strstemp[fileidxvec]);
								char a = strstemp[fileidxvec].at(0);
								std::string thefirststr(1, a);
								if (strstemp[fileidxvec].find(lstr) != std::string::npos && thefirststr == "l")
								{
									std::string itertemp = strstemp[fileidxvec];
									std::string numstr = itertemp.erase(0, lstr.length());

									if (isnumber(numstr))
									{
										return lstr;

									}

								}
							}
						}
					}
					return "";
				};

				level_str = getlevelstr();
			}
			if (level_str == "")
			{
				continue;
			}
			std::map< int, std::vector<std::string>> levelmap;
			//获取所有文件的文件名
			for (int fileidx = 0; fileidx < files.size(); fileidx++)
			{
				std::string name = AI3D::CORE::File::GetFileNameWithoutExtension(files[fileidx]);
				/*names.push_back(name); */
				//提取L后面的数字
				std::vector<std::string> strs = AI3D::CORE::String::StringSplit(name, "_");
				for (auto iter : strs)
				{
					/*for (int sizel = 0; sizel < 2; sizel++)*/
					{
						/*std::string lstr = levelmaskstr[sizel];
						AI3D::CORE::String::StringToLower(&lstr);*/
						AI3D::CORE::String::StringToLower(&iter);
						char a = iter.at(0);
						std::string thefirststr(1, a);
						if (iter.find(level_str) != std::string::npos && thefirststr == "l")
						{
							std::string itertemp = iter;
							std::string numstr = itertemp.erase(0, level_str.length());

							if (isnumber(numstr))
							{
								int num = std::atoi(numstr.c_str());
								if (levelnum < num)
									levelnum = num;
								levelmap[num].push_back(files[fileidx]);
							}

						}
					}

				}
			}

			//获取到最大的数字后，获取本层和倒数第二层的
			for (auto itermap : levelmap)
			{
				if (itermap.first == levelnum - 1 || itermap.first == levelnum)
				{
					for (auto itervec : itermap.second)
					{


						if (IsLeafNode(itervec))//武大的末节点组织很奇怪，有pagelod但没有孩子，所以在此设置
						{
							trees/*[deepsetdirs[i]]*/.push_back(itervec);
						}

					}
				}
			}



		}
	}
	return;
}

//
//void LodTreeProcessor::GetTileDirHighestLevelTreesByL(std::string dir, std::map<std::string, std::vector<std::string> >& trees, std::string extension)
//{
//	AI3D::CORE::String::StringToLower(&extension);
//
//	dir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(dir));
//	std::vector<std::string> deepsetdirs;
//	AI3D::CORE::File::FindDeepsestDir(dir, deepsetdirs);
//	
//
//	//找出L后边数字最大然后再找上一级
//	
//
//	
//	if (!deepsetdirs.empty())
//	{
//		
//
//
//		std::string level_str = "";
//		for (int i = 0; i < deepsetdirs.size(); i++)
//		{
//			int levelnum = 0;
//			std::vector<std::string> files = AI3D::CORE::File::GetFileList(deepsetdirs[i], extension);
//			if (files.empty())
//				continue;
//			if (level_str == "")
//			{
//				auto getlevelstr = [&files]()->std::string
//				{
//					
//					std::string level_str = "";
//					for (int fileidx = 0; fileidx < files.size(); fileidx++)
//					{
//						std::string nametemp = AI3D::CORE::File::GetFileNameWithoutExtension(files[fileidx]);
//						/*names.push_back(name); */
//						//提取L后面的数字
//						std::vector<std::string> strstemp = AI3D::CORE::String::StringSplit(nametemp, "_");
//						for (int fileidxvec = 0; fileidxvec < strstemp.size(); fileidxvec++)
//						{
//							//获取基础的level字符串
//							for (int sizel = 0; sizel < 2; sizel++)
//							{
//								std::string lstr = levelmaskstr[sizel];
//								AI3D::CORE::String::StringToLower(&lstr);
//								AI3D::CORE::String::StringToLower(&strstemp[fileidxvec]);
//								char a = strstemp[fileidxvec].at(0);
//								std::string thefirststr(1, a);
//								if (strstemp[fileidxvec].find(lstr) != std::string::npos && thefirststr == "l")
//								{
//									std::string itertemp = strstemp[fileidxvec];
//									std::string numstr = itertemp.erase(0, lstr.length());
//
//									if (isnumber(numstr))
//									{
//										return lstr;
//
//									}
//
//								}
//							}
//						}
//					}
//					return "";
//				};
//
//				level_str = getlevelstr();
//			}
//			if (level_str == "")
//			{
//				continue;
//			}
//			std::map< int, std::vector<std::string>> levelmap;
//			//获取所有文件的文件名
//			for (int fileidx = 0; fileidx < files.size(); fileidx++)
//			{
//				std::string name = AI3D::CORE::File::GetFileNameWithoutExtension(files[fileidx]);
//				/*names.push_back(name); */
//				//提取L后面的数字
//				std::vector<std::string> strs = AI3D::CORE::String::StringSplit(name,"_");
//				for (auto iter : strs)
//				{
//					/*for (int sizel = 0; sizel < 2; sizel++)*/
//					{
//						/*std::string lstr = levelmaskstr[sizel];
//						AI3D::CORE::String::StringToLower(&lstr);*/
//						AI3D::CORE::String::StringToLower(&iter);
//						char a = iter.at(0);
//						std::string thefirststr(1,a);
//						if (iter.find(level_str) != std::string::npos && thefirststr =="l")
//						{
//							std::string itertemp = iter;
//							std::string numstr = itertemp.erase(0, level_str.length());
//							
//							if(isnumber(numstr))
//							{
//								int num = std::atoi(numstr.c_str());
//								if (levelnum < num)
//									levelnum = num;
//								levelmap[num].push_back(files[fileidx]);
//							}
//
//						}
//					}
//					
//				}
//			}
//			
//			//获取到最大的数字后，获取本层和倒数第二层的
//			for (auto itermap: levelmap)
//			{
//				if (itermap.first == levelnum - 1  || itermap.first == levelnum)
//				{
//					for (auto itervec : itermap.second)
//					{
//						
//
//						if (IsLeafNode(itervec))//武大的末节点组织很奇怪，有pagelod但没有孩子，所以在此设置
//						{
//							trees[deepsetdirs[i]].push_back(itervec);
//						}
//
//					}
//				}
//			}
//			
//
//
//		}
//	}
//	return;
//}

bool  LodTreeProcessor::IsLeafNode(std::string file)
{
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(file);
	std::vector<std::string> children;
	LodTreeProcessor::ExtractNode(node, children, AI3D::CORE::File::GetParentDir(file));

	return (children.empty());
	

}

void LodTreeProcessor::GetLastDirLevelTreeFiles(std::vector<std::string>& input, std::vector<std::string>& resulttrees)
{
	for (int i = 0; i < input.size(); i++)
	{
		LodTreeProcessor gridtemp;
		gridtemp.ExtractTree(input[i]);
		/*std::vector<std::string> resultlod;*/
		std::vector<LODTree*> resultlod;
		gridtemp.GetLastDirLevelTrees(gridtemp.GetTree(), resultlod);

		for (int idx = 0; idx < resultlod.size(); idx++)
		{
			resulttrees.push_back(resultlod[idx]->GetName());

		}

	}
}

//现获取最顶层的然后找到每个树的最精细层
void  LodTreeProcessor::GetLastDirLevelTrees( LODTree* tree, std::vector<LODTree*>& resulttrees)
{
	if (tree == NULL)
		return;
	std::string filetemp = tree->GetName();
	std::string root, extend;
	AI3D::CORE::File::SplitFileExtension(filetemp, &root, &extend);
	std::vector<LODTree*> _trees;
	/*GetTopLevelTrees(dir, _trees, extension);*/
	//对每个最顶层找最精细层；
	//for
	//找到所有最精细层的节点，归类出其文件夹,并找出该文件夹下的顶层
	GetHighestLevelTrees(tree,_trees);
	std::map<std::string, std::string> tile_map;
	std::set<std::string> tile_set;
	//std::vector<LODTree*>  resulttrees;
	//

	for (int i = 0; i < _trees.size(); i++)
	{
		
		std::string name = _trees[i]->GetName();
		std::string path = AI3D::CORE::File::GetParentDir(name);
		if (tile_set.count(path))
			continue;
		
		std::string parentpath;
		auto parent = _trees[i]->GetParent();
		int level = _trees[i]->GetLevel();
		if (parent != NULL)
		{
			std::string parentname = parent->GetName();
			parentpath = AI3D::CORE::File::GetParentDir(parentname);

		
			while (path == parentpath)
			{
				if (level > parent->GetLevel())
				{
					level = parent->GetLevel();
					parent = parent->GetParent();
					if (parent != NULL)
					{
						std::string parentname = parent->GetName();
						parentpath = AI3D::CORE::File::GetParentDir(parentname);
					}
					else
						break;
				}
			}
		}
		std::vector<LODTree*> treestemp;
		tree->GetLevelNodes(level, treestemp);
		if (!treestemp.empty())
		{
			resulttrees.insert(resulttrees.end(), treestemp.begin(), treestemp.end());
			for (int idx = 0; idx < treestemp.size(); idx++)
			{
				std::string parentname = treestemp[idx]->GetName();
				parentpath = AI3D::CORE::File::GetParentDir(parentname);
				tile_set.insert(parentpath);
			}
		}
	
	}
	return /*resulttrees*/;
}



void LodTreeProcessor::GetTileDirCoarseLevelTrees(std::string dir, std::vector<std::string>& trees, std::string extension )
{
	AI3D::CORE::String::StringToLower(&extension);

	dir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(dir));
	std::vector<std::string> deepsetdirs;
	AI3D::CORE::File::FindDeepsestDir(dir, deepsetdirs);
	//在每个最深的文件夹下找最短名字的文件


	int validnum = 0;
	if (!deepsetdirs.empty())
	{
		for (int i = 0; i < deepsetdirs.size(); i++)
		{
			std::vector<std::string> files = AI3D::CORE::File::GetFileList(deepsetdirs[i], extension);

			if (files.empty())
			{
				continue;
			}
				//则根据名字最小的取

				int min = files.at(0).size();
				std::string minpath = files.at(0);
				for (int j = 0; j < files.size(); j++)
				{
						if (min > files.at(j).size())
						{
							min = files.at(j).size();
						
						}
					
				}
				//可能有长度一致的.跟寻找根节点不同，因为根节点必须保证只有一个文件，但是这个不一样的地方如acroosstile他最后一层可能会有好几个，因为其根节点在文件夹外
				for (int j = 0; j < files.size(); j++)
				{
					if (files.at(j).size() == min)
					{
						minpath = files.at(j);
						trees.push_back(minpath);
					}
				}
				

		}

		
	}
	return;
}

//输入的路径下有可能
//先查找当前目录下是否有节点文件
//有则看是否有多个如果只有一个则只解析此
//多余一个则解析多个
//若没有 则搜索子目录 有子目录继续上一层的处理方式
//如没有则退出
//待改：使之支持最后一层
void LodTreeProcessor::GetTopLevelFiles(std::string dir,std::vector<std::string>& trees, std::string extension)
{
	AI3D::CORE::String::StringToLower(&extension);
	
	dir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(dir));
	std::vector<std::string>  files = AI3D::CORE::File::GetFileList(dir, extension);
	

	int validnum = 0;
	if (!files.empty())
	{
		std::string dirtemp = AI3D::CORE::File::GetParentDir(files[0]);
		if (AI3D::CORE::File::GetDirList(dirtemp).empty())//最后一层
		{
			
			if (files.size() == 1)
			{
						trees.push_back(files[0]);
						validnum++;	
			}
			else if (files.size() > 1)
			{
				//则根据名字最小的取

				int min = files.at(0).size();
				std::string minpath = files.at(0);
				for (int i = 0; i < files.size(); i++)
				{
					
					{
						if (min > files.at(i).size())
						{
							min = files.at(i).size();
							minpath = files.at(i);
						}
					}
				}
				trees.push_back(minpath);
				validnum++;
			}
		}
		else
		{
			for (int i = 0; i < files.size(); i++)
			{

				std::string filetemp = files[i];
				trees.push_back(filetemp);
				validnum++;


			}
		}
	}
	if (validnum > 0)
		return;
	if (files.empty()|| validnum==0)//纯文件夹
	{
		std::vector<std::string>  files = AI3D::CORE::File::GetDirList(dir);
		if (files.empty())
		{
			return;
			/*GetTopLevelTrees(filetemp, trees, extension);*/

		}
		else
		{
			for (int i = 0; i < files.size(); i++)
			{
				std::string filetemp = files[i];
				GetTopLevelFiles(filetemp, trees, extension);
			}
		}
	}
	return;


	

}



void LodTreeProcessor::ExtractNode
	(osg::ref_ptr<osg::Node> node, std::vector<std::string> & sameNode, std::string path)
{
	osg::ref_ptr<osg::PagedLOD> pagedLodNode = dynamic_cast<osg::PagedLOD*>(node.get());
	osg::ref_ptr<osg::Group> groupNode = node->asGroup();
	osg::ref_ptr<osg::Geode> geodeNode = node->asGeode();
	
	if(pagedLodNode.get() != NULL)
	{
		int numFileName = pagedLodNode->getNumFileNames();
		std::string filePath = pagedLodNode->getDatabasePath();

		const std::filesystem::path p = AI3D::CORE::File::BoostPathFromUtf8(filePath);
		if (p.is_relative())
		{
			std::string path1 = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(path));
			
			
			filePath = path1 + filePath;
			 
		}
		for (int i = 0; i < numFileName; i++)
		{
			if ( pagedLodNode->getFileName(i) != "")
			{
				
				std::string file = filePath+pagedLodNode->getFileName(i);
				if (AI3D::CORE::File::ExistsPath(file))
				{
					sameNode.push_back(file);
				}
			}			
		}
	}
	else if (groupNode.get() != NULL)
	{
		/** 获取节点的孩子节点个数.*/
		int childNum = groupNode->getNumChildren();

		for(int ci=0; ci<childNum; ++ci)
		{
			osg::ref_ptr<osg::Node> childNode = groupNode->getChild(ci);
			if(childNode.get() != NULL)
				ExtractNode(childNode, sameNode, path);
		}
	}
	/*else if(geodeNode.get() != NULL){}*/
}

bool LodTreeProcessor::ExtractSameLevelList(std::vector<std::string> &sameLevel, std::string basepath)
{
	if(sameLevel.empty()) 
		return false;
	std::vector<std::string> getSameNode;
	vector<LODTree*> result;
	/*if (level_count_ >= 8)
	{
		std::cout << 1 << std::endl;
	}*/
	tree_->GetLevelNodes(level_count_, result);
	for (unsigned int i = 0 ; i < sameLevel.size(); i++)
	{
		/*if (result[i] != NULL)
		{

		}*/
		std::vector<std::string> getSameNodetemp;
		LODTree* parent = result[i];
		
		osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(sameLevel[i]);
		if (node == NULL)
		{
			continue;
		}
			ExtractNode(node, getSameNodetemp,basepath);
			/*parent->getTreeLevelMutual() += 1;*/
		for (int j = 0; j < getSameNodetemp.size(); j++)
		{
			LODTree* temp = new LODTree(getSameNodetemp[j]);
			temp->SetLevel(level_count_+1);
			temp->SetParent(parent);
			result[i]->GetChildren().push_back(temp);
			/*result[i]->GetChildren()[j]->getTreeLevelMutual() = parent->GetLevel()-1;*/
		}
		
		getSameNode.insert(getSameNode.end(),getSameNodetemp.begin(), getSameNodetemp.end());
			//return false;
	
	}
	/** Now getSameNode include  all filenames in the next 
	 * level of the sameLevel. 
	*/
	if(getSameNode.empty())  
		return false;
	filenames_vec_.push_back(getSameNode);
	level_count_++;
	/*if (level_count_ != 0)*/
	{
		/*tree->getTreeLevelMutual() += 1;*/

		for (int j = 0; j <= level_count_; j++)
		{
			vector<LODTree*> resulttemp;

			tree_->GetLevelNodes(j, resulttemp);
			for (int i = 0; i < resulttemp.size(); i++)
			{
				bool a = resulttemp[i]->GetChildren().empty();
				if (level_count_ > resulttemp[i]->GetLevel() && a)
				{
					/*std::cout << resulttemp[i]->GetName() << " " << j << " " << resulttemp[i]->GetLevel() << std::endl;*/
				}
				else
				/*if (resulttemp[i]->GetName().find("Tile_+087_+096_L22_000002210") != std::string::npos)
				{
					std::cout << resulttemp[i]->GetName()<< " " << j<<" " << resulttemp[i]->GetLevel() << std::endl;
				}*/
				{
					resulttemp[i]->GetMaxLevelMutual() += 1;
				}
			}
		}
	}
	return true;
}
int LodTreeProcessor::GetLevelCount()
{
	return level_count_;
}
LODTree* LodTreeProcessor::GetTree()
{
	return tree_;
}



class DefaultNormalsGeometryVisitor
	: public osg::NodeVisitor
{
public:

	DefaultNormalsGeometryVisitor()
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
	}

	virtual void apply(osg::Geode& geode)
	{
		for (unsigned int ii = 0; ii < geode.getNumDrawables(); ++ii)
		{
			osg::ref_ptr< osg::Geometry > geometry = dynamic_cast<osg::Geometry*>(geode.getDrawable(ii));
			if (geometry.valid())
			{
				osg::ref_ptr< osg::Vec3Array > newnormals = new osg::Vec3Array;
				newnormals->push_back(osg::Z_AXIS);
				geometry->setNormalArray(newnormals.get(), osg::Array::BIND_OVERALL);
			}
		}
	}

	virtual void apply(osg::Node& node)
	{
		traverse(node);
	}

};


int LodTreeProcessor::MergeMeshImpl(std::vector<std::string> file_name, std::string fileNameOut,
	std::string baseosgbfile, mergeoptions_s options)
{
	
	if (file_name.size()<2)
		return -1;
	std::string fileroot, fileext;
	AI3D::CORE::File::SplitFileExtension(file_name[0], &fileroot, &fileext);
	AI3D::CORE::String::StringToLower(&fileext);
	
	
	osg::ref_ptr<osg::Node> root;
	osg::ref_ptr<osgDB::ReaderWriter::Options> obj_options = new osgDB::ReaderWriter::Options;

	std::string imgexten = "";
	CollectGeom collect_geom;
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(file_name[0], options.readptions_);
	node->accept(collect_geom);
	GeomArr geom_arr = collect_geom.GetGeomArr();

	unsigned int num_geom = geom_arr.size();
	std::cout << "num_geom "<< num_geom << std::endl;
	for (unsigned int i = 0; i < num_geom; i++)
	{
		osg::ref_ptr<osg::StateSet> stateset = geom_arr[i]->getStateSet();
		osg::ref_ptr<osg::Texture2D> tex2D = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
		if (tex2D.valid())
		{
			osg::ref_ptr<osg::Image> teximg = tex2D->getImage();
			if (teximg.valid())
			{
				std::string imgpath = tex2D->getImage()->getFileName();

				imgexten = osgDB::getFileExtensionIncludingDot(imgpath);
				AI3D::CORE::String::StringToLower(&imgexten);
				break;
			}
		}
	}
	std::vector<osg::ref_ptr<osg::Node>> nodes;
	for (int file_index = 0; file_index < file_name.size(); file_index++)
	{
		
		
		osg::ref_ptr<osg::Object> object = osgDB::readNodeFile(file_name[file_index], options.readptions_);

		if (object.valid())
		{
			if (object->asNode())
			{
				nodes.push_back(object->asNode());
				
				
			}
		}
	}
	
	
	if (nodes.size() == 1)
	{
		root = nodes.front();
	}
	else if (nodes.size() > 1)
	{
		osg::ref_ptr<osg::Group> group = new osg::Group;
		for (std::vector<osg::ref_ptr<osg::Node>>::iterator itr = nodes.begin(); itr != nodes.end(); ++itr)
		{
			group->addChild(itr->get());
		}
		root = group;
	}
	if (root.valid())
	{
		osgUtil::Optimizer optimizer;
		optimizer.optimize(root.get());
	}
	if (options.do_overallnormal_)
	{
		DefaultNormalsGeometryVisitor dngv;
		root->accept(dngv);
	}

	std::cout << "fileNameOut " << fileNameOut << std::endl;
	osg::ref_ptr<osgDB::ReaderWriter::Options> osgb_options = options.saveptions_;
	std::string outdir = (fileNameOut + "/");
	const std::filesystem::path outdirPath = AI3D::CORE::File::BoostPathFromUtf8(outdir);
	if (!std::filesystem::exists(outdirPath))
	{
		std::filesystem::create_directory(outdirPath);

	}
	
	std::vector<std::string> types = AI3D::CORE::String::StringSplit(options.outfiletypes_, " ");
	fileNameOut = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(fileNameOut));;
	fileNameOut += baseosgbfile  ;
	for (auto iter : types)
	{
		std::string  outfilebase = fileNameOut+ iter;
		if (AI3D::CORE::File::ExistsPath(outfilebase))
		{
			AI3D::CORE::File::BackupFile(outfilebase);
		}
		bool result = osgDB::writeNodeFile(*root, outfilebase, osgb_options);
		//如果原始是osgb 输出是obj则需要拷贝纹理
		AI3D::CORE::String::StringToLower(&iter);
		if (fileext == ".osgb" && iter == ".obj")
		{
			
			std::vector<std::string> tilenames;
			for (auto itervec : file_name)
			{
				std::string tilename = itervec;
				std::string tileshortname = AI3D::CORE::File::GetDirName(tilename, false);
				tilenames.push_back(tileshortname);
			}
			std::string apppath = AI3D::CORE::Application::Getinstance().GetAPPPath();
			std::vector<std::string> imgfiles = AI3D::CORE::File::GetFileList(apppath, imgexten);
			AI3D::CORE::File::CopyFiles(imgfiles, outdir);
			
		}


		if (result)
		{
			std::cout << "Data written to '" << outfilebase  << std::endl;

		}
		else {
			std::cout << "Data written error" << std::endl;

		}
	}

	
	return 1;

}

int LodTreeProcessor::MergeMeshImpl(std::vector<std::string> file_name, std::string fileNameOut, std::string baseosgbfile, bool outputTexture, std::vector<std::string>  texprix)
{
	bool do_overallNormal = false;
	osg::ref_ptr<osg::Node> root;
	osg::ref_ptr<osgDB::ReaderWriter::Options> obj_options = new osgDB::ReaderWriter::Options;

	if (outputTexture)
	{
		obj_options->setOptionString("OutputTextureFiles");          // 输出纹理
	}
	
	std::vector<osg::ref_ptr<osg::Node>> nodes;
	for (int file_index = 0; file_index < file_name.size(); file_index++)
	{
		osg::ref_ptr<osgDB::ReaderWriter::Options> read_options = new osgDB::ReaderWriter::Options;
		read_options->setOptionString("noTriStripPolygons");
		
		read_options->setOptionString("noRotation");
		osg::ref_ptr<osg::Object> object = osgDB::readNodeFile(file_name[file_index], read_options);
		
		if (object.valid())
		{			
			if (object->asNode())
			{
							nodes.push_back(object->asNode());		

			}
		}
	}
	if (nodes.size() == 1)
	{
		root = nodes.front();
	}
	else if (nodes.size() > 1)
	{
		osg::ref_ptr<osg::Group> group = new osg::Group;
		for (std::vector<osg::ref_ptr<osg::Node>>::iterator itr = nodes.begin(); itr != nodes.end(); ++itr)
		{
			group->addChild(itr->get());
		}
		root = group;
	}
	if (root.valid())
	{
		osgUtil::Optimizer optimizer;
		optimizer.optimize(root.get());
	}
	if (do_overallNormal)
	{
		DefaultNormalsGeometryVisitor dngv;
		root->accept(dngv);
	}
	osg::ref_ptr<osgDB::ReaderWriter::Options> osgb_options = new osgDB::ReaderWriter::Options;

	osgb_options->setOptionString("Compressor=0");          // 设置压缩
	osgb_options->setOptionString("WriteImageHint=IncludeFile");
	if (outputTexture)
	{
		osgb_options->setOptionString("OutputTextureFiles");          // 输出纹理
	}
	std::string outosgbfile = fileNameOut + "/" + baseosgbfile + ".osgb";
	if (AI3D::CORE::File::ExistsPath(outosgbfile))
	{
		AI3D::CORE::File::BackupFile(outosgbfile);
	}
	bool result = osgDB::writeNodeFile(*root, fileNameOut + "/" + baseosgbfile + ".osgb", osgb_options);

	if (result) 
	{
		std::cout << "Data written to '" << fileNameOut << baseosgbfile << " .osgb'." << std::endl;

	}
	else {
		std::cout << "Data written error" << std::endl;

	}
	std::string outdir = (fileNameOut + "/");
	const std::filesystem::path outdirPath = AI3D::CORE::File::BoostPathFromUtf8(outdir);
	if (!std::filesystem::exists(outdirPath))
	{
		std::filesystem::create_directory(outdirPath);

	}
	bool result2 = osgDB::writeNodeFile(*root, outdir + "/" + baseosgbfile + ".osg", obj_options);

	/*if (result2) {
		std::cout << "Data written to '" << outdir + "/" + baseosgbfile << ".osg'." << std::endl;

	}
	else {
		std::cout << "Data written error" << std::endl;

	}*/


	bool result1 = osgDB::writeNodeFile(*root, outdir + "/" + baseosgbfile + ".obj", obj_options);
	if (result1)
	{
		const std::filesystem::path appRoot = AI3D::CORE::File::BoostPathFromUtf8(apppath + "/");
		const std::filesystem::path outdirPathTex = AI3D::CORE::File::BoostPathFromUtf8(outdir);

		for (const auto& entry : std::filesystem::directory_iterator(appRoot))
		{
			if (!entry.is_regular_file())
				continue;
			std::string filepath = AI3D::CORE::File::BoostPathToUtf8String(entry.path().filename());
			if (AI3D::CORE::File::BoostPathToUtf8String(entry.path().extension()) != ".jpg")
				continue;
			for (int ii = 0; ii < texprix.size(); ii++)
			{
				std::string str = texprix[ii];
				//	if ((itr->path().extension().string() == ".json"))
				{
					if ((filepath.find(str) != std::string::npos))
					{

						const std::filesystem::path destPath = outdirPathTex / entry.path().filename();
						std::filesystem::copy_file(entry.path(), destPath, std::filesystem::copy_options::overwrite_existing);
						if (std::filesystem::exists(entry.path()))
							std::filesystem::remove(entry.path());
					}
				}
			}
		}

		std::cout << "Data written to '" << outdir + "/" + baseosgbfile << ".obj'." << std::endl;

	}
	else {
		std::cout << "Data written error" << std::endl;

	}
	const std::filesystem::path osgResidual = AI3D::CORE::File::BoostPathFromUtf8(outdir + "/" + baseosgbfile + ".osg");
	if (std::filesystem::exists(osgResidual))
		std::filesystem::remove(osgResidual);
	return 1;
}


void LodTreeProcessor::MergeMeshes(const std::vector<std::string>& inputfiles, std::string outpath,  mergeoptions_s options)
{
	if (inputfiles.empty())
		return;
	std::string fileNameOut = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(outpath));;
	
	std::map<std::string, std::vector<std::string>> meshfilemap, meshfilemap1;//first:父目录，文件
	std::map<std::string, std::vector<std::string>> name_idx_map;//first：合并后的文件名，sec文件目录

	std::string root, ext;
	AI3D::CORE::File::SplitFileExtension(inputfiles[0], &root, &ext);
	for (int i = 0; i < inputfiles.size(); i++)
	{
		std::string file = inputfiles[i];

		std::string tiledirname = AI3D::CORE::File::GetParentDir(file);//最后一个文件夹

		std::string meshname = AI3D::CORE::File::GetPathBaseName(file);//模型文件名
		meshfilemap[tiledirname].push_back(file);
	}
	for (auto iter : meshfilemap)
	{
		std::string tiledirname = iter.first;
		//上一级目录的名字
		std::string lastparentdir = AI3D::CORE::File::GetDirName(tiledirname);
		std::string dir = AI3D::CORE::File::GetParentDir(tiledirname);
		dir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(dir));
		meshfilemap1[dir].insert(meshfilemap1[dir].end(),iter.second.begin(), iter.second.end());
	}
	for (auto iter : meshfilemap1)
	{
		std::string tiledirname = iter.first;
		//上一级目录的名字
		std::string lastparentdir = AI3D::CORE::File::GetDirName(tiledirname);
		std::string dir = AI3D::CORE::File::GetParentDir(tiledirname);
		dir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(dir));
		//搜索该目录下是否有根节点文件，如果有则以该节点文件名为名，如没有则以该目录的最后一个文件夹为名
		std::vector<std::string> files = AI3D::CORE::File::GetFileList(iter.first, ext);
		std::string mergedname = "";
		for (int idx = 0; idx < files.size(); idx++)
		{
			LodTreeProcessor gridtemp;
			gridtemp.ExtractTree(files[idx],1);//武大的osgb特别多效率低下因此不再extracttree而是改为直接判断是否有文件，如果有则认为是根节点文件
			if (!gridtemp.GetTree()->GetChildren().empty())
			{
				if (AI3D::CORE::File::ExistsPath(gridtemp.GetTree()->GetChildren()[0]->GetName()))
				{
					mergedname = files[idx];
					break;
				}
			}

		}

		if (mergedname == "")
		{
			mergedname = dir+ lastparentdir + ext;
		}
		for (int idx = 0; idx < iter.second.size(); idx++)
			name_idx_map[mergedname].push_back(iter.second[idx]);

	}
	for (auto iter : name_idx_map)
	{
		std::string name = AI3D::CORE::File::GetFileNameWithoutExtension(iter.first);
		if (iter.second.empty())
		{
			continue;
		}
		/*std::vector<std::string> tilenames;
		for (auto itervec : iter.second)
		{
			std::string tilename = itervec;
			std::string tileshortname = AI3D::CORE::File::GetDirName(tilename, true);
			tilenames.push_back(tileshortname);
		}*/
		
		MergeMeshImpl(iter.second, fileNameOut, name, options);
		/*MergeMeshImpl(iter.second, fileNameOut, name, true, tilenames);*/
	}


	//for (int i = 0; i < inputfiles.size(); i++)
	//{

	//	std::string file = inputfiles[i];

	//	std::string tilename = AI3D::CORE::File::GetParentDir(file);
	//	std::string name = AI3D::CORE::File::GetPathBaseName(file);
	//	QString qtilename = QString::fromStdString(tilename);

	//	//拷贝文件
	//	QString a = qtilename.remove(baseosgpath);

	//	fileNameOut += a.toStdString();

	//	QFileInfo fileinfo1(QString::fromStdString(fileNameOut));
	//	QString tpath1 = fileinfo1.filePath();

	//	QStringList parts1 = tpath1.split('/');
	//	std::string pathtemp = parts1.at(0).toStdString();
	//	for (int i = 1; i < parts1.size() - 1; i++)
	//	{
	//		pathtemp += "/" + parts1.at(i).toStdString();
	//		if (!boost::filesystem::exists(pathtemp))
	//		{
	//			std::cout << pathtemp << std::endl;
	//			boost::filesystem::create_directory(pathtemp);

	//		}
	//	}

	//	fileNameOut = pathtemp + "/";
	//	if (!boost::filesystem::exists(fileNameOut))
	//	{
	//		std::cout << fileNameOut << std::endl;
	//		boost::filesystem::create_directory(fileNameOut);
	//		fileNameOut = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(fileNameOut));
	//		std::string newfile = fileNameOut + name;
	//		boost::filesystem::copy_file(file, newfile, boost::filesystem::copy_option::overwrite_if_exists);
	//		std::string tilename1 = AI3D::CORE::File::GetParentDir(newfile);

	//		meshfilemap[tilename1].push_back(newfile);

	//	}
	//}

	//for (auto iter : meshfilemap)
	//{
	//	std::string tilename = iter.first;
	//	std::string tileshortname = AI3D::CORE::File::GetDirName(tilename, true);
	//	std::vector<std::string> tilenames(1, tileshortname);
	//	//基础名字的获取原则
	//	//获取当前路径下哪个含有那些子块，有则以此为名
	//	//若当前路径下没有则命名为model
	//	std::string basefilename = "Model";

	//	MergeMeshImpl(iter.second, fileName.toStdString(), basefilename, true, tilenames);
	//}
}


void  LodTreeProcessor::GetHighestLevelTrees(LODTree* tree, std::vector<std::string>& result)
{
	/*std::vector<LODTree*> last_level_trees;*/
	if (tree->IsLeafNode())
	{
		result.push_back(tree->GetName());
	}
	else
	{
		for (int i = 0; i < tree->GetChildren().size(); i++)
		{

			GetHighestLevelTrees(tree->GetChildren()[i], result);

		}

	}

	return;

}

void  LodTreeProcessor::GetHighestLevelTrees(LODTree* tree, std::vector<LODTree*>& result)
{
	/*std::vector<LODTree*> last_level_trees;*/
	if (tree->IsLeafNode())
	{
		result.push_back(tree);
	}
	else
	{
		for (int i = 0; i < tree->GetChildren().size(); i++)
		{
			
			GetHighestLevelTrees( tree->GetChildren()[i], result);
			
		}
		
	}
	
	return ;

}

void LodTreeProcessor::ExtractTree(std::string filename,int levelcancelled)
{
	std::vector<std::string> sameLevel;
	if (filename.empty()) return;
	sameLevel.push_back(filename);
	filenames_vec_.push_back(sameLevel);

	if (filenames_vec_.empty()) return;

	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename);
	
	tree_ = new LODTree(filename);
	tree_->GetMaxLevelMutual() += 1;
	level_count_ = 0;
	bool flag = true;
	std::string basepath = AI3D::CORE::File::GetParentDir(filename);
	
	while (flag && level_count_ <= levelcancelled)
	{
		std::vector<std::string>  sameLevels = filenames_vec_.back();
		
		flag = ExtractSameLevelList(sameLevels, basepath);
		
		
	}
	
}
////调此函数需要先提取出每个tile的顶层osgb文件
//void LodTreeProcessor::ExtractTree(std::vector<std::string> &filenamelist)
//{
//	if (filenamelist.empty()) return;
//	filenames_vec_.push_back(filenamelist);
//
//	if(filenames_vec_.empty()) return;
//
//	osg::ref_ptr<osg::Node> node = osgDB::readNodeFiles(filenamelist);
//	
//	int i = 0;
//	bool flag = true;
//	
//	while (flag)
//	{
//		std::vector<std::string>  sameLevels = filenames_vec_.back();
//		flag= ExtractSameLevelList(sameLevels);
//		
//		
//		
//		i++;
//
//		
//	}
//	
//	level_count_ = i;
//}
