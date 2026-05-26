#include <iostream>
#include <fstream>
#include <proj.h>
#include <proj/io.hpp>
#include <proj/metadata.hpp>
#include <proj/util.hpp>
#include "Core/Proj/CoordinateReferenceSystem.h"
//#include "ProjCore/qgsprojectionselectiontreewidget.h"
//#include "ProjCore/ProjectionSelectionDialog.h"
//#include "ProjCore/QgsProjectionSelectionWidget.h"
int main1(int argc, char** argv)
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
	//dlg.show();
	double x = 116.283774;
	double y = 39.808981;
	double z = -56.127;
	PJ_CONTEXT* C;
	PJ* P;
	PJ* P_for_GIS;
	
	// 创建一个上下文对象，这个是非线程安全的，建议为每个线程都创建一个
	// 如果是单线程使用，可以直接使用 PJ_DEFAULT_CTX ，而不创建
	C = proj_context_create();// proj_context_create();



	P = proj_create_crs_to_crs(C,
		"EPSG:4326+5773", "EPSG:4326", /* or EPSG:32632 */
		NULL);


	if (0 == P)
	{

		return false;
	}


	P_for_GIS = proj_normalize_for_visualization(C, P);
	if (0 == P_for_GIS)
	{

		return false;
	}
	proj_destroy(P);
	P = P_for_GIS;
	proj_trans_generic(P, PJ_FWD,
		&x, sizeof(double), 1,
		&y, sizeof(double), 1,
		&z, sizeof(double), 1,
		nullptr, sizeof(double), 0);
//	b = proj_trans(P, PJ_FWD, a);
	



	// 清理使用对象
	proj_destroy(P);
	// 上下文对象在线程结束前，或不再使用的时候进行销毁
	proj_context_destroy(C); /* 如果是单线程也可忽略这一步 */

	return 1;
}


void testQGS()
{
	QString crsstr = "ENU:22.30000,116.33000";
	QgsCoordinateReferenceSystem showcrs = QgsCoordinateReferenceSystem(crsstr);
	std::cout << "GRT " << showcrs.authid().toStdString() << std::endl;
	crsstr = "EPSG:4326+5773";
	QgsCoordinateReferenceSystem showcrs1 = QgsCoordinateReferenceSystem(crsstr);
	std::cout << "GET" << showcrs1.authid().toStdString() << std::endl;
}
#include <sqlite3.h>
#include "Core/Proj/SqliteUtils.h"
#include "Core/Proj/CoordinateReferenceSystem.h"
using namespace AI3D::PROJ;
bool IsExists(std::string dbfile, std::string definition)
{


	QString authName = QString::fromStdString(definition);
	QString checksql = QStringLiteral("select COUNT(*) from tbl_srs where auth_id = '%1' ").arg(authName);
	int rc;
	AI3D::PROJ::sqlite3_database_unique_ptr database1;
	auto db = QString::fromStdString(dbfile);
	int result1 = AI3D::PROJ::CoordinateReferenceSystem::openDatabase(db, database1);
	if (result1 != SQLITE_OK)
	{
		//QgsDebugMsg( "failed : " + db + " could not be opened!" );
		return false;
	}

	long          myRecordCount = 0;
	auto statement1 = database1.prepare(checksql, rc);
	if (rc == SQLITE_OK)
	{
		if (statement1.step() == SQLITE_ROW)
		{
			QString myRecordCountString = statement1.columnAsText(0);

			myRecordCount = myRecordCountString.toLong();
			if (myRecordCount > 0)
			{
				return true;
			}

		}
	}

	return false;
}


int GetUSERCrsID(QString db)
{
	AI3D::PROJ::sqlite3_statement_unique_ptr statement;
	int rc;
	AI3D::PROJ::sqlite3_database_unique_ptr database1;

	int result1 = AI3D::PROJ::CoordinateReferenceSystem::openDatabase(db, database1);
	if (result1 != SQLITE_OK)
	{
		//QgsDebugMsg( "failed : " + db + " could not be opened!" );
		return -1;
	}
	QString sql = QStringLiteral("select srs_id from tbl_srs where srs_id >= %1").arg(AI3D::PROJ::USER_CRS_START_ID);
	statement = database1.prepare(sql, rc);
	std::set<long> srsids;
	while (true)
	{
		int ret = statement.step();

		if (ret == SQLITE_DONE)
		{
			// there are no more rows to fetch - we can stop looping
			break;
		}

		if (ret == SQLITE_ROW)
		{

			long id = statement.columnAsInt64(0);
			srsids.insert(id);
		}
		else
		{
			std::cout << "1" << std::endl;
			//CHYQgsMessageLog::logMessage( QObject::tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( database.get() ) ), QObject::tr( "SpatiaLite" ) );
			break;
		}
	}


	if (srsids.empty())
		return AI3D::PROJ::USER_CRS_START_ID;
	return *srsids.rbegin() + 1;//63320
}
#include "Core/Application.h"
bool ADDUserSRS()
{
	std::string auth_id = "EPSG:4326+5773";
	auto dbFile = AI3D::CORE::Application::Getinstance().GetProjUserSrsFullPath();
	if (IsExists(dbFile, auth_id))
		return true;
	int id = GetUSERCrsID(QString::fromStdString(dbFile));
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
	//testQGS();
	//QApplication a(argc, argv);
	//QString crsstr = "PROJCS[\"Xian_1980_3_Degree_GK_CM_105E\", GEOGCS[\"GCS_Xian_1980\", DATUM[\"D_Xian_1980\", SPHEROID[\"Xian_1980\", 6378140, 298.257]], PRIMEM[\"Greenwich\", 0], UNIT[\"Degree\", 0.0174532925199433]], PROJECTION[\"Gauss_Kruger\"], PARAMETER[\"False_Easting\", 500000], PARAMETER[\"False_Northing\", 0], PARAMETER[\"Central_Meridian\", 105], PARAMETER[\"Scale_Factor\", 1], PARAMETER[\"Latitude_Of_Origin\", 0], UNIT[\"Meter\", 1]]";// "ENU:22.30000,116.33000";
	//QgsCoordinateReferenceSystem showcrs = QgsCoordinateReferenceSystem(crsstr);
	//std::cout << showcrs.authid().toStdString() << std::endl;;
	/*QgsProjectionSelectionTreeWidget ProjectionSelectionWidget dlg;*/
	/*dlg.setCrs(showcrs);
	dlg.show();*/
	//dlg.selectCrs();
//	return a.exec();

}