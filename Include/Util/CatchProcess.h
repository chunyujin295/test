#pragma once
#include <QObject>
#include <QThread>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include "windows.h"
#include "tlhelp32.h"
class CatchProcess : public QObject
{
	Q_OBJECT

public:
	CatchProcess(QObject *parent = nullptr);
	~CatchProcess();
#ifdef WIN32
	char *Wchar2char(const wchar_t *szStr);
	bool IsProgramRunning(QString program_name, DWORD& processId);
	bool IsProgramRunning(QString program_name);
	int NumProgramRunning(QString program_name);
#endif 
};

class MsgBoxThread : public QThread
{
	Q_OBJECT
public:
	MsgBoxThread(QObject* parent);

signals:
	void msgbox_sig(const QString &str);

public:
	void showInformation(const QString &str);

public slots:
	void showMsgBox(const QString &str);

};

class CustomRoundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomRoundWidget(QWidget* parent = nullptr);

    void resizeEvent(QResizeEvent* event);
};

class CustomMsgBox :
    public CustomRoundWidget 
{
    Q_OBJECT
public:
    CustomMsgBox(QWidget* parent = nullptr);
    virtual ~CustomMsgBox();

public slots:
    void funcUpdate();
    void funcClose();

signals:
    void SettingsClosed();
    void SettingsChanged();

private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;
    
    QLabel* lblJobQueue;

    QLabel* lblMaster;
    QLineEdit* leMaster;

    QLabel* lblEngine;
    QLineEdit* leEngine;

    QPushButton* butPathMaster;
    QPushButton* butPathEngine;

    QPushButton* butClose;
    QPushButton* butUpdate;
    QPushButton* butReset;
};

class LoadingPromptV3 :
    public CustomRoundWidget
{
    Q_OBJECT
public:
    LoadingPromptV3(QWidget* parent = nullptr, QString strInformation = "");
    virtual ~LoadingPromptV3();

public slots:
    void funcTimeout();
    void funcClose();

    
    

private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;

    QLabel* lblPrompt;
    QLabel* lblCloseTime;
    QPushButton* butClose;

    QTimer* pTimer;
    QTime* pTime;
    QString strDefault;
};

extern void OpenLoadingPromptV3(QString strInformation);
extern void CloseLoadingPromptV3();