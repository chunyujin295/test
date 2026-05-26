#ifndef _LOD_TREE_H
#define _LOD_TREE_H
#include "OSGEditor/Base.h"
#include <vector>
#include <list>
#include <map>
#include <iostream>

using namespace std;




class DLL_API LODTree
{
public:
	LODTree(std::string name);
	~LODTree();

	 void DelNullTreeNode() ;
	 void DelNode() ;
	 //获取某一层的所有节点
	 void GetLevelNodes(int index, vector<LODTree*> & result) ;
	 //name为非全路径
	 LODTree* GetNodeByName(std::string name, int depth=0);
	 void SetName(std::string name) ;
	 vector<LODTree*>& GetChildren();
	void SetParent(LODTree *parent);
	LODTree* GetParent();
	bool IsRootNode();
	bool IsLeafNode();
	/*bool HasLeafNode();*/
	 int GetNodeId();
	
	 int GetMaxLevel();
	
	 std::string GetName();
	 int& GetMaxLevelMutual();
	void SetLevel(int level);
	int GetLevel();
	
	
	void CalcTriNum();
	int GetNumTri();
	int GetNumSameLevelTri(int level);//获取指定层的面片数
	int GetNumChildNodeTri();//获取某个节点的所有孩子的面片数

	
protected:
	
	 int                                   level_=0;                     /**<树节点所在层号，层号从零开始编号 */
	 int                                   maxLevel_=0;                  /**<树的最大层数,从1开始 */
	 int                                   id_ = 0;                        /**<树节点ID（标示该节点属于哪一个分支） */
	std::string                                       name_ = "";          /**<树节点名称（树节点唯一标示） */
	

	LODTree*                                     parent_ ;                     /**<本节点的父节点 */
	vector<LODTree*> 								children_;
	int tricount_;//面片数
};

#endif _LOD_TREE_H    //!_LOD_TREE_H

