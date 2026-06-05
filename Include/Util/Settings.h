#pragma once
#include <qwidget.h>
#include <QPushButton>
#include <QResizeEvent>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCloseEvent>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLCDNumber>
#include <QTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QCheckBox>

#include <QTextStream>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QListWidget>
#include <QResizeEvent>

#include <QSortFilterProxyModel>
#include <qdatetime.h>
#include <qtimer.h>
#include <QMutex>

struct RunningEngineNode
{
    QString hostName;
    QString ipAddr;
    QString projectName;
    
    QString projectPath;
    QString atBlockPath;
    QString jobName;
};

class JobStage
{
public:
    JobStage() {}

public:
    QString functionName;
    int status; 
    int percent;
    int percentAcc;
    QString totalTime;
    int completedNum;
    int stagedTotalNum;
    QString stageTotalTime;
};


class JobTask
{
public:
    JobTask() {}

public:
    int status;
    int progress;
    int type;
    QString sdtime;
    QString edtime;
    QString functionName;
};

extern QString progName;
extern QString progFileName;
class Settings
{

public:

    Settings() {};
    virtual ~Settings() {};
    static  bool isEngine();
    static QString getMasterJobQueue();
    static QString getEngineJobQueue();
    static QString getGenEngineJobQueue();
   
    static QSettings* pSettings;
    
};


class ExecTask
{
public:
    QString taskName; 
    QString taskStatusFile;
};

extern void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
