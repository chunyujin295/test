///////////////////////////////////////////////////////////
//  FileLibrary.h
//  Implementation of the Class FileLibrary
//  Created on:      11-五月-2017 17:40:43
//  Original author: bfzhao
///////////////////////////////////////////////////////////
#pragma once


#include <vector>
#include <map>
#include <list>
#include <sstream>
#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include  <io.h>

using namespace std;

struct Quate{
    float x, y, z, w;

};
#ifndef OSGEDITOR_DLL_API_GUARD
#ifdef API_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API
#endif
#endif
/**
    * 公用方法文件
    */
class  FileLibrary
{

public:
    FileLibrary();

    virtual ~FileLibrary();

    static FileLibrary * getInstance();

    bool isDirExists(const std::string& strArgFilePath);
    /**
        * 判断文件是否存在
        */
    bool isFileExists(const std::string& strArgFilePath);
    /**
        * 根据路径取出文件名称
        */
    std::string getFileNameFromPath(const std::string& strFilePath);


    /**
        * 链接字符串目录
        */
    std::string combineFilePath(const std::string prm1, const std::string prm2);

    std::string convertToWinPath(const std::string prm1);

    std::string convertToLinuxPath(const std::string prm1);

    /** 获取当前路径*/
    std::string getCurrentFilePath();

    /**
        * 获取所有子文件和目录
        */
    void getAllSubFiles(const std::string& strArgFilePath, list<std::string>& lstSubFiles, bool bIncludeDirs = true, bool bIncludeFiles = true, bool bRecursive = true, const std::string& strFilter = std::string(""));

   
    //获取系统当前时间
    std::string getCurrentTime();

    //字符串分割过滤
    vector< std::string> splitString(std::string str, std::string pattern, vector<std::string>& vectstring);


    bool readFile(const std::string& filepath, vector<std::string>& vecFileInfo, const std::string& type);

    //获取文件的父路径
    std::string getFileParentPath(const std::string& strFilePath);

    /*复制文件*/
    int copyFile(const std::string &src,const std::string &dst);

    std::string getRand();

    //HSV过滤绿色值
    bool Rgb2Hsv(float R, float G, float B);

    //字符串转数值类型
    template <class T> T convertFromstring(T &value, const std::string &s);
    template <class T> std::string convertTostring(T &value, int precision = 9);

    //四元数转roll, pitch, yaw
    void toEulerAngle(const Quate& q, double& roll, double& pitch, double& yaw);
    
    void encryptData(string &data);
    
    //绿色rgb值过滤
    bool filterGreen(float R, float G, float B, float green);

    template<class T> int   parse_arg(int argc, char** argv, const char* argument_name, T& value1)
    {
        int index = find_argument(argc, argv, argument_name) + 1;

        if (index > 0 && index < argc)
        {
            std::istringstream stream;
            stream.clear();
            stream.str(argv[index]);
            stream >> value1;
        }

        return (index - 2);
    }

    template<class T> int   parse_2x_arg(int argc, char** argv, const char* argument_name, T& value1, T& value2)
    {
        int index = find_argument(argc, argv, argument_name) + 1;

        if (index > 0 && index < argc)
        {
            std::istringstream stream;
            stream.clear();
            stream.str(argv[index]);
            stream >> value1;
        }

        index = find_argument(argc, argv, argument_name) + 2;

        if (index > 0 && index < argc)
        {
            std::istringstream stream;
            stream.clear();
            stream.str(argv[index]);
            stream >> value2;
        }

        return (index - 2);
    }


    template<class T> int parse_3x_arg(int argc, char** argv, const char* argument_name, T& value1, T& value2, T& value3)
    {
        int index = find_argument(argc, argv, argument_name) + 1;

        if (index > 0 && index < argc)
        {
            std::istringstream stream;
            stream.clear();
            stream.str(argv[index]);
            stream >> value1;
        }

        index = find_argument(argc, argv, argument_name) + 2;

        if (index > 0 && index < argc)
        {
            std::istringstream stream;
            stream.clear();
            stream.str(argv[index]);
            stream >> value2;
        }

        index = find_argument(argc, argv, argument_name) + 3;

        if (index > 0 && index < argc)
        {
            std::istringstream stream;
            stream.clear();
            stream.str(argv[index]);
            stream >> value3;
        }

        return (index - 3);
    }

    bool FindProcess(const string &, int);
private:
    static FileLibrary* m_pInstance;


};


#ifdef QT_DEF
///////////////////////////////////////////////////////////
//  FileLibrary.h
//  Implementation of the Class QFileLibrary
//  Created on:      11-11-2018 17:40:43
//  Original author: bfzhao
///////////////////////////////////////////////////////////

#include <qfile.h>
#include <qdir.h>
#include <qmutex.h>
#include <qstring.h>
#include <qlist.h>
#include <qvector.h>
#include <qmap.h>
#include <qtextstream.h>
#include "qdatetime.h"
//
class QFileLibrary {
    public:


        virtual ~QFileLibrary();

        static QFileLibrary * getInstance();

        bool mkDir(const QString&);
        bool mkFile(const QString&);

        bool rmDir(const QString &);
        bool rmFile(const QString&);

        bool delDir(const QString &);
        bool copyFileToPath(QString , QString , bool );
        bool copyDirectoryFiles(const QString &, const QString &, bool );


        qint64 getFileSize(const QString&);

        //查找后缀结束的文件
        void getAllFile(const QString &, QVector<QString>&, bool, bool,bool,const QString&);

        //查找关键字匹配的文件
        void getKeywordFile(const QString &, QVector<QString>&, const QString&);

        QStringList  splitString(const QString&,const QString&);

        bool getFileContentByLine(const QString&, QVector<QString>&);
        bool writeFile(const QString&, const QString&);
        bool writeFile(const QString&, const QVector<QString>&);

        bool isDirExist(const QString&);        
        bool isFileExist(const QString&);

        QString getFileName(const QString&);
        QString getFileParentPath(const QString&);

        QString getCurrentDataTime();

        QString getCurrentFilePath();

        void setViewDataForm(qlonglong);

        qlonglong getViewDataForm();

        //是否纯数字
        int isDigitStr(QString src);

        QString getValueFromConfig(const QString &);

        QString getLogPath();
        //void LogMsgOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
private:
    QFileLibrary();//禁止构造函数。
    QFileLibrary(const QFileLibrary &);//禁止拷贝构造函数。
    QFileLibrary & operator=(const QFileLibrary &);

    QMutex mutex;//日志代码互斥锁
    QString timePoint;


private:

    static QAtomicPointer<QFileLibrary> instance;
    static QMutex m_mutex;
    QMutex m_fileMutex;

    qlonglong m_obj;
};



enum errEnum
{
    ERROR_DATA_FILE_OPEN = 0x00,
    ERROR_DATA_FILE,

    INFO_UPLOAD_START,
    INFO_UPLOAD_STOP,
    INFO_UPLOAD_SUCCESS,
    INFO_UPLOAD_PROCESSING,

    ERROR_DB_CREATE,
    ERROR_DB_INSERT,

    ERROR_BOS_INIT,
    ERROR_BOS_COMPLETE,
};

class  ErrClass
{
public:
    static ErrClass *getInstance() {
        if (instance == NULL)
            instance = new ErrClass;

        return instance;
    };

    ErrClass() {
        errMsg[ERROR_DATA_FILE_OPEN] = QStringLiteral("源文件打开失败");
        errMsg[ERROR_DATA_FILE] = QStringLiteral("源文件错误");

        errMsg[INFO_UPLOAD_START] = QStringLiteral("开始");
        errMsg[INFO_UPLOAD_STOP] = QStringLiteral("暂停");
        errMsg[INFO_UPLOAD_SUCCESS] = QStringLiteral("失败");
        errMsg[INFO_UPLOAD_PROCESSING] = QStringLiteral("上传完成,处理中");


        errMsg[ERROR_DB_CREATE] = QStringLiteral("数据库文件打开失败");
        errMsg[ERROR_DB_INSERT] = QStringLiteral("数据插入失败");

        errMsg[ERROR_BOS_INIT] = QStringLiteral("bos初始化失败");
        errMsg[ERROR_BOS_COMPLETE] = QStringLiteral("bos提交错误");



    };

    ~ErrClass() {};

    QString getMsg(errEnum type) { return errMsg[type]; };

private:
    QString errMsg[20];
    static ErrClass *instance;
};

#endif