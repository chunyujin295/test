#ifndef _AI3D_CORE_KML_H_
#define _AI3D_CORE_KML_H_
#include <Eigen/Core>
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>


static std::vector < std::vector<Eigen::Vector3d>> ReadKML(const std::string filename)
{
	GDALAllRegister();
	std::vector < std::vector<Eigen::Vector3d>> ret;
	GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
	if (poDS == nullptr)
	{
		return ret;
	}

	GDALDataset::Layers layers = poDS->GetLayers();
	if (layers.size() > 1)
	{
		return ret;
	}

	OGRLayer* poLayer = layers[0];
	OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
	poLayer->ResetReading();
	OGRFeature* poFeature;
	while ((poFeature = poLayer->GetNextFeature()) != nullptr)
	{
		OGRGeometry* poGeometry = poFeature->GetGeometryRef();
		OGRwkbGeometryType type = poGeometry->getGeometryType();
		if (poGeometry != nullptr && wkbFlatten(type) == wkbPolygon)
		{
			OGRPolygon* polygon = static_cast<OGRPolygon*>(poGeometry);
			OGRLinearRing* ring1 = (OGRLinearRing*)poGeometry;
			std::vector<Eigen::Vector3d> vectemp1;
			for (int i = 0; i < ring1->getNumPoints(); ++i)
			{
				OGRPoint point;
				ring1->getPoint(i, &point);

				vectemp1.emplace_back(point.getX(), point.getY(), point.getZ());
			}
			OGRLinearRing* ring = polygon->getExteriorRing();
			std::vector<Eigen::Vector3d> vectemp;
			
			for (int i = 0; i < ring->getNumPoints(); ++i)
			{
				std::cout << ring->getX(i) << " " << ring->getY(i) << " " << ring->getZ(i) << std::endl;
				vectemp.emplace_back(ring->getX(i), ring->getY(i), ring->getZ(i));
			}
			ret.emplace_back(vectemp);
		}
		else if (poGeometry != nullptr && wkbFlatten(type) == wkbMultiPolygon)
		{
			OGRMultiPolygon* polygons = static_cast<OGRMultiPolygon*>(poGeometry);
			const int polygon_count = polygons->getNumGeometries();
			
			
			for (int cnt = 0; cnt < polygon_count; cnt++)
			{

				OGRLinearRing* ring1 = (OGRLinearRing*)polygons->getGeometryRef(cnt);
				std::vector<Eigen::Vector3d> vectemp1;
				for (int i = 0; i < ring1->getNumPoints(); ++i)
				{
					OGRPoint point;
					ring1->getPoint(i, &point);

					vectemp1.emplace_back(point.getX(), point.getY(), point.getZ());
				}
				std::vector<Eigen::Vector3d> vectemp;
				OGRPolygon* polygon = static_cast<OGRPolygon*>(polygons->getGeometryRef(cnt));
				OGRLinearRing* ring = polygon->getExteriorRing();

				
				for (int i = 0; i < ring->getNumPoints(); ++i)
				{
					std::cout << ring->getX(i)<<" " << ring->getY(i)<<" " << ring->getZ(i) << std::endl;
					vectemp.emplace_back(ring->getX(i), ring->getY(i), ring->getZ(i));
				}
				ret.emplace_back(vectemp);
			}
			

		}
		OGRFeature::DestroyFeature(poFeature);
	}
	GDALClose(poDS);
	return ret;
}

static  inline OGRGeometry* ToPolygon(const std::vector<Eigen::Vector3d>& points)
{
	OGRLinearRing* ring = (OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
	OGRPolygon* polygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);

	
	for (int i = 0; i < points.size(); ++i)
	{
		ring->addPoint(points[i][0], points[i][1]);
	}
	ring->closeRings();
	polygon->addRing(ring);
	return polygon;
}



static  inline OGRLinearRing* ToPolygon(const ABBox3d& box)
{

	OGRLinearRing* ring = (OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
	
	std::vector<Eigen::Vector2d> points;

	Eigen::Vector2d cornor1 = Eigen::Vector2d(box.min().x(), box.min().y());
	
	Eigen::Vector2d cornor2 = Eigen::Vector2d(box.min().x(), box.max().y());
	Eigen::Vector2d cornor3 = Eigen::Vector2d(box.max().x(), box.min().y());
	Eigen::Vector2d cornor4 = Eigen::Vector2d(box.max().x(), box.max().y());
	points.push_back(cornor1); points.push_back(cornor2); points.push_back(cornor3); points.push_back(cornor4);
	for (int i = 0; i < points.size(); ++i)
	{
		ring->addPoint(points[i][0], points[i][1]);
	}
	ring->closeRings();
	

	return ring;
}

static  inline OGRPolygon* ToOGRPolygon(const ABBox3d& box)
{

	OGRLinearRing* ring = (OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
	OGRPolygon* polygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
	std::vector<Eigen::Vector2d> points;

	Eigen::Vector2d cornor1 = Eigen::Vector2d(box.min().x(), box.min().y());

	Eigen::Vector2d cornor2 = Eigen::Vector2d(box.min().x(), box.max().y());
	Eigen::Vector2d cornor3 = Eigen::Vector2d(box.max().x(), box.min().y());
	Eigen::Vector2d cornor4 = Eigen::Vector2d(box.max().x(), box.max().y());
	points.push_back(cornor1); points.push_back(cornor2); points.push_back(cornor3); points.push_back(cornor4);
	for (int i = 0; i < points.size(); ++i)
	{
		ring->addPoint(points[i][0], points[i][1]);
	}
	
	polygon->addRingDirectly(ring);
	polygon->closeRings();
	

	return polygon;
}

static  inline OGRGeometry* ToPolygon(const std::vector<Eigen::Vector2d>& points)
{
	std::vector<Eigen::Vector3d> points_3d(points.size());
	for (int i = 0; i < points_3d.size(); ++i)
	{
		points_3d[i] = points[i].homogeneous();
	}
	return ToPolygon(points_3d);
}

static  inline OGRGeometry* BoxToPolygon(const ABBox3d& box)
{

	
	std::vector<Eigen::Vector2d> points;

	Eigen::Vector2d cornor1 = Eigen::Vector2d(box.min().x(), box.min().y());
	Eigen::Vector2d cornor2 = Eigen::Vector2d(box.min().x(), box.max().y());
	Eigen::Vector2d cornor3 = Eigen::Vector2d(box.max().x(), box.min().y());
	Eigen::Vector2d cornor4 = Eigen::Vector2d(box.max().x(), box.max().y());
	points.push_back(cornor1);
	points.push_back(cornor2); 
	points.push_back(cornor3);
	points.push_back(cornor4);
	
	return ToPolygon(points);
}


#endif