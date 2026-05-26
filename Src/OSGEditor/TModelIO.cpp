#include <osg/ComputeBoundsVisitor>
#include "Core/File.h"
#include "OSGEditor/TModelIO.h"
#include "OSGEditor/TMesh.h"
TModelIO::TModelIO()
{
	//
}


TModelIO::~TModelIO()
{
	//
}
bool TModelIO::ExportStatiInfo(std::vector<std::string> result, std::string fileout)
{
	std::map<std::string, std::vector<std::string>> meshfilemap1;
	for (int i = 0; i < result.size(); i++)
	{
		std::string file = result[i];
		std::string tilename = AI3D::CORE::File::GetParentDir(file);
		meshfilemap1[tilename].push_back(file);
	}
	std::map<std::string, tileinfo_s> tileinfos1;
	for (auto iter : meshfilemap1)
	{
		tileinfos1[iter.first].fullpath = iter.first;
		tileinfos1[iter.first].tri_count_ = TModelIO::GetNumTri(iter.second);

	}
	return TModelIO::ExportStatiInfo(tileinfos1, fileout);
}
bool TModelIO::ExportStatiInfo(std::map<std::string, tileinfo_s> tileinfos, std::string fileout)
{
	std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(fileout, std::ios::out);
	ofs << "name " << " tri_count " << "\n";
	for (auto iter : tileinfos)
	{
		ofs << iter.first << " " << iter.second.tri_count_ << "\n";
	}
	ofs.close();
	return true;
}
int TModelIO::GetNumTri(std::vector<std::string> files)
{

	osg::ref_ptr<osg::Node> node = ReadModelFiles(files);// osgDB::readNodeFiles(files);
	CollectGeom collect_geom;
	node->accept(collect_geom);
	GeomArr geom_arr = collect_geom.GetGeomArr();

	unsigned int num_geom = geom_arr.size();
	std::vector<float> edge_lengths;
	std::vector<float> edge_pixel_lengths;
	std::cout << num_geom << std::endl;
	int num = 0;
	for (unsigned int i = 0; i < num_geom; i++)
	{


		TMesh* pMesh = new TMesh(true);
		pMesh->SetGeometry(geom_arr[i].get());
		num += pMesh->triangleset_.size();


		if (pMesh)
		{
			delete pMesh;
			pMesh = NULL;
		}

	}

	return num;
}

osg::Node* TModelIO::ReadModelFile(const std::string &path, osgDB::Options *options/* =0 */)
{
	if (path.empty())
	{
		return NULL;
	}
	

	std::string suffix = osgDB::getFileExtension(path);
	if (std::string("obj") == osgDB::convertToLowerCase(suffix))         /**<对OBJ文件进行操作 */
	{
		osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
	
		options->setOptionString("noTriStripPolygons");
		options->setOptionString("noRotation");
		osg::ref_ptr<osg::Node> objmodel = osgDB::readNodeFile(path, options);
		if (!objmodel.valid()) return NULL;
		TModifyObjVertex modifyVertex;
		objmodel->accept(modifyVertex);
		return objmodel.release();
	}
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(path, options);
	return node.release();
}

osg::Node* TModelIO::ReadModelFiles(std::vector<std::string>& fileList, osgDB::Options* options/* = 0*/)
{
	typedef std::vector<osg::Node*> NodeList;
	NodeList nodelist;

	for (auto iter = fileList.begin(); iter != fileList.end(); ++iter)
	{
		osg::Node* node = TModelIO::ReadModelFile(*iter, options);
		if (node != NULL)
		{
			if (node->getName().empty())  node->setName(*iter);
			nodelist.push_back(node);
		}

	}

	if (nodelist.empty()) return NULL;

	if (nodelist.size() == 1)
	{
		return nodelist.front();
	}
	else
	{
		osg::Group* group = new osg::Group();
		for (NodeList::iterator iter = nodelist.begin();
		iter != nodelist.end();
			++iter)
		{
			group->addChild(*iter);

		}

		return group;
	}

}


bool TModelIO::WriteModelFile(osg::Node *node, const std::string &path, osgDB::Options *options/* =0 */)
{
	std::vector<std::string> outputtexpath;
	if (node)
	{
		ModifyTextureName modifytexname;
		modifytexname.setTextureImageBasePath(path.c_str());
		node->accept(modifytexname);
		outputtexpath = modifytexname.getOutputTexPath();
	}
	else
	{
		return false;
	}

	osg::ref_ptr<osgDB::Options> esoptions = new osgDB::Options();

	osgDB::Registry::instance()->setDataFilePathList(osgDB::getFilePath(path));
	std::string strop;
	if (options)
	{
		strop = options->getOptionString();
	}
	
	if (strop.empty())
	{
		strop = std::string("WriteImageHint=IncludeFile Compressor=zlib");
	}
	else
	{
		strop = std::string("WriteImageHint=IncludeFile Compressor=zlib") + std::string(" ") + strop;
	}
	esoptions->setOptionString(strop);

	bool is_sucess = osgDB::writeNodeFile(*node, path, esoptions);

	std::string file_suffix = osgDB::getLowerCaseFileExtension(path);
	if (file_suffix == std::string("osgb"))
	{
		for (int i = 0; i < outputtexpath.size(); i++)
		{
			std::remove(outputtexpath[i].c_str());
		}
		
	}

	return is_sucess;
}


ShpPolyLine TModelIO::ReadShpFile(const std::string &path, double offsetx/* = 0.0*/, double offsety/* = 0.0*/)
{
	ShpPolyLine result;
	osg::ref_ptr<osg::Node> shp_node = osgDB::readNodeFile(path);
	if (!shp_node.valid()) return result;
	CollectGeom collect_geom;

	shp_node->accept(collect_geom);

	GeomArr  geom_arr = collect_geom.GetGeomArr();

	for (std::size_t i = 0; i < geom_arr.size(); i++)
	{
		osg::ref_ptr<osg::Vec3Array> geom_vertex = dynamic_cast<osg::Vec3Array*>(geom_arr[i]->getVertexArray());
		if (!geom_vertex.valid()) continue;

		std::vector<osg::Vec3>  temp_poly;
		for (std::size_t j = 0; j < geom_vertex->size(); j++)
		{
			osg::Vec3 temp_vertex = geom_vertex->at(j);

			temp_poly.push_back(osg::Vec3(temp_vertex[0] - offsetx, temp_vertex[1] - offsety, temp_vertex[2]));
		}

		result.push_back(temp_poly);
	}

	return result;

}

LodNode TModelIO::GetLodNodeInfo(std::string fpath)
{
	LodNode root_tile;
	osg::ref_ptr<osg::Node> root = osgDB::readNodeFile(fpath);
	if (!root.valid()) 
	{
		return root_tile;
	}
	root_tile.file_name = fpath;

	osg::ComputeBoundsVisitor bbox_visitor;
	root->accept(bbox_visitor);
	root_tile.bbox = bbox_visitor.getBoundingBox();

	InfoVisitor infoVisitor;
	root->accept(infoVisitor);

	for (int i = 0; i < infoVisitor.sub_node_names.size(); i++)
	{
		LodNode tree_node = GetLodNodeInfo(infoVisitor.sub_node_names[i]);
		if (!tree_node.file_name.empty())
		{
			root_tile.sub_nodes.push_back(tree_node);
		}
	}
	return root_tile;
}