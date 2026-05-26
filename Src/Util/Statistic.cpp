#include "Util/Statistic.h"
#include "Core/File.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"


#include <fstream>
#include <sstream>
#include <string>



bool MasterInfo::ExportMasterInfoJson(const std::string& MasterJsonPath)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	rapidjson::Document document;
	document.SetObject();

	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

	document.AddMember("MachineCode", rapidjson::Value(MachineCode_.c_str(), allocator), allocator);
	rapidjson::Value versions(rapidjson::kArrayType);
	for (const auto& useinfo : useinfos_)
	{
		rapidjson::Value version(rapidjson::kObjectType);
		version.AddMember("VersionName", rapidjson::Value(useinfo.VersionName.c_str(), allocator), allocator);
		version.AddMember("VersionCode", rapidjson::Value(useinfo.VersionCode.c_str(), allocator), allocator);
		version.AddMember("StartTime", rapidjson::Value(useinfo.StartTime.c_str(), allocator), allocator);
		version.AddMember("QuitTime", rapidjson::Value(useinfo.QuitTime.c_str(), allocator), allocator);
		rapidjson::Value photodirobj(rapidjson::kObjectType);
		for (const auto& photodir : useinfo.PhotosOfDir)
		{
			photodirobj.AddMember(rapidjson::GenericStringRef<char>(photodir.first.c_str()), rapidjson::Value(photodir.second), allocator);
		}
		version.AddMember("PhotosOfDir", photodirobj, allocator);

		rapidjson::Value atJobObj(rapidjson::kObjectType);
		for (const auto& atJob : useinfo.AtJobPercent)
		{
			atJobObj.AddMember(rapidjson::GenericStringRef<char>(atJob.first.c_str()),rapidjson::Value(atJob.second),allocator);
		}
		version.AddMember("PercentOfATJobs",atJobObj,allocator);

		versions.PushBack(version, allocator);
	}
	document.AddMember("Versions", versions, allocator);

	document.Accept(writer);

	if (!TextSaveFile(MasterJsonPath, buffer.GetString()) )
	{
		LOGE("File save error:" + MasterJsonPath);
		return false;
	}
	return true;
}

bool MasterInfo::LoadMasterInfoJson(const std::string& MasterJsonPath)
{
	std::string mastercontent;
	bool ret = TextReadFile(MasterJsonPath, mastercontent);
	if (!ret)
	{
		LOGE("File read error:" + MasterJsonPath);
		return false;
	}

	rapidjson::Document doc_master;

	if (doc_master.Parse(mastercontent.data()).HasParseError())
	{
		LOGE("File parse error:" + MasterJsonPath);
		return false;
	}

	if (!doc_master.IsObject())
	{
		LOGE("File content error:" + MasterJsonPath);
		return false;
	}

	if (doc_master.HasMember("MachineCode"))
	{
		MachineCode_ = doc_master["MachineCode"].GetString();
	}
	if (doc_master.HasMember("Versions"))
	{
		std::vector<APPUseInfo> useinfos;
		rapidjson::Value& Versions = doc_master["Versions"];
		for (int i = 0; i < Versions.Size(); i++)
		{
			APPUseInfo appuseinfo;
			if (Versions[i].HasMember("VersionName"))
			{
				appuseinfo.VersionName = Versions[i]["VersionName"].GetString();
			}
			if (Versions[i].HasMember("VersionCode"))
			{
				appuseinfo.VersionCode = Versions[i]["VersionCode"].GetString();
			}
			if (Versions[i].HasMember("StartTime"))
			{
				appuseinfo.StartTime = Versions[i]["StartTime"].GetString();
			}
			if (Versions[i].HasMember("QuitTime"))
			{
				appuseinfo.QuitTime = Versions[i]["QuitTime"].GetString();
			}
			if (Versions[i].HasMember("PhotosOfDir"))
			{
				for (rapidjson::Value::MemberIterator iter = Versions[i]["PhotosOfDir"].MemberBegin(); iter != Versions[i]["PhotosOfDir"].MemberEnd(); iter++)
				{
					const char* key = iter->name.GetString();
					const rapidjson::Value& val = iter->value;
					
					if (val.IsInt())
					{
						appuseinfo.PhotosOfDir.insert(std::make_pair(key, val.GetInt()));
					}
				}
			}

			if (Versions[i].HasMember("PercentOfATJobs"))
			{
				for (rapidjson::Value::MemberIterator iter = Versions[i]["PercentOfATJobs"].MemberBegin(); iter != Versions[i]["PercentOfATJobs"].MemberEnd(); iter++)
				{
					const char* key = iter->name.GetString();
					const rapidjson::Value& val = iter->value;
					
					if (val.IsInt())
					{
						appuseinfo.AtJobPercent.insert(std::make_pair(key, val.GetInt()));
					}
				}

			}

			useinfos.push_back(appuseinfo);
		}
		useinfos_ = useinfos;
	}
	return true;
}

std::string& MasterInfo::GetMachineCodeMutual()
{
	return MachineCode_;
}

std::string MasterInfo::GetMachineCode() const
{
	return MachineCode_;
}

std::vector<APPUseInfo>& MasterInfo::GetAPPUseInfosMutual()
{
	
	return useinfos_;

}

bool EngineInfo::ExportEngineInfoBin()
{
	std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(jsonenginepath_, std::ios::binary);
	
	if (!out.is_open()) {
		LOGE("Save engine state bin failed!");
		return false;
	}
	EngineStatFile engineStatFile;
	engineStatFile.machineCode = MachineCode_;
	int versionNum = useinfos_.size();
	engineStatFile.versionListNum = versionNum;
	engineStatFile.versionList.clear();
	for (const auto& useinfo : useinfos_) {
		EngineInfoData engineInfoData;
		engineInfoData.versionName = useinfo.VersionName;
		engineInfoData.versionCode = useinfo.VersionCode;
		engineInfoData.startTime = useinfo.StartTime;
		engineInfoData.quitTime = useinfo.QuitTime;
		int photoMapNum = useinfo.PhotosOfDir.size();
		engineInfoData.photoMapNum = photoMapNum; 
		engineInfoData.photoMap.clear();
		for (const auto& photodir : useinfo.PhotosOfDir)
		{
			StaticMapData staticMapData;
			staticMapData.key = photodir.first;
			staticMapData.value = photodir.second;
			engineInfoData.photoMap.push_back(staticMapData);
		}
		int percentMapNum = useinfo.AtJobPercent.size();
		engineInfoData.percentMapNum = percentMapNum;
		engineInfoData.percentMap.clear();
		for (const auto& atJob : useinfo.AtJobPercent)
		{
			StaticMapData staticMapData;
			staticMapData.key = atJob.first;
			staticMapData.value = atJob.second;
			engineInfoData.percentMap.push_back(staticMapData);
		}
		engineStatFile.versionList.push_back(engineInfoData);
	}

	engineStatFile.Serialize(out);
	out.close();

	return true;
}

bool EngineInfo::ExportEngineInfoJson()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	rapidjson::Document document;
	document.SetObject();

	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

	document.AddMember("MachineCode", rapidjson::Value(MachineCode_.c_str(), allocator), allocator);
	rapidjson::Value versions(rapidjson::kArrayType);
	for (const auto& useinfo : useinfos_)
	{
		rapidjson::Value version(rapidjson::kObjectType);
		version.AddMember("VersionName", rapidjson::Value(useinfo.VersionName.c_str(), allocator), allocator);
		version.AddMember("VersionCode", rapidjson::Value(useinfo.VersionCode.c_str(), allocator), allocator);
		version.AddMember("StartTime", rapidjson::Value(useinfo.StartTime.c_str(), allocator), allocator);
		version.AddMember("QuitTime", rapidjson::Value(useinfo.QuitTime.c_str(), allocator), allocator);
		rapidjson::Value photodirobj(rapidjson::kObjectType);
		for (const auto& photodir : useinfo.PhotosOfDir)
		{
			photodirobj.AddMember(rapidjson::GenericStringRef<char>(photodir.first.c_str()), rapidjson::Value(photodir.second), allocator);
		}
		version.AddMember("PhotosOfDir", photodirobj, allocator);

		rapidjson::Value atJobObj(rapidjson::kObjectType);
		for (const auto& atJob : useinfo.AtJobPercent)
		{
			atJobObj.AddMember(rapidjson::GenericStringRef<char>(atJob.first.c_str()), rapidjson::Value(atJob.second), allocator);
		}
		version.AddMember("PercentOfATJobs", atJobObj, allocator);

		versions.PushBack(version, allocator);
	}
	document.AddMember("Versions", versions, allocator);

	document.Accept(writer);

	if (!TextSaveFile(jsonenginepath_, buffer.GetString()))
	{
		LOGE("save file error: " + jsonenginepath_);
		return false;
	}
	return true;
}

EngineInfo::EngineInfo()
{
	std::string UUID;
	AI3D::Util::GetUUIDByCmd(UUID);
	MachineCode_ = UUID;
}

bool EngineInfo::LoadEngineInfoBin()
{
	std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(jsonenginepath_, std::ios::binary);
	
	if (!in.is_open()) {
		LOGE("Load engine stat bin failed!");
		return false;
	}

	EngineStatFile engineStatFile;
	engineStatFile.Deserialize(in);
	MachineCode_ = engineStatFile.machineCode;
	int versionNum = engineStatFile.versionListNum;
	std::vector<APPUseInfo> useinfos;
	for (int i = 0; i < versionNum; i++) {
		EngineInfoData engineInfoData = engineStatFile.versionList[i];
		APPUseInfo appuseinfo;
		appuseinfo.VersionName = engineInfoData.versionName;
		appuseinfo.VersionCode = engineInfoData.versionCode;
		appuseinfo.StartTime = engineInfoData.startTime;
		appuseinfo.QuitTime = engineInfoData.quitTime;
		int photoMapNum = engineInfoData.photoMapNum;
		for (int i = 0; i < photoMapNum; i++)
		{
			StaticMapData staticMapData = engineInfoData.photoMap[i];
			std::string key = staticMapData.key;
			int value = staticMapData.value;
			
			appuseinfo.PhotosOfDir.insert(std::make_pair(key, value));
		}
		int percentMapNum = engineInfoData.percentMapNum;
		for (int i = 0; i < percentMapNum; i++)
		{
			StaticMapData staticMapData = engineInfoData.percentMap[i];
			std::string key = staticMapData.key;
			int value = staticMapData.value;
			
			appuseinfo.AtJobPercent.insert(std::make_pair(key, value));
		}
		useinfos.push_back(appuseinfo);
	}
	useinfos_ = useinfos;
	in.close();

	return true;
}

bool EngineInfo::LoadEngineInfoJson()
{
	std::string mastercontent;
	bool ret = TextReadFile(jsonenginepath_, mastercontent);
	if (!ret)
	{
		LOGE("File read error: " + jsonenginepath_);
		return false;
	}

	rapidjson::Document doc_master;

	if (doc_master.Parse(mastercontent.data()).HasParseError())
	{
		LOGE("File parse error: " + jsonenginepath_);
		return false;
	}

	if (!doc_master.IsObject())
	{
		LOGE("File content error: " + jsonenginepath_);
		return false;
	}

	if (doc_master.HasMember("MachineCode"))
	{
		MachineCode_ = doc_master["MachineCode"].GetString();
	}
	if (doc_master.HasMember("Versions"))
	{
		std::vector<APPUseInfo> useinfos;
		rapidjson::Value& Versions = doc_master["Versions"];
		for (int i = 0; i < Versions.Size(); i++)
		{
			APPUseInfo appuseinfo;
			if (Versions[i].HasMember("VersionName"))
			{
				appuseinfo.VersionName = Versions[i]["VersionName"].GetString();
			}
			if (Versions[i].HasMember("VersionCode"))
			{
				appuseinfo.VersionCode = Versions[i]["VersionCode"].GetString();
			}
			if (Versions[i].HasMember("StartTime"))
			{
				appuseinfo.StartTime = Versions[i]["StartTime"].GetString();
			}
			if (Versions[i].HasMember("QuitTime"))
			{
				appuseinfo.QuitTime = Versions[i]["QuitTime"].GetString();
			}
			if (Versions[i].HasMember("PhotosOfDir"))
			{
				for (rapidjson::Value::MemberIterator iter = Versions[i]["PhotosOfDir"].MemberBegin(); iter != Versions[i]["PhotosOfDir"].MemberEnd(); iter++)
				{
					const char* key = iter->name.GetString();
					const rapidjson::Value& val = iter->value;
					
					if (val.IsInt())
					{
						appuseinfo.PhotosOfDir.insert(std::make_pair(key, val.GetInt()));
					}
				}
			}

			if (Versions[i].HasMember("PercentOfATJobs"))
			{
				for (rapidjson::Value::MemberIterator iter = Versions[i]["PercentOfATJobs"].MemberBegin(); iter != Versions[i]["PercentOfATJobs"].MemberEnd(); iter++)
				{
					const char* key = iter->name.GetString();
					const rapidjson::Value& val = iter->value;
					
					if (val.IsInt())
					{
						appuseinfo.AtJobPercent.insert(std::make_pair(key, val.GetInt()));
					}
				}

			}

			useinfos.push_back(appuseinfo);
		}
		useinfos_ = useinfos;
	}
	return true;
}

std::string& EngineInfo::GetMachineCodeMutual()
{
	return MachineCode_;
}

std::string EngineInfo::GetMachineCode() const
{
	return MachineCode_;
}

std::vector<APPUseInfo>& EngineInfo::GetAPPUseInfosMutual()
{
	return useinfos_;
}

bool TextReadFile(const std::string& path, std::string& strs)
{

	std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::in);
	if (!in.is_open())
		return false;
	std::string line;

	while (std::getline(in, line)) {
		if (line.empty() || line.back() != '\n')
			line.append("\n");

		strs.append(line);
	}
	in.close();

	return true;
}


bool TextSaveFile(const std::string& path, const std::string& strs)
{
	std::ofstream fileout = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::out);
	if (!fileout.good())
		return false;

	fileout << strs;
	fileout.close();

	return true;
}

bool EngineInfo::ParseConfig(const std::string& ConfigPath, std::string& versionName, std::string& versionCode)
{
	std::string strBuffer;
	TextReadFile(ConfigPath, strBuffer);

	try
	{
		const static boost::regex expression(
			">([0-9]{1,2})\\.([0-9]{1,2})\\.([0-9]{1,3})</version>",
			boost::regex::perl | boost::regex::icase);
		boost::cmatch what;
		if (boost::regex_search(strBuffer.c_str(), what, expression))
		{
			versionName = what[1] + "." + what[2] + "." + what[3];
			std::vector<std::string> vectmp;
			vectmp.emplace_back(what[1]);
			vectmp.emplace_back(what[2]);
			vectmp.emplace_back(what[3]);

			vectmp[0] = vectmp[0].size() == 2 ? vectmp[0] : ("0" + vectmp[0]);
			vectmp[1] = vectmp[1].size() == 2 ? vectmp[1] : ("0" + vectmp[1]);
			vectmp[2] = vectmp[2].size() == 3 ? vectmp[2] : (vectmp[2].size() == 2 ? "0" + vectmp[2] : "00" + vectmp[2]);
			char strtmp[128];
			std::sprintf(strtmp, "%2s%2s%3s", vectmp[0], vectmp[1], vectmp[2]);
			versionCode = strtmp;
			return true;
		}
	}
	catch (std::exception& err)
	{
		std::ostringstream oss;
		oss << "exception:" << err.what();
		LOGI(oss.str());
	}
	return true;
}

bool EngineInfo::ParseConfig(const std::string& ConfigPath, std::string& versionName, std::string& versionCode, std::string& language)
{
	std::string strBuffer;
	TextReadFile(ConfigPath, strBuffer);

	try
	{
		const static boost::regex expression(
			">([0-9]{1,2})\\.([0-9]{1,2})\\.([0-9]{1,3})</VS>",
			boost::regex::perl | boost::regex::icase);

		const static boost::regex language_expression(
			">([0-1])</LI>",
			boost::regex::perl | boost::regex::icase);

		boost::cmatch what;
		if (boost::regex_search(strBuffer.c_str(), what, expression))
		{
			versionName = what[1] + "." + what[2] + "." + what[3];
			std::vector<std::string> vectmp;
			vectmp.emplace_back(what[1]);
			vectmp.emplace_back(what[2]);
			vectmp.emplace_back(what[3]);

			vectmp[0] = vectmp[0].size() == 2 ? vectmp[0] : ("0" + vectmp[0]);
			vectmp[1] = vectmp[1].size() == 2 ? vectmp[1] : ("0" + vectmp[1]);
			vectmp[2] = vectmp[2].size() == 3 ? vectmp[2] : (vectmp[2].size() == 2 ? "0" + vectmp[2] : "00" + vectmp[2]);
			char strtmp[128];
			std::sprintf(strtmp, "%2s%2s%3s", vectmp[0], vectmp[1], vectmp[2]);
			versionCode = strtmp;

		}

		if (boost::regex_search(strBuffer.c_str(), what, language_expression))
		{
			language = what[1];
		}

		return true;
	}
	catch (std::exception& err)
	{
		std::ostringstream oss;
		oss << "exception:" << err.what();
		LOGI(oss.str());
	}
	return true;
}

std::string& EngineInfo::GetEngineJsonPathMutual()
{
	return jsonenginepath_;
}
std::string EngineInfo::GetEngineJsonPath()const
{
	return jsonenginepath_;
}