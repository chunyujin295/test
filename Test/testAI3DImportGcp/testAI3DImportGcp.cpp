
#include <windows.h>
#include <iomanip>
#include "stdlib.h"

//#include "./ImportGcpDlg.h"
#include <QApplication>
#include <QFileIconProvider>
#include <QFileInfo>
#include <qDebug>
#include "qgsprojectionselectionwidget.h"
#include "Core/CoordinateSystem.h"
#include "QComboxTree.h"
#include "qgshighlightablecombobox.h"
#include "qgsprojectionselectiondialog.h"
#include "qgscoordinatetransform.h"
#include "qgssettings.h"
#include "qgsprojutils.h"
#include "qgssqliteutils.h"
#include "qgscoordinatereferencesystem.h"
static int callback(void* data, int argc, char** argv, char** azColName) {
	int i;
	fprintf(stderr, "%s: ", (const char*)data);
	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}



bool IsExists(std::string dbfile,std::string definition)
{

	
	QString authName = QString::fromStdString(definition);
	QString checksql = QStringLiteral("select COUNT(*) from tbl_srs where auth_id = '%1' ").arg(authName);
	int rc;
	sqlite3_database_unique_ptr database1;
	auto db = QString::fromStdString(dbfile);
	int result1 = QgsCoordinateReferenceSystem::openDatabase(db, database1);
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
	sqlite3_statement_unique_ptr statement;
	int rc;
	sqlite3_database_unique_ptr database1;
	
	int result1 = QgsCoordinateReferenceSystem::openDatabase(db, database1);
	if (result1 != SQLITE_OK)
	{
		//QgsDebugMsg( "failed : " + db + " could not be opened!" );
		return -1;
	}
	QString sql = QStringLiteral("select srs_id from tbl_srs where srs_id >= %1").arg(USER_CRS_START_ID);
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
		return USER_CRS_START_ID;
	return *srsids.rbegin()+1;//63320
}

bool ADDUserENUSRS(QString dbFile, std::string auth_id)
{
	
	
	if (IsExists(dbFile.toStdString(), auth_id))
		return true;
	int id = GetUSERCrsID(dbFile);
	if (id == -1)
	{
		return false;
	}
	int nextSrsId = id;//63320
	int nextSrId = 520000000 + nextSrsId - 60000;// 520003321;
	
	std::string srsid = "";
	std::string srId = "";
	if (srId.empty())
	{
		srId = std::to_string(nextSrId);
		
	}
	if (srsid.empty())
	{
		srsid = std::to_string(nextSrsId);
		
	}
	//往库里添加一个坐标系；
	if (!srsid.empty())
	{

		std::string path = dbFile.toStdString();
		sqlite3* database = nullptr;
		int result = sqlite3_open(path.c_str(), &database);

		if (result != SQLITE_OK)
		{
			std::cout << "Could not open .db" << std::endl;

			return false;
		}



		std::string name = auth_id;
		std::string description = name;
		std::string projection_acronym = "";
		std::string ellipsoid_acronym = "";// "WGS84";
		std::string parameters = auth_id;// "EPSG:4326+5773";//

		std::string auth_name = "ENU";
		//std::string auth_id =  "EPSG:4326+5773";
		std::string str = "INSERT INTO tbl_srs(srs_id, description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) VALUES (";
		str += srsid + ",";
		str += "'" + description + "',";
		str += "'" + projection_acronym + "',";
		str += "'" + ellipsoid_acronym + "',";
		str += "'" + parameters + "',";
		str += srId + ",";
		str += "'" + auth_name + "',";
		str += "'" + auth_id + "',0,0)";


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


bool ADDUserSRS()
{
	std::string auth_id = "EPSG:4326+5773";
	auto dbFile = qgisUserDatabaseFilePath();
	if (IsExists(dbFile.toStdString(), auth_id))
		return true;
	int id = GetUSERCrsID(dbFile);
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

		std::string path = qgisUserDatabaseFilePath().toStdString();
		sqlite3* database = nullptr;
		int result = sqlite3_open(path.c_str(), &database);

		if (result != SQLITE_OK)
		{
			std::cout << "Could not open .db" << std::endl;

			return false;
		}

		

		std::string name =  "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";
		std::string description = name;
		std::string projection_acronym =  "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";
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
//添加到自定义的数据库
bool TestAddToUserSrs()
{
	

	//先获取srsid
	//SELECT COUNT(*) FROM students WHERE id='123456'
	QList<long> results;
	QString sql = QStringLiteral("select srs_id from tbl_srs where srs_id >= 63320");
	QString authName = "ENU:22.3,116.33";
	QString checksql = QStringLiteral("select COUNT(*) from tbl_srs where auth_id = 'ENU:22.3,116.30' ");// .arg(authName);
	int rc;
	sqlite3_database_unique_ptr database1;
	auto db = qgisUserDatabaseFilePath();
	int result1 = QgsCoordinateReferenceSystem::openDatabase(db, database1);
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
			QString myRecorddesString = statement1.columnAsText(1);
			std::string desString = myRecorddesString.toStdString();
			QString myRecordNameString = statement1.columnAsText(6);
			std::string nameString = myRecordNameString.toStdString();
			myRecordCount = myRecordCountString.toLong();
			if (myRecordCount > 0)
			{
				return false;
			}
			std::cout << desString << " --- " << nameString << " +++" << myRecordCount << std::endl;

		}
	}

	//char* errMsg = nullptr;
	//	if (sqlite3_exec(database1.get(), checksql.toUtf8(), nullptr, nullptr, &errMsg) == SQLITE_OK)
	//	{
	//		std::cout<< "000" << std::endl;
	//	}
	//	else
	//	{
	//		std::cout << "0001" << std::endl;
	//	}
	sqlite3_statement_unique_ptr statement;
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
			results.append(statement.columnAsInt64(0));
			std::cout << statement.columnAsInt64(0) << std::endl;
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



	long nextSrsId = *srsids.rbegin();//63320
	int nextSrId = 520000000+ nextSrsId-60000;// 520003321;
	nextSrId++;
	nextSrsId++;
	std::string srsid = "";
	std::string srId = "";
	if (srId.empty())
	{
		srId = std::to_string(nextSrId);
		nextSrId++;
	}
	if (srsid.empty())
	{
		srsid = std::to_string(nextSrsId);
		nextSrsId++;
	}
	//往库里添加一个坐标系；
	if (!srsid.empty())
	{

		std::string path = qgisUserDatabaseFilePath().toStdString();
		sqlite3* database = nullptr;
		int result = sqlite3_open(path.c_str(), &database);

		if (result != SQLITE_OK)
		{
			std::cout << "Could not open .db" << std::endl;

			return false;
		}

		/*char* errMsg = nullptr;
		if (sqlite3_exec(database, checksql.toUtf8(), nullptr, nullptr, &errMsg) == SQLITE_OK)
		{
			std::cout<< "000" << std::endl;
		}
		else
		{
			std::cout << "0001" << std::endl;
		}*/

		std::string name = "ENU:22.3,116.33";// "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";
		std::string description = name;
		std::string projection_acronym = "";// "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";
		std::string ellipsoid_acronym = "";// "WGS84";
		std::string parameters = name;// "EPSG:4326+5773";//

		std::string auth_name = "ENU";
		std::string auth_id = name;// "4326+5773";
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
		}

	}

	return true;
}
//测试是否含有某个字段
bool TestHasProj()
{
	std::string path = qgisUserDatabaseFilePath().toStdString();
	sqlite3* database = nullptr;
	int result = sqlite3_open(path.c_str(), &database);

	if (result != SQLITE_OK)
	{
		std::cout << "Could not open qgis.db" << std::endl;

		return false;
	}

	char* errmsg = nullptr;

	const char* data = "Callback function called";
	std::string  selectsql = "SELECT parameters description srid,auth_name,auth_id FROM tbl_srs WHERE description='ECEF' ";


	int res = sqlite3_exec(database, selectsql.c_str(), callback, (void*)data, &errmsg);

	if (res != SQLITE_OK)
	{
		sqlite3_free(errmsg);
	}
	return true;
}

int main(int argc, char** argv)
{
	std::string pstr = "D:/jiaojie/thirdparty/third_party/Windows/vc142/proj/6.3.2/data/";
	std::string strEnv = "PROJ_LIB=" + pstr;
	int status = putenv(strEnv.c_str());
	//double x = 116.283774;
	//double y = 39.808981;
	//double z = -66.339502;
	///*AI3D::CORE::CoordinateTransformer::TransformByEpsgCode(1,&x,&y,&z,"epsg:5773","epsg:4326");
	//std::cout << z << std::endl;*/
	////TestAddToUserSrs();
	QApplication a(argc, argv);
	//PJ_CONTEXT* pjContext = nullptr;;
	//pjContext = proj_context_create();// proj_context_create();
	//auto crs = proj_create(pjContext, "epsg:5773");
	//if (crs)
	//{
	//	
	//	int* confidence = nullptr;
	//	PJ_OBJ_LIST* crsList = proj_identify(QgsProjContext::get(), crs, nullptr, nullptr, &confidence);
	//	const int count = proj_list_get_count(crsList);
	//	std::cout << "1++" << count << std::endl;
	//	
	//}
	//else
	//{
	//std::cout << "2" << std::endl;
	//}
	////先给一个definition 模仿导入进来的坐标系；然后看一下是否有没有就加入数据库
	//QString crsstr = "ENU:22.30000,116.33000";
	////在两个库中都检查是否存在，如果不存在则加入到use类型中
	///*bool bret = ADDUserSRS();
	//if (!bret)
	//	return -1;*/
	//QgsCoordinateReferenceSystem showcrs = QgsCoordinateReferenceSystem(crsstr);//EPSG:4326+5773
	//auto dbFile = qgisUserDatabaseFilePath();
	//ADDUserENUSRS(dbFile,crsstr.toStdString());


	QgsProjectionSelectionWidget dlg2;
	//dlg2.setCrs(showcrs);
	dlg2.show();
	//QString globalsettingsfile = "D:/P/config.ini";
	//QSettings setting(globalsettingsfile,QSettings::IniFormat);
	//QString strGroupNameUser = "user";
	//QString strGroupNameDevice = "device";
	//QString strUser = "admin";
	//QString strPassword = "A123456";
	//QString strIp = "192.168.0.2";
	//QString strPort = "8000";
	////

	////// 分组保存
	//setting.beginGroup(strGroupNameUser);
	//setting.setValue("user_name", strUser);
	//setting.setValue("password", strPassword);
	//setting.endGroup();

	//setting.beginGroup(strGroupNameDevice);
	//setting.setValue("IP", strIp);
	//setting.setValue("IP", strIp);
	////setting.setValue("port", strPort);
	//setting.endGroup();
	//
	//QgsSettings::setGlobalSettingsPath(globalsettingsfile);
	/*QgsSettings setting;
	
	setting.setValue(QStringLiteral("UI/recentProjectionsAuthId"), 1);
	setting.setValue(QStringLiteral("UI/recentProjectionsWkt"), 2);
	setting.setValue(QStringLiteral("UI/recentProjectionsProj4"), 3);*/
	/*QgsProjectionSelectionTreeWidget dlg;*/
	//QgsProjectionSelectionWidget dlg2;
	/*QgsCoordinateTransform trans;
	QgsCoordinateReferenceSystem src,dst;
	src.createFromString("EPSG:4326");
	dst.createFromString("WGS 84 + EGM96 height");
	trans.setSourceCrs(src);
	trans.setDestinationCrs(dst);
	double x = 116.283774;
	double y = 39.808981;
	double z = -66.339502;
	trans.transformCoords(1,&x,&y,&z);*/
	//dlg.show();
	//QSet<QString> crsFilter;
	
	//dlg.show();
	/*double x = 116.283774;
	double y = 39.808981;
	double z = -66.339502;*/
	//AI3D::CORE::CoordinateTransformer::TransformByEpsgCode(1,&x,&y,&z,"epsg:4326+5773","epsg:4326");
	//TestDialog dlg1;
	//dlg2.show();
	int ret = a.exec();
	auto crsnew1 = dlg2.crs();
	
	//
	

	//return 0;
}
 
#include <QObject>
#include<QString>
#include"exiv2/exiv2.hpp"
#include <qmath.h>
using namespace std;



//经度  纬度  高度
const QString GPS_Longitude = "Exif.GPSInfo.GPSLongitude";
const QString GPS_Latitude = "Exif.GPSInfo.GPSLatitude";
const QString GPS_Altitude = "Exif.GPSInfo.GPSAltitude";
//数字字符串转化为exif，保留到小数点后四位，39.6转换后396000/10000
QString AltitudeToExiivGps(const QString& altitude)
{
	QString temp = altitude;
	if (!altitude.contains("."))
	{
		return altitude + "/1";
	}
	QString fz, after, front;
	QStringList tList = altitude.split(".");
	//小数点前
	front = tList.at(0);
	//小数点后，截取前四位
	after = tList.at(1) + "0000";
	after = after.mid(0, 4);
	//整理后的分子
	fz = front + after;
	return fz + "/10000";
}

// 根据n的值递归求度，分，秒
QString GetDDMMSS(const QString& degree, int n, bool bEnd = false)
{
	QStringList list = degree.split(".");
	if (n == 1)
	{
		if (bEnd == false)
			return list.at(0);
		else
			return degree;
	}
	while (n--)
	{
		QString temp = list.at(1);
		int len = temp.length();
		double d = 60.0 * temp.toDouble() / (double)qPow(10, len);
		return GetDDMMSS(QString::number(d, 'g', 12), n, bEnd);
	}
}
///度转度分秒，秒保留小数点后4位
QStringList DegreeToDDMMSS(const QString& degree)
{
	QStringList qlist;
	QString dd, mm, ss;
	if (!degree.contains("."))
	{
		dd = degree;
	}
	else
	{
		dd = GetDDMMSS(degree, 1);
		mm = GetDDMMSS(degree, 2);
		ss = GetDDMMSS(degree, 3, true);
	}
	qlist << dd << mm << ss;
	return qlist;
}

///经纬度转换为EXIF信息 如：113.211  133/1 12/1 396000/10000
QString DDMMSSToExivGps(QStringList& strList)
{

	QString dd, mm, ss;
	dd = strList.at(0) + "/1 ";
	mm = strList.at(1) + "/1 ";
	ss = strList.at(2);
	ss = AltitudeToExiivGps(ss);
	return dd + mm + ss;
}
bool AddExifGPSInfo(const QString& keyStr, const QString& value, Exiv2::ExifData& m_ed)
{
	QStringList tempList;
	QString tempValue;
	if (keyStr == "Exif.GPSInfo.GPSAltitude")
	{
		tempValue = AltitudeToExiivGps(value);
	}
	else
	{
		tempList = DegreeToDDMMSS(value);
		tempValue = DDMMSSToExivGps(tempList);
	}
	std::string _keyStr = keyStr.toLocal8Bit();
	std::string _value = tempValue.toLocal8Bit();
	Exiv2::ExifKey tmp = Exiv2::ExifKey(_keyStr);
	Exiv2::ExifData::iterator pos = m_ed.findKey(tmp);
	//重复判断
	if (pos == m_ed.end())
	{
		Exiv2::URationalValue::AutoPtr rv(new Exiv2::URationalValue);
		//从一个字符串中设置rational组件
		rv->read(_value);
		Exiv2::ExifKey key = Exiv2::ExifKey(_keyStr);
		m_ed.add(key, rv.get());
	}
	else//exif有 key
	{
		//获取指向该值副本的指针 Exiv2::Value::AutoPtr
		Exiv2::Value::AutoPtr v = pos->getValue();
		//将值指针向下强制转换为其实际类型
		Exiv2::URationalValue* prv = dynamic_cast<Exiv2::URationalValue*>(v.release());
		if (prv == 0)
			throw Exiv2::Error(Exiv2::kerErrorMessage, "Downcast failed");
		Exiv2::URationalValue::AutoPtr rv(prv);
		rv->read(_value);
		pos->setValue(rv.get());
	}

	return true;
}



int main111(int argc, char** argv)
{

	double lat = 46.572025;
	QString str = QString::fromStdString(std::to_string(lat));
	std::string temp = "D:/jiaojie/XhxqB00016.JPG";
	auto m_imagePtr = Exiv2::ImageFactory::open(temp);
	if (m_imagePtr.get() == 0)
	{
		
		return 0;
	}
	m_imagePtr->readMetadata();
	Exiv2::ExifKey tmp = Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude");
	auto m_ed = m_imagePtr->exifData();
	Exiv2::ExifData::iterator pos = m_ed.findKey(tmp);
	double deg[3];
	
	{
		for (int i = 0; i < 3; ++i) {
			const int32_t z = pos->toRational(i).first;
			const int32_t d = pos->toRational(i).second;
			if (d == 0) {

				return 0;
			}
			// Hack: Need Value::toDouble
			deg[i] = static_cast<double>(z) / d;
		}
	}

	double min = deg[0] * 60.0 + deg[1] + deg[2] / 60.0;
	int ideg = static_cast<int>(min / 60.0);
	min -= ideg * 60;
	min /= 60.;
	
	AddExifGPSInfo(GPS_Latitude, str, m_ed);
	double lon = 6.539584;
	QString str1 = QString::fromStdString(std::to_string(lon));
	AddExifGPSInfo(GPS_Longitude , str1, m_ed);
	double alt = 563.466;
	QString str2 = QString::fromStdString(std::to_string(alt));
	AddExifGPSInfo(GPS_Altitude, str2, m_ed);
	m_imagePtr->setExifData(m_ed);
	m_imagePtr->writeMetadata();

	return 0;
}

//int main(int argc, char** argv)
//{
////    QApplication a(argc, argv);
////
////    AI3D::CORE::BlockObject obj("D:/TestData/testGDAL");
////    std::vector<std::string> filenames;
////    std::vector<std::string> image_extension;
////    image_extension.push_back(".jpg");
////    obj.SearchImages("D:/TestData/xinghan-gps-2704/photo", filenames, image_extension);
////    AI3D::CORE::Timer time;
////    time.Start();
////#ifdef USE_OPENMP
////#pragma omp parallel for schedule(dynamic)
////#endif
////    for (int it = 0; it < filenames.size(); it++)
////    {
////        std::string outpath = "D:/TestData/testGDAL/" + boost::filesystem::path(filenames[it]).filename().string();
////        // testGDALRead(filenames[it], outpath + ".jpg");
////        // QPixmap pixmap = LoadImage(QString::fromStdString(filenames[it]), QSize(160, 106));// 
////        QString  outfile = QString::fromStdString(outpath + ".jpg");
////        QFileInfo file(QString::fromStdString(filenames[it]));
////        QFileIconProvider incoprovider;
////        QIcon icon = incoprovider.icon(file);
////        QPixmap pixmap = icon.pixmap(160, 106);
////        pixmap.save(outfile);
////    }
////    time.PrintSeconds();
////
////    //QString  infile = "D:/1.JPG";
////
////    //QIcon icon(infile); icon.pixmap(160, 106); //
////
////
////    int ret = a.exec();
//    Application::Getinstance();
//    std::string src_crs =  "ENU:37.73484,112.59318";//"epsg:4978";//
//    std::string dst_crs = "epsg:4326";
//    //PJ* P;
//    //PJ_CONTEXT* C;
//    //PJ* P_for_GIS;
//    //C = proj_context_create();
//  
//    //P = proj_create_crs_to_crs(C,
//    //    src_crs.c_str(),
//    //    dst_crs.c_str(), /* or EPSG:32632 */
//    //    NULL);
//
//
//    //if (0 == P)
//    //{
//
//    //    return false;
//    //}
//
//
//    //P_for_GIS = proj_normalize_for_visualization(C, P);
//    //if (0 == P_for_GIS)
//    //{
//
//    //    return false;
//    //}
//    //proj_destroy(P);
//    //P = P_for_GIS;
//#define USE_PROJ
//    Timer time;
//    /*time.Restart();
//
//   const int64_t size = 1000000;
//
//    std::vector<double> x2(size),y2(size),z2(size);
//    for (int64_t i = 0; i < size; i++)
//    {
//        x2[i] = i;
//        y2[i] = i;
//        z2[i] = i;
//    }
//    time.PrintSeconds();*/
//
//
//  
//
//    std::vector<double> x, y,  z;
//    BlockObject block("D:/MyLearning/Learning_Materials/run/camera/testblock/results");
//    auto Atdata = std::make_shared<ATData>();
//    block.LoadATXML("D:/DOC/gongzuo/4326-1AT.xml", Atdata);//4978-
//    const int64_t size = Atdata->GetPoint3DIds().size();
//   /* double x1[size];
//    double y1[size];
//    double z1[size];*/
//   /* int64_t i = 0;
//    std::vector<double> x1(size), y1(size), z1(size);
//    time.Restart();
//    
//    for (auto& it: Atdata->GetPoints3D())
//    {
//        x1[i] = it.second.GetX();
//        y1[i] = it.second.GetY();
//        z1[i] = it.second.GetZ();
//        i++;
//    }
//    time.PrintSeconds();*/
//
//   // block.LoadATXML("D:/TestData/cc/testpreview/block_AT_notiepoint.xml", Atdata);
//    block.SetATData(Atdata);
//    BlockObject::BlockExportOptions opt; opt.export_tiepoint_ = true;
//    block.ExportATXML("D:/DOC/gongzuo/1AT_4978.xml",opt);
//   // for (auto& it : Atdata->GetPoints3D())
//   // {
//   //    
//   //     x.push_back(it.second.GetX());
//   //     y.push_back(it.second.GetY());
//   //     z.push_back(it.second.GetZ());
//   //    /* double mx, my, mz;
//   //     CoordinateTransformer::Transform(it.second.GetX(), it.second.GetY(), it.second.GetZ(),
//   //         mx,my,mz, src_crs,dst_crs);*/
//   //    // std::cout << mx << " " << mz << " " << std::endl;
//   // }
//   //int numPoints = x.size();
//   //CoordinateTransformer::Transform(numPoints, &x[0], &y[0], &z[0], src_crs, dst_crs);
//   //   /* proj_trans_generic(P,  PJ_FWD ,
//   //     &x[0], sizeof(double), numPoints,
//   //     &y[0], sizeof(double), numPoints,
//   //     &z[0], sizeof(double), numPoints,
//   //     nullptr, sizeof(double), 0);*/
//   //std::cout << x[0] << " " << x[1] << " " << std::endl;
//    time.PrintSeconds();
//    
//    return 0;
//}