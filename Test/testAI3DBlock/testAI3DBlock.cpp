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
#include "libraw/libraw.h"
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
//???????????
// ????????????????id


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
////    imagefiles_.insert(std::pair<std::string, image_t >(file1,100 ));//??????
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
////??????????
//bool testDeleteImage(BlockObject block, image_t id)
//{
//	return true;
//};
//
////?????
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
//	//???????????
//	HRESULT h = CoCreateGuid(&guid);
//	//?????????????????????
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
///// @brief ??????????????????MAC???
///// @remark ????????????????????????OS?????MAC???????????????
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
///// @brief ???? boost::regex?????MAC
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
//	//?????????MAC?????????
//	SECURITY_ATTRIBUTES sa;
//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//	//???????
//	HANDLE hReadPipe, hWritePipe;
//	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
//	{
//		//?????????????????
//		STARTUPINFO si;
//		//??????????
//		PROCESS_INFORMATION pi;
//		si.cb = sizeof(STARTUPINFO);
//		GetStartupInfo(&si);
//		si.hStdError = hWritePipe;
//		si.hStdOutput = hWritePipe;
//		si.wShowWindow = SW_HIDE; //??????????????
//		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//
//		//?????????????????
//		TCHAR szCommandLine[] = TEXT("ipconfig /all");
//		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
//		{
//			WaitForSingleObject(pi.hProcess, 3000); // ?????????????Vista??Win7???????????
//			unsigned long count;
//			CloseHandle(hWritePipe);
//			std::string strBuffer(1024 * 10, '\0'); // ?????????????
//			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
//			{
//				strBuffer.resize(strBuffer.find_first_of('\0')); // ?????????????????'\0'
//				ret = ParseMac(strBuffer, macOUT);//???MAC?????
//			}
//			CloseHandle(pi.hThread);
//			CloseHandle(pi.hProcess);
//		}
//		else
//		{
//			CloseHandle(hWritePipe); // VS2010???????????????An invalid handle was specified??????????????????????????????VS2008????????
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
//	//?????????MAC?????????
//	SECURITY_ATTRIBUTES sa;
//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//	//???????
//	HANDLE hReadPipe, hWritePipe;
//	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
//	{
//		//?????????????????
//		STARTUPINFO si;
//		//??????????
//		PROCESS_INFORMATION pi;
//		si.cb = sizeof(STARTUPINFO);
//		GetStartupInfo(&si);
//		si.hStdError = hWritePipe;
//		si.hStdOutput = hWritePipe;
//		si.wShowWindow = SW_HIDE; //??????????????
//		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//
//		//?????????????????
//		TCHAR szCommandLine[] = TEXT("wmic csproduct list full");
//		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
//		{
//			WaitForSingleObject(pi.hProcess, 3000); // ?????????????Vista??Win7???????????
//			unsigned long count;
//			CloseHandle(hWritePipe);
//			std::string strBuffer(1024 * 10, '\0'); // ?????????????
//			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
//			{
//				strBuffer.resize(strBuffer.find_first_of('\0')); // ?????????????????'\0'
//				ret = ParseUUID(strBuffer, uuidOUT);//???MAC?????
//			}
//			CloseHandle(pi.hThread);
//			CloseHandle(pi.hProcess);
//		}
//		else
//		{
//			CloseHandle(hWritePipe); // VS2010???????????????An invalid handle was specified??????????????????????????????VS2008????????
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
//	//?????????MAC?????????
//	SECURITY_ATTRIBUTES sa;
//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//	//???????
//	HANDLE hReadPipe, hWritePipe;
//	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
//	{
//		//?????????????????
//		STARTUPINFO si;
//		//??????????
//		PROCESS_INFORMATION pi;
//		si.cb = sizeof(STARTUPINFO);
//		GetStartupInfo(&si);
//		si.hStdError = hWritePipe;
//		si.hStdOutput = hWritePipe;
//		si.wShowWindow = SW_HIDE; //??????????????
//		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//
//		//?????????????????
//		TCHAR szCommandLine[] = TEXT("wmic diskdrive get serialnumber");
//		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
//		{
//			WaitForSingleObject(pi.hProcess, 3000); // ?????????????Vista??Win7???????????
//			unsigned long count;
//			CloseHandle(hWritePipe);
//			std::string strBuffer(1024 * 10, '\0'); // ?????????????
//			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
//			{
//				strBuffer.resize(strBuffer.find_first_of('\0')); // ?????????????????'\0'
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
//			CloseHandle(hWritePipe); // VS2010???????????????An invalid handle was specified??????????????????????????????VS2008????????
//		}
//		CloseHandle(hReadPipe);
//	}
//
//	return ret;
//}
//
//
//void testBlock()
//{
//	cv::Mat mat = cv::imread("X:/bug/DSC01264.png");
//	//std::string buffer = "30-9C-23-B7-07-BA";
//	//std::string macOUT;
//	//std::string uuidOUT;
//	//std::string diskSerialNUm;
//	////std::cout << ParseMac(buffer, macOUT) << std::endl;
//	//GetMacByCmd(macOUT);
//	//std::cout << macOUT << std::endl;
//	//GetUUIDByCmd(uuidOUT);
//	//std::cout << uuidOUT << std::endl;
//	//GetDiskSerialNumByCmd(diskSerialNUm);
//	//std::cout << diskSerialNUm << std::endl;
//	//system("pause");
//	//return;
//
//	//boost::uuids::uuid uid = boost::uuids::random_generator()();
//	//const std::string uid_str = boost::uuids::to_string(uid);
//	//std::cout << uid_str << std::endl;
//	std::string reportjson_GPS = "X:/Projects/test_V833449/Block_2/ATreport.json";
//	std::string reportjson_GCP = "X:/Projects/test_V833449/Block_4/ATreport.json";
//	std::string reportjson_html = "X:/Projects/test_V833449/Block_4/ATreport.html";
//	BlockObject block1;
//	//block1.SetPath("X:/Projects/test_V833449");
//	//block1.SetName("Block_4");
//	ATReport at_report;
//	auto Atdata_1 = std::make_shared<ATData>();
//	//block1.LoadATXML("Y:/Test_Data/DATA/QZ/AT/gcp_AT/02014-CP.xml", Atdata_1);
//	block1.LoadATXML("X:/testxml/Block_2-export.xml", Atdata_1);
//	block1.SetATData(Atdata_1);
//	//block1.ParseATReport(at_report);
//	block1.GenerateATReportPicture(at_report, "X:/bug/img.jpg");
//	std::cout << block1.ExportATReport(at_report, reportjson_html) << std::endl;
//	std::string testfile = "D:/MyLearning/Learning_Materials/run/camera/testblock/shouban/20220126110406.png";
//	//Bitmap bitmap;
//	//bitmap.ReadExifTag()
//	//std::string  projdbfile = "D:/MyLearning/Git/third_party/Windows/vc142/proj/6.3.2/data";//proj.db
//	//std::string strEnv = "PROJ_LIB=" + projdbfile;
//	//int status = putenv(strEnv.c_str());
//
//	time_t timer1 = clock();
//	srs_s srs;
//	// srs.ID = 4;
//	 //srs.definition = "ENU:29.40552,105.02035";
//	 //srs.type = coord_system_type_e::LOCAL_ENU;
////#define USE_PROJ
//	BlockObject::BlockExportOptions export_block;
//	// export_block.export_tiepoint = false;
//	export_block.srs_ = srs;
//	export_block.export_tiepoint_ = true;
//	std::string image_dir_ = "T:/Test_Data/DATA/NN01014/NN01014/PHOTO";
//	std::string image_dir = "D:/MyLearning/Testing/2haoji/photo";
//	std::string ATxml = "E:/Block_8-export.xml";
//	//std::string xml_filepath = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/Block_4 - export.xml";
//	//std::string xml_filepath = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/Block_5 - export.xml";
//	//std::string xml_filepath = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/local_coor.xml";//???????????
//	std::string xml_filepath = "D:/data/Projects/NewProject11/Block_4/block_AT.xml";
//	//std::string xml_filepath = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/76EPSG(4547)- export.xml";
//	//std::string xml_filepath = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/Block_1 - AT - AT 3_3 - exportafterGCP(1).xml";
//	//std::string xml_filepath_ = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/Block5_export.xml";
//	//std::string xml_filepath_ = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/Block_1withtiepoint_.xml";
//	std::string xml_filepath_ = "D:/MyLearning/Learning_Materials/run/camera/testblock/results/MyTestBlock.xml";
//	BlockObject *block = new BlockObject("D:/MyLearning/Learning_Materials/run/camera/testblock/results");
//	//block.SetStatus(BlockObject::bs_e::STATUS_UNKNOWN);
//	std::vector<std::string> image_filter = { ".jpg" ,".tif",".png",".arw",};
//	//block->AddImages(image_dir, image_filter);
//    //block.AddImages(image_dir, image_filter);
//	Timer time;
//	//time.Start();
//	std::vector<std::string> filenames;
//	int* cbProgress_tmp = new int(0);
//	bool* cbCancle = false;
//	//block->SearchImages(image_dir_, filenames, image_filter);
//	LOGD(String::StringPrintf("Search images spends %f s", time.ElapsedSeconds()));
//	time.Restart();
//	//std::thread addimage_thread(std::bind(&BlockObject::Addimages_Beta, std::ref(block), filenames, std::ref(cbProgress_tmp), std::ref(cbCancle)));
//	//addimage_thread.detach();
//	//while (true)
//	//{
//	//	std::cout << *cbProgress_tmp << std::endl;
//	//	if (*cbProgress_tmp == 100)
//	//	{
//	//		std::cout << "??????????" << std::endl;
//	//		break;
//	//	}	
//	//}
//	//addimage_thread.join();
//	//LOGD(String::StringPrintf("Add images spends %f s", time.ElapsedSeconds()));
//	filenames.clear();
//	//std::string image_dir_tmp = "D:/MyLearning/Testing/2haoji/photo";
//	//block->SearchImages(image_dir_tmp, filenames, image_filter);
//	//block->Addimages_Beta(filenames, &cbProgress);
//	//filenames.clear();
//	//image_dir_tmp = "D:/MyLearning/Learning_Materials/run/camera/testblock/testexif";
//	//block->SearchImages(image_dir_tmp, filenames, image_filter);
//	//block->Addimages_Beta(filenames, &cbProgress);
//	filenames.push_back("D:/MyLearning/Learning_Materials/run/camera/testblock/20211221152103.jpg");
//	//block.AddImages(filenames);
//	//Application::Getinstance();
//
//	/*
//	* ????SetAT0()?????????	
//	auto block_ = Getblock(xml_filepath);
//	LOG(INFO) << block_->GetAT0().GetATData().use_count();
//	*/
//	auto Atdata = std::make_shared<ATData>();
//
//	time.Start();
//	std::cout << "\n????Block XML????????" << std::endl;
//	block->LoadATXML(ATxml, Atdata);
//	time.PrintSeconds();
//	block->ClearImageIds();
//	block->SetATData(Atdata);
//	std::cout << "\n???Block Bin??????" << std::endl;
//	time.Restart();
//	
//
//	//std::string file_path_tiepoints = "D:/MyLearning/Testing/tiepoints.bin";
//	//std::string file_path_images = "D:/MyLearning/Testing/block.bin";
//	//block->ExportATBinaryWithoutTiepoints(file_path_images);
//	//block->ExportTiepointsBinary(file_path_tiepoints);
//	//time.PrintSeconds();
//
//	{
//		std::string AT_file_path = "E:/block_AT.xml";
//		//block->ExportATBinary(AT_file_path);
//		LOGI(String::StringPrintf("??SCSFR.bin?????%f", time.ElapsedSeconds()));
//
//		time.Restart();
//		auto atdata = std::make_shared<ATData>();
//		//block->LoadATBinary(AT_file_path, atdata);
//		block->LoadATXML(AT_file_path, atdata);
//
//		block->UpdateATGroup(atdata);
//		LOGI(String::StringPrintf("??SCSFR.bin?????%f", time.ElapsedSeconds()));
//		BlockObject::BlockExportOptions opt;
//		opt.export_tiepoint_ = true;
//		block->SetATData(atdata);
//		block->ExportATXML("X:/AT_tmp.xml", opt);
//		return;
//	}
//
//	block->GetCurrentATMutual()->GetPoints3DMutual().clear();
//	block->GetCurrentATMutual()->GetCamerasMutual().clear();
//	block->GetCurrentATMutual()->GetImagesMutual().clear();
//	block->GetCurrentATMutual()->GetControlPointsMutual().clear();
//	block->GetPhotoGroupsMutual().clear();
//	block->GetSRSsMutual().clear();
//	std::cout << "\n????Block Bin????????" << std::endl;
//	time.Restart();
//	//block->LoadATBinaryWithoutTiepoints(file_path_images, block->GetCurrentATMutual());
//	//block->LoadTiepointsBinary(file_path_tiepoints, block->GetCurrentATMutual());
//	time.PrintSeconds();
//	//block->SetAT0(Atdata);
//	//std::string orign_crs = block.GetBlockSRS().definition;
//	std::cout << "\n????Block XML??????" << std::endl;
//	time.Restart();
//	block->ExportATXML(xml_filepath_, export_block);
//	time.PrintSeconds();
//	LOG(INFO) << "????????" << double(clock() - timer1) / CLOCKS_PER_SEC;
//	//block.AddImages(image_dir, ".*");
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
//	// ?????????????
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
//	// ???vector<Point>????????????
//	std::vector<std::vector<cv::Point>> pic;
//	pic.push_back(points1);
//	pic.push_back(points2);
//	// ????????????
//	fillPoly(result, pic, cv::Scalar(0, 0, 255), 16, 0);
//	// ?????????????
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
//	strftime(tmp, sizeof(tmp), "%Y/%m/%d %X %A ?????%j?? %z", localtime(&t));
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
//		//	// ?????? ?????????????????????????
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
////?????????????????
////??????????????????????
//void storageLowerTriangle(int array[], int i, int j, int e,int N) {
//	array[(2 * N - i + 2) * (i - 1) / 2 + (j - i)] = e;
//}
////????????C
//void storageConstant(int array[], int constant, int N) {
//	array[N * (N + 1) / 2] = constant;
//}
//
//
////?????????????
//int getValue(int array[], int i, int j, int N) {
//	if (i <= j) {
//		//???????????????
//		return array[(2 * N - i + 2) * (i - 1) / 2 + (j - i)];
//	}
//	else {
//		//?????????????????
//		return array[N * (N + 1) / 2];
//	}
//}
//
//void testUpperTriangle(int N)
//{
//	//???????????
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
//	//????????
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
//	//??????????
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
//	std::cout << "???????" << num_pairs << std::endl;
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
//					std::cout << "???block????????????" << match_pair.size() << std::endl;
//					match_pair.clear();
//					num_images_per_blcok.clear();
//				}
//				match_pair.emplace_back(row);
//				match_pair.emplace_back(col);
//			}
//		}
//	}
//	//???????????????
//	match_blocks.push_back(match_pair);
//	std::sort(match_pair.begin(), match_pair.end());
//	auto pos = std::unique(match_pair.begin(), match_pair.end());
//	match_pair.erase(pos, match_pair.end());
//	std::cout << "???block????????????" << match_pair.size() << std::endl;
//
//	//block???
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
//
//int testCameraDB(const int& argc, char** argv)
//{
//	std::string imageFilepath = "";
//	std::string sfileDatabase = "";
//	std::string outputdir = "";
//	//?????????????
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
//		//????????????????????????????????(????????Thumbs.db???)
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
//
//	std::vector<Exiv2::ExifData> ed_vector;
//	for (std::vector<std::string>::const_iterator iter_image = imageabsFilepath.begin();
//		iter_image != imageabsFilepath.end(); ++iter_image)
//	{
//		if ((*iter_image).find("mask.png") != std::string::npos
//			|| (*iter_image).find("_mask.png") != std::string::npos)
//		{
//			std::cout << *iter_image << " is a mask image" << "\n";
//			continue;
//		}
//		Exiv2::Image::AutoPtr image_;
//		try
//		{
//			image_ = Exiv2::ImageFactory::open(*iter_image);
//		}
//		catch (Exiv2::Error& e)
//		{
//			LOGE(String::StringPrintf("Caught Exiv2 exception %s", e));
//			return false;
//		}
//
//
//		if (image_.get() == 0)
//		{
//			LOGW("Exif is empty");
//			return false;
//		}
//
//		image_->readMetadata();
//		Exiv2::ExifData ed = image_->exifData();
//		ed_vector.push_back(ed);
//
//		if (0 && !ed.empty())
//		{
//			//??exif??????????????????
//			std::string path = File::GetParentDir(imageFilepath);
//			path = path + "/ThumbC";
//			File::CreateDirIfNotExists(path);
//			Exiv2::ExifThumbC thumbc(ed);
//			//auto ret = thumbc.writeFile(path + PATH_SEPARATOR_STR + File::GetPathBaseName(*iter_image));
//			//if (ret)
//			//{
//			//	num_thumb++;
//			//}
//		}
//
//		//??????????????focal length??focal length35mm?????????
//		//double focal_length, focal_lengthIn35mm, width, height, ppx, ppy;
//		//focal_length = focal_lengthIn35mm = width = height = ppx = ppy = -1;
//
//		//ExifInfo exifInfo;
//		//std::unique_ptr<ExifIO> exif_(new ExifIO);
//		//exif_->Open(*iter_image);
//
//		//short orientation = exif_->GetOrientation();
//		//std::cout << orientation << std::endl;
//		//exif_->SetBrand("DJI");
//		//exif_->SetFocal(21);
//		//exif_->SetFocalLengthIn35mm(35);
//		//exif_->SetModel("?????");
//		//exif_->SetLatitude(36.10);
//		//exif_->SetLongitude(120.11);
//		//exif_->SetAltitude(100);
//		//exif_->Write();
//
//	}
//	LOGI(String::StringPrintf("Reading Exif thumbs spends %f s", time.ElapsedSeconds()));
//	std::string path = File::GetParentDir(imageFilepath);
//	path = path + "/ThumbC";
//	File::CreateDirIfNotExists(path);
//	time.Restart();
//	for (int i = 0; i < ed_vector.size(); i++)
//	{
//		if (!ed_vector[i].empty())
//		{
//			//??exif??????????????????
//			std::string thumbname = "img_" + std::to_string(i);
//			Exiv2::ExifThumbC thumbc(ed_vector[i]);
//			auto ret = thumbc.writeFile(path + PATH_SEPARATOR_STR + File::GetPathBaseName(thumbname));
//			if (ret)
//			{
//				num_thumb++;
//			}
//		}
//	}
//	LOGI(String::StringPrintf("Getting thumbs spends %f s", time.ElapsedSeconds()));
//	std::cout << "The num of Thumb is " << num_thumb << std::endl;
//	return 0;
//}
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
//	//?????????????
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
//		//????????????????????????????????(????????Thumbs.db???)
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
//	//?????????????
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
//		//????????????????????????????????(????????Thumbs.db???)
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
//	if (isFirstCall) { // ??????????????????????
//		// ?????ASCII?????????????????
//		isFirstCall = 0;
//
//		// ????MAX_CHAR??????????????????
//		lists = glGenLists(MAX_CHAR);
//
//		// ??????????????????????????????????
//		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
//	}
//	// ??????????????????????????????????
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
//	//???????
//	glClear(GL_COLOR_BUFFER_BIT);
//	glColor3f(0.0f, 0.0f, 0.0f);
//
//	GLfloat lineWidth = 2.0f;
//	glLineWidth(lineWidth);
//	glBegin(GL_LINES);
//	glVertex2f(10.0f, 5.0f);   //????
//	glVertex2f(10.0f, 147.0f);
//	glVertex2f(9.0f, 143.0f);
//	glVertex2f(10.0f, 147.0f);
//	glVertex2f(11.0f, 143.0f);
//	glVertex2f(10.0f, 147.0f);
//	glVertex2f(5.0f, 10.0f);   //????
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
//	//???????n?????x?????y????
//	int x_data[n], y_data[n];
//	for (i = 0; i < n; i++) {
//		x_data[i] = 1 + rand() % 101;  //x???????????1??100??
//		y_data[i] = 1 + rand() % 51;   //y???????????1??50??
//	}
//	//????????????????????????
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
//	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  //???????
//	glMatrixMode(GL_PROJECTION);
//	gluOrtho2D(0.0, 200.0, 0.0, 150.0);   //????????????????????
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
//????????xml??????
int main12(int argc, char** argv)
{
	
	//????????????????id???????
	//map<int,int> IDMAPS; first?s3d???second?mok???
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
	std::map<int, int> imgids;//??mok????s3d
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
//???????? ???????????ecef???enu??????????????cc?????????4547????????
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
//20230801:???? ??????????????????????????????????????
//param[1]:??????????xml???
//param[2]:???????????xml???
//param[3]:??????
int GCP(int argc, char** argv)
{

	//????????????????id???????
	//map<int,int> IDMAPS; first?s3d???second?mok???
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


	std::map<int, int> imgids_base_vs_gcp;//?????????GCP??
	std::map<int, int> imgids_gcp_vs_base;//??GCP?????????
	for (auto iterimg : atdata_target->GetImages())
	{
		auto img = atdata_withgcp->FindImageWithName(iterimg.second.GetName(), atdata_withgcp->GetImagesIds());
		if (img != nullptr)
		{
			 imgids_gcp_vs_base[img->GetImageId()] = iterimg.first;//????ids??????
			 imgids_base_vs_gcp[iterimg.first] = img->GetImageId();//????ids??????
		}
	}

	//????????
	block_target.GetCurrentATMutual()->GetControlPointsMutual().clear();
	//??????????
	auto& gcps = atdata_withgcp->GetControlPointsMutual();

	for (auto& itergcp : gcps)
	{

		int id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
		if (id == kInvalidSrsId)//??????
		{
			//???????
			block_target.UpdateSRSMap(itergcp.second.GetSrs());
			id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
			//?????Core??????block_Absolute_xml??GCP???srs_id?????

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
//????????????????????????????
int main00(int argc, char** argv)
{

	//????????????????id???????
	//map<int,int> IDMAPS; first?s3d???second?mok???
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

	std::map<int, int> imgids;//??mok????s3d
	for (auto iterimg : atdata_target->GetImages())
	{
		auto img = atdata_withgcp->FindImageWithName(iterimg.second.GetName(), atdata_withgcp->GetImagesIds());
		if (img != nullptr)
		{
			imgids[img->GetImageId()] = iterimg.first;
		}
	}

	//????????
	block_target.GetCurrentATMutual()->GetControlPointsMutual().clear();
	//??????????
	auto& gcps = atdata_withgcp->GetControlPointsMutual();

	for (auto& itergcp : gcps)
	{

		int id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
		if (id == kInvalidSrsId)//?????
		{
			//???????
			block_target.UpdateSRSMap(itergcp.second.GetSrs());
			id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
			//?????Core??????block_Absolute_xml??GCP???srs_id?????

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
//???????????????????????
#include "Core/S3DProjectFile.h"
int main77(int argc, char** argv)
{
	std::string basepath = "D:/TestData/S3DResult/13wan/partition/";
	//???txt??????????
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
	////??????? 
	///*block1.GetATData()->GetPoints3DMutual() = atdata1->GetPoints3DMutual();
	//block1.GetATData()->TransFormTiepoints(atdata1->GetLocalSrs(), block1.GetATData()->GetLocalSrs());*/
	//block1.ExportATDataToXML("D:/MyLearning/CHY/TEST/Proj/testat/Block_6/input-notiepoint.xml", opt1, *block1.GetATData().get());
	////
	//////???????????????????????

	////block1.UpdateATGroup(atdata1);
	////block1.SetATData(atdata1);
	////??????4978?4326?????
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

	//????????????????id???????
	//map<int,int> IDMAPS; first?s3d???second?mok???
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

	std::map<int, int> imgids;//??mok????s3d
	for (auto iterimg : atdata_target->GetImages())
	{
		auto img = atdata_withgcp->FindImageWithName(iterimg.second.GetName(), atdata_withgcp->GetImagesIds());
		if (img != nullptr)
		{
			imgids[img->GetImageId()] = iterimg.first;
		}
	}

	//????????
	block_target.GetCurrentATMutual()->GetControlPointsMutual().clear();
	//??????????
	auto& gcps = atdata_withgcp->GetControlPointsMutual();
	
	for (auto& itergcp : gcps)
	{
		
		int id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
		if (id== kInvalidSrsId)//?????
		{
			//???????
			block_target.UpdateSRSMap(itergcp.second.GetSrs());
			id = block_target.ExistSRS(itergcp.second.GetSrs().definition);
			//?????Core??????block_Absolute_xml??GCP???srs_id?????
			
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

int mainold(int argc, char** argv)
{
	LibRaw libraw;
	int ret = libraw.open_file("D:/BaiduNetdiskDownload/0829/1/N055.CR2");

	if (ret != int(LIBRAW_SUCCESS))
	{
		return -1;
	}

	libraw.imgdata.params.use_camera_wb = 1;
	//ret = libraw
	std::string pstr = "D:/jiaojie/thirdparty/third_party/Windows/vc142/proj/6.3.2/data/proj.db";
	std::string strEnv = "PROJ_LIB=" + pstr;
	int status = putenv(strEnv.c_str());
	//dlg.show();
	double x = 116.283774;
	double y = 39.808981;
	double z = -56.127;
	AI3D::CORE::CoordinateTransformer::TransformByEpsgCode(1, &x, &y, &z, "epsg:4326+5773", "epsg:4326");
	
	CoordinateTransformer::Transform(x,y,z,x,y,z,"epsg:4326+5773","epsg:4326");
	std::cout << std::setprecision(16)<< x << " " << y << " " <<z<< std::endl;
	BlockObject block;
	auto atdata1 = std::make_shared<ATData>();
	block.LoadATXML("D:/jiaojie/xml-yanshou/user_point/usertiepointsi-raw.xml", atdata1);
	std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> gcp_error_map, gcp_error_map1;
	
	//atdata1->UpdataGCPGlobalErrorInfo(gcp_error_map1);
	atdata1->UpdataUserTiepointsGlobalErrorInfo(gcp_error_map);
	/*std::string atbin1 = "C:/data/Projects/roi/NewProject/Block_11/SCSFR.bin";
	block.LoadATBinary(atbin1, atdata1);
	atbin1 = "C:/data/Projects/roi/NewProject/Block_4/Block_4.bin";
	block.LoadATBinaryWithoutTiepoints(atbin1, atdata1);*/

	return 1;
}


int main(int argc, char** argv)
{
	BlockObject block;
	auto atdata1 = std::make_shared<ATData>();
	std::string atbin1 = "D:\\project\\0112\\Block_2\\SCSFR.bin";
	block.LoadATBinary(atbin1, atdata1);
	/*atbin1 = "C:/data/Projects/roi/NewProject/Block_4/Block_4.bin";
	block.LoadATBinaryWithoutTiepoints(atbin1, atdata1);*/

	return 0;
}