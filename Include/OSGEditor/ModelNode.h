#pragma once
#include "OsgEngine.h"
#include "Unitl.h"
class OsgEngine;

class ModelNode : public CustomNode
{
public:
    ModelNode(const int& id, const std::string& name, std::string& filePath, OsgEngine *pOsgEngine);
    ModelNode():CustomNode() {};
    ~ModelNode() {};

    virtual void Init();
    virtual void Reset();
    virtual void Picked(const SELECT_TYPE& type = SELECT_TYPE::SELECT_ONE);

private:
    osg::ref_ptr<osg::Node> m_pModelNode;
    std::string m_strModelPath;
    OsgEngine* m_pOsgEngine;
};

