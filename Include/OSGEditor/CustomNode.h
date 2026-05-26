#pragma once

#include "Base.h"
#include "ThreadManager.h"

class ThreadManager;

class DLL_API CustomNode : public osg::MatrixTransform
{
public:
    CustomNode(const bool& bThread = true);

    CustomNode(const int& id, const std::string& name, const Element_Type& type);

    ~CustomNode();

    virtual void Init() {};

    //恢复默认颜色
    virtual void Reset() {};
    //修改hover颜色     
    virtual void Hover() {};

    //修改选中颜色
    virtual void Picked(const SELECT_TYPE& type = SELECT_TYPE::SELECT_ONE) {};

    virtual void Drag(const Vec3& dragPoint) {};

    const Element_Type& GetElementType() { return m_iElementType; }

    void AddChild(const int& id, ref_ptr<CustomNode> node);

    ref_ptr<CustomNode> GetChild(const int& id);

    std::map<int, CustomNode*>* GetAllChild() { return &m_pTotalNode; };

    void RemoveChild(const int& id);

    void RemoveChild(ref_ptr<CustomNode> node);

    void RemoveAll();

    void Visible(bool value);

    void Scale(float value);

public:
    bool m_bThread;
    int m_iID;
    std::string m_strName;
    Element_Type m_iElementType;
    MOUSE_TYPE m_eMouseType;
    osg::ref_ptr<osg::Geometry> m_pSelectGeometry;  //拾取到的几何体
    osg::Vec3 m_PickedPoint;                    //拾取到的位置

    std::map<int, CustomNode*> m_pTotalNode;   //保存所有添加节点
    osg::ref_ptr<osg::Switch> m_pRootSwitch;   //显隐节点

    ThreadManager* m_pThreadPool;  //线程池
};