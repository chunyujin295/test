#include <iostream>
#include <fstream>
#include <proj.h>
#include <proj/io.hpp>
#include <proj/metadata.hpp>
#include <proj/util.hpp>
#include "Core/Proj/ProjUtils.h"
#include "Core/CoordinateSystem.h"
#include <proj.h>
#include <sqlite3.h>
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/Application.h"
bool ADDUserSRS()
{
	std::string auth_id = "EPSG:4326+5773";
	auto dbFile = AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath();
	if (AI3D::PROJ::CoordinateReferenceSystem::IsExists(dbFile, auth_id))
		return true;
	int id = AI3D::PROJ::CoordinateReferenceSystem::GetUSERCrsID(QString::fromStdString(dbFile));
	if (id == -1)
	{
		return false;
	}
	int nextSrsId = id;//63320
	int nextSrId = 520000000 + nextSrsId - 60000;// 520003321;
	/*nextSrId++;
	nextSrsId++;*/
	std::string srsid = "";
	std::string srId = "";
	if (srId.empty())
	{
		srId = std::to_string(nextSrId);
		//nextSrId++;
	}
	if (srsid.empty())
	{
		srsid = std::to_string(nextSrsId);
		//nextSrsId++;
	}
	//往库里添加一个坐标系；
	if (!srsid.empty())
	{

		std::string path = dbFile;
		
		sqlite3* database = nullptr;
		int result = sqlite3_open(path.c_str(), &database);

		if (result != SQLITE_OK)
		{
			std::cout << "Could not open .db" << std::endl;

			return false;
		}



		std::string name = "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";
		std::string description = name;
		std::string projection_acronym = "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";
		std::string ellipsoid_acronym = "";// "WGS84";
		std::string parameters = auth_id;// "EPSG:4326+5773";//

		std::string auth_name = "EPSG";
		//std::string auth_id =  "EPSG:4326+5773";
		std::string str = "INSERT INTO tbl_srs(srs_id, description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) VALUES (";
		str += srsid + ",";
		str += "'" + description + "',";
		str += "'" + projection_acronym + "',";
		str += "'" + ellipsoid_acronym + "',";
		str += "'" + parameters + "',";
		str += srId + ",";
		str += "'" + auth_name + "',";
		str += "'" + auth_id + "',1,0)";


		// description, projection_acronym, ellipsoid_acronym, parameters, srId, auth_name, auth_id, 1, 0);
		std::string  sql = str;
		char* errMsg = nullptr;
		int inserted = 0;

		if (sqlite3_exec(database, sql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK)
		{
			inserted++;
		}
		else
		{
			if (errMsg)
				sqlite3_free(errMsg);
			return  false;
		}

	}

	return true;
}

//PROJCS["Xian_1980_3_Degree_GK_CM_105E", GEOGCS["GCS_Xian_1980", DATUM["D_Xian_1980", SPHEROID["Xian_1980", 6378140, 298.257]], PRIMEM["Greenwich", 0], UNIT["Degree", 0.0174532925199433]], PROJECTION["Gauss_Kruger"], PARAMETER["False_Easting", 500000], PARAMETER["False_Northing", 0], PARAMETER["Central_Meridian", 105], PARAMETER["Scale_Factor", 1], PARAMETER["Latitude_Of_Origin", 0], UNIT["Meter", 1]] 
int main(int argc, char** argv)
{



	/* This is just to check that pj_init() is locale-safe */
	/* Used by nad/testvarious */
	if (getenv("PROJ_USE_ENV_LOCALE") != nullptr)
		bool use_env_locale = 1;

	/* Enable compatibility mode for init=epsg:XXXX by default */
	if (getenv("PROJ_USE_PROJ4_INIT_RULES") == nullptr) {
		proj_context_use_proj4_init_rules(nullptr, true);
	}
	std::string pstr = "D:/jiaojie/thirdparty/third_party/Windows/vc142/proj/6.3.2/data/";
	std::string strEnv = "PROJ_LIB=" + pstr;
	int status = putenv(strEnv.c_str());
	ADDUserSRS();
	
	double x = 116.283774;
	double y = 39.808981;
	double z = -56.127000;// -66.339502; //4061673.090456 
	//AI3D::CORE::CoordinateTransformer::TransformByEpsgCode(1, &x, &y, &z, "epsg:4326+5773", "epsg:4978");
	//std::cout<< std::setprecision(17) << z << std::endl;

	PJ_CONTEXT* pjContext = nullptr;;
	pjContext = proj_context_create();// proj_context_create();
	QString definition = "EPSG:4326+5773";
	AI3D::PROJ::ProjUtils::proj_pj_unique_ptr crs(proj_create(pjContext, definition.toLatin1().constData()));
	if (crs)
		std::cout <<"--"   << std::endl;
	QStringList parts = definition.split(':');
	QString auth = parts.at(0);
	QString code = parts.at(1);
	code = definition;
	{
		AI3D::PROJ::ProjUtils::proj_pj_unique_ptr crs1(proj_create_from_database(AI3D::PROJ::ProjContext::get(), auth.toLatin1(), code.toLatin1(), PJ_CATEGORY_CRS, false, nullptr));
	if(crs1)
		std::cout << auth.toStdString() << " " << code.toStdString() << " "  << std::endl;
	}
	
	return 0;

}