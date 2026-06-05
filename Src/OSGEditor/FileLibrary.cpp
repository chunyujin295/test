///////////////////////////////////////////////////////////
//  FileLibrary.cpp
//  Implementation of the Class FileLibrary
//  Created on:      11-五月-2017 17:40:43
//  Original author: bfzhao
///////////////////////////////////////////////////////////

#include <FileLibrary.h>
#include "Core/File.h"
#include <filesystem>
#include <shlobj.h>
#include <shlwapi.h>
#include "psapi.h"    
#include"stdio.h"  
#include <tlhelp32.h>  

//using namespace std;

#pragma comment(lib, "Shlwapi")

FileLibrary* FileLibrary::m_pInstance = NULL;
//ErrClass *ErrClass::instance = NULL;


FileLibrary *FileLibrary::getInstance(){
    if (m_pInstance == NULL){
        m_pInstance = new FileLibrary();
    }
    return m_pInstance;
}

FileLibrary::FileLibrary(){

    srand((unsigned)time(NULL));
}

FileLibrary::~FileLibrary(){

    if (m_pInstance != NULL)
    {
        delete m_pInstance;
    }
}


std::string FileLibrary::getCurrentFilePath(){
    wchar_t buffer[MAX_PATH] = { 0 };
    const DWORD len = GetModuleFileNameW(NULL, buffer, MAX_PATH);
    if (len == 0) {
        return {};
    }
    std::filesystem::path p(buffer);
    p = p.parent_path();
    return AI3D::CORE::File::BoostPathToUtf8String(p);
}


std::string FileLibrary::convertToLinuxPath(const std::string strWinPath){

    std::string strRet = strWinPath;
    strRet.replace(strRet.begin(), strRet.end(), '\\', '/');

    return strRet;
}

std::string FileLibrary::convertToWinPath(const std::string strLinuxPath){

    std::string strRet = strLinuxPath;
    if (strRet.size() > 0)
        strRet.replace(strRet.begin(), strRet.end(), '/', '\\');

    return strRet;
}

std::string FileLibrary::combineFilePath(const std::string strParent, const std::string strFileName){
    if (strParent.length() == 0) {
        return strFileName;
    }
    else if (strFileName.length() == 0) {
        return strParent;
    }

    std::string strResult;
    const char * pstring;
    strResult = strParent;
    pstring = strResult.c_str();

    //if no / or \ at the end of path,append it
    if ((pstring[strResult.length() - 1] != '\\')
        && (pstring[strResult.length() - 1] != '/')) {
        strResult.append("\\");
    }

    //if a '/ or \ at front of file name ,skip it
    pstring = strFileName.c_str();
    for (size_t i = 0; i < strFileName.length(); i++) {
        if ((*pstring == '\\') || (*pstring == '/')) {
            pstring++;
        }
        else
            break;
    }

    strResult = strResult + pstring;

    if ((strResult.at(strResult.length() - 1) == '\\' || strResult.at(strResult.length() - 1) == '/') && strResult.size() != 1)
        strResult = strResult.substr(0, strResult.length() - 1);

    return strResult;
}

std::string FileLibrary::getFileParentPath(const std::string &strFilePath){
    std::string strResult = "";
    if (strFilePath.size() == 0)
        return strResult;

    size_t lastPos2;
    lastPos2 = strFilePath.find_last_of('\\');

    //strResult.append(strFilePath, lastPos, strFilePath.length() - lastPos);
    strResult = strFilePath.substr(0, lastPos2);
    return strResult;
}


std::string FileLibrary::getFileNameFromPath(const std::string &strFilePath){

    std::string strResult = "";
    if (strFilePath.size() == 0)
        return strResult;

    //get the last \ or '/'
    size_t lastPos1, lastPos2, lastPos;
    lastPos1 = strFilePath.find_last_of('/');
    lastPos2 = strFilePath.find_last_of('\\');

    // neither / nor \ found.
    if (std::string::npos == lastPos1 && std::string::npos == lastPos2) {
        strResult = strFilePath;
        return strResult;
    }
    // only \ found
    else if (std::string::npos == lastPos1) {
        lastPos = lastPos2;
    }
    // only / found
    else if (std::string::npos == lastPos2) {
        lastPos = lastPos1;
    }
    else {
        lastPos = max(lastPos1, lastPos2);
    }

    lastPos++;

    if (lastPos == strFilePath.size())//the last is / or '\\'
    {
        strResult.append(strFilePath, 0, lastPos - 1);
        strResult = getFileNameFromPath(strResult);
    }
    else {
        //copy data after last '/' or '\\'  
        strResult.append(strFilePath, lastPos, strFilePath.length() - lastPos);
    }
    if ((strResult.size() == 0) && (strFilePath.size() == 1))//root
        strResult = "\\";

    return strResult;

}
void FileLibrary::getAllSubFiles(const std::string& strArgFilePath, list<std::string>& lstSubFiles, bool bIncludeDirs, bool bIncludeFiles, bool bRecursive, const std::string& strFilter){


    std::string strFindStr;
    std::string strLongFindStr;
    std::string strMyFindStr;
    BOOL bFinished = FALSE;


    std::string strFilePath = strArgFilePath;

    if (!bIncludeDirs && !bIncludeFiles) {
        return;
    }

    WIN32_FIND_DATAA FileData;
    HANDLE hSearch;

    strFindStr = strFilePath;
    /* if (strFilter.size())
    strFindStr = combineFilePath(strFilePath, strFilter);
    else*/
    strFindStr = combineFilePath(strFilePath, "*.*");

    /* strLongFindStr = strFindStr;
    convert2LongFilePath(strLongFindStr);
    strMyFindStr = strLongFindStr;*/


    hSearch = FindFirstFileA(strFindStr.c_str(), &FileData);
    if (hSearch == INVALID_HANDLE_VALUE)
    {
        hSearch = FindFirstFileA(strFindStr.c_str(), &FileData);
        if (hSearch == INVALID_HANDLE_VALUE)
        {
            bFinished = TRUE; //该目录下没有文件
        }
    }


    while (!bFinished)
    {
        if ((strcmp(FileData.cFileName, ".") == 0)
            || (strcmp(FileData.cFileName, "..") == 0))
        {
            if (!FindNextFileA(hSearch, &FileData))
            {
                bFinished = TRUE;
            }
            continue;
        }
        std::string strChild = combineFilePath(strFilePath, std::string(FileData.cFileName));

        if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //目录
        {
            if (bIncludeDirs)
                lstSubFiles.push_back(strChild);
        }
        else //文件
        {
            if (bIncludeFiles){
                std::string tmpname = std::string(FileData.cFileName);
                if (tmpname.find(strFilter) != std::string::npos && strFilter.size() != 0 && tmpname.substr(tmpname.length() - strFilter.length(), strFilter.length()) == strFilter)
                    lstSubFiles.push_back(strChild);

            }
        }

        //是目录 & bRecursive 就进入递归调用   
        if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && bRecursive)
        {

            getAllSubFiles(strChild, lstSubFiles, bIncludeDirs, bIncludeFiles, bRecursive, strFilter);

        }

        if (!FindNextFileA(hSearch, &FileData))
        {
            bFinished = TRUE;
        }
    }
    FindClose(hSearch);

}

bool FileLibrary::isFileExists(const std::string &strArgFilePath)
{
    if (_access(strArgFilePath.c_str(), 0) == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}


bool FileLibrary::isDirExists(const std::string &strArgFilePath){
    return isFileExists(strArgFilePath);
}


bool  FileLibrary::readFile(const std::string &filepath, vector<std::string> &vecFileInfo, const std::string &type){
    if (filepath == "")
        return false;

    if (!isFileExists(filepath))
        return false;

    fstream infile;
    std::string line;

    infile.open(filepath);

    while (getline(infile, line))
    {
        if (line.find(type) != std::string::npos){
            std::string tmpStr = line.substr(line.find("=") + 1, line.length());
            vecFileInfo.push_back(tmpStr);
        }
    }

    infile.close();

    return true;
}



vector<std::string> FileLibrary::splitString(std::string str, std::string pattern, vector<std::string> &vectstring){
    vector<std::string> ret;
    if (pattern.empty())
        return ret;

    size_t start = 0, index = str.find_first_of(pattern, 0);

    while (index != str.npos)
    {
        if (start != index)
            ret.push_back(str.substr(start, index - start));

        start = index + 1;
        index = str.find_first_of(pattern, start);
    }

    if (!str.substr(start).empty())
        ret.push_back(str.substr(start));

    vectstring = ret;

    return ret;
}


std::string FileLibrary::getRand(){
        
    return to_string(rand());
    
}

std::string FileLibrary::getCurrentTime(){

    SYSTEMTIME sys_time;
    GetLocalTime(&sys_time);
    char timebug[50] = { 0 };
    sprintf_s(timebug, "%4d-%02d-%02d %02d:%02d:%02d", sys_time.wYear, sys_time.wMonth,sys_time.wDay, sys_time.wHour, sys_time.wMinute, sys_time.wSecond/*, sys_time.wMilliseconds*/);

    return timebug;
}


int FileLibrary::copyFile(const std::string &src, const std::string &dest){
    std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(src, std::ios::binary);
    if (!in.is_open() || in.fail())
    {
        cout << "Error 1: Fail to open the source file." << endl;
        return 0;
    }
    std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(dest, std::ios::binary);

    if (!out.is_open() || out.fail())
    {
        cout << "Error 2: Fail to create the new file." << endl;
        in.close();
        return 0;
    }
    else
    {
        out << in.rdbuf();
        out.close();
        in.close();
        return 1;
    }
}

bool FileLibrary::filterGreen(float R, float G, float B, float green){
    if (G - R > green && G - B > green)
    {
        return true;
    }
    return false;
}


void FileLibrary::encryptData(string &data)
{
    return;
    static string key1 = "httpwwwmoldaidataearth";
    static string key2 = "htraeatadiadlomwwwptth";
    for (int i = 0; i < data.size(); ++i)
    {
        int keyIndex1 = i % key1.size();
        int keyIndex2 = i % key2.size();
        data[i] = data[i] ^ key1[keyIndex1];
        data[i] = data[i] ^ key2[keyIndex2];
    }
}


template <typename T>
T FileLibrary::convertFromstring(T &value, const std::string &s) {
    if (s.length() == 0 || s.empty())
    {
        value = 0;
        return value;
    }

    std::stringstream ss(s);
    ss >> value;
    return value;
};

template <class T> std::string FileLibrary::convertTostring(T &value, int precision){
    std::stringstream ss;
    ss << std::setprecision(precision) << value;
    return ss.str();
}


void FileLibrary::toEulerAngle(const Quate& q, double& roll, double& pitch, double& yaw)
{
    #define M_PI       3.14159265358979323846   // pi

    // roll (x-axis rotation)
    double sinr = +2.0 * (q.w * q.x + q.y * q.z);
    double cosr = +1.0 - 2.0 * (q.x * q.x + q.y * q.y);
    roll = atan2(sinr, cosr);

    // pitch (y-axis rotation)
    double sinp = +2.0 * (q.w * q.y - q.z * q.x);
    if (fabs(sinp) >= 1)
        pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

    // yaw (z-axis rotation)
    double siny = +2.0 * (q.w * q.z + q.x * q.y);
    double cosy = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);
    yaw = atan2(siny, cosy);
}

//判断进程是否存在  
//2012-09-10  
bool FileLibrary::FindProcess(const string & process, int num)
{
    int i = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        i += 0;
    }
    BOOL bMore = ::Process32First(hProcessSnap, &pe32);
    while (bMore)
    {
        char buf[255] = {0};
        wcstombs(buf, pe32.szExeFile,sizeof(pe32.szExeFile));

        if (strcmp(process.c_str(),buf) == 0)
        {
            //printf("进程运行中");  
           i++;
          
        }
        bMore = ::Process32Next(hProcessSnap, &pe32);
    }
    ::CloseHandle(hProcessSnap);
   if (i > num)
   {
       return true;
   }
   return false;
}


/////////////////////////////////////////////////////////////////////////////
#ifdef QT_DEF


QMutex QFileLibrary::m_mutex;
QAtomicPointer<QFileLibrary> QFileLibrary::instance = 0;

QFileLibrary *QFileLibrary::getInstance() {
    if (instance.testAndSetOrdered(0, 0))
    {
        QMutexLocker locker(&m_mutex);//加互斥锁。
        instance.testAndSetOrdered(0, new QFileLibrary);
    }
    return instance;
}
QFileLibrary::QFileLibrary() {}

QFileLibrary::~QFileLibrary() {}


bool QFileLibrary::mkDir(const QString&d) {

    QDir dir;
    return dir.mkpath(d);
}
bool QFileLibrary::mkFile(const QString&f) {
    QFile file(f);
    bool ret = file.open(QIODevice::WriteOnly);
    file.close();
    return ret;
}

bool QFileLibrary::rmDir(const QString &d) {
    QDir dir;
    return dir.rmdir(d);
}
bool QFileLibrary::rmFile(const QString&f) {
    QFile::remove(f);   
    return true;
}

qint64 QFileLibrary::getFileSize(const QString&f) {
    QFile file(f);
    return file.size(); 
}

void QFileLibrary::getKeywordFile(const QString &dirPath, QVector<QString>&vector, const QString&keyword) {
    QDir dir(dirPath);
    int result = 0;

    QDir::Filters type = QDir::Files;
    foreach(QFileInfo fileInfo, dir.entryInfoList(type))
    {
        QString strName = fileInfo.fileName();
        if ((strName == QString(".")) || (strName == QString("..")))
            continue;

        if (fileInfo.isDir())
        {
            continue;
        }
        else
        {
            if (strName.indexOf(keyword) != -1) {
                QString str = dirPath + "/" + strName;
                vector.push_back(str);
            }

        }
    }

}


void QFileLibrary::getAllFile(const QString &dirPath, QVector<QString>&vector, bool isDir, bool isFile, bool isRecursion, const QString &postfix) {
    QDir dir(dirPath);
    int result = 0;
    
    QDir::Filters type= QDir::Files;

    if (isDir) {
        type |= QDir::Dirs;
    }

    if(isFile)
    {
        type |= QDir::Files;

    }
    foreach(QFileInfo fileInfo, dir.entryInfoList(type))
    {
        QString strName = fileInfo.fileName();
        if ((strName == QString(".")) || (strName == QString("..")))
            continue;

        if (fileInfo.isDir())
        {
            QString str = dirPath + "/" + strName + "/";
            if (isDir)
                vector.push_back(str);

            if(isRecursion)
                getAllFile(str,vector, isDir, isFile, isRecursion, postfix);
        }
        else
        {
            if (postfix == "*.*" || (strName.endsWith(postfix) && isFile)) {
                QString str = dirPath + "/" + strName;
                vector.push_back(str);
            }
        }
    }

}

QStringList  QFileLibrary::splitString(const QString& str, const QString&filter) {
    return str.split(filter);
}

bool QFileLibrary::getFileContentByLine(const QString&f, QVector<QString>&vector) {

    QFile file(f);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        vector.push_back(line);
    }
    file.close();

    return true;
}

bool QFileLibrary::writeFile(const QString&f , const QString&str) {
    QMutexLocker lock(&m_fileMutex);
    QFile file(f);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(str.toUtf8());
    file.close();

    return true;
}
bool QFileLibrary::writeFile(const QString&f, const QVector<QString>&v) {
    QMutexLocker lock(&m_fileMutex);

    QFile file(f);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!file.isOpen())
        return false;

    QTextStream out(&file);
    for (size_t i = 0; i < v.size(); i++)
    {   
        out << v.at(0) << endl;
    }
    file.close();


    return true;
}

bool QFileLibrary::isDirExist(const QString&d) {
    QDir dir;
    return dir.exists(d);
}

bool QFileLibrary::isFileExist(const QString&f) {
    //return  ((_access(f.toStdString().c_str(), 0)) != -1);
    return QFile::exists(f);
}

QString QFileLibrary::getFileName(const QString&f) {
    int charPos = f.lastIndexOf("/");
    if (charPos < 1)
    {
        charPos = f.lastIndexOf("\\");
    }
    return f.right(f.length() - charPos -1);
}

QString QFileLibrary::getFileParentPath(const QString&f) {

    int charPos = f.lastIndexOf("/");
    if (charPos < 1)
    {
        charPos = f.lastIndexOf("\\");
    }

    return f.left(charPos);
}

QString QFileLibrary::getCurrentDataTime() {

    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}



//删除文件夹
bool QFileLibrary::delDir(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }
    QDir dir(path);
    if (!dir.exists()) {
        return true;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    foreach(QFileInfo file, fileList) { //遍历文件信息
        if (file.isFile()) { // 是文件，删除
            file.dir().remove(file.fileName());
        }
        else { // 递归删除
            delDir(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath()); // 删除文件夹
}

//拷贝文件：
bool QFileLibrary::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
{
    toDir.replace("\\", "/");
    if (sourceDir == toDir) {
        return true;
    }
    if (!QFile::exists(sourceDir)) {
        return false;
    }
    QDir *createfile = new QDir;
    bool exist = createfile->exists(toDir);
    if (exist) {
        if (coverFileIfExist) {
            createfile->remove(toDir);
        }
    }//end if

    if (!QFile::copy(sourceDir, toDir))
    {
        return false;
    }
    return true;
}

//拷贝文件夹：
bool QFileLibrary::copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if (!targetDir.exists()) {    /**< 如果目标目录不存在，则进行创建 */
        if (!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir()) {    /**< 当为目录时，递归的进行copy */
            if (!copyDirectoryFiles(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()),
                coverFileIfExist))
                continue;
        }
        else {            /**< 当允许覆盖操作时，将旧文件进行删除操作 */
            if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
                targetDir.remove(fileInfo.fileName());
            }

            /// 进行文件copy
            if (!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) {
                continue;
            }
        }
    }
    return true;
}

QString QFileLibrary::getCurrentFilePath() {

    return QDir::currentPath();
}

void QFileLibrary::setViewDataForm(qlonglong obj) {
    m_obj = obj;
}

qlonglong QFileLibrary::getViewDataForm() {
    return m_obj;
}


int QFileLibrary::isDigitStr(QString src)
{
    QByteArray ba = src.toLatin1();//QString 转换为 char*
    const char *s = ba.data();

    while (*s && *s >= '0' && *s <= '9') s++;

    if (*s)
    { //不是纯数字
        return -1;
    }
    else
    { //纯数字
        return 0;
    }
}

QString QFileLibrary::getValueFromConfig(const QString &key) {
    QFile file("./config.ini");
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        file.setFileName("./bin/config.ini");
        file.open(QIODevice::ReadOnly);
    }

    QString value;
    while (!file.atEnd())
    {
        QString line = file.readLine();
        if (line.indexOf(key) > -1 && line.lastIndexOf("=") > -1) {
            value = line.right(line.length() - line.lastIndexOf("=") - 1).trimmed();
            break;
        }
    }

    file.close();       
    return value;

}

QString QFileLibrary::getLogPath() {
    QDir dir;
    QString logPath;
    if (dir.exists("bin"))
    {
        logPath = "bin/MoldAIMapBuilder.log";
    }
    else {
        logPath = "./MoldAIMapBuilder.log";
    }

    return logPath;
}
#if 0
//日志生成
void QFileLibrary::LogMsgOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    
    QMutexLocker look(&mutex);
    cout << msg.toStdString() << endl;
    //Critical Resource of Code
    QByteArray localMsg = msg.toUtf8();
    QString log;

    switch (type) {
    case QtDebugMsg:
        log.append(QString("date:%1: File:%2: Line:%3: %4").arg(getCurrentDataTime()).arg(FileLibrary::getInstance()->getFileNameFromPath(context.file).c_str()).arg(context.line).arg(msg));
        break;
    case QtInfoMsg:
        log.append(QString("Info: %1  %2  %3  %4").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function));
        break;
    case QtWarningMsg:
        log.append(QString("Warning: %1  %2  %3  %4").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function));
        break;
    case QtCriticalMsg:
        log.append(QString("Critical: %1  %2  %3  %4").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function));
        break;
    case QtFatalMsg:
        log.append(QString("Fatal: %1  %2  %3  %4").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function));
        abort();
    }

    QFile file;
    QString path = QString("MoldAIDataProcessPro.log");// .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    file.setFileName(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QString erinfo = file.errorString();
        cout << erinfo.toStdString() << endl;
        return;
    }
    QTextStream out(&file);
    out << "\n\r" << log;
    file.close();

    mutex.unlock();
}
#endif

#endif // QT_DEF
