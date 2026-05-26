// Copyright Airlook, Inc. All Rights Reserved.
#pragma once

#include "Unitl.h"

class SurveyPointsNode : public CustomNode
{
public:
    SurveyPointsNode() :m_iType(0x00), CustomNode() {};

    SurveyPointsNode(const int& id, const std::string& name, const osg::Vec3& location, const int type)
        :m_vecLocation(location), m_iType(type), CustomNode(id, name, Element_Type::ELEMENT_SURVEY_POINTS)
    {
       // Init();
    };

    virtual void Init();
    virtual void Reset();
    virtual void Hover();
    virtual void Picked(const SELECT_TYPE& type = SELECT_TYPE::SELECT_ONE) {};

private:


private:


    osg::Vec3 m_vecLocation;
    std::string m_strImageName;
    int m_iType;
};

