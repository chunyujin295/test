#include "Core/File.h"

#include <filesystem>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <share.h>
#endif
#include <cstdio>
#include <cstdarg>
#include <iomanip> 
#include <boost/algorithm/string.hpp>

namespace AI3D
{
    namespace CORE
    {

        std::filesystem::path File::BoostPathFromUtf8(const std::string& utf8Path)
        {
            return std::filesystem::u8path(utf8Path);
        }

        std::string File::BoostPathToUtf8String(const std::filesystem::path& path)
        {
#if defined(_WIN32)
            const std::wstring w = path.wstring();
            if (w.empty()) {
                return {};
            }
            const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
            if (n <= 0) {
                return {};
            }
            std::string out(static_cast<size_t>(n), '\0');
            WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()), &out[0], n, nullptr, nullptr);
            return out;
#else
            return path.generic_string();
#endif
        }

        std::string File::EnsureUnitPath(const std::string& path)
        {
            if (path.find(" ") == std::string::npos)
            {
                return path;
            }
            return std::string("\""+path+ "\"");
        }

		 std::string  File::ToStringWithHighPrecision(double val, int precision)
        {
            std::ostringstream os;
            os << std::setiosflags(std::ios::fixed) << std::setprecision(precision) << val;
            std::string str = os.str();
            String::StringRightTrim(str, "0");
            return str;
        }

         

        std::string File::EnsureUnifySlash(const std::string& path)
        {
            std::string temp = path;
            size_t start = 0;
            while ((start = temp.find(REVERSE_PATH_SEPARATOR, start)) != std::string::npos)
                temp[start] = PATH_SEPARATOR;
			return TrimUnifySlash(temp);
        }

        bool File::CheckNetPathExists(const std::string& path)
        {
            std::string temp = path;
            
            
            bool bGotLetterOrDigit = false;
            int iPeriodPointCount = 0;
            int i = 0;

            for (; i < temp.size(); i++)
            {
                if (temp.at(i) == PATH_SEPARATOR || temp.at(i) == REVERSE_PATH_SEPARATOR || temp.at(i) == '.')
                    ;
                else
                    break;
            }

            if (i >= temp.size())
                return false;

            

            for (; i < temp.size(); i++)
            {
                if (isdigit(temp.at(i)) || isalpha(temp.at(i)))
                {
                    bGotLetterOrDigit = true;
                }
                else if (bGotLetterOrDigit && (temp.at(i) == PATH_SEPARATOR || temp.at(i) == REVERSE_PATH_SEPARATOR))
                    break;
                else if (bGotLetterOrDigit && temp.at(i) == '.')
                    iPeriodPointCount++;
                else
                    ;
            }

            if (bGotLetterOrDigit && iPeriodPointCount >= 1)
                return true;

            return false;
        }

        std::string File::TrimUnifySlash(const std::string& path)
        {
            
            std::string temp = path;
            size_t start = 2;
            while ((start = temp.find(PATH_SEPARATOR, start)) != std::string::npos)
                if (temp[start - 1] == PATH_SEPARATOR)
                    temp.erase(start, 1);
                else
                    ++start;
            return temp;
        }

        bool File::IsFileExistent(const std::string& filename)
        {
            try
            {
                
                const std::filesystem::path path = File::BoostPathFromUtf8(filename);
                std::error_code error;
                auto file_status = std::filesystem::status(path, error);
                
                if (error)
                {
                    
                    
                    
                    return false;
                }

                if (!std::filesystem::exists(file_status))
                {
                    LOGI(filename, __FUNCTION__, __LINE__);
                    return false;
                }

                if (std::filesystem::is_directory(file_status))
                {
                    LOGI(filename, __FUNCTION__, __LINE__);
                    return false;
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return true;
        }

        bool File::IsFileExistent_1(const std::string& filename)
        {
            try
            {
                const std::filesystem::path path = File::BoostPathFromUtf8(filename);
                std::error_code error;
                auto file_status = std::filesystem::status(path, error);
                if (error) {
                    return false;
                }

                if (!std::filesystem::exists(file_status)) {
                    return false;
                }

                if (std::filesystem::is_directory(file_status)) {
                    return false;
                }
                
                
                
                
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return true;
        }
        std::string File::AutoGeneratedFullFilePath(std::string currFilePath, std::string currFileName,bool& isnew)
        {
            std::string fullFilePath = currFilePath + "/" + currFileName;


            std::string strfile = (fullFilePath);
            strfile = AI3D::CORE::File::EnsureUnifySlash(strfile);

            auto names = File::GetDirList(currFilePath);
            std::vector<std::string> dirnames ;
            for (auto iter : names)
            {
                dirnames.push_back(File::GetDirName(iter));
            }
            bool bFoundNotExistingOne = false;
            
          
            std::string newname = currFileName;
            String::MakeDuplicatedName(dirnames, newname);
            

            std::string testBaseFilename = currFilePath + "/" + newname;
            fullFilePath = testBaseFilename;
            if (0)
            {
                
                if (ExistsPath(fullFilePath) && ExistsDir(fullFilePath))
                {
                    int number = 2;
                    do
                    {
                        
                        String::MakeDuplicatedName(dirnames, newname);
                        

                        std::string testBaseFilename = currFilePath + "/" + newname;
                        if (!ExistsPath(testBaseFilename) && !ExistsDir(testBaseFilename))
                        {
                            bFoundNotExistingOne = true;
                            fullFilePath = testBaseFilename;

                            isnew = true;
                            break;
                        }
                        number++;
                    } while (true);
                }
                else
                {
                    bFoundNotExistingOne = true;
                }
            }
            return fullFilePath;
        }

        void File::MakeDirEmpty(const std::string& path)
        {
            if (File::ExistsPath(path) && File::ExistsDir(path))
            {

              std::vector<std::string> dirs =   GetRecursiveDirList(path);
              for (auto& dir : dirs)
              {
                  Remove(dir);
              }
              std::vector<std::string> files = GetFileList(path);
              RemoveFiles(files);
            }

        }
        bool  File::RemoveFile(const std::string& file)
        {
            try
            {
                
                
                if (File::ExistsPath(file) && File::ExistsFile(file))
                    {
                        bool result = std::filesystem::remove(File::BoostPathFromUtf8(file));
                        return result;
                    }
                
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return true;
        }
        bool  File::RemoveFiles(const std::vector<std::string>& files)
        {
            try
            {
                for (auto iter : files)
                {
                    if (File::ExistsPath(iter) && File::ExistsFile(iter))
                    {
                        std::filesystem::remove(File::BoostPathFromUtf8(iter));
                    }
                }
            }
            catch (std::filesystem::filesystem_error &fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception &ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }
            
            return true;
        }

		bool File::Remove(const std::string& path)
        {
            std::string path_temp = path;
            path_temp = EnsureUnifySlash(path_temp);

			try
			{
                std::filesystem::path block_dir_path = File::BoostPathFromUtf8(path_temp);
                std::filesystem::remove_all(block_dir_path);
			}
            catch (std::filesystem::filesystem_error &fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
			catch (const std::exception& ex)
			{
				
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str()); ;
				return false;
			}

            return true;
        }

        std::string File::GetDirName(const std::string& path, bool isDir)
        {
            std::string temp = path;
            if (!isDir)
            {
                temp = GetParentDir(temp);
            }
            temp = EnsureUnifySlash(temp);
            
            if (temp.back() == PATH_SEPARATOR)
            {
                temp.erase(temp.length()-1,1);
            }
            
            std::string dir_name = temp.substr(temp.find_last_of(PATH_SEPARATOR) +1);
            return dir_name;
        }						  
        std::string File::EnsureTrailingSlash(const std::string& str)
        {
            if (str.length() > 0) 
            {
                if (str.back() != '/') {
                    return str + "/";
                }
            }
            else {
                return str + "/";
            }
            return str;
        }

        bool File::HasFileExtension(const std::string& file_name, const std::string& ext) 
        {
            if (!CHECK_OPTION(!ext.empty()))
                return false;
            if (!CHECK_OPTION_EQ(ext.at(0), '.'))
                return false;
            std::string ext_lower = ext;
            String::StringToLower(&ext_lower);
            if (file_name.size() >= ext_lower.size() &&
                file_name.substr(file_name.size() - ext_lower.size(), ext_lower.size()) ==
                ext_lower) {
                return true;
            }
            return false;
        }

        void File::SplitFileExtension(const std::string& path, std::string* root,
            std::string* ext) {
            const auto parts = String::StringSplit(path, ".");
            CHECK_OPTION_GT(parts.size(), 0);
            if (parts.size() == 1) {
                *root = parts[0];
                *ext = "";
            }
            else {
                *root = "";
                for (size_t i = 0; i < parts.size() - 1; ++i) {
                    *root += parts[i] + ".";
                }
                *root = root->substr(0, root->length() - 1);
                if (parts.back() == "") {
                    *ext = "";
                }
                else {
                    *ext = "." + parts.back();
                }
            }
        }

        bool File::ExistsFile(const std::string& path) 
        {
            try
            {
                const std::filesystem::path bp = File::BoostPathFromUtf8(path);
                if (std::filesystem::exists(bp))
                    return std::filesystem::is_regular_file(bp);
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception &ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return false;
        }


        bool File::ExistsDir(const std::string& path) 
        {
            try
            {
                return std::filesystem::is_directory(File::BoostPathFromUtf8(path));
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception &ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }
        }

        void File::FindDeepsestDir(const std::string& path, std::vector<std::string>& lastdirfiles )
        {
            std::string _path = path;
            std::string dir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(_path));
            std::vector<std::string> files = AI3D::CORE::File::GetDirList(dir);
            if (files.empty())
            {
                lastdirfiles.push_back(dir);
                return;
            }
            else
            {
                for (int i = 0; i < files.size(); i++)
                {
                    FindDeepsestDir(files[i], lastdirfiles);
                }
            }
        }

        bool File::ExistsPath(const std::string& path) {
            try
            {
                return std::filesystem::exists(File::BoostPathFromUtf8(path));
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }
        }

        void File::GetLastSecondDir(const std::string& path, std::string& parentdir, std::string& lastdir, bool isDir)
        {
            std::string _path = path;
            if (!isDir)
            {
                _path = GetParentDir(_path);
            }
            _path = AI3D::CORE::File::EnsureTrailingSlash(EnsureUnifySlash(_path));
            String::StringRightTrim(_path, "/");
            size_t lastpos = _path.find_last_of(PATH_SEPARATOR);
            lastdir = _path.substr(lastpos + 1);
            parentdir = _path.substr(0, lastpos)  ;
            parentdir = AI3D::CORE::File::EnsureTrailingSlash(EnsureUnifySlash(parentdir));
        }

        bool File::IsNetFile(const std::string& path)
        {
            std::string pathtemp = path;
            pathtemp = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(pathtemp));

            bool bFoundNetPrefix = false;

            size_t iPointCount = std::count(pathtemp.begin(), pathtemp.end(), '.');
            size_t iSlashCount = std::count(pathtemp.begin(), pathtemp.end(), '/');

            if ((iSlashCount + iPointCount) == pathtemp.size())
                return bFoundNetPrefix;

            size_t iFirstPointPos = pathtemp.find(".", 0);
            size_t iLastSlashPos = pathtemp.find_last_of("/");

            if (iLastSlashPos != std::string::npos && iFirstPointPos != std::string::npos &&
                iFirstPointPos < iLastSlashPos)
            {
                bFoundNetPrefix = true;
            }

            return bFoundNetPrefix;
        }

        void File::CreateDirIfNotExists(const std::string& path,bool bdeepcreate)
        {
            
            if(0)
            {
                std::ostringstream oss;
                oss << "CreateDir:" << path;
                LOGI(oss.str());
            }

            try
            {

                std::string pathtemp = path;
                
                pathtemp = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(pathtemp));

                

                bool bFoundNetPrefix = false;

                size_t iPointCount = std::count(pathtemp.begin(), pathtemp.end(), '.');
                size_t iSlashCount = std::count(pathtemp.begin(), pathtemp.end(), '/');
                
                if ((iSlashCount + iPointCount) == pathtemp.size()) 
                    return;

                
                size_t iFirstPointPos = pathtemp.find(".", 0);
                size_t iLastSlashPos = pathtemp.find_last_of("/");

                if (iLastSlashPos != std::string::npos && iFirstPointPos != std::string::npos &&
                    iFirstPointPos < iLastSlashPos)
                {
                    bFoundNetPrefix = true;

                    if(!File::CheckNetPathExists(pathtemp))
                    {
                        bFoundNetPrefix = false;
                    }
                }

                if (bFoundNetPrefix)
                {
                    
                    int iLeftMostSlashNum = 0;
                    
                    
                    
                    if(pathtemp.at(0) == '/' && pathtemp.at(1) == '/')
                        ;
                    else if(pathtemp.at(0) == '/')
                        pathtemp = "/" + pathtemp;
                    else 
                        pathtemp = "//" + pathtemp;
                    
                    
                }


                {
                    
                    if (!ExistsDir(pathtemp))
                    {
                        
                        std::filesystem::create_directories(File::BoostPathFromUtf8(pathtemp));
                    }
                    
                }
                
                    
                   
                    
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   
                   


                    
                    
                    
                    

                    
                    
                    
                    
                    
                    
                    
                    
                 
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }
            

        }

        std::string  File::GetFileName(const std::string& path)
        {

            try
            {
                std::filesystem::path p = File::BoostPathFromUtf8(path);
                return File::BoostPathToUtf8String(p.filename());
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return "";
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return "";
            }

        }
        
        std::string  File::GetFileNameWithoutExtension(const std::string& path)
        {
           
            try
            {
                std::filesystem::path p = File::BoostPathFromUtf8(path);
                return File::BoostPathToUtf8String(p.stem());
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return "";
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return "";
            }
        }

        std::string File::GetPathBaseName(const std::string& path) {
            const std::vector<std::string> names =
                String::StringSplit(String::StringReplace(path, "\\", "/"), "/");
            if (names.size() > 1 && names.back() == "") {
                return names[names.size() - 2];
            }
            else {
                return names.back();
            }
        }



         std::string File::EnsureDirectorySlash(const std::string& aFile)
        {
             std::string File = aFile;
            if (File.empty())
            {
                return File;
            }

            size_t nEnd = File.size() - 1;
            if (File[nEnd] != PATH_SEPARATOR)
            {
                File += PATH_SEPARATOR;
            }

            return File;
        }

        std::string File::GetParentDir(const std::string& path) 
        {
            try
            {
                
                std::string path_temp = path;
                path_temp = File::BoostPathToUtf8String(File::BoostPathFromUtf8(path_temp).parent_path());
                path_temp = AI3D::CORE::File::EnsureTrailingSlash(EnsureUnifySlash(path_temp));
                
              
                return (path_temp);
                 
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return "";
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return "";
            }
        }

        std::vector<std::string> File::GetFileList(const std::string& path, std::string mask)
        {
            std::vector<std::string> file_list;
            try
            {
                for (auto it = std::filesystem::directory_iterator(File::BoostPathFromUtf8(path));
                    it != std::filesystem::directory_iterator(); ++it)
                {
                    if (std::filesystem::is_regular_file(it->path()))
                    {
                        const std::filesystem::path file_path = *it;
                        if (mask != "*.*")
                        {
                            std::string root, ext;
                           
                          std::string name=  AI3D::CORE::File::GetFileName(File::BoostPathToUtf8String(file_path));
                            String::StringToLower(&name);
                            String::StringToLower(&mask);
                            if (String::StringContains(name,mask))
                                file_list.push_back(File::BoostPathToUtf8String(file_path));
                        }
                        else
                        {
                            file_list.push_back(File::BoostPathToUtf8String(file_path));
                        }
                    }
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            return file_list;
        }

        std::vector<std::string> File::GetRecursiveFileList(const std::string& path, std::string mask ) 
        {
            std::vector<std::string> file_list;
            try
            {
                for (auto it = std::filesystem::recursive_directory_iterator(File::BoostPathFromUtf8(path));
                    it != std::filesystem::recursive_directory_iterator(); ++it) {
                    if (std::filesystem::is_regular_file(it->path()))
                    {
                        const std::filesystem::path file_path = *it;
                        if (mask != "*.*")
                        {
                            std::string root, ext;
                            std::string name=  AI3D::CORE::File::GetFileName(File::BoostPathToUtf8String(file_path));
                            String::StringToLower(&name);
                            String::StringToLower(&mask);
                            if (String::StringContains(name,mask))
                                file_list.push_back(File::BoostPathToUtf8String(file_path));
                        }
                        else
                        {
                            file_list.push_back(File::BoostPathToUtf8String(file_path));
                        }

                    }
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            return file_list;
        }

        std::vector<std::string> File::GetDirList(const std::string& path) {
            std::vector<std::string> dir_list;
            try
            {
                for (auto it = std::filesystem::directory_iterator(File::BoostPathFromUtf8(path));
                    it != std::filesystem::directory_iterator(); ++it) {
                    if (std::filesystem::is_directory(it->path())) {
                        const std::filesystem::path dir_path = *it;
                        dir_list.push_back(File::BoostPathToUtf8String(dir_path));
                    }
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            return dir_list;
        }

        std::vector<std::string> File::GetRecursiveDirList(const std::string& path) {
            std::vector<std::string> dir_list;
            try
            {
                for (auto it = std::filesystem::recursive_directory_iterator(File::BoostPathFromUtf8(path));
                    it != std::filesystem::recursive_directory_iterator(); ++it) {
                    if (std::filesystem::is_directory(it->path())) {
                        const std::filesystem::path dir_path = *it;
                        dir_list.push_back(File::BoostPathToUtf8String(dir_path));
                    }
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            return dir_list;
        }


        std::string File::GetFileExtension(const std::string& path)
        {
            return File::BoostPathFromUtf8(path).extension().string();
        }

        size_t File::GetFileSize(const std::string& path)
        {
           
            std::ifstream file = OpenIfstreamUtf8(path, std::ifstream::ate | std::ifstream::binary);
            if (!CHECK_OPTION(file.is_open()))
            {
                LOGE("open " +path + "failed.");
            }
            return file.tellg();
        }

        void File::PrintHeading1(const std::string& heading) {
            std::cout << std::endl << std::string(78, '=') << std::endl;
            std::cout << heading << std::endl;
            std::cout << std::string(78, '=') << std::endl << std::endl;
        }

        void File::PrintHeading2(const std::string& heading) {
            std::cout << std::endl << heading << std::endl;
            std::cout << std::string(std::min<int>(heading.size(), 78), '-') << std::endl;
        }

        template <>
        std::vector<std::string> File::CSVToVector(const std::string& csv) {
            auto elems = String::StringSplit(csv, ",;");
            std::vector<std::string> values;
            values.reserve(elems.size());
            for (auto& elem : elems) {
                String::StringTrim(&elem);
                if (elem.empty()) {
                    continue;
                }
                values.push_back(elem);
            }
            return values;
        }

        template <>
        std::vector<int> File::CSVToVector(const std::string& csv) {
            auto elems = String::StringSplit(csv, ",;");
            std::vector<int> values;
            values.reserve(elems.size());
            for (auto& elem : elems) {
                String::StringTrim(&elem);
                if (elem.empty()) {
                    continue;
                }
                try {
                    values.push_back(std::stoi(elem));
                }
                catch (std::exception) {
                    return std::vector<int>(0);
                }
            }
            return values;
        }

        template <>
        std::vector<float> File::CSVToVector(const std::string& csv) {
            auto elems = String::StringSplit(csv, ",;");
            std::vector<float> values;
            values.reserve(elems.size());
            for (auto& elem : elems) {
                String::StringTrim(&elem);
                if (elem.empty()) {
                    continue;
                }
                try {
                    values.push_back(std::stod(elem));
                }
                catch (std::exception) {
                    return std::vector<float>(0);
                }
            }
            return values;
        }

        template <>
        std::vector<double> File::CSVToVector(const std::string& csv) {
            auto elems = String::StringSplit(csv, ",;");
            std::vector<double> values;
            values.reserve(elems.size());
            for (auto& elem : elems) {
                String::StringTrim(&elem);
                if (elem.empty()) {
                    continue;
                }
                try {
                    values.push_back(std::stold(elem));
                }
                catch (std::exception) {
                    return std::vector<double>(0);
                }
            }
            return values;
        }

        std::vector<std::string> File::ReadTextFileLines(const std::string& path) {
            std::ifstream file = OpenIfstreamUtf8(path, std::ios::in);
            if (!CHECK_OPTION(file.is_open()))
            {
                LOGE("open " + path + "failed.");
            }

            std::string line;
            std::vector<std::string> lines;
            while (std::getline(file, line)) {
                String::StringTrim(&line);

                if (line.empty()) {
                    continue;
                }

                lines.push_back(line);
            }

            return lines;
        }
        bool File::CopySingleFile(std::string file, std::string outfile)
        {
            try
            {
                std::error_code ec;
                std::filesystem::copy_file(File::BoostPathFromUtf8(file), File::BoostPathFromUtf8(outfile), std::filesystem::copy_options::overwrite_existing, ec);
                

                if (ec)
                {
                    return false;
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return true;
        }

        bool File::CopyFiles(std::vector<std::string> files,std::string out, bool bremovefile)
        {
            try
            {
                std::error_code ec;
                if (bremovefile)
                {
                    for (std::vector<std::string>::iterator it = files.begin();
                        it != files.end();)
                    {
                        out = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(out));;
                        if (ExistsPath(*it))
                        {
                            std::string outfile = out + GetPathBaseName(*it);

                            std::filesystem::copy_file(File::BoostPathFromUtf8(*it), File::BoostPathFromUtf8(outfile), std::filesystem::copy_options::overwrite_existing, ec);
                            std::filesystem::remove(File::BoostPathFromUtf8(*it));
                            files.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }
                else
                {
                    for (std::vector<std::string>::iterator it = files.begin();
                        it != files.end(); it++)
                    {
                        out = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(out));;
                        if (ExistsPath(*it))
                        {
                            std::string outfile = out + GetPathBaseName(*it);

                            std::filesystem::copy_file(File::BoostPathFromUtf8(*it), File::BoostPathFromUtf8(outfile), std::filesystem::copy_options::overwrite_existing, ec);

                        }

                    }
                }

                if (ec)
                {
                    return false;
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return true;
        }

        void File::BackupFile(std::string srcfile, std::string extensionadded)
        {
            try
            {
                std::string  tagfile = srcfile + extensionadded;
                std::filesystem::rename(File::BoostPathFromUtf8(srcfile), File::BoostPathFromUtf8(tagfile));
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }
        }

        bool File::CopyDirectory(const std::string& strSourceDir, const std::string& strDestDir,bool iscopyblk, std::string extension)
        {
            try
            {
                std::filesystem::recursive_directory_iterator end; 
                std::error_code ec;
                for (std::filesystem::recursive_directory_iterator pos(File::BoostPathFromUtf8(strSourceDir)); pos != end; ++pos)
                {
                    
                    if (std::filesystem::is_directory(pos->path()))
                        continue;
                    if (!iscopyblk)
                    {
                        if (pos->path().extension() == std::filesystem::path(BLOCKFILE) || pos->path().extension() == std::filesystem::path(BLOCKBINFILE))
                            continue;
                    }
                    std::string strAppPath = File::BoostPathToUtf8String(pos->path());
                    std::string strRestorePath;
                    
                    boost::replace_first_copy(std::back_inserter(strRestorePath), strAppPath, strSourceDir, strDestDir);
                    const std::filesystem::path dstPath = File::BoostPathFromUtf8(strRestorePath);
                    if (!std::filesystem::exists(dstPath.parent_path()))
                    {
                        std::filesystem::create_directories(dstPath.parent_path(), ec);
                    }
                    std::filesystem::copy_file(File::BoostPathFromUtf8(strAppPath), dstPath, std::filesystem::copy_options::overwrite_existing, ec);
                }
                if (ec)
                {
                    return false;
                }
            }
            catch (std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

            return true;
        }

        bool File::ReadBinaryFileUtf8(const std::string& utf8Path, std::vector<unsigned char>& out)
        {
            out.clear();
            std::ifstream ifs = OpenIfstreamUtf8(utf8Path, std::ios::binary | std::ios::ate);
            if (!ifs) {
                return false;
            }
            const std::streamsize sz = ifs.tellg();
            if (sz <= 0) {
                return false;
            }
            out.resize(static_cast<size_t>(sz));
            ifs.seekg(0, std::ios::beg);
            if (!ifs.read(reinterpret_cast<char*>(out.data()), sz)) {
                out.clear();
                return false;
            }
            return true;
        }

        std::ifstream File::OpenIfstreamUtf8(const std::string& utf8Path, std::ios::openmode mode)
        {
            const std::filesystem::path p = std::filesystem::u8path(utf8Path);
            return std::ifstream(p, mode);
        }

        std::ofstream File::OpenOfstreamUtf8(const std::string& utf8Path, std::ios::openmode mode)
        {
            const std::filesystem::path p = std::filesystem::u8path(utf8Path);
            return std::ofstream(p, mode);
        }

        FILE* File::FopenDenyWriteLockUtf8(const std::string& utf8LockPath)
        {
#if defined(_WIN32) && (defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__))
            try {
                const std::wstring w = std::filesystem::u8path(utf8LockPath).wstring();
                return _wfsopen(w.c_str(), L"wt", _SH_DENYWR);
            }
            catch (const std::filesystem::filesystem_error&) {
                return nullptr;
            }
            catch (const std::bad_alloc&) {
                return nullptr;
            }
#else
            return std::fopen(utf8LockPath.c_str(), "wt");
#endif
        }
        
    }
} 
