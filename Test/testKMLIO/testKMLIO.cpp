#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <Eigen/Core>
#include <iostream>
#include <string>
#include "kml/dom.h"  // The KML DOM header.

int main99() {
	// Parse KML from a memory buffer.
	std::string errors; std::string filename = "D:/jiaojie/test/yueshu/water.kml";
	kmldom::ElementPtr element = kmldom::Parse("<kml>"
      "<Placemark>"
        "<name>hi</name>"
        "<Point>"
          "<coordinates>1,2,3</coordinates>"
        "</Point>"
      "</Placemark>"
		"</kml>", 
		&errors);

	// Convert the type of the root element of the parse.
	const kmldom::KmlPtr kml = kmldom::AsKml(element);
	const kmldom::PlacemarkPtr placemark =
		kmldom::AsPlacemark(kml->get_feature());

	// Access the value of the <name> element.
	std::cout << "The Placemark name is: " << placemark->get_name()
		<< std::endl;
	return 0;
}


int main(int argc, char** argv)
{
	
	std::string filename = "D:/jiaojie/test/yueshu/water.kml";
	GDALAllRegister();
	std::vector < std::vector<Eigen::Vector3d>> ret;
	GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
	if (poDS == nullptr)
	{
		return 1;
	}

	GDALDataset::Layers layers = poDS->GetLayers();
	if (layers.size() > 1)
	{
		return 1;
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
			OGRLinearRing* ring = polygon->getExteriorRing();
			std::vector<Eigen::Vector3d> vectemp;
			if (!ring->isClockwise())
			{
				ring->reversePoints();
			}
			for (int i = 0; i < ring->getNumPoints(); ++i)
			{
				vectemp.emplace_back(ring->getY(i), ring->getX(i), ring->getZ(i));
			}
			ret.emplace_back(vectemp);
		}
		else if (poGeometry != nullptr && wkbFlatten(type) == wkbMultiPolygon)
		{
			OGRMultiPolygon* polygons = static_cast<OGRMultiPolygon*>(poGeometry);
			const int polygon_count = polygons->getNumGeometries();
			/*if (polygon_count != 1)
			{
				break;
			}*/

			for (int cnt = 0; cnt < polygon_count; cnt++)
			{
				std::vector<Eigen::Vector3d> vectemp;
				OGRPolygon* polygon = static_cast<OGRPolygon*>(polygons->getGeometryRef(cnt));
				OGRLinearRing* ring = polygon->getExteriorRing();

				if (!ring->isClockwise())
				{
					ring->reversePoints();
				}
				for (int i = 0; i < ring->getNumPoints(); ++i)
				{
					vectemp.emplace_back(ring->getY(i), ring->getX(i), ring->getZ(i));
				}
				ret.emplace_back(vectemp);
			}


		}
		OGRFeature::DestroyFeature(poFeature);
	}
	GDALClose(poDS);
	return 0;
}
