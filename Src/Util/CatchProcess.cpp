#include "Util/CatchProcess.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QCloseEvent>
#include "Util/TaskProcess.h"

CatchProcess::CatchProcess(QObject *parent)
	: QObject(parent)
{
}

CatchProcess::~CatchProcess()
{
}

#ifdef WIN32
char *CatchProcess::Wchar2char(const wchar_t *szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

bool CatchProcess::IsProgramRunning(QString program_name)
{
    bool ret = false;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        qWarning("CreateToolhelp32Snapshot failed!");
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    
    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            
            QString exeName = QString::fromWCharArray(pe32.szExeFile);

            if (program_name.compare(exeName, Qt::CaseInsensitive) == 0)
            {
                ret = true;
                break;
            }

        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return ret;
}
bool CatchProcess::IsProgramRunning(QString program_name, DWORD& processId)
{
    bool ret = false;
    processId = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        qWarning("CreateToolhelp32Snapshot fail!!");
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            QString exeName = QString::fromWCharArray(pe32.szExeFile);

            if (program_name.compare(exeName, Qt::CaseInsensitive) == 0)
            {
                processId = pe32.th32ProcessID;
                ret = true;
                break;
            }

        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return ret;
}

int CatchProcess::NumProgramRunning(QString program_name)
{
    int ret = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        qWarning("CreateToolhelp32Snapshot fail!!");
        return 0;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            QString exeName = QString::fromWCharArray(pe32.szExeFile);

            if (program_name.compare(exeName, Qt::CaseInsensitive) == 0)
            {
                ret++;
            }

        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return ret;
}
#endif 

MsgBoxThread::MsgBoxThread(QObject* parent)
	: QThread()
{
	connect(this, SIGNAL(msgbox_sig(const QString &)), SLOT(showMsgBox(const QString &)),Qt::BlockingQueuedConnection);
}

void MsgBoxThread::showMsgBox(const QString &str)
{


#if 1

	OpenLoadingPromptV3(str);
#else
	QMessageBox::information(nullptr,"path info",str);
#endif

	
}

void MsgBoxThread::showInformation(const QString &str)
{
	
	emit msgbox_sig(str);
}

CustomRoundWidget::CustomRoundWidget(QWidget* parent)
	: QWidget(parent)
{
	
	setWindowFlags(Qt::FramelessWindowHint);
	setStyleSheet("background-color:#303030;border-radius:6px;");

	QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
	shadowEffect->setOffset(2, 2);
	
	shadowEffect->setColor(QColor(0, 0, 0, 128));
	shadowEffect->setBlurRadius(8);
	this->setGraphicsEffect(shadowEffect);
}

void CustomRoundWidget::resizeEvent(QResizeEvent* event)
{

}

CustomMsgBox::CustomMsgBox(QWidget* parent)
	: CustomRoundWidget(parent) 
{
	
	QVBoxLayout* vlayoutTop = new QVBoxLayout();
	vlayoutTop->setContentsMargins(0, 0, 0, 0);

	
	vlayout = new QVBoxLayout();
	vlayout->setContentsMargins(0, 0, 0, 0);

	QFrame* topFrame = new QFrame(this);
	

	topFrame->setStyleSheet("background-color:#303030;margin:0px;");

	QHBoxLayout* hlayout0 = new QHBoxLayout();
	hlayout0->setContentsMargins(0, 0, 0, 0);

	lblJobQueue = new QLabel(this);
	lblJobQueue->setText("Job setting");
	lblJobQueue->setStyleSheet("color:#FFFFFF;margin:17px 0px 17px 24px;font: 14px 'Arial';letter-spacing: 0;");


	hlayout0->addWidget(lblJobQueue, 0, Qt::AlignLeft);

	vlayout->addLayout(hlayout0, 0);

	QFrame* frameMaster = new QFrame(this);
	frameMaster->setStyleSheet("background-color:#3D3D3D;margin:10px 20px 0px 20px;border-radius:4px;");

	QVBoxLayout* vLayoutMaster = new QVBoxLayout();

	QHBoxLayout* hLayout10 = new QHBoxLayout();

	QLabel* lblMaster = new QLabel(this);
	lblMaster->setText("Master job setting");
	lblMaster->setStyleSheet("color:#FFFFFF;font: 14px 'Arial';letter-spacing: 0;");

	hLayout10->addWidget(lblMaster, 0, Qt::AlignLeft);

	QHBoxLayout* hLayout11 = new QHBoxLayout();

	hLayout11->setSpacing(0);
	
	hLayout11->addStretch(1);

	QLabel* lblMasterJobQueue = new QLabel(this);
	lblMasterJobQueue->setText("Job queue path");
	lblMasterJobQueue->setStyleSheet("color:#A5A5A5;margin:0px 16px 0px 0px;font: 14px 'Arial';letter-spacing: 0;");
	
	hLayout11->addWidget(lblMasterJobQueue, 0, Qt::AlignRight );

	leMaster = new QLineEdit(this);
	leMaster->setText("");
	

	leMaster->setEnabled(false);

	leMaster->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:234px;height:30px;border:none;border-top-left-radius:4px;border-top-right-radius:0px;border-bottom-right-radius:0px;border-bottom-left-radius:4px;font: 14px 'Arial';letter-spacing: 0;");

	hLayout11->addWidget(leMaster, 0, Qt::AlignRight | Qt::AlignVCenter);

	butPathMaster = new QPushButton(this);
	butPathMaster->setText("...");

	butPathMaster->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:30px;height:30px;border:1px #3D3D3D solid;border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;border-bottom-left-radius:0px;");
	hLayout11->addWidget(butPathMaster, 0, Qt::AlignRight | Qt::AlignVCenter);

	vLayoutMaster->addLayout(hLayout10);
	vLayoutMaster->addLayout(hLayout11);

	frameMaster->setLayout(vLayoutMaster);

	QFrame* frameEngine = new QFrame(this);
	frameEngine->setStyleSheet("background-color:#3D3D3D;margin:10px 20px 0px 20px;border-radius:4px;");

	QVBoxLayout* vLayoutEngine = new QVBoxLayout();

	QHBoxLayout* hLayout20 = new QHBoxLayout();

	QLabel* lblEngine = new QLabel(this);
	lblEngine->setText("Engine job setting");
	lblEngine->setStyleSheet("color:#FFFFFF;font: 14px 'Arial';letter-spacing: 0;");

	hLayout20->addWidget(lblEngine, 0, Qt::AlignLeft);

	QHBoxLayout* hLayout21 = new QHBoxLayout();

	hLayout21->setAlignment(Qt::AlignVCenter);
	hLayout21->setSpacing(0);
	hLayout21->addStretch(1);

	QLabel* lblEngineJobQueue = new QLabel(this);
	lblEngineJobQueue->setText("Job queue path");
	
	
	lblEngineJobQueue->setStyleSheet("color:#A5A5A5;margin:0px 16px 0px 0px;font: 14px 'Arial';letter-spacing: 0;");
	hLayout21->addWidget(lblEngineJobQueue, 0, Qt::AlignRight | Qt::AlignVCenter);

	leEngine = new QLineEdit(this);
	leEngine->setText("");
	leEngine->setEnabled(false);

		
	leEngine->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:234px;height:30px;border:none;border-top-left-radius:4px;border-top-right-radius:0px;border-bottom-right-radius:0px;border-bottom-left-radius:4px;font: 14px 'Arial';letter-spacing: 0;");
	hLayout21->addWidget(leEngine, 0, Qt::AlignRight | Qt::AlignVCenter);

	butPathEngine = new QPushButton(this);
	butPathEngine->setText("...");
	
	butPathEngine->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:30px;height:30px;border:1px #3D3D3D solid;border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;border-bottom-left-radius:0px;");
	hLayout21->addWidget(butPathEngine, 0, Qt::AlignRight | Qt::AlignVCenter);

	vLayoutEngine->addLayout(hLayout20);
	vLayoutEngine->addLayout(hLayout21);

	frameEngine->setLayout(vLayoutEngine);

	QHBoxLayout* hLayout30 = new QHBoxLayout();
	hLayout30->addStretch(1);

	butUpdate = new QPushButton(this);
	butUpdate->setText("OK");
	butUpdate->setStyleSheet("background-color:#538CCF;color:white;width:140px;height:28px;margin:20px 0px 20px 20px;border-radius:4px;font: 12px 'Arial';letter-spacing: 0;");
	hLayout30->addWidget(butUpdate, 0, Qt::AlignRight);


	butClose = new QPushButton(this);
	butClose->setStyleSheet("background-color:#545454;color:white;width:140px;height:28px;margin:20px 20px 20px 10px;border-radius:4px;font: 12px 'Arial';letter-spacing: 0;");
	butClose->setText("Close");
	hLayout30->addWidget(butClose, 0, Qt::AlignRight);

	vlayout->addWidget(frameMaster, 1);
	vlayout->addWidget(frameEngine, 1);

	vlayout->addLayout(hLayout30);

	QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
	shadowEffect->setOffset(0, 1);
	
	shadowEffect->setColor(QColor(0, 0, 0, 128));
	shadowEffect->setBlurRadius(1);

	butUpdate->setGraphicsEffect(shadowEffect);
	butClose->setGraphicsEffect(shadowEffect);

	topFrame->setLayout(vlayout);
	vlayoutTop->addWidget(topFrame);

	setLayout(vlayoutTop);

	connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));
	connect(butUpdate, SIGNAL(clicked()), this, SLOT(funcUpdate()));
}

CustomMsgBox::~CustomMsgBox()
{

}

void CustomMsgBox::funcUpdate()
{
	close();
}

void CustomMsgBox::closeEvent(QCloseEvent* evt)
{
	evt->accept();
	
	exit(0);
}

void CustomMsgBox::funcClose() {
	close();
}




LoadingPromptV3::LoadingPromptV3(QWidget* parent, QString strInformation)
	: CustomRoundWidget(parent), strDefault(strInformation)
{
	
	QVBoxLayout* vlayoutTop = new QVBoxLayout();
	vlayoutTop->setContentsMargins(0, 0, 0, 0);

	
	vlayout = new QVBoxLayout();
	vlayout->setContentsMargins(0, 0, 0, 0);

	QFrame* topFrame = new QFrame(this);

	
	topFrame->setStyleSheet("background-color:#303030;margin:0px;");



	lblPrompt = new QLabel(this);
	
	
	lblPrompt->setText("Network Unavailable");
	
	lblPrompt->setStyleSheet("margin-top:8px;color:white;");

	vlayout->addWidget(lblPrompt, 0, Qt::AlignHCenter);

	lblCloseTime = new QLabel(this);
	
	lblCloseTime->setText(strDefault);
	std::cout  << __FUNCTION__ << qstr2str(strDefault) << std::endl;
	
	lblCloseTime->setStyleSheet("margin-left:15px;margin-right:15px;margin-bottom:8px;color:#A5A5A5;");
	

	vlayout->addWidget(lblCloseTime, 1, Qt::AlignHCenter);

	butClose = new QPushButton(this);
	butClose->setText("Close");


	butClose->setStyleSheet("background-color:#538CCF;color:white;width:140px;height:28px;margin:2px 10px 10px 10px;border-radius:4px;font: 12px 'Arial';letter-spacing: 0;");

	vlayout->addWidget(butClose, 0, Qt::AlignHCenter);

	topFrame->setLayout(vlayout);
	vlayoutTop->addWidget(topFrame);

	setLayout(vlayoutTop);

	connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));


}

LoadingPromptV3::~LoadingPromptV3()
{

}

void LoadingPromptV3::funcTimeout()
{
	
	

	

	
	

	
	
}

void LoadingPromptV3::funcClose()
{
	close();
	exit(0);
}


void LoadingPromptV3::closeEvent(QCloseEvent* evt)
{
	evt->accept();
}

LoadingPromptV3* pLoadingPromptV3 = nullptr;

void OpenLoadingPromptV3(QString strInformation)
{
	if (pLoadingPromptV3 != nullptr)
		return;

	pLoadingPromptV3 = new LoadingPromptV3(nullptr, strInformation);

	pLoadingPromptV3->setWindowModality(Qt::ApplicationModal);
	pLoadingPromptV3->setAttribute(Qt::WA_DeleteOnClose);

	
	pLoadingPromptV3->resize(450, 60);
	pLoadingPromptV3->show();
	
}

void CloseLoadingPromptV3()
{
	

	if (pLoadingPromptV3 == nullptr)
		return;

	pLoadingPromptV3->close();
	pLoadingPromptV3 = nullptr;
}
