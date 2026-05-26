#pragma once
#include "Unitl.h"


class TileNode : public CustomNode
{
public:
    TileNode(const ST_BOUNDINGBOX& box);
    TileNode() : CustomNode() {};
    ~TileNode();

    virtual void Init();

    virtual void Reset();
    virtual void Hover();
    virtual void Picked(const SELECT_TYPE& type = SELECT_TYPE::SELECT_ONE);

private:
    void UpdateBorder(const osg::Vec4& color);
    void UpdatePlane(const osg::Vec4& color);


private:
    ST_BOUNDINGBOX m_stBoxInfo;
    osg::ref_ptr<osg::Box> m_pBox;
    Unitl g_tileUnitl;

    //卡片模型
    osg::ref_ptr<osg::Node> m_pTileBorderGeometry;
    //视椎体面模型

    osg::ref_ptr<osg::Group> m_pBoxGroup;


};