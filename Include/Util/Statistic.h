#ifndef _STATISTIC_H
#define _STATISTIC_H
#include <iostream>
#include <vector>
#include <map>
#include <boost/regex.hpp>
#include <Util/TaskProcess.h>
#ifdef _MSC_VER
#include <Windows.h>
#endif 
extern bool TextSaveFile(const std::string& path, const std::string& strs);
extern bool TextReadFile(const std::string& path, std::string& strs);

typedef struct APPUseInfo_
{
	std::string VersionName;
	std::string VersionCode;
	std::string language;
	std::string StartTime;
	std::string QuitTime;
	std::map<std::string, int> PhotosOfDir;
	std::map<std::string, int> AtJobPercent;
}APPUseInfo;

namespace AI3D {
	namespace Util {
		
		static bool GetMacByCmd(std::string& macOUT);
		static bool GetUUIDByCmd(std::string& uuidOUT);

		static bool ParseMac(const std::string& str, std::string& macOUT);
		static bool ParseUUID(const std::string& str, std::string& uuidOUT);

		static const std::string GetMachineCode();
	

bool ParseMac(const std::string& str, std::string& macOUT)
{
	const static boost::regex expression(
		"([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})-([0-9a-fA-F]{2})",
		boost::regex::perl | boost::regex::icase);
	boost::cmatch what;
	if (boost::regex_search(str.c_str(), what, expression))
	{
		macOUT = what[1] + "-" + what[2] + "-" + what[3] + "-" + what[4] + "-" + what[5] + "-" + what[6];
		return true;
	}
	return false;
}

bool ParseUUID(const std::string& str, std::string& uuidOUT)
{
	const static boost::regex expression(
		"UUID=([0-9a-fA-F]{8})-([0-9a-fA-F]{4})-([0-9a-fA-F]{4})-([0-9a-fA-F]{4})-([0-9a-fA-F]{12})", boost::regex::perl | boost::regex::icase
	);
	boost::cmatch what;
	if (boost::regex_search(str.c_str(), what, expression))
	{
		uuidOUT = what[1] + "-" + what[2] + "-" + what[3] + "-" + what[4] + "-" + what[5];
		return true;
	}
	return false;
}

bool GetMacByCmd(std::string& macOUT)
{
	bool ret = false;
#ifdef _MSC_VER
	
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	
	HANDLE hReadPipe, hWritePipe;
	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
	{
		
		STARTUPINFO si;
		
		PROCESS_INFORMATION pi;
		si.cb = sizeof(STARTUPINFO);
		GetStartupInfo(&si);
		si.hStdError = hWritePipe;
		si.hStdOutput = hWritePipe;
		si.wShowWindow = SW_HIDE; 
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		
		TCHAR szCommandLine[] = TEXT("ipconfig /all");
		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
		{
			WaitForSingleObject(pi.hProcess, 3000); 
			unsigned long count;
			CloseHandle(hWritePipe);
			std::string strBuffer(1024 * 10, '\0'); 
			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
			{
				strBuffer.resize(strBuffer.find_first_of('\0')); 
				ret = ParseMac(strBuffer, macOUT);
			}
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		else
		{
			CloseHandle(hWritePipe); 
		}
		CloseHandle(hReadPipe);
	}
#endif 


	return ret;
}
const std::string GetMachineCode()
{
	std::string Mac;
	std::string UUID;
	AI3D::Util::GetMacByCmd(Mac);
	AI3D::Util::GetUUIDByCmd(UUID);
	return Mac + "_" + UUID;
}
bool GetUUIDByCmd(std::string& uuidOUT)
{
	bool ret = false;
#ifdef _MSC_VER
	
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	
	HANDLE hReadPipe, hWritePipe;
	if (CreatePipe(&hReadPipe, &hWritePipe, &sa, 0) == TRUE)
	{
		
		STARTUPINFO si;
		
		PROCESS_INFORMATION pi;
		si.cb = sizeof(STARTUPINFO);
		GetStartupInfo(&si);
		si.hStdError = hWritePipe;
		si.hStdOutput = hWritePipe;
		si.wShowWindow = SW_HIDE; 
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		
		TCHAR szCommandLine[] = TEXT("wmic csproduct list full");
		if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
		{
			WaitForSingleObject(pi.hProcess, 3000); 
			unsigned long count;
			CloseHandle(hWritePipe);
			std::string strBuffer(1024 * 10, '\0'); 
			if (ReadFile(hReadPipe, const_cast<char*>(strBuffer.data()), strBuffer.size() - 1, &count, 0) == TRUE)
			{
				strBuffer.resize(strBuffer.find_first_of('\0')); 
				ret = ParseUUID(strBuffer, uuidOUT);
			}
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		else
		{
			CloseHandle(hWritePipe); 
		}
		CloseHandle(hReadPipe);
	}
#endif 


	return ret;
}
}
}

class EngineInfo
{
public:
	EngineInfo();
	static EngineInfo& Getinstance()
	{
		static EngineInfo app;
		return app;
	}
	bool LoadEngineInfoJson();
	bool LoadEngineInfoBin();
	bool ExportEngineInfoJson();
	bool ExportEngineInfoBin();
	std::string& GetMachineCodeMutual();
	std::string GetMachineCode()const;
	std::vector<APPUseInfo>& GetAPPUseInfosMutual();
	bool ParseConfig(const std::string& ConfigPath, std::string& versionName,std::string&versionCode);
	bool ParseConfig(const std::string& ConfigPath, std::string& versionName, std::string& versionCode,std::string &language);
	std::string& GetEngineJsonPathMutual();
	std::string GetEngineJsonPath()const;

private:
	std::string jsonenginepath_;
	std::string MachineCode_;
	std::vector<APPUseInfo> useinfos_;
	EngineInfo_s machineinfo_;
};



class MasterInfo
{
public:
	static MasterInfo& Getinstance()
	{
		static MasterInfo app;
		return app;
	}
	bool LoadMasterInfoJson(const std::string& MasterJsonPath);
	bool ExportMasterInfoJson(const std::string& MasterJsonPath);
	std::string& GetMachineCodeMutual();
	std::string GetMachineCode()const;
	std::vector<APPUseInfo>& GetAPPUseInfosMutual();

private:
	std::string MachineCode_;
	std::vector<APPUseInfo> useinfos_;
};




#endif 



