/*
 * File：LodTreeProcessor
 * Brief：树形结构处理器
 * Author:陈海燕
*/

#ifndef _LODTREEPROCESSOR_H
#define _LODTREEPROCESSOR_H

#include "OSGEditor/Base.h"

#include<osg/Node>
#include<osg/PagedLOD>
#include<osgDB/ReadFile>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include "LODTree.h"


typedef std::vector<std::vector<std::string> > FilesVec;

struct mergeoptions_s
{
	
	osgDB::Options* readptions_ = 0;
	osgDB::Options* saveptions_ = 0;
	std::string outfiletypes_ = ".osgb";
	bool do_overallnormal_ = false;
	bool bviolence = false;//是否直接merge
	mergeoptions_s() 
	{
		std::string optstr="";
	/*	if (readptions_)
		{
			optstr = readptions_->getOptionString();

		}*/
		optstr = std::string("noTriStripPolygons") + std::string(" ") + std::string("noRotation") + std::string(" ") + optstr;
		readptions_ = new osgDB::Options();
		readptions_->setOptionString(optstr);
		optstr = "";
	/*	if (saveptions_)
		{
			optstr = saveptions_->getOptionString();

		}*/
		saveptions_ = new osgDB::Options();
		optstr = std::string("OutputTextureFiles") + std::string(" ") + std::string("WriteImageHint=IncludeFile Compressor=0") + std::string(" ") + optstr;
		saveptions_->setOptionString(optstr);
		outfiletypes_ = std::string(".osgb");// +std::string(" ") + std::string(".obj");
	}
	mergeoptions_s(osgDB::Options* readptions, osgDB::Options* saveptions, std::string outfiletypes)
	{
		readptions_ = readptions;
		saveptions_ = saveptions;
		outfiletypes_ = outfiletypes;
	}
};

class DLL_API LodTreeProcessor
{
	
public:
	
	LodTreeProcessor();
	//LodTreeProcessor(std::vector<std::string>& filenamelist, LODTree* tree);
	~LodTreeProcessor();
	/*void ExtractTree(std::vector<std::string> &filenamelist);*/

	void ExtractTree(std::string filename,int levelcancelled = 999);//第二个参数代表检测到哪一层，-1代表到实际层
	int GetLevelCount();
	LODTree* GetTree();
	int BatchExtractTree(std::vector<std::string>& filenamelist, std::vector<LODTree*>& trees) 
	{
		return 1000;
	};
	//根据文件夹获取该文件夹下所有的树 

	//获取同一层的所有节点
	std::vector<std::string> GetLevelTrees(std::vector<LODTree*> trees, int level) 
	{
		return std::vector<std::string>();
	};
	//获取每个tile块最粗糙的那个文件
	static void GetTileDirCoarseLevelTrees(std::string dir, std::vector<std::string>& trees, std::string extension = ".osgb");
	//根据文件名来找最精细层，因为通过读节点的方式遇到武大的数据时太慢了
	static void GetTileDirHighestLevelTreesByL(std::string dir,  std::vector<std::string> & trees, std::string extension = ".osgb");
	//不用了
	//static void GetTileDirHighestLevelTreesByL(std::string dir, std::map<std::string,std::vector<std::string> >& trees, std::string extension = ".osgb");
	//获取某个树的最深处文件夹下的顶层,先获取最精细层，通过最精细层找到最顶层
	static void GetLastDirLevelTrees( LODTree* tree, std::vector<LODTree*>& resulttrees);
	static void GetLastDirLevelTreeFiles(std::vector<std::string>& input, std::vector<std::string>& resulttrees);
	//获取最顶层的文件，返回的是文件名，这个是用于加载批量的模型文件时因有的文件是在文件夹外层有一个顶层的osgb文件，有的是每个tile块下有一个顶层文件
	static void GetTopLevelFiles(std::string dir, std::vector<std::string>& trees,std::string extension = ".osgb");

	//获取最精细层的所有节点 1:单个文件输出的tree 测试通过
	static void GetHighestLevelTrees(LODTree* tree, std::vector<LODTree*>& result) ;
	static void  GetHighestLevelTrees(LODTree* tree, std::vector<std::string>& result);
	static void ExtractNode(osg::ref_ptr<osg::Node> node, std::vector<std::string>& sameNode, std::string path = "");
	static void MergeMeshes(const std::vector<std::string>& inputfiles,std::string outpath , mergeoptions_s options = mergeoptions_s());
	//此方法不用了
	static int MergeMeshImpl(std::vector<std::string> file_name, std::string fileNameOut, 
		std::string baseosgbfile, bool outputTexture, std::vector<std::string>  texprix/*, std::vector<std::string> outputfiletypes*/);

	static int MergeMeshImpl(std::vector<std::string> file_name, std::string fileNameOut,
		std::string baseosgbfile, mergeoptions_s options = mergeoptions_s());

	static bool IsLeafNode(std::string file);
private:
	
	
	
	bool ExtractSameLevelList(std::vector<std::string> &sameLevel,std::string basepath="");
	LODTree* tree_;
	int level_count_;
	FilesVec filenames_vec_;
};
#endif//_LODTREEPROCESSOR_H