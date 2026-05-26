#ifndef _TASK_DEFINE_
#define _TASK_DEFINE_
#include "Core/json.h"
#include <iostream>

#include <fstream>  
#include <streambuf>
#include <iomanip>
#include <sstream>

#include <chrono>
#include <string>
#include <memory>
#include <ctime>

#include <rapidjson\document.h>
#include <rapidjson\writer.h>
#include <rapidjson\stringbuffer.h>
#include "rapidjson\prettywriter.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "Core/Logging.h"
#include "Types.h"
#include "File.h"
#include "DataStruct.h"


#include <Windows.h>





#define NO_CHINESE_PATH_CONFIG2 "nochinesepath.json"



static std::string GetCurrentTimeStr() {
	using std::chrono::system_clock;

	system_clock::time_point tp = system_clock::now();
	time_t raw_time = system_clock::to_time_t(tp);
	struct tm* timeinfo = std::localtime(&raw_time);

	char buf[24] = { 0 };

	strftime(buf, 24, "%Y%m%d%H%M%S", timeinfo);

	return std::string(buf);
}

static bool CheckUsingNoChinesePathVersion()
{
	


#if 0
	if (!QDir(sOTASecondPath2).exists() || !QDir(sOTASecondBinPath2).exists())
	{
		if (bForceCheckLocal2)
		{
			if (QFileInfo::exists(NO_CHINESE_PATH_CONFIG2))
			{
				
				return true;
			}
		}

		return false;
	}

	if (!QFileInfo::exists(sRemoteNoChinesePathConfig2))
	{
		if (bForceCheckLocal2)
		{
			if (QFileInfo::exists(NO_CHINESE_PATH_CONFIG2))
			{
				
				return true;
			}
		}

		return false;
	}

	
#endif
	
	return false;
}

static bool CheckUsingNoChinesePathVersionOther()
{
	
	
	return true;
}


static bool Allow2LoadXLS()
{
	return false;
}

static void DumpStdString(std::string& str)
{
	std::cout << "dumpss:" << str << " size:" << str.size() << " len:" << str.length() <<  std::endl;
	if (str.size() > 0)
	{
		unsigned char* cdata = (unsigned char *)str.data();

		for (int i = 0; i < str.size(); i++)
		{

	
		}
	}
}

static std::string str2hex(unsigned char *data,int len)
{
	std::stringstream ss;

	ss << std::uppercase << std::hex << std::setfill('0');
	for (int i = 0; i < len; i++)
	{
		ss << std::setw(2) << static_cast<unsigned>(data[i]);
	}

	return ss.str();
}

static void DumpStdStr(std::string &name)
{
	std::cout << str2hex((unsigned char *)(name.c_str()),name.size()) << std::endl;
}


static char *GBKToUTF8Chars(char *inData,int inLen,int *outLen)
{
	if (!inData)
		return 0;

	if (outLen)
		*outLen = 0;

	WCHAR* wszSrcString;
	int n = MultiByteToWideChar(CP_ACP, 0, inData, -1, NULL, 0);
	wszSrcString = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, inData, -1, wszSrcString, n);

	n = WideCharToMultiByte(CP_UTF8, 0, wszSrcString, -1, NULL, 0, NULL, NULL);

	char* szDestString = new char[n];
	memset(szDestString,0x0,n);

	WideCharToMultiByte(CP_UTF8, 0, wszSrcString, -1, szDestString, n, NULL, NULL);

	delete[]wszSrcString;
	wszSrcString = NULL;

	if (outLen)
		*outLen = n;

	return szDestString;
}


static char *UTF8ToGBKChars(char *inData,int inLen,int *outLen)
{
	if (!inData)
		return 0;

	if (outLen)
		*outLen = 0;

	int len = MultiByteToWideChar(CP_UTF8, 0, inData, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, inData, -1, wstr, len);

	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* pszStr = new char[len + 1];
	memset(pszStr, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, pszStr, len, NULL, NULL);

	if (wstr) delete[] wstr;
	if (outLen)
		*outLen = len;

	return pszStr;
}

static std::string GBK2UTF8(const std::string& strGBK)
{
	WCHAR* wszSrcString;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	wszSrcString = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wszSrcString, n);
	n = WideCharToMultiByte(CP_UTF8, 0, wszSrcString, -1, NULL, 0, NULL, NULL);
	char* szDestString = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, wszSrcString, -1, szDestString, n, NULL, NULL);
	std::string strDestUTF8 = szDestString;
	delete[]wszSrcString;
	wszSrcString = NULL;
	delete[]szDestString;
	szDestString = NULL;
	return strDestUTF8;
}

static std::string UTF82GBK(const std::string& strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* pszStr = new char[len + 1];
	memset(pszStr, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, pszStr, len, NULL, NULL);
	if (wstr) delete[] wstr;
	std::string str = pszStr;
	delete[] pszStr;
	return str;
}



struct preparetaskinfo_s
{
	std::string ImagePath="";
	std::string Prefix="";
	std::string SRS= "EPSG:4326";
	std::string GcpPath = "";
	std::string PosfilePath = "";

	std::string ImagePath2 = "";
	std::string Prefix2 = "";
	std::string SRS2 = "EPSG:4326";
	std::string GcpPath2 = "";
	std::string PosfilePath2 = "";

	uint8_t NumLength = 4;
	int NumStart = 1;

	bool load(std::string file)
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

		if (doc.HasMember("ImagePath"))
		{
			ImagePath = doc["ImagePath"].GetString();
	// ImagePath2 = UTF82GBK(ImagePath);
	 ImagePath2 = ImagePath;
		}

		if (doc.HasMember("Prefix"))
		{
			Prefix = doc["Prefix"].GetString();
	// Prefix2 = UTF82GBK(Prefix);
	 Prefix2 = Prefix;
		}

		if (doc.HasMember("SRS"))
		{
			SRS = doc["SRS"].GetString();
	// SRS2 = UTF82GBK(SRS);
	 SRS2 = SRS;
		}

		if (doc.HasMember("PosfilePath"))
		{
			PosfilePath = doc["PosfilePath"].GetString();
	// PosfilePath2 = UTF82GBK(PosfilePath);
	 PosfilePath2 = PosfilePath;
		}

		if (doc.HasMember("GcpPath"))
		{
			GcpPath = doc["GcpPath"].GetString();
	// GcpPath2 = UTF82GBK(GcpPath);
	 GcpPath2 = GcpPath;
		}

		if (doc.HasMember("NumLength"))
		{
			NumLength = std::atoi(doc["NumLength"].GetString());
		}

		if (doc.HasMember("NumStart"))
		{
			NumStart = std::atoi(doc["NumStart"].GetString());
		}
		
		return true;
	}

	bool save(std::string file)
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		rapidjson::Document document;
		document.SetObject();
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		if (ImagePath != "")
		{
			document.AddMember("ImagePath", rapidjson::Value(ImagePath.c_str(), allocator), allocator);
		}
		if (Prefix != "")
		{
			document.AddMember("Prefix", rapidjson::Value(Prefix.c_str(), allocator), allocator);
		}

		if (SRS != "")
		{
			document.AddMember("SRS", rapidjson::Value(SRS.c_str(), allocator), allocator);
		}
		if (PosfilePath != "")
		{
			document.AddMember("PosfilePath", rapidjson::Value(PosfilePath.c_str(), allocator), allocator);
		}
		if (GcpPath != "")
		{
			document.AddMember("GcpPath", rapidjson::Value(GcpPath.c_str(), allocator), allocator);
		}
		if (NumStart >=0)
		{
			document.AddMember("NumStart", rapidjson::Value(std::to_string(NumStart).c_str(), allocator), allocator);
		}
		if (NumLength >= 0)
		{
			document.AddMember("NumLength", rapidjson::Value(std::to_string(NumLength).c_str(), allocator), allocator);
		}
		
		document.Accept(writer);
		std::ofstream fileout = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
		if (!fileout.good())
			return false;

		fileout << buffer.GetString();
		fileout.close();
		return true;
	}
};


struct TaskDescriptor
{
	int id_;
	
	std::string msg_;
	std::string name_;
	std::string msg2_;
	std::string name2_;

	float begin_ = -1.f;
	float end_ = -1.f;
	int fatherId_ = -1;
	int match_id_ = -1;
	int type_ ;
	std::vector<int> imgIds_;
	std::vector<int> depends_;

	int sfmId_ = -1;	
	std::vector<int> matchIds_;
	int match_task_num_ = 0;
	int match_maximage_num_ = -1;
	int key_maximage_num_ = -1;
	int sfm_task_num_ = -1;

	std::string fun_name_;
	std::string fun_name2_;

	int sfmmem_ = -1;
	TaskDescriptor() {};

	








	static TaskDescriptor CreateFromJson(nlohmann::json json_str)
	{
		TaskDescriptor task;
		task.id_ = json_str.at("id");
		task.msg_ = json_str.at("msg").get<std::string>();;
		task.name_ = json_str.at("name").get<std::string>();
		task.type_ = json_str.at("type");
		auto _it_begin = json_str.find("begin");
		auto _it_end = json_str.find("end");
		if (_it_begin != json_str.end() && _it_end != json_str.end())
		{
			task.begin_ = json_str.at("begin");
			task.end_ = json_str.at("end");
		}
		
		if (json_str.find("sfmmem") != json_str.end())
		{
			task.sfmmem_ = json_str.at("sfmmem");
		}
		auto _it = json_str.find("fatherId");
		if (_it != json_str.end())
		{
			task.fatherId_ = json_str.at("fatherId");
		}
		_it = json_str.find("depends");
		if (_it != json_str.end())
		{
			nlohmann::json dependstr = json_str.at("depends");

			for (auto depend = 0; depend < dependstr.size();
				depend++)
			{
				task.depends_.push_back(dependstr[depend]);

			}
		}

		_it = json_str.find("imgIds");
		if (_it != json_str.end())
		{
			nlohmann::json imgIDsstr = json_str.at("imgIds");

			for (auto idx = 0; idx < imgIDsstr.size();
				idx++)
			{
				task.imgIds_.push_back(imgIDsstr[idx]);

			}
		}

		if (json_str.find("sfmId") != json_str.end())
		{
			task.sfmId_ = json_str.at("sfmId");
		}

		if (json_str.find("keyMaxImgNum") != json_str.end())
		{
			task.key_maximage_num_ = json_str.at("keyMaxImgNum");

		}
		

		if (json_str.find("matchMaxImgNum") != json_str.end())
		{
			task.match_maximage_num_ = json_str.at("matchMaxImgNum");

		}

		
		if (json_str.find("matchTaskNum") != json_str.end())
		{
			task.match_task_num_ = json_str.at("matchTaskNum");

		}


		if (json_str.find("matchIds") != json_str.end())
		{
			nlohmann::json imgIDsstr = json_str.at("matchIds");

			for (auto idx = 0; idx < imgIDsstr.size();
				idx++)
			{
				task.matchIds_.push_back(imgIDsstr[idx]);

			}
		}
		if (json_str.find("sfm_task_num_") != json_str.end())
		{
			task.sfmId_ = json_str.at("sfm_task_num_");
		}

		auto _it_fun = json_str.find("function");
		if (_it_fun != json_str.end())
		{
			task.fun_name_ = json_str.at("function").get<std::string>(); ;
		}
		return task;
	};



	static TaskDescriptor CreateFromJsonV2(std::string json_str)
	{
		TaskDescriptor task;

		if (json_str.empty())
			return task;

#if 0
		rapidjson::Document document;
		document.SetObject();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		if (document.Parse(json_str.data()).HasParseError())
			return  task;

		task = CreateFromJsonV3(document);
#else
		rapidjson::Document value;
		value.SetObject();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		rapidjson::Document::AllocatorType& allocator = value.GetAllocator();

		if (value.Parse(json_str.data()).HasParseError())
			return  task;

		if (value.HasMember("id"))
		{
			task.id_ = value["id"].GetInt();
		}

		if (value.HasMember("msg"))
		{
			task.msg_ = value["msg"].GetString();
			// task.msg2_ = UTF82GBK(task.msg_);
			task.msg2_ = task.msg_;
		}

		if (value.HasMember("name"))
		{
			task.name_ = value["name"].GetString();
			// task.name2_ = UTF82GBK(task.name_);
			task.name2_ = task.name_;
		}

		if (value.HasMember("type"))
		{
			task.type_ = value["type"].GetInt();
		}

		if (value.HasMember("begin"))
		{
			task.begin_ = value["begin"].GetFloat();
		}

		if (value.HasMember("end"))
		{
			task.end_ = value["end"].GetFloat();
		}

		if (value.HasMember("sfmmem"))
		{
			task.sfmmem_ = value["sfmmem"].GetFloat();
		}

		if (value.HasMember("fatherId"))
		{
			task.fatherId_ = value["fatherId"].GetInt();
		}

		if (value.HasMember("depends"))
		{
			rapidjson::Value& valDepends = value["depends"];
			if (valDepends.IsArray() && valDepends.Size() > 0)
			{
				for (unsigned idx = 0; idx < valDepends.Size(); idx++)
				{
					rapidjson::Value& valDepend = valDepends[idx];
					task.depends_.push_back(valDepend.GetInt());
				}
			}
		}

		if (value.HasMember("imgIds"))
		{
			rapidjson::Value& valImgIds = value["imgIds"];
			if (valImgIds.IsArray() && valImgIds.Size() > 0)
			{
				for (unsigned idx = 0; idx < valImgIds.Size(); idx++)
				{
					rapidjson::Value& valImgId = valImgIds[idx];
					task.imgIds_.push_back(valImgId.GetInt());
				}
			}
		}

		if (value.HasMember("function"))
		{
			task.fun_name_ = value["function"].GetString();
			// task.fun_name2_ = UTF82GBK(task.fun_name_);
			task.fun_name2_ = task.fun_name_;
		}


		if (value.HasMember("sfmId"))
		{
			task.sfmId_ = value["sfmId"].GetInt();
		}
		

		if (value.HasMember("keyMaxImgNum"))
		{
			task.key_maximage_num_ = value["keyMaxImgNum"].GetInt();

		}
		

		if (value.HasMember("matchMaxImgNum"))
		{
			task.match_maximage_num_ = value["matchMaxImgNum"].GetInt();

		}

		
		if (value.HasMember("matchTaskNum"))
		{
			task.match_task_num_ = value["matchTaskNum"].GetInt();

		}
		if (value.HasMember("sfmNum"))
		{
			task.sfm_task_num_ = value["sfmNum"].GetInt();
		}

		if (value.HasMember("matchIds"))
		{
			rapidjson::Value&  matchIDsstr = value["matchIds"];

			for (auto& id : matchIDsstr.GetArray())
			{
				task.matchIds_.push_back(id.GetInt());

			}
		}
#endif

		return task;
	};

	static TaskDescriptor CreateFromJsonV3(rapidjson::Value &value)
	{
		TaskDescriptor task;

		if (!value.IsObject())
			return task;

		if (value.HasMember("id"))
		{
			task.id_ = value["id"].GetInt();
		}

		if (value.HasMember("msg"))
		{
			task.msg_ = value["msg"].GetString();
		}

		if (value.HasMember("name"))
		{
			task.name_ = value["name"].GetString();
		}

		if (value.HasMember("type"))
		{
			task.type_ = value["type"].GetInt();
		}

		if (value.HasMember("begin"))
		{
			task.begin_ = value["begin"].GetFloat();
		}

		if (value.HasMember("end"))
		{
			task.end_ = value["end"].GetFloat();
		}

		if (value.HasMember("sfmmem"))
		{
			task.sfmmem_ = value["sfmmem"].GetFloat();
		}

		if (value.HasMember("fatherId"))
		{
			task.fatherId_ = value["fatherId"].GetInt();
		}

		if (value.HasMember("depends"))
		{
			rapidjson::Value& valDepends = value["depends"];
			if (valDepends.IsArray() && valDepends.Size() > 0)
			{
				for (unsigned idx = 0; idx < valDepends.Size(); idx++)
				{
					rapidjson::Value& valDepend = valDepends[idx];
					task.depends_.push_back(valDepend.GetInt());
				}
			}
		}

		if (value.HasMember("imgIds"))
		{
			rapidjson::Value& valImgIds = value["imgIds"];
			if (valImgIds.IsArray() && valImgIds.Size() > 0)
			{
				for (unsigned idx = 0; idx < valImgIds.Size(); idx++)
				{
					rapidjson::Value& valImgId = valImgIds[idx];
					task.imgIds_.push_back(valImgId.GetInt());
				}
			}
		}

		if (value.HasMember("function"))
		{
			task.fun_name_ = value["function"].GetString();
		}

		if (value.HasMember("sfmId"))
		{
			task.sfmId_ = value["sfmId"].GetInt();
		}
		

		if (value.HasMember("keyMaxImgNum"))
		{
			task.key_maximage_num_ = value["keyMaxImgNum"].GetInt();

		}
		

		if (value.HasMember("matchMaxImgNum"))
		{
			task.match_maximage_num_ = value["matchMaxImgNum"].GetInt();

		}

		
		if (value.HasMember("matchTaskNum"))
		{
			task.match_task_num_ = value["matchTaskNum"].GetInt();

		}
		if (value.HasMember("sfmNum"))
		{
			task.sfm_task_num_ = value["sfmNum"].GetInt();
		}

		if (value.HasMember("matchIds"))
		{
			rapidjson::Value&  matchIDsstr = value["matchIds"];

			for (auto& id : matchIDsstr.GetArray())
			{
				task.matchIds_.push_back(id.GetInt());

			}
		}
		
		return task;
	}


	nlohmann::json WriteToJson()
	{
		nlohmann::json task_json;
		task_json["id"] = id_;
		task_json["msg"] = msg_;
		task_json["name"] = name_;
		if (begin_ > -1.f && end_ > -1.f)
		{
			task_json["begin"] = begin_;
			task_json["end"] = end_;
		}

		task_json["type"] = type_;
		if (fatherId_ != -1)
			task_json["fatherId"] = fatherId_;
		if (sfmmem_ >0)
			task_json["sfmmem"] = sfmmem_;
		if (!depends_.empty())
			task_json["depends"] = depends_;
		if (!imgIds_.empty())
			task_json["imgIds"] = imgIds_;
		if (!fun_name_.empty())
		{
			task_json["function"] = fun_name_;
		}
		return task_json;
	};



	std::string WriteToJsonV2()
	{
		std::string json_str;
#if 0
		rapidjson::Document document;
		document.SetObject();

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		WriteToJsonV3(document, document);

		document.Accept(writer);
		json_str = buffer.GetString();
#else
		rapidjson::Document val;
		val.SetObject();

		rapidjson::Document::AllocatorType& allocator = val.GetAllocator();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		val.AddMember("id", rapidjson::Value(id_), allocator);
		val.AddMember("msg", rapidjson::Value(msg_.data(), allocator), allocator);
		val.AddMember("name", rapidjson::Value(name_.data(), allocator), allocator);
		val.AddMember("type", rapidjson::Value(type_), allocator);
		val.AddMember("begin", rapidjson::Value(begin_), allocator);
		val.AddMember("end", rapidjson::Value(end_), allocator);
		val.AddMember("sfmmem", rapidjson::Value(sfmmem_), allocator);
		val.AddMember("fatherId", rapidjson::Value(fatherId_), allocator);

		if (imgIds_.size() > 0)
		{
			rapidjson::Value valImgIds(rapidjson::kArrayType);
			for (auto imgId_ : imgIds_)
			{
				valImgIds.PushBack(rapidjson::Value(imgId_), allocator);
			}

			val.AddMember("imgIds", rapidjson::Value(valImgIds, allocator), allocator);
		}

		if (depends_.size() > 0)
		{
			rapidjson::Value valDepends(rapidjson::kArrayType);
			for (auto depend_ : depends_)
			{
				valDepends.PushBack(rapidjson::Value(depend_), allocator);
			}
			val.AddMember("depends", rapidjson::Value(valDepends, allocator), allocator);
		}


		if (sfm_task_num_ >= 0)
		{
			val.AddMember("sfmNum", rapidjson::Value(sfm_task_num_), allocator);

		}
		if (match_task_num_ > 0)
		{
			val.AddMember("matchTaskNum", rapidjson::Value(match_task_num_), allocator);

		}
		if (!matchIds_.empty())
		{
			rapidjson::Value value(rapidjson::kArrayType);
			for (int i = 0; i < matchIds_.size(); i++)
			{

				value.PushBack(matchIds_[i], allocator);
			}
			val.AddMember("matchIds", (value), allocator);
		}
		if (match_maximage_num_ > 0)
		{
			val.AddMember("matchMaxImgNum", rapidjson::Value(match_maximage_num_), allocator);

		}
		if (key_maximage_num_ > 0)
		{
			val.AddMember("keyMaxImgNum", rapidjson::Value(key_maximage_num_), allocator);

		}
		{
			val.AddMember("sfmId", rapidjson::Value(sfmId_), allocator);

		}

		val.Accept(writer);
		json_str = buffer.GetString();
#endif

		return json_str;
	};

	void WriteToJsonV3(rapidjson::Value &val,rapidjson::Document &doc)
	{
		if (!val.IsObject() || !doc.IsObject())
			return;

		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

		val.AddMember("id", rapidjson::Value(id_), allocator);
		val.AddMember("msg", rapidjson::Value(msg_.data(),allocator), allocator);
		val.AddMember("name", rapidjson::Value(name_.data(), allocator), allocator);
		val.AddMember("type", rapidjson::Value(type_), allocator);
		val.AddMember("begin", rapidjson::Value(begin_), allocator);
		val.AddMember("end", rapidjson::Value(end_), allocator);
		val.AddMember("sfmmem", rapidjson::Value(sfmmem_), allocator);
		val.AddMember("fatherId", rapidjson::Value(fatherId_), allocator);

		if (imgIds_.size() > 0)
		{
			rapidjson::Value valImgIds(rapidjson::kArrayType);
			for (auto imgId_ : imgIds_)
			{
				valImgIds.PushBack(rapidjson::Value(imgId_),allocator);
			}

			val.AddMember("imgIds", rapidjson::Value(valImgIds, allocator), allocator);
		}


		if (depends_.size() > 0)
		{
			rapidjson::Value valDepends(rapidjson::kArrayType);
			for (auto depend_ : depends_)
			{
				valDepends.PushBack(rapidjson::Value(depend_),allocator);
			}
			val.AddMember("depends",rapidjson::Value(valDepends,allocator),allocator);
		}
		if (sfm_task_num_ >= 0)
		{
			val.AddMember("sfmNum", rapidjson::Value(sfm_task_num_), allocator);

		}
		if (match_task_num_ > 0)
		{
			val.AddMember("matchTaskNum", rapidjson::Value(match_task_num_), allocator);

		}
		if (!matchIds_.empty())
		{
			rapidjson::Value value(rapidjson::kArrayType);
			for (int i = 0; i < matchIds_.size(); i++)
			{

				value.PushBack(matchIds_[i], allocator);
			}
			val.AddMember("matchIds", (value), allocator);
		}
		if (match_maximage_num_ > 0)
		{
			val.AddMember("matchMaxImgNum", rapidjson::Value(match_maximage_num_), allocator);

		}
		if (key_maximage_num_ > 0)
		{
			val.AddMember("keyMaxImgNum", rapidjson::Value(key_maximage_num_), allocator);

		}
		{
			val.AddMember("sfmId", rapidjson::Value(sfmId_), allocator);

		}

		val.AddMember("function", rapidjson::Value(fun_name_.data(),allocator), allocator);
	}
};

struct ATTaskInfo
{
	std::string job_;
	std::string blockItem_;
	std::string projectFile_;
	std::string ATJson_;
	std::string GCPJson_;

	std::string job2_;
	std::string blockItem2_;
	std::string projectFile2_;
	std::string ATJson2_;
	std::string GCPJson2_;

	TaskDescriptor task_;

	ATTaskInfo() {};

	bool load(const std::string& file) {
		if (TASK_USE_BIN) {
			return LoadBin(file);
		}
		else {
			return LoadJson(file);
		}
	}

	bool LoadJson(const std::string& file)
	{
		
		

		
		
		
		
		
		
		
		
		{
			std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::in);
			if (ifs.fail())
				return false;
		
			if(CheckUsingNoChinesePathVersion())
			{
				try
				{
					std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
					if (str == "")
						return false;

					ATTaskInfo taskgraph(nlohmann::json::parse(str.begin(), str.end()));
	

					job_ = taskgraph.job_;
					blockItem_ = taskgraph.blockItem_;
					projectFile_ = taskgraph.projectFile_;
					task_ = taskgraph.task_;

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
				std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
				if (str == "")
					return false;

				ATTaskInfo taskgraph(str,0);
	#if 1			
				job_ = taskgraph.job_;
				blockItem_ = taskgraph.blockItem_;
				projectFile_ = taskgraph.projectFile_;

		// job2_ = UTF82GBK(job_);
		 job2_ = job_;
		// blockItem2_ = UTF82GBK(blockItem_);
		 blockItem2_ = blockItem_;
		// projectFile2_ = UTF82GBK(projectFile_);
		 projectFile2_ = projectFile_;
	#else
				job2_ = taskgraph.job_;
				blockItem2_ = taskgraph.blockItem_;
				projectFile2_ = taskgraph.projectFile_;

				// job_ = GBK2UTF8(job_);
				// blockItem_ = GBK2UTF8(blockItem_);
				// projectFile_ = GBK2UTF8(projectFile_);
	#endif
				task_ = taskgraph.task_;

				ifs.close();
			}
		}
		
		return true;
	}

	bool Save(const std::string& file)
	{
		if (TASK_USE_BIN) {
			return SaveBin(file);
		}
		else {
			return SaveJson(file);
		}
	}

	bool SaveJson(const std::string& file)
	{
		std::string ext = AI3D::CORE::File::GetFileExtension(file);
		AI3D::CORE::String::StringToLower(&ext);


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
			std::string outjson_str = WriteToJsonV2();

			std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::out);
			if (ofs.fail())
				return false;

			ofs << outjson_str;
			ofs.close();
		}

		

		return true;
	}

	bool LoadBin(const std::string& file) {
		std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(file, std::ios::binary);
		
		if (!in.is_open()) {
			LOGE("Load taskdef bin failed!");
			return false;
		}
			
		SPTaskInfoFile sPTaskInfoFile;
		sPTaskInfoFile.Deserialize(in);
		bool hasATParams = sPTaskInfoFile.hasATParam;
		bool hasRCParams = sPTaskInfoFile.hasRecParam;
		std::string blockItem = sPTaskInfoFile.blockItem;
		std::string projectfile = sPTaskInfoFile.projectfile;
		job_ = sPTaskInfoFile.jobName;
#ifdef WIN32
		// blockItem = UTF82GBK(blockItem);
		// projectfile = UTF82GBK(projectfile);
#endif 
		blockItem_ = blockItem;
		projectFile_ = projectfile;
		ATJson_ = sPTaskInfoFile.hasAT ? sPTaskInfoFile.ATFile : "";
		GCPJson_ = sPTaskInfoFile.hasGCP ? sPTaskInfoFile.GCPFile : "";

		task_.id_ = sPTaskInfoFile.taskMetaData.id;
		task_.msg_ = sPTaskInfoFile.taskMetaData.msg;
		task_.name_ = sPTaskInfoFile.taskMetaData.name;
		task_.begin_ = sPTaskInfoFile.taskMetaData.begin;
		task_.end_ = sPTaskInfoFile.taskMetaData.end;
		task_.fatherId_ = sPTaskInfoFile.taskMetaData.fatherId;
		task_.match_id_ = sPTaskInfoFile.taskMetaData.match_id;
		task_.type_ = sPTaskInfoFile.taskMetaData.type;
		task_.imgIds_.clear();
		task_.imgIds_.resize(sPTaskInfoFile.taskMetaData.imagNum);
		if (sPTaskInfoFile.taskMetaData.imagNum > 0) {
			for (size_t i = 0; i < sPTaskInfoFile.taskMetaData.imagNum; ++i) {
				task_.imgIds_[i] = sPTaskInfoFile.taskMetaData.imgIds[i];
			}
		}
		task_.depends_.clear();
		task_.depends_.resize(sPTaskInfoFile.taskMetaData.dependNum);
		if (sPTaskInfoFile.taskMetaData.dependNum > 0) {
			for (size_t i = 0; i < sPTaskInfoFile.taskMetaData.dependNum; ++i) {
				task_.depends_[i] = sPTaskInfoFile.taskMetaData.depends[i];
			}
		}
		task_.fun_name_ = sPTaskInfoFile.taskMetaData.functionName;
		task_.sfmmem_ = sPTaskInfoFile.taskMetaData.sfmmem;
		task_.sfmId_ = sPTaskInfoFile.taskMetaData.sfmId;
		task_.match_maximage_num_ = sPTaskInfoFile.taskMetaData.matchMaxImgNum;
		task_.key_maximage_num_ = sPTaskInfoFile.taskMetaData.keyMaxImgNum;
		task_.sfm_task_num_ = sPTaskInfoFile.taskMetaData.sfm_task_num;
		task_.match_task_num_ = sPTaskInfoFile.taskMetaData.match_task_num;
		task_.matchIds_.clear();
		task_.matchIds_.resize(sPTaskInfoFile.taskMetaData.matchIdNum);
		if (sPTaskInfoFile.taskMetaData.matchIdNum > 0) {
			for (size_t i = 0; i < sPTaskInfoFile.taskMetaData.matchIdNum; ++i) {
				task_.matchIds_[i] = sPTaskInfoFile.taskMetaData.matchIds[i];
			}
		}
		in.close();
		return true;
	}

	bool SaveBin(const std::string& file) {
		std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(file, std::ios::binary);
		
		if (!out.is_open()) {
			LOGE("Save taskdef bin failed!");
			return false;
		}
		SPTaskInfoFile sPTaskInfoFile;
		sPTaskInfoFile.hasATParam = true;           
		sPTaskInfoFile.hasRecParam = false;
		std::string blockItem = blockItem_;
		std::string projectfile = projectFile_;
		sPTaskInfoFile.jobName = job_;
#ifdef WIN32
		// blockItem = GBK2UTF8(blockItem);
		// projectfile = GBK2UTF8(projectfile);
#endif 
		sPTaskInfoFile.blockItem = blockItem;
		sPTaskInfoFile.projectfile = projectfile;
		sPTaskInfoFile.hasAT = false;
		if (ATJson_ != "") {
			sPTaskInfoFile.hasAT = true;
			sPTaskInfoFile.ATFile = ATJson_;
		}
		sPTaskInfoFile.hasGCP = false;
		if (GCPJson_ != "") {
			sPTaskInfoFile.hasGCP = true;
			sPTaskInfoFile.GCPFile = GCPJson_;
		}
		sPTaskInfoFile.taskMetaData.id = task_.id_;
		sPTaskInfoFile.taskMetaData.msg = task_.msg_;
		sPTaskInfoFile.taskMetaData.name = task_.name_;
		sPTaskInfoFile.taskMetaData.begin = task_.begin_;
		sPTaskInfoFile.taskMetaData.end = task_.end_;
		sPTaskInfoFile.taskMetaData.fatherId = task_.fatherId_;
		sPTaskInfoFile.taskMetaData.match_id = task_.match_id_;
		sPTaskInfoFile.taskMetaData.type = task_.type_;
		sPTaskInfoFile.taskMetaData.imgIds.clear();
		sPTaskInfoFile.taskMetaData.imagNum = task_.imgIds_.size();
		if (task_.imgIds_.size() > 0)
		{
			for (auto imgId_ : task_.imgIds_)
			{
				int id = imgId_;
				sPTaskInfoFile.taskMetaData.imgIds.push_back(id);
			}
		}
		sPTaskInfoFile.taskMetaData.depends.clear();
		sPTaskInfoFile.taskMetaData.dependNum = task_.depends_.size();
		if (task_.depends_.size() > 0)
		{
			for (auto depend_ : task_.depends_)
			{
				int id = depend_;
				sPTaskInfoFile.taskMetaData.depends.push_back(id);
			}
		}
		sPTaskInfoFile.taskMetaData.functionName = task_.fun_name_;
		sPTaskInfoFile.taskMetaData.sfmmem = task_.sfmmem_;
		sPTaskInfoFile.taskMetaData.sfmId = task_.sfmId_;
		sPTaskInfoFile.taskMetaData.matchMaxImgNum = task_.match_maximage_num_;
		sPTaskInfoFile.taskMetaData.keyMaxImgNum = task_.key_maximage_num_;
		sPTaskInfoFile.taskMetaData.sfm_task_num = task_.sfm_task_num_;
		sPTaskInfoFile.taskMetaData.match_task_num = task_.match_task_num_;
		sPTaskInfoFile.taskMetaData.matchIdNum = task_.matchIds_.size();
		if (task_.matchIds_.size() > 0)
		{
			for (auto matchid : task_.matchIds_)
			{
				int id = matchid;
				sPTaskInfoFile.taskMetaData.matchIds.push_back(id);
			}
		}

		sPTaskInfoFile.Serialize(out);

		out.close();
		return true;
	}

	static ATTaskInfo CreateFromJsonV2(std::string json_str)
	{
		ATTaskInfo taskInfo;
		if (json_str.empty())
			return taskInfo;


		rapidjson::Document document;
		if (document.Parse(json_str.data()).HasParseError())
			return taskInfo;		

		if (document.HasMember("job"))
		{
			taskInfo.job_ = document["job"].GetString();
			// taskInfo.job2_ = UTF82GBK(taskInfo.job_);
			taskInfo.job2_ = taskInfo.job_;
			
			
		}

		if (document.HasMember("blockItem"))
		{
			taskInfo.blockItem_ = document["blockItem"].GetString();
			// taskInfo.blockItem2_ = UTF82GBK(taskInfo.blockItem_);
			taskInfo.blockItem2_ = taskInfo.blockItem_;

			
			
		}

		if (document.HasMember("projectPath"))
		{
			taskInfo.projectFile_ = document["projectPath"].GetString();
			// taskInfo.projectFile2_ = UTF82GBK(taskInfo.projectFile_);
			taskInfo.projectFile2_ = taskInfo.projectFile_;

			
			

			
			
		}

		if (document.HasMember("GCPJson"))
		{
			taskInfo.GCPJson_ = document["GCPJson"].GetString();
			// taskInfo.GCPJson2_ = UTF82GBK(taskInfo.GCPJson_);
			taskInfo.GCPJson2_ = taskInfo.GCPJson_;
		}

		if (document.HasMember("ATJson"))
		{
			taskInfo.ATJson_ = document["ATJson"].GetString();
			// taskInfo.ATJson2_ = UTF82GBK(taskInfo.ATJson_);
			taskInfo.ATJson2_ = taskInfo.ATJson_;
		}

		if (document.HasMember("meta_data"))
		{
			
			



			rapidjson::Value& value = document["meta_data"];
			if (value.HasMember("id"))
			{
				taskInfo.task_.id_ = value["id"].GetInt();
			}

			if (value.HasMember("msg"))
			{
				taskInfo.task_.msg_ = value["msg"].GetString();
				// taskInfo.task_.msg2_ = UTF82GBK(taskInfo.task_.msg_);
				taskInfo.task_.msg2_ = taskInfo.task_.msg_;
			}

			if (value.HasMember("name"))
			{
				taskInfo.task_.name_ = value["name"].GetString();
				// taskInfo.task_.name2_ = UTF82GBK(taskInfo.task_.name_);
				taskInfo.task_.name2_ = taskInfo.task_.name_;
			}

			if (value.HasMember("type"))
			{
				taskInfo.task_.type_ = value["type"].GetInt();
			}

			if (value.HasMember("begin"))
			{
				taskInfo.task_.begin_ = value["begin"].GetFloat();
			}

			if (value.HasMember("end"))
			{
				taskInfo.task_.end_ = value["end"].GetFloat();
			}

			if (value.HasMember("sfmmem"))
			{
				taskInfo.task_.sfmmem_ = value["sfmmem"].GetFloat();
			}

			if (value.HasMember("fatherId"))
			{
				taskInfo.task_.fatherId_ = value["fatherId"].GetInt();
			}

			if (value.HasMember("depends"))
			{
				rapidjson::Value& valDepends = value["depends"];
				if (valDepends.IsArray() && valDepends.Size() > 0)
				{
					for (unsigned idx = 0; idx < valDepends.Size(); idx++)
					{
						rapidjson::Value& valDepend = valDepends[idx];
						taskInfo.task_.depends_.push_back(valDepend.GetInt());
					}
				}
			}

			if (value.HasMember("imgIds"))
			{
				rapidjson::Value& valImgIds = value["imgIds"];
				if (valImgIds.IsArray() && valImgIds.Size() > 0)
				{
					for (unsigned idx = 0; idx < valImgIds.Size(); idx++)
					{
						rapidjson::Value& valImgId = valImgIds[idx];
						taskInfo.task_.imgIds_.push_back(valImgId.GetInt());
					}
				}
			}

			if (value.HasMember("function"))
			{
				taskInfo.task_.fun_name_ = value["function"].GetString();
				// taskInfo.task_.fun_name2_ = UTF82GBK(taskInfo.task_.fun_name_);
				taskInfo.task_.fun_name2_ = taskInfo.task_.fun_name_;
			}
		}

		return taskInfo;
	}


	nlohmann::json WriteToJson()
	{
		nlohmann::json task_json;
		task_json["job"] = job_;
		task_json["blockItem"] = blockItem_;
		task_json["projectPath"] = projectFile_;
		if(GCPJson_!="")
			task_json["GCPJson"] = GCPJson_;
		if (ATJson_ != "")
		task_json["ATJson"] = ATJson_;
		task_json["meta_data"] = task_.WriteToJson();
		return task_json;
	};


	std::string WriteToJsonV2()
	{
		std::string json_str;

		rapidjson::Document document;
		document.SetObject();
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		document.AddMember("job", rapidjson::Value(job_.data(), allocator), allocator);
		document.AddMember("blockItem", rapidjson::Value(blockItem_.data(), allocator), allocator);
		document.AddMember("projectPath", rapidjson::Value(projectFile_.data(), allocator), allocator);
		if (GCPJson_ != "")
		{
			document.AddMember("GCPJson",rapidjson::Value(GCPJson_.data(),allocator),allocator);
		}

		if (ATJson_ != "")
		{
			document.AddMember("ATJson",rapidjson::Value(ATJson_.data(),allocator),allocator);
		}

		{
			
			
			
			

			rapidjson::Value val(rapidjson::kObjectType);

			val.AddMember("id", rapidjson::Value(task_.id_), allocator);
			val.AddMember("msg", rapidjson::Value(task_.msg_.data(), allocator), allocator);
			val.AddMember("name", rapidjson::Value(task_.name_.data(), allocator), allocator);
			val.AddMember("type", rapidjson::Value(task_.type_), allocator);
			val.AddMember("begin", rapidjson::Value(task_.begin_), allocator);
			val.AddMember("end", rapidjson::Value(task_.end_), allocator);
			val.AddMember("sfmmem", rapidjson::Value(task_.sfmmem_), allocator);
			val.AddMember("fatherId", rapidjson::Value(task_.fatherId_), allocator);

			if (task_.imgIds_.size() > 0)
			{
				rapidjson::Value valImgIds(rapidjson::kArrayType);
				for (auto imgId_ : task_.imgIds_)
				{
					valImgIds.PushBack(rapidjson::Value(imgId_), allocator);
				}

				val.AddMember("imgIds", rapidjson::Value(valImgIds, allocator), allocator);
			}

			if (task_.depends_.size() > 0)
			{
				rapidjson::Value valDepends(rapidjson::kArrayType);
				for (auto depend_ : task_.depends_)
				{
					valDepends.PushBack(rapidjson::Value(depend_), allocator);
				}
				val.AddMember("depends", rapidjson::Value(valDepends, allocator), allocator);
			}

			val.AddMember("function", rapidjson::Value(task_.fun_name_.data(), allocator), allocator);

			document.AddMember("meta_data", val, allocator);
		}

		document.Accept(writer);
		json_str = buffer.GetString();

		return json_str;
	};



	ATTaskInfo(const nlohmann::json& json_str)
	{
		using namespace nlohmann;
		try
		{
			job_ = json_str.at("job").get<std::string>();
			blockItem_ = json_str.at("blockItem").get<std::string>();
			projectFile_ = json_str.at("projectPath").get<std::string>();

			if (json_str.find("ATJson") != json_str.end())
			{
				ATJson_ = json_str.at("ATJson").get<std::string>();
			}
			if (json_str.find("GCPJson") != json_str.end())
			{
				GCPJson_ = json_str.at("GCPJson").get<std::string>();
			}
			json images_json;
			const auto _it = json_str.find("meta_data");
			if (_it != json_str.end())
			{
				

				TaskDescriptor task;
				task_ = task.CreateFromJson(*_it);
				
				

			}
		}
		catch (std::exception& ex)
		{
			std::ostringstream oss;
			oss << "exception:" << ex.what();
			LOGI(oss.str());
		}
	}


	ATTaskInfo(std::string json_str,int dummy)
	{
		ATTaskInfo atTaskInfo = CreateFromJsonV2(json_str);
		*this = atTaskInfo;
	}
};

#endif