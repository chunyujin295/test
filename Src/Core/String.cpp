#include "Core/String.h"
#include "Core/ReturnCode.h"
#include "Core/File.h"
#include <algorithm>
#include <cstdarg>
#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <openssl/sha.h>

#include <string_view>

namespace AI3D
{
    namespace CORE
    {
        namespace 
        {

            
           

            void StringAppendV(std::string* dst, const char* format, va_list ap) {
                
                static const int kFixedBufferSize = 1024;
                char fixed_buffer[kFixedBufferSize];

                
                
                
                va_list backup_ap;
                va_copy(backup_ap, ap);
                int result = vsnprintf(fixed_buffer, kFixedBufferSize, format, backup_ap);
                va_end(backup_ap);

                if (result < kFixedBufferSize) 
                {
                    if (result >= 0)
                    {
                        
                        dst->append(fixed_buffer, result);
                        return;
                    }

#ifdef _MSC_VER
                    
                    
                    va_copy(backup_ap, ap);
                    result = vsnprintf(nullptr, 0, format, backup_ap);
                    va_end(backup_ap);
#endif

                    if (result < 0) 
                    {
                        
                        return;
                    }
                }

                
                
                const int variable_buffer_size = result + 1;
                std::unique_ptr<char> variable_buffer(new char[variable_buffer_size]);

                
                va_copy(backup_ap, ap);
                result =
                    vsnprintf(variable_buffer.get(), variable_buffer_size, format, backup_ap);
                va_end(backup_ap);

                if (result >= 0 && result < variable_buffer_size) 
                {
                    dst->append(variable_buffer.get(), result);
                }
            }

            bool IsNotWhiteSpace(const int character) 
            {
                return character != ' ' && character != '\n' && character != '\r' &&
                    character != '\t';
            }


           
        }  
       
        void String::MakeDuplicatedName(std::vector<std::string> strs, std::string& rawstr)
        {
            
            bool bFoundNotExistingOne = false;
            std::string name = rawstr;
            int count = std::count(strs.begin(), strs.end(), name);;
            if (count >= 1)
            {
                do
                {
                    count++;
                    
                    name = rawstr + "_" + std::to_string(count) + "";;

                    std::string testBaseFilename = name;
                    int count = std::count(strs.begin(), strs.end(), testBaseFilename);;
                    if (count < 1)
                    
                    {
                        bFoundNotExistingOne = true;
                        name = testBaseFilename;


                        break;
                    }
                    
                } while (true);
            }
            else
            {
                bFoundNotExistingOne = true;
            }
            rawstr = name;
               
        }
        
        bool String::ReadFileToString(const std::string& path, std::string& strs)
        {

            std::ifstream in = File::OpenIfstreamUtf8(path, std::ios::in);
            if (!in.is_open())
            {
                return false;
            }

            std::string line;

            while (std::getline(in, line)) {

                if (line[line.size() - 1] != '\n')
                    line.append("\n");

                strs.append(line);
            }
            in.close();

            return true;
        }

        bool String::SaveFileFromString(const std::string& path, const std::string& strs) 
        {

            std::ofstream fileout = File::OpenOfstreamUtf8(path, std::ios::out);
            if (!fileout.good())
                return false;

            fileout << strs;
            fileout.close();

            return true;
        }


        std::string String::StringPrintf(const char* format, ...)
        {
            va_list ap;
            va_start(ap, format);
            std::string result;
            StringAppendV(&result, format, ap);
            va_end(ap);
            return result;
        }

        std::string String::StringReplace(const std::string& str, const std::string& old_str,
            const std::string& new_str)
        {
            if (old_str.empty()) 
            {
                return str;
            }
            size_t position = 0;
            std::string mod_str = str;
            while ((position = mod_str.find(old_str, position)) != std::string::npos)
            {
                mod_str.replace(position, old_str.size(), new_str);
                position += new_str.size();
            }
            return mod_str;
        }

        std::vector<std::string> String::StringSplit(const std::string& str,
            const std::string& delim)
        {
            std::vector<std::string> elems;
            boost::split(elems, str, boost::is_any_of(delim), boost::token_compress_on);
            return elems;
        }
        void String::StringRemove(std::string& path, std::string str)
        {

            if (path.find(str) != std::string::npos)
            {
                int pos = path.find(str);
                int n = str.size();
                path = path.erase(pos, n);
            }
           
        }
        void String::StringRemoveALL(std::string& path, const std::string& str, bool is_All)
        {
			if (path.empty() || str.empty())
            {
                return;
            }
            size_t pos = 0;
            while ((pos = path.find(str, pos)) != std::string::npos)
            {
                if (!is_All)
                {
                    
					if (pos == 0 || (pos + str.size()) == path.size())
                    {
                        path.erase(pos, str.size());
                    }
                    else
                    {
                        pos++;
                    }
                }
                else
                {
                    path.erase(pos, str.size());
                }
            }

            StringTrim(&path);
        }
        
        bool String::StringStartsWith(const std::string& str, const std::string& prefix)
        {
            return !prefix.empty() && prefix.size() <= str.size() &&
                str.substr(0, prefix.size()) == prefix;
        }

        void String::StringLeftTrim(std::string* str)
        {
            str->erase(str->begin(),
                std::find_if(str->begin(), str->end(), IsNotWhiteSpace));
        }

        void String::StringRightTrim(std::string* str)
        {
            str->erase(std::find_if(str->rbegin(), str->rend(), IsNotWhiteSpace).base(),
                str->end());
        }

        void  String::StringRightTrim(std::string& str, std::string strTrim)
        {
            if (str.empty())
                return;
           
            while (str.substr(str.size() - strTrim.size(), strTrim.size()) == strTrim)
                str.erase(str.size() - strTrim.size(), strTrim.size());
        }

        void String::StringTrim(std::string* str)
        {
            StringLeftTrim(str);
            StringRightTrim(str);
        }
        std::string String::StringTrim(const std::string& str, const std::string& strTrim)
        {
            std::string result = str;
            if (result.empty())
                return result;
            while (result.substr(0, strTrim.size()) == strTrim)
                result.erase(0, strTrim.size());
            while (result.substr(result.size() - strTrim.size(), strTrim.size()) == strTrim)
                result.erase(result.size() - strTrim.size(), strTrim.size());
            return result;
        }
        void String::StringToLower(std::string* str)
        {
            std::transform(str->begin(), str->end(), str->begin(), ::tolower);
        }

        void String::StringToUpper(std::string* str)
        {
            std::transform(str->begin(), str->end(), str->begin(), ::toupper);
        }

        bool String::StringContains(const std::string& str, const std::string& sub_str)
        {
            return str.find(sub_str) != std::string::npos;
        }
       std::string String::ToSHA256(const std::string str)
        {
            char buf[2];
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256,str.c_str(),str.size());
            SHA256_Final(hash,&sha256);
            std::string newstring = "";
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            {
                sprintf(buf, "%02x", hash[i]);
                newstring = newstring + buf;
            }
            return newstring;
        }

        bool String::StringIsNullOrBlank(const std::string &str)
        {
            return std::find_if(str.begin(), str.end(), [](unsigned char ch)
            {
                // return !std::isspace(ch);
                return ch != ' ' &&
                       ch != '\t' &&
                       ch != '\n' &&
                       ch != '\r' &&
                       ch != '\f' &&
                       ch != '\v';
            }) == str.end();
        }
    }
} 
