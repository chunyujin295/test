




#ifndef _AI3D_CORE_LOGGING_H_
#define _AI3D_CORE_LOGGING_H_

#include <iostream>

#include <glog/logging.h>

#include "Core/String.h"





#define CHECK_OPTION_IMPL(condition) \
CheckOptionImpl(__FILE__,__FUNCTION__, __LINE__,condition,#condition)


#define CHECK_OPTION_OP(name, op, val1, val2)                              \
 __CheckOptionOpImpl(__FILE__,__FUNCTION__, __LINE__, (val1 op val2), val1, val2, \
                           #val1, #val2, #op)



#define CHECK_OPTION_EQ(val1, val2) CHECK_OPTION_OP(_EQ, ==, val1, val2)
#define CHECK_OPTION_NE(val1, val2) CHECK_OPTION_OP(_NE, !=, val1, val2)
#define CHECK_OPTION_LE(val1, val2) CHECK_OPTION_OP(_LE, <=, val1, val2)
#define CHECK_OPTION_LT(val1, val2) CHECK_OPTION_OP(_LT, <, val1, val2)
#define CHECK_OPTION_GE(val1, val2) CHECK_OPTION_OP(_GE, >=, val1, val2)
#define CHECK_OPTION_GT(val1, val2) CHECK_OPTION_OP(_GT, >, val1, val2)

template <typename T1, typename T2>
void CheckOpImpl(const char* file,const char* func,const int line,const bool result,
    const T1 & val1, const T2 & val2, const char* op_str)
{
    if (result)
    {
        return;
    }
    else
    {
        
       
        LOG(ERROR) <<
            AI3D::CORE::String::StringPrintf("%s:%s:%d,Check Failed:  %d %s %d", 
                file,func,line, val1, op_str,val2);
        return;
    }
}



#define CHECK_OPTION(condition) CheckOptionImpl(__FILE__,__FUNCTION__, __LINE__,condition,#condition)

#define CHECK_OPTION_NOTNULL(val)  CheckOptionNotNull(__FILE__,__FUNCTION__, __LINE__,"'" #val "' Must be non NULL", (val))


#define CHECK_LOG_EQ(val1, val2) CHECK_LOG_OP(_EQ, ==, val1, val2)
#define CHECK_LOG_NE(val1, val2) CHECK_LOG_OP(_NE, !=, val1, val2)
#define CHECK_LOG_LE(val1, val2) CHECK_LOG_OP(_LE, <=, val1, val2)
#define CHECK_LOG_LT(val1, val2) CHECK_LOG_OP(_LT, <, val1, val2)
#define CHECK_LOG_GE(val1, val2) CHECK_LOG_OP(_GE, >=, val1, val2)
#define CHECK_LOG_GT(val1, val2) CHECK_LOG_OP(_GT, >, val1, val2)






























enum LOGLEVEL
{
    LOG_VERBOSE = 2,
    LOG_DEBUG = 1,
    LOG_INFO = 0,
    LOG_WARN = -1,
    LOG_ERROR = -2,
    LOG_FATAL = -3,
};

AI3D_API std::string BuildLogPrefix(int level);

#define LogFile(message, level) \
    do { \
        const int _ll = static_cast<int>(level); \
        if (_ll == LOG_ERROR || _ll == LOG_FATAL) { \
            LOG(ERROR) << (message); \
        } else if (_ll == LOG_WARN) { \
            LOG(WARNING) << (message); \
        } else { \
            LOG(INFO) << (message); \
        } \
    } while (0)

#define LOGW(message) LogFile(message, LOG_WARN)
#define LOGE(message) LogFile(message, LOG_ERROR)
#define LOGI(message) LogFile(message, LOG_INFO)
#define LOGD(message) LogFile(message, LOG_DEBUG)
#define LOGV(message) LogFile(message, LOG_VERBOSE)
#define LOGF(message) LogFile(message, LOG_FATAL)





        
        extern "C" void AI3D_API InitializeLog(char** argv, bool alsotostd = true, int loglevel = 0);
        extern "C" void AI3D_API InitializeLogEngine(char** argv);
        extern "C" void AI3D_API ShutDownLog();
        
        extern "C" void AI3D_API SetLogStyle();
        extern "C" void AI3D_API ResetLog(std::string log_parent_dir, std::string logprefix = "log", bool alsotostd = true, int loglevel = 0);
        extern "C" bool AI3D_API CheckOptionImpl(const char* file, const char* func, const int line, const bool result,
            const char* op_str);

       
        template <typename T>
        T CheckOptionNotNull(const char* file, const char* func, int line, const char* names, T&& t)
        {

            if (t == nullptr)
            {
                LOG(ERROR) << (AI3D::CORE::String::StringPrintf("%s ", new std::string(names)));

            }
            return std::forward<T>(t);
        }

        template <typename T1, typename T2>
        bool __CheckOptionOpImpl(const char* file, const char* func, const int line, const bool result,
            const T1& val1, const T2& val2, const char* val1_str,
            const char* val2_str, const char* op_str) 
        {
            if (result) 
            {
                return true;
            }
            else 
            {

                LOG(ERROR)<<(AI3D::CORE::String::StringPrintf("%s:%s:%d,Check Failed:  %d %s %d",
                    file,func,line,val1,op_str, val2));            
                return false;
            }
        }



#define CHECK_LOG(condition) if(!condition) LOG(ERROR)<<\
AI3D::CORE::String::StringPrintf("Function=%s,Check Failed:  %s" ,\
__FUNCTION__,#condition) ;

#define CHECK_LOG_OP(name, op, val1, val2)   \
LOG_IF(ERROR, GOOGLE_PREDICT_BRANCH_NOT_TAKEN(!(val1 op val2))) \
             << "Check failed: " #val1 #op #val2 " "   



extern "C" __declspec(dllexport) void LogConsole(std::string msg);
extern "C" __declspec(dllexport) const char* LogPerifix(int level);

#endif  
