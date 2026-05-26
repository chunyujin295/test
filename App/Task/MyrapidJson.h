#pragma once
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include <iostream>
#include <fstream>
#include <string>
#include <list>

inline bool exists_File(const std::string& name) { struct stat buffer; return (stat(name.c_str(), &buffer) == 0); };

class MyrapidJson
{
public:
	MyrapidJson();
	MyrapidJson(std::string& pathName);
	~MyrapidJson();


	std::tuple<std::string, std::string, int> parseFunctionName(std::string taskName);
	std::tuple<std::string, std::string, int,std::string> parseFunctionName(std::string taskName,std::string taskContent);
	
	int saveFile(const std::string& path, const std::string& strs);

private:
	rapidjson::Document    _jsonDocument;
	rapidjson::Document doc;
	std::string my_pathName;
};

