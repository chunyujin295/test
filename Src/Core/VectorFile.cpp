#include "Core/VectorFile.h"



std::shared_ptr<OGRPolygon> convertToOGRPolygon(const std::vector<Eigen::Vector3d>& vertices)
{

	OGRLinearRing ring;
	for (const auto& vertex : vertices)
	{
		ring.addPoint(vertex.x(), vertex.y(), vertex.z());

	}
	std::shared_ptr<OGRPolygon> polygon = std::make_shared<OGRPolygon>();
	polygon->addRing(&ring);
	return polygon;

}

bool CreateVecFile(const std::vector<std::shared_ptr<OGRPolygon>>& polygons, const std::string& vec_filename)
{
	GDALAllRegister();
	std::string ext = vec_filename.substr(vec_filename.rfind('.')+1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {return std::tolower(c); });
	GDALDriver* driver = ext == "kml" ? GetGDALDriverManager()->GetDriverByName("KML") :
		(ext == "shp" ? GetGDALDriverManager()->GetDriverByName("ESRI Shapefile") : nullptr);
	if (!driver)
	{
		return false;
	}
	GDALDataset* dataSource = driver->Create(vec_filename.c_str(),0,0,0,GDT_Unknown,NULL);

	if (!dataSource)
		return false;

	OGRLayer* layer = dataSource->CreateLayer("polygons", NULL, wkbPolygon, NULL);
	if (!layer)
		return false;

	OGRFieldDefn fieldDefn("Name",OFTString);
	layer->CreateField(&fieldDefn);

	for (int i = 0; i < polygons.size(); ++i)
	{
		OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
		feature->SetGeometry(polygons[i].get());
		feature->SetField("Name",("Polgon" + std::to_string(i+1)).c_str());
		layer->CreateFeature(feature);
		OGRFeature::DestroyFeature(feature);
	}
	GDALClose(dataSource);
	return true;
}


std::vector<std::vector<Eigen::Vector3d>>  ReadPolygonsFromVecFile(const std::string& filename)
{
	GDALAllRegister();
	std::vector < std::vector<Eigen::Vector3d>> ret;
	GDALDataset* poDS = static_cast<GDALDataset*>(
		GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
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

		std::vector<OGRPolygon*> ogrPolygons;


		if (poGeometry != nullptr && wkbFlatten(type) == wkbPolygon)
		{
			ogrPolygons.push_back(static_cast<OGRPolygon*>(poGeometry));
		}
		else if (poGeometry != nullptr && wkbFlatten(type) == wkbMultiPolygon)
		{
			OGRMultiPolygon* polygons = static_cast<OGRMultiPolygon*>(poGeometry);
			const int polygon_count = polygons->getNumGeometries();
			

			for (int cnt = 0; cnt < polygon_count; cnt++)
			{
				ogrPolygons.push_back(static_cast<OGRPolygon*>(polygons->getGeometryRef(cnt)));
			}
		}
		
		if (!ogrPolygons.empty())
		{
			for (int cnt = 0; cnt < ogrPolygons.size(); cnt++)
			{

				const auto& polygon = ogrPolygons[cnt];
				OGRLinearRing* ring = polygon->getExteriorRing();
				if (ring->isClockwise())
				{
					ring->reversePoints();
				}
				std::vector<Eigen::Vector3d> poly_pts;
				for (int i = 0; i < ring->getNumPoints(); ++i)
				{
					poly_pts.emplace_back(ring->getX(i), ring->getY(i), ring->getZ(i));
				}
				ret.emplace_back(std::move(poly_pts));
			}
		}

			
		OGRFeature::DestroyFeature(poFeature);
	}
	GDALClose(poDS);
	return ret;
}
bool SavePolygonsToVecFile(std::vector<std::vector<Eigen::Vector3d>>& polygons, const std::string& vec_filename)
{
	if (polygons.empty())
		return false;
	std::vector<std::shared_ptr<OGRPolygon>> ogrPolygons(polygons.size());
	for (int cnt = 0; cnt < polygons.size(); cnt++)
	{
		ogrPolygons[cnt] = convertToOGRPolygon(polygons[cnt]);
	}
	return CreateVecFile(ogrPolygons, vec_filename);
}

