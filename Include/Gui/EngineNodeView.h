#pragma once
#include <qwidget.h>
//#include <QWidget>
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


#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QListWidget>
#include <QResizeEvent>

#include <QSortFilterProxyModel>
#include <qdatetime.h>
#include <qtimer.h>
#include "Util/JobMonitor.h"
//extern void OpenSettings();
//? 好像该界面窗口不可拉伸也不可移动
//只显示正在执行的吗，还是
enum EngineInfoCol_e
{
    HOSTNAME_COL = 0,
    IP_COL,
    FREEMEM_COL,
    TOTALMEM_COL,
    ENGINESTATUS_COL,
    PROJECTNAME_COL,
    JOBPATH_COL,
    COUNT,
};
class EngineNodeView :
    public QWidget
{
    Q_OBJECT
public:
    explicit EngineNodeView(QWidget* parent = nullptr);
    virtual ~EngineNodeView();
   

    void resizeEvent(QResizeEvent* event);
  
public slots:
    void funcOK();
    void funcClose();

   
    void funcUpdateTimeout();

    // todo:
    // read engineNodeInfo inside other thread.

signals:
    void SettingsClosed();
    void SettingsChanged();
    void finishedProcess(const QString& msg);


private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;
    QHBoxLayout* hlayout0;

    QLabel* lblEngineNodeInfo;
    QTableWidget* twEngineNodeList;

    QHBoxLayout* hlayout;

    QPushButton* butOK;
    QPushButton* butClose;

    QTimer* pGetEngineNodeInfoTimer;
    QTimer* pUpdateTimer;

   
};
