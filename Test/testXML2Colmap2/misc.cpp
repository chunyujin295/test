// Copyright (c) 2018, ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: Johannes L. Schoenberger (jsch-at-demuc-dot-de)

#include "misc.h"
#include "Core/File.h"

#include <cstdarg>
#include <filesystem>
#include <system_error>

#include <boost/algorithm/string.hpp>

namespace colmap {

    bool IsNotWhiteSpace(const int character) {
        return character != ' ' && character != '\n' && character != '\r' &&
            character != '\t';
    }

    std::vector<std::string> StringSplit(const std::string& str,
        const std::string& delim) {
        std::vector<std::string> elems;
        boost::split(elems, str, boost::is_any_of(delim), boost::token_compress_on);
        return elems;
    }

    bool StringStartsWith(const std::string& str, const std::string& prefix) {
        return !prefix.empty() && prefix.size() <= str.size() &&
            str.substr(0, prefix.size()) == prefix;
    }

    void StringLeftTrim(std::string* str) {
        str->erase(str->begin(),
            std::find_if(str->begin(), str->end(), IsNotWhiteSpace));
    }

    void StringRightTrim(std::string* str) {
        str->erase(std::find_if(str->rbegin(), str->rend(), IsNotWhiteSpace).base(),
            str->end());
    }

    void StringTrim(std::string* str) {
        StringLeftTrim(str);
        StringRightTrim(str);
    }

    void StringToLower(std::string* str) {
        std::transform(str->begin(), str->end(), str->begin(), ::tolower);
    }

    void StringToUpper(std::string* str) {
        std::transform(str->begin(), str->end(), str->begin(), ::toupper);
    }



std::string EnsureTrailingSlash(const std::string& str) {
  if (str.length() > 0) {
    if (str.back() != '/') {
      return str + "/";
    }
  } else {
    return str + "/";
  }
  return str;
}

bool HasFileExtension(const std::string& file_name, const std::string& ext) {
 /* CHECK(!ext.empty());
  CHECK_EQ(ext.at(0), '.');*/
  std::string ext_lower = ext;
  StringToLower(&ext_lower);
  if (file_name.size() >= ext_lower.size() &&
      file_name.substr(file_name.size() - ext_lower.size(), ext_lower.size()) ==
          ext_lower) {
    return true;
  }
  return false;
}

void SplitFileExtension(const std::string& path, std::string* root,
                        std::string* ext) {
  const auto parts = StringSplit(path, ".");
  //CHECK_GT(parts.size(), 0);
  if (parts.size() == 1) {
    *root = parts[0];
    *ext = "";
  } else {
    *root = "";
    for (size_t i = 0; i < parts.size() - 1; ++i) {
      *root += parts[i] + ".";
    }
    *root = root->substr(0, root->length() - 1);
    if (parts.back() == "") {
      *ext = "";
    } else {
      *ext = "." + parts.back();
    }
  }
}

void FileCopy(const std::string& src_path, const std::string& dst_path,
              CopyType type) {
  namespace fs = std::filesystem;
  const fs::path src = AI3D::CORE::File::BoostPathFromUtf8(src_path);
  const fs::path dst = AI3D::CORE::File::BoostPathFromUtf8(dst_path);
  switch (type) {
    case CopyType::COPY:
      fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
      break;
    case CopyType::HARD_LINK:
      fs::create_hard_link(src, dst);
      break;
    case CopyType::SOFT_LINK:
      fs::create_symlink(src, dst);
      break;
  }
}

bool ExistsFile(const std::string& path) {
  return std::filesystem::is_regular_file(AI3D::CORE::File::BoostPathFromUtf8(path));
}

bool ExistsDir(const std::string& path) {
  return std::filesystem::is_directory(AI3D::CORE::File::BoostPathFromUtf8(path));
}

bool ExistsPath(const std::string& path) {
  return std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(path));
}

void CreateDirIfNotExists(const std::string& path) {
  if (!ExistsDir(path)) {
    std::error_code ec;
    std::filesystem::create_directory(AI3D::CORE::File::BoostPathFromUtf8(path), ec);
    static_cast<void>(ec);
  }
}

//std::string GetPathBaseName(const std::string& path) {
//  const std::vector<std::string> names =
//      StringSplit(StringReplace(path, "\\", "/"), "/");
//  if (names.size() > 1 && names.back() == "") {
//    return names[names.size() - 2];
//  } else {
//    return names.back();
//  }
//}

std::string GetParentDir(const std::string& path) {
  return AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(path).parent_path());
}

std::string GetRelativePath(const std::string& from, const std::string& to) {
  // This implementation is adapted from:
  // https://stackoverflow.com/questions/10167382
  namespace fs = std::filesystem;

  const fs::path from_path = fs::canonical(AI3D::CORE::File::BoostPathFromUtf8(from));
  const fs::path to_path = fs::canonical(AI3D::CORE::File::BoostPathFromUtf8(to));

  fs::path::const_iterator from_iter = from_path.begin();
  fs::path::const_iterator to_iter = to_path.begin();

  while (from_iter != from_path.end() && to_iter != to_path.end() &&
         (*to_iter) == (*from_iter)) {
    ++to_iter;
    ++from_iter;
  }

  fs::path rel_path;
  while (from_iter != from_path.end()) {
    rel_path /= "..";
    ++from_iter;
  }

  while (to_iter != to_path.end()) {
    rel_path /= *to_iter;
    ++to_iter;
  }

  return AI3D::CORE::File::BoostPathToUtf8String(rel_path);
}

std::vector<std::string> GetFileList(const std::string& path) {
  std::vector<std::string> file_list;
  namespace fs = std::filesystem;
  for (fs::directory_iterator it(AI3D::CORE::File::BoostPathFromUtf8(path)), end; it != end; ++it) {
    const fs::path& file_path = it->path();
    if (fs::is_regular_file(file_path)) {
      file_list.push_back(AI3D::CORE::File::BoostPathToUtf8String(file_path));
    }
  }
  return file_list;
}

std::vector<std::string> GetRecursiveFileList(const std::string& path) {
  std::vector<std::string> file_list;
  namespace fs = std::filesystem;
  for (fs::recursive_directory_iterator it(AI3D::CORE::File::BoostPathFromUtf8(path)), end; it != end; ++it) {
    if (it->is_regular_file()) {
      file_list.push_back(AI3D::CORE::File::BoostPathToUtf8String(it->path()));
    }
  }
  return file_list;
}

std::vector<std::string> GetDirList(const std::string& path) {
  std::vector<std::string> dir_list;
  namespace fs = std::filesystem;
  for (fs::directory_iterator it(AI3D::CORE::File::BoostPathFromUtf8(path)), end; it != end; ++it) {
    if (it->is_directory()) {
      dir_list.push_back(AI3D::CORE::File::BoostPathToUtf8String(it->path()));
    }
  }
  return dir_list;
}

std::vector<std::string> GetRecursiveDirList(const std::string& path) {
  std::vector<std::string> dir_list;
  namespace fs = std::filesystem;
  for (fs::recursive_directory_iterator it(AI3D::CORE::File::BoostPathFromUtf8(path)), end; it != end; ++it) {
    if (it->is_directory()) {
      dir_list.push_back(AI3D::CORE::File::BoostPathToUtf8String(it->path()));
    }
  }
  return dir_list;
}

//size_t GetFileSize(const std::string& path) {
//  std::ifstream file(path, std::ifstream::ate | std::ifstream::binary);
//  CHECK(file.is_open()) << path;
//  return file.tellg();
//}

void PrintHeading1(const std::string& heading) {
  std::cout << std::endl << std::string(78, '=') << std::endl;
  std::cout << heading << std::endl;
  std::cout << std::string(78, '=') << std::endl << std::endl;
}

void PrintHeading2(const std::string& heading) {
  std::cout << std::endl << heading << std::endl;
  std::cout << std::string(std::min<int>(heading.size(), 78), '-') << std::endl;
}
//
//template <>
//std::vector<std::string> CSVToVector(const std::string& csv) {
//  auto elems = StringSplit(csv, ",;");
//  std::vector<std::string> values;
//  values.reserve(elems.size());
//  for (auto& elem : elems) {
//    StringTrim(&elem);
//    if (elem.empty()) {
//      continue;
//    }
//    values.push_back(elem);
//  }
//  return values;
//}
//
//template <>
//std::vector<int> CSVToVector(const std::string& csv) {
//  auto elems = StringSplit(csv, ",;");
//  std::vector<int> values;
//  values.reserve(elems.size());
//  for (auto& elem : elems) {
//    StringTrim(&elem);
//    if (elem.empty()) {
//      continue;
//    }
//    try {
//      values.push_back(std::stoi(elem));
//    } catch (const std::invalid_argument&) {
//      return std::vector<int>(0);
//    }
//  }
//  return values;
//}
//
//template <>
//std::vector<float> CSVToVector(const std::string& csv) {
//  auto elems = StringSplit(csv, ",;");
//  std::vector<float> values;
//  values.reserve(elems.size());
//  for (auto& elem : elems) {
//    StringTrim(&elem);
//    if (elem.empty()) {
//      continue;
//    }
//    try {
//      values.push_back(std::stod(elem));
//    } catch (const std::invalid_argument&) {
//      return std::vector<float>(0);
//    }
//  }
//  return values;
//}
//
//template <>
//std::vector<double> CSVToVector(const std::string& csv) {
//  auto elems = StringSplit(csv, ",;");
//  std::vector<double> values;
//  values.reserve(elems.size());
//  for (auto& elem : elems) {
//    StringTrim(&elem);
//    if (elem.empty()) {
//      continue;
//    }
//    try {
//      values.push_back(std::stold(elem));
//    } catch (const std::invalid_argument&) {
//      return std::vector<double>(0);
//    }
//  }
//  return values;
//}

std::vector<std::string> ReadTextFileLines(const std::string& path) {
  std::ifstream file = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::in);
 // CHECK(file.is_open()) << path;

  std::string line;
  std::vector<std::string> lines;
  while (std::getline(file, line)) {
    StringTrim(&line);

    if (line.empty()) {
      continue;
    }

    lines.push_back(line);
  }

  return lines;
}

void RemoveCommandLineArgument(const std::string& arg, int* argc, char** argv) {
  for (int i = 0; i < *argc; ++i) {
    if (argv[i] == arg) {
      for (int j = i + 1; j < *argc; ++j) {
        argv[i] = argv[j];
      }
      *argc -= 1;
      break;
    }
  }
}

}  // namespace colmap
