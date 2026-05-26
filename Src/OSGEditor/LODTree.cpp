#include "OSGEditor/LODTree.h"
#include "Core/String.h"
#include "Core/File.h"
#include <filesystem>
#include "OSGEditor/TModelIO.h"


LODTree::LODTree(std::string name)
{
	
	level_ = 0;
	maxLevel_ = 0;
	parent_ = NULL;
	children_.clear();
	id_ = 0;
	name_ =  name;
}


LODTree::~LODTree()
{
	if(parent_!=NULL)
	{
		delete parent_;
		parent_ = NULL;
	}
	for (std::vector<LODTree*>::iterator child = children_.begin(); child!=children_.end(); child++)
	{
		if (*child != NULL)
		{
			delete *child;
		}
	}

}

void LODTree::CalcTriNum()
{
	std::vector<std::string> files(1,name_);
	tricount_ = TModelIO::GetNumTri(files);
}


int LODTree::GetNumTri()
{
	return tricount_;
}


int LODTree::GetNumSameLevelTri(int level)
{
	return 0;
}
int LODTree::GetNumChildNodeTri()
{
	return 0;
}

LODTree* LODTree::GetNodeByName(std::string name,int depth)
{
	if (depth < 0)
		depth = 0;
	depth++;
	if (depth > level_ + maxLevel_)
	{
		return nullptr;
	}
	
	name = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(name));
	const std::filesystem::path p = AI3D::CORE::File::BoostPathFromUtf8(name_);
	std::string treename = AI3D::CORE::File::BoostPathToUtf8String(p.filename());
	if (name == treename)
	{
		return this;
	}
	else
	{
		for (std::vector<LODTree*>::iterator child = children_.begin(); child != children_.end(); child++)
		{
			if (*child != NULL)
			{
				LODTree* newtree = *child;
				const std::filesystem::path pt = AI3D::CORE::File::BoostPathFromUtf8(newtree->GetName());
				std::string ptname = AI3D::CORE::File::BoostPathToUtf8String(pt.filename());
				if (ptname == name)
				{
					return newtree;
				}
				else 
				{
					newtree->GetNodeByName(name,depth);
				}

			}
		}

	}
	
}

void LODTree::SetLevel(int level)
{
	level_ = level;
}
int  LODTree::GetLevel()
{
	return level_;
}

void LODTree::SetParent(LODTree *parent)
{
	parent_ = parent;
}

LODTree* LODTree::GetParent()
{
	return parent_;
}

bool LODTree::IsRootNode()
{
return level_==0;
}
vector<LODTree*>& LODTree::GetChildren()
{
	return children_;
}

bool LODTree::IsLeafNode()
{
	return maxLevel_==1;
}

 int LODTree::GetNodeId()
{
	return id_;
}

string LODTree::GetName()
{
	return name_;
}

 

 int LODTree::GetMaxLevel()
{
	return maxLevel_;
}

 int& LODTree::GetMaxLevelMutual()
 {
	 return maxLevel_;
 }
 

void LODTree::DelNode()
{
	
	
}

void LODTree::DelNullTreeNode()
{
	

}
void LODTree::SetName(std::string name)
{
	name_ = name;
	/*LODTree*parent = dynamic_cast<LODTree*>(parent_);
	string name = "";
	if(parent)
	{
		list<LODTree*> parentpath;
		parentpath.clear();
		getParentPath(parentpath);

		for (auto iter = parentpath.begin();
			iter != parentpath.end();
			iter++)
		{
			LODTree*temp_node = dynamic_cast<LODTree*>(*iter);
			if (temp_node)
			{
				unsigned int temp_id = temp_node->getNodeId();
				char str_id[10];
				sprintf(str_id, "%d", temp_id);
				name = name + string(str_id);
			}

		}
	}
	char str_level[10];
	sprintf(str_level, "%d", level_);
	char str_id[10];
	sprintf(str_id, "%d", id_);
	_name = string("L") + string(str_level) + string("_") + name + string(str_id);*/
}
void LODTree::GetLevelNodes( int index, vector<LODTree*> &result)
{
	if (index >= 0 && index < level_+maxLevel_)
	{
		if (index == level_)
			result.push_back(this);
		else
		{
			for (std::vector<LODTree*>::iterator child = children_.begin(); child != children_.end(); child++)
			{
				if (*child != NULL)
				{
					LODTree* newtree = *child;
					if (newtree->GetLevel() == index)
					{
						result.push_back(newtree);
					}
					else if (newtree->GetLevel() < index)
					{
						newtree->GetLevelNodes(index, result);
					}

				}
			}
		}
	}
	else
	{
		return;
	}
	

}





