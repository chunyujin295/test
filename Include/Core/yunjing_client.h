#pragma once
#ifdef API_CLIENT_DLL_EXPORTS
#define API_CLIENT_DLL_API __declspec(dllexport)
#else
#define API_CLIENT_DLL_API __declspec(dllimport)
#endif

extern "C" {
	API_CLIENT_DLL_API char* TaskStartApi(char* param);
	API_CLIENT_DLL_API char* TaskStateApi(char* param);
	API_CLIENT_DLL_API char* TaskStopApi(char* param);
	API_CLIENT_DLL_API char* TaskMergeApi(char* param);
}
