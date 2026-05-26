#pragma once

#include <string>
#include <iostream>
#include <fstream>


#include <QDebug>
#include <QList>
#include <QSettings>

#include <QDir>
#include <QFileInfoList>
#include <QStringList>
#include <QTime>
#include <QtGlobal>

#include <stdio.h>
#include <share.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>
#include <QMap>
#include <QJsonArray>
#include <QApplication>
#include <QCoreApplication>
#include <QProcess>

#include <Reconstruction/Reconstruct.h>

#include <functional>
#define CURVERSION(x) x 
#define STR(x) #x


class XLog
{
public:
	static std::ostream& stream() {
		return *os;
	}

	static std::ostream& showMsg(std::string& msg, const char* file, char* func, int line)
	{
		*os << file << " " << func << " " << line << " :" << msg << std::endl;
		return *os;
	}

	static std::string& std_string_format(std::string& _str, const char* _Format, ...) {
		std::string tmp;

		va_list marker = NULL;
		va_start(marker, _Format);

		size_t num_of_chars = _vscprintf(_Format, marker);

		if (num_of_chars > tmp.capacity()) {
			tmp.resize(num_of_chars + 1);
		}

		vsprintf_s((char*)tmp.data(), tmp.capacity(), _Format, marker);

		va_end(marker);

		_str = tmp.c_str();
		return _str;
	}

	static std::ostream& log(const char* file, char* func, int line, const char* format, ...)
	{
		std::string msg;

		va_list va = NULL;
		va_start(va, format);

		size_t num_of_chars = _vscprintf(format, va);

		if (num_of_chars > msg.capacity()) {
			msg.resize(num_of_chars + 1);
		}

		vsprintf_s((char*)msg.data(), msg.capacity(), format, va);

		va_end(va);

		*os << file << " " << func << " " << line << " :" << msg << std::endl;

		return *os;
	}

public:
	static std::ostream* os;
};

std::ostream* XLog::os = &std::cout;

#define XLOGI(msg) XLog::stream() << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << ":" << msg << std::endl;
#define XLOGV(msg) XLog::showMsg(msg,__FILE__,__FUNCTION__,__LINE__);



