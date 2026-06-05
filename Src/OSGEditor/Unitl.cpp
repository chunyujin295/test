#include "OSGEditor/Unitl.h"
//#include "CameraNode.h"
#include "osg/LightModel"
#include <osgUtil/Tessellator>
Unitl::Unitl()
{

}
osg::BoundingBox* Unitl::GetBox(osg::ref_ptr<Vec3Array> vecArray)
{
    Vec3 minValue(FLT_MAX, FLT_MAX, FLT_MAX), maxValue(FLT_MIN, FLT_MIN, FLT_MIN);

    for (size_t i = 0; i < vecArray->size(); i++)
    {
        minValue.x() = std::fmin(minValue.x(), vecArray->at(i).x());
        minValue.y() = std::fmin(minValue.y(), vecArray->at(i).y());
        minValue.z() = std::fmin(minValue.z(), vecArray->at(i).z());

        maxValue.x() = std::fmax(maxValue.x(), vecArray->at(i).x());
        maxValue.y() = std::fmax(maxValue.y(), vecArray->at(i).y());
        maxValue.z() = std::fmax(maxValue.z(), vecArray->at(i).z());

    }

    return new osg::BoundingBox(minValue, maxValue);
}
osg::ref_ptr<osg::Geode> Unitl::CreatePlane(osg::ref_ptr<osg::Vec3Array> vertex, const Vec4& color, const std::string& name)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    geode->addDrawable(geometry);
    geometry->setVertexArray(vertex.get());

    geometry->setName(name);
    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(color);
    geometry->setColorArray(vc);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::PrimitiveSet> primitiveSet = new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertex->size());
    geometry->addPrimitiveSet(primitiveSet);

    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    return geode.get();
}

osg::ref_ptr<osg::Geode> Unitl::CreateLineGeometry(osg::Vec3Array* array, osg::Vec4 color)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    geode->addDrawable(geometry);
    geometry->setVertexArray(array);

    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    //for (size_t i = 0; i < array->size(); i++)
    {
        vc->push_back(color);
    }

    geometry->setColorArray(vc);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth();
    lineWidth->setWidth(1.0f);

    osg::ref_ptr<osg::StateSet> stateset = geometry->getOrCreateStateSet();
    stateset->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);

    osg::ref_ptr<osg::PrimitiveSet> primitiveSet = new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, array->size());
    geometry->addPrimitiveSet(primitiveSet);

    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    //stateset->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);



    return geode.get();
}

osg::ref_ptr<osg::Group> Unitl::CreateBox(const osg::BoundingBox* pBox, const Vec4& color)
{
    osg::ref_ptr<osg::Group> pNode = new osg::Group;
    //上

    osg::ref_ptr<osg::Vec3Array> vec1 = new osg::Vec3Array;
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));

    pNode->addChild(CreatePlane(vec1.get(), color, "Z"));

    //下
    osg::ref_ptr<osg::Vec3Array> vec2 = new osg::Vec3Array;
    vec2->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec2->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec2->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));
    vec2->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));
    pNode->addChild(CreatePlane(vec2.get(), color, "-Z"));

    //左
    osg::ref_ptr<osg::Vec3Array> vec3 = new osg::Vec3Array;
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));
    pNode->addChild(CreatePlane(vec3.get(), color, "-X"));
    //右
    osg::ref_ptr<osg::Vec3Array> vec4 = new osg::Vec3Array;
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));
    pNode->addChild(CreatePlane(vec4.get(), color, "X"));
    //前
    osg::ref_ptr<osg::Vec3Array> vec5 = new osg::Vec3Array;
    vec5->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec5->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec5->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec5->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));
    pNode->addChild(CreatePlane(vec5.get(), color, "-Y"));
    //后
    osg::ref_ptr<osg::Vec3Array> vec6 = new osg::Vec3Array;
    vec6->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));
    vec6->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));
    vec6->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec6->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));

    pNode->addChild(CreatePlane(vec6.get(), color, "Y"));

    osg::ref_ptr<osg::StateSet> stateSet = pNode->getOrCreateStateSet();

    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    return pNode.get();
}

const std::string BoxBorderShader =
"uniform vec4 color1;\n"
"void main(void){\n"
//"   vec4 C(0.290196091, 0.913725495, 1.00000000, 1.00000000)\n;"
"   gl_FragColor = color1;\n"
"}\n";

const std::string BoxBorderTileShader =
"uniform vec4 color1;\n"
"void main(void){\n"
//"   vec4 C(0.290196091, 0.913725495, 1.00000000, 1.00000000)\n;"
"   gl_FragColor = color1;\n"
"}\n";


osg::ref_ptr<osg::Node> Unitl::CreateBoxBorder(const osg::BoundingBox* pBox, const Vec4& color)
{
    osg::ref_ptr<osg::Vec3Array> vec1 = new osg::Vec3Array;
    //上
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));
    //下
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));


    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    // 底面
    indices->push_back(0); indices->push_back(1); // AB
    indices->push_back(1); indices->push_back(2); // BC
    indices->push_back(2); indices->push_back(3); // CD
    indices->push_back(3); indices->push_back(0); // DA
    // 顶面
    indices->push_back(4); indices->push_back(5); // EF
    indices->push_back(5); indices->push_back(6); // FG
    indices->push_back(6); indices->push_back(7); // GH
    indices->push_back(7); indices->push_back(4); // HE
    // 垂直边
    indices->push_back(0); indices->push_back(4); // AE
    indices->push_back(1); indices->push_back(5); // BF
    indices->push_back(2); indices->push_back(6); // CG
    indices->push_back(3); indices->push_back(7); // DH


    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray(vec1.get());
    geometry->addPrimitiveSet(indices.get());

    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(color);
    geometry->setColorArray(vc);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::StateSet* stateSet = geometry->getOrCreateStateSet();
    osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth(1.0);
    stateSet->setAttributeAndModes(lineWidth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    //stateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateSet->setRenderBinDetails(100, "RenderBin");

    //osg::ref_ptr<osg::Program> program = new osg::Program;
    //program->addShader(new osg::Shader(osg::Shader::FRAGMENT, BoxBorderShader));
    //stateSet->addUniform(new osg::Uniform("color1", color), osg::StateAttribute::ON);
    //stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
    //std::cout << "box color: " << color.r() << ", " << color.g() << "," << color.b() << "," << color.a() << std::endl;;

    // 
    //osg::Material* m = new osg::Material;
    //m->setColorMode(Material::AMBIENT_AND_DIFFUSE);
    //m->setAmbient(osg::Material::FRONT_AND_BACK, color);
    //m->setDiffuse(osg::Material::FRONT_AND_BACK, color);
    //stateSet->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geometry.get());

    return geode.get();

}

osg::ref_ptr<osg::Group> Unitl::CreateBoxTile(const osg::BoundingBox* pBox, const Vec4& color)
{
    osg::ref_ptr<osg::Group> pNode = new osg::Group;

    //上

    osg::ref_ptr<osg::Vec3Array> vec1 = new osg::Vec3Array;
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));


    pNode->addChild(CreatePlane(vec1.get(), color, "Z"));

    //下
    osg::ref_ptr<osg::Vec3Array> vec2 = new osg::Vec3Array;
    vec2->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec2->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec2->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));
    vec2->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));

    pNode->addChild(CreatePlane(vec2.get(), color, "-Z"));

    //左
    osg::ref_ptr<osg::Vec3Array> vec3 = new osg::Vec3Array;
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));
    vec3->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));

    pNode->addChild(CreatePlane(vec3.get(), color, "-X"));
    //右
    osg::ref_ptr<osg::Vec3Array> vec4 = new osg::Vec3Array;
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec4->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));

    pNode->addChild(CreatePlane(vec4.get(), color, "X"));
    //前
    osg::ref_ptr<osg::Vec3Array> vec5 = new osg::Vec3Array;
    vec5->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec5->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec5->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec5->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));

    pNode->addChild(CreatePlane(vec5.get(), color, "-Y"));
    //后
    osg::ref_ptr<osg::Vec3Array> vec6 = new osg::Vec3Array;
    vec6->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));
    vec6->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));
    vec6->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec6->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));

    pNode->addChild(CreatePlane(vec6.get(), color, "Y"));

    osg::ref_ptr<osg::StateSet> stateSet = pNode->getOrCreateStateSet();

    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    //stateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateSet->setRenderBinDetails(-100, "RenderBin");

    return pNode.get();
}

osg::ref_ptr<osg::Node> Unitl::CreateBoxBorderTile(const osg::BoundingBox* pBox, const Vec4& color)
{
    osg::ref_ptr<osg::Vec3Array> vec1 = new osg::Vec3Array;
    //上
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMax()));
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMax()));
    //下
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMin(), pBox->zMin()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMin(), pBox->zMin()));
    vec1->push_back(osg::Vec3(pBox->xMax(), pBox->yMax(), pBox->zMin()));
    vec1->push_back(osg::Vec3(pBox->xMin(), pBox->yMax(), pBox->zMin()));


    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    // 底面
    indices->push_back(0); indices->push_back(1); // AB
    indices->push_back(1); indices->push_back(2); // BC
    indices->push_back(2); indices->push_back(3); // CD
    indices->push_back(3); indices->push_back(0); // DA
    // 顶面
    indices->push_back(4); indices->push_back(5); // EF
    indices->push_back(5); indices->push_back(6); // FG
    indices->push_back(6); indices->push_back(7); // GH
    indices->push_back(7); indices->push_back(4); // HE
    // 垂直边
    indices->push_back(0); indices->push_back(4); // AE
    indices->push_back(1); indices->push_back(5); // BF
    indices->push_back(2); indices->push_back(6); // CG
    indices->push_back(3); indices->push_back(7); // DH


    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray(vec1.get());
    geometry->addPrimitiveSet(indices.get());

    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(color);
    geometry->setColorArray(vc);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::StateSet* stateSet = geometry->getOrCreateStateSet();

    osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth(1.0);
    stateSet->setAttributeAndModes(lineWidth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    //stateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    //stateSet->setRenderBinDetails(-100, "RenderBin");

    //osg::ref_ptr<osg::Program> program = new osg::Program;
    //program->addShader(new osg::Shader(osg::Shader::FRAGMENT, BoxBorderTileShader));
    //stateSet->addUniform(new osg::Uniform("color1", color), osg::StateAttribute::ON);
    ////std::cout << "box color: " << color.r() << ", " << color.g() << "," << color.b() << "," << color.a() << std::endl;;
    //stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);


    //osg::Material* m = new osg::Material;
    //m->setColorMode(Material::AMBIENT_AND_DIFFUSE);
    //m->setAmbient(osg::Material::FRONT_AND_BACK, color);
    //m->setDiffuse(osg::Material::FRONT_AND_BACK, color);
    //stateSet->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);


    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geometry.get());

    return geode.get();

}

osg::ref_ptr<osg::Geode> Unitl::CreatePolygon(osg::ref_ptr<osg::Vec3Array> vertex, const Vec4& color, const std::string& name)
{
    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setName(name);
    osg::ref_ptr<osg::DrawElementsUInt> drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);

    for (int i = 0; i < vertex->size(); i++)
    {
        drawIndexs->push_back(i);
    }

    pGeometry->setVertexArray(vertex);

    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(color);
    pGeometry->setColorArray(vc);
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    pGeometry->addPrimitiveSet(drawIndexs);

    pGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    //解决凹凸多边形问题
    osg::ref_ptr<osgUtil::Tessellator> tscx = new osgUtil::Tessellator;
    tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
    tscx->retessellatePolygons(*(pGeometry.get()));
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(pGeometry);

    return pGeode.get();
}

osg::ref_ptr<osg::Geode> Unitl::CreatePolygonBorder(const PolygonBox& box, const Vec4& color)
{
    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;

    osg::ref_ptr<osg::DrawElementsUInt> drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);
    osg::ref_ptr<osg::Vec3Array> pPoint = new osg::Vec3Array;

    for (int i = 0; i < box.points.size(); i++)
    {
        pPoint->push_back(osg::Vec3(box.points[i].x(), box.points[i].y(), box.maxHeight));
        drawIndexs->push_back(i);
        if (i == box.points.size() - 1)
        {
            drawIndexs->push_back(0);
        }
        else
        {
            drawIndexs->push_back(i + 1);

        }
    }

    for (int i = 0; i < box.points.size(); i++)
    {
        pPoint->push_back(osg::Vec3(box.points[i].x(), box.points[i].y(), box.minHeight));
        drawIndexs->push_back(box.points.size() + i);

        if (i == box.points.size() - 1)
        {
            drawIndexs->push_back(box.points.size());
        }
        else
        {
            drawIndexs->push_back(box.points.size() + i + 1);
        }
    }

    for (int i = 0; i < box.points.size(); i++)
    {
        drawIndexs->push_back(i);
        drawIndexs->push_back(box.points.size() + i);
    }

    pGeometry->setVertexArray(pPoint);

    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(color);
    pGeometry->setColorArray(vc);
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    pGeometry->addPrimitiveSet(drawIndexs);

    osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth(1.0);
    pGeometry->getOrCreateStateSet()->setAttributeAndModes(lineWidth.get(), osg::StateAttribute::ON);

    pGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(pGeometry);
    return pGeode;
}

osg::ref_ptr<osg::Node> Unitl::CreateCentrum(const ST_CAMERA_INFO& stCamera)
{
    return nullptr;
}

const std::string fragmentShader =
"uniform sampler2D baseTexture;\n"
"void main(void){\n"
"   vec2 coord = gl_TexCoord[0].xy;\n"
"   vec4 C = texture2D(baseTexture, coord)\n;"
"   gl_FragColor = C * 2.0;\n"
"}\n";

osg::ref_ptr<osg::Geode> Unitl::CreateImage(const osg::Vec3& corner, const osg::Vec3& width, const osg::Vec3& height, osg::ref_ptr<osg::Image> image)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0] = corner - width - height;
    (*coords)[1] = corner + width - height;
    (*coords)[2] = corner + width + height;
    (*coords)[3] = corner - width + height;

    geometry->setVertexArray(coords);


    //osg::Vec3Array* norms = new osg::Vec3Array(1);
    //(*norms)[0] = width ^ height;
    //(*norms)[0].normalize();
    //geometry->setNormalArray(norms);
    //geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::Vec2Array* tcoords = new osg::Vec2Array(4);
    (*tcoords)[0].set(0.0f, 0.0f);
    (*tcoords)[1].set(1.0f, 0.0f);
    (*tcoords)[2].set(1.0f, 1.0f);
    (*tcoords)[3].set(0.0f, 1.0f);
    geometry->setTexCoordArray(0, tcoords);
    //geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    geode->addChild(geometry);

    osg::StateSet* stateSet = geometry->getOrCreateStateSet();
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setDataVariance(osg::Object::DYNAMIC);
        texture->setImage(image);
        stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
        //自定义着色器，高亮图片
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShader));

        stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
        stateSet->addUniform(new osg::Uniform("baseTexture", 0));
    }

    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    return geode.get();
}

std::string Unitl::GetCurrentDir()
{
    char szFilePath[MAX_PATH] = { 0 };

    GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
    std::string res = osgDB::getFilePath(szFilePath);
    return res;
}

osg::Vec4 Unitl::FromHex(const std::string& HexString)
{
    int StartIndex = (!HexString.empty() && HexString[0] == TCHAR('#')) ? 1 : 0;

    if (HexString.length() == 3 + StartIndex)
    {
        const int R = HexDigit(HexString[StartIndex++]);
        const int G = HexDigit(HexString[StartIndex++]);
        const int B = HexDigit(HexString[StartIndex]);

        return osg::Vec4(((R << 4) + R) / 255.0, ((G << 4) + G) / 255.0, ((B << 4) + B) / 255.0, 1.0);
    }

    if (HexString.length() == 6 + StartIndex)
    {
        osg::Vec4 Result;

        Result.r() = ((HexDigit(HexString[StartIndex + 0]) << 4) + HexDigit(HexString[StartIndex + 1])) / 255.0;
        Result.g() = ((HexDigit(HexString[StartIndex + 2]) << 4) + HexDigit(HexString[StartIndex + 3])) / 255.0;
        Result.b() = ((HexDigit(HexString[StartIndex + 4]) << 4) + HexDigit(HexString[StartIndex + 5])) / 255.0;
        Result.a() = 1;

        return Result;
    }

    if (HexString.length() == 8 + StartIndex)
    {
        osg::Vec4 Result;

        Result.r() = ((HexDigit(HexString[StartIndex + 0]) << 4) + HexDigit(HexString[StartIndex + 1])) / 255.0;
        Result.g() = ((HexDigit(HexString[StartIndex + 2]) << 4) + HexDigit(HexString[StartIndex + 3])) / 255.0;
        Result.b() = ((HexDigit(HexString[StartIndex + 4]) << 4) + HexDigit(HexString[StartIndex + 5])) / 255.0;
        Result.a() = ((HexDigit(HexString[StartIndex + 6]) << 4) + HexDigit(HexString[StartIndex + 7])) / 255.0;

        return Result;
    }

    return osg::Vec4();
}


osg::ref_ptr<osg::Node> Unitl::HoverText(const std::string& name, const osg::Vec3& location)
{
    if (name.empty())
    {
        return nullptr;
    }
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setFont(osgText::readFontFile("simhei.ttf"));
    text->setText(name, osgText::String::ENCODING_UTF8); //解决乱码问题
    text->setCharacterSize(14.0f);
   
    text->setCharacterSize(14.0f);
    text->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

    {


        text->setDrawMode(osgText::Text::TEXT | osgText::Text::FILLEDBOUNDINGBOX);//添加文字边框

        text->setBoundingBoxColor(FromHex("3A3A3A"));
        text->setBoundingBoxMargin(6.0f);
    }
    text->setAlignment(osgText::Text::LEFT_BOTTOM);
    //text->setAxisAlignment(osgText::Text::SCREEN);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(text);

    osg::AutoTransform* pAt = new osg::AutoTransform();
    pAt->setAutoScaleToScreen(true);
    pAt->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    //pAt->setMinimumScale(1.0);
    //pAt->setMaximumScale(2.0);
    pAt->addChild(geode);
    pAt->setPosition(location);

    pAt->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    pAt->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    pAt->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    //pParentNode->getOrCreateStateSet()->setRenderBinDetails(-1, "RenderBin");

    return pAt;
}

bool Unitl::IsPointInPolygon(const osg::Vec3& point, const std::vector<osg::Vec3>& polygon) {
    int crossings = 0;
    size_t numPoints = polygon.size();

    for (size_t i = 0; i < numPoints; ++i) {
        const osg::Vec3& p1 = polygon[i];
        const osg::Vec3& p2 = polygon[(i + 1) % numPoints];

        // 检查线段与水平射线的相交情况
        if (((p1.y() > point.y()) != (p2.y() > point.y())) &&
            (point.x() < (p2.x() - p1.x()) * (point.y() - p1.y()) / (p2.y() - p1.y()) + p1.x())) {
            ++crossings;
        }
    }

    // 如果交叉次数为奇数，则点在多边形内部
    return (crossings % 2 == 1);
}

bool Unitl::IsPointInsideBoundingBox(const osg::Vec3& point, const osg::Vec3& minCorner, const osg::Vec3& maxCorner)
{
    if (point.x() >= minCorner.x() && point.x() <= maxCorner.x()
        && point.y() >= minCorner.y() && point.y() <= maxCorner.y())
    {
        return true;
    }

    return false;
}


void Unitl::AddAxis(Geode* coord, Vec3 pt, std::string text, Vec4 color)
{
    Vec3Array* v = new Vec3Array();
    v->push_back(Vec3(0, 0, 0));
    v->push_back(pt);

    Vec4Array* c = new Vec4Array();
    c->push_back(color);

    Geometry* axis = new Geometry;
    axis->setVertexArray(v);
    axis->setColorArray(c);
    axis->setColorBinding(Geometry::BIND_OVERALL);
    axis->addPrimitiveSet(new DrawArrays(PrimitiveSet::LINES, 0, 2));
  

    osg::ref_ptr<osg::Material> material = new osg::Material();
    material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
   
    coord->addDrawable(axis);
    coord->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

}
osg::ref_ptr<osg::AutoTransform> Unitl::AddAxisText(Vec3 pt, std::string text, float textlenth )
{
    osgText::Text* tx = new osgText::Text;
    tx->setText(text, osgText::String::ENCODING_UTF8);
    tx->setFont("Fonts/simhei.ttf");
    tx->setAlignment(osgText::Text::CENTER_CENTER);
    tx->setCharacterSize(textlenth);

    //osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    //geode->addDrawable(tx);

    osg::AutoTransform* pAt = new osg::AutoTransform();
    pAt->setAutoScaleToScreen(true);
    pAt->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    //pAt->setMinimumScale(1.0);
    //pAt->setMaximumScale(2.0);

    pAt->addChild(tx);
    pAt->setPosition(pt);

    pAt->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    pAt->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    return pAt;

}
