#pragma once
#include <QVector>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <iostream>

#include <thread>
#include <fstream>  
#include <streambuf>
#include "Settings.h"
#include "Core/Types.h"
#include "Core/json.h"
#include "Core/File.h"
#include "Core/String.h"
#include <set>
#include <rapidjson\document.h>
#include <rapidjson\writer.h>
#include <rapidjson\stringbuffer.h>
#include "rapidjson\prettywriter.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "Core/StringResource.h"
#include "Core/Logging.h"
#include "Core/PointManager.h"

#include "Util/OTA.h"
#include "Core/TaskDef.h"



#ifdef _MSC_VER
#include "Windows.h"
#include "DbgHelp.h"



#endif 

QDateTime getEarlyDateTime(QVector<QDateTime>& stageDateTimeVector);
QDateTime getLateDateTime(QVector<QDateTime>& stageDateTimeVector);
QString getTotalTime(QDateTime& firstDateTime, const QDateTime& lastDateTime);
int getTotalTimeinSec(const QDateTime& firstDateTime, const QDateTime& lastDateTime);
int getDiffTime(QDateTime& firstDateTime, QDateTime& lastDateTime);

static void DumpQString(QString& str)
{
	qDebug() << "dumpqs:" << str << " size:" << str.size() << " len:" << str.length();
	QByteArray ba = str.toUtf8();
	qDebug() << " ba size:" << ba.size() << " len:" << ba.length();
	for (int i = 0; i < ba.size(); i++)
	{
		char ch = ba.at(i);
		qDebug() << QString("%1").arg(static_cast<int>(ch), 2, 16, QChar('0'));
	}
}

/** Narrow std::string is UTF-8 (matches MSVC /utf-8 and Core string literals). */
static QString str2qstr(std::string str)
{
	return QString::fromUtf8(str.data(), static_cast<int>(str.size()));
}

static std::string qstr2str(QString qstr)
{
	const QByteArray ba = qstr.toUtf8();
	return std::string(ba.constData(), static_cast<size_t>(ba.size()));
}

static QString str2qstr2(std::string str)
{
	return str2qstr(str);
}

static std::string qstr2str2(QString qstr)
{
	return qstr2str(qstr);
}

static jobtype_e GetJobType(std::string jobstr)
{
	auto strs = AI3D::CORE::String::StringSplit(jobstr, "_");

	if (std::count(strs.begin(), strs.end(), JOB_TYPE_SC))
		return JOB_AT;
	else if (std::count(strs.begin(), strs.end(), JOB_TYPE_RE))
		return JOB_TILE;
	else if (std::count(strs.begin(), strs.end(), BATCH_STRING))
		return JOB_BATCH;
	else
	{
		return JOB_UNKNOWN;
	}
}

static int GetTypeId(std::string jobstr)
{
	jobtype_e type = GetJobType(jobstr);
	if (type == JOB_AT)
		return ATSTARTTYPE;
	else if (type == JOB_TILE)
		return RECONSTRUCTIONSTARTTYPE;
	else if (type == JOB_BATCH)
		return BATCHSTARTTYPE;
}

enum jobpriority_e
{
	PRIORITY_PAUSE,
	PRIORITY_URGENT,
	PRIORITY_HIGH,
	PRIORITY_NORMAL,
	PRIORITY_LOW,
	 
};


static std::map< jobpriority_e, std::string> proritydir_str =
{
	{PRIORITY_URGENT ,"Urgent"},
	{PRIORITY_HIGH ,HIGHLEVEL},
	{PRIORITY_NORMAL, NORMALLEVEL},
	{PRIORITY_LOW,LOWLEVEL},
	{PRIORITY_PAUSE,"Pause"},

};

enum StepAT
{
	GenTasks,FeatureDetection, PairSelection, MatchPairs, SfM, OptimizeAT,Reconstruction, BatchPrepare
};

#define FEATURE_PROGRESS 30
#define PAIR_PROGRESS 40
#define MATCH_PROGRESS 60
#define SFMPART_PROGRESS 90
#define COMPLETE_PROGRESS 100
static std::array<float, 2> GetATProgress(StepAT step, bool sfmwithopt = false)
{
	if (!sfmwithopt)
	{
		static std::map<StepAT, std::array<float, 2>> at_progrees =
		{
		{GenTasks ,std::array<float, 2>{0.0f,1.0f}},
		{FeatureDetection ,std::array<float, 2>{1.0f,FEATURE_PROGRESS * 1.0f}},
		{PairSelection,std::array<float, 2>{FEATURE_PROGRESS * 1.0f,PAIR_PROGRESS * 1.0f}},
		{MatchPairs,std::array<float, 2>{PAIR_PROGRESS * 1.0f,MATCH_PROGRESS * 1.0f}},
		{SfM,std::array<float, 2>{MATCH_PROGRESS * 1.0f,COMPLETE_PROGRESS * 1.0f}},
		{OptimizeAT,std::array<float, 2>{1.0f,COMPLETE_PROGRESS * 1.0f}},
		};
		return at_progrees.at(step);
	}
	else
	{
		static std::map<StepAT, std::array<float, 2>> at_progrees =
		{
		{GenTasks ,std::array<float, 2>{0.0f,1.0f}},
		{FeatureDetection ,std::array<float, 2>{1.0f,FEATURE_PROGRESS * 1.0f}},
		{PairSelection,std::array<float, 2>{FEATURE_PROGRESS * 1.0f,PAIR_PROGRESS * 1.0f}},
		{MatchPairs,std::array<float, 2>{PAIR_PROGRESS * 1.0f,MATCH_PROGRESS * 1.0f}},
		{SfM,std::array<float, 2>{MATCH_PROGRESS * 1.0f,SFMPART_PROGRESS * 1.0f}},
		{OptimizeAT,std::array<float, 2>{SFMPART_PROGRESS * 1.0f,COMPLETE_PROGRESS * 1.0f}},
		{Reconstruction,std::array<float, 2>{0 * 1.0f,COMPLETE_PROGRESS * 1.0f}},
		};
		return at_progrees.at(step);
	}
	
}

static std::map< std::string, StepAT> StepAT_str =
{
	{"GenTasks completed" ,GenTasks},
	{"Keypoint tasks completed" ,FeatureDetection},
	{"pair selection completed", PairSelection},
	{"Matching points completed",MatchPairs},
	{"SfM completed",SfM},
	{"Optimize AT",OptimizeAT},
	{"Reconstruction",Reconstruction},
	
};


static std::map<  std::string, std::string > StepATFromfunctionToshow =
{
	{"RunBatchPrepare","starting  BatchPrePare"},
	{"RunGenTasks", "starting AT"},
	{"RunFeatureDetection","keypoints extracting"},
	{"RunPairSelection" ,"pair selecting"},
	{"RunMatchPairs","pair matching"},
	{"RunSfM","sfm"},
	{"RunOptimizeAT","at optimizing"},
	{"RunReconstruction","starting  Reconstruction"},
};

static std::map<  std::string, std::string > StepATFromfunctionToshow_chinese =
{
	{"RunBatchPrepare","批处理 "},
	{"RunGenTasks", "创建任务 "},
	{"RunFeatureDetection","特征检测 "},
	{"RunPairSelection" ,"像对选择 "},
	{"RunMatchPairs","特征匹配 "},
	{"RunSfM","平差 "},
	{"RunOptimizeAT","空三优化 "},
	{"RunReconstruction","重建 "},
};

static std::map< StepAT,std::string > StepAT_function =
{
	{GenTasks,"RunGenTasks" },
	{FeatureDetection,"RunFeatureDetection" },
	{PairSelection,"RunPairSelection" },
	{MatchPairs,"RunMatchPairs"},
	{SfM,"RunSfM"},
	{OptimizeAT,"RunOptimizeAT"},
	{Reconstruction,"RunReconstruction"},
	{BatchPrepare,"RunBatchPrepare"}
};

static std::map<  std::string,StepAT > StepATFromfunction =
{
	{"RunGenTasks", GenTasks},
	{"RunFeatureDetection",FeatureDetection},
	{"RunPairSelection" ,PairSelection},
	{"RunMatchPairs",MatchPairs},
	{"RunSfM",SfM},
	{"RunOptimizeAT",OptimizeAT},
	{"RunReconstruction",Reconstruction},
};

static std::string GetTaskStartingString(std::string jobstr)
{
	jobtype_e jobtype = GetJobType(jobstr);
	std::string msg = "";
	if (jobtype == JOB_AT)
	{
		msg = at_starting_string;
	}
	else if (jobtype == JOB_TILE)
	{
		msg = model_starting_string;
	}
	else if (jobtype == JOB_BATCH)
	{
		msg = batchprepare_starting_string;
	}
	else
	{

	}
	return msg;
}

static std::string GetTaskEndingString(std::string jobstr)
{
	jobtype_e type = GetJobType(jobstr);
	if (type == JOB_AT)
		return "at finished";
	else if (type == JOB_TILE)
		return "model finished";
	else if (type == JOB_BATCH)
		return "at preprocess finished";
}


struct JobInfo_s
{
	std::string ProjectPath = "";
	std::string ItemPath = "";

	std::string ProjectPath2 = "";
	std::string ItemPath2 = "";
	AI3D::CORE::PointInfoBase point_info;

	JobInfo_s() {};
	
	
	JobInfo_s(std::string projectpath, std::string itempath) 
	{
// ProjectPath2 = GBK2UTF8(projectpath);
 ProjectPath2 = projectpath;
// ItemPath2 = GBK2UTF8(itempath);
 ItemPath2 = itempath;

		ProjectPath = projectpath;
		ItemPath = itempath;
	};
	

	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		if (ProjectPath != "")
			json_str["ProjectPath"] = ProjectPath;
		if (ItemPath != "")
			json_str["ItemPath"] = ItemPath;
		return json_str;
	}


	std::string WriteToJson2()
	{
		rapidjson::Document document;
		rapidjson::StringBuffer buffer;

		document.SetObject();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		




		if (ProjectPath != "")
			document.AddMember("ProjectPath", rapidjson::Value(ProjectPath.c_str(), allocator), allocator);

		if (ItemPath != "")
			document.AddMember("ItemPath", rapidjson::Value(ItemPath.c_str(), allocator), allocator);


		document.Accept(writer);

		return buffer.GetString();
	}

	void WriteToJson3(rapidjson::Value &value,rapidjson::Document &document)
	{
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		if (ProjectPath != "")
			value.AddMember("ProjectPath", rapidjson::Value(ProjectPath.c_str(), allocator), allocator);

		if (ItemPath != "")
			value.AddMember("ItemPath", rapidjson::Value(ItemPath.c_str(), allocator), allocator);
	}



	static  JobInfo_s CreateFromJson(nlohmann::json json_str)
	{
		JobInfo_s jobinfo;
		if (json_str.find("ProjectPath") != json_str.end())
			jobinfo.ProjectPath = json_str.at("ProjectPath").get<std::string>();
		if (json_str.find("ItemPath") != json_str.end())
			jobinfo.ItemPath = json_str.at("ItemPath").get<std::string>();
		return jobinfo;
	}


	static  JobInfo_s CreateFromJsonV2(std::string json_str)
	{
		JobInfo_s jobinfo;
		if (json_str.empty())
			return jobinfo;

		rapidjson::Document document;
		if (!document.Parse(json_str.c_str()).HasParseError())
		{



			if (document.HasMember("ProjectPath") && document["ProjectPath"].IsString())
			{
					jobinfo.ProjectPath = document["ProjectPath"].GetString();
					// jobinfo.ProjectPath2 = UTF82GBK(jobinfo.ProjectPath);
					jobinfo.ProjectPath2 = jobinfo.ProjectPath;
			}

			if (document.HasMember("ItemPath") && document["ItemPath"].IsString())
			{
				jobinfo.ItemPath = document["ItemPath"].GetString();
				// jobinfo.ItemPath2 = UTF82GBK(jobinfo.ItemPath);
				jobinfo.ItemPath2 = jobinfo.ItemPath;
			}

		}

		return jobinfo;
	}

	static  JobInfo_s CreateFromJsonV3(rapidjson::Value &value)
	{
		JobInfo_s jobinfo;
		if (!value.IsObject())
			return jobinfo;

		if (value.HasMember("ProjectPath"))
		{
			jobinfo.ProjectPath = value["ProjectPath"].GetString();
			// jobinfo.ProjectPath2 = UTF82GBK(jobinfo.ProjectPath);
			jobinfo.ProjectPath2 = jobinfo.ProjectPath;
		}

		if (value.HasMember("ItemPath"))
		{
			jobinfo.ItemPath = value["ItemPath"].GetString();
			// jobinfo.ItemPath2 = UTF82GBK(jobinfo.ItemPath);
			jobinfo.ItemPath2 = jobinfo.ItemPath;
		}

		return jobinfo;
	}
};

struct EngineInfo_s
{
	int Status = -1;

	std::string Version = "";
	std::string HostName = "";
	std::string UserName = "";
	std::string IPAddr = "";
	std::string ProjectName = "";
	std::string StartTime = "";

	std::string Version2 = "";
	std::string HostName2 = "";
	std::string UserName2 = "";
	std::string IPAddr2 = "";
	std::string ProjectName2 = "";
	std::string StartTime2 = "";

	int ProcessId = -1;
	std::string TaskFile = "";
	std::string TaskFile2 = "";

	
	
	
	
	
	
	
	
	

	std::string EndTime = "";
	std::string EndTime2 = "";

	int FreeMem;
	EngineInfo_s() 
	{
		 GetMemory(TotalMem, FreeMem);
	};

	static void GetMemory(int& allMemory,int& freeMemory)
	{
		int MB = 1024 * 1024;
		int GB = 1024 * 1024 * 1024;
		int HalfGB = 512 * 1024 * 1024;

		MEMORYSTATUSEX memoryStatusEx;
		memoryStatusEx.dwLength = sizeof(memoryStatusEx);

		GlobalMemoryStatusEx(&memoryStatusEx);

		int allMemoryMB = memoryStatusEx.ullTotalPhys / MB;
		int freeMemoryMB = memoryStatusEx.ullAvailPhys / MB;

		 allMemory = (memoryStatusEx.ullTotalPhys + HalfGB) / GB;
		 freeMemory = (memoryStatusEx.ullAvailPhys + HalfGB) / GB;

		
	}
	int TotalMem ;

	bool loadbin(std::string file) {
		std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::binary);
		
		if (!in.is_open())
			return false;

		EngineFile engineFile;
		engineFile.Deserialize(in);
		std::string projectName = engineFile.projectName;
		std::string hostName = engineFile.hostName;
		std::string userName = engineFile.userName;
		std::string taskFile = engineFile.taskFile;
#ifdef WIN32
		// hostName = UTF82GBK(hostName);
		// userName = UTF82GBK(userName);
		// projectName = UTF82GBK(projectName);
		// taskFile = UTF82GBK(taskFile);
#endif 
		HostName = hostName;
		UserName = userName;
		TaskFile = taskFile;
		Version = engineFile.version;
		IPAddr = engineFile.ipAddr;
		Status = engineFile.status;		
		TotalMem = engineFile.totalMem;
		FreeMem = engineFile.freeMem;
		StartTime = engineFile.startTime;
		StartTime2 = StartTime;
		EndTime = engineFile.endTime;
		EndTime2 = EndTime;
		ProcessId = engineFile.processId;
		
		in.close();
		return true;
	}

	bool load(std::string file) {
		if (ENGINE_USE_BIN) {
			return loadbin(file);
		}
		else {
			return loadJson(file);
		}
	}

	bool loadJson(std::string file)
	{
		std::string blkcontent;
		std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
		if (!in.is_open())
			return false;
		std::string line;
		std::string content;
		while (std::getline(in, line)) 
		{

			if (line[line.size() - 1] != '\n')
				line.append("\n");

			content.append(line);
		}
		in.close();
		rapidjson::Document doc;
		if (doc.Parse(content.data()).HasParseError())
		{
			return false;
		}
		if (!doc.IsObject())
		{
			return false;
		}

		if (doc.HasMember("Version"))
		{
			Version = doc["Version"].GetString();
	// Version2 = UTF82GBK(Version);
	 Version2 = Version;
		}

		if (doc.HasMember("HostName"))
		{
			HostName = doc["HostName"].GetString();
	// HostName2 = UTF82GBK(HostName);
	 HostName2 = HostName;
		}
		if (doc.HasMember("UserName"))
		{
			UserName = doc["UserName"].GetString();
	// UserName2 = UTF82GBK(UserName);
	 UserName2 = UserName;
		}
		if (doc.HasMember("IPAddr"))
		{
			IPAddr = doc["IPAddr"].GetString();
	// IPAddr2 = UTF82GBK(IPAddr);
	 IPAddr2 = IPAddr;
		}
		if (doc.HasMember("Status"))
		{
			std::string strstatus = doc["Status"].GetString();
			if (strstatus == "Ready")
				Status = 0;
			if (strstatus == "Busy")
				Status = 1;
		}
		if (doc.HasMember("ProjectName"))
		{
			ProjectName = doc["ProjectName"].GetString();
	// ProjectName2 = UTF82GBK(ProjectName);
	 ProjectName2 = ProjectName;
		}
		if (doc.HasMember("TotalMem"))
		{
			TotalMem = std::atoi(doc["TotalMem"].GetString());	
		}
		if (doc.HasMember("FreeMem"))
		{
			FreeMem = std::atoi(doc["FreeMem"].GetString());			
		}
		if (doc.HasMember("StartTime"))
		{
			StartTime = doc["StartTime"].GetString();
	// StartTime2 = UTF82GBK(StartTime);
	 StartTime2 = StartTime;
		}
		if (doc.HasMember("EndTime"))
		{
			EndTime = doc["EndTime"].GetString();
	// EndTime2 = UTF82GBK(EndTime);
	 EndTime2 = EndTime;
		}

		if (doc.HasMember("TaskFile"))
		{
			TaskFile = doc["TaskFile"].GetString();
	// TaskFile2 = UTF82GBK(TaskFile);
	 TaskFile2 = TaskFile;
		}

		if (doc.HasMember("ProcessId"))
		{
			ProcessId = doc["ProcessId"].GetInt();
		}

		return true;
	}

	bool savebin(std::string file) {
		std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::binary);
		
		if (!out.is_open()) {
			LOGE("Save jobqueue bin failed! file:" + file);
			return false;
		}
		EngineFile engineFile;
		engineFile.version = Version;
		std::string hostName = HostName;
		std::string userName = UserName;
		std::string projectName = ProjectName;
		std::string taskFile = TaskFile;
#ifdef WIN32
		// hostName = GBK2UTF8(hostName);
		// userName = GBK2UTF8(userName);
		// projectName = GBK2UTF8(projectName);
		// taskFile = GBK2UTF8(taskFile);
#endif 
		engineFile.taskFile = taskFile;
		engineFile.hostName = hostName;
		engineFile.userName = userName;
		engineFile.projectName = projectName;
		engineFile.startTime = StartTime;
		engineFile.endTime = EndTime;
		engineFile.ipAddr = IPAddr;
		engineFile.status = Status;
		engineFile.totalMem = TotalMem;
		engineFile.freeMem = FreeMem;
		engineFile.processId = ProcessId;
		
		engineFile.Serialize(out);

		out.close();
	}

	bool save(std::string file)
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		rapidjson::Document document;
		document.SetObject();
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		if (Version != "")
		{
			document.AddMember("Version", rapidjson::Value(Version.c_str(), allocator), allocator);
		}
		if (HostName != "")
		{
			document.AddMember("HostName", rapidjson::Value(HostName.c_str(), allocator), allocator);
		}
		
		if (UserName != "")
		{
			document.AddMember("UserName", rapidjson::Value(UserName.c_str(), allocator), allocator);
		}
		if (ProjectName != "")
		{
			document.AddMember("ProjectName", rapidjson::Value(ProjectName.c_str(), allocator), allocator);
		}
		if (StartTime != "")
		{
			document.AddMember("StartTime", rapidjson::Value(StartTime.c_str(), allocator), allocator);
		}
		if (EndTime != "")
		{
			document.AddMember("EndTime", rapidjson::Value(EndTime.c_str(), allocator), allocator);
		}
		if (IPAddr != "")
		{
			document.AddMember("IPAddr", rapidjson::Value(IPAddr.c_str(), allocator), allocator);
		}
		if (Status != -1)
		{
			std::string strstatus = "";
			if (Status == 0)
			{
				strstatus = "Ready";
			
			}
			if (Status == 1)
			{
				strstatus = "Busy";
				
			}

			document.AddMember("Status", rapidjson::Value(strstatus.c_str(), allocator), allocator);		
		}

		if(TotalMem>0)
			document.AddMember("TotalMem", rapidjson::Value(std::to_string(TotalMem).c_str(), allocator), allocator);
		
		if (FreeMem > 0)
			document.AddMember("FreeMem", rapidjson::Value(std::to_string(FreeMem).c_str(), allocator), allocator);

		document.AddMember("ProcessId", rapidjson::Value(std::to_string(ProcessId).c_str(), allocator), allocator);
		document.AddMember("TaskFile", rapidjson::Value(TaskFile.c_str(), allocator), allocator);

		document.Accept(writer);
		std::ofstream fileout = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
		if (!fileout.good())
			return false;


		fileout << buffer.GetString();
		fileout.close();
		return true;
	}
};

struct Run_s
{
	std::string RunHostName = "";
	std::string RunUserName = "";
	std::string StartTime = "";
	std::string EndTime = "";

	std::string RunHostName2 = "";
	std::string RunUserName2 = "";
	std::string StartTime2 = "";
	std::string EndTime2 = "";

	Run_s() {};

	bool load(const std::string& file)
	{
		QString logJson = str2qstr(const_cast<std::string &>(file));

		if (!QFile(logJson).exists())
			return false;


		QFile fileLog(logJson);
		if (!fileLog.open(QIODevice::ReadOnly))
		{
			
			return false;
		}

		QByteArray baLog = fileLog.readAll();
		QJsonParseError errLog;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(baLog, &errLog);

		fileLog.close();

		if (errLog.error != QJsonParseError::NoError || jsonDoc.isNull())
		{
			
			return false;
		}

		QJsonObject jsonObj = jsonDoc.object();
		

		RunHostName = qstr2str(jsonObj.value("RunHostName").toString());
		RunUserName = qstr2str(jsonObj.value("RunUserName").toString());
		StartTime = qstr2str(jsonObj.value("StartTime").toString());
		EndTime = qstr2str(jsonObj.value("EndTime").toString());

		RunHostName2 = RunHostName;
		RunUserName2 = RunUserName;
		StartTime2 = StartTime;
		EndTime2 = EndTime;



		return true;


	}

	bool save(const std::string& file)
	{
		if( CheckUsingNoChinesePathVersion())
		{
			try
			{
				nlohmann::json outjson = WriteToJson();

				std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
				if (ofs.fail())
					return false;
				ofs << outjson.dump(4);
				ofs.close();
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return false;
			}
		}
		else
		{
			std::string outjson_str = WriteToJsonV2();
			std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
			if (ofs.fail())
				return false;

			ofs << outjson_str;
			ofs.close();
		}

		return true;
	}

	Run_s(std::string runhostname, std::string runusername,
		std::string starttime, std::string endtime)
	{

		RunHostName = runhostname;
		RunUserName = runusername;
		StartTime = starttime;
		EndTime = endtime;

		RunHostName2 = runhostname;
		RunUserName2 = runusername;
		StartTime2 = starttime;
		EndTime2 = endtime;
	};


	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		if (RunHostName != "")
		{
			json_str["RunHostName"] = RunHostName;
		}
		if (RunUserName != "")
		{
			json_str["RunUserName"] = RunUserName;
		}
		if (StartTime != "")
		{
			json_str["StartTime"] = StartTime;
		}
		if (EndTime != "")
		{
			json_str["EndTime"] = EndTime;
		}	
		
		return json_str;
	}


	std::string WriteToJsonV2()
	{
		rapidjson::Document document;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		document.SetObject();

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();




		if (RunHostName != "")
		{
			document.AddMember("RunHostName", rapidjson::Value(RunHostName.c_str(), allocator), allocator);
		}

		if (RunUserName != "")
		{
			document.AddMember("RunUserName", rapidjson::Value(RunUserName.c_str(), allocator), allocator);
		}

		if (StartTime != "")
		{
			document.AddMember("StartTime", rapidjson::Value(StartTime.c_str(), allocator), allocator);
		}

		if (EndTime != "")
		{
			document.AddMember("EndTime", rapidjson::Value(EndTime.c_str(), allocator), allocator);
		}


		document.Accept(writer);

		return buffer.GetString();
	}

	void WriteToJsonV3(rapidjson::Value &value,rapidjson::Document &doc)
	{
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
		if (!value.IsObject())
			return;

		if (RunHostName != "")
		{
			value.AddMember("RunHostName", rapidjson::Value(RunHostName.c_str(), allocator), allocator);
		}

		if (RunUserName != "")
		{
			value.AddMember("RunUserName", rapidjson::Value(RunUserName.c_str(), allocator), allocator);
		}

		if (StartTime != "")
		{
			value.AddMember("StartTime", rapidjson::Value(StartTime.c_str(), allocator), allocator);
		}

		if (EndTime != "")
		{
			value.AddMember("EndTime", rapidjson::Value(EndTime.c_str(), allocator), allocator);
		}	
	}


	Run_s(nlohmann::json json_str)
	{
		if (json_str.find("RunHostName") != json_str.end())
		{
			RunHostName = json_str.at("RunHostName").get<std::string>();
		}

		if (json_str.find("RunUserName") != json_str.end())
		{
			RunUserName = json_str.at("RunUserName").get<std::string>();
		}
		if (json_str.find("StartTime") != json_str.end())
		{
			StartTime = json_str.at("StartTime").get<std::string>();
		}
		if (json_str.find("EndTime") != json_str.end())
		{ 
			EndTime = json_str.at("EndTime").get<std::string>();
		}
	}

	
	Run_s(std::string json_str,int dummy)
	{
		Run_s jobinfo = Run_s::CreateFromJsonV2(json_str);
		*this = jobinfo;
	}


	static Run_s CreateFromJson(nlohmann::json json_str)
	{
		Run_s jobinfo;
		if (json_str.find("RunHostName") != json_str.end())
		{
			jobinfo.RunHostName = json_str.at("RunHostName").get<std::string>();
		}

		if (json_str.find("RunUserName") != json_str.end())
		{
			jobinfo.RunUserName = json_str.at("RunUserName").get<std::string>();
		}
		if (json_str.find("StartTime") != json_str.end())
		{
			jobinfo.StartTime = json_str.at("StartTime").get<std::string>();
		}
		if (json_str.find("EndTime") != json_str.end())
		{
			jobinfo.EndTime = json_str.at("EndTime").get<std::string>();
		}
		
		return jobinfo;
	}


	static Run_s CreateFromJsonV2(std::string json_str)
	{
		Run_s jobinfo;

		if (json_str.empty())
			return jobinfo;

		rapidjson::Document document;
		if (document.Parse(json_str.data()).HasParseError())
			return jobinfo;

		if (!document.IsObject())
			return jobinfo;




		if (document.HasMember("RunHostName"))
		{
			jobinfo.RunHostName = document["RunHostName"].GetString();
			jobinfo.RunHostName2 = jobinfo.RunHostName;
		}

		if (document.HasMember("RunUserName"))
		{
			jobinfo.RunUserName = document["RunUserName"].GetString();
			jobinfo.RunUserName2 = jobinfo.RunUserName;
		}

		if (document.HasMember("StartTime"))
		{
			jobinfo.StartTime = document["StartTime"].GetString();
			jobinfo.StartTime2 = jobinfo.StartTime;
		}

		if (document.HasMember("EndTime"))
		{
			jobinfo.EndTime = document["EndTime"].GetString();
			jobinfo.EndTime2 = jobinfo.EndTime;
		}


		return jobinfo;
	}

	static Run_s CreateFromJsonV3(const rapidjson::Value &value)
	{
		Run_s jobinfo;
		if (!value.IsObject())
			return jobinfo;

		if (value.HasMember("RunHostName"))
		{
			jobinfo.RunHostName = value["RunHostName"].GetString();
			jobinfo.RunHostName2 = jobinfo.RunHostName;
		}

		if (value.HasMember("RunUserName"))
		{
			jobinfo.RunUserName = value["RunUserName"].GetString();
			jobinfo.RunUserName2 = jobinfo.RunUserName;
		}

		if (value.HasMember("StartTime"))
		{
			jobinfo.StartTime = value["StartTime"].GetString();
			jobinfo.StartTime2 = jobinfo.StartTime;
		}

		if (value.HasMember("EndTime"))
		{
			jobinfo.EndTime = value["EndTime"].GetString();
			jobinfo.EndTime2 = jobinfo.EndTime;
		}

		return jobinfo;
	}

};


struct RunInfo_s
{
	std::string SubmitHostName = "";
	std::string SubmitUser = "";
	std::string SubmitTime = "";
	std::string EndTime = "";

	std::string SubmitHostName2 = "";
	std::string SubmitUser2 = "";
	std::string SubmitTime2 = "";
	std::string EndTime2 = "";

	Run_s runninginfo;
	
	RunInfo_s() {};

	RunInfo_s(std::string submithostname, std::string submituser, std::string submittime) 
	{
		SubmitHostName = submithostname;
		SubmitUser = submituser;
		SubmitTime = submittime;

		SubmitHostName2 = submithostname;
		SubmitUser2 = submituser;
		SubmitTime2 = submittime;

	};

	void SetPendingInfo(std::string submithostname, std::string submituser,std::string submittime) 
	{
		SubmitHostName = submithostname;
		SubmitUser = submituser;
		SubmitTime = submittime;

		SubmitHostName2 = submithostname;
		SubmitUser2 = submituser;
		SubmitTime2 = submittime;
	};


	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		if (SubmitHostName != "")
		{
			json_str["SubmitHostName"] = SubmitHostName;
		}
		if (SubmitUser != "")
		{
			json_str["SubmitUser"] = SubmitUser;
		}
		if (SubmitTime != "")
		{
			json_str["SubmitTime"] = SubmitTime;
		}
		if (EndTime != "")
		{
			json_str["EndTime"] = EndTime;
		}

		if (runninginfo.RunHostName != "")
		{
			json_str["Run"] = runninginfo.WriteToJson();
		}

		return json_str;
	}


	std::string WriteToJsonV2()
	{
		std::string json_str;
		rapidjson::Document document;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		document.SetObject();

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();




		if (SubmitHostName != "")
		{
			document.AddMember("SubmitHostName", rapidjson::Value(SubmitHostName.c_str(), allocator), allocator);
		}

		if (SubmitUser != "")
		{
			document.AddMember("SubmitUser", rapidjson::Value(SubmitUser.c_str(),allocator), allocator);
		}

		if (SubmitTime != "")
		{
			document.AddMember("SubmitTime", rapidjson::Value(SubmitTime.c_str(),allocator), allocator);
		}

		if (EndTime != "")
		{
			document.AddMember("EndTime", rapidjson::Value(EndTime.c_str(), allocator), allocator);
		}

		if (runninginfo.RunHostName != "")
		{

			rapidjson::Value documentRun(rapidjson::kObjectType);
			if (runninginfo.RunHostName != "")
			{
				documentRun.AddMember("RunHostName", rapidjson::Value(runninginfo.RunHostName.c_str(), allocator), allocator);
			}

			if (runninginfo.RunUserName != "")
			{
				documentRun.AddMember("RunUserName", rapidjson::Value(runninginfo.RunUserName.c_str(), allocator), allocator);
			}

			if (runninginfo.StartTime != "")
			{
				documentRun.AddMember("StartTime", rapidjson::Value(runninginfo.StartTime.c_str(), allocator), allocator);
			}

			if (runninginfo.EndTime != "")
			{
				documentRun.AddMember("EndTime", rapidjson::Value(runninginfo.EndTime.c_str(), allocator), allocator);
			}

			document.AddMember("Run", documentRun, allocator);

		}

		document.Accept(writer);
		json_str = buffer.GetString();

		return json_str;		
	}

	
	void WriteToJsonV3(rapidjson::Value& document, rapidjson::Document& doc)
	{
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
		if (!document.IsObject() || !doc.IsObject())
			return;

		if (SubmitHostName != "")
		{
			document.AddMember("SubmitHostName", rapidjson::Value(SubmitHostName.c_str(), allocator), allocator);
		}

		if (SubmitUser != "")
		{
			document.AddMember("SubmitUser", rapidjson::Value(SubmitUser.c_str(), allocator), allocator);
		}

		if (SubmitTime != "")
		{
			document.AddMember("SubmitTime", rapidjson::Value(SubmitTime.c_str(), allocator), allocator);
		}

		if (EndTime != "")
		{
			document.AddMember("EndTime", rapidjson::Value(EndTime.c_str(), allocator), allocator);
		}

		if (runninginfo.RunHostName != "")
		{
			
			rapidjson::Value valRun(rapidjson::kObjectType);
			runninginfo.WriteToJsonV3(valRun, doc);
			document.AddMember("Run", valRun, allocator);
		}
	}



	static RunInfo_s CreateFromJson(nlohmann::json json_str)
	{
		RunInfo_s jobinfo;
		if (json_str.find("SubmitHostName") != json_str.end())
		{
			jobinfo.SubmitHostName = json_str.at("SubmitHostName").get<std::string>();
		}

		if (json_str.find("SubmitUser") != json_str.end())
		{
			jobinfo.SubmitUser = json_str.at("SubmitUser").get<std::string>();
		}
		if (json_str.find("SubmitTime") != json_str.end())
		{
			jobinfo.SubmitTime = json_str.at("SubmitTime").get<std::string>();
		}
		if (json_str.find("EndTime") != json_str.end())
		{
			jobinfo.EndTime = json_str.at("EndTime").get<std::string>();
		}

		if (json_str.find("Run") != json_str.end())
			jobinfo.runninginfo = Run_s::CreateFromJson(json_str.at("Run"));

		return jobinfo;
	}


	static RunInfo_s CreateFromJsonV2(std::string json_str)
	{
		RunInfo_s jobinfo;
		if (json_str.empty())
			return jobinfo;

		rapidjson::Document document;
		if (document.Parse(json_str.data()).HasParseError())
			return jobinfo;

		if (!document.IsObject())
			return jobinfo;




		if (document.HasMember("SubmitHostName"))
		{
			jobinfo.SubmitHostName = document["SubmitHostName"].GetString();
			jobinfo.SubmitHostName2 = jobinfo.SubmitHostName;
		}

		if (document.HasMember("SubmitUser"))
		{
			jobinfo.SubmitUser = document["SubmitUser"].GetString();
			jobinfo.SubmitUser2 = jobinfo.SubmitUser;
		}

		if (document.HasMember("SubmitTime"))
		{
			jobinfo.SubmitTime = document["SubmitTime"].GetString();
			jobinfo.SubmitTime2 = jobinfo.SubmitTime;
		}

		if (document.HasMember("EndTime"))
		{
			jobinfo.EndTime = document["EndTime"].GetString();
			jobinfo.EndTime2 = jobinfo.EndTime;
		}

		if (document.HasMember("Run"))
		{
#if 0
			
			
			rapidjson::Value& documentRun = document["Run"];
			jobinfo.runninginfo = Run_s::CreateFromJsonV3(documentRun);
#else
			rapidjson::Value& documentRun = document["Run"];

			if (documentRun.HasMember("RunHostName"))
			{
				jobinfo.runninginfo.RunHostName = documentRun["RunHostName"].GetString();
				jobinfo.runninginfo.RunHostName2 = jobinfo.runninginfo.RunHostName;
			}

			if (documentRun.HasMember("RunUserName"))
			{
				jobinfo.runninginfo.RunUserName = documentRun["RunUserName"].GetString();
				jobinfo.runninginfo.RunUserName2 = jobinfo.runninginfo.RunUserName;
			}

			if (documentRun.HasMember("StartTime"))
			{
				jobinfo.runninginfo.StartTime = documentRun["StartTime"].GetString();
				jobinfo.runninginfo.StartTime2 = jobinfo.runninginfo.StartTime;
			}

			if (documentRun.HasMember("EndTime"))
			{
				jobinfo.runninginfo.EndTime = documentRun["EndTime"].GetString();
				jobinfo.runninginfo.EndTime2 = jobinfo.runninginfo.EndTime;
			}
#endif

		}


		return jobinfo;
	}

	static RunInfo_s CreateFromJsonV3(rapidjson::Value &value)
	{
		RunInfo_s jobinfo;
		if (!value.IsObject())
			return jobinfo;

		if (value.HasMember("SubmitHostName"))
		{
			jobinfo.SubmitHostName = value["SubmitHostName"].GetString();
			jobinfo.SubmitHostName2 = jobinfo.SubmitHostName;
		}

		if (value.HasMember("SubmitUser"))
		{
			jobinfo.SubmitUser = value["SubmitUser"].GetString();
			jobinfo.SubmitUser2 = jobinfo.SubmitUser;
		}

		if (value.HasMember("SubmitTime"))
		{
			jobinfo.SubmitTime = value["SubmitTime"].GetString();
			jobinfo.SubmitTime2 = jobinfo.SubmitTime;
		}

		if (value.HasMember("EndTime"))
		{
			jobinfo.EndTime = value["EndTime"].GetString();
			jobinfo.EndTime2 = jobinfo.EndTime;
		}

		if (value.HasMember("Run"))
		{

			rapidjson::Value& runValue = value["Run"];
			jobinfo.runninginfo = Run_s::CreateFromJsonV3(runValue);
		}

		return jobinfo;
	}



	RunInfo_s(nlohmann::json json_str)
	{
		
		if (json_str.find("SubmitHostName") != json_str.end())
		{
			SubmitHostName = json_str.at("SubmitHostName").get<std::string>();
		}

		if (json_str.find("SubmitUser") != json_str.end())
		{
			SubmitUser = json_str.at("SubmitUser").get<std::string>();
		}
		if (json_str.find("SubmitTime") != json_str.end())
		{
			SubmitTime = json_str.at("SubmitTime").get<std::string>();
		}

		if (json_str.find("EndTime") != json_str.end())
		{
			EndTime = json_str.at("EndTime").get<std::string>();
		}	
	}


	RunInfo_s(std::string json_str,int dummy)
	{
		RunInfo_s runInfo = RunInfo_s::CreateFromJsonV2(json_str);
		*this = runInfo;
	}
};


struct JobFeedBack_s
{
	jobsta_e Status = jobsta_e::STATUS_PENDDING;
	float Percent = 0.0f;
	std::string Msg = ""; 
	std::string Msg2 = ""; 

	JobFeedBack_s(){};
	int TaskRetVal = -1;


	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		json_str["Status"] = Status;
		json_str["Percent"] = Percent;
		json_str["Msg"] = Msg;
		if (TaskRetVal != -1)
		{
			json_str["TaskRetVal"] = TaskRetVal;
		}
		return json_str;
	}


	std::string WriteToJsonV2()
	{
		std::string json_str;
		
		rapidjson::Document document;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		document.SetObject();	
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();




		document.AddMember("Status", rapidjson::Value((int)Status), allocator);
		document.AddMember("Percent",rapidjson::Value(Percent),allocator);
		document.AddMember("TaskRetVal",rapidjson::Value(TaskRetVal),allocator);
		if (Msg != "")
		{
			document.AddMember("Msg", rapidjson::Value(Msg.data(), allocator), allocator);
		}

		document.Accept(writer);
		json_str = buffer.GetString();

		return json_str;
	}

	void WriteToJsonV3(rapidjson::Value &document,rapidjson::Document &doc)
	{
		if (!document.IsObject() || !doc.IsObject())
			return;

		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

		document.AddMember("Status", rapidjson::Value((int)Status), allocator);
		document.AddMember("Percent", rapidjson::Value(Percent), allocator);
		document.AddMember("TaskRetVal", rapidjson::Value(TaskRetVal), allocator);
		if (Msg != "")
		{
			document.AddMember("Msg", rapidjson::Value(Msg.data(), allocator), allocator);
		}
	}



	static JobFeedBack_s CreateFromJson(nlohmann::json json_str)
	{
		JobFeedBack_s jobinfo;
		jobinfo.Status = json_str.at("Status");
		jobinfo.Percent = json_str.at("Percent");
		if (json_str.find("TaskRetVal") != json_str.end())
		{
			jobinfo.TaskRetVal = json_str.at("TaskRetVal");
		}
		jobinfo.Msg = json_str.at("Msg").get<std::string>();
		return jobinfo;
	}


	static JobFeedBack_s CreateFromJsonV2(std::string json_str)
	{
		JobFeedBack_s jobinfo;
		rapidjson::Document document;

		if (json_str.empty())
			return jobinfo;

		if (document.Parse(json_str.data()).HasParseError())
			return jobinfo;

		if (!document.IsObject())
			return jobinfo;




		if (document.HasMember("Status"))
		{
			jobinfo.Status = (jobsta_e)document["Status"].GetInt();
		}

		if (document.HasMember("Percent"))
		{
			jobinfo.Percent = document["Percent"].GetFloat();
		}

		if (document.HasMember("TaskRetVal"))
		{
			jobinfo.TaskRetVal = document["TaskRetVal"].GetInt();
		}

		if (document.HasMember("Msg"))
		{
			jobinfo.Msg = document["Msg"].GetString();
			// jobinfo.Msg2 = UTF82GBK(jobinfo.Msg);
			jobinfo.Msg2 = jobinfo.Msg;
		}


		return jobinfo;
	}

	static JobFeedBack_s CreateFromJsonV3(rapidjson::Value &value)
	{
		JobFeedBack_s jobinfo;
		if (!value.IsObject())
			return jobinfo;

		if (value.HasMember("Status"))
		{
			jobinfo.Status = (jobsta_e)value["Status"].GetInt();
		}

		if (value.HasMember("Percent"))
		{
			jobinfo.Percent = value["Percent"].GetFloat();
		}

		if (value.HasMember("TaskRetVal"))
		{
			jobinfo.TaskRetVal = value["TaskRetVal"].GetInt();
		}

		if (value.HasMember("Msg"))
		{
			jobinfo.Msg = value["Msg"].GetString();
			// jobinfo.Msg2 = UTF82GBK(jobinfo.Msg);
			jobinfo.Msg2 = jobinfo.Msg;
		}

		return jobinfo;
	}



	JobFeedBack_s(nlohmann::json json_str)
	{	
		Status = json_str.at("Status");
		Percent = json_str.at("Percent");
		Msg = json_str.at("Msg").get<std::string>();
		if (json_str.find("TaskRetVal") != json_str.end())
		{
			TaskRetVal = json_str.at("TaskRetVal");
		}	
	}


	JobFeedBack_s(std::string json_str,int dummy)
	{
		JobFeedBack_s feedbackInfo = JobFeedBack_s::CreateFromJsonV2(json_str);
		*this = feedbackInfo;
	}

	JobFeedBack_s(const std::string& file)
	{
		load(file);
	}
	
	bool load(const std::string& file)
	{	
		if (JOB_FEEDBACK_USE_BIN) {
			bool result = LoadFeedbackBin(file);
			if (!result) {
				LOGE("Save feedback bin failed!");
			}
		}
		else {
			std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
			if (!ifs)
			{
				return false;
			}
			if (ifs.fail())
			{
				return false;
			}
			std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			if (str == "")
				return false;

			if (CheckUsingNoChinesePathVersion())
			{
				try
				{
					JobFeedBack_s feedbakinfo(nlohmann::json::parse(str.begin(), str.end()));
					
					Status = feedbakinfo.Status;
					Percent = feedbakinfo.Percent;

					TaskRetVal = feedbakinfo.TaskRetVal;
					Msg = feedbakinfo.Msg;
				}
				catch (std::exception ex)
				{
					ifs.close();
					return false;
				}
			}
			else
			{
				JobFeedBack_s feedbackinfo = JobFeedBack_s::CreateFromJsonV2(str);
				Status = feedbackinfo.Status;
				Percent = feedbackinfo.Percent;
				TaskRetVal = feedbackinfo.TaskRetVal;
				Msg = feedbackinfo.Msg;
			}

			ifs.close();
		}
		
		return true;	
	}

	
	bool load_with_retry(const std::string& file, bool bInsideUI = true)
	{
		int retryTimes = 0;
		FILE* fpLock = NULL;

		do
		{
			{
				fpLock = AI3D::CORE::File::FopenDenyWriteLockUtf8(file + ".lock");
				if (fpLock == NULL)
				{
					goto load_with_retry_loop;
				}
				if (JOB_FEEDBACK_USE_BIN) {
					bool result = LoadFeedbackBin(file);
					if (!result) {
						LOGE("load feedback bin failed!");
						goto load_with_retry_loop;
					}
					
				}
				else {
					std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
					if (!ifs)
					{
						
						goto load_with_retry_loop;
					}

					if (ifs.fail())
					{
						
						
						goto load_with_retry_loop;
					}


					std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
					if (str == "")
					{
						
						ifs.close();
						goto load_with_retry_loop;
					}

					if (CheckUsingNoChinesePathVersion())
					{
						try
						{
							JobFeedBack_s feedbakinfo(nlohmann::json::parse(str.begin(), str.end()));
							
							Status = feedbakinfo.Status;
							Percent = feedbakinfo.Percent;

							TaskRetVal = feedbakinfo.TaskRetVal;
							Msg = feedbakinfo.Msg;
						}
						catch (std::exception ex)
						{
							ifs.close();
							
							goto load_with_retry_loop;
						}
					}
					else
					{
						JobFeedBack_s feedbackinfo = JobFeedBack_s::CreateFromJsonV2(str);
						Status = feedbackinfo.Status;
						Percent = feedbackinfo.Percent;
						TaskRetVal = feedbackinfo.TaskRetVal;
						Msg = feedbackinfo.Msg;
					}

					ifs.close();
				}

				
			}
			fclose(fpLock);
			break;

		load_with_retry_loop:
			if (fpLock != NULL)
				fclose(fpLock);

			retryTimes++;

			if (bInsideUI)
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		} while (retryTimes < 3);
		if (retryTimes >= 3)
			return false;

		return true;
	}

	bool LoadFeedbackBin(std::string file) {
		std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::binary);
		
		if (!in.is_open())
			return false;

		FeedBackFile feedBackFile;
		feedBackFile.Deserialize(in);
		Status = (jobsta_e)feedBackFile.feedBackData.status;
		Percent = feedBackFile.feedBackData.percent;
		TaskRetVal = feedBackFile.feedBackData.taskRetVal;
		std::string msg = feedBackFile.feedBackData.msg;
#ifdef WIN32
		// msg = UTF82GBK(msg);
#endif 
		Msg = msg;
		
		in.close();
		return true;
	}

	bool load_with_retry2(const std::string& file, bool bInsideUI = true)
	{
		int retryTimes = 0;
		

		do
		{
			{


				if (JOB_FEEDBACK_USE_BIN) {
					bool result = LoadFeedbackBin(file);
					if (!result) {
						LOGE("Save feedback bin failed!");
						goto load_with_retry_loop;
					}
				}
				else {

					std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
					if (!ifs)
					{
						
						goto load_with_retry_loop;
					}

					if (ifs.fail())
					{
						
						
						goto load_with_retry_loop;
					}


					std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
					if (str == "")
					{
						
						ifs.close();
						goto load_with_retry_loop;
					}

					if (CheckUsingNoChinesePathVersion())
					{
						try
						{
							JobFeedBack_s feedbakinfo(nlohmann::json::parse(str.begin(), str.end()));
							
							Status = feedbakinfo.Status;
							Percent = feedbakinfo.Percent;

							TaskRetVal = feedbakinfo.TaskRetVal;
							Msg = feedbakinfo.Msg;
						}
						catch (std::exception ex)
						{
							ifs.close();
							
							goto load_with_retry_loop;
						}
					}
					else
					{
						JobFeedBack_s feedbackinfo = JobFeedBack_s::CreateFromJsonV2(str);
						Status = feedbackinfo.Status;
						Percent = feedbackinfo.Percent;
						TaskRetVal = feedbackinfo.TaskRetVal;
						Msg = feedbackinfo.Msg;
					}

					ifs.close();
				}
			}

			
			break;

		load_with_retry_loop:
			
			

			retryTimes++;

		} while (retryTimes < 1);

		if (retryTimes >= 1)
			return false;

		return true;
	}

	bool save(const std::string& file)
	{
		if (JOB_FEEDBACK_USE_BIN) {
			bool result = WriteToBin(file);
			if (!result) {
				LOGE("Save feedback bin failed!");
			}
		}
		else {
			if ( CheckUsingNoChinesePathVersion())
			{
				try
				{
					std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
					if (ofs.fail())
						return false;

					nlohmann::json outjson = WriteToJson();
					ofs << outjson.dump(4);

					ofs.close();
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					return false;
				}
			}
			else
			{
				std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
				if (ofs.fail())
					return false;

				std::string outjson_str = WriteToJsonV2();
				ofs << outjson_str;

				ofs.close();
			}
		}
		

		return true;
	}

	
	bool save_with_retry(const std::string& file,bool bInsideUI = true)
	{
		
		if( CheckUsingNoChinesePathVersion())
		{
			try
			{
				if (JOB_FEEDBACK_USE_BIN) {
					
					int retryTimes = 0;
					do
					{
						FILE* fpLock = AI3D::CORE::File::FopenDenyWriteLockUtf8(file + ".lock");
						if (fpLock == NULL)
						{
							retryTimes++;
							if (bInsideUI)
								std::this_thread::sleep_for(std::chrono::milliseconds(200));
							else
								std::this_thread::sleep_for(std::chrono::milliseconds(2000));

							continue;
						}

						bool result = WriteToBin(file);
						if (!result) {
							LOGE("Save feedback bin failed!");							
						}
						fclose(fpLock);
						break;
					} while (retryTimes < 3);

					if (retryTimes >= 3)
						return false;
				}
				else {
					
					nlohmann::json outjson;
					outjson = WriteToJson();
					int retryTimes = 0;

					do
					{
						FILE* fpLock = AI3D::CORE::File::FopenDenyWriteLockUtf8(file + ".lock");
						if (fpLock == NULL)
						{
							retryTimes++;
							if (bInsideUI)
								std::this_thread::sleep_for(std::chrono::milliseconds(200));
							else
								std::this_thread::sleep_for(std::chrono::milliseconds(2000));

							continue;
						}

						std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
						if (ofs.fail())
						{
							fclose(fpLock);
							return false;
						}

						ofs << outjson.dump(4);
						ofs.close();

						fclose(fpLock);
						break;
					} while (retryTimes < 3);

					if (retryTimes >= 3)
						return false;
				}
				
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return false;
			}
		}
		else
		{
			if (JOB_FEEDBACK_USE_BIN) {
				
				int retryTimes = 0;
				do
				{
					FILE* fpLock = AI3D::CORE::File::FopenDenyWriteLockUtf8(file + ".lock");
					if (fpLock == NULL)
					{
						retryTimes++;
						if (bInsideUI)
							std::this_thread::sleep_for(std::chrono::milliseconds(200));
						else
							std::this_thread::sleep_for(std::chrono::milliseconds(2000));

						continue;
					}

					bool result = WriteToBin(file);
					if (!result) {
						LOGE("Save feedback bin failed!");
					}
					fclose(fpLock);
					break;
				} while (retryTimes < 3);

				if (retryTimes >= 3)
					return false;
			}
			else {
				std::string outjson_str;
				outjson_str = WriteToJsonV2();
				int retryTimes = 0;

				do
				{
					FILE* fpLock = AI3D::CORE::File::FopenDenyWriteLockUtf8(file + ".lock");
					if (fpLock == NULL)
					{
						retryTimes++;
						if (bInsideUI)
							std::this_thread::sleep_for(std::chrono::milliseconds(200));
						else
							std::this_thread::sleep_for(std::chrono::milliseconds(2000));

						continue;
					}

					std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
					if (ofs.fail())
					{
						fclose(fpLock);
						return false;
					}

					ofs << outjson_str;
					ofs.close();

					fclose(fpLock);
					break;
				} while (retryTimes < 3);

				if (retryTimes >= 3)
					return false;
			}
		}

		return true;
	}

	bool WriteToBin(std::string file) {
		std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::binary);
		if (!out.is_open()) {
			LOGE("Save feedback bin failed!");
			return false;
		}
		FeedBackFile feedBackFile;
		feedBackFile.feedBackData.status = (int)Status;
		feedBackFile.feedBackData.percent = Percent;
		feedBackFile.feedBackData.taskRetVal = TaskRetVal;
		std::string msg = Msg;
#ifdef WIN32
		// msg = GBK2UTF8(msg);
#endif 
		feedBackFile.feedBackData.msg = msg;

		feedBackFile.Serialize(out);

		out.close();
		return true;
	}
	
};

struct Task_s
{
	std::string Msg ;
	std::string Msg2;

	float Percent = 0;
	int Status = 0;
	int Type = 1;

	std::string  ProjectPath = "";
	std::string ItemPath = "";

	std::string  ProjectPath2 = "";
	std::string ItemPath2 = "";

	int Id = -1;
	int FatherId = -1;
	std::set<int> Depends;
	Run_s runinfo;

	void SetRunningInfo(Run_s _runinfo)
	{
		Status = int(job_status_e::STATUS_RUNNING);
		runinfo = _runinfo;
	}


	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		json_str["Status"] = Status;
		json_str["Percent"] = Percent;
		json_str["Msg"] = Msg;
		json_str["Type"] = Type;
		json_str["ProjectPath"] = ProjectPath;
		json_str["ItemPath"] = ItemPath;
		json_str["Id"] = Id;
		if (runinfo.StartTime != "")
		{			
			json_str["Run"] = runinfo.WriteToJson();
		}

		if (FatherId != -1)
		{
			json_str["FatherId"] = FatherId;
		}
		
		if(!Depends.empty())
			json_str["Depends"] = Depends;
		return json_str;
	}


	std::string WriteToJson2()
	{
		std::string json_str;
		rapidjson::Document document;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		document.SetObject();
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();




		document.AddMember("Status",rapidjson::Value(Status),allocator);
		document.AddMember("Percent", rapidjson::Value(Percent), allocator);
		document.AddMember("Msg", rapidjson::Value(Msg.data(),allocator), allocator);
		document.AddMember("Type",rapidjson::Value(Type),allocator);
		document.AddMember("ProjectPath", rapidjson::Value(ProjectPath.data(),allocator), allocator);
		document.AddMember("ItemPath", rapidjson::Value(ItemPath.data(), allocator), allocator);
		document.AddMember("Id",rapidjson::Value(Id),allocator);
		if (runinfo.StartTime != "")
		{
			

			
			
			
			
			
#if 0		
			rapidjson::Value runInfoValue(rapidjson::kObjectType);
			runinfo.WriteToJsonV3(runInfoValue, document);
			document.AddMember("Run", runInfoValue, allocator);
#else
			rapidjson::Value documentRun(rapidjson::kObjectType);
			if (runinfo.RunHostName != "")
			{
				documentRun.AddMember("RunHostName", rapidjson::Value(runinfo.RunHostName.c_str(), allocator), allocator);
			}

			if (runinfo.RunUserName != "")
			{
				documentRun.AddMember("RunUserName", rapidjson::Value(runinfo.RunUserName.c_str(), allocator), allocator);
			}

			if (runinfo.StartTime != "")
			{
				documentRun.AddMember("StartTime", rapidjson::Value(runinfo.StartTime.c_str(), allocator), allocator);
			}

			if (runinfo.EndTime != "")
			{
				documentRun.AddMember("EndTime", rapidjson::Value(runinfo.EndTime.c_str(), allocator), allocator);
			}
			document.AddMember("Run", documentRun, allocator);
#endif
		}

		if (FatherId != -1)
		{
			document.AddMember("FatherId", rapidjson::Value(FatherId), allocator);
		}

		if (!Depends.empty())
		{


			
			rapidjson::Value valDepends(rapidjson::kArrayType);
			for (auto depId : Depends)
			{
				valDepends.PushBack(rapidjson::Value(depId),allocator);
			}
			document.AddMember("Depends", valDepends, allocator);
		}

		document.Accept(writer); 
		json_str = buffer.GetString();
		std::cout << __FILE__ << __FUNCTION__ << json_str << std::endl;

		return json_str;	
	}

	void WriteToJson3(rapidjson::Value &document,rapidjson::Document &doc)
	{
		if (!document.IsObject() || !doc.IsObject())
			return;

		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

		document.AddMember("Status", rapidjson::Value(Status), allocator);
		document.AddMember("Percent", rapidjson::Value(Percent), allocator);
		document.AddMember("Msg", rapidjson::Value(Msg.data(), allocator), allocator);
		document.AddMember("Type", rapidjson::Value(Type), allocator);
		document.AddMember("ProjectPath", rapidjson::Value(ProjectPath.data(), allocator), allocator);
		document.AddMember("ItemPath", rapidjson::Value(ItemPath.data(), allocator), allocator);
		document.AddMember("Id", rapidjson::Value(Id), allocator);

		if (runinfo.StartTime != "")
		{
			
			rapidjson::Value runInfoValue(rapidjson::kObjectType);
			runinfo.WriteToJsonV3(runInfoValue, doc);
			document.AddMember("Run", runInfoValue, allocator);
		}

		if (FatherId != -1)
		{
			document.AddMember("FatherId", rapidjson::Value(FatherId), allocator);
		}

		if (!Depends.empty())
		{
			
			
			
			rapidjson::Value valDepends(rapidjson::kArrayType);
			for (auto depId : Depends)
			{
				valDepends.PushBack(rapidjson::Value(depId), allocator);
			}

			document.AddMember("Depends", valDepends, allocator);
		}
	}
	

	static Task_s CreateFromJson(nlohmann::json json_str)
	{
		Task_s jobinfo;
		jobinfo.Status = json_str.at("Status");
		jobinfo.Percent = json_str.at("Percent");
		jobinfo.Msg = json_str.at("Msg").get<std::string>();
		jobinfo.ProjectPath = json_str.at("ProjectPath").get<std::string>();
		jobinfo.ItemPath = json_str.at("ItemPath").get<std::string>();
		jobinfo.Id = json_str.at("Id");
		jobinfo.Type = json_str.at("Type");
		if (json_str.find("Run") != json_str.end())
		{
			
			jobinfo.runinfo =  Run_s::CreateFromJson(json_str.at("Run"));
		}

		if (json_str.find("FatherId") != json_str.end())
		{
			jobinfo.FatherId = json_str.at("FatherId");
			
		}

		if (json_str.find("Depends") != json_str.end())
		{
			nlohmann::json depend_str = json_str.at("Depends");
			
			jobinfo.Depends.clear();
			for (auto i = 0; i < depend_str.size(); i++)
			{
				
				jobinfo.Depends.insert(depend_str[i].get<int>());
			}
		
		}
		return jobinfo;
	}


	static Task_s CreateFromJsonV2(std::string json_str)
	{
		Task_s jobinfo;
		rapidjson::Document document;

		std::cout << __FILE__ << __FUNCTION__ << json_str << std::endl;

		if (json_str.empty())
			return jobinfo;

		if (document.Parse(json_str.data()).HasParseError())
			return jobinfo;




		if (document.HasMember("Status"))
		{
			jobinfo.Status = document["Status"].GetInt();
		}

		if (document.HasMember("Percent"))
		{
			jobinfo.Percent = document["Percent"].GetInt();
		}

		if (document.HasMember("Msg"))
		{
			jobinfo.Msg = document["Msg"].GetString();
			// jobinfo.Msg2 = UTF82GBK(jobinfo.Msg);
			jobinfo.Msg2 = jobinfo.Msg;
		}

		if (document.HasMember("ProjectPath"))
		{
			jobinfo.ProjectPath = document["ProjectPath"].GetString();
			// jobinfo.ProjectPath2 = UTF82GBK(jobinfo.ProjectPath);
			jobinfo.ProjectPath2 = jobinfo.ProjectPath;
		}

		if (document.HasMember("ItemPath"))
		{
			jobinfo.ItemPath = document["ItemPath"].GetString();
			// jobinfo.ItemPath2 = UTF82GBK(jobinfo.ItemPath);
			jobinfo.ItemPath2 = jobinfo.ItemPath;
		}

		if (document.HasMember("Id"))
		{
			jobinfo.Id = document["Id"].GetInt();
		}

		if (document.HasMember("Type"))
		{
			jobinfo.Type = document["Type"].GetInt();
		}

		
		if (document.HasMember("Run"))
		{
#if 0		
			const rapidjson::Value& run = document["Run"];
			if (run.IsObject())
			{
				jobinfo.runinfo = Run_s::CreateFromJsonV3(run);
			}
#else
			const rapidjson::Value& documentRun = document["Run"];
			if (documentRun.HasMember("RunHostName"))
			{
				jobinfo.runinfo.RunHostName = documentRun["RunHostName"].GetString();
				jobinfo.runinfo.RunHostName2 = jobinfo.runinfo.RunHostName;
			}

			if (documentRun.HasMember("RunUserName"))
			{
				jobinfo.runinfo.RunUserName = documentRun["RunUserName"].GetString();
				jobinfo.runinfo.RunUserName2 = jobinfo.runinfo.RunUserName;
			}

			if (documentRun.HasMember("StartTime"))
			{
				jobinfo.runinfo.StartTime = documentRun["StartTime"].GetString();
				jobinfo.runinfo.StartTime2 = jobinfo.runinfo.StartTime;
			}

			if (documentRun.HasMember("EndTime"))
			{
				jobinfo.runinfo.EndTime = documentRun["EndTime"].GetString();
				jobinfo.runinfo.EndTime2 = jobinfo.runinfo.EndTime;
			}
#endif
		}

		if (document.HasMember("FatherId"))
		{
			jobinfo.FatherId = document["FatherId"].GetInt();
		}

		if (document.HasMember("Depends"))
		{
			const rapidjson::Value& dependsValue = document["Depends"];
			if (dependsValue.IsArray())
			{
				jobinfo.Depends.clear();
				for (unsigned i = 0; i < dependsValue.Size(); i++)
				{
					const rapidjson::Value& val = dependsValue[i];
					if (val.IsInt())
					{
						jobinfo.Depends.insert(val.GetInt());
					}
				}
			}
		}

		return jobinfo;
	}
	
	static Task_s CreateFromJsonV3(rapidjson::Value &value)
	{
		Task_s jobinfo;

		if (!value.IsObject())
			return jobinfo;

		if (value.HasMember("Status"))
		{
			jobinfo.Status = value["Status"].GetInt();
		}

		if (value.HasMember("Percent"))
		{
			jobinfo.Percent = value["Percent"].GetFloat();
		}

		if (value.HasMember("Msg"))
		{
			jobinfo.Msg = value["Msg"].GetString();
			// jobinfo.Msg2 = UTF82GBK(jobinfo.Msg);
			jobinfo.Msg2 = jobinfo.Msg;
		}

		if (value.HasMember("ProjectPath"))
		{
			jobinfo.ProjectPath = value["ProjectPath"].GetString();
			// jobinfo.ProjectPath2 = UTF82GBK(jobinfo.ProjectPath);
			jobinfo.ProjectPath2 = jobinfo.ProjectPath;
		}

		if (value.HasMember("ItemPath"))
		{
			jobinfo.ItemPath = value["ItemPath"].GetString();
			// jobinfo.ItemPath2 = UTF82GBK(jobinfo.ItemPath);
			jobinfo.ItemPath2 = jobinfo.ItemPath;
		}

		if (value.HasMember("Id"))
		{
			jobinfo.Id = value["Id"].GetInt();
		}

		if (value.HasMember("Type"))
		{
			jobinfo.Type = value["Type"].GetInt();
		}

		if (value.HasMember("FatherId"))
		{
			jobinfo.FatherId = value["FatherId"].GetInt();
		}

		if (value.HasMember("Run"))
		{
			
			rapidjson::Value& runValue = value["Run"];
			if (runValue.IsObject())
			{
				jobinfo.runinfo = Run_s::CreateFromJsonV3(runValue);
			}
		}

		if (value.HasMember("Depends"))
		{
			
			
			
			
			
			jobinfo.Depends.clear();
			rapidjson::Value& dependsVal = value["Depends"];
			if (dependsVal.IsArray())
			{
				for (unsigned i = 0; i < dependsVal.Size(); i++)
				{
					rapidjson::Value& val = dependsVal[i];
					jobinfo.Depends.insert(val.GetInt());
				}
			}
		}

		return jobinfo;
	}
};

struct ATTimeSummary_s
{
	ATTimeSummary_s() {};
	RunInfo_s runinfo;
	struct TaskTime_s
	{
		TaskTime_s() {};
		int Id = -1;
		int Type =-1 ;

		std::string FunctionName = "";
		std::string StartTime = "";
		std::string EndTime = "";

		std::string FunctionName2 = "";
		std::string StartTime2 = "";
		std::string EndTime2 = "";

		int Status = 0;
		

		nlohmann::json WriteToJson()
		{
			nlohmann::json json_str;

			if (Id != -1)
				json_str["Id"] = Id;
			if (Type != -1)
				json_str["Type"] = Type;
			
			json_str["Status"] = Status;
			if (FunctionName!="")
			{
				json_str["FunctionName"] = FunctionName;
			}
			if (StartTime != "")
			{
				json_str["StartTime"] = StartTime;
			}
			if (EndTime != "")
			{
				json_str["EndTime"] = EndTime;
			}
			return json_str;
		}


		std::string WriteToJsonV2()
		{
			std::string json_str;
			rapidjson::Document document;
			document.SetObject();
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();




			if (Id != -1)
			{
				document.AddMember("Id",rapidjson::Value(Id),allocator);
			}

			if (Type != -1)
			{
				document.AddMember("Type",rapidjson::Value(Type),allocator);
			}

			document.AddMember("Status",rapidjson::Value(Status),allocator);

			if (FunctionName != "")
			{
				document.AddMember("FunctionName",rapidjson::Value(FunctionName.data(),allocator),allocator);
			}

			if (StartTime != "")
			{
				document.AddMember("StartTime",rapidjson::Value(StartTime.data(),allocator),allocator);
			}

			if (EndTime != "")
			{
				document.AddMember("EndTime",rapidjson::Value(EndTime.data(),allocator),allocator);
			}

			document.Accept(writer);
			json_str = buffer.GetString();

			return json_str;
		}

		void WriteToJsonV3(rapidjson::Value &value,rapidjson::Document &document)
		{
			if (!document.IsObject() || !value.IsObject())
				return;

			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

			if (Id != -1)
			{
				document.AddMember("Id",rapidjson::Value(Id),allocator);
			}

			if (Type != -1)
			{
				document.AddMember("Type",rapidjson::Value(Type),allocator);
			}

			document.AddMember("Status",rapidjson::Value(Status),allocator);

			if (FunctionName != "")
			{
				document.AddMember("FunctionName",rapidjson::Value(FunctionName.data(),allocator),allocator);
			}

			if (StartTime != "")
			{
				document.AddMember("StartTime",rapidjson::Value(StartTime.data(),allocator),allocator);
			}

			if (EndTime != "")
			{
				document.AddMember("EndTime",rapidjson::Value(EndTime.data(),allocator),allocator);
			}
		}


		TaskTime_s(nlohmann::json json_str)
		{
			if (json_str.find("Status") != json_str.end())
			{
				Status = json_str.at("Status");
			}
			if (json_str.find("Type") != json_str.end())
			{
				Type = json_str.at("Type");
			}

			if (json_str.find("Id") != json_str.end())
			{
				Id = json_str.at("Id");
			}
			if (json_str.find("FunctionName") != json_str.end())
			{

				FunctionName = json_str.at("FunctionName").get<std::string>();;
			}
			if (json_str.find("StartTime") != json_str.end())
			{

				StartTime = json_str.at("StartTime").get<std::string>();
			}
			if (json_str.find("EndTime") != json_str.end())
			{

				EndTime = json_str.at("EndTime").get<std::string>();
			}
		} 


		static TaskTime_s CreateFromJsonV2(std::string json_str)
		{
			TaskTime_s taskTime;
			if (json_str.empty())
				return taskTime;

#if 0
			rapidjson::Document document;
			if (document.Parse(json_str.data()).HasParseError())
				return taskTime;

			taskTime = CreateFromJsonV3(document);
#else
			rapidjson::Document value;
			if (value.Parse(json_str.data()).HasParseError())
				return taskTime;

			if (value.HasMember("Id"))
			{
				taskTime.Id = value["Id"].GetInt();
			}

			if (value.HasMember("Type"))
			{
				taskTime.Type = value["Type"].GetInt();
			}

			if (value.HasMember("Status"))
			{
				taskTime.Status = value["Status"].GetInt();
			}

			if (value.HasMember("FunctionName"))
			{
				taskTime.FunctionName = value["FunctionName"].GetString();
				taskTime.FunctionName2 = taskTime.FunctionName;
			}

			if (value.HasMember("StartTime"))
			{
				taskTime.StartTime = value["StartTime"].GetString();
				taskTime.StartTime2 = taskTime.StartTime;
			}

			if (value.HasMember("EndTime"))
			{
				taskTime.EndTime = value["EndTime"].GetString();
				taskTime.EndTime2 = taskTime.EndTime;
			}
#endif

			return taskTime;
		}

		static TaskTime_s CreateFromJsonV3(rapidjson::Value& value)
		{
			TaskTime_s taskTime;
			if (!value.IsObject())
				return taskTime;

			if (value.HasMember("Id"))
			{
				taskTime.Id = value["Id"].GetInt();
			}

			if (value.HasMember("Type"))
			{
				taskTime.Type = value["Type"].GetInt();
			}

			if (value.HasMember("Status"))
			{
				taskTime.Status = value["Status"].GetInt();
			}

			if (value.HasMember("FunctionName"))
			{
				taskTime.FunctionName = value["FunctionName"].GetString();
			}

			if (value.HasMember("StartTime"))
			{
				taskTime.StartTime = value["StartTime"].GetString();
			}

			if (value.HasMember("EndTime"))
			{
				taskTime.EndTime = value["EndTime"].GetString();
			}

			return taskTime;
		}

		TaskTime_s(std::string json_str,int dummy)
		{
			TaskTime_s taskTime = CreateFromJsonV2(json_str);
			*this = taskTime;
		}

	};
	

	std::map<int, TaskTime_s> tasksmap;
	
	int GetFirstPendingTaskId()
	{
		
		int id = -1;

		for (auto& iter : tasksmap)
		{
			if (iter.second.Status == int(jobsta_e::STATUS_PENDDING))
			{
				

				id = iter.first;
				if (id >= 0)
				{
					return id;
				}

			}
		}
		return id;
	}

	int GetLastRunningTaskId()
	{
		int id = -1;

		for (auto& iter : tasksmap)
		{
			if (iter.second.Status == int(jobsta_e::STATUS_RUNNING))
			{
				id = iter.first;
				break;
			}
		}
		return id;
	}

	ATTimeSummary_s(const std::string& file)
	{

		if (JOB_FEEDBACK_USE_BIN) {
			loadBin(file);
		}
		else
		{
			load(file);
		}		
		
	}

	bool loadBin(const std::string& file)
	{
		std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::binary);
		
		if (!in.is_open())
			return false;

		TaskTimeFile taskTimeFile;
		taskTimeFile.Deserialize(in);

		runinfo.SubmitHostName = taskTimeFile.runInfoData.submitHostName;
		runinfo.SubmitUser = taskTimeFile.runInfoData.submitUser;
		runinfo.SubmitTime = taskTimeFile.runInfoData.submitTime;
		runinfo.EndTime = taskTimeFile.runInfoData.submitEndTime;
		runinfo.runninginfo.RunHostName = taskTimeFile.runInfoData.runData.runHostName;
		runinfo.runninginfo.RunUserName = taskTimeFile.runInfoData.runData.runUserName;
		runinfo.runninginfo.StartTime = taskTimeFile.runInfoData.runData.runStartTime;
		runinfo.runninginfo.EndTime = taskTimeFile.runInfoData.runData.runEndTime;
		int size = taskTimeFile.taskNum;
		for (int i = 0; i < size; i++) {
			TaskTime_s taskItem;
			TaskInfoData taskInfoItem = taskTimeFile.taskVec[i];
			taskItem.Id = taskInfoItem.id;
			taskItem.Type = taskInfoItem.type;
			taskItem.Status = taskInfoItem.status;
			taskItem.FunctionName = taskInfoItem.functionName;
			taskItem.StartTime = taskInfoItem.startTime;
			taskItem.EndTime = taskInfoItem.endTime;

			tasksmap[taskItem.Id] = taskItem;
		}
		in.close();

		return true;
	}

	int load(const std::string& file)
	{
		if (JOB_FEEDBACK_USE_BIN) {
			bool result = loadBin(file);
			if (!result) {
				LOGE("Save time bin failed!");
			}
		}
		else {
			if (CheckUsingNoChinesePathVersion())
			{
				try
				{
					std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
					if (ifs.fail())
						return false;
					std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
					if (str == "")
						return false;

					ATTimeSummary_s jobinfo(nlohmann::json::parse(str.begin(), str.end()));
					
					runinfo = jobinfo.runinfo;
					tasksmap = jobinfo.tasksmap;

					ifs.close();
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					return false;
				}
			}
			else
			{
				std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
				if (ifs.fail())
					return false;
				std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
				if (str == "")
					return false;

				ATTimeSummary_s jobinfo(str, 0);
				runinfo = jobinfo.runinfo;
				tasksmap = jobinfo.tasksmap;

				ifs.close();
			}
		}
		

		return true;
	}

	bool save(const std::string& file)
	{
		if (JOB_FEEDBACK_USE_BIN) {
			bool result = WriteToBin(file);
			if (!result) {
				LOGE("Save time bin failed!");
			}
		}
		else {
			if ( CheckUsingNoChinesePathVersion())
			{
				
				try
				{
					nlohmann::json outjson = WriteToJson();
					std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
					if (ofs.fail())
						return false;
					ofs << outjson.dump(4);
					ofs.close();
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					return false;
				}
				
			}
			else
			{
				std::string ext = AI3D::CORE::File::GetFileExtension(file);
				AI3D::CORE::String::StringToLower(&ext);

				if (JOB_FEEDBACK_USE_BIN) {
					WriteToBin(file);
				}
				else
				{
					std::string outjson_str = WriteToJsonV2();
					std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
					if (ofs.fail())
						return false;
					ofs << outjson_str;
					ofs.close();
				}

			}
		}
		

		return true;
	}

	bool WriteToBin(std::string file)
	{
		std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::binary);
		if (!out.is_open()) {
			LOGE("Save timefile bin failed!");
			return false;
		}
		TaskTimeFile taskTimeFile;

		taskTimeFile.runInfoData.submitHostName = runinfo.SubmitHostName;
		taskTimeFile.runInfoData.submitUser = runinfo.SubmitUser;
		taskTimeFile.runInfoData.submitTime = runinfo.SubmitTime;
		taskTimeFile.runInfoData.submitEndTime = runinfo.EndTime;
		taskTimeFile.runInfoData.runData.runHostName = runinfo.runninginfo.RunHostName;
		taskTimeFile.runInfoData.runData.runUserName = runinfo.runninginfo.RunUserName;
		taskTimeFile.runInfoData.runData.runStartTime = runinfo.runninginfo.StartTime;
		taskTimeFile.runInfoData.runData.runEndTime = runinfo.runninginfo.EndTime;
		int mapSize = tasksmap.size();
		taskTimeFile.taskNum = mapSize;
		taskTimeFile.taskVec.clear();
		for (auto iter : tasksmap) {
			TaskTime_s taskTime = iter.second;
			TaskInfoData taskInfoData;
			taskInfoData.id = taskTime.Id;
			taskInfoData.type = taskTime.Type;
			taskInfoData.status = taskTime.Status;
			taskInfoData.functionName = taskTime.FunctionName;
			taskInfoData.startTime = taskTime.StartTime;
			taskInfoData.endTime = taskTime.EndTime;
			taskTimeFile.taskVec.push_back(taskInfoData);
		}

		taskTimeFile.Serialize(out);

		out.close();

		return true;
	};


	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		
		if (runinfo.SubmitHostName != "")
			json_str["RunInfo"] = runinfo.WriteToJson();
		
		if (tasksmap.size() > 0)
		{
			nlohmann::json tasksjson;
			
			for (auto iter : tasksmap)
			{
				json_str["Tasks"].push_back(iter.second.WriteToJson());
			}

		}
		return json_str;
	}


	std::string WriteToJsonV2()
	{
		std::string json_str;

		rapidjson::Document document;
		document.SetObject();
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		if (runinfo.SubmitHostName != "")
		{
			
#if 0
			rapidjson::Value value(rapidjson::kObjectType);
			runinfo.WriteToJsonV3(value, document);
			document.AddMember("RunInfo", rapidjson::Value(value, allocator), allocator);
#else
			rapidjson::Value documentRunInfo(rapidjson::kObjectType);
			if (runinfo.SubmitHostName != "")
			{
				documentRunInfo.AddMember("SubmitHostName", rapidjson::Value(runinfo.SubmitHostName.c_str(), allocator), allocator);
			}

			if (runinfo.SubmitUser != "")
			{
				documentRunInfo.AddMember("SubmitUser", rapidjson::Value(runinfo.SubmitUser.c_str(), allocator), allocator);
			}

			if (runinfo.SubmitTime != "")
			{
				documentRunInfo.AddMember("SubmitTime", rapidjson::Value(runinfo.SubmitTime.c_str(), allocator), allocator);
			}

			if (runinfo.EndTime != "")
			{
				documentRunInfo.AddMember("EndTime", rapidjson::Value(runinfo.EndTime.c_str(), allocator), allocator);
			}

			if (runinfo.runninginfo.RunHostName != "")
			{
#if 0
				
				

				
				
				
#else
				rapidjson::Value documentRun(rapidjson::kObjectType);
				if (runinfo.runninginfo.RunHostName != "")
				{
					documentRun.AddMember("RunHostName", rapidjson::Value(runinfo.runninginfo.RunHostName.c_str(), allocator), allocator);
				}

				if (runinfo.runninginfo.RunUserName != "")
				{
					documentRun.AddMember("RunUserName", rapidjson::Value(runinfo.runninginfo.RunUserName.c_str(), allocator), allocator);
				}

				if (runinfo.runninginfo.StartTime != "")
				{
					documentRun.AddMember("StartTime", rapidjson::Value(runinfo.runninginfo.StartTime.c_str(), allocator), allocator);
				}

				if (runinfo.runninginfo.EndTime != "")
				{
					documentRun.AddMember("EndTime", rapidjson::Value(runinfo.runninginfo.EndTime.c_str(), allocator), allocator);
				}

				documentRunInfo.AddMember("Run", documentRun, allocator);
#endif
			}

			document.AddMember("RunInfo",documentRunInfo,allocator);
#endif
		}

		if (tasksmap.size() > 0)
		{
			rapidjson::Value valueTasksMap(rapidjson::kArrayType);



			for (auto iter : tasksmap)
			{
				
				
#if 0				 
				rapidjson::Value childValue(rapidjson::kObjectType);
				iter.second.WriteToJsonV3(childValue,document);
				valueTasksMap.PushBack(childValue,allocator);
#else
				TaskTime_s taskTime = iter.second;
				rapidjson::Value valueTaskTime(rapidjson::kObjectType);
				if (taskTime.Id != -1)
				{
					valueTaskTime.AddMember("Id", rapidjson::Value(taskTime.Id), allocator);
				}

				if (taskTime.Type != -1)
				{
					valueTaskTime.AddMember("Type", rapidjson::Value(taskTime.Type), allocator);
				}

				valueTaskTime.AddMember("Status", rapidjson::Value(taskTime.Status), allocator);

				if (taskTime.FunctionName != "")
				{
					valueTaskTime.AddMember("FunctionName", rapidjson::Value(taskTime.FunctionName.data(), allocator), allocator);
				}

				if (taskTime.StartTime != "")
				{
					valueTaskTime.AddMember("StartTime", rapidjson::Value(taskTime.StartTime.data(), allocator), allocator);
				}

				if (taskTime.EndTime != "")
				{
					valueTaskTime.AddMember("EndTime", rapidjson::Value(taskTime.EndTime.data(), allocator), allocator);
				}

				valueTasksMap.PushBack(valueTaskTime,allocator);
#endif
			}

			document.AddMember("Tasks",valueTasksMap,allocator);
		}

		document.Accept(writer);
		json_str = buffer.GetString();

		return json_str;
	}

	static ATTimeSummary_s CreateFromJsonV2(std::string json_str)
	{
		ATTimeSummary_s atTimeSummary;
		rapidjson::Document document;

		if (json_str.empty())
			return atTimeSummary;

		if (document.Parse(json_str.data()).HasParseError())
			return atTimeSummary;

#if 0
		atTimeSummary = CreateFromJsonV3(document);
#else
		if (document.HasMember("RunInfo"))
		{
			rapidjson::Value& documentRunInfo = document["RunInfo"];

			if (documentRunInfo.HasMember("SubmitHostName"))
			{
				atTimeSummary.runinfo.SubmitHostName = documentRunInfo["SubmitHostName"].GetString();
				atTimeSummary.runinfo.SubmitHostName2 = atTimeSummary.runinfo.SubmitHostName;
			}

			if (documentRunInfo.HasMember("SubmitUser"))
			{
				atTimeSummary.runinfo.SubmitUser = documentRunInfo["SubmitUser"].GetString();
				atTimeSummary.runinfo.SubmitUser2 = atTimeSummary.runinfo.SubmitUser;
			}

			if (documentRunInfo.HasMember("SubmitTime"))
			{
				atTimeSummary.runinfo.SubmitTime = documentRunInfo["SubmitTime"].GetString();
				atTimeSummary.runinfo.SubmitTime2 = atTimeSummary.runinfo.SubmitTime;
			}

			if (documentRunInfo.HasMember("EndTime"))
			{
				atTimeSummary.runinfo.EndTime = documentRunInfo["EndTime"].GetString();
				atTimeSummary.runinfo.EndTime2 = atTimeSummary.runinfo.EndTime;
			}

			if (documentRunInfo.HasMember("Run"))
			{
#if 0
				
				
				rapidjson::Value& documentRun = document["Run"];
				jobinfo.runninginfo = Run_s::CreateFromJsonV3(documentRun);
#else
				rapidjson::Value& documentRun = documentRunInfo["Run"];
				if (documentRun.HasMember("RunHostName"))
				{
					atTimeSummary.runinfo.runninginfo.RunHostName = documentRun["RunHostName"].GetString();
					atTimeSummary.runinfo.runninginfo.RunHostName2 = atTimeSummary.runinfo.runninginfo.RunHostName;
				}

				if (documentRun.HasMember("RunUserName"))
				{
					atTimeSummary.runinfo.runninginfo.RunUserName = documentRun["RunUserName"].GetString();
					atTimeSummary.runinfo.runninginfo.RunUserName2 = atTimeSummary.runinfo.runninginfo.RunUserName;
				}

				if (documentRun.HasMember("StartTime"))
				{
					atTimeSummary.runinfo.runninginfo.StartTime = documentRun["StartTime"].GetString();
					atTimeSummary.runinfo.runninginfo.StartTime2 = atTimeSummary.runinfo.runninginfo.StartTime;
				}

				if (documentRun.HasMember("EndTime"))
				{
					atTimeSummary.runinfo.runninginfo.EndTime = documentRun["EndTime"].GetString();
					atTimeSummary.runinfo.runninginfo.EndTime2 = atTimeSummary.runinfo.runninginfo.EndTime;
				}
#endif
			}
		}

		if (document.HasMember("Tasks"))
		{
			
			rapidjson::Value& valueTasks = document["Tasks"];
			if (valueTasks.IsArray())
			{
				for (unsigned idx = 0; idx < valueTasks.Size(); idx++)
				{
					rapidjson::Value& valueTask = valueTasks[idx];
					TaskTime_s taskTime;
#if 0
					taskTime = TaskTime_s::CreateFromJsonV3(valueTask);
#else
					if (valueTask.HasMember("Id"))
					{
						taskTime.Id = valueTask["Id"].GetInt();
					}

					if (valueTask.HasMember("Type"))
					{
						taskTime.Type = valueTask["Type"].GetInt();
					}

					if (valueTask.HasMember("Status"))
					{
						taskTime.Status = valueTask["Status"].GetInt();
					}

					if (valueTask.HasMember("FunctionName"))
					{
						taskTime.FunctionName = valueTask["FunctionName"].GetString();
						taskTime.FunctionName2 = taskTime.FunctionName;
					}

					if (valueTask.HasMember("StartTime"))
					{
						taskTime.StartTime = valueTask["StartTime"].GetString();
						taskTime.StartTime2 = taskTime.StartTime;
					}

					if (valueTask.HasMember("EndTime"))
					{
						taskTime.EndTime = valueTask["EndTime"].GetString();
						taskTime.EndTime2 = taskTime.EndTime;
					}
#endif
					atTimeSummary.tasksmap[taskTime.Id] = taskTime;
				}
			}
		}

#endif

		return atTimeSummary;
	}

	static ATTimeSummary_s CreateFromJsonV3(rapidjson::Value &value)
	{
		ATTimeSummary_s atTimeSummary;

		if (!value.IsObject())
			return atTimeSummary;

		rapidjson::Document document;

		if (value.HasMember("RunInfo"))
		{
			rapidjson::Value& valueRunInfo = value["RunInfo"];
			atTimeSummary.runinfo = RunInfo_s::CreateFromJsonV3(valueRunInfo);
		}

		if (value.HasMember("Tasks"))
		{
			
			rapidjson::Value& valueTasks = value["Tasks"];
			if (valueTasks.IsArray())
			{
				for (unsigned idx = 0; idx < valueTasks.Size(); idx++)
				{
					rapidjson::Value& valueTask = valueTasks[idx];
					TaskTime_s task = TaskTime_s::CreateFromJsonV3(valueTask);
					atTimeSummary.tasksmap[task.Id] = task;
				}
			}
		}

		return atTimeSummary;
	}


	ATTimeSummary_s(nlohmann::json json_str)
	{
		try
		{

			if (json_str.find("RunInfo") != json_str.end())
			{

				runinfo = RunInfo_s::CreateFromJson(json_str.at("RunInfo"));
			}

			if (json_str.find("Tasks") != json_str.end())
			{
				auto it_task = json_str.at("Tasks");


				for (int idx = 0; idx < it_task.size(); idx++)
				{

					TaskTime_s task(it_task[idx]);
					tasksmap[task.Id] = task;

				}
			}
		}
		catch (std::exception& ex)
		{
			std::ostringstream oss;
			oss << "exception:" << ex.what();
			LOGI(oss.str());
		}
	}

	
	ATTimeSummary_s(std::string json_str,int dummy)
	{
		ATTimeSummary_s atTimeSummary = CreateFromJsonV2(json_str);
		*this = atTimeSummary;
	}

	void GetStageStartAndEndTime(std::string functionname, QDateTime& dateEarly, QDateTime& dateLate)
	{
		QVector<QDateTime> stageStartDateTimeVector;
		QVector<QDateTime> stageEndDateTimeVector;

		for (auto iter : tasksmap)
		{
			if (iter.second.FunctionName == functionname)
			{
				std::string starttime = tasksmap.at(iter.first).StartTime;
				std::string endtime = tasksmap.at(iter.first).EndTime;
				if ((starttime != "") && (endtime != ""))
				{
					QDateTime QStartTime = QDateTime::fromString(QString::fromStdString(starttime), "yyyyMMddhhmmss");

					stageStartDateTimeVector.push_back(QStartTime);

					QDateTime QEndTime = QDateTime::fromString(QString::fromStdString(endtime), "yyyyMMddhhmmss");
					stageEndDateTimeVector.push_back(QEndTime);



				}
				else if ((starttime != "") && (endtime == ""))
				{
					

					if (runinfo.runninginfo.EndTime != "")
					{
						endtime = runinfo.runninginfo.EndTime;
					}
					QDateTime QStartTime = QDateTime::fromString(QString::fromStdString(starttime), "yyyyMMddhhmmss");

					stageStartDateTimeVector.push_back(QStartTime);

					QDateTime QEndTime = QDateTime::fromString(QString::fromStdString(endtime), "yyyyMMddhhmmss");
					stageEndDateTimeVector.push_back(QEndTime);


				}
			}
		}
		dateEarly = getEarlyDateTime(stageStartDateTimeVector);
		dateLate = getLateDateTime(stageEndDateTimeVector);

	}


	QString GetTimeSummaryBetweenStages(std::string functionname)
	{
		QDateTime dateEarly, dateLate;
		GetStageStartAndEndTime(functionname, dateEarly, dateLate);
		QString diff = getTotalTime(dateEarly, dateLate);
		return diff;
	}

	QString GetTimeSummaryBetweenStagesToCurrenttime(std::string functionname)
	{
		QDateTime dateEarly, dateLate;
		GetStageStartAndEndTime(functionname, dateEarly, dateLate);
		
		QString diff = getTotalTime(dateEarly, QDateTime::currentDateTime());
		return diff;
	}
	


	QString GetTimeSummaryToEnd(std::string functionname)
	{

		std::string jobstarttime = runinfo.runninginfo.StartTime;
		QDateTime dateEarly,  dateLate;
		 GetStageStartAndEndTime(functionname, dateEarly,  dateLate);
		dateEarly = QDateTime::fromString(QString::fromStdString(jobstarttime), "yyyyMMddhhmmss");
		QString diff = getTotalTime(dateEarly, dateLate);
		return diff;
	}
	QDateTime GetSubimitTime()
	{
		QDateTime time = QDateTime::fromString(QString::fromStdString(runinfo.SubmitTime), "yyyyMMddhhmmss");
		return time;
	}

	QDateTime GetLastTime()
	{
		QDateTime time = QDateTime::fromString(QString::fromStdString(runinfo.runninginfo.EndTime), "yyyyMMddhhmmss");
		return time;
		
	}

	QDateTime GetStartTime()
	{
		QDateTime time = QDateTime::fromString(QString::fromStdString(runinfo.runninginfo.StartTime), "yyyyMMddhhmmss");
		return time;
		
	}

	void GetTaskFinishedNum(std::string functionname,int& total,int& finished)
	{
		std::map<std::string, std::vector<int>> stagemap;
		for (auto iter : tasksmap)
		{
			stagemap[iter.second.FunctionName].push_back(iter.first);
		}
		total = stagemap.at(functionname).size(); 
		finished = 0;
		for (auto iter : stagemap.at(functionname))
		{
			
			std::string starttime = tasksmap.at(iter).StartTime;
			std::string endtime = tasksmap.at(iter).EndTime;
			if ((starttime != "") && (endtime != ""))
			{
				finished++;
			}
			
		}
	}

	int GetLastStage()
	{
		
		int beginid = -1,endid = -1;
		
		

		for (auto iter : tasksmap)
		{
			std::string starttime = tasksmap.at(iter.first).StartTime;
			std::string endtime = tasksmap.at(iter.first).EndTime;
			if(starttime!="")
			{
				beginid = iter.first;
			}
			if (endtime != "")
			{
				endid = iter.first;
			}
			
		}
		
		if(beginid>=0 && endid>=0)
		{
			if ((beginid >= endid))
			{
				return endid;
			}
		}
		
		return -1;
	}
};

struct TaskGraph_s
{
	JobInfo_s job;
	RunInfo_s runinfo;
	JobFeedBack_s feedback;
	
	std::map<int, Task_s> tasksmap;
	TaskGraph_s() {};
	TaskGraph_s(const std::string& file) {
		load(file);
	};

	bool HasTaskDef0()
	{
		if (tasksmap.count(0) && tasksmap.at(0).Type == ATSTARTTYPE)
			return true;
		return false;
	}
	
	

	

	bool IsTaskComplete(int id)
	{
		if (id == 0 && tasksmap.at(id).Status == jobsta_e::STATUS_COMPLETE)
		{
			return true;
		}

		auto task = tasksmap[id];
		auto depends = tasksmap[id].Depends;
		int cnout = 0;
		bool allcomple = false;

		

		if (!depends.empty())
		{
			for (auto idx : depends)
			{
				if (tasksmap.at(idx).Status == jobsta_e::STATUS_COMPLETE)
				{
					cnout++;
				}
			}
			
			if (cnout == depends.size() )
			{
				allcomple = true;	
				
			}
		}
		else 
		{
			allcomple = true;
		}
		

		
		if (allcomple)
		{
			if ((task.FatherId == -1) || (task.FatherId != -1 && tasksmap[task.FatherId].Status == jobsta_e::STATUS_COMPLETE))
			{
				return true;
			}
		}
		return false;
	}
	
	

	int GetFirstPendingTaskId()
	{

		int id = -1;
		
		for (auto& iter : tasksmap)
		{
		
			if (iter.second.Status == int(jobsta_e::STATUS_PENDDING))
			{
				 id = iter.first;
				 if (id == 0)
				 {
					 return 0;
				 }

				if (IsTaskComplete(id))
				{
					return id;
				}
				else
				{
					return -1;
				}		
			}
		}

		return id;
	}

	int GetLastRunningTaskId()
	{
		int id = -1;

		for (auto& iter : tasksmap)
		{
			if (iter.second.Status == int(jobsta_e::STATUS_RUNNING))
			{
				id = iter.first;
				break;
			}
		}
		return id;
	}

	
	
	void SetPendingInfo(int Type,JobInfo_s _job ,RunInfo_s _runinfo,std::string msg)
	{
		job = _job;
		runinfo = _runinfo;
		
		Task_s task;
		task.Id = 0;
		task.Type = Type;



		// task.Msg2 = GBK2UTF8(msg);
		task.Msg2 = msg;
		task.Msg = msg;


		task.ProjectPath = job.ProjectPath;
		task.ItemPath = job.ItemPath;

		task.ProjectPath2 = job.ProjectPath2;
		task.ItemPath2 = job.ItemPath2;

		std::vector<Task_s > tasks;
		tasks.push_back(task);
		for (auto taskiter : tasks)
		{
			tasksmap[taskiter.Id] = taskiter;
		}
	}
	

	float GetSubTasksPercent(int complete_task, float lastvalue,const std::set<int>& depends)
	{
		auto tasks = tasksmap;
		

		int taskid = complete_task;

		

		float progresss_step = 100.0 / depends.size();
		int success_task_num = 0;
		int last_success_id = *depends.cbegin();
		float finalprogress = 0.0;
		


		for (auto dependid : depends)
		{
			auto task = tasks.at(dependid);
			
			{
				if (fabs(task.Percent - COMPLETE_PROGRESS)<1e-6 && task.Status == int(job_status_e::STATUS_COMPLETE))
				{
					success_task_num++;
					if (task.Id > last_success_id)
						last_success_id = task.Id;
				}
			}
		}
		
		


		finalprogress = success_task_num * progresss_step + lastvalue * 100.0 / depends.size();


		return finalprogress;
	}

	float GetSubTasksPercent(int complete_task,float lastvalue)
	{
		auto tasks = tasksmap;
		
		
			int taskid = complete_task;

			auto depends = tasksmap.at(taskid).Depends;
			
			float progresss_step = 100.0 / depends.size();
			int success_task_num = 0;
			int last_success_id = *depends.cbegin();
			float finalprogress = 0.0;



			for (auto dependid : depends)
			{
				auto task = tasks.at(dependid);
				
				{
					
					if ( task.Status == int(job_status_e::STATUS_COMPLETE))
					{
						success_task_num++;
						if (task.Id > last_success_id)
							last_success_id = task.Id;
					}
				}
			}
			
			
			
			
			finalprogress = success_task_num * progresss_step+ lastvalue* 100.0/ depends.size();
			

			return finalprogress;
	}

	float GetProgress(float value, int current_taskid)
	{
		
		if (current_taskid == 0)
		{
			if (tasksmap.at(current_taskid).Type == ATSTARTTYPE)
			{
				return value;
			}

			if (tasksmap.at(current_taskid).Type == RECONSTRUCTIONSTARTTYPE)
			{
				return value;
			}
		}
		bool isATTask = (tasksmap.at(0).Type == ATSTARTTYPE);

		std::map <int, Task_s>::iterator ittask = tasksmap.end();
		--ittask;
		Task_s tasklast = ittask->second;
		float totalprogress = 0.0;

		if (isATTask)
		{

			if (1)
			{
				
				
				
				{
					for (auto taskiter : tasksmap)
					{
						auto tasktemp = taskiter.second;

						if (tasktemp.Type == ATCOMPLETETYPE || tasktemp.Type == ATLASTTASKTYPE)
						{
							if (StepAT_str.count(tasktemp.Msg))
							{
								auto stepat = StepAT_str.at(tasktemp.Msg);
								auto atprogress_range = GetATProgress(stepat);
								float range = atprogress_range.at(1) - atprogress_range.at(0);
								float taskprogress_temp = 0.0;
								if (current_taskid < tasktemp.Id && tasktemp.Depends.count(current_taskid))
								{
									taskprogress_temp = GetSubTasksPercent(tasktemp.Id, value);
									totalprogress = atprogress_range.at(0) + taskprogress_temp * range / 100.0;
									return	 totalprogress;
								}
							}
							else
							{

							}
						}
					}
				}
				
				
				
				

				
				
				
				
				
				
				
				

				
				
				
				

				

				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				

				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
			}
			else
			{
				for (auto taskiter : tasksmap)
				{
					auto tasktemp = taskiter.second;

					if (tasktemp.Type == 0)
					{
						if (StepAT_str.count(tasktemp.Msg))
						{
							auto stepat = StepAT_str.at(tasktemp.Msg);
							auto atprogress_range = GetATProgress(stepat);
							float range = atprogress_range.at(1) - atprogress_range.at(0);
							float taskprogress_temp = 0.0;
							if (current_taskid < tasktemp.Id && tasktemp.Depends.count(current_taskid))
							{
								taskprogress_temp = GetSubTasksPercent(tasktemp.Id, value);
								totalprogress = atprogress_range.at(0) + taskprogress_temp * range / 100.0;
								return	 totalprogress;
							}
						}
						else
						{

						}
					}
				}
			}
		}
		return 0.0;
	}
	
	
	
	bool load(const std::string& file)
	{
		std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
		if (ifs.fail())
			return false;
		std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		if (str == "")
			return false;
		if(CheckUsingNoChinesePathVersion())
		{
			try
			{
				TaskGraph_s taskgraph(nlohmann::json::parse(str.begin(), str.end()));
				
				
				job = taskgraph.job;
				runinfo = taskgraph.runinfo;
				feedback = taskgraph.feedback;

				tasksmap = taskgraph.tasksmap;
				ifs.close();
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return false;
			}
		}
		else
		{

			std::string jobname;
			TaskGraph_s taskgraph = TaskGraph_s::CreateFromJsonV4(str, jobname);

			job = taskgraph.job;
			runinfo = taskgraph.runinfo;
			feedback = taskgraph.feedback;
			tasksmap = taskgraph.tasksmap;

			ifs.close();
		}

		return true;
	}
	bool save(const std::string& file)
	{
		if( CheckUsingNoChinesePathVersion())
		{

			try
			{
				nlohmann::json outjson = WriteToJson();
				std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
				if (ofs.fail())
					return false;
				ofs << outjson.dump(4);
				ofs.close();
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return false;
			}

		}
		else
		{
			std::string outjson_str = WriteToJsonV2();
			std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
			if (ofs.fail())
				return false;
			ofs << outjson_str;
			ofs.close();
		}

		return true;
	}


	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		
		if (job.ProjectPath != "")
			json_str["JobInfo"] = job.WriteToJson();
		if (runinfo.SubmitHostName != "")
		json_str["RunInfo"] = runinfo.WriteToJson();
		json_str["JobFeedBack"] = feedback.WriteToJson();
		if (tasksmap.size() > 0)
		{
			nlohmann::json tasksjson;
			
			for(auto iter : tasksmap)
			{
				json_str["Tasks"].push_back(iter.second.WriteToJson());			
			}
			
		}
		return json_str;
	}


	std::string WriteToJsonV2()
	{
		std::string json_str;

		rapidjson::Document document;
		document.SetObject();

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	

		if (job.ProjectPath != "")
		{
			
			rapidjson::Value valJobInfo(rapidjson::kObjectType);
#if 0			
			WriteToJson3(valJobInfo, document);
#else
			if (job.ProjectPath != "")
				valJobInfo.AddMember("ProjectPath", rapidjson::Value(job.ProjectPath.c_str(), allocator), allocator);

			if (job.ItemPath != "")
				valJobInfo.AddMember("ItemPath", rapidjson::Value(job.ItemPath.c_str(), allocator), allocator);
#endif
			document.AddMember("JobInfo", valJobInfo, allocator);
		}

		if (runinfo.SubmitHostName != "")
		{
			
			rapidjson::Value valRunInfo(rapidjson::kObjectType);

			

			if (runinfo.SubmitHostName != "")
			{
				valRunInfo.AddMember("SubmitHostName", rapidjson::Value(runinfo.SubmitHostName.c_str(), allocator), allocator);
			}

			if (runinfo.SubmitUser != "")
			{
				valRunInfo.AddMember("SubmitUser", rapidjson::Value(runinfo.SubmitUser.c_str(), allocator), allocator);
			}

			if (runinfo.SubmitTime != "")
			{
				valRunInfo.AddMember("SubmitTime", rapidjson::Value(runinfo.SubmitTime.c_str(), allocator), allocator);
			}

			if (runinfo.EndTime != "")
			{
				valRunInfo.AddMember("EndTime", rapidjson::Value(runinfo.EndTime.c_str(), allocator), allocator);
			}

			if (runinfo.runninginfo.RunHostName != "")
			{
				rapidjson::Value documentRun(rapidjson::kObjectType);
				if (runinfo.runninginfo.RunHostName != "")
				{
					documentRun.AddMember("RunHostName", rapidjson::Value(runinfo.runninginfo.RunHostName.c_str(), allocator), allocator);
				}

				if (runinfo.runninginfo.RunUserName != "")
				{
					documentRun.AddMember("RunUserName", rapidjson::Value(runinfo.runninginfo.RunUserName.c_str(), allocator), allocator);
				}

				if (runinfo.runninginfo.StartTime != "")
				{
					documentRun.AddMember("StartTime", rapidjson::Value(runinfo.runninginfo.StartTime.c_str(), allocator), allocator);
				}

				if (runinfo.runninginfo.EndTime != "")
				{
					documentRun.AddMember("EndTime", rapidjson::Value(runinfo.runninginfo.EndTime.c_str(), allocator), allocator);
				}

				valRunInfo.AddMember("Run", documentRun, allocator);
			}

			
			document.AddMember("RunInfo", valRunInfo, allocator);
		}

		{
			
			rapidjson::Value valJobFeedback(rapidjson::kObjectType);
			

			valJobFeedback.AddMember("Status", rapidjson::Value((int)feedback.Status), allocator);
			valJobFeedback.AddMember("Percent", rapidjson::Value(feedback.Percent), allocator);
			valJobFeedback.AddMember("TaskRetVal", rapidjson::Value(feedback.TaskRetVal), allocator);
			if (feedback.Msg != "")
			{
				valJobFeedback.AddMember("Msg", rapidjson::Value(feedback.Msg.data(), allocator), allocator);
			}

			document.AddMember("JobFeedBack", valJobFeedback, allocator);
		}

		if (tasksmap.size() > 0)
		{
			
			rapidjson::Value valTasks(rapidjson::kArrayType);
			for (auto iter : tasksmap)
			{
				Task_s task = iter.second;
				rapidjson::Value valTask(rapidjson::kObjectType);
				
				

				valTask.AddMember("Status", rapidjson::Value(task.Status), allocator);
				valTask.AddMember("Percent", rapidjson::Value(task.Percent), allocator);
				valTask.AddMember("Msg", rapidjson::Value(task.Msg.data(), allocator), allocator);
				valTask.AddMember("Type", rapidjson::Value(task.Type), allocator);
				valTask.AddMember("ProjectPath", rapidjson::Value(task.ProjectPath.data(), allocator), allocator);
				valTask.AddMember("ItemPath", rapidjson::Value(task.ItemPath.data(), allocator), allocator);
				valTask.AddMember("Id", rapidjson::Value(task.Id), allocator);

				if (task.runinfo.StartTime != "")
				{
					
					

					
					
					
					

					
					
					
					rapidjson::Value documentRun(rapidjson::kObjectType);
					if (task.runinfo.RunHostName != "")
					{
						documentRun.AddMember("RunHostName", rapidjson::Value(task.runinfo.RunHostName.c_str(), allocator), allocator);
					}

					if (task.runinfo.RunUserName != "")
					{
						documentRun.AddMember("RunUserName", rapidjson::Value(task.runinfo.RunUserName.c_str(), allocator), allocator);
					}

					if (task.runinfo.StartTime != "")
					{
						documentRun.AddMember("StartTime", rapidjson::Value(task.runinfo.StartTime.c_str(), allocator), allocator);
					}

					if (task.runinfo.EndTime != "")
					{
						documentRun.AddMember("EndTime", rapidjson::Value(task.runinfo.EndTime.c_str(), allocator), allocator);
					}

					valTask.AddMember("Run", documentRun, allocator);
				}

				if (task.FatherId != -1)
				{
					valTask.AddMember("FatherId", rapidjson::Value(task.FatherId), allocator);
				}

				if (!task.Depends.empty())
				{
					
					
								
					rapidjson::Value valDepends(rapidjson::kArrayType);
					for (auto depId : task.Depends)
					{
						valDepends.PushBack(rapidjson::Value(depId), allocator);
					}
					valTask.AddMember("Depends", valDepends, allocator);
				}


				valTasks.PushBack(valTask, allocator);
			}

			document.AddMember("Tasks", valTasks, allocator);
		}

		document.Accept(writer);
		json_str = buffer.GetString();

		return json_str;
	}

	void WriteToJsonV3(rapidjson::Value &value,rapidjson::Document &document)
	{
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
		
		if (!value.IsObject())
			return;

		if (job.ProjectPath != "")
		{

			rapidjson::Value valJobInfo(rapidjson::kObjectType);
			job.WriteToJson3(valJobInfo, document);
			document.AddMember("JobInfo",valJobInfo,allocator);
		}

		if (runinfo.SubmitHostName != "")
		{

			rapidjson::Value valRunInfo(rapidjson::kObjectType);
			runinfo.WriteToJsonV3(valRunInfo,document);
			document.AddMember("RunInfo",valRunInfo,allocator);
		}

		{
			
			rapidjson::Value valJobFeedback(rapidjson::kObjectType);
			feedback.WriteToJsonV3(valJobFeedback,document);
			document.AddMember("JobFeedBack",valJobFeedback,allocator);
		}

		if (tasksmap.size() > 0)
		{
			
			rapidjson::Value valTasks(rapidjson::kArrayType);		
			for (auto iter : tasksmap)
			{
				Task_s task = iter.second;
				rapidjson::Value valTask(rapidjson::kObjectType);
				task.WriteToJson3(valTask,document);
				valTasks.PushBack(valTask, allocator);
			}

			document.AddMember("Tasks",valTasks,allocator);
		}

	}


	TaskGraph_s(nlohmann::json json_str)
	{

		if (json_str.find("JobInfo") != json_str.end())
		{
			job = JobInfo_s::CreateFromJson(json_str.at("JobInfo"));
		}
		if (json_str.find("RunInfo") != json_str.end())
		{		
			runinfo = RunInfo_s::CreateFromJson(json_str.at("RunInfo"));
		}
		if (json_str.find("JobFeedBack") != json_str.end())
		{
			feedback = JobFeedBack_s::CreateFromJson(json_str.at("JobFeedBack"));
		}
		if (json_str.find("Tasks") != json_str.end())
		{
			auto it_task = json_str.at("Tasks");
			
			for (int idx = 0; idx < it_task.size(); idx++)
			{
				Task_s task;
				task = Task_s::CreateFromJson(it_task[idx]);
				tasksmap[task.Id] = task;				
			}
			std::cout << "tasks size:" << it_task.size();
		}

	}

	TaskGraph_s(std::string json_str,int dummy)
	{
		TaskGraph_s taskGraph = CreateFromJsonV2(json_str);
		*this = taskGraph;
	}


	static TaskGraph_s CreateFromJson(nlohmann::json json_str)
	{
		TaskGraph_s _tg;
		if (json_str.find("JobInfo") != json_str.end())
		{
			
			_tg.job = JobInfo_s::CreateFromJson(json_str.at("JobInfo"));
		}
		if (json_str.find("RunInfo") != json_str.end())
		{
		
			_tg.runinfo = RunInfo_s::CreateFromJson(json_str.at("RunInfo"));
		}
		if (json_str.find("JobFeedBack") != json_str.end())
		{
			
			_tg.feedback = JobFeedBack_s::CreateFromJson(json_str.at("JobFeedBack"));
		}
		if (json_str.find("Tasks") != json_str.end())
		{
			auto it_task = json_str.at("Tasks");
		
			
			for (int idx = 0; idx < it_task.size(); idx++)
			{
				
				Task_s task;

				task = Task_s::CreateFromJson(it_task[idx]);
				_tg.tasksmap[task.Id] = task;
				
			}
			
		}
		return _tg;
	} 


	
	
	static TaskGraph_s CreateFromJsonV4(std::string str,std::string &jobName)
	{
		TaskGraph_s _tg;

		rapidjson::Document document;





		if (document.Parse(str.data()).HasParseError())
			return _tg;

		if (document.HasMember("JobName"))
		{
			jobName = document["JobName"].GetString();
		}

		if (document.HasMember("TaskGraph"))
		{
			rapidjson::Value& valueTaskGraph = document["TaskGraph"];

			if (valueTaskGraph.HasMember("JobFeedBack"))
			{
				rapidjson::Value& valueJobFeedBack = valueTaskGraph["JobFeedBack"];
				_tg.feedback = JobFeedBack_s::CreateFromJsonV3(valueJobFeedBack);
			}

			if (valueTaskGraph.HasMember("JobInfo"))
			{
				rapidjson::Value& valueJobInfo = valueTaskGraph["JobInfo"];
				_tg.job = JobInfo_s::CreateFromJsonV3(valueJobInfo);
			}

			if (valueTaskGraph.HasMember("RunInfo"))
			{
				rapidjson::Value& valueRunInfo = valueTaskGraph["RunInfo"];
				_tg.runinfo = RunInfo_s::CreateFromJsonV3(valueRunInfo);
			}

			if (valueTaskGraph.HasMember("Tasks"))
			{
				rapidjson::Value& valueTasks = valueTaskGraph["Tasks"];
				if (valueTasks.IsArray())
				{
					for (unsigned idx = 0; idx < valueTasks.Size(); idx++)
					{
						Task_s task;
						rapidjson::Value& valueTask = valueTasks[idx];
						
						task = Task_s::CreateFromJsonV3(valueTask);
						_tg.tasksmap[task.Id] = task;
					}
				}
			}
		}


		return _tg;
	}

	static TaskGraph_s CreateFromJsonV2(std::string json_str)
	{
		TaskGraph_s _tg;
		if (json_str.empty())
			return _tg;

		rapidjson::Document document;
		document.SetObject();
		rapidjson::StringBuffer  buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		if (document.Parse(json_str.data()).HasParseError())
			return _tg;




		
		if (document.HasMember("JobInfo"))
		{
			
			rapidjson::Value& valueJobInfo = document["JobInfo"];
			if (valueJobInfo.IsObject())
			{

				if (valueJobInfo.HasMember("ProjectPath"))
				{
					_tg.job.ProjectPath = valueJobInfo["ProjectPath"].GetString();
					// _tg.job.ProjectPath2 = UTF82GBK(_tg.job.ProjectPath);
					_tg.job.ProjectPath2 = _tg.job.ProjectPath;
				}

				if (valueJobInfo.HasMember("ItemPath"))
				{
					_tg.job.ItemPath = valueJobInfo["ItemPath"].GetString();
					// _tg.job.ItemPath2 = UTF82GBK(_tg.job.ItemPath);
					_tg.job.ItemPath2 = _tg.job.ItemPath;
				}
			}
		}

		if (document.HasMember("RunInfo"))
		{
			rapidjson::Value& valueRunInfo = document["RunInfo"];
			if (valueRunInfo.IsObject())
			{
				
	
				if (valueRunInfo.HasMember("SubmitHostName"))
				{
					_tg.runinfo.SubmitHostName = valueRunInfo["SubmitHostName"].GetString();
					_tg.runinfo.SubmitHostName2 = _tg.runinfo.SubmitHostName;
				}

				if (valueRunInfo.HasMember("SubmitUser"))
				{
					_tg.runinfo.SubmitUser = valueRunInfo["SubmitUser"].GetString();
					_tg.runinfo.SubmitUser2 = _tg.runinfo.SubmitUser;
				}

				if (valueRunInfo.HasMember("SubmitTime"))
				{
					_tg.runinfo.SubmitTime = valueRunInfo["SubmitTime"].GetString();
					_tg.runinfo.SubmitTime2 = _tg.runinfo.SubmitTime;
				}

				if (valueRunInfo.HasMember("EndTime"))
				{
					_tg.runinfo.EndTime = valueRunInfo["EndTime"].GetString();
					_tg.runinfo.EndTime2 = _tg.runinfo.EndTime;
				}

				if (valueRunInfo.HasMember("Run"))
				{
					
					rapidjson::Value& valueRun = valueRunInfo["Run"];
					

					if (valueRun.HasMember("RunHostName"))
					{
						_tg.runinfo.runninginfo.RunHostName = valueRun["RunHostName"].GetString();
						_tg.runinfo.runninginfo.RunHostName2 = _tg.runinfo.runninginfo.RunHostName;
					}

					if (valueRun.HasMember("RunUserName"))
					{
						_tg.runinfo.runninginfo.RunUserName = valueRun["RunUserName"].GetString();
						_tg.runinfo.runninginfo.RunUserName2 = _tg.runinfo.runninginfo.RunUserName;
					}

					if (valueRun.HasMember("StartTime"))
					{
						_tg.runinfo.runninginfo.StartTime = valueRun["StartTime"].GetString();
						_tg.runinfo.runninginfo.StartTime2 = _tg.runinfo.runninginfo.StartTime;
					}

					if (valueRun.HasMember("EndTime"))
					{
						_tg.runinfo.runninginfo.EndTime = valueRun["EndTime"].GetString();
						_tg.runinfo.runninginfo.EndTime2 = _tg.runinfo.runninginfo.EndTime;
					}
				}
			}
		}

		if (document.HasMember("JobFeedBack"))
		{
			
			rapidjson::Value& valueJobFeedback = document["JobFeedBack"];
			if (valueJobFeedback.IsObject())
			{

				if (valueJobFeedback.HasMember("Status"))
				{
					_tg.feedback.Status = (jobsta_e)valueJobFeedback["Status"].GetInt();
				}

				if (valueJobFeedback.HasMember("Percent"))
				{
					_tg.feedback.Percent = valueJobFeedback["Percent"].GetInt();
				}

				if (valueJobFeedback.HasMember("TaskRetVal"))
				{
					_tg.feedback.TaskRetVal = valueJobFeedback["TaskRetVal"].GetInt();
				}

				if (valueJobFeedback.HasMember("Msg"))
				{
					_tg.feedback.Msg = valueJobFeedback["Msg"].GetString();
					// _tg.feedback.Msg2 = UTF82GBK(_tg.feedback.Msg);
					_tg.feedback.Msg2 = _tg.feedback.Msg;
				}
			}
		}

		if (document.HasMember("Tasks"))
		{
			rapidjson::Value& valTasks = document["Tasks"];
			if (valTasks.IsArray())
			{
				for (unsigned idx = 0; idx < valTasks.Size(); idx++)
				{
					Task_s task;
					rapidjson::Value& valTask = valTasks[idx];

					
					
	
					if (valTask.HasMember("Status"))
					{
						task.Status = valTask["Status"].GetInt();
					}

					if (valTask.HasMember("Percent"))
					{
						task.Percent = valTask["Percent"].GetInt();
					}

					if (valTask.HasMember("Msg"))
					{
						task.Msg = valTask["Msg"].GetString();
						// task.Msg2 = UTF82GBK(task.Msg);
						task.Msg2 = task.Msg;
					}

					if (valTask.HasMember("ProjectPath"))
					{
						task.ProjectPath = valTask["ProjectPath"].GetString();
						// task.ProjectPath2 = UTF82GBK(task.ProjectPath);
						task.ProjectPath2 = task.ProjectPath;
					}

					if (valTask.HasMember("ItemPath"))
					{
						task.ItemPath = valTask["ItemPath"].GetString();
						// task.ItemPath2 = UTF82GBK(task.ItemPath);
						task.ItemPath2 = task.ItemPath;
					}

					if (valTask.HasMember("Id"))
					{
						task.Id = valTask["Id"].GetInt();
					}

					if (valTask.HasMember("Type"))
					{
						task.Type = valTask["Type"].GetInt();
					}

					
					if (valTask.HasMember("Run"))
					{
						
						
						
						
						

						const rapidjson::Value& documentRun = valTask["Run"];
						if (documentRun.HasMember("RunHostName"))
						{
							task.runinfo.RunHostName = documentRun["RunHostName"].GetString();
							task.runinfo.RunHostName2 = task.runinfo.RunHostName;
						}

						if (documentRun.HasMember("RunUserName"))
						{
							task.runinfo.RunUserName = documentRun["RunUserName"].GetString();
							task.runinfo.RunUserName2 = task.runinfo.RunUserName;
						}

						if (documentRun.HasMember("StartTime"))
						{
							task.runinfo.StartTime = documentRun["StartTime"].GetString();
							task.runinfo.StartTime2 = task.runinfo.StartTime;
						}

						if (documentRun.HasMember("EndTime"))
						{
							task.runinfo.EndTime = documentRun["EndTime"].GetString();
							task.runinfo.EndTime2 = task.runinfo.EndTime;
						}

					}

					if (valTask.HasMember("FatherId"))
					{
						task.FatherId = valTask["FatherId"].GetInt();
					}

					if (valTask.HasMember("Depends"))
					{
						const rapidjson::Value& dependsValue = valTask["Depends"];
						if (dependsValue.IsArray())
						{
							task.Depends.clear();
							for (unsigned i = 0; i < dependsValue.Size(); i++)
							{
								const rapidjson::Value& val = dependsValue[i];
								if (val.IsInt())
								{
									task.Depends.insert(val.GetInt());
								}
							}
						}
					}
					_tg.tasksmap[task.Id] = task;
				}
			}
		}


		return _tg;
	}

	static TaskGraph_s CreateFromJsonV3(rapidjson::Value &document)
	{
		TaskGraph_s _tg;
		if (!document.IsObject())
			return _tg;



		if (document.HasMember("JobInfo"))
		{
			
			rapidjson::Value& valueJobInfo = document["JobInfo"];
			if (valueJobInfo.IsObject())
			{
				_tg.job = JobInfo_s::CreateFromJsonV3(valueJobInfo);
			}
		}

		if (document.HasMember("RunInfo"))
		{
			rapidjson::Value& valueRunInfo = document["RunInfo"];
			if (valueRunInfo.IsObject())
			{
				
				_tg.runinfo = RunInfo_s::CreateFromJsonV3(valueRunInfo);
			}
		}

		if (document.HasMember("JobFeedBack"))
		{
			
			rapidjson::Value& valueJobFeedback = document["JobFeedBack"];
			if (valueJobFeedback.IsObject())
			{
				_tg.feedback = JobFeedBack_s::CreateFromJsonV3(valueJobFeedback);
			}
		}

		if (document.HasMember("Tasks"))
		{
			rapidjson::Value& valTasks = document["Tasks"];
			if (valTasks.IsArray())
			{
				for (unsigned idx = 0; idx < valTasks.Size(); idx++)
				{
					Task_s task;
					rapidjson::Value& valTask = valTasks[idx];
					
					task = Task_s::CreateFromJsonV3(valTask);
					_tg.tasksmap[task.Id] = task;
				}
			}
		}

		return _tg;
	}



};

struct JobFullInfo_s
{
	std::string JobName = "";
	std::string JobName2 = "";

	TaskGraph_s tg;

	JobFullInfo_s() {}
	JobFullInfo_s(const std::string& file) 
	{
		load(file);
	}

	
	 void SetPendingInfo(int Type,std::string jobname, std::string projectpath, std::string itempath, 
		 RunInfo_s runinfo)
	 {
		 
		 JobName = jobname;
		 JobName2 = jobname;
		 JobInfo_s _job(projectpath, itempath);
		
		 std::string msg = GetTaskStartingString(jobname);

		 tg.SetPendingInfo(Type,_job, runinfo,msg);		
	}

	bool load(const std::string& file)
	{
		if (JOB_INFO_USE_BIN) {
			bool result = LoadFromBin(file);
			if (!result) {
				LOGE("Load jobfile bin failed!");
				return false;
			}
		}
		else {
			std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
			if (ifs.fail())
				return false;
			std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			if (str == "")
				return false;

			if (CheckUsingNoChinesePathVersion())
			{
				try
				{
					
					JobFullInfo_s jobfullinfo(nlohmann::json::parse(str.begin(), str.end()));
					JobName = jobfullinfo.JobName;



					tg = jobfullinfo.tg;

					ifs.close();
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					return false;
				}
			}
			else
			{


				tg = TaskGraph_s::CreateFromJsonV4(str, JobName);

				ifs.close();
			}
		}
		return true;
	}

	bool save(const std::string& file)
	{
		if(0)
		{
			std::ostringstream oss;
			oss.clear();
			std::string msg = "";

			msg = "save job file:" + file;
			oss << __FUNCTION__ << " LINE " << __LINE__ << msg;
			LOGI(oss.str());
		}
		if (JOB_INFO_USE_BIN) {
			bool result = WriteToBin(file);
			if (!result) {
				LOGE("Save jobfile bin failed!");
				return false;
			}
		}
		else {

			if( CheckUsingNoChinesePathVersion())
			{

				try
				{
					nlohmann::json outjson = WriteToJson();
					std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
					if (ofs.fail())
						return false;
					ofs << outjson.dump(4);
					ofs.flush();
					ofs.close();
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					return false;
				}

			}
			else
			{

				std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
				if (ofs.fail())
					return false;

				std::string outjson_str = WriteToJsonV2();
				ofs << outjson_str;
				
				ofs.flush();
				ofs.close();
			}
			
		}

		return true;
	}

	bool WriteToBin(std::string file) {
		std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::binary);

		if (!out.is_open()) {
			LOGE("Save jobqueue bin failed!");
			return false;
		}
		JobListFile jobListFile;
		jobListFile.jobName = JobName;
		std::string projectPath = tg.job.ProjectPath;
		std::string itemPath = tg.job.ItemPath;
		std::string hostName = tg.runinfo.SubmitHostName;
		std::string submitUser = tg.runinfo.SubmitUser;
		std::string runHostName = tg.runinfo.runninginfo.RunHostName;
		std::string runUserName = tg.runinfo.runninginfo.RunUserName;
		
#ifdef WIN32
		// projectPath = GBK2UTF8(projectPath);
		// itemPath = GBK2UTF8(itemPath);
		// hostName = GBK2UTF8(hostName);
		// submitUser = GBK2UTF8(submitUser);
		// runHostName = GBK2UTF8(runHostName);
		// runUserName = GBK2UTF8(runUserName);
#endif 
		jobListFile.jobInfoData.projectPath = projectPath;
		jobListFile.jobInfoData.itemPath = itemPath;
		jobListFile.runInfoData.submitHostName = hostName;
		jobListFile.runInfoData.submitUser = submitUser;
		jobListFile.runInfoData.submitTime = tg.runinfo.SubmitTime;
		jobListFile.runInfoData.submitEndTime = tg.runinfo.EndTime;
		jobListFile.runInfoData.runData.runHostName = runHostName;
		jobListFile.runInfoData.runData.runUserName = runUserName;
		jobListFile.runInfoData.runData.runStartTime = tg.runinfo.runninginfo.StartTime;
		jobListFile.runInfoData.runData.runEndTime = tg.runinfo.runninginfo.EndTime;
		jobListFile.feedBackData.status = (int)tg.feedback.Status;
		jobListFile.feedBackData.percent = tg.feedback.Percent;
		jobListFile.feedBackData.taskRetVal = tg.feedback.TaskRetVal;
		std::string msg = tg.feedback.Msg;
#ifdef WIN32
		// msg = GBK2UTF8(msg);
#endif 
		jobListFile.feedBackData.msg = msg;
		int taskSize = tg.tasksmap.size();
		jobListFile.taskNum = taskSize;
		for (const auto& pair : tg.tasksmap) {
			Task_s tastItem = pair.second;
			TaskItemData taskItemData;
			taskItemData.msg = tastItem.Msg;
			taskItemData.percent = tastItem.Percent;
			taskItemData.status = tastItem.Status;
			taskItemData.type = tastItem.Type;
			std::string tmpProjectPath = tastItem.ProjectPath;
			std::string tmpItemPath = tastItem.ItemPath;
			std::string tmpRunHostName = tastItem.runinfo.RunHostName;
			std::string tmpRunUserName = tastItem.runinfo.RunUserName;
#ifdef WIN32
			// tmpProjectPath = GBK2UTF8(tmpProjectPath);
			// tmpItemPath = GBK2UTF8(tmpItemPath);
			// tmpRunHostName = GBK2UTF8(tmpRunHostName);
			// tmpRunUserName = GBK2UTF8(tmpRunUserName);
#endif 
			taskItemData.projectPath = tmpProjectPath;
			taskItemData.itemPath = tmpItemPath;
			taskItemData.id = tastItem.Id;
			taskItemData.fatherId = tastItem.FatherId;
			taskItemData.dependNum = tastItem.Depends.size();
			taskItemData.depends = tastItem.Depends;
			taskItemData.runData.runHostName = tmpRunHostName;
			taskItemData.runData.runUserName = tmpRunUserName;
			taskItemData.runData.runStartTime = tastItem.runinfo.StartTime;
			taskItemData.runData.runEndTime = tastItem.runinfo.EndTime;
			jobListFile.taskVec.push_back(taskItemData);
		}

		jobListFile.jobInfoData.freeze_no        = tg.job.point_info.freeze_no;
		jobListFile.jobInfoData.frozen_points    = tg.job.point_info.frozen_points;
		jobListFile.jobInfoData.consumed         = tg.job.point_info.consumed;
		jobListFile.jobInfoData.refunded         = tg.job.point_info.refunded;
		jobListFile.jobInfoData.total_balance    = tg.job.point_info.total_balance;
		jobListFile.jobInfoData.available_points = tg.job.point_info.available_points;
		jobListFile.jobInfoData.points_settled   = tg.job.point_info.points_settled;

		jobListFile.Serialize(out);

		out.close();
		return true;
	}

	bool LoadFromBin(std::string file) {
		std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::binary);

		if (!in.is_open())
			return false;

		JobListFile jobListFile;
		jobListFile.Deserialize(in);

		JobName = jobListFile.jobName;
		std::string projectPath = jobListFile.jobInfoData.projectPath;
		std::string itemPath = jobListFile.jobInfoData.itemPath;
		std::string hostName = jobListFile.runInfoData.submitHostName;
		std::string submitUser = jobListFile.runInfoData.submitUser;
		std::string runHostName = jobListFile.runInfoData.runData.runHostName;
		std::string runUserName = jobListFile.runInfoData.runData.runUserName;
#ifdef WIN32
		// projectPath = UTF82GBK(projectPath);
		// itemPath = UTF82GBK(itemPath);
		// hostName = UTF82GBK(hostName);
		// submitUser = UTF82GBK(submitUser);
		// runHostName = UTF82GBK(runHostName);
		// runUserName = UTF82GBK(runUserName);
#endif 
		tg.job.ProjectPath = projectPath;
		tg.job.ItemPath = itemPath;
		tg.runinfo.SubmitHostName = hostName;
		tg.runinfo.SubmitUser = submitUser;
		tg.runinfo.SubmitTime = jobListFile.runInfoData.submitTime;
		tg.runinfo.EndTime = jobListFile.runInfoData.submitEndTime;
		tg.runinfo.runninginfo.RunHostName = runHostName;
		tg.runinfo.runninginfo.RunUserName = runUserName;
		tg.runinfo.runninginfo.StartTime = jobListFile.runInfoData.runData.runStartTime;
		tg.runinfo.runninginfo.EndTime = jobListFile.runInfoData.runData.runEndTime;
		tg.feedback.Status = (jobsta_e)jobListFile.feedBackData.status;
		tg.feedback.Percent = jobListFile.feedBackData.percent;
		tg.feedback.TaskRetVal = jobListFile.feedBackData.taskRetVal;
		tg.feedback.Msg = jobListFile.feedBackData.msg;
		int taskSize = jobListFile.taskNum;
		for (int i = 0; i < taskSize; ++i) {
			Task_s tastItem;
			std::string msg = jobListFile.taskVec[i].msg;
#ifdef WIN32
			// msg = UTF82GBK(msg);
#endif 
			tastItem.Msg = msg;
			tastItem.Percent = jobListFile.taskVec[i].percent;
			tastItem.Status = jobListFile.taskVec[i].status;
			tastItem.Type = jobListFile.taskVec[i].type;
			std::string tmpProjectPath = jobListFile.taskVec[i].projectPath;
			std::string tmpItemPath = jobListFile.taskVec[i].itemPath;
			std::string tmpRunHostName = jobListFile.taskVec[i].runData.runHostName;
			std::string tmpRunUserName = jobListFile.taskVec[i].runData.runUserName;
#ifdef WIN32
			// tmpProjectPath = UTF82GBK(tmpProjectPath);
			// tmpItemPath = UTF82GBK(tmpItemPath);
			// tmpRunHostName = UTF82GBK(tmpRunHostName);
			// tmpRunUserName = UTF82GBK(tmpRunUserName);
#endif 
			tastItem.ProjectPath = tmpProjectPath;
			tastItem.ItemPath = tmpItemPath;
			tastItem.Id = jobListFile.taskVec[i].id;
			tastItem.FatherId = jobListFile.taskVec[i].fatherId;
			tastItem.Depends = jobListFile.taskVec[i].depends;
			tastItem.runinfo.RunHostName = tmpRunHostName;
			tastItem.runinfo.RunUserName = tmpRunUserName;
			tastItem.runinfo.StartTime = jobListFile.taskVec[i].runData.runStartTime;
			tastItem.runinfo.EndTime = jobListFile.taskVec[i].runData.runEndTime;

			tg.tasksmap[tastItem.Id] = tastItem;
		}
		tg.job.point_info.freeze_no        = jobListFile.jobInfoData.freeze_no;
		tg.job.point_info.frozen_points    = jobListFile.jobInfoData.frozen_points;
		tg.job.point_info.consumed         = jobListFile.jobInfoData.consumed;
		tg.job.point_info.refunded         = jobListFile.jobInfoData.refunded;
		tg.job.point_info.total_balance    = jobListFile.jobInfoData.total_balance;
		tg.job.point_info.available_points = jobListFile.jobInfoData.available_points;
		tg.job.point_info.points_settled   = jobListFile.jobInfoData.points_settled;

		in.close();
		return true;
	}


	nlohmann::json WriteToJson()
	{
		nlohmann::json json_str;
		json_str["JobName"] = JobName;
		json_str["TaskGraph"] = tg.WriteToJson();
		return json_str;
	}


	std::string WriteToJsonV2()
	{
		std::string json_str;

		rapidjson::Document document;
		document.SetObject();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		

		if (JobName != "")
		{
			document.AddMember("JobName", rapidjson::Value(JobName.data(), allocator), allocator);
		}


		rapidjson::Value valueTaskGraph(rapidjson::kObjectType);
		
		
		if (tg.job.ProjectPath != "")
		{
			
			rapidjson::Value valJobInfo(rapidjson::kObjectType);



			if (tg.job.ProjectPath != "")
				valJobInfo.AddMember("ProjectPath", rapidjson::Value(tg.job.ProjectPath.c_str(), allocator), allocator);

			if (tg.job.ItemPath != "")
				valJobInfo.AddMember("ItemPath", rapidjson::Value(tg.job.ItemPath.c_str(), allocator), allocator);

			valueTaskGraph.AddMember("JobInfo", valJobInfo, allocator);
		}

		if (tg.runinfo.SubmitHostName != "")
		{
			
			rapidjson::Value valRunInfo(rapidjson::kObjectType);

			

			if (tg.runinfo.SubmitHostName != "")
			{
				valRunInfo.AddMember("SubmitHostName", rapidjson::Value(tg.runinfo.SubmitHostName.c_str(), allocator), allocator);
			}

			if (tg.runinfo.SubmitUser != "")
			{
				valRunInfo.AddMember("SubmitUser", rapidjson::Value(tg.runinfo.SubmitUser.c_str(), allocator), allocator);
			}

			if (tg.runinfo.SubmitTime != "")
			{
				valRunInfo.AddMember("SubmitTime", rapidjson::Value(tg.runinfo.SubmitTime.c_str(), allocator), allocator);
			}

			if (tg.runinfo.EndTime != "")
			{
				valRunInfo.AddMember("EndTime", rapidjson::Value(tg.runinfo.EndTime.c_str(), allocator), allocator);
			}

			if (tg.runinfo.runninginfo.RunHostName != "")
			{
				rapidjson::Value documentRun(rapidjson::kObjectType);
				if (tg.runinfo.runninginfo.RunHostName != "")
				{
					documentRun.AddMember("RunHostName", rapidjson::Value(tg.runinfo.runninginfo.RunHostName.c_str(), allocator), allocator);
				}

				if (tg.runinfo.runninginfo.RunUserName != "")
				{
					documentRun.AddMember("RunUserName", rapidjson::Value(tg.runinfo.runninginfo.RunUserName.c_str(), allocator), allocator);
				}

				if (tg.runinfo.runninginfo.StartTime != "")
				{
					documentRun.AddMember("StartTime", rapidjson::Value(tg.runinfo.runninginfo.StartTime.c_str(), allocator), allocator);
				}

				if (tg.runinfo.runninginfo.EndTime != "")
				{
					documentRun.AddMember("EndTime", rapidjson::Value(tg.runinfo.runninginfo.EndTime.c_str(), allocator), allocator);
				}

				valRunInfo.AddMember("Run", documentRun, allocator);
			}

			
			valueTaskGraph.AddMember("RunInfo", valRunInfo, allocator);
		}

		{
			
			rapidjson::Value valJobFeedback(rapidjson::kObjectType);
			

			valJobFeedback.AddMember("Status", rapidjson::Value((int)tg.feedback.Status), allocator);
			valJobFeedback.AddMember("Percent", rapidjson::Value(tg.feedback.Percent), allocator);
			valJobFeedback.AddMember("TaskRetVal", rapidjson::Value(tg.feedback.TaskRetVal), allocator);
			if (tg.feedback.Msg != "")
			{
				valJobFeedback.AddMember("Msg", rapidjson::Value(tg.feedback.Msg.data(), allocator), allocator);
			}

			valueTaskGraph.AddMember("JobFeedBack", valJobFeedback, allocator);
		}

		if (tg.tasksmap.size() > 0)
		{
			
			rapidjson::Value valTasks(rapidjson::kArrayType);
			for (auto iter : tg.tasksmap)
			{
				Task_s task = iter.second;
				rapidjson::Value valTask(rapidjson::kObjectType);

				

				valTask.AddMember("Status", rapidjson::Value(task.Status), allocator);
				valTask.AddMember("Percent", rapidjson::Value(task.Percent), allocator);
				valTask.AddMember("Msg", rapidjson::Value(task.Msg.data(), allocator), allocator);
				valTask.AddMember("Type", rapidjson::Value(task.Type), allocator);
				valTask.AddMember("ProjectPath", rapidjson::Value(task.ProjectPath.data(), allocator), allocator);
				valTask.AddMember("ItemPath", rapidjson::Value(task.ItemPath.data(), allocator), allocator);
				valTask.AddMember("Id", rapidjson::Value(task.Id), allocator);

				if (task.runinfo.StartTime != "")
				{

					rapidjson::Value documentRun(rapidjson::kObjectType);
					if (task.runinfo.RunHostName != "")
					{
						documentRun.AddMember("RunHostName", rapidjson::Value(task.runinfo.RunHostName.c_str(), allocator), allocator);
					}

					if (task.runinfo.RunUserName != "")
					{
						documentRun.AddMember("RunUserName", rapidjson::Value(task.runinfo.RunUserName.c_str(), allocator), allocator);
					}

					if (task.runinfo.StartTime != "")
					{
						documentRun.AddMember("StartTime", rapidjson::Value(task.runinfo.StartTime.c_str(), allocator), allocator);
					}

					if (task.runinfo.EndTime != "")
					{
						documentRun.AddMember("EndTime", rapidjson::Value(task.runinfo.EndTime.c_str(), allocator), allocator);
					}

					valTask.AddMember("Run", documentRun, allocator);
				}

				if (task.FatherId != -1)
				{
					valTask.AddMember("FatherId", rapidjson::Value(task.FatherId), allocator);
				}

				if (!task.Depends.empty())
				{
					
					
								
					rapidjson::Value valDepends(rapidjson::kArrayType);
					for (auto depId : task.Depends)
					{
						valDepends.PushBack(rapidjson::Value(depId), allocator);
					}
					valTask.AddMember("Depends", valDepends, allocator);
				}


				valTasks.PushBack(valTask, allocator);
			}

			valueTaskGraph.AddMember("Tasks", valTasks, allocator);
		}
		
		document.AddMember("TaskGraph", valueTaskGraph, allocator);

		document.Accept(writer);
		json_str = buffer.GetString();

		

		return json_str;
	}

	void WriteToJsonV3(rapidjson::Value &document,rapidjson::Document &doc)
	{
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

		if (JobName != "")
		{
			document.AddMember("JobName", rapidjson::Value(JobName.data(), allocator), allocator);
		}

		rapidjson::Value valueTaskGraph(rapidjson::kObjectType);
		tg.WriteToJsonV3(valueTaskGraph, doc);
		document.AddMember("TaskGraph", valueTaskGraph, allocator);
	}

	static JobFullInfo_s CreateFromJsonV2(std::string _json_str)
	{
		JobFullInfo_s jobinfo;
		if (_json_str.empty())
			return jobinfo;

		
		std::string json_str = _json_str;

		rapidjson::Document document;

		if (document.Parse(json_str.data()).HasParseError())
			return jobinfo;

		if (document.HasMember("JobName"))
		{
			jobinfo.JobName = document["JobName"].GetString();
			// jobinfo.JobName2 = UTF82GBK(jobinfo.JobName);
			jobinfo.JobName2 = jobinfo.JobName;
		}

		if (document.HasMember("TaskGraph"))
		{

			rapidjson::Value& documentTaskGraph = document["TaskGraph"];
			if (documentTaskGraph.IsObject())
			{


				if (documentTaskGraph.HasMember("JobInfo"))
				{
					
					rapidjson::Value& valueJobInfo = documentTaskGraph["JobInfo"];
					if (valueJobInfo.IsObject())
					{
						
						if (valueJobInfo.HasMember("ProjectPath"))
						{
							jobinfo.tg.job.ProjectPath = valueJobInfo["ProjectPath"].GetString();
							// jobinfo.tg.job.ProjectPath2 = UTF82GBK(jobinfo.tg.job.ProjectPath);
							jobinfo.tg.job.ProjectPath2 = jobinfo.tg.job.ProjectPath;
						}

						if (valueJobInfo.HasMember("ItemPath"))
						{
							jobinfo.tg.job.ItemPath = valueJobInfo["ItemPath"].GetString();
							// jobinfo.tg.job.ItemPath2 = UTF82GBK(jobinfo.tg.job.ItemPath);
							jobinfo.tg.job.ItemPath2 = jobinfo.tg.job.ItemPath;
						}
					}
				}

				if (documentTaskGraph.HasMember("RunInfo"))
				{
					rapidjson::Value& valueRunInfo = documentTaskGraph["RunInfo"];
					if (valueRunInfo.IsObject())
					{
						
			
						if (valueRunInfo.HasMember("SubmitHostName"))
						{
							jobinfo.tg.runinfo.SubmitHostName = valueRunInfo["SubmitHostName"].GetString();
							// jobinfo.tg.runinfo.SubmitHostName2 = UTF82GBK(jobinfo.tg.runinfo.SubmitHostName);
							jobinfo.tg.runinfo.SubmitHostName2 = jobinfo.tg.runinfo.SubmitHostName;
						}

						if (valueRunInfo.HasMember("SubmitUser"))
						{
							jobinfo.tg.runinfo.SubmitUser = valueRunInfo["SubmitUser"].GetString();
							// jobinfo.tg.runinfo.SubmitUser2 = UTF82GBK(jobinfo.tg.runinfo.SubmitUser);
							jobinfo.tg.runinfo.SubmitUser2 = jobinfo.tg.runinfo.SubmitUser;
						}

						if (valueRunInfo.HasMember("SubmitTime"))
						{
							jobinfo.tg.runinfo.SubmitTime = valueRunInfo["SubmitTime"].GetString();
							// jobinfo.tg.runinfo.SubmitTime2 = UTF82GBK(jobinfo.tg.runinfo.SubmitTime);
							jobinfo.tg.runinfo.SubmitTime2 = jobinfo.tg.runinfo.SubmitTime;
						}

						if (valueRunInfo.HasMember("EndTime"))
						{
							jobinfo.tg.runinfo.EndTime = valueRunInfo["EndTime"].GetString();
							// jobinfo.tg.runinfo.EndTime2 = UTF82GBK(jobinfo.tg.runinfo.EndTime);
							jobinfo.tg.runinfo.EndTime2 = jobinfo.tg.runinfo.EndTime;
						}

						if (valueRunInfo.HasMember("Run"))
						{
							
							rapidjson::Value& valueRun = valueRunInfo["Run"];
							

							if (valueRun.HasMember("RunHostName"))
							{
								jobinfo.tg.runinfo.runninginfo.RunHostName = valueRun["RunHostName"].GetString();
								// jobinfo.tg.runinfo.runninginfo.RunHostName2 = UTF82GBK(jobinfo.tg.runinfo.runninginfo.RunHostName);
								jobinfo.tg.runinfo.runninginfo.RunHostName2 = jobinfo.tg.runinfo.runninginfo.RunHostName;
							}

							if (valueRun.HasMember("RunUserName"))
							{
								jobinfo.tg.runinfo.runninginfo.RunUserName = valueRun["RunUserName"].GetString();
								// jobinfo.tg.runinfo.runninginfo.RunUserName2 = UTF82GBK(jobinfo.tg.runinfo.runninginfo.RunUserName);
								jobinfo.tg.runinfo.runninginfo.RunUserName2 = jobinfo.tg.runinfo.runninginfo.RunUserName;
							}

							if (valueRun.HasMember("StartTime"))
							{
								jobinfo.tg.runinfo.runninginfo.StartTime = valueRun["StartTime"].GetString();
								// jobinfo.tg.runinfo.runninginfo.StartTime2 = UTF82GBK(jobinfo.tg.runinfo.runninginfo.StartTime);
								jobinfo.tg.runinfo.runninginfo.StartTime2 = jobinfo.tg.runinfo.runninginfo.StartTime;
							}

							if (valueRun.HasMember("EndTime"))
							{
								jobinfo.tg.runinfo.runninginfo.EndTime = valueRun["EndTime"].GetString();
								// jobinfo.tg.runinfo.runninginfo.EndTime2 = UTF82GBK(jobinfo.tg.runinfo.runninginfo.EndTime);
								jobinfo.tg.runinfo.runninginfo.EndTime2 = jobinfo.tg.runinfo.runninginfo.EndTime;
							}
						}
					}
				}

				if (documentTaskGraph.HasMember("JobFeedBack"))
				{
					
					rapidjson::Value& valueJobFeedback = documentTaskGraph["JobFeedBack"];
					if (valueJobFeedback.IsObject())
					{
						
						if (valueJobFeedback.HasMember("Status"))
						{
							jobinfo.tg.feedback.Status = (jobsta_e)valueJobFeedback["Status"].GetInt();
						}

						if (valueJobFeedback.HasMember("Percent"))
						{
							jobinfo.tg.feedback.Percent = valueJobFeedback["Percent"].GetInt();
						}

						if (valueJobFeedback.HasMember("TaskRetVal"))
						{
							jobinfo.tg.feedback.TaskRetVal = valueJobFeedback["TaskRetVal"].GetInt();
						}

						if (valueJobFeedback.HasMember("Msg"))
						{
							jobinfo.tg.feedback.Msg = valueJobFeedback["Msg"].GetString();
							// jobinfo.tg.feedback.Msg2 = UTF82GBK(jobinfo.tg.feedback.Msg);
							jobinfo.tg.feedback.Msg2 = jobinfo.tg.feedback.Msg;
						}
					}
				}

				if (documentTaskGraph.HasMember("Tasks"))
				{
					rapidjson::Value& valTasks = documentTaskGraph["Tasks"];
					if (valTasks.IsArray())
					{
						for (unsigned idx = 0; idx < valTasks.Size(); idx++)
						{
							Task_s task;
							rapidjson::Value& valTask = valTasks[idx];

							
							

							if (valTask.HasMember("Status"))
							{
								task.Status = valTask["Status"].GetInt();
							}

							if (valTask.HasMember("Percent"))
							{
								task.Percent = valTask["Percent"].GetInt();
							}

							if (valTask.HasMember("Msg"))
							{
								task.Msg = valTask["Msg"].GetString();
								// task.Msg2 = UTF82GBK(task.Msg);
								task.Msg2 = task.Msg;
							}

							if (valTask.HasMember("ProjectPath"))
							{
								task.ProjectPath = valTask["ProjectPath"].GetString();
								// task.ProjectPath2 = UTF82GBK(task.ProjectPath);
								task.ProjectPath2 = task.ProjectPath;
							}

							if (valTask.HasMember("ItemPath"))
							{
								task.ItemPath = valTask["ItemPath"].GetString();
								// task.ItemPath2 = UTF82GBK(task.ItemPath);
								task.ItemPath2 = task.ItemPath;
							}

							if (valTask.HasMember("Id"))
							{
								task.Id = valTask["Id"].GetInt();
							}

							if (valTask.HasMember("Type"))
							{
								task.Type = valTask["Type"].GetInt();
							}

							
							if (valTask.HasMember("Run"))
							{
								
								
								
								
								

								const rapidjson::Value& documentRun = valTask["Run"];
								if (documentRun.HasMember("RunHostName"))
								{
									task.runinfo.RunHostName = documentRun["RunHostName"].GetString();
									// task.runinfo.RunHostName2 = UTF82GBK(task.runinfo.RunHostName);
									task.runinfo.RunHostName2 = task.runinfo.RunHostName;
								}

								if (documentRun.HasMember("RunUserName"))
								{
									task.runinfo.RunUserName = documentRun["RunUserName"].GetString();
									// task.runinfo.RunUserName2 = UTF82GBK(task.runinfo.RunUserName);
									task.runinfo.RunUserName2 = task.runinfo.RunUserName;
								}

								if (documentRun.HasMember("StartTime"))
								{
									task.runinfo.StartTime = documentRun["StartTime"].GetString();
									// task.runinfo.StartTime2 = UTF82GBK(task.runinfo.StartTime);
									task.runinfo.StartTime2 = task.runinfo.StartTime;
								}

								if (documentRun.HasMember("EndTime"))
								{
									task.runinfo.EndTime = documentRun["EndTime"].GetString();
									// task.runinfo.EndTime2 = UTF82GBK(task.runinfo.EndTime);
									task.runinfo.EndTime2 = task.runinfo.EndTime;
								}

							}

							if (valTask.HasMember("FatherId"))
							{
								task.FatherId = valTask["FatherId"].GetInt();
							}

							if (valTask.HasMember("Depends"))
							{
								const rapidjson::Value& dependsValue = valTask["Depends"];
								if (dependsValue.IsArray())
								{
									task.Depends.clear();
									for (unsigned i = 0; i < dependsValue.Size(); i++)
									{
										const rapidjson::Value& val = dependsValue[i];
										if (val.IsInt())
										{
											task.Depends.insert(val.GetInt());
										}
									}
								}
							}
							jobinfo.tg.tasksmap[task.Id] = task;
						}
					}
				}


			}
		}

		return jobinfo;
	}


	JobFullInfo_s(nlohmann::json json_str)
	{
		JobName = json_str.at("JobName").get<std::string>();	
		tg = TaskGraph_s::CreateFromJson(json_str.at("TaskGraph"));
	}


	JobFullInfo_s(std::string json_str,int dummy)
	{
		JobFullInfo_s jobFullInfo = CreateFromJsonV2(json_str);
		*this = jobFullInfo;
	}

};


extern bool bNeedLoadingPrompt;


extern void OpenEngineNodeView();




struct infoforshow_s
{
	jobsta_e status;
	float progreesvalue;
	std::string progressstylestr;
	QString SubmitTime = "--/--";
	QString EndTime = "--/--";

	QString ATStagetext = "";
	std::string ATStagetextstylestr;
	QString ATStatustext = "";
	std::string ATStatustextstylestr;
	QString ATReporttext = "";
	std::string ATReporttextstylestr;
};


extern void doCancelJobs(std::vector<std::pair<std::string, std::string> > jobs_to_delete);

extern bool doCancelJob2(const std::string& tocancelPath, const std::string& jobName, int& errorCode);


struct jobexchangefile_s
{
	std::string timesumfile;
	std::string feadbackfile;
	jobsta_e status;
	std::string jobname;
	QVector<JobStage> vec_job;  
	
	infoforshow_s show;
};

extern int UpdateEngineStatus();
extern bool GetRealTimeInfo(jobexchangefile_s& jobinfo, bool* bGotNewJobInfo,bool* bGettingJobInfo);