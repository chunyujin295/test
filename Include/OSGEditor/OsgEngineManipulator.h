#pragma once
#include "OSGEditor/Base.h"

class OsgEngineManipulator : public osgGA::OrbitManipulator
{
public:
    OsgEngineManipulator();
    ~OsgEngineManipulator() {};

    virtual void home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
};

