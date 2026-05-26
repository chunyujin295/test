

#ifndef T_MODEL_IO_H
#define T_MODEL_IO_H

#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <cstdio>

#include <osg/Group>
#include <osg/Geode>
#include <osg/Node>
#include <osg/Texture2D>
#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>


typedef std::vector< osg::ref_ptr<osg::Geometry> >          GeomArr;

typedef std::vector< std::vector<osg::Vec3> >              ShpPolyLine;


/**
*	@class TModifyObjVertex
*	@brief 模型顶点访问器，用于修改obj模型坐标
*   @attention
*   @details
*/
class TModifyObjVertex : public osg::NodeVisitor
{
public:
	TModifyObjVertex() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
	}
	~TModifyObjVertex()
	{
		//
	}

	void apply(osg::Geode &geode)
	{
		for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
		{
			osg::ref_ptr<osg::Geometry> geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
			if (!geom.valid())
			{
				std::cout << "Some Geometry Error" << std::endl;
				continue;
			}

			osg::ref_ptr<osg::Vec3Array> verts = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
			if (!verts.valid())
			{
				std::cout << "Some Vertex Error" << std::endl;
				continue;
			}

			/**<修改坐标 */
			for (unsigned int iverts = 0; iverts < verts->size(); iverts++)
			{
				float temp = (verts->at(iverts)).z();
				(verts->at(iverts)).z() = -(verts->at(iverts)).y();
				(verts->at(iverts)).y() = temp;
			}
		}
	}
};


/**
*	@class CollectGeom
*	@brief 用于获得模型中的几何体
*   @attention
*   @details
*/
class CollectGeom : public osg::NodeVisitor
{
public:
	CollectGeom() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
		geomArr_.clear();
	}

	~CollectGeom() {}

	void apply(osg::Geode &geode)
	{
		for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
		{
			osg::ref_ptr<osg::Geometry> geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
			if (!geom.valid())
			{
				std::cout << "Some Geometry Error" << std::endl;
				continue;
			}

			geomArr_.push_back(geom);

		}
	}

	GeomArr GetGeomArr()
	{
		return geomArr_;
	}

private:
	GeomArr          geomArr_;
};


/**
*	纹理格式
*/
enum TextureFormat
{
	JPGTEX,
	BMPTEX,
	PNGTEX,
	DDSTEX,
	RGBTEX,
	TIFTEX,
	TIFFTEX
};



class ModifyTextureName : public osg::NodeVisitor
{
public:

	ModifyTextureName()
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
		tex_basepath_ = std::string();
		tex_basename_ = std::string();
		texformat_ = std::string();
		setTexFormatMap();
		//_numTex = 0;

	}

	~ModifyTextureName() {}

	void setTextureImageBasePath(const std::string texpath, TextureFormat texformat = JPGTEX)
	{
		//_texPath = texpath;
		tex_basepath_ = osgDB::getFilePath(texpath);
		tex_basepath_ += std::string("/");

		tex_basename_ = osgDB::getSimpleFileName(texpath);
		tex_basename_ = osgDB::getNameLessAllExtensions(tex_basename_);

		texformat_ = tex_format_map_[texformat];
	}

	std::vector<std::string> getOutputTexPath()
	{
		return outputtex_path_;
	}

protected:
	void apply(osg::Node &node)
	{
		if (node.getStateSet())
		{
			apply(node.getStateSet());
		}
		traverse(node);
	}

	void apply(osg::Geode &geode)
	{
		if (geode.getStateSet())
		{
			apply(geode.getStateSet());
		}

		unsigned int cnt = geode.getNumDrawables();
		for (unsigned int i = 0; i < cnt; i++)
		{
			apply(geode.getDrawable(i)->getOrCreateStateSet());
		}
		traverse(geode);
	}

	void apply(osg::StateSet *state)
	{
		osg::StateSet::TextureAttributeList &texAttribList = state->getTextureAttributeList();
		for (unsigned int i = 0; i < texAttribList.size(); i++)
		{
			//osg::Texture2D *tex2D = NULL;
			osg::ref_ptr<osg::Texture2D> tex2D = dynamic_cast<osg::Texture2D*>(state->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
			if (tex2D.valid())
			{
				osg::ref_ptr<osg::Image> teximg = tex2D->getImage();
				if (teximg.valid())
				{
					std::string imgpath = tex2D->getImage()->getFileName();
					std::string imgname = osgDB::getSimpleFileName(imgpath);
					std::string imagename_usuffix = osgDB::getNameLessAllExtensions(imgname);

					//string output_tex_name = tex_basename_ + string("_") + getFilled(_numTex, 2) + texformat_;
					std::string output_tex_name = imagename_usuffix + texformat_;
					//修改纹理名称
					teximg->setFileName(output_tex_name);
					//输出纹理图像
					std::string output_tex_path = tex_basepath_ + output_tex_name;

					std::remove(output_tex_path.c_str());     //删除旧文件
					
					//if (!osgDB::fileExists(output_tex_path))
					//{
					osgDB::writeImageFile(*(teximg.get()), output_tex_path);
					outputtex_path_.push_back(output_tex_path);
					//}

					//_numTex++;
				}
			}
		}
	}

private:
	void setTexFormatMap()
	{
		tex_format_map_[JPGTEX] = std::string(".jpg");
		tex_format_map_[BMPTEX] = std::string(".bmp");
		tex_format_map_[PNGTEX] = std::string(".png");
		tex_format_map_[DDSTEX] = std::string(".dds");
		tex_format_map_[RGBTEX] = std::string(".rgb");
		tex_format_map_[TIFTEX] = std::string(".tif");
		tex_format_map_[TIFFTEX] = std::string(".tiff");
	}

private:

	std::string                               tex_basepath_;                      //纹理输出基础路径
	std::string                               tex_basename_;                      //纹理输出基础名字
	std::map<TextureFormat, std::string>      tex_format_map_;                     //纹理格式映射
	std::string                               texformat_;                        //纹理格式
	//unsigned int                         _numTex;                           //纹理计数
	std::vector<std::string>                  outputtex_path_;
};



class InfoVisitor : public osg::NodeVisitor
{
public:
	InfoVisitor()
		:osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
	{
	}

	~InfoVisitor()
	{
	}

	void apply(osg::Geometry& geometry)
	{
		geometry_array.push_back(&geometry);
		if (auto ss = geometry.getStateSet()) 
		{
			//仅考虑只有一个纹理的情况
			osg::Texture* tex = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));    
			if (tex) 
			{
				texture_array.insert(tex);
			}
		}
	}

	void apply(osg::PagedLOD& node)
	{
		std::string path = node.getDatabasePath();
		int n = node.getNumFileNames();
		for (std::size_t i = 1; i < n; i++)
		{
			std::string file_name = path + node.getFileName(i);
			sub_node_names.push_back(file_name);
		}
		traverse(node);
	}

public:
	std::vector<osg::Geometry*> geometry_array;
	std::set<osg::Texture*> texture_array;
	std::vector<std::string> sub_node_names;
};

struct tileinfo_s
{
	int tri_count_;
	std::string fullpath;
};

struct LodNode
{
	osg::BoundingBox                bbox;
	std::string                     file_name;
	std::vector<LodNode>            sub_nodes;
};

/**
*	@class TModelIO
*	@brief 模型文件读写
*   @attention
*   @details
*/

class  TModelIO
{

public:
	TModelIO();
	~TModelIO();

	static osg::Node* ReadModelFile(const std::string &path, osgDB::Options *options = 0);

	static osg::Node* ReadModelFiles(std::vector<std::string>& fileList, osgDB::Options* options = 0);
	
	static bool WriteModelFile(osg::Node *node, const std::string& path, osgDB::Options *options = 0);

	static ShpPolyLine  ReadShpFile(const std::string &path, double offsetx = 0.0, double offsety = 0.0);

	static LodNode GetLodNodeInfo(std::string fpath);
	static int GetNumTri(std::vector<std::string> files);
	static bool ExportStatiInfo(std::vector<std::string> result, std::string fileout);
	//输出统计信息，第一个参数是每个tile块的面片数量
	static bool ExportStatiInfo(std::map<std::string, tileinfo_s> tileinfos, std::string fileout);
protected:

private:
};



#endif   //!ES_MODEL_IO_H
