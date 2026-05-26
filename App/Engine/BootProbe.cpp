#include "BootProbe.h"

#include <Windows.h>

#include <cstdio>
#include <cstring>

namespace {

static void WriteBootProbeLine(const char* phase)
{
	static const char kBootProbeMarker[] = "BOOT_PROBE_MARKER_V2_20260324";

	const char* logPath =
		"C:\\Users\\Microsoft\\Code\\front_end\\3d_build_C17\\MoldAI\\Bin\\x64\\Release\\engine_boot_probe.log";

	char modulePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, modulePath, MAX_PATH);

	SYSTEMTIME st;
	GetLocalTime(&st);

	char line[1024] = { 0 };
	_snprintf_s(
		line,
		sizeof(line),
		_TRUNCATE,
		"%04d-%02d-%02d %02d:%02d:%02d.%03d pid=%lu phase=%s exe=%s marker=%s\r\n",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		static_cast<unsigned long>(GetCurrentProcessId()),
		(phase != nullptr ? phase : "null"),
		modulePath,
		kBootProbeMarker
	);

	HANDLE hFile = CreateFileA(
		logPath,
		FILE_APPEND_DATA,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten = 0;
		WriteFile(hFile, line, static_cast<DWORD>(strlen(line)), &bytesWritten, NULL);
		CloseHandle(hFile);
	}

	OutputDebugStringA(line);
}

struct BootProbeStaticInitializer
{
	BootProbeStaticInitializer()
	{
		WriteBootProbeLine("before_main_static_ctor");
	}
};

static BootProbeStaticInitializer g_bootProbeStaticInitializer;

} 

void EngineBootProbeMainEnter()
{
	WriteBootProbeLine("main_enter");
}
