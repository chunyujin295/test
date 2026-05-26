#include "OSGEditor/SurveyPointsNode.h"
//SurveyPointsNode::SurveyPointsNode(const std::string& name, const osg::Vec3& location, const int type)
//{
//}
void SurveyPointsNode::Init()
{
   /* std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " pointer:"
        << std::hex << std::showbase << this << std::dec << std::endl;*/

    std::string strResourceDir = Unitl::GetCurrentDir();
    std::string image;
    switch (m_iType)
    {
    case 1:
        image = Unitl::GetCurrentDir() + "/gcpred.png";//red
        break;
    case 2:
        image = Unitl::GetCurrentDir() + "/gcpgreen.png"; //green
        break;
    default:
        break;
    }

   // std::cout << "id======== "<<m_iID << " name========== " << m_strName<< "========= "<< image << std::endl;

   static  osg::ref_ptr<osg::Image> pImage = osgDB::readRefImageFile(image);
    if (pImage == nullptr || !pImage->valid())
    {
        return;
    }

    static osg::ref_ptr<osg::Geode> pNode = Unitl::CreateImage(osg::Vec3(0.f, 0.0f, 0.f), osg::Vec3(pImage->s() / 2.0, 0.0f, 0.0f), osg::Vec3(0.0f, pImage->t() / 2.0, 0.f), pImage);
    
    //pNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    //pNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    //pNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osg::AutoTransform* pAt = new osg::AutoTransform();
    pAt->setAutoScaleToScreen(true);
    pAt->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    //pAt->setMinimumScale(1.0);
    //pAt->setMaximumScale(2.0);
    pAt->addChild(pNode);

    osg::PositionAttitudeTransform* pParentNode = new osg::PositionAttitudeTransform();
    pParentNode->setPosition(m_vecLocation);
    pParentNode->addChild(pAt);
    pParentNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    addChild(pParentNode);
}

void SurveyPointsNode::Reset()
{
    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
   
}
void SurveyPointsNode::Hover()
{
    if (m_eMouseType != MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_HOVER;



}