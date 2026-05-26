#include "Core/Rapidjson.h"
#include <list>
#include <vector>
#include <fstream>
#include <string>
#include "Core/ReturnCode.h"
#include "Core/File.h"

namespace AI3D
{

	namespace CORE
	{

		int RapidJsonCore::SaveFile(const std::string& path, const rapidjson::Document& document)
		{
			
			FILE* fp = fopen(path.c_str(), "wb");
			char writeBuffer[65536];
			rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
			rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
			document.Accept(writer);

			fclose(fp);
			return AI3D_SUCCESS;
		}

		
		int RapidJsonCore::ReadFile(const std::string& path, std::string& strs) {

			std::ifstream in = File::OpenIfstreamUtf8(path, std::ios::in);
			if (!in.is_open())
				return FILE_OPENFILE_FAILED;
			std::string line;

			while (std::getline(in, line)) {

				if (line[line.size() - 1] != '\n')
					line.append("\n");

				strs.append(line);
			}
			in.close();

			return AI3D_SUCCESS;
		}

		
		int RapidJsonCore::SaveFile(const std::string& path, const std::string& strs) {

			std::ofstream fileout = File::OpenOfstreamUtf8(path, std::ios::out);
			if (!fileout.good())
				return SAVE_FILE_FAILED;
			
			fileout << strs;
			fileout.close();

			return AI3D_SUCCESS;
		}

		std::string RapidJsonCore::UTF8ToANSI(const std::string& s)
		{
#ifdef _MSC_VER
			BSTR    bstrWide;
			char* pszAnsi;
			int     nLength;
			const char* pszCode = s.c_str();

			nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);
			bstrWide = SysAllocStringLen(NULL, nLength);

			MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);

			nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
			pszAnsi = new char[nLength];

			WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
			SysFreeString(bstrWide);

			std::string r(pszAnsi);
			delete[] pszAnsi;
			return r;
#endif 
		}



		RapidJsonCore::RapidJsonCore()
		{
		}

		RapidJsonCore::~RapidJsonCore()
		{

		}

	}
}