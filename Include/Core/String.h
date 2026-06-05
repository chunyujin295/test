#ifndef _AI3D_CORE_STRING_H_
#define _AI3D_CORE_STRING_H_

#include <string>
#include <boost/locale.hpp>
#include <vector>
#include "Constants.h"

namespace AI3D
{
    namespace CORE
    {


        class AI3D_API String
        {
        public:

            static std::string LocaleToUtf8(const std::string& locale_string)
            {
                boost::locale::generator g;
                g.locale_cache_enabled(true);
                const std::locale loc = g(boost::locale::util::get_system_locale());
                return boost::locale::conv::to_utf<char>(locale_string, loc);
            }
            static std::string Utf8ToLocale(const std::string& utf8Str)
            {
                boost::locale::generator g;
                g.locale_cache_enabled(true);
                const std::locale loc = g(boost::locale::util::get_system_locale());
                return boost::locale::conv::from_utf<char>(utf8Str, loc);
            }

            static bool ReadFileToString(const std::string& path, std::string& strs);
            

            static bool SaveFileFromString(const std::string& path, const std::string& strs) ;
         
            
            
            static void MakeDuplicatedName(std::vector<std::string> strs,std::string& rawstr);
           

            
            
            
            
            static std::string StringPrintf(const char* format, ...);

            
            static std::string StringReplace(const std::string& str, const std::string& old_str,
                const std::string& new_str);

            
            static std::vector<std::string> StringSplit(const std::string& str,
                const std::string& delim);
            static std::string ToSHA256(std::string imagepath);
            
            static bool StringStartsWith(const std::string& str, const std::string& prefix);
            
            static  std::string StringTrim(const std::string& str, const std::string& strTrim = " ");
            
            
            static void StringRemove(std::string& path, std::string str);
            
            static void StringRemoveALL(std::string& path, const std::string& str,bool is_All = true);
            
            static void StringTrim(std::string* str);
            
            static void StringLeftTrim(std::string* str);
            static void StringRightTrim(std::string* str);

            static void StringRightTrim(std::string& str,std::string strTrim);

            
            static void StringToLower(std::string* str);
            static void StringToUpper(std::string* str);

            
            static bool StringContains(const std::string& str, const std::string& sub_str);

            static bool StringIsNullOrBlank(const std::string& str);
        };
    }
} 

#endif  
