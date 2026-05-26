#include <iostream>
#include <fstream>
#include "Core/ATData.h"
#include "Core/File.h"
#include "Core/ProjectObject.h"
#include <set>
#include "Core/ExifIO.h"
#include <glog/logging.h>
#include <pugixml.hpp>
#include "Core/Application.h"
#include "Core/AlgorithmBase.h"
#include "Core/CoordinateSystem.h"
#include "Core/Types.h"
#include "Core/Timer.h"
#include "Core/Rapidjson.h"
#include "Core/CameraModels.h"
#include <Core/String.h>
#include <Eigen/Sparse>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <qimage.h>
//#include <thread>
//#include <opencv2/core/core.hpp>
//#include <opencv2/opencv.hpp>
//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/uuid_generators.hpp>
//#include <boost/uuid/uuid_io.hpp>
//#include <regex>
//#include <Windows.h>
//#include <boost/regex.hpp>
//#include <exif.h>
//#include <alkcore/2.1/include/Reconstruction/Reconstruct.h>
using namespace AI3D::CORE;
//using namespace easyexif;
//需要测试的几项
// 测试排序。测试删除后id


//
////
////int main1(int argc, char **argv) 
////{
////    EIGEN_STL_UMAP(std::string, image_t) imagefiles_,map2;
////    
////    std::string file1 = "e:/1.jpg";
////    std::string file2 = "e:/2.jpg";
////    std::string file3 = "e:/3.jpg";
////   
////    imagefiles_.insert(std::pair<std::string, image_t >(file1,100 ));//不会覆盖
////    imagefiles_.insert(std::pair<std::string, image_t >(file1,1001 ));
////    imagefiles_.insert(std::pair<std::string, image_t >(file2,100 ));
////    //imagefiles_.insert(std::pair<std::string, image_t >(file1,100  ));
////
////    map2[file1] = 100;
////    map2[file1] = 1001;
////    map2[file2] = 100;
////    
////    IndMatch f1,f2,f3;
////    f1.filename = file1;
////    f1.id = 100;
////    f2.filename = file1;
////    f2.id = 1001;
////    f3.filename = file2;
////    f3.id = 100;
////
////    std::set<IndMatch > imageset = { f1, f2, f3 };
////
////    std::cout << imagefiles_.size() << std::endl;
////    std::cout << map2.size() << std::endl;
////    std::cout << imageset.size() << std::endl;
////    std::cout << " imagefiles_ " << std::endl;
////    for (auto& it : imagefiles_)
////    {
////        std::cout << it.first << " " << it.second << std::endl;
////    }
////    std::cout << " map2 " << std::endl;
////    for (auto& it : map2)
////    {
////        std::cout << it.first << " " << it.second << std::endl;
////    }
////    std::cout << " set " << std::endl;
////    for (auto& it : imageset)
////    {
////        std::cout << it.filename << " " << it.id  << std::endl;
////    }
////
////    //Project project("1.tri","D:/");
////    //project.SaveJson();
////    return EXIT_SUCCESS;
////}
//
////测试删除影像
//bool testDeleteImage(BlockObject block, image_t id)
//{
//	return true;
//};
//
////测试该
//void cbFinish(int ivalue)
//{
//
//
//}
//
//void cbMessage(const std::string& msg)
//{
//
//
//}
//
//void cbProgress(float fvalue)
//{
//
//
//}
//
//
//
//void testProject()
//{
//	//uint8_t i = 0;
//	//while (true)
//	//{
//	//	i += 10000;
//	//}
//	std::string jobpath = "D:/data/Projects/NewProject11/Block_2/job_20220318203016/task_def_0.json";
//	//std::string task_def_1 = "D:/MyLearning/Testing/block_1/gcp/task_def_1.json";
//	std::string task_def_1 = "D:/data/Projects/NewProject11/Block_2/job_20220318203016/task_def_1.json";
//	std::string task_def_3 = "D:/data/Projects/NewProject1/Block_4/job_20211226112055/task_def_3.json";
//	std::string task_def_5 = "D:/data/Projects/NewProject1/Block_4/job_20211226112055/task_def_5.json";
//
//	std::string task_def_6 = "D:/data/Projects/NewProject3/Block_4/job_20211226115358/task_def_1.json";
//
//	std::string task_def_0_json;
//	std::string task_def_1_json;
//	std::string task_def_3_json;
//	std::string task_def_5_json;
//	std::string task_def_6_json;
//	RapidJsonCore::ReadFile(jobpath, task_def_0_json);
//	RapidJsonCore::ReadFile(task_def_1, task_def_1_json);
//	//RapidJsonCore::ReadFile(task_def_3, task_def_3_json);
//	//RapidJsonCore::ReadFile(task_def_5, task_def_5_json);
//	//RapidJsonCore::ReadFile(task_def_6, task_def_6_json);
//	bool GShoudStop = false;
//	// core 
//	//static ReconstructCallBack ReconstructCallBack_(cbFinish, cbProgress, cbMessage, &GShoudStop);
//	//std::cout << GenTasks(task_def_0_json, ReconstructCallBack_) << std::endl;
//	//std::cout << "RunFeatureDetection: " << RunFeatureDetection(task_def_1_json, ReconstructCallBack_) << std::endl;
//	//std::cout << "RunFeatureMatch: " << RunFeatureMatch(task_def_3_json, ReconstructCallBack_) << std::endl;
//	//std::cout << "RunSfM: " << RunSfM(task_def_5_json, ReconstructCallBack_) << std::endl;
//
//	//std::cout << RunOptimizeAT(task_def_0_json, ReconstructCallBack_) << std::endl;
//	//project.Load(project_path);
//	// 
//	// auto atdata = project.GetBlock(1).GetCurrentAT();
//	 //project.DeleteBlock(3);
//	 //project.NewProject("1.tri", "D:/");
//	 //BlockObject block("D:/");
//
//	 //project.AddBlock(block);
//	 //std::cout << project.GetNumBlocks() << std::endl;;
//
//	 //project.AddBlock(block);
//	 //project.ImportBlock("d:/1.xml");
//
//	 //project.DeleteBlock(1);
//	 //std::cout << project.GetNumBlocks() << std::endl;;
//
//	 //project.CloneBlock(1);
//	 //project.CloneBlock(2)
//}
//
//BlockObject* Getblock(std::string &xml_path)
//{
//	BlockObject* block = new BlockObject("");
//	auto Atdata = std::make_shared<ATData>();
//	block->LoadATXML(xml_path, Atdata);
//	//block->SetATData(Atdata);
//	block->SetAT0(Atdata);
//	LOG(INFO) << Atdata.use_count();
//	return block;
//}
//
//void cbProcess(int process)
//{
//	std::cout << process << std::endl;
//}
//
//#ifdef _MSC_VER
////#include <combaseapi.h>
//#endif
//std::string newGUID()
//{
//	std::string resbuf;
//	GUID guid;
//	//调用生成函数
//	HRESULT h = CoCreateGuid(&guid);
//	//若成功生成，则进行数值转换
//	if (h == S_OK) {
//		char buf[64] = { 0 };
//		sprintf_s(buf, sizeof(buf),
//			"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
//			guid.Data1, guid.Data2, guid.Data3,
//			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
//			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
//		resbuf = std::string(buf);
//	}
//	return resbuf;
//}
//
//#if 0
///// @brief 采用字符串查找来提取MAC地址
///// @remark 该方法有很大局限性，并非全部OS返回的MAC地址前导字符串都是
/////     "Physical Address. . . . . . . . . : "
//bool ParseMac(const std::string& str, std::string& macOUT)
//{
//	static const std::string beginMarkOfMAC("Physical Address. . . . . . . . . : ");
//	static const std::string endMarkOfMAC("\r\n");
//	size_t begin = str.find(beginMarkOfMAC);
//	if (begin != std::string::npos)
//	{
//		begin += beginMarkOfMAC.size();
//		size_t end = str.find(endMarkOfMAC, begin);
//		if (end != std::string::npos)
//		{
//			macOUT = str.substr(begin, end - begin - 1);
//			return true;
//		}
//	}
//	return false;
//}
//#else
///// @brief 采用 boost::regex来提取MAC
//bool ParseMac(const std::string& str, std::string& macOUT)
//{
//	const static boost::regex expression(
//		"([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})",
//		boost::regex::perl | boost::regex::icase);
//	boost::cmatch what;
//	if (boost::regex_search(str.c_str(), what, expression))
//	{
//		macOUT = what[1] + "-" + what[2] + "-" + what[3] + "-" + what[4] + "-" + what[5] + "-" + what[6];
//		return true;
//	}
//	return false;
//}
//#endif
//
//bool ParseUUID(const std::string& str, std::string& uuidOUT)
//{
//	const static boost::regex expression(
//		"UUID=([0-9a-fA-F]{8})-([0-9a-fA-F]{4})-([0-9a-fA-F]{4})-([0-9a-fA-F]{4})-([0-9a-fA-F]{12})", boost::regex::perl | boost::regex::icase
//	);
//	boost::cmatch what;
//	if (boost::regex_search(str.c_str(), what, expression))
//	{
//		uuidOUT = what[1] + "-" + what[2] + "-" + what[3] + "-" + what[4] + "-" + what[5];
//		return true;
//	}
//	return false;
//}
//bool GetMacByCmd(std::string& macOUT)
//{
//	bool ret = false;
//
//	//初始化返回MAC地址缓冲区
//	SECURITY_ATTRIBUTES sa;
//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//	//创建管道
//	HANDLE hReadPipe, hWritePipe;
//	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
//	{
//		//控制命令行窗口信息
//		STARTUPINFO si;
//		//返回进程信息
//		PROCESS_INFORMATION pi;
//		si.cb = sizeof(STARTUPINFO);
//		GetStartupInfo(&si);
//		si.hStdError = hWritePipe;
//		si.hStdOutput = hWritePipe;
//		si.wShowWindow = SW_HIDE; //隐藏命令行窗口
//		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//
//		//创建获取命令行进程
//		TCHAR szCommandLine[] = TEXT("ipconfig /all");
//		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
//		{
//			WaitForSingleObject(pi.hProcess, 3000); // 设置超时时间，防止Vista、Win7等操作系统卡死
//			unsigned long count;
//			CloseHandle(hWritePipe);
//			std::string strBuffer(1024 * 10, '\0'); // 准备足够大的缓冲区
//			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
//			{
//				strBuffer.resize(strBuffer.find_first_of('\0')); // 截掉缓冲区后面多余的'\0'
//				ret = ParseMac(strBuffer, macOUT);//提取MAC地址串
//			}
//			CloseHandle(pi.hThread);
//			CloseHandle(pi.hProcess);
//		}
//		else
//		{
//			CloseHandle(hWritePipe); // VS2010下调试，此处会有“An invalid handle was specified”的中断，直接运行正常，原因未知。VS2008上正常。
//		}
//		CloseHandle(hReadPipe);
//	}
//
//	return ret;
//}
//bool GetUUIDByCmd(std::string& uuidOUT)
//{
//	bool ret = false;
//
//	//初始化返回MAC地址缓冲区
//	SECURITY_ATTRIBUTES sa;
//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//	//创建管道
//	HANDLE hReadPipe, hWritePipe;
//	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
//	{
//		//控制命令行窗口信息
//		STARTUPINFO si;
//		//返回进程信息
//		PROCESS_INFORMATION pi;
//		si.cb = sizeof(STARTUPINFO);
//		GetStartupInfo(&si);
//		si.hStdError = hWritePipe;
//		si.hStdOutput = hWritePipe;
//		si.wShowWindow = SW_HIDE; //隐藏命令行窗口
//		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//
//		//创建获取命令行进程
//		TCHAR szCommandLine[] = TEXT("wmic csproduct list full");
//		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
//		{
//			WaitForSingleObject(pi.hProcess, 3000); // 设置超时时间，防止Vista、Win7等操作系统卡死
//			unsigned long count;
//			CloseHandle(hWritePipe);
//			std::string strBuffer(1024 * 10, '\0'); // 准备足够大的缓冲区
//			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
//			{
//				strBuffer.resize(strBuffer.find_first_of('\0')); // 截掉缓冲区后面多余的'\0'
//				ret = ParseUUID(strBuffer, uuidOUT);//提取MAC地址串
//			}
//			CloseHandle(pi.hThread);
//			CloseHandle(pi.hProcess);
//		}
//		else
//		{
//			CloseHandle(hWritePipe); // VS2010下调试，此处会有“An invalid handle was specified”的中断，直接运行正常，原因未知。VS2008上正常。
//		}
//		CloseHandle(hReadPipe);
//	}
//
//	return ret;
//}
//bool GetDiskSerialNumByCmd(std::string& DiskSerialNum)
//{
//	bool ret = false;
//
//	//初始化返回MAC地址缓冲区
//	SECURITY_ATTRIBUTES sa;
//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//	//创建管道
//	HANDLE hReadPipe, hWritePipe;
//	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
//	{
//		//控制命令行窗口信息
//		STARTUPINFO si;
//		//返回进程信息
//		PROCESS_INFORMATION pi;
//		si.cb = sizeof(STARTUPINFO);
//		GetStartupInfo(&si);
//		si.hStdError = hWritePipe;
//		si.hStdOutput = hWritePipe;
//		si.wShowWindow = SW_HIDE; //隐藏命令行窗口
//		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//
//		//创建获取命令行进程
//		TCHAR szCommandLine[] = TEXT("wmic diskdrive get serialnumber");
//		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
//		{
//			WaitForSingleObject(pi.hProcess, 3000); // 设置超时时间，防止Vista、Win7等操作系统卡死
//			unsigned long count;
//			CloseHandle(hWritePipe);
//			std::string strBuffer(1024 * 10, '\0'); // 准备足够大的缓冲区
//			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
//			{
//				strBuffer.resize(strBuffer.find_first_of('\0')); // 截掉缓冲区后面多余的'\0'
//				//ret = ParseUUID(strBuffer, uuidOUT);
//				std::ostringstream ostr(strBuffer);
//				std::string key;
//				while (ostr << key)
//				{
//					if (!key.empty() && key.compare("SerialNumber"))
//					{
//						DiskSerialNum += key + "/";
//					}
//				}
//				
//				DiskSerialNum = strBuffer;
//			}
//			CloseHandle(pi.hThread);
//			CloseHandle(pi.hProcess);
//		}
//		else
//		{
//			CloseHandle(hWritePipe); // VS2010下调试，此处会有“An invalid handle was specified”的中断，直接运行正常，原因未知。VS2008上正常。
//		}
//		CloseHandle(hReadPipe);
//	}
//
//	return ret;
//}
//
//

//}
//void testRename()
//{
//	BlockObject* block = new BlockObject("");
//	std::string photoDir = "X:/TY0102_2500/photo";
//	std::string pref = "ZKZKZK";
//	std::string poseTXT = "X:/TY0102_2500/pos/TY2500Pos.txt";
//	std::string poseXLSX = "X:/TY0102_2500/pos/POS2500.xlsx";
//	std::string gcpTXT = "X:/TY0102_2500/kzd/controlpoints.txt";
//	srs_s srs;
//	srs.definition = "EPSG:4326";
//	srs.ID = 0;
//	srs.type = coord_system_type_e::GEOGRAPHIC;
//	int progressValue = 0;
//	int numLen = 4;
//	int begin = 1;
//	std::thread AddBlockThread(std::bind(&BlockObject::AddBlockData, block, photoDir, &progressValue, pref,  srs,gcpTXT, poseTXT, numLen, begin));
//	AddBlockThread.detach();
//	int originprogress = 0;
//	while (true)
//	{
//		if (originprogress != progressValue)
//		{
//			std::cout << progressValue << std::endl;
//		}
//		if (progressValue == 100)
//		{
//			std::cout << "Finished!" << std::endl;
//			break;
//		}
//		originprogress = progressValue;
//	}
//
//}
//void testothers()
//{
//	cv::Mat src = cv::imread("X:/bug/DSC01264.png",cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
//	std::cout << src.rows << std::endl;
//	std::set<uint8_t> image;
//	for (int i = 0; i < src.rows; i++)
//	{
//		//for (int j = 0; j < src.cols; j++)
//		//{
//		//	src.
//		//	image.insert(src(i, j));
//		//}
//	}
//	cv::Mat result = src.clone();
//	cv::Mat result2 = src.clone();
//	// 多边形顶点坐标
//	std::vector<cv::Point> points1, points2;
//	points1.push_back(cv::Point(src.cols / 4, src.rows / 4));
//	points1.push_back(cv::Point(src.cols / 4, src.rows / 8));
//	points1.push_back(cv::Point(src.cols / 2, src.rows / 6));
//	points1.push_back(cv::Point(src.cols / 3, src.rows / 2));
//	points1.push_back(cv::Point(src.cols / 2, src.rows / 8));
//	points2.push_back(cv::Point(src.cols / 3, src.rows / 3));
//	points2.push_back(cv::Point(src.cols / 2, src.rows / 2));
//	points2.push_back(cv::Point(3 * src.cols / 4, src.rows / 2));
//	points2.push_back(cv::Point(5 * src.cols / 8, 3 * src.rows / 4));
//	points2.push_back(cv::Point(src.cols / 2, src.rows / 3));
//	// 每个vector<Point>就是一个多边形
//	std::vector<std::vector<cv::Point>> pic;
//	pic.push_back(points1);
//	pic.push_back(points2);
//	// 绘制多边形集合
//	fillPoly(result, pic, cv::Scalar(0, 0, 255), 16, 0);
//	// 绘制单个凸多边形
//	fillConvexPoly(result2, points2, cv::Scalar(0, 0, 255), 16, 0);
//
//	cv::imshow("original", src);
//	cv::imshow("result", result);
//	cv::imshow("result2", result2);
//	cv::waitKey(0);
//	system("pause");
//
//
//	BlockObject* block = new BlockObject("X:/Projects/test_V833449/Block_4");
//	auto ATdata = std::make_shared<ATData>();
//	block->LoadATXML("X:/Projects/test_V833449/Block_4/block_AT_absolute.xml", ATdata);
//	std::string icons_path = block->GetPath() + PATH_SEPARATOR_STR + "icons";
//	File::CreateDirIfNotExists(icons_path);
//	std::string apppath = Application::Getinstance().GetAPPPath();
//	boost::filesystem::copy_file(apppath + PATH_SEPARATOR_STR + "moldai32.png", icons_path + PATH_SEPARATOR_STR + "moldai32.png", boost::filesystem::copy_option::overwrite_if_exists);
//	boost::filesystem::copy_file(apppath + PATH_SEPARATOR_STR + "down.png", icons_path + PATH_SEPARATOR_STR + "down.png", boost::filesystem::copy_option::overwrite_if_exists);
//	boost::filesystem::copy_file(apppath + PATH_SEPARATOR_STR + "up.png", icons_path + PATH_SEPARATOR_STR + "up.png", boost::filesystem::copy_option::overwrite_if_exists);
//
//	try 
//	{
//		std::string DJ_image_path = "D:/MyLearning/Learning_Materials/run/camera/testblock/DJ/100_0038_0002.JPG";
//		ExifIO exif;
//		exif.Open(DJ_image_path);
//		exif.SetBrand("");
//		exif.SetModel("");
//
//		exif.Write();
//	}
//	catch (const std::exception& err)
//	{
//		LOGD(err.what());
//	}
//
//
//
//
//	std::string image_path = "D:/MyLearning/Learning_Materials/run/camera/testblock/testarw/DSC08630.ARW";
//	std::string image_path_ = "D:/TestData/tif/B/19003010010001B.tif";
//	std::string thumbnail_path = "D:/MyLearning/Learning_Materials/run/camera/testblock/testarw/DSC08630thumb.ARW";
//	Bitmap bmp;
//	bmp.Read(image_path_);
//	std::cout << bmp.GetWidth() << " " << bmp.GetHeight() << std::endl;
//
//	char* name = "**";
//	std::cout << sizeof(name) << std::endl;
//	std::cout << sizeof(uint64_t) << std::endl;
//	std::fstream iofile("D:/MyLearning/Learning_Materials/run/camera/testblock/test.bin", std::ios::out | std::ios::in | std::ios::binary);
//	if (iofile)
//	{
//		std::string a1("************************9999999999999999999999999999999999999999999999999999999999999999999999999999900000000000000000000abcDEF");
//		iofile.write((char*)&a1, sizeof(a1));;
//		Eigen::Vector2d xy;
//		xy[0] = 0.1;
//		xy[1] = 0.2;
//		iofile.write((char*)&xy, sizeof(xy));
//		Eigen::Vector2d xy_tmp;
//
//		iofile.seekg(0, std::ios::beg);
//		std::string a2;
//		iofile.read((char*)&a2, sizeof(std::string));
//		iofile.read((char*)&xy_tmp, sizeof(Eigen::Vector2d));
//		std::cout << xy_tmp << std::endl;
//	}
//	iofile.close();
//
//	Exiv2::Image::AutoPtr image_ = Exiv2::ImageFactory::open(image_path);
//	image_->readMetadata();
//	Exiv2::ExifData ed = image_->exifData();
//	
//	Exiv2::ExifThumb thumb(ed);
//
//	Exiv2::URational xre(160, 1);
//	Exiv2::URational yre(106, 1);
//
//	thumb.setJpegThumbnail(thumbnail_path+ "-thumb.jpg", xre, yre, 0);
//	image_->writeMetadata();
//	auto ret = thumb.writeFile(thumbnail_path);
//
//	std::string image_path1 = "D:/MyLearning/Learning_Materials/run/camera/testblock/dir3/IMG_2347.JPG";
//
//
//	Application::Getinstance().ExportConfig();
//	std::string make = "Canon";
//	std::string model = "Canon PowerShot Canon A3300 IS Canon";
//	String::StringRemoveALL(model, make, false);
//
//	std::string tm(Timer::TimeNow());
//	std::cout << tm << std::endl;
//
//	time_t t = time(0);
//	char tmp[64];
//	strftime(tmp, sizeof(tmp), "%Y/%m/%d %X %A 本年第%j天 %z", localtime(&t));
//	std::cout << tmp << std::endl;
//	std::string src_dir = "C:/data/Projects/NewProject_8/Block_2";
//	std::string dst_dir = "C:/data/Projects/NewProject_8_Copy/Block_2";
//	File::CopyDirectory(src_dir, dst_dir);
//	std::string test_path1 = "D:/MyLearning/Learning_Materials/run/camera/";
//	std::string test_path2 = "D: / TestData / xinghan - gps - 2704 / photo//Right//XhxqR00015.JPG";
//	std::cout << File::EnsureUnifySlash(test_path2) << std::endl;
//	std::cout << File::GetParentDir(test_path1) << std::endl;
//	std::cout << File::EnsureTrailingSlash(test_path1);
//	std::string coord_name1 = "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height(EPSG:5773)";
//	std::string coord_name2 = "CGCS2000 / 3-degree Gauss-Kruger CM 114E (EPSG:4547)";
//
//	std::cout << CoordinateTransformer::GetSRSDB().size() << std::endl;
//	srs_s srs1 = CoordinateDescriptor::GetSRSFromName(coord_name1);
//	srs_s srs2 = CoordinateDescriptor::GetSRSFromName(coord_name2);
//
//}
//void testGCPERROR()
//{
//	/*Eigen::Vector3d xyz0{ 1044.81961928888 ,
//							-12.1022566967715 ,
//							970.986779407052  };
//	Eigen::Matrix3d R;
//	R << 0.0212143940633193,
//							0.999457157056348  ,
//							-0.0252059654321933,
//							0.821661223868686  ,
//							-0.0030661176705614, 
//							0.56996792200355   ,
//							0.569581234482967  ,
//							-0.0328022885066415,
//							-0.821280236699744 ;
//	srs_s src_crs = CoordinateDescriptor::GetSRSFromDefinition("ENU:37.73427,112.58421");
//	srs_s dst_crs = CoordinateDescriptor::GetSRSFromDefinition(BASESRS);*/
//	//AI3D::CORE::CoordinateTransformer::Transform(xyz0, R,
//	//	src_crs, dst_crs);
//	//"D:/TestData/cc/test/ff/Block_11(gcptest+1g3measure).xml";//
//	std::string xml_filepath_ ="D:/data/Projects/NewProject23/Block_2-export.xml" ;// "D:/TestData/gcptest/block_AT-TY2500-1.xml";//"D:/DOC/gongzuo/1AT.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
//	std::string xml_filepath ="D:/data/Projects/NewProject23/block_AT.xml" ;// "D:/TestData/gcptest/block_AT-TY2500-1.xml";//"D:/DOC/gongzuo/1AT.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
//	BlockObject block;
//	BlockObject block_origin;
//	auto atdata = std::make_shared<ATData>();
//	auto atdata_origin = std::make_shared<ATData>();
//	//block.LoadATXML(xml_filepath_, atdata);
//	//block.SetATData(atdata);
//	block.LoadATXML(xml_filepath, atdata);
//	block_origin.LoadATXML(xml_filepath_, atdata_origin);
//	block_origin.SetATData(atdata_origin);
//	double z_avg = 0;
//
//	block_origin.UpdateATGroup(atdata);
//	block_origin.SetAT0(atdata);
//
//
//	EIGEN_STL_UMAP(point3D_t, class ControlPoint) &controlpoints = atdata->GetControlPointsMutual();
//	float offset = 0.00;
//	for (auto& cp : controlpoints)
//	{
//		ControlPoint &controlpoint = cp.second;
//
//		//auto trackelements = controlpoint.GetObjectPoint().GetTrack().GetElements();
//		//auto images = atdata->GetImages();
//		//auto cameras = atdata->GetCameras();
//
//		//std::vector<Eigen::Vector2f> points;
//		//std::vector<Eigen::Matrix<float, 3, 4>> poses;
//
//		//for (int i = 0; i < 3; i++)
//		//{
//
//		//	Image& image = images[trackelements[i].image_id];
//		//	if (!image.HasPosition() && !image.HasRotationMatrix())
//		//	{
//		//		continue;
//		//	}
//		//	Eigen::Vector2d xy = trackelements[i].xy;
//		//	Camera& camera = cameras[image.GetCameraId()];
//		//	if (camera.GetParams()[0] == 0.0)
//		//	{
//		//		continue;
//		//	}
//		//	Eigen::Vector2d world_point = camera.ImageToWorld(xy);
//
//		//	Camera undistorted_camera;
//		//	undistorted_camera.SetModelId(PinholeCameraModel::model_id);
//		//	undistorted_camera.SetWidth(camera.GetWidth());
//		//	undistorted_camera.SetHeight(camera.GetHeight());
//
//		//	// Copy focal length parameters.
//		//	const std::vector<size_t>& focal_length_idxs = camera.GetFocalLengthIdxs();
//		//	CHECK_LE(focal_length_idxs.size(), 2)
//		//		<< "Not more than two focal length parameters supported.";
//		//	if (focal_length_idxs.size() == 1) {
//		//		undistorted_camera.SetFocalLengthX(camera.GetFocalLength());
//		//		undistorted_camera.SetFocalLengthY(camera.GetFocalLength());
//		//	}
//		//	else if (focal_length_idxs.size() == 2)
//		//	{
//		//		undistorted_camera.SetFocalLengthX(camera.GetFocalLengthX());
//		//		undistorted_camera.SetFocalLengthY(camera.GetFocalLengthY());
//		//	}
//
//		//	// Copy principal point parameters.
//		//	undistorted_camera.SetPrincipalPointX(camera.GetPrincipalPointX());
//		//	undistorted_camera.SetPrincipalPointY(camera.GetPrincipalPointY());
//
//		//	Eigen::Vector2d  undis_xy = (undistorted_camera.GetCalibrationMatrix() * world_point.homogeneous()).hnormalized();
//
//
//
//		//	points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });
//
//		//	poses.push_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
//		//	/*std::cout << std::setprecision(19) << " 0 " << image.GetName() << std::endl;
//		//	std::cout << std::setprecision(19) << " 1 " <<image.GetPosition() << std::endl;
//		//	std::cout << std::setprecision(19) << " 2 " << image.GetRotationMatrix() << std::endl;
//		//	std::cout << std::setprecision(19) << " 3 " << image.GetProjectionMatrix() << std::endl;
//		//	std::cout << std::setprecision(19) << " 4 " << xy << std::endl;
//		//}
//
//
//		//Eigen::Vector3d xyz{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
//
//		//if (points.size() >= VALIDTRIANGLENUM)
//		//{
//		//	// 待测试 算出来的结果异常的时候具体表现形式
//		//	/*Eigen::Vector3d xyz{ -DBL_MAX, -DBL_MAX, -DBL_MAX };*/
//		//	Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
//		//	xyz[0] = xyzf.x();
//		//	xyz[1] = xyzf.y();
//		//	xyz[2] = xyzf.z();
//
//		//	controlpoint.GetObjectPointMutual().GetEstimatedXYZMutual() = xyz;
//
//		//	CoordinateTransformer::Transform(1, &xyz[0], &xyz[1], &xyz[2],
//		//		CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs()).definition, controlpoint.GetSrs().definition);
//		//	controlpoint.GetEstimatedXYZMutual() = xyz;
//
//		//	std::cout << std::setprecision(19) << xyz << std::endl;
//		//}
//		//else
//		//{
//		//	controlpoint.GetObjectPointMutual().GetEstimatedXYZMutual() = xyz;
//		//	std::cout << std::setprecision(19) << xyz << std::endl;
//		//}
//
//		//for (int i = 3; i < 6; i++)
//		//{
//		//	Image& image = images[trackelements[i].image_id];
//		//	std::cout << std::setprecision(8) << "Origin:\n"
//		//		<< "\t" << trackelements[i].xy.x() << " " << trackelements[i].xy.y() << std::endl;
//		//	Camera camera = cameras[image.GetCameraId()];
//		//	auto image_Point2d = AI3D::CORE::AlgorithmBase::ProjectPointToImage(controlpoint.GetObjectPointMutual().GetEstimatedXYZMutual(), image.GetProjectionMatrix(), camera);
//		//	std::cout << std::setprecision(8) << "Predict:\n"
//		//		<< "\t" << image_Point2d.x()<<" " << image_Point2d.y() << std::endl;
//		//	std::cout << std::endl;
//		//}
//
//
//		if (controlpoint.GetObjectPointMutual().GetTrackMutual().Length() >= 2)
//		{
//
//			//if(controlpoint.GetId() == 33)
//			if (controlpoint.GetName() == "0769J1-01-025-001"/*"0531P02-0021"*/)
//			{
//				std::cout << controlpoint.GetName() << "-------" << std::endl;
//				//for (auto&ele : controlpoint.GetObjectPointMutual().GetTrackMutual().GetElements())
//				int cnt = 0;
//				//for (auto& image_id : atdata->GetImagesIds())
//				//{
//				//	//	if ((image_id == 376)|| image_id == 377)
//				//		//if (image_id == 435)
//				//	{
//				//		auto oriimage = atdata->GetImage(image_id);
//
//
//				//		camera_t cameraid = oriimage.GetCameraIdMutual();
//				//		auto camera = atdata->GetCamera(cameraid);
//
//				//		Eigen::Vector3d xyz = controlpoint.GetObjectPointMutual().GetXYZ();
//
//
//				//		auto image_Point2d = AI3D::CORE::AlgorithmBase::ProjectPointToImage(xyz,
//				//			oriimage.GetProjectionMatrix(), camera);
//
//				//		bool status1 = (camera.GetWidth() - offset) > image_Point2d(0) && image_Point2d(0) > (0 + offset);
//				//		bool status2 = (camera.GetHeight() - offset) > image_Point2d(1) && image_Point2d(1) > (0 + offset);
//				//		if (status1 && status2)
//				//		{
//				//			//std::cout << cnt++ << std::endl;
//				//			//std::cout << oriimage.GetPath() + "/"+ oriimage.GetName() << " " <<image_Point2d.x() << " " << image_Point2d.y() << " " <<std::endl;
//				//		}
//				//	}
//
//				//}
//			}
//				//ATData atdata_ = *atdata;
//				std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > estimate_xy;
//				atdata->UpdateGCPMeasurementError(controlpoint.GetId(),estimate_xy);
//				//controlpoint.GetObjectPointMutual().GetTrackMutual().DeleteElement(2);
//				atdata->ComputeGCPEstimatedXYZ(controlpoint.GetId());
//				atdata->Compute3DErrorForGCP(controlpoint.GetId());
//				atdata->ComputeDistErrorForGCP(controlpoint.GetId());
//				atdata->ComputeSquaredReprojectionErrorForGCP(controlpoint.GetId());
//				//CoordinateTransformer::Transform(Estimate[0], Estimate[1], Estimate[2], Estimate[0], Estimate[1], Estimate[2], CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs()).definition, "EPSG:4326");
//				std::cout << controlpoint.GetId() << " " << controlpoint.GetName() << " "
//					<< "\n\tGivenXYZ: " << File::ToStringWithHighPrecision(controlpoint.GetGivenXYZ()[0]) << ", " << File::ToStringWithHighPrecision(controlpoint.GetGivenXYZ()[1]) << ", " << File::ToStringWithHighPrecision(controlpoint.GetGivenXYZ()[2]) <<
//					"\n\tEstimateXYZ: " << File::ToStringWithHighPrecision(controlpoint.GetEstimatedXYZ()[0]) << ", " << File::ToStringWithHighPrecision(controlpoint.GetEstimatedXYZ()[1]) << ", " << File::ToStringWithHighPrecision(controlpoint.GetEstimatedXYZ()[2]) << "\n\tRMS of reproj. error:[px] " << controlpoint.GetObjectPoint().GetPixelRMS() << "\tRMS of dist. rays[m]" << controlpoint.GetObjectPointMutual().GetDistRMS()\
//					<< "\t3D error[m]: " << controlpoint.Get3DError() << "\t3D horizontal error[m] " << 
//					controlpoint.GetXY3DError() << "\t3D vertical error[m] " << controlpoint.GetZ3DError() << std::endl;
//
//				//std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > measurement_error_map;
//				//atdata_.UpdataGCPErrorInfo(0, measurement_error_map);
//				//auto gcp_ = atdata_.GetControlPoints().at(0);
//				//std::cout << gcp_.GetId() << " " << gcp_.GetName() << " "
//				//	<< "\n\tGivenXYZ: " << File::ToStringWithHighPrecision(gcp_.GetGivenXYZ()[0]) << ", " << File::ToStringWithHighPrecision(gcp_.GetGivenXYZ()[1]) << ", " << File::ToStringWithHighPrecision(gcp_.GetGivenXYZ()[2]) <<
//				//	"\n\tEstimateXYZ: " << File::ToStringWithHighPrecision(gcp_.GetEstimatedXYZ()[0]) << ", " << File::ToStringWithHighPrecision(gcp_.GetEstimatedXYZ()[1]) << ", " << File::ToStringWithHighPrecision(gcp_.GetEstimatedXYZ()[2]) << "\n\tRMS of reproj. error:[px] " << gcp_.GetObjectPoint().GetPixelRMS() << "\tRMS of dist. rays[m]" << gcp_.GetObjectPointMutual().GetDistRMS()\
//				//	<< "\t3D error[m]: " << gcp_.Get3DError() << "\t3D horizontal error[m] " <<
//				//	gcp_.GetXY3DError() << "\t3D vertical error[m] " << gcp_.GetZ3DError() << std::endl;
//			
//
//		}std::cout << std::endl;
//	}
//}
//
////上三角矩阵的存储和访问
////先把矩阵的上三角矩阵元素存了
//void storageLowerTriangle(int array[], int i, int j, int e,int N) {
//	array[(2 * N - i + 2) * (i - 1) / 2 + (j - i)] = e;
//}
////再存储常数C
//void storageConstant(int array[], int constant, int N) {
//	array[N * (N + 1) / 2] = constant;
//}
//
//
////三角矩阵的元素获取
//int getValue(int array[], int i, int j, int N) {
//	if (i <= j) {
//		//访问上三角区元素
//		return array[(2 * N - i + 2) * (i - 1) / 2 + (j - i)];
//	}
//	else {
//		//访问下三角区的常数
//		return array[N * (N + 1) / 2];
//	}
//}
//
//void testUpperTriangle(int N)
//{
//	//二维数组初始化
//	int** matrix = new int* [N];
//	for (int i = 0; i < N; i++)
//	{
//		matrix[i] = new int[N];
//		for (int j = 0; j < N; j++)
//		{
//			matrix[i][j] = 1;
//		}
//	}
//
//}
//
//#define N 10000
//void testmatch(const int& argc, char** argv)
//{
//	if (argc < 2)
//	{
//		std::cout << "Usage: " << argv[0] << ": match filepath" << std::endl;
//	}
//	std::string match_path = argv[1];
//
//	std::ifstream ifs(match_path, std::ios::in);
//	if (!ifs.is_open())
//	{
//		std::cout << "Error read file" << std::endl;
//		return;
//	}
//
//	//存储匹配结果
//	std::vector<int> match_pairs;
//
//	while (ifs.good())
//	{
//		std::istringstream iss;
//		std::string key, line;
//
//		std::getline(ifs, line);
//		if (!line.empty())
//		{
//			auto vec_line = String::StringSplit(line, ",");
//			match_pairs.push_back(std::atoi(vec_line[2].c_str()));
//			match_pairs.push_back(std::atoi(vec_line[3].c_str()));
//		}
//
//		//iss.str(line);
//		//while (iss >> key)
//		//{
//		//	if (!key.empty())
//		//	{
//		//		match_pairs.push_back(std::atoi(key.c_str()));
//		//	}
//		//}
//	}
//	ifs.close();
//
//	//构造匹配矩阵
//	auto minmax = std::max_element(match_pairs.begin(), match_pairs.end());
//	std::vector<std::vector<int>>matchMatrix(*minmax + 1, std::vector<int>(*minmax + 1));
//
//	std::ofstream ofs(match_path + "numsImagesPerimage.txt");
//	std::vector<int>numsImagesPerimage;
//	for (int i = 0; i <= *minmax; i++)
//	{
//		int num = std::count(match_pairs.begin(), match_pairs.end(), i);
//		numsImagesPerimage.emplace_back(num);
//		ofs << i << " " << num << "\n";
//	}
//	ofs.close();
//
//	std::cout << *std::max_element(numsImagesPerimage.begin(), numsImagesPerimage.end()) << " " << *std::min_element(numsImagesPerimage.begin(), numsImagesPerimage.end()) << std::endl;
//
//	cv::Size matchsize(*minmax + 1, *minmax + 1);
//	cv::Mat mapimage(matchsize, CV_8UC3);
//	cv::Scalar red(0, 0, 255);
//	for (int i_pair = 0; i_pair < match_pairs.size() - 1; i_pair = i_pair + 2)
//	{
//		matchMatrix[match_pairs[i_pair]][match_pairs[i_pair + 1]] = 1;
//		cv::Point point(match_pairs[i_pair], match_pairs[i_pair + 1]);
//		cv::drawMarker(mapimage, point, red, 0, 0.5);
//	}
//
//	std::string binary_path = match_path + "match_matrix.jpg";
//	cv::imwrite(binary_path, mapimage);
//
//	int num_pairs = 0;
//	for (int i = 0; i < *minmax + 1; i++)
//	{
//		num_pairs += std::accumulate(matchMatrix[i].begin(), matchMatrix[i].end(),0);
//	}
//	std::cout << "像对数：" << num_pairs << std::endl;
//
//	std::vector<std::vector<int>>match_blocks;
//	std::vector<int>match_pair;
//	std::set<int> num_images_per_blcok ;
//	for (int row = 0; row <= *minmax; row++)
//	{
//		num_images_per_blcok.insert(row);
//		for (int col = row + 1; col <= *minmax; col++)
//		{
//			if (matchMatrix[row][col])
//			{
//				num_images_per_blcok.insert(col);
//				if (num_images_per_blcok.size() >= N)
//				{
//					match_blocks.push_back(match_pair);
//					std::sort(match_pair.begin(), match_pair.end());
//					auto pos = std::unique(match_pair.begin(), match_pair.end());
//					match_pair.erase(pos,match_pair.end());
//					std::cout << "每个block中的影像数为：" << match_pair.size() << std::endl;
//					match_pair.clear();
//					num_images_per_blcok.clear();
//				}
//				match_pair.emplace_back(row);
//				match_pair.emplace_back(col);
//			}
//		}
//	}
//	//剩余影像数单独成块
//	match_blocks.push_back(match_pair);
//	std::sort(match_pair.begin(), match_pair.end());
//	auto pos = std::unique(match_pair.begin(), match_pair.end());
//	match_pair.erase(pos, match_pair.end());
//	std::cout << "每个block中的影像数为：" << match_pair.size() << std::endl;
//
//	//block输出
//	for (int i = 0; i < match_blocks.size(); i++)
//	{
//		std::string out_match = match_path + std::to_string(i);
//		std::ofstream os(out_match, std::ios::out);
//		for (int j = 0; j < match_blocks[i].size() - 1; j++)
//		{
//			os << match_blocks[i][j] << " " << match_blocks[i][j + 1] << "\n";
//		}
//		os.close();
//	}
//}
//

int testCameraDB(const int& argc, char** argv)
{
	std::string imageFilepath = "";
	std::string sfileDatabase = "";
	std::string outputdir = "";
	//解析文件夹路径
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == std::string("-i"))
		{
			imageFilepath = argv[i + 1];
		}
		else if (argv[i] == std::string("-o"))
		{
			outputdir = argv[i + 1];
		}
		else if (argv[i] == std::string("--db"))
		{
			sfileDatabase = argv[i + 1];
		}
	}

	//std::string path = "//10.10.82.21/Test_Data/sjgy/sjgy/photo/";
	//size_t start = 1;
	//while ((start = path.find(PATH_SEPARATOR, start)) != std::string::npos)
	//	if (path[start - 1] == PATH_SEPARATOR)
	//		path.erase(start, 1);
	//	else
	//		++start;

	std::vector<std::string> imageabsFilepath;
	if (!imageFilepath.empty())
	{
		//递归获取文件夹下所有文件，包括子文件夹(会搜索到Thumbs.db文件)
		//CamerasGenerator::GetFiles(imageFilepath, imageabsFilepath);
		BlockObject block;
		std::vector<std::string> image_extesion = { ".jpg" };
		block.SearchImages(imageFilepath, imageabsFilepath, image_extesion);
	}
	//CamerasGenerator camgener;
	//CameraDatabase database;
	//database.OutputDB(sfileDatabase);

	//std::vector<CamerasGenerator::camera_datasheet_s> vec_database;
	//if (!sfileDatabase.empty())
	//{
	//	if (!camgener.parseDatabase(sfileDatabase, vec_database))
	//	{
	//		std::cerr
	//			<< "\nInvalid input database: " << sfileDatabase
	//			<< ", please specify a valid file." << std::endl;
	//		return -1;
	//	}
	//}
	Timer time;
	time.Start();
	int num_thumb = 0;

	std::vector<Exiv2::ExifData> ed_vector;
	for (std::vector<std::string>::const_iterator iter_image = imageabsFilepath.begin();
		iter_image != imageabsFilepath.end(); ++iter_image)
	{
		if ((*iter_image).find("mask.png") != std::string::npos
			|| (*iter_image).find("_mask.png") != std::string::npos)
		{
			std::cout << *iter_image << " is a mask image" << "\n";
			continue;
		}
		Exiv2::Image::UniquePtr image_;
		try
		{
			image_ = Exiv2::ImageFactory::open(*iter_image);
		}
		catch (Exiv2::Error& e)
		{
			LOGE(String::StringPrintf("Caught Exiv2 exception %s", e));
			return false;
		}


		if (image_.get() == 0)
		{
			LOGW("Exif is empty");
			return false;
		}

		image_->readMetadata();
		Exiv2::ExifData ed = image_->exifData();
		ed_vector.push_back(ed);

		if (0 && !ed.empty())
		{
			//从exif中解析并保存略图信息
			std::string path = File::GetParentDir(imageFilepath);
			path = path + "/ThumbC";
			File::CreateDirIfNotExists(path);
			Exiv2::ExifThumbC thumbc(ed);
			//auto ret = thumbc.writeFile(path + PATH_SEPARATOR_STR + File::GetPathBaseName(*iter_image));
			//if (ret)
			//{
			//	num_thumb++;
			//}
		}

		//通过可交换文件获取focal length和focal length35mm等效等信息
		//double focal_length, focal_lengthIn35mm, width, height, ppx, ppy;
		//focal_length = focal_lengthIn35mm = width = height = ppx = ppy = -1;

		//ExifInfo exifInfo;
		//std::unique_ptr<ExifIO> exif_(new ExifIO);
		//exif_->Open(*iter_image);

		//short orientation = exif_->GetOrientation();
		//std::cout << orientation << std::endl;
		//exif_->SetBrand("DJI");
		//exif_->SetFocal(21);
		//exif_->SetFocalLengthIn35mm(35);
		//exif_->SetModel("无人机");
		//exif_->SetLatitude(36.10);
		//exif_->SetLongitude(120.11);
		//exif_->SetAltitude(100);
		//exif_->Write();

	}
	LOGI(String::StringPrintf("Reading Exif thumbs spends %f s", time.ElapsedSeconds()));
	std::string path = File::GetParentDir(imageFilepath);
	path = path + "/ThumbC";
	File::CreateDirIfNotExists(path);
	time.Restart();
	for (int i = 0; i < ed_vector.size(); i++)
	{
		if (!ed_vector[i].empty())
		{
			//从exif中解析并保存略图信息
			std::string thumbname = "img_" + std::to_string(i);
			Exiv2::ExifThumbC thumbc(ed_vector[i]);
			auto ret = thumbc.writeFile(path + PATH_SEPARATOR_STR + File::GetPathBaseName(thumbname));
			if (ret)
			{
				num_thumb++;
			}
		}
	}
	LOGI(String::StringPrintf("Getting thumbs spends %f s", time.ElapsedSeconds()));
	std::cout << "The num of Thumb is " << num_thumb << std::endl;
	return 0;
}
//static inline unsigned int pix_to_bucket(const Eigen::Vector2i& x, int W, int H)
//{
//	if (x(1) == 0) return x(0); // Top border
//	if (x(0) == W - 1) return W - 1 + x(1); // Right border
//	if (x(1) == H - 1) return 2 * W + H - 3 - x(0); // Bottom border
//	return 2 * (W + H - 2) - x(1); // Left border
//}
//void testeasyexif(const int& argc, char** argv)
//{
//	std::string imageFilepath = "";
//	std::string sfileDatabase = "";
//	std::string outputdir = "";
//	//解析文件夹路径
//	for (int i = 0; i < argc; i++)
//	{
//		if (argv[i] == std::string("-i"))
//		{
//			imageFilepath = argv[i + 1];
//		}
//		else if (argv[i] == std::string("-o"))
//		{
//			outputdir = argv[i + 1];
//		}
//		else if (argv[i] == std::string("--db"))
//		{
//			sfileDatabase = argv[i + 1];
//		}
//	}
//
//	//std::string path = "//10.10.82.21/Test_Data/sjgy/sjgy/photo/";
//	//size_t start = 1;
//	//while ((start = path.find(PATH_SEPARATOR, start)) != std::string::npos)
//	//	if (path[start - 1] == PATH_SEPARATOR)
//	//		path.erase(start, 1);
//	//	else
//	//		++start;
//
//	std::vector<std::string> imageabsFilepath;
//	if (!imageFilepath.empty())
//	{
//		//递归获取文件夹下所有文件，包括子文件夹(会搜索到Thumbs.db文件)
//		//CamerasGenerator::GetFiles(imageFilepath, imageabsFilepath);
//		BlockObject block;
//		std::vector<std::string> image_extesion = { ".jpg" };
//		block.SearchImages(imageFilepath, imageabsFilepath, image_extesion);
//	}
//	//CamerasGenerator camgener;
//	//CameraDatabase database;
//	//database.OutputDB(sfileDatabase);
//
//	//std::vector<CamerasGenerator::camera_datasheet_s> vec_database;
//	//if (!sfileDatabase.empty())
//	//{
//	//	if (!camgener.parseDatabase(sfileDatabase, vec_database))
//	//	{
//	//		std::cerr
//	//			<< "\nInvalid input database: " << sfileDatabase
//	//			<< ", please specify a valid file." << std::endl;
//	//		return -1;
//	//	}
//	//}
//	Timer time;
//	time.Start();
//	int num_thumb = 0;
//	int num_success_qimage = 0;
//	int num_success_bitmap = 0;
//	for (std::vector<std::string>::const_iterator iter_image = imageabsFilepath.begin();
//		iter_image != imageabsFilepath.end(); ++iter_image)
//	{
//		if ((*iter_image).find("mask.png") != std::string::npos
//			|| (*iter_image).find("_mask.png") != std::string::npos)
//		{
//			std::cout << *iter_image << " is a mask image" << "\n";
//			continue;
//		}
//		std::string imagefullpath = *iter_image;
//		QImage image;
//		if (image.load(QString::fromStdString(*iter_image)))
//		{
//			num_success_qimage++;
//		}
//		//std::unique_ptr<ExifIO> exif_(new ExifIO);
//		//exif_->Open_Beta(*iter_image);	
//	}
//	LOGI(String::StringPrintf("Qimage reading %d images spends %f s", num_success_qimage, time.ElapsedSeconds()));
//
//	time.Restart();
//	for (std::vector<std::string>::const_iterator iter_image = imageabsFilepath.begin();
//		iter_image != imageabsFilepath.end(); ++iter_image)
//	{
//		if ((*iter_image).find("mask.png") != std::string::npos
//			|| (*iter_image).find("_mask.png") != std::string::npos)
//		{
//			std::cout << *iter_image << " is a mask image" << "\n";
//			continue;
//		}
//		std::string imagefullpath = *iter_image;
//		Bitmap bitmap;
//		if (bitmap.Read(*iter_image))
//		{
//			num_success_bitmap++;
//		}
//	}
//	LOGI(String::StringPrintf("Bitmap reading %d images spends %f s", num_success_bitmap, time.ElapsedSeconds()));
//}
//
//void testLoadAndSaveSeparally()
//{
//	BlockObject block("");
//	auto ATdata = std::make_shared<ATData>();
//	std::string xml_path = "D:/MyLearning/Testing/AT.xml";
//	block.LoadATXML(xml_path, ATdata);
//
//	block.SetATData(ATdata);
//
//	Timer time;
//	time.Start();
//	block.GetCurrentATMutual()->UpdateTiepoints();
//	time.PrintSeconds();
//
//}
//
//void testCheckFile(int argc, char** argv)
//{
//	std::string imageFilepath = "";
//	std::string sfileDatabase = "";
//	std::string outputdir = "";
//	//解析文件夹路径
//	for (int i = 0; i < argc; i++)
//	{
//		if (argv[i] == std::string("-i"))
//		{
//			imageFilepath = argv[i + 1];
//		}
//		else if (argv[i] == std::string("-o"))
//		{
//			outputdir = argv[i + 1];
//		}
//		else if (argv[i] == std::string("--db"))
//		{
//			sfileDatabase = argv[i + 1];
//		}
//	}
//
//	Timer time;
//	time.Start();
//
//	std::vector<std::string> imageabsFilepath;
//	File::EnsureUnifySlash(imageFilepath);
//	if (!imageFilepath.empty())
//	{
//		//递归获取文件夹下所有文件，包括子文件夹(会搜索到Thumbs.db文件)
//		//CamerasGenerator::GetFiles(imageFilepath, imageabsFilepath);
//		BlockObject block;
//		std::vector<std::string> image_extesion = { ".jpg" };
//		block.SearchImages(imageFilepath, imageabsFilepath, image_extesion);
//	}
//	LOGW(*imageabsFilepath.begin());
//	LOGW(String::StringPrintf("block.SearchImages() spends %f s", time.ElapsedSeconds()));
//	LOGW(String::StringPrintf("imageabsFilepath size %d", imageabsFilepath.size()));
//
//	time.Restart();
//	for (const auto& image_path : imageabsFilepath)
//	{
//		File::IsFileExistent(image_path);
//	}
//	LOGW(String::StringPrintf("File::IsFileExistent() spends %f s", time.ElapsedSeconds()));
//
//	time.Restart();
//	for (const auto& image_path : imageabsFilepath)
//	{
//		struct stat info;
//		//if (stat(image_path.c_str(), &info) != 0) {  // does not exist
//		//	printf("cannot access %s\n", image_path.c_str());
//		//}
//		//else if (info.st_mode & S_IFDIR) {          // directory
//		//	printf("%s is a directory\n", image_path.c_str());
//		//}
//		//else {
//		//	printf("%s is no directory\n", image_path.c_str());
//		//}
//
//		if (stat(image_path.c_str(), &info) != 0) 
//		{  // does not exist
//			printf("cannot access %s\n", image_path.c_str());
//		}
//		else if (!(info.st_mode & S_IFREG))
//		{          // regular file
//			printf("%s is not a regular file\n", image_path.c_str());
//		}
//	}
//	LOGW(String::StringPrintf("stat spends %f s", time.ElapsedSeconds()));
//
//	time.Restart();
//	File file;
//	int num_exist_file = 0;
//	for (const auto& image_path : imageabsFilepath)
//	{
//		if (!file.IsFileExistent_1(image_path))
//		{
//			num_exist_file++;
//		}
//	}
//	LOGW(String::StringPrintf("%d images does not exist!", num_exist_file));
//	LOGW(String::StringPrintf("none static spends %f s", time.ElapsedSeconds()));
//}
//
//
//#define GLUT_DISABLE_ATEXIT_HACK 
//#define MAX_CHAR        128
//
//#include <gl/glut.h>
//#include <stdio.h>
//#include <string.h>
//#include <sstream>
//
//
//using namespace std;
//string num2string(int i) {
//	stringstream ss;
//	ss << i;
//	return ss.str();
//}
//
//void drawString(string strn) {
//	static int isFirstCall = 1;
//	static GLuint lists;
//	const char* str = strn.c_str();
//	if (isFirstCall) { // 如果是第一次调用，执行初始化
//		// 为每一个ASCII字符产生一个显示列表
//		isFirstCall = 0;
//
//		// 申请MAX_CHAR个连续的显示列表编号
//		lists = glGenLists(MAX_CHAR);
//
//		// 把每个字符的绘制命令都装到对应的显示列表中
//		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
//	}
//	// 调用每个字符对应的显示列表，绘制每个字符
//	for (; *str != '\0'; ++str)
//		glCallList(lists + *str);
//}
//
//void createCoordinate(GLfloat x0, GLfloat y0)
//{
//	int i = 0, k = 5;
//	GLfloat x[5], y[5];
//	for (i = 0; i < k; i++) {
//		x[i] = x0 + (200 - 2 * x0) / k * (i + 1);
//		y[i] = y0 + (150 - 2 * y0) / k * (i + 1);
//	}
//
//	//设置颜色
//	glClear(GL_COLOR_BUFFER_BIT);
//	glColor3f(0.0f, 0.0f, 0.0f);
//
//	GLfloat lineWidth = 2.0f;
//	glLineWidth(lineWidth);
//	glBegin(GL_LINES);
//	glVertex2f(10.0f, 5.0f);   //纵轴
//	glVertex2f(10.0f, 147.0f);
//	glVertex2f(9.0f, 143.0f);
//	glVertex2f(10.0f, 147.0f);
//	glVertex2f(11.0f, 143.0f);
//	glVertex2f(10.0f, 147.0f);
//	glVertex2f(5.0f, 10.0f);   //横轴
//	glVertex2f(197.0f, 10.0f);
//	glVertex2f(193.0f, 9.0f);
//	glVertex2f(197.0f, 10.0f);
//	glVertex2f(193.0f, 11.0f);
//	glVertex2f(197.0f, 10.0f);
//	glEnd();
//
//	lineWidth = 1.0f;
//	glLineWidth(lineWidth);
//	glBegin(GL_LINES);
//	for (i = 0; i < k; i++) {
//		glVertex2f(x[i], x0 + 3.0);
//		glVertex2f(x[i], x0);
//	}
//	for (i = 0; i < k; i++) {
//		glVertex2f(y0 + 3.0, y[i]);
//		glVertex2f(y0, y[i]);
//	}
//	glEnd();
//
//	string s;
//	for (i = 0; i < k; i++) {
//		s = num2string(20 * (i + 1));
//		glRasterPos2f(x[i] - 2.0f, 5.0f);
//		drawString(s);
//		s = num2string(10 * (i + 1));
//		glRasterPos2f(5.0f, y[i] - 1.0f);
//		drawString(s);
//	}
//}
//
//void scatter(GLfloat x0, GLfloat y0)
//{
//	const int n = 100;
//	int i, k = 5;
//	//随机生成n个点的x坐标和y坐标
//	int x_data[n], y_data[n];
//	for (i = 0; i < n; i++) {
//		x_data[i] = 1 + rand() % 101;  //x坐标的范围在【1，100】
//		y_data[i] = 1 + rand() % 51;   //y坐标的范围在【1，50】
//	}
//	//由坐标系坐标转成这个图的坐标
//	float x_real[n], y_real[n];
//	for (i = 0; i < n; i++) {
//		x_real[i] = x_data[i] * (200 - 2 * x0) / k / 20 + x0;
//		y_real[i] = y_data[i] * (150 - 2 * y0) / k / 10 + y0;
//	}
//
//	GLfloat pointSize = 5.0f;
//	glPointSize(pointSize);
//	glBegin(GL_POINTS);
//	glClear(GL_COLOR_BUFFER_BIT);
//	glColor3f(1.0f, 0.0f, 0.0f);
//	for (i = 0; i < n / 2; i++) {
//		glVertex2f(x_real[i], y_real[i]);
//	}
//	glClear(GL_COLOR_BUFFER_BIT);
//	glColor3f(0.0f, 0.0f, 1.0f);
//	for (i = n / 2; i < n; i++) {
//		glVertex2f(x_real[i], y_real[i]);
//	}
//	glEnd();
//}
//
//void Display()
//{
//	GLfloat x0 = 10.0f, y0 = 10.0f;
//	createCoordinate(x0, y0);
//	scatter(x0, y0);
//
//	glFlush();
//}
//
//void Initial()
//{
//	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  //清屏颜色
//	glMatrixMode(GL_PROJECTION);
//	gluOrtho2D(0.0, 200.0, 0.0, 150.0);   //投影到裁剪窗大小：世界
//}
//
//void testOpenGL(int argc, char** argv)
//{
//
//	glutInit(&argc, argv);
//	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
//	glutInitWindowSize(800, 500);
//	glutInitWindowPosition(300, 300);
//	glutCreateWindow("Scatter Plot");
//	glutDisplayFunc(Display);
//	Initial();
//	glutMainLoop();
//}
//
//
//void GeneratePic()
//{
//
//}
//导入两个xml解析其
int main12(int argc, char** argv)
{
	
	//判断两个文件的影像id是否一致，
	//map<int,int> IDMAPS; first为s3d的，second为mok的；
	//
	std::string xml_filepathmok = argv[1];//gcptest/block_AT-TY2500.xml";// "D:/TestData/testwu/cc.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
	BlockObject blockmokd,blockmokd1;
	auto atdatamok = std::make_shared<ATData>();
	auto atdatamok1 = std::make_shared<ATData>();
	blockmokd.LoadATXML(xml_filepathmok, atdatamok);
	blockmokd.SetATData(atdatamok);
	/*blockmokd.ExportATXML("D:/MyLearning/CHY/TEST/Proj/mok/1/Block_5/block_AT1.xml");
	blockmokd.ExportATBinary("D:/MyLearning/CHY/TEST/Proj/mok/1/Block_5/block_AT1.bin");
	blockmokd1.LoadATBinary("D:/MyLearning/CHY/TEST/Proj/mok/1/Block_5/block_AT1.bin",atdatamok1);
	blockmokd1.SetATData(atdatamok1);
	blockmokd1.ExportATXML("D:/MyLearning/CHY/TEST/Proj/mok/1/Block_5/block_AT21.xml");*/

	std::string xml_filepaths3d = argv[2];//gcptest/block_AT-TY2500.xml";// "D:/TestData/testwu/cc.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
	BlockObject blocks3d;
	auto atdatas3d = std::make_shared<ATData>();
	blocks3d.LoadATXML(xml_filepaths3d, atdatas3d);
	blocks3d.SetATData(atdatas3d);
	int cnt = 0;
	std::map<int, int> imgids;//前为mok后卫s3d
	for (auto iterimg : atdatas3d->GetImages())
	{
		auto img = atdatamok->FindImageWithName(iterimg.second.GetName(), atdatamok->GetImagesIds());
		if (img != nullptr)
		{
			imgids[img->GetImageId()] = iterimg.first;
		}
		/*if (atdatamok->GetImages().at(iterimg.first).GetName() != iterimg.second.GetName())
		{
			std::cout << iterimg.second.GetName() << std::endl;
			cnt++;
		}*/
	}

	for (auto& itergcp : blocks3d.GetCurrentATMutual()->GetControlPointsMutual())
	{
		for (auto& measure : itergcp.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
		{
			measure.image_id = imgids.at(measure.image_id);

		}
	}
	BlockObject::BlockExportOptions opt;
	opt.export_controlpoint_ = true;
	opt.export_not_registered_ = false;
	opt.export_tiepoint_ = true;
	/*opt.srs_*/
	blocks3d.ExportATXML(argv[3], opt);
	/*std::cout << cnt << std::endl;*/
	return 1;
}
//此实验结论 将数据转换到ecef下和enu下物方误差相近且与cc相近，但是4547的误差很小
int main90(int argc, char** argv)
{
	double x = 480218.423;
	double y = 2552098.301;
	double z = 13.393;
	CoordinateTransformer::Transform(1, &x, &y, &z, "EPSG:4547","EPSG:4978" );
	std::cout << std::setprecision(15) << " off " << x << " "<< y<<" "<< z << std::endl;
	std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> gcp_error_map, gcp_error_map1;
	std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > error_map;
	std::string xml_filepath_withgcp = argv[1];
	BlockObject block_withgcp;
	auto atdata_withgcp = std::make_shared<ATData>();

	block_withgcp.LoadATXML(xml_filepath_withgcp, atdata_withgcp);
	//atdata_withgcp->TransFormATData("ENU:23.06950,113.81530");
	auto images = atdata_withgcp->GetImagesMutual();
	auto cameras = atdata_withgcp->GetCamerasMutual();
	
	for (auto& iter : atdata_withgcp->GetControlPoints())
	{
		if (iter.second.GetName() == "0769P1-01-022-007")
		{
			auto gcp = iter.second;
			Eigen::Vector3d sum = Eigen::Vector3d::Zero();
			int cnt = 0;
			std::vector<Eigen::Matrix<float, 3, 4>> poses;
			std::vector<Eigen::Vector2f> points;
			for (auto& ele : gcp.GetObjectPointMutual().GetTrackMutual().GetElements())
			{
				Image image = images[ele.image_id];
				Eigen::Vector2d xy = ele.xy;
				Camera& camera = cameras[image.GetCameraId()];
				Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);
				
				points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });
				sum += image.GetPosition();
				cnt++;
			}
			Eigen::Vector3d position_offset = sum / cnt;
			/*position_offset.x() = -2370331.57225059;
			position_offset.y() = 5371726.81061911;
			position_offset.z() = 2483656.43081297;*/
			std::cout << std::setprecision(15) <<   " off " << position_offset << std::endl;
			for (auto& ele : gcp.GetObjectPointMutual().GetTrackMutual().GetElements())
			{
				Image image = images[ele.image_id] ;
				std::cout << std::setprecision(15) << ele.image_id << " " << image.GetPositionMutual() << std::endl;
				image.GetPositionMutual() -= position_offset;
			
				Camera& camera = cameras[image.GetCameraId()];
				poses.push_back((cameras[image.GetCameraId()].GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
				
			}
			Eigen::Vector3d xyz, xyzraw;
			xyzraw = gcp.GetObjectPoint().GetXYZ();
			Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);

			xyz = xyzf.cast<double>() + position_offset;
			double x = xyz.x();
			double y = xyz.y();
			double z = xyz.z();
			
			CoordinateTransformer::Transform(1, &x, &y, &z, atdata_withgcp->GetLocalSrs(), BASESRS);
			double xr = xyzraw.x();
			double yr = xyzraw.y();
			double zr = xyzraw.z();

			CoordinateTransformer::Transform(1, &xr, &yr, &zr, atdata_withgcp->GetLocalSrs(), BASESRS);

			std::cout << std::setprecision(15) << xyz << std::endl;
			
			double dx =x - xr;
			double dy = y -yr;
			double dz = z -zr;
			double  dxyz = std::sqrt(dx * dx + dy * dy + dz * dz);
			std::cout << std::setprecision(15) << dxyz << " "<< (xyz - gcp.GetObjectPoint().GetXYZ()).norm() << std::endl;
		}
	}
	return 0;
}
//20230801:需求 将一次空三的结果和另外已经刺好点的文件合并到一起
//param[1]:带刺点信息的xml文件
//param[2]:有空三结果的xml文件
//param[3]:输出文件
int GCP(int argc, char** argv)
{

	//判断两个文件的影像id是否一致，
	//map<int,int> IDMAPS; first为s3d的，second为mok的；
	//
	

	std::string xml_filepath_withgcp = argv[1];
	BlockObject block_withgcp;
	auto atdata_withgcp = std::make_shared<ATData>();

	block_withgcp.LoadATXML(xml_filepath_withgcp, atdata_withgcp);
	if (atdata_withgcp->GetNumControlPoints() <= 0)
	{
		std::cout << " no gcp in " << xml_filepath_withgcp << std::endl;
		return -1;
	}
	block_withgcp.SetATData(atdata_withgcp);



	std::string xml_filepath_target = argv[2];
	BlockObject block_target;
	auto atdata_target = std::make_shared<ATData>();
	block_target.LoadATXML(xml_filepath_target, atdata_target);
	block_target.SetATData(atdata_target);


	std::map<int, int> imgids_base_vs_gcp;//前为空三后为GCP的
	std::map<int, int> imgids_gcp_vs_base;//前为GCP后为空三的
	for (auto iterimg : atdata_target->GetImages())
	{
		auto img = atdata_withgcp->FindImageWithName(iterimg.second.GetName(), atdata_withgcp->GetImagesIds());
		if (img != nullptr)
		{
			 imgids_gcp_vs_base[img->GetImageId()] = iterimg.first;//先得到ids的对应表
			 imgids_base_vs_gcp[iterimg.first] = img->GetImageId();//先得到ids的对应表
		}
	}

	//去除控制点
	block_target.GetCurrentATMutual()->GetControlPointsMutual().clear();
	//获取原始控制点
	auto& gcps = atdata_withgcp->GetControlPointsMutual();

	for (auto& itergcp : gcps)
	{

		int id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
		if (id == kInvalidSrsId)//说明没有
		{
			//插入一个
			block_target.UpdateSRSMap(itergcp.second.GetSrs());
			id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
			//适应从Core导出的block_Absolute_xml中GCP没有srs_id的情况

		}
		itergcp.second.SetSrs(block_target.GetSRSsMutual()[id]);
		for (auto& measure : itergcp.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
		{
			measure.image_id = imgids_gcp_vs_base.at(measure.image_id);
		}
	}
	block_target.GetCurrentATMutual()->SetControlPoints(gcps);


	BlockObject::BlockExportOptions opt;
	opt.export_controlpoint_ = true;
	opt.export_not_registered_ = atdata_target->HasUnRegisteredImages();
	opt.export_tiepoint_ = true;
	/*opt.srs_*/
	block_target.ExportATXML(argv[3], opt);
	/*std::cout << cnt << std::endl;*/
	return 1;
}
//解决当时合并完之后空三过不去的问题
int main00(int argc, char** argv)
{

	//判断两个文件的影像id是否一致，
	//map<int,int> IDMAPS; first为s3d的，second为mok的；
	//
	ProjectObject project;

	std::string xml_filepath_withgcp = argv[1];//gcptest/block_AT-TY2500.xml";// "D:/TestData/testwu/cc.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
	BlockObject block_withgcp;
	auto atdata_withgcp = std::make_shared<ATData>();

	block_withgcp.LoadATXML(xml_filepath_withgcp, atdata_withgcp);
	//if (atdata_withgcp->GetNumControlPoints() <= 0)
	//{
	//	std::cout << " no gcp in " << xml_filepath_withgcp << std::endl;
	//	return -1;
	//}
	block_withgcp.SetATData(atdata_withgcp);
	
	
		ATData::SimplifyOptions simopts;
		simopts.max_overlap_ = -1;
		simopts.min_overlap_ = 3;
		simopts.max_proj_error_ = 1.2;
		simopts.max_tiepoint_count_ = atdata_withgcp->GetPoints3D().size() - 5000/*200000*/;
	
	BlockObject::BlockExportOptions opt;
	opt.export_controlpoint_ = true;
	opt.export_not_registered_ = true;
	opt.export_tiepoint_ = true;

	block_withgcp.GetCurrentATMutual()->Simplify(simopts);
	block_withgcp.ExportATXML(xml_filepath_withgcp+"_w.xml", opt);

	project.AddBlock(&block_withgcp);
	std::string xml_filepath_target = argv[2];
	BlockObject block_target;
	auto atdata_target = std::make_shared<ATData>();
	block_target.LoadATXML(xml_filepath_target, atdata_target);
	block_target.SetATData(atdata_target);

	/*block_target.GetCurrentATMutual()->Simplify(simopts);
	block_target.ExportATXML(xml_filepath_target + "_20w.xml", opt);*/

	project.AddBlock(&block_target);
	std::set<block_t> ids;
	ids.insert(block_withgcp.GetId());
	ids.insert(block_target.GetId());
	project.MergeBlocks(ids);
	auto MergeBlockId = *project.GetBlockIds().rbegin();
	AI3D::CORE::BlockObject* MergeBlock = project.GetBlock(MergeBlockId);
	MergeBlock->ExportATXML(xml_filepath_withgcp + "_merge.xml", opt);
	int cnt = 0;

	std::map<int, int> imgids;//前为mok后卫s3d
	for (auto iterimg : atdata_target->GetImages())
	{
		auto img = atdata_withgcp->FindImageWithName(iterimg.second.GetName(), atdata_withgcp->GetImagesIds());
		if (img != nullptr)
		{
			imgids[img->GetImageId()] = iterimg.first;
		}
	}

	//去除控制点
	block_target.GetCurrentATMutual()->GetControlPointsMutual().clear();
	//获取原始控制点
	auto& gcps = atdata_withgcp->GetControlPointsMutual();

	for (auto& itergcp : gcps)
	{

		int id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
		if (id == kInvalidSrsId)//说明有
		{
			//插入一个
			block_target.UpdateSRSMap(itergcp.second.GetSrs());
			id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
			//适应从Core导出的block_Absolute_xml中GCP没有srs_id的情况

		}
		itergcp.second.SetSrs(block_target.GetSRSsMutual()[id]);
		for (auto& measure : itergcp.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
		{
			measure.image_id = imgids.at(measure.image_id);
		}
	}
	block_target.GetCurrentATMutual()->SetControlPoints(gcps);


	//BlockObject::BlockExportOptions opt;
	opt.export_controlpoint_ = true;
	opt.export_not_registered_ = false;
	opt.export_tiepoint_ = true;
	/*opt.srs_*/
	block_target.ExportATXML(argv[3], opt);
	/*std::cout << cnt << std::endl;*/
	return 1;
}
//还要统计一下重叠影像的位置误差
#include "Core/S3DProjectFile.h"
int main77(int argc, char** argv)
{
	std::string basepath = "D:/TestData/S3DResult/13wan/partition/";
	//读取txt的点云信息；
	std::vector<Eigen::Vector3d> pt7, pt2,pts;
	pt7.reserve(100000);
	pt2.reserve(100000);
	std::string path7,path2;
	path7 = basepath + "structure_refine_m_7 - Cloud.txt";
	path2 = basepath + "structure_refine_m_2 - Cloud.txt";
	std::ifstream file7 = File::OpenIfstreamUtf8(path7, std::ios::in);
	
	std::string line;
	std::string item;
	while (std::getline(file7, line))
	{

		std::stringstream line_stream1(line);
		Eigen::Vector3d pt;
	
		std::getline(line_stream1, item, ' ');			
		pt(0) = std::stold(item);

		std::getline(line_stream1, item, ' ');
		pt(1) = std::stold(item);

		std::getline(line_stream1, item, ' ');
		pt(2) = std::stold(item);
		pt7.push_back(pt);

	}
	file7.close();
	std::ifstream file2 = File::OpenIfstreamUtf8(path2, std::ios::in);
	while (std::getline(file2, line))
	{

		std::stringstream line_stream1(line);
		Eigen::Vector3d pt;

		std::getline(line_stream1, item, ' ');
		pt(0) = std::stold(item);

		std::getline(line_stream1, item, ' ');
		pt(1) = std::stold(item);

		std::getline(line_stream1, item, ' ');
		pt(2) = std::stold(item);
		pt2.push_back(pt);

	}
	file2.close();
	std::vector<Eigen::Vector3d> allpts;
	allpts.insert(allpts.end(), pt2.begin(), pt2.end());
	allpts.insert(allpts.end(), pt7.begin(), pt7.end());
	bbox_s box_;
	box_.xmin_ = DBL_MAX;
	box_.xmax_ = -DBL_MAX;
	box_.ymin_ = DBL_MAX;
	box_.ymax_ = -DBL_MAX;
	box_.zmin_ = DBL_MAX;
	box_.zmax_ = -DBL_MAX;
	for (auto& it : allpts)
	{

		Eigen::Vector3d point = it;
		
		if (point(0) > box_.xmax_)
		{
			box_.xmax_ = point(0);
		}
		if (point(0) < box_.xmin_)
		{
			box_.xmin_ = point(0);
		}
		if (point(1) > box_.ymax_)
		{
			box_.ymax_ = point(1);
		}
		if (point(1) < box_.ymin_)
		{
			box_.ymin_ = point(1);
		}
		if (point(2) > box_.zmax_)
		{
			box_.zmax_ = point(2);
		}
		if (point(2) < box_.zmin_)
		{
			box_.zmin_ = point(2);
		}
	}
	Eigen::Vector3d offset{ box_.xmin_,box_.ymin_,box_.zmin_ };
	for (int j = 0; j < pt7.size(); j++)
	{
		pt7[j] -= offset;
	}
	for (int i = 0; i < pt2.size(); i++)
	{
		pt2[i] -= offset;
		
	}
	box_.xmin_ -= offset.x();
	box_.ymin_ -= offset.y();
	box_.zmin_ -= offset.z();
	box_.xmax_ -= offset.x();
	box_.ymax_ -= offset.y();
	box_.zmax_ -= offset.z();
	

	std::set<int> sets;
	int width = 200;
	int col = std::ceil(box_.xmax_ - box_.xmin_) / width;
	std::map<int, std::vector<int> > pt2grid, pt7grid;
	for (int i = 0; i < pt2.size(); i++)
	{
		int gridinx = col * (pt2[i].y() / width) + pt2[i].x() / width;;
		pt2grid[gridinx].push_back(i);
	}
	for (int i = 0; i < pt7.size(); i++)
	{
		int gridinx = col * (pt7[i].y() / width) + pt7[i].x() / width;;
		pt7grid[gridinx].push_back(i);
	}
	
	for (auto& iter : pt2grid)
	{
		if (pt7grid.count(iter.first))
		{
			for (std::vector<int>::iterator iter1 = iter.second.begin();
				iter1 != iter.second.end(); iter1++)
			//for (auto& iter1 : iter.second)
			{
				for (std::vector<int>::iterator iter2 = pt7grid.at(iter.first).begin();
					iter2 != pt7grid.at(iter.first).end(); iter2++)
				//for (auto& iter2 : pt7grid.at(iter.first))
				{
					if (pt2[*iter1].x() != pt7[*iter2].x())
					{
						continue;
					}
					if (pt2[*iter1].y() != pt7[*iter2].y())
					{
						continue;
					}
					if (pt2[*iter1].z() != pt7[*iter2].z())
					{
						continue;
					}
				
					/*if (abs(pt2[*iter1].x() - pt7[*iter2].x()) < 0.000001 &&
						abs(pt2[*iter1].y() - pt7[*iter2].y()) < 0.000001 && abs(pt2[*iter1].z() - pt7[*iter2].z()) < 0.000001)*/
					{
						pts.push_back(pt7[*iter2]);
						/*pt7grid.at(iter.first).erase(iter2);
						pt2grid.at(iter.first).erase(iter1);*/
						break;
					}
					
				}
			}
		}
	}

	//for (int i = 0; i < pt2.size(); i++)
	//{

	//	for (int j = 0; j < pt7.size(); j++)
	//	{
	//		if (abs(pt2[i].x() - pt7[j].x()) < 0.000001&&
	//			abs(pt2[i].y() - pt7[j].y()) < 0.000001&& abs(pt2[i].z() - pt7[j].z()) < 0.000001)
	//		{
	//			/*if (sets.count(i) > 0)
	//			{
	//				
	//			}*/
	//			/*std::cout<<std::setprecision(18) << pt2[i] << std::endl;
	//			std::cout << std::setprecision(18) << pt7[j] << std::endl;*/
	//			pts.push_back(pt7[j]);
	//			break;
	//		}
	//	}
	//}
	std::string path = basepath + "gcp.txt";
	std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc);
	
	{
		// Ensure that we don't loose any precision by storing in text.
		file.precision(17);


		for ( auto& pt : pts) {
			std::ostringstream line;
			line.precision(17);
			pt+= offset;
			line << pt.x()<< " "<< pt.y()<<" " << pt.z();
			std::string line_string = line.str();
			file << line_string << std::endl;
		}
		file.close();
	}
	return 0;
	std::vector<BlockObject> blocks;
	
	std::string basestr = "sfm_data_n_";
	BlockObject::BlockExportOptions opto;
	opto.srs_ = CoordinateDescriptor::GetSRSFromDefinition(BASESRS);
	opto.srs_.ID = 0;

	/*AI3D::SMT3D::S3DJSON json;
	
	std::string file1 =  "D:/TestData/S3DResult/13wan/cam/part1/initial_sfm_data91.json";
	json.load(file1);
	AI3D::CORE::BlockObject BK(basepath);
	json.ConvertSMT3DToATData(BK);
	BK.ExportATXML(file1 + ".xml", opto);*/
	/*for (int i = 0; i < 8; i++)
	{
		BlockObject block,block1;
		std::string atxm = basepath + basestr + std::to_string(i) + ".json.xml";
		auto atdata = std::make_shared<ATData>();
		block.LoadATXML(atxm, atdata);
		block.SetATData(atdata);
		for (auto& iter : block.GetCurrentATMutual()->GetImages())
		{
			if (!iter.second.IsRegistered())
			{
				block.GetCurrentATMutual()->DeleteImage(iter.first);
			}
		}
		block.ExportATXML(basepath + basestr + std::to_string(i) + "_onlyregistere.xml", opto);
		blocks.push_back(block);
		
	}*/


	/*
	for (int i = 0; i < blocks.size()-1; i++)
	{

		for (int j = i+1; j < blocks.size() ; j++)
		{
			BlockObject block1, block2;
			blocks[i].MakeNewBlockForCommonImages(blocks[j],block1,block2);
			if(block1.GetCurrentATMutual()->HasImages())
				block1.ExportATXML(basepath+ basestr + std::to_string(i)+"_"+ std::to_string(j) + ".xml", opto);
			if (block2.GetCurrentATMutual()->HasImages())
			block2.ExportATXML(basepath + basestr + std::to_string(j) + "_" + std::to_string(i) + ".xml", opto);
		}
	}
	for (int i = 0; i < 8; i++)
	{
		AI3D::SMT3D::S3DJSON json;
		std::string file1 = basepath + "sfm_data_c_" + std::to_string(i) + ".json";
		json.load(file1);
		AI3D::CORE::BlockObject BK(basepath);
		json.ConvertSMT3DToATData(BK);
		BK.ExportATXML(file1+".xml", opto);

		AI3D::SMT3D::S3DJSON jsonn;
		std::string filen = basepath + "sfm_data_n_" + std::to_string(i) + ".json";
		jsonn.load(filen);
		AI3D::CORE::BlockObject BKn(basepath);
		jsonn.ConvertSMT3DToATData(BKn);
		BKn.ExportATXML(filen + ".xml", opto);
	}*/
	return 0;

	/*std::string atxml22 = "E:/0109/01057-2.6.xml";
	BlockObject block122;
	auto atdata122 = std::make_shared<ATData>();
	block122.LoadATXML(atxml22, atdata122);*/

	
	
	
	//std::string atbin11 = "E:/Gcpyanzheng/Block_13/Block_13.bin";
	//std::string atbin1 = "E:/Gcpyanzheng/Block_13/SCSFR.bin";// /SCSFR.bin.bak gcptest/block_AT-TY2500.xml";// "D:/TestData/testwu/cc.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
	//std::string parentdir; std::string lastdir;
 //	//File::GetLastSecondDir(atbin1, parentdir, lastdir);
	BlockObject block1,block2;
	//auto atdata1 = std::make_shared<ATData>();
	//auto atdata11 = std::make_shared<ATData>();
	///*block1.LoadATBinaryWithoutTiepoints(atbin11, atdata11);
	//std::string tiepoints_file_path = "D:/MyLearning/CHY/TEST/Proj/testat/Block_6/Tiepoints.bin";*/
	///*block1.LoadTiepointsBinary(tiepoints_file_path, atdata11);
	//block1.GetATData()->TriangulateTiePoints();*/
	//BlockObject::BlockExportOptions opt1;
	//opt1.export_tiepoint_ = true;
	//opt1.srs_  = block1.GetBlockSRS();
	////opt1.srs_ = CoordinateDescriptor::GetSRSFromDefinition();
	//opt1.srs_.ID = 0;
	//block1.LoadATBinary(atbin1, atdata1);
	//
	////转换坐标 
	///*block1.GetATData()->GetPoints3DMutual() = atdata1->GetPoints3DMutual();
	//block1.GetATData()->TransFormTiepoints(atdata1->GetLocalSrs(), block1.GetATData()->GetLocalSrs());*/
	//block1.ExportATDataToXML("D:/MyLearning/CHY/TEST/Proj/testat/Block_6/input-notiepoint.xml", opt1, *block1.GetATData().get());
	////
	//////以上为恢复原始的空三结果的代码

	////block1.UpdateATGroup(atdata1);
	////block1.SetATData(atdata1);
	////经测试4978转4326没问题
	std::string rawsrs = "ENU:25.669690,114.733240";// 
	rawsrs = "EPSG:4544";
	//
	
	//block1.ExportATXML("D:/MyLearning/CHY/TEST/Proj/testat/Block_6/block_AT_bin4547.xml", opt1);
	std::map<point3D_t,std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> gcp_error_map, gcp_error_map1;
	//std::string current_srs = atdata1->GetLocalSrs();
	//atdata1->UpdataGCPGlobalErrorInfo(gcp_error_map, current_srs);
	std::string atbin2 = "E:/Gcpyanzheng/Block_18/block_AT_absolute.xml";//
	auto atdata2 = std::make_shared<ATData>();
	block2.LoadATXML(atbin2, atdata2);
	block2.SetATData(atdata2);
	std::string current_srs1 = atdata2->GetLocalSrs();
	atdata2->UpdataGCPGlobalErrorInfo(gcp_error_map1);
	/*std::string atxm2 = "D:/MyLearning/CHY/TEST/Proj/testat/Block_6/block_AT_bin.xml";
	std::string atxm1 = "D:/MyLearning/CHY/TEST/Proj/testat/Block_6/block_AT_bin1.xml";*/
	
	//block1.ExportATXML(atxm2,opt1);
	//block2.ExportATXML(atxm1, opt1);
	// 
	
	/*atdata2->TransFormImages(current_srs1, "EPSG:4547");
	atdata2->TransFormTiepoints(current_srs1, rawsrs);
	atdata2->TransformControlPoints(rawsrs);
	atdata2->SetLocalGcpSrs(rawsrs);
	atdata2->SetLocalSrs(rawsrs);*/
	EIGEN_STL_UMAP(point3D_t, class ControlPoint) gcps11= atdata2->GetControlPointsMutual();
	std::string outfile = "E:/Gcpyanzheng/Block_13/export_4544-010-022.txt";
	std::ofstream outfilestr = File::OpenOfstreamUtf8(outfile, std::ios::out);
	int count = 0;
	for (auto iter : gcps11)
	{
		if (iter.second.GetId() == 5)
		{
			for (auto iter1 : iter.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
			{
				image_t imageid=iter1.image_id;
				auto image = atdata2->GetImages().at(imageid);
				outfilestr << std::setprecision(16)<<count<< " "<<  iter1.xy.x() << " " << iter1.xy.y() << " " << image.GetName() << " "
					<< imageid << " " << image.GetPosition().x() << " " << image.GetPosition().y() << " "
					<< image.GetPosition().z() << " " << image.GetRotationMatrix() << "\n";
				count++;
			}
		}
	}
	outfilestr.close();
	
	//atdata1->TransFormImages("EPSG:4547", current_srs1);
	//atdata1->TransFormTiepoints("EPSG:4547", current_srs1);

	//atdata1->TransformControlPoints(current_srs1);
	//atdata1->SetLocalGcpSrs(current_srs1);
	//atdata1->SetLocalSrs(current_srs1);
	//atdata1->UpdataGCPGlobalErrorInfo(gcp_error_map1, current_srs1);

	//判断两个文件的影像id是否一致，
	//map<int,int> IDMAPS; first为s3d的，second为mok的；
	//
	std::string xml_filepath_withgcp =  argv[1];//gcptest/block_AT-TY2500.xml";// "D:/TestData/testwu/cc.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
	BlockObject block_withgcp;
	auto atdata_withgcp = std::make_shared<ATData>();
	
	block_withgcp.LoadATXML(xml_filepath_withgcp, atdata_withgcp);



	if (atdata_withgcp->GetNumControlPoints() <= 0)
	{
		std::cout << " no gcp in " << xml_filepath_withgcp << std::endl;
		return -1;
	}
	block_withgcp.SetATData(atdata_withgcp);
	
	
	std::string xml_filepath_target = argv[2];
	BlockObject block_target;
	auto atdata_target = std::make_shared<ATData>();
	block_target.LoadATXML(xml_filepath_target, atdata_target);
	block_target.SetATData(atdata_target);
	int cnt = 0;

	std::map<int, int> imgids;//前为mok后卫s3d
	for (auto iterimg : atdata_target->GetImages())
	{
		auto img = atdata_withgcp->FindImageWithName(iterimg.second.GetName(), atdata_withgcp->GetImagesIds());
		if (img != nullptr)
		{
			imgids[img->GetImageId()] = iterimg.first;
		}
	}

	//去除控制点
	block_target.GetCurrentATMutual()->GetControlPointsMutual().clear();
	//获取原始控制点
	auto& gcps = atdata_withgcp->GetControlPointsMutual();
	
	for (auto& itergcp : gcps)
	{
		
		int id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
		if (id== kInvalidSrsId)//说明有
		{
			//插入一个
			block_target.UpdateSRSMap(itergcp.second.GetSrs());
			id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
			//适应从Core导出的block_Absolute_xml中GCP没有srs_id的情况
			
		}
		itergcp.second.SetSrs(block_target.GetSRSsMutual()[id]);
		for (auto& measure : itergcp.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
		{
			measure.image_id = imgids.at(measure.image_id);
		}
	}
	block_target.GetCurrentATMutual()->SetControlPoints(gcps);
	
	
	BlockObject::BlockExportOptions opt;
	opt.export_controlpoint_ = true;
	opt.export_not_registered_ = false;
	opt.export_tiepoint_ = true;
	/*opt.srs_*/
	block_target.ExportATXML(argv[3], opt);
	/*std::cout << cnt << std::endl;*/
	return 1;
}
//param[0]:pos文件的；
//param[1]:影像路径
//param[2]:输出路径;
//param[3] : 经维度顺序

int main0(int argc, char** argv)
{
	if (argc != 5)
	{
		std::cout << " params ara wrong,should be,参数不对，共四个参数，应该如下: \n \
			1:pos file ,like D:/POS.TXT;\n \
                  pos文件中先影像名再坐标值，空格隔开，第一列为影像名，可以是全路径如 E:/1.jpg,也可以是影像的名字 1.jpg,也可以不带.jpg后缀 \n \
            2:image path,影像目录，可以包含子目录\n \
			3:output path ;输出目录 \n \
			4: 0或1，0代表pos文件中的顺序是经纬高，1代表纬经高 " << std::endl;
		return 0;
	}
	int order = std::atoi(argv[4]);
	//读pos
	std::vector<pose_s>  poses;
	std::string posfile( argv[1]);
	posfile = (File::EnsureUnifySlash(posfile));
	std::ifstream file = File::OpenIfstreamUtf8(posfile, std::ios::in);

	std::string line;
	std::string item;

	while (std::getline(file, line)) 
	{
		AI3D::CORE::String::StringTrim(&line);

		if (line.empty() || line[0] == '#') {
			continue;
		}
		pose_s pose;

		std::stringstream line_stream(line);
		std::getline(line_stream, item, ' ');
		pose.name = String::Utf8ToLocale(item);
		
		
		// 经纬高
		std::getline(line_stream, item, ' ');
		pose.metadata_.center(0) = std::stold(item);

		std::getline(line_stream, item, ' ');
		pose.metadata_.center(1) = std::stold(item);

		std::getline(line_stream, item, ' ');
		pose.metadata_.center(2) = std::stold(item);

		if (order == 1)
		{
			auto temp = pose.metadata_.center(0);
			pose.metadata_.center(0) = pose.metadata_.center(1);
			pose.metadata_.center(1) = temp;
		}
		poses.push_back(pose);
	}
	
	//
	BlockObject block;
	auto atdata1 = std::make_shared<ATData>();
	std::string inpath(argv[2]);
	inpath = File::EnsureTrailingSlash(File::EnsureUnifySlash(inpath));
	std::string outpath(argv[3]);
	 outpath = File::EnsureTrailingSlash(File::EnsureUnifySlash(outpath));
	std::vector<std::string> image_extesion = { ".jpg", ".jpeg" ,".png" ,".tiff" ,".tif" };
	File::CreateDirIfNotExists(outpath);
	
	std::vector<std::string> filenames;
	
	block.SearchImages(inpath, filenames, image_extesion);
			
	for (auto& iter : filenames)
	{
		std::string image_path(iter);
		image_path = (File::EnsureUnifySlash(image_path));
	
		auto iterpos = std::find_if(poses.begin(), poses.end(), [&](const pose_s element)
		//for (auto& element : poses)
		{
			std::string name(element.name);

			name = File::EnsureUnifySlash(name);
			String::StringToLower(&name);
			bool bFullname = false;
			if (name.find_last_of("/") != std::string::npos)
			{
				bFullname = true;
			}
			std::string findname, posename;
			if (bFullname)
			{
				findname = image_path;
				String::StringToLower(&findname);
				posename = name;
			}
			else
			{
				findname = File::GetFileNameWithoutExtension(image_path);
				String::StringToLower(&findname);
				posename = File::GetFileNameWithoutExtension(name);
			}
		/*	if (findname == posename)
			{
				break;
			}
		}*/
				
				return findname == posename; });
		
		if (iterpos == poses.end())
		{
			continue;
		}
		//先拷贝
		std::string lowimagepath = image_path;
		
		String::StringToLower(&lowimagepath);
		std::string oldstr = inpath;
		
		String::StringToLower(&oldstr);
		//outpath = File::EnsureTrailingSlash(File::EnsureUnifySlash(outpath));
		std::string newname = String::StringReplace(lowimagepath, oldstr, outpath);
		newname = File::EnsureUnifySlash(newname);
		
		File::CreateDirIfNotExists(File::GetParentDir(newname));
		std::vector<std::string> fileimg(1, image_path);
		if (!File::CopyFiles(fileimg, File::GetParentDir(newname), false))
			continue;

		ExifIO exif;
		exif.Open(newname);
		double alt = iterpos->metadata_.center(2);
		double lon = iterpos->metadata_.center(0);
		double lat= iterpos->metadata_.center(1);
		/*std::string str = "46;34;19.291100000";
		Exiv2::ExifKey tmp = Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude");
		Exiv2::ExifData::iterator pos = exif.GetImagePtr()->exifData().findKey(tmp);

		
		double deg[3];
		deg[0] = 46;
		deg[1] = 34;
		deg[2] = 19.29220;
		double min = deg[0] * 60.0 + deg[1] + deg[2] / 60.0;
		int ideg = static_cast<int>(min / 60.0);
		min -= ideg * 60;
		std::ostringstream oss;
		oss << ideg << ","
			<< std::fixed << std::setprecision(7) << deg[0] << ";"<< deg[1]<<";"<<deg[2]			;*/
		exif.SetLatitude(lat);
		exif.SetAltitude(alt);
		exif.SetLongitude(lon);
		
		exif.Write();
	
	}
	return 1;
}
std::string GetExifValueString(Exiv2::ExifData& ed, std::string key)
{
	Exiv2::ExifKey tmp = Exiv2::ExifKey(key);
	Exiv2::ExifData::iterator pos = ed.findKey(tmp);
	if (pos == ed.end())
		return "";
	return pos->value().toString();
}

bool Addimageslist(const std::vector<std::string>& images) {
	for (int i_img = 0; i_img < images.size(); i_img++)
	{
		std::string imageFullpath = images[i_img];

		//1.判断图片是否存在
		bool isExist = false;

		imageFullpath = File::EnsureUnifySlash(imageFullpath);

		//2.初始化图片id，获取图片文件夹名称和类型名称，解析图片获取相机信息
		Image img;
		img.SetImageId(i_img);
		img.SetPath(File::GetParentDir(imageFullpath));
		img.SetName(File::GetPathBaseName(imageFullpath));

		//设置相机信息
		ExifInfo exif;
		std::string pathtemp ="C:\\Users\\Microsoft\\Downloads\\TestImg";

		std::string preview_path = File::JoinPaths(pathtemp, "previews");
		File::CreateDirIfNotExists(preview_path);
		//解析影像获取相机信息
		if (img.ParseExif(preview_path) == ERROR_IMAGE)
		{
			LOGW(String::StringPrintf("Invalid image %s", imageFullpath.c_str()));

			continue;
		}
		exif = img.GetExifinfo();
		std::cout << "--------width: " << exif.width << std::endl;
		std::cout << "--------heith: " << exif.height << std::endl;

	}
	return true;
}

void readExifInThread(const std::string& imageFullpath) {
	try {
		// 打开图片
		auto imgPtr = Exiv2::ImageFactory::open(imageFullpath);
		if (!imgPtr ) {
			std::cout << "Failed to open image or IO object is null: " << imageFullpath << std::endl;
			return;
		}

		// 读取元数据
		imgPtr->readMetadata();

		// 获取 EXIF 数据
		Exiv2::ExifData& ed = imgPtr->exifData();
		if (ed.empty()) {
			std::cout << "No EXIF data in image: " << imageFullpath << std::endl;
		}
		else {
			std::cout << "Successfully read EXIF from: " << imageFullpath << std::endl;
			// 可以遍历 EXIF 数据
			for (const auto& kv : ed) {
				std::cout << kv.key() << " = " << kv.value() << std::endl;
			}
		}

	}
	catch (Exiv2::Error& e) {
		std::cout << "Exiv2 error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "Unknown error reading EXIF from: " << imageFullpath << std::endl;
	}
}

int main(int argc, char** argv)
{
	/*Exiv2::Image::UniquePtr image_;
	std::string sFileName = "C:\\Users\\Microsoft\\Downloads\\stone\\stone\\JPEG\\IMG_4440.jpg";
	try
	{
		image_ = Exiv2::ImageFactory::open(sFileName);
	}
	catch (Exiv2::Error& e)
	{
		std::cout << "Caught Exiv2 exception " << e.what() << std::endl;
		return ERROR_IMAGE;
	}


	if (image_.get() == 0)
	{
		std::cout << "Exif is empty " << std::endl;
		return NOEXIF_IMAGE;
	}

	std::cout << "-------------1 " << std::endl;
	image_->readMetadata();
	std::cout << "-------------2 " << std::endl;
	Exiv2::ExifData ed = image_->exifData();
	std::cout << "-------------3 " << std::endl;

	if (ed.empty())
	{
		std::cout << "has no exif file " << std::endl;
		return NOEXIF_IMAGE;
	}
	else {
		std::string makeStr = GetExifValueString(ed, "Exif.Image.Make");
		std::cout << "--------makeStr: " << makeStr << std::endl;
	}*/


	//std::string sFileName = "C:\\Users\\Microsoft\\Downloads\\stone\\stone\\JPEG\\IMG_4440.jpg";
	//std::string sFileName1 = "C:\\Users\\Microsoft\\Downloads\\herb\\herb\\DATA\\Cam01_0070.jpg";
	//解析影像获取相机信息
	//Image img1;
	//img1.SetImageId(1);
	//img1.SetPath(File::GetParentDir(sFileName1));
	//img1.SetName(File::GetPathBaseName(sFileName1));
	//ExifInfo exif1;
	//if (img1.ParseExif(sFileName1) == ERROR_IMAGE)
	//{
	//	std::cout << "--------ivalid image: "<< sFileName1  << std::endl;
	//	return -1;
	//}
	//exif1 = img1.GetExifinfo();
	//std::cout << "--------width: " << exif1.width << std::endl;
	//std::cout << "--------heith: " << exif1.height << std::endl;

	//std::string sFileName2 = "C:\\Users\\Microsoft\\Downloads\\stone\\stone\\JPEG\\IMG_4440.jpg";
	////解析影像获取相机信息
	//Image img2;
	//img2.SetImageId(1);
	//img2.SetPath(File::GetParentDir(sFileName2));
	//img2.SetName(File::GetPathBaseName(sFileName2));
	//ExifInfo exif2;
	//if (img2.ParseExif(sFileName2) == ERROR_IMAGE)
	//{
	//	std::cout << "--------ivalid image: " << sFileName2 << std::endl;
	//	return -1;
	//}
	//exif2 = img2.GetExifinfo();
	//std::cout << "--------width: " << exif2.width << std::endl;
	//std::cout << "--------heith: " << exif2.height << std::endl;

	//std::vector<std::string> filenames;
	//std::string sFileName1 = "C:\\Users\\Microsoft\\Downloads\\stone\\stone\\JPEG\\IMG_4440.jpg";
	//std::string sFileName2 = "C:\\Users\\Microsoft\\Downloads\\herb\\herb\\DATA\\Cam01_0070.jpg";
	//filenames.push_back(sFileName1);
	//filenames.push_back(sFileName2);

	//bool result = Addimageslist(filenames);
	//if (!result) {
	//	return -1;
	//}
	std::string imageFullpath = "C:\\Users\\Microsoft\\Downloads\\stone\\stone\\JPEG\\IMG_4440.jpg";

	std::thread t(readExifInThread, imageFullpath);
	t.join();

	std::cout << "Thread finished." << std::endl;
	return 0;


	//return 1;
}