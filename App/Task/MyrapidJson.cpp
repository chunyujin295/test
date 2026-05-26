#include "MyrapidJson.h"
#include "Core/File.h"
#include <tuple>

MyrapidJson::MyrapidJson()
{

}

MyrapidJson::MyrapidJson(std::string &pathName)
{
	my_pathName = pathName;
	if (!exists_File(pathName)) {

		doc.SetObject();
		rapidjson::Value a1(rapidjson::kArrayType);
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		rapidjson::Document document;
		document.SetObject();
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
		rapidjson::Value root(rapidjson::kObjectType);
		


		rapidjson::Value myArray(rapidjson::kArrayType);
		for (int i = 0; i < 0; i++) {

			rapidjson::Value objectTemp(rapidjson::kObjectType);
			std::string key = std::to_string(i);
			std::string value = std::to_string(i) + "_" + "block";
			objectTemp.AddMember("blockName", rapidjson::Value(value.c_str(), allocator), allocator);
			myArray.PushBack(objectTemp, allocator);
		}

		

		document.AddMember("file", myArray, allocator);
		document.Accept(writer);
		saveFile(pathName, buffer.GetString());

	}
	
}

MyrapidJson::~MyrapidJson()
{

}

std::tuple<std::string, std::string, int> MyrapidJson::parseFunctionName(std::string taskName)
{
	std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(taskName, std::ios::in);
	if (!in.is_open()) {
		fprintf(stderr, "fail to read json file: %s\n", taskName.c_str());
		return std::make_tuple<std::string, std::string, int>("", "", -1);
	}

	std::string json_content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();

	rapidjson::Document dom;
	if (!dom.Parse(json_content.c_str()).HasParseError()) {
		if (dom.HasMember("meta_data") && dom["meta_data"].IsObject()) {
			std::string funcStr, msgStr;
			int typeNum = -1;
			const rapidjson::Value& obj = dom["meta_data"];
			if (obj.HasMember("function") && obj["function"].IsString()) {
				funcStr = obj["function"].GetString();
			}

			if (obj.HasMember("msg") && obj["msg"].IsString()) {
				msgStr = obj["msg"].GetString();
			}

			if (obj.HasMember("type") && obj["type"].IsInt())
				typeNum = obj["type"].GetInt();

			std::tuple<std::string, std::string, int> b = std::make_tuple<>(funcStr, msgStr, typeNum);
			return b;
		}
	}

	
	fprintf(stderr, "fail to read json file: %s\n", taskName.c_str());

	return std::make_tuple<std::string, std::string, int>("", "", -1);
}


std::tuple<std::string, std::string, int,std::string> MyrapidJson::parseFunctionName(std::string taskName,std::string json_content)
{

	rapidjson::Document dom;
	if (!dom.Parse(json_content.c_str()).HasParseError()) {
		std::string jobStr;
		if (dom.HasMember("job") && dom["job"].IsString()) {
			jobStr = dom["job"].GetString();
		}

		if (dom.HasMember("meta_data") && dom["meta_data"].IsObject()) {
			std::string funcStr, msgStr;
			int typeNum = -1;
			const rapidjson::Value& obj = dom["meta_data"];
			if (obj.HasMember("function") && obj["function"].IsString()) {
				funcStr = obj["function"].GetString();
			}

			if (obj.HasMember("msg") && obj["msg"].IsString()) {
				msgStr = obj["msg"].GetString();
			}

			if (obj.HasMember("type") && obj["type"].IsInt())
				typeNum = obj["type"].GetInt();

			std::tuple<std::string, std::string, int,std::string> b = std::make_tuple<>(funcStr, msgStr, typeNum,jobStr);
			return b;
		}
	}

	fprintf(stderr, "fail to process json file: %s content.\n", taskName.c_str());
	return std::make_tuple<std::string, std::string, int>("", "", -1,"");
}

int MyrapidJson::saveFile(const std::string& path, const std::string& strs) {

	std::ofstream fileout = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::out);
	if (!fileout.good())
		return -1;

	fileout << strs;
	fileout.close();

	return -1;
}
