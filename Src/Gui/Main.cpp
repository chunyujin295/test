
#include <QMutex>
#include <QDateTime>
#include <QTextStream>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QThreadPool>
#include <QVector3D>

#include "Util/TaskProcess.h"
#include "Gui/TheFirstDlg.h"
#include "Gui/MohackerWin.h"
#include "Core/Application.h"
#include "Core/Logging.h"
#include "Core/File.h"
#include <filesystem>
#include "3DViewer/3dview.h"
#include "Gui/GlobalStruct.h"
#include <QMessageBox>
#include "Core/Application.h"
#include "Core/String.h"
#include "Gui/message_box.h"
#include "Util/Statistic.h"
#include "Util/JobMonitor.h"
#include "Util/OTA.h"
#include "Util/CatchProcess.h"
#include <curl/curl.h>
#include <QHostInfo>
#include "Gui/LoginDialog.h"
#ifdef USE_AI3D_PROJ
#include "Core/Proj/CrsSettings.h"
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/WorkPath.h"

#endif
CatchProcess catchProcess;

//QtConcurrent::run的使用原则，局限，与enigne中thread的关系
//不会阻塞，并行的
//
bool SetStyleSheet(QString style)
{
	QFile qss(style);
	if (!qss.open(QFile::ReadOnly))
	{
		return false;
	}
	QString styleSheet = QLatin1String(qss.readAll());
	qApp->setStyleSheet(styleSheet);
	qss.close();
	return true;
}
//extern void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);


#ifdef _MSC_VER
#include "Windows.h"
#include "DbgHelp.h"

int GenerateMiniDump(PEXCEPTION_POINTERS pExceptionPointers)
{
	// 定义函数指针
	typedef BOOL(WINAPI* MiniDumpWriteDumpT)(
		HANDLE,
		DWORD,
		HANDLE,
		MINIDUMP_TYPE,
		PMINIDUMP_EXCEPTION_INFORMATION,
		PMINIDUMP_USER_STREAM_INFORMATION,
		PMINIDUMP_CALLBACK_INFORMATION
		);
	// 从 "DbgHelp.dll" 库中获取 "MiniDumpWriteDump" 函数
	MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
	HMODULE hDbgHelp = LoadLibrary((L"DbgHelp.dll"));
	if (NULL == hDbgHelp)
	{
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

	if (NULL == pfnMiniDumpWriteDump)
	{
		FreeLibrary(hDbgHelp);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	// 创建 dmp 文件件
	TCHAR szFileName[MAX_PATH] = { 0 };
	const TCHAR* szVersion = L"DumpMoldAI_v1.0";
	SYSTEMTIME stLocalTime;
	GetLocalTime(&stLocalTime);
	wsprintf(szFileName, L"%s-%04d%02d%02d-%02d%02d%02d.dmp",
		szVersion, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond);
	HANDLE hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	if (INVALID_HANDLE_VALUE == hDumpFile)
	{
		FreeLibrary(hDbgHelp);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	// 写入 dmp 文件
	MINIDUMP_EXCEPTION_INFORMATION expParam;
	expParam.ThreadId = GetCurrentThreadId();
	expParam.ExceptionPointers = pExceptionPointers;
	expParam.ClientPointers = FALSE;
	pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
		hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &expParam : NULL), NULL, NULL);
	// 释放文件
	CloseHandle(hDumpFile);
	FreeLibrary(hDbgHelp);
	return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
	// 这里做一些异常的过滤或提示
	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	return GenerateMiniDump(lpExceptionInfo);
}
#endif // _MSC_VER


//int main(int argc, char* argv[])
//{
//	std::shared_ptr<AI3D::CORE::ATData> atdata = std::make_shared<AI3D::CORE::ATData>();
//	AI3D::CORE::BlockObject block("D:/TestData/cc/test/testpreview/");
//	//block.SetPath(inpath);
//
//	  block.LoadATXML("D:/TestData/cc/testpreview/tesw.xml",atdata);
//	AI3D::GUI::ViewWidget*  wgt = new AI3D::GUI::ViewWidget(&block);
//	wgt->show();
//	//wgt->exec();
//	return 0;
//}

extern bool CheckOTADownloadedPackageInsideIntranet(int& newVer, QString& newVerName, int& oldVer, QString& oldVerName);

void doIntranetVersionCheckThread()
{
	while (true)
	{
		bool state;
		int oldVer, newVer;
		QString oldVerName, newVerName;

		state = CheckOTADownloadedPackageInsideIntranet(newVer, newVerName, oldVer, oldVerName);
		if (state)
		{
			///			std::cout << "ota package has new version,please quit current application and update it.(from " << std::to_string(oldVer) << "(" + oldVerName.toStdString()
			///				<< ")" << std::to_string(newVer) << "(" << newVerName.toStdString() << ")." << std::endl;
			/// 
			std::cout << "======= NEW OTA PACKAGE HAS BEEN COMING FROM " << std::to_string(oldVer) << "(" + oldVerName.toStdString()
				<< ") TO " << std::to_string(newVer) << "(" << newVerName.toStdString() << ") ======" << std::endl;

			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}
}

extern bool CheckNetworkState(QStringList& networkPathList,QStringList &errorPathList);

void doNetworkCheckThread()
{
	while (true)
	{
		bool state;
		QStringList networkPathList;
		QStringList errorPathList;

		/// fill up networkPathList with actual network paths here,or from various modules where some 
		/// important network directories or network files have been used.
		/// some  important network directories might be as follows:
		/// 1)job queue paths
		/// 2)software path on drive Y,which has been used for ota updating and logs collecting from all the engine nodes on internat.
		/// 3)otherwise,such as image directories and etc.

		state = CheckNetworkState(networkPathList,errorPathList);

		if (state)
		{
			///			std::cout << "ota package has new version,please quit current application and update it.(from " << std::to_string(oldVer) << "(" + oldVerName.toStdString()
			///				<< ")" << std::to_string(newVer) << "(" << newVerName.toStdString() << ")." << std::endl;
			/// 
			//std::cout << "======= NEW OTA PACKAGE HAS BEEN COMING FROM " << std::to_string(oldVer) << "(" + oldVerName.toStdString()
			//	<< ") TO " << std::to_string(newVer) << "(" << newVerName.toStdString() << ") ======" << std::endl;

			//break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}
}


bool killTaskProcess()
{
	DWORD processId = 0;
	if (catchProcess.IsProgramRunning("MoldAINode.exe", processId))
	{
	//	if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << "to kill engine task.exe(" << processId << ").";
			LOGI(oss.str());
			std::cout << oss.str() << std::endl;
		}

		// add log to monitor taskkill command.
		QProcess process;
		//process.start("cmd", QStringList() << "/c" << "taskkill /f /t /im MoldAINode.exe");

		process.start("cmd", QStringList() << "/c" << "taskkill /f /t /im MoldAINode.exe");
		
		process.waitForStarted();
		process.waitForFinished();

		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << "to kill engine engine.exe(" << processId << ") end.";
			LOGI(oss.str());
			std::cout << oss.str() << std::endl;
		}

		return true;
	}
	else
	{
//		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << " engine.exe is not running ";
			LOGI(oss.str());
			std::cout << oss.str() << std::endl;
		}

		return false;
	}
}

int startEngine()
{
	QString path = QCoreApplication::applicationDirPath();
	QString workingDirectory = path;

	path.append("/MoldAINode.exe");

	QStringList argumentList;

	qint64 pid = -1;

	QProcess process;

	LOGI("start process:" + qstr2str(path));
	std::cout << "start process:" << qstr2str(path) << std::endl;

	argumentList << "true";

	//process.start(path, argumentList);
	//process.start("cmd", QStringList() << "/c" << "taskkill /f /t /im MoldAINode.exe");
	//process.start("start", QStringList() << path << "true");

	process.start("cmd", QStringList() << "/c" << "start" << path << "true");

	bool suce = process.waitForStarted(-1);
	if (!suce)
	{
		process.close();

		LOGI("start process failed:" + qstr2str(path));
		std::cout << "start process failed:" << qstr2str(path) << std::endl;

		return -1;
	}

	pid = process.processId();

	LOGI("start process succ:" + qstr2str(path) + " pid:" + std::to_string(pid));
	std::cout << "start process succ:" << qstr2str(path) << " pid:" << pid << std::endl;

	process.waitForFinished(-1);


	int iTaskRetVal = -1;

	if (process.exitStatus() == QProcess::NormalExit)
	{
		// normally exited.
	//	std::cout << __FILE__ << __FUNCTION__ << " " << __LINE__ << " task exited normally:" << process.exitCode() << std::endl;
		iTaskRetVal = process.exitCode();
		LOGI("process normal exit:" + qstr2str(path) + " pid:" + std::to_string(pid) + " normalexit:" + std::to_string(iTaskRetVal));
		std::cout << "process normal exit:" << qstr2str(path) << " pid:" << pid << "normalexit:" << iTaskRetVal << std::endl;

		process.close();
	}
	else
	{
		// crashed.

		LOGI("process crash:" + qstr2str(path) + " pid:" + std::to_string(pid));
		std::cout << "process crash:" << qstr2str(path) << " pid:" << pid << std::endl;

		process.close();
	}

	return 0;
}

qint64 nTest3Num = 0;
void execTest3Thread()
{
	while (true)
	{
		//if (bQuitingApplication)
		//{
		//	std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " quiting=====... " << nTest2Num << std::endl;
		//	LOGI("nTest2Num:" + std::to_string(nTest2Num) + " quiting... ");
		//	break;
		//}

		std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << nTest3Num << std::endl;
		LOGI("nTest3Num:" + std::to_string(nTest3Num));

		nTest3Num++;

		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		if (nTest3Num == 5)
		{
			int a = 30;
			int b = 0;
			//		int c = a / b;
///			exit(0);
			std::cout << "===================call kill========= 1" << std::endl;
			killTaskProcess();
		}
	}
}

qint64 nTest4Num = 0;
void execTest4Thread()
{
	while (true)
	{
		//if (bQuitingApplication)
		//{
		//	std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " quiting=====... " << nTest2Num << std::endl;
		//	LOGI("nTest2Num:" + std::to_string(nTest2Num) + " quiting... ");
		//	break;
		//}

		std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " === nTest4Num === " << nTest4Num << std::endl;
		LOGI("nTest4Num:" + std::to_string(nTest4Num));

		nTest4Num++;

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if (nTest4Num == 2)
		{
			int a = 30;
			int b = 0;
			//		int c = a / b;
///			exit(0);
			//killTaskProcess();
			startEngine();
		}
	}
}

//QString testTaskJsonLog = "Z:/MoldAIjob/job128/Logs";
//QString testTaskJsonFile = "Z:/Data/reconstruct_data/project/mok/guangzhou/guangzhou/Block_7/job_20230322115153_AT/task_def_230.json";

QString testTaskJsonLog = "D:/worksp/jobs/Logs";
QString testTaskJsonFile = "D:/worksp/projs/NewProject11221/Block_2/job_20230316095227_AT/task_def_10.json";

void writeCurrentLockInfo(bool bLock)
{
	qint64 qid = QApplication::applicationPid();
	QString qname = QApplication::applicationName();
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss.zzz");
	QString hostName = QHostInfo::localHostName();

	std::ostringstream oss;
	oss << "host:" << hostName.toStdString() << " qname:" << qname.toStdString() << " qid:" << qid << " now:" << nowTime.toStdString() << (bLock ? " lock." : " unlock.");
	std::cout << oss.str() << std::endl;
	LOGI(oss.str());
}

// taking statistics for all the logs collected from various machines.
// Some vital suggestions are as follows:
// 1)


void execTest6Thread()
{
	while (true)
	{
		//if (bQuitingApplication)
		//{
			//			std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " quiting... " << nTest1Num << std::endl;
			//			LOGI("nTest1Num:" + std::to_string(nTest1Num) + " quiting... ");
		//	break;
		//}

		FILE* fpLock = NULL;

		fpLock = AI3D::CORE::File::FopenDenyWriteLockUtf8(qstr2str(testTaskJsonFile + ".lock"));

		if (fpLock == NULL)
		{
			//std::cout << "++++lock failed++++++++++++" << std::endl;
		//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}


		std::cout << "====== lock succ ==================" << std::endl;

		writeCurrentLockInfo(true);
		std::this_thread::sleep_for(std::chrono::milliseconds(6000));
		std::cout << "====== lock succ2 ==================" << std::endl;

		fclose(fpLock);

		writeCurrentLockInfo(false);

		std::cout << "====== lock close ==================" << std::endl;
	}
}


int main(int argc, char* argv[])
{
	
	//SetConsoleOutputCP(CP_UTF8);
	SetConsoleOutputCP(936);

	/// std::cout << "enter MoldAI.exe..." << std::endl;
#ifdef _MSC_VER
	// 加入崩溃dump文件功能
	SetUnhandledExceptionFilter(ExceptionFilter);
#endif // _MSC_VER

	//QTextCodec* codec = QTextCodec::codecForName("utf-8");

	///QTextCodec* codec = QTextCodec::codecForName("gbk");
	///QTextCodec::setCodecForLocale(codec);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	//测奔溃
	/*char* str;
	printf("%s", str);*/

	QApplication a(argc, argv);

	///UsingNoChinesePathVersion();

	std::cout << qstr2str(a.applicationName()) << " compiled at " << __DATE__ << " " << __TIME__ << std::endl;

	{
		//chy ?此处是说只能打开一个？
		QSharedMemory singleton(a.applicationName());
		if (!singleton.create(1))
		{
			Message_Box::critical(nullptr, QString("Error"), QString("Software already open!"));
			return -1;
		}
	}

	QSettings* pSettingsOld = new QSettings("HKEY_CURRENT_USER\\Software", QSettings::NativeFormat);
	pSettingsOld->remove("AirLook\\JobQueue");
	pSettingsOld->remove("AirLook\\RecentCRS\\Crs");
	pSettingsOld->remove("AirLook\\RecentCRS");
	pSettingsOld->remove("AirLook");
	//判断引擎文件
	if (!JobMonitor::CreateJobQueueDir(Settings::getMasterJobQueue()))
	{
#if 1
		JobMonitor::CreateLocalJobQueueDir();
		JobMonitor::CreateJobQueueDir(Settings::getMasterJobQueue());
#endif
	}
	
	QString  Qenginedir = Settings::getMasterJobQueue() + "/" + JOBENGINESSTR;

	std::string enginedir = qstr2str(Qenginedir);

	try
	{
		const std::filesystem::path engPath = AI3D::CORE::File::BoostPathFromUtf8(enginedir);
		if (!std::filesystem::exists(engPath))
		{
			JobMonitor::CreateJobQueueDir(Settings::getMasterJobQueue());
		}
		else
		{
			std::vector<std::string> enginefiles;
			for (const auto& entry : std::filesystem::directory_iterator(engPath))
			{
				if (entry.is_regular_file())
				{
					enginefiles.push_back(AI3D::CORE::File::BoostPathToUtf8String(entry.path()));
				}
			}
			for (auto& iter : enginefiles)
			{
				const std::filesystem::path fpath = AI3D::CORE::File::BoostPathFromUtf8(iter);
				if (std::filesystem::exists(fpath))
				{
					EngineInfo_s engineinfo;
					if (ENGINE_USE_BIN) {
						engineinfo.loadbin(iter);
					}
					else {
						engineinfo.load(iter);
					}
					
					if (engineinfo.EndTime != "")
					{
						QDateTime QEndTime = QDateTime::fromString(QString::fromStdString(engineinfo.EndTime), "yyyyMMddhhmmss");

						std::cout << std::to_string(getTotalTimeinSec(QEndTime, QDateTime::currentDateTime())) << std::endl;
						if (getTotalTimeinSec(QEndTime, QDateTime::currentDateTime()) > 10)
						{
							std::cout << "remove  engin" << std::endl;
							std::filesystem::remove(fpath);

						}
					}
				}
			}

		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
		LOGI(oss.str());
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
	}

	//if (checkCurrentVersion4OTA())
	//{
	//	std::cout << " get version info about OTA succ." << std::endl;
	//}
	//else
	//{
	//	std::cout << " get version info about OTA failed." << std::endl;
	//}



	//读取master.json
	//QString path = QCoreApplication::applicationDirPath();
	std::string apppath = GetWorkPath();

	std::string machinecode = AI3D::Util::GetMachineCode();

	std::string masterjsonpath = apppath + PATH_SEPARATOR_STR + machinecode + "Master.json";


	// OTA更新后,将上一个版本安装目录中的主要生成文件拷贝到当前安装目录中.
	/*if (doFirstCopyAfterUpgrading(machinecode))
	{
		std::cout << " First run after recent upgrading,do copying previous version's related files." << std::endl;
	}
	else
	{
		std::cout << " Not first run after recent upgrading,no copying action needed." << std::endl;
	}*/
	//chy ?此处应该换个函数万一不存在会引起程序崩溃的

	try
	{
		if (std::filesystem::is_regular_file(AI3D::CORE::File::BoostPathFromUtf8(masterjsonpath)))
		{
			MasterInfo::Getinstance().LoadMasterInfoJson(masterjsonpath);
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
		LOGI(oss.str());
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
	}

	MasterInfo::Getinstance().GetMachineCodeMutual() = machinecode;
	APPUseInfo appuseinfo;
	appuseinfo.StartTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
	appuseinfo.VersionName = AI3D::GUI::VERSION;
	std::string versioncode_tmp = AI3D::GUI::VERSION;
	if (!versioncode_tmp.empty())
	{
		auto vectmp = AI3D::CORE::String::StringSplit(versioncode_tmp, ".");
		//vectmp[0] = vectmp[0].size() == 2 ? vectmp[0] : ("0" + vectmp[0]);
		vectmp[1] = vectmp[1].size() == 2 ? vectmp[1] : ("0" + vectmp[1]);
		vectmp[2] = vectmp[2].size() == 3 ? vectmp[2] : (vectmp[2].size() == 2 ? "0" + vectmp[2] : "00" + vectmp[2]);
		char strtmp[128];
		std::sprintf(strtmp, "%s%2s%3s", vectmp[0].c_str(), vectmp[1].c_str(), vectmp[2].c_str());
		appuseinfo.VersionCode = strtmp;
	}
	MasterInfo::Getinstance().GetAPPUseInfosMutual().push_back(appuseinfo);
	//qInstallMessageHandler(customMessageHandler);

	//TestJobStageTime();

	InitializeLog(argv);
	
	AI3D::CORE::Application::Getinstance().SetUpGDALSettings();
	AI3D::CORE::Application::Getinstance().SetProjLibENV();
	if(!currentVersionName.isEmpty())
		LOGI(AI3D::CORE::String::StringPrintf("Welcome Use.%s", currentVersionName.toStdString()));
	else
		LOGI(AI3D::CORE::String::StringPrintf("Welcome Use.%s", AI3D::GUI::VERSION));

	Q_INIT_RESOURCE(MoldAI);

	//CollectStatistics();
	
	curl_global_init(CURL_GLOBAL_ALL);

	//AI3D::CORE::Application::Getinstance();

	QDir::setCurrent(QCoreApplication::applicationDirPath());
	AI3D::GUI::MohackerWin* w = AI3D::GUI::MohackerWin::GetInstance();
#ifdef USE_AI3D_PROJ
	//std::string globalsettingsfile = AI3D::CORE::Application::Getinstance().GetAPPPath() + "/crsconfig.ini";
	//globalsettingsfile =AI3D::CORE::File::EnsureUnifySlash(globalsettingsfile);
	//QString qglobalsettingsfile = QString::fromStdString(globalsettingsfile);
	//AI3D::PROJ::CrsSettings::setGlobalSettingsPath(qglobalsettingsfile);
	//AI3D::PROJ::CrsSettings settings;
	///*settings.setGlobalSettingsPath(qglobalsettingsfile);*/
	//auto lists =  settings.allKeys();
	//
	//std::cout << lists.size() << std::endl;
	//QStringList authids;
	//authids << " epsg:4978 " << " epsg:4326" ;
	//settings.setValue(QStringLiteral("Crs/recentAuthId"), authids);
	//auto lists1 = settings.allKeys();
	//std::cout << lists1.size() << std::endl;
	//auto lists2 = settings.value(QStringLiteral("Crs/recentAuthId")).toStringList();
	//std::cout << lists2.size() << std::endl;
	/*std::string strcrs =  "EPSG:4978";
	AI3D::PROJ::CoordinateReferenceSystem crs(strcrs);
	AI3D::PROJ::CoordinateReferenceSystem::InsertRecentCoordinateReferenceSystem((crs));*/
	//
	//std::cout << lists2.size() << std::endl;
#endif
	/*Settings::RemoveRecentCrs("ENU:12,12");
	auto lists = Settings::GetRecentCrs();
	std::cout << lists.size() << std::endl;*/

	//QSettings settings(qglobalsettingsfile,QSettings::IniFormat);
	//QStringList authids;
	//authids << " 1 " << " 2" << "enu 4";
	//settings.setValue(QStringLiteral("UI/recentProjectionsAuthId"), authids);
	//QStringList projectionsAuthId = settings.value(QStringLiteral("UI/recentProjectionsAuthId")).toStringList();
	//std::cout << projectionsAuthId.size() << std::endl;
    //set StyleSheet
	SetStyleSheet(":/new/prefix1/skin/skin.qss");
	
    QThreadPool::globalInstance()->setStackSize(16);
	QThreadPool::globalInstance()->setMaxThreadCount(12);

	/*AI3D::GUI::LoginDialog* loginDialog = new AI3D::GUI::LoginDialog;
	QObject::connect(loginDialog, &AI3D::GUI::LoginDialog::closeAll, w, &AI3D::GUI::MohackerWin::Slot_quit);
	if (loginDialog->exec() == QDialog::Accepted)
	{
		;
	}*/
		

	AI3D::GUI::TheFirstDlg* firstDialog = new AI3D::GUI::TheFirstDlg;
	QObject::connect(firstDialog, &AI3D::GUI::TheFirstDlg::newProject, w, &AI3D::GUI::MohackerWin::Slot_Action_NewProject);
	QObject::connect(firstDialog, &AI3D::GUI::TheFirstDlg::openProject, w, &AI3D::GUI::MohackerWin::Slot_Action_OpenProject);
	QObject::connect(w, &AI3D::GUI::MohackerWin::CloseFirstWgt,firstDialog, &AI3D::GUI::TheFirstDlg::close );
	if (firstDialog->exec() == QDialog::Accepted)
	{
		;
	}

	/*std::thread doIntranetVersionCheck(doIntranetVersionCheckThread);
	doIntranetVersionCheck.detach();*/

///	std::thread doNetworkCheck(doNetworkCheckThread);
///	doNetworkCheck.detach();


	//std::thread execTest3(execTest3Thread);
	//execTest3.detach();

	//std::thread execTest4(execTest4Thread);
	//execTest4.detach();

	//std::thread execTest6(execTest6Thread);
	//execTest6.detach();

    int ret = a.exec();
	curl_global_cleanup();
	LOGI("shutdown log.");
	ShutDownLog();
    return ret;
}
