















#include "Core/Logging.h"
#include "Core/Application.h"
#include "Core/File.h"
#include "Core/WorkPath.h"
#include <filesystem>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif








#ifndef MOLDAI_GLOG_LOGSINK_API
#define MOLDAI_GLOG_LOGSINK_API 1
#endif

namespace {

#ifdef _WIN32



static CRITICAL_SECTION g_moldai_unified_log_cs;
static bool g_moldai_unified_log_cs_inited = false;

static void MoldaiInitUnifiedLogCs()
{
	if (!g_moldai_unified_log_cs_inited) {
		InitializeCriticalSection(&g_moldai_unified_log_cs);
		g_moldai_unified_log_cs_inited = true;
	}
}

static void MoldaiFiniUnifiedLogCs()
{
	if (g_moldai_unified_log_cs_inited) {
		DeleteCriticalSection(&g_moldai_unified_log_cs);
		g_moldai_unified_log_cs_inited = false;
	}
}

struct MoldaiUnifiedLogLock {
	MoldaiUnifiedLogLock()
	{
		MoldaiInitUnifiedLogCs();
		EnterCriticalSection(&g_moldai_unified_log_cs);
	}
	~MoldaiUnifiedLogLock() { LeaveCriticalSection(&g_moldai_unified_log_cs); }
	MoldaiUnifiedLogLock(const MoldaiUnifiedLogLock&) = delete;
	MoldaiUnifiedLogLock& operator=(const MoldaiUnifiedLogLock&) = delete;
};

#else
static std::once_flag g_moldai_unified_log_mutex_once;
static std::unique_ptr<std::recursive_mutex> g_moldai_unified_log_mutex;

static std::recursive_mutex& GetUnifiedLogMutex()
{
	std::call_once(g_moldai_unified_log_mutex_once, [] {
		g_moldai_unified_log_mutex.reset(new std::recursive_mutex());
	});
	return *g_moldai_unified_log_mutex;
}
#endif

std::tm ResolveLocalTm(const std::tm* tm_time)
{
	std::tm out{};
	if (tm_time) {
		out = *tm_time;
	} else {
		const auto now = std::chrono::system_clock::now();
		const std::time_t tt = std::chrono::system_clock::to_time_t(now);
		out = *std::localtime(&tt);
	}
	return out;
}

const char* SeverityBracket(google::LogSeverity severity)
{
	if (severity == google::GLOG_WARNING) {
		return "[WARNING]";
	}
	if (severity == google::GLOG_ERROR) {
		return "[ERROR]";
	}
	if (severity == google::GLOG_FATAL) {
		return "[FATAL]";
	}
	return "[INFO]";
}

void WriteMoldaiUnifiedLogLine(const std::string& root_dir, google::LogSeverity severity,
	const std::tm& local_copy, const char* message, size_t message_len)
{
	if (root_dir.empty()) {
		return;
	}
	if (message_len > 0 && message == nullptr) {
		return;
	}
	while (message_len > 0) {
		const char c = message[message_len - 1];
		if (c != '\n' && c != '\r') {
			break;
		}
		--message_len;
	}

	
	
	std::tm lc = local_copy;
	char day_buf[16];
	char ts_buf[40];
	if (strftime(day_buf, sizeof(day_buf), "%Y-%m-%d", &lc) == 0u) {
		return;
	}
	if (strftime(ts_buf, sizeof(ts_buf), "%Y-%m-%d %H:%M:%S", &lc) == 0u) {
		return;
	}

	std::filesystem::path out = AI3D::CORE::File::BoostPathFromUtf8(root_dir)
		/ (std::string("MoldAI-") + std::string(day_buf) + ".log");
	const std::string utf8_log_path = AI3D::CORE::File::BoostPathToUtf8String(out);
	const char* const sev_str = SeverityBracket(severity);

#ifdef _WIN32
	MoldaiUnifiedLogLock lock;
#else
	std::lock_guard<std::recursive_mutex> lock(GetUnifiedLogMutex());
#endif
	std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(utf8_log_path, std::ios::binary | std::ios::app);
	if (!ofs.is_open()) {
		return;
	}
	ofs << '[' << ts_buf << "] " << sev_str << ' ';
	if (message_len > 0) {
		ofs.write(message, static_cast<std::streamsize>(message_len));
	}
	ofs.put('\n');
}

class MoldaiUnifiedLogSink final : public google::LogSink {
public:
	explicit MoldaiUnifiedLogSink(std::string root_dir) : root_dir_(std::move(root_dir)) {}

#if MOLDAI_GLOG_LOGSINK_API == 0
	void send(google::LogSeverity severity, const char* , int ,
		const std::tm* tm_time, const char* message, size_t message_len) override
	{
		SendToUnifiedFile(severity, tm_time, message, message_len);
	}
#elif MOLDAI_GLOG_LOGSINK_API == 1
	
	void send(google::LogSeverity severity, const char* ,
		const char* , int , const std::tm* tm_time, const char* message,
		size_t message_len, google::int32 ) override
	{
		SendToUnifiedFile(severity, tm_time, message, message_len);
	}
	void send(google::LogSeverity severity, const char* ,
		const char* , int , const std::tm* tm_time, const char* message,
		size_t message_len) override
	{
		SendToUnifiedFile(severity, tm_time, message, message_len);
	}
#else
	void send(google::LogSeverity severity, const char* ,
		const char* , int , const google::LogMessageTime& time,
		const char* message, size_t message_len) override
	{
		WriteMoldaiUnifiedLogLine(root_dir_, severity, time.tm(), message, message_len);
	}
#endif

private:
#if MOLDAI_GLOG_LOGSINK_API == 0 || MOLDAI_GLOG_LOGSINK_API == 1
	void SendToUnifiedFile(google::LogSeverity severity, const std::tm* tm_time,
		const char* message, size_t message_len)
	{
		const std::tm local_copy = ResolveLocalTm(tm_time);
		WriteMoldaiUnifiedLogLine(root_dir_, severity, local_copy, message, message_len);
	}
#endif
	std::string root_dir_;
};

std::unique_ptr<MoldaiUnifiedLogSink> g_unified_sink;

void TearDownMoldaiUnifiedSink()
{
	if (!g_unified_sink || !google::IsGoogleLoggingInitialized()) {
		return;
	}
	
	google::FlushLogFiles(google::GLOG_INFO);
	google::RemoveLogSink(g_unified_sink.get());
	g_unified_sink.reset();
#ifdef _WIN32
	MoldaiFiniUnifiedLogCs();
#endif
}

void DisableDefaultGlogFiles()
{
	google::SetLogDestination(google::GLOG_INFO, "");
	google::SetLogDestination(google::GLOG_WARNING, "");
	google::SetLogDestination(google::GLOG_ERROR, "");
}

void SetupMoldaiUnifiedSink(const std::string& log_dir)
{
	if (log_dir.empty()) {
		return;
	}
#ifdef _WIN32
	
	MoldaiInitUnifiedLogCs();
#endif
	g_unified_sink.reset(new MoldaiUnifiedLogSink(log_dir));
	google::AddLogSink(g_unified_sink.get());
}

} 








static void SetGlogDirFromPath(const std::string& log_dir)
{
	if (log_dir.empty()) {
		return;
	}
	std::filesystem::path p = AI3D::CORE::File::BoostPathFromUtf8(log_dir);
	p = p.lexically_normal();
	std::error_code ec;
	std::filesystem::create_directories(p, ec);
	(void)ec;
	FLAGS_log_dir = AI3D::CORE::File::BoostPathToUtf8String(p.make_preferred());
}

AI3D_API std::string BuildLogPrefix(int level)
{
	const char* level_tag = "[INFO]";
	if (level == LOG_WARN) {
		level_tag = "[WARNING]";
	} else if (level == LOG_ERROR || level == LOG_FATAL) {
		level_tag = "[ERROR]";
	}
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm local_time = *std::localtime(&now_time);
	std::ostringstream oss;
	oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
	return "[" + oss.str() + "] " + std::string(level_tag) + " ";
}

extern "C" __declspec(dllexport) void LogConsole(std::string msg)
{
	std::cout << BuildLogPrefix(0) << msg << std::endl;
}

extern "C" __declspec(dllexport) const char* LogPerifix(int level)
{
	static thread_local std::string s_prefix;
	s_prefix = BuildLogPrefix(level);
	return s_prefix.c_str();
}





    
   

void  ShutDownLog()
{
	TearDownMoldaiUnifiedSink();
    if (google::IsGoogleLoggingInitialized())
    {
        google::ShutdownGoogleLogging();
    }
}





















void SetLogStyle(bool alsologtostderr =true,int minloglevel = 0)
{
    FLAGS_timestamp_in_logfile_name = false;
    FLAGS_colorlogtostderr = true;
    
    FLAGS_log_prefix = false;
    FLAGS_max_log_size = 512;

    FLAGS_alsologtostderr = alsologtostderr;



    FLAGS_minloglevel = minloglevel;
    
    if (FLAGS_alsologtostderr)
    {
        google::SetStderrLogging(google::GLOG_INFO);
    }
    FLAGS_logbufsecs = 0;
}


    void  InitializeLogEngine(char** argv)
    {
#ifndef _MSC_VER  
        google::InstallFailureSignalHandler();
#endif
	TearDownMoldaiUnifiedSink();
	if (google::IsGoogleLoggingInitialized()) {
		google::ShutdownGoogleLogging();
	}
        std::string workPath = GetWorkPath();
        std::string logdir = workPath + "/logs";

        AI3D::CORE::File::CreateDirIfNotExists(logdir);
        std::string log_dir = logdir + "/logengine";
	AI3D::CORE::File::CreateDirIfNotExists(log_dir);
	SetGlogDirFromPath(log_dir); 
        google::InitGoogleLogging("moldai");
        FLAGS_timestamp_in_logfile_name = false;       
        
        FLAGS_log_prefix = false;
        FLAGS_alsologtostderr = false;
        FLAGS_minloglevel = 0;
        FLAGS_max_log_size = 512;
        if (FLAGS_alsologtostderr)
        {
            FLAGS_colorlogtostderr = true;
            google::SetStderrLogging(google::GLOG_INFO);
        }
        FLAGS_logbufsecs = 0;
	(void)argv;
	DisableDefaultGlogFiles();
	SetupMoldaiUnifiedSink(log_dir);

	LOG(INFO) << "glog log_dir=" << log_dir;
	google::FlushLogFiles(google::GLOG_INFO);
       
    }

       void InitializeLog(char** argv,bool alsotostd,int loglevel)
        {
#ifndef _MSC_VER  
            google::InstallFailureSignalHandler();
#endif
	TearDownMoldaiUnifiedSink();
	if (google::IsGoogleLoggingInitialized()) {
		google::ShutdownGoogleLogging();
	}
            std::string workPath = GetWorkPath();
            std::string logdir = workPath + "/logs";
            
            AI3D::CORE::File::CreateDirIfNotExists(logdir);
            std::string log_dir = logdir + "/log";
	    AI3D::CORE::File::CreateDirIfNotExists(log_dir);
	SetGlogDirFromPath(log_dir);
            google::InitGoogleLogging("moldai");
            SetLogStyle(alsotostd, loglevel);
	DisableDefaultGlogFiles();
	SetupMoldaiUnifiedSink(log_dir);
	(void)argv;

	LOG(INFO) << "glog log_dir=" << log_dir;
	google::FlushLogFiles(google::GLOG_INFO);
        }


      void ResetLog(std::string log_parent_dir,std::string logprefix, bool alsotostd , int loglevel )
       {
	 TearDownMoldaiUnifiedSink();
         if (google::IsGoogleLoggingInitialized())
          {
              google::ShutdownGoogleLogging();
          }
	  if (log_parent_dir.empty()) {
		  log_parent_dir = GetWorkPath();
	  }
	  
	  if (logprefix.find_first_of("/\\") != std::string::npos) {
		  logprefix = AI3D::CORE::File::GetPathBaseName(
		      AI3D::CORE::File::EnsureUnifySlash(logprefix));
	  }
	  if (logprefix.empty()) {
		  logprefix = "log";
	  }
	  
	  std::filesystem::path log_dir_p = AI3D::CORE::File::BoostPathFromUtf8(log_parent_dir) / "logs" / AI3D::CORE::File::BoostPathFromUtf8(logprefix);
	  log_dir_p = log_dir_p.lexically_normal();
	  std::error_code mk_ec;
	  std::filesystem::create_directories(log_dir_p, mk_ec);
	  (void)mk_ec;
	  const std::string log_dir = AI3D::CORE::File::BoostPathToUtf8String(log_dir_p.make_preferred());
	  SetGlogDirFromPath(log_dir);
          google::InitGoogleLogging("moldai");
          FLAGS_logbufsecs = 0;
          SetLogStyle(alsotostd,loglevel);
	  DisableDefaultGlogFiles();
	  SetupMoldaiUnifiedSink(log_dir);

	LOG(INFO) << "glog log_dir=" << log_dir;
	google::FlushLogFiles(google::GLOG_INFO);
       }

 

      bool CheckOptionImpl(const char* file, const char* func, const int line, const bool result,
          const char* op_str)
      {
          if (result)
          {
              return true;
          }
          else
          {

              LOG(ERROR) <<
                  AI3D::CORE::String::StringPrintf("%s:%s:%d,Check Failed: %s ",
                      file, func, line, op_str);
              return false;
          }
      }

  

