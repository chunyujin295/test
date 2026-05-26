#ifndef _AI3D_CORE_RAPIDJSON_H_
#define _AI3D_CORE_RAPIDJSON_H_

#include <iostream>
#include <vector>
#include <rapidjson\document.h>
#include <rapidjson\writer.h>
#include <rapidjson\stringbuffer.h>
#include "rapidjson\prettywriter.h"
#include <Constants.h>
#include "rapidjson/filewritestream.h"
#include <rapidjson/writer.h>
#include <cstdio>
#ifdef _MSC_VER
#include <Windows.h>
#endif
namespace AI3D
{

	namespace CORE
	{
		class AI3D_API RapidJsonCore
		{
		public:
			RapidJsonCore();
			~RapidJsonCore();
			static int ReadFile(const std::string& path, std::string& strs);
			static int WReadFile(const std::wstring& path, std::wstring& strs);
			
			
			static std::string UTF8ToANSI(const std::string& s);
			static int SaveFile(const std::string& path, const std::string& strs);
			static int SaveFile(const std::string& path, const rapidjson::Document& doc);

		};

		


	}
}




#endif