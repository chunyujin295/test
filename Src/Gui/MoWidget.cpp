#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Gui/MoWidget.h"
#include "Core/ReconstructionOptions.h"
#include "Gui/BlockWgt.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QDateTime>
#include <sstream>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPixmapCache>
#include "Gui/GlobalStruct.h"
#include "Gui/MohackerWin.h"

#include <QToolTip>
#include <QApplication>
#include <QBitmap>
#include <QScrollBar>
#include <QtConcurrent>
//#include "Core/Image.h"
#include <QSet>
#include <QHash>
#include <QCryptographicHash>
#ifdef USE_AI3D_PROJ
#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Gui/ProjectionSelectionTreeWidget.h"

#endif // USE_AI3D_PROJ

static int iCurrentImagetoProcess = 0;
static int iTotalImagetoProcess = 0;

bool bNeedLoadingPrompt = false;
bool bNeedLoadingPromptV2 = false;
bool bNeedLoadingPromptV4 = false;

QSet<QWidget*> widgetSet;

extern void OpenSettingsPrompt();

QHash<QString, QPixmap> pixmapCaches;


extern void OpenSettingsPrompt();


RoundWidget::RoundWidget(QWidget* parent)
    : QWidget(parent)
{
    //setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setStyleSheet("background-color:#303030;border-radius:6px;");

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(2, 2);
    //shadowEffect->setColor(Qt::black);
    shadowEffect->setColor(QColor(0, 0, 0, 128));
    shadowEffect->setBlurRadius(8);
    this->setGraphicsEffect(shadowEffect);
}

void RoundWidget::resizeEvent(QResizeEvent* event)
{

}

RoundWidget2::RoundWidget2(QWidget* parent)
    : QWidget(parent)
{
    //setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setStyleSheet("background-color:#303030;border-radius:6px;");

    ///QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    ///shadowEffect->setOffset(2, 2);
    //shadowEffect->setColor(Qt::black);
    ///shadowEffect->setColor(QColor(0, 0, 0, 128));
    ///shadowEffect->setBlurRadius(8);
    ///this->setGraphicsEffect(shadowEffect);
}

void RoundWidget2::resizeEvent(QResizeEvent* event)
{

}

// adjust font for some widget.
// JOB队列设置
SettingsWgt::SettingsWgt(QWidget* parent)
    : RoundWidget(parent) //QWidget(parent)
{

    bHttpServerMode = true;
    bHasUpdated = false;
    bCloseButtonClicked = false;

    //QVBoxLayout *vlayoutTop = new QVBoxLayout(this);
    QVBoxLayout* vlayoutTop = new QVBoxLayout();
    vlayoutTop->setMargin(0);

    //vlayout = new QVBoxLayout(this);
    vlayout = new QVBoxLayout();
    vlayout->setMargin(0);

    QFrame* topFrame = new QFrame(this);
    //this->setAttribute(Qt::WA_StyledBackground, true);

    topFrame->setStyleSheet("background-color:#303030;margin:0px;");

    QHBoxLayout* hlayout0 = new QHBoxLayout();
    hlayout0->setMargin(0);

    lblJobQueue = new QLabel(this);
    if (AI3D::CORE::BlockObject::isChineseVersion())
    {
        lblJobQueue->setText("工作队列设置");
    }
    else
    {
        lblJobQueue->setText("Job setting");
    }
    lblJobQueue->setStyleSheet("color:#FFFFFF;margin:17px 0px 17px 24px;font: 14px 'Arial';letter-spacing: 0;");


    hlayout0->addWidget(lblJobQueue, 0, Qt::AlignLeft);

    vlayout->addLayout(hlayout0, 0);

    if (AI3D::CORE::Application::Getinstance().GetDistribution() > 0)
    {

        QFrame* frameMaster = new QFrame(this);
        frameMaster->setStyleSheet("background-color:#3D3D3D;margin:10px 20px 0px 20px;border-radius:4px;");

        QVBoxLayout* vLayoutMaster = new QVBoxLayout();

        QHBoxLayout* hLayout10 = new QHBoxLayout();

        QLabel* lblMaster = new QLabel(this);

        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblMaster->setText("主控端工作队列");
        }
        else
        {
            lblMaster->setText("Master job setting");
        }
        lblMaster->setStyleSheet("color:#FFFFFF;font: 14px 'Arial';letter-spacing: 0;");
        //lblMaster->setStyleSheet(font14QSS);

        hLayout10->addWidget(lblMaster, 0, Qt::AlignLeft);

        QHBoxLayout* hLayout11 = new QHBoxLayout();

        hLayout11->setSpacing(0);
        //hLayout11->setAlignment(Qt::AlignVCenter);
        hLayout11->addStretch(1);

        QLabel* lblMasterJobQueue = new QLabel(this);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblMasterJobQueue->setText("主控端工作队列路径");
        }
        else
        {
            lblMasterJobQueue->setText("Job queue path");
        }
        lblMasterJobQueue->setStyleSheet("color:#A5A5A5;margin:0px 16px 0px 0px;font: 14px 'Arial';letter-spacing: 0;");
        //  lblMasterJobQueue->setStyleSheet(font14QSS);
        hLayout11->addWidget(lblMasterJobQueue, 0, Qt::AlignRight /* | Qt::AlignVCenter*/);

        leMaster = new QLineEdit(this);
        leMaster->setText("");
        leMaster->setReadOnly(true);
        //  leMaster->setStyleSheet(font14QSS);
            ///leMaster->setEnabled(false);


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
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblEngine->setText("引擎工作队列");
        }
        else
        {
            lblEngine->setText("Engine job setting");
        }
        lblEngine->setStyleSheet("color:#FFFFFF;font: 14px 'Arial';letter-spacing: 0;");
        //lblEngine->setStyleSheet(font14QSS);

        hLayout20->addWidget(lblEngine, 0, Qt::AlignLeft);

        QHBoxLayout* hLayout21 = new QHBoxLayout();

        hLayout21->setAlignment(Qt::AlignVCenter);
        hLayout21->setSpacing(0);

        hLayout21->addStretch(1);

        QLabel* lblEngineJobQueue = new QLabel(this);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblEngineJobQueue->setText("引擎工作队列路径");
        }
        else
        {
            lblEngineJobQueue->setText("Job queue path");
        }
        //lblEngineJobQueue->setStyleSheet(font14QSS);
        //lblEngineJobQueue->setStyleSheet("color:#A5A5A5;margin-right:16px;height:30px;");
        lblEngineJobQueue->setStyleSheet("color:#A5A5A5;margin:0px 16px 0px 0px;font: 14px 'Arial';letter-spacing: 0;");
        hLayout21->addWidget(lblEngineJobQueue, 0, Qt::AlignRight | Qt::AlignVCenter);

        leEngine = new QLineEdit(this);
        leEngine->setText("");
        leEngine->setReadOnly(true);
        ///leEngine->setEnabled(false);
    //  leEngine->setStyleSheet(font14QSS);

        //leEngine->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:234px;height:30px;border:none;border-radius:4px 0px 0px 4px;");
        leEngine->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:234px;height:30px;border:none;border-top-left-radius:4px;border-top-right-radius:0px;border-bottom-right-radius:0px;border-bottom-left-radius:4px;font: 14px 'Arial';letter-spacing: 0;");
        hLayout21->addWidget(leEngine, 0, Qt::AlignRight | Qt::AlignVCenter);

        butPathEngine = new QPushButton(this);
        butPathEngine->setText("...");
        //  butPathEngine->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:30px;height:30px;border:none;border-radius:0px 4px 4px 0px;");
        butPathEngine->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:30px;height:30px;border:1px #3D3D3D solid;border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;border-bottom-left-radius:0px;");
        hLayout21->addWidget(butPathEngine, 0, Qt::AlignRight | Qt::AlignVCenter);

        vLayoutEngine->addLayout(hLayout20);
        vLayoutEngine->addLayout(hLayout21);

        frameEngine->setLayout(vLayoutEngine);

        QHBoxLayout* hLayout30 = new QHBoxLayout();
        hLayout30->addStretch(1);

        butUpdate = new QPushButton(this);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            butUpdate->setText("确定");
        }
        else
        {
            butUpdate->setText("OK");
        }
        butUpdate->setStyleSheet("background-color:#538CCF;color:white;width:140px;height:28px;margin:20px 0px 20px 20px;border-radius:4px;font: 12px 'Arial';letter-spacing: 0;");
        hLayout30->addWidget(butUpdate, 0, Qt::AlignRight);


        butClose = new QPushButton(this);
        butClose->setStyleSheet("background-color:#545454;color:white;width:140px;height:28px;margin:20px 20px 20px 10px;border-radius:4px;font: 12px 'Arial';letter-spacing: 0;");
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            butClose->setText("关闭");
        }
        else
        {
            butClose->setText("Close");
        }
        hLayout30->addWidget(butClose, 0, Qt::AlignRight);

        vlayout->addWidget(frameMaster, 1);
        vlayout->addWidget(frameEngine, 1);

        vlayout->addLayout(hLayout30);

        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
        shadowEffect->setOffset(0, 1);
        //shadowEffect->setColor(Qt::black);
        shadowEffect->setColor(QColor(0, 0, 0, 128));
        shadowEffect->setBlurRadius(1);

        butUpdate->setGraphicsEffect(shadowEffect);
        butClose->setGraphicsEffect(shadowEffect);

        topFrame->setLayout(vlayout);
        vlayoutTop->addWidget(topFrame);

        setLayout(vlayoutTop);

        connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));
        connect(butUpdate, SIGNAL(clicked()), this, SLOT(funcUpdate()));

        connect(butPathMaster, SIGNAL(clicked()), this, SLOT(funcPathMaster()));
        connect(butPathEngine, SIGNAL(clicked()), this, SLOT(funcPathEngine()));

        if (Settings::pSettings) {
            // 从系统注册表获取JOB队列已有设置.
            // master: Master端访问的JOB队列路径.
            // engine: Engine端访问的JOB队列路径.
            QString masterPath = Settings::pSettings->value("master").toString();
            QString enginePath = Settings::pSettings->value("engine").toString();

            //bHttpServerMode = pSettings->value("UseHttpServer", true).toBool();

            QString path = QCoreApplication::applicationDirPath();
            if (!masterPath.isEmpty()) {
                // 界面显示Master的JOB队列设置.
                leMaster->setText(masterPath);

                currMasterPath = masterPath;
            }


            if (!enginePath.isEmpty()) {
                // 界面显示Engine的JOB队列设置.
                leEngine->setText(enginePath);
                currEnginePath = enginePath;
            }

        }
    }
    else
    {
        QFrame* frameMaster = new QFrame(this);
        frameMaster->setStyleSheet("background-color:#3D3D3D;margin:10px 20px 0px 20px;border-radius:4px;");

        QVBoxLayout* vLayoutMaster = new QVBoxLayout();

        QHBoxLayout* hLayout10 = new QHBoxLayout();



        QHBoxLayout* hLayout11 = new QHBoxLayout();

        //  hLayout11->setSpacing(0);
          //hLayout11->setAlignment(Qt::AlignVCenter);
          //hLayout11->addStretch(1);

        QLabel* lblMasterJobQueue = new QLabel(this);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblMasterJobQueue->setText("工作队列路径");
        }
        else
        {
            lblMasterJobQueue->setText("Job queue path");
        }
        lblMasterJobQueue->setStyleSheet("color:#A5A5A5;margin:0px 16px 0px 0px;font: 14px 'Arial';letter-spacing: 0;");

        hLayout11->addWidget(lblMasterJobQueue, 0,/* Qt::AlignRight | */Qt::AlignVCenter);
        //hLayout11->addWidget(lblMasterJobQueue, 0, Qt::AlignLeft);
        leMaster = new QLineEdit(this);
        leMaster->setText("");
        leMaster->setReadOnly(true);



        leMaster->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:234px;height:30px;border:none;border-top-left-radius:4px;border-top-right-radius:0px;border-bottom-right-radius:0px;border-bottom-left-radius:4px;font: 14px 'Arial';letter-spacing: 0;");

        hLayout11->addWidget(leMaster, 0, /*Qt::AlignRight | */Qt::AlignVCenter);

        butPathMaster = new QPushButton(this);
        butPathMaster->setText("...");

        butPathMaster->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;margin:0px;width:30px;height:30px;border:1px #3D3D3D solid;border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-right-radius:4px;border-bottom-left-radius:0px;");
        hLayout11->addWidget(butPathMaster, 0, /*Qt::AlignRight |*/ Qt::AlignVCenter);

        vLayoutMaster->addLayout(hLayout10);
        vLayoutMaster->addLayout(hLayout11);

        frameMaster->setLayout(vLayoutMaster);



        QHBoxLayout* hLayout30 = new QHBoxLayout();
        hLayout30->addStretch(1);

        butUpdate = new QPushButton(this);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            butUpdate->setText("确定");
        }
        else
        {
            butUpdate->setText("OK");
        }
        butUpdate->setStyleSheet("background-color:#538CCF;color:white;width:140px;height:28px;margin:20px 0px 20px 20px;border-radius:4px;font: 12px 'Arial';letter-spacing: 0;");
        hLayout30->addWidget(butUpdate, 0, Qt::AlignRight);


        butClose = new QPushButton(this);
        butClose->setStyleSheet("background-color:#545454;color:white;width:140px;height:28px;margin:20px 20px 20px 10px;border-radius:4px;font: 12px 'Arial';letter-spacing: 0;");
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            butClose->setText("关闭");
        }
        else
        {
            butClose->setText("Close");
        }
        hLayout30->addWidget(butClose, 0, Qt::AlignRight);

        vlayout->addWidget(frameMaster, 1);


        vlayout->addLayout(hLayout30);

        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
        shadowEffect->setOffset(0, 1);
        //shadowEffect->setColor(Qt::black);
        shadowEffect->setColor(QColor(0, 0, 0, 128));
        shadowEffect->setBlurRadius(1);

        butUpdate->setGraphicsEffect(shadowEffect);
        butClose->setGraphicsEffect(shadowEffect);

        topFrame->setLayout(vlayout);
        vlayoutTop->addWidget(topFrame);

        setLayout(vlayoutTop);

        connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));
        connect(butUpdate, SIGNAL(clicked()), this, SLOT(funcUpdate()));

        connect(butPathMaster, SIGNAL(clicked()), this, SLOT(funcPathMaster()));


        if (Settings::pSettings) {
            // 从系统注册表获取JOB队列已有设置.
            // master: Master端访问的JOB队列路径.
            // engine: Engine端访问的JOB队列路径.
            QString masterPath = Settings::pSettings->value("master").toString();


            //bHttpServerMode = pSettings->value("UseHttpServer", true).toBool();

            QString path = QCoreApplication::applicationDirPath();
            if (!masterPath.isEmpty()) {
                // 界面显示Master的JOB队列设置.
                leMaster->setText(masterPath);

                currMasterPath = masterPath;
            }




        }
    }

}

SettingsWgt::~SettingsWgt()
{

}

void SettingsWgt::funcUpdate()
{
    if (AI3D::CORE::Application::Getinstance().GetDistribution() > 0)
    {
        QString strMaster = leMaster->text();
        QString strEngine = leEngine->text();


        bool bUpdateValue = false;

        // 向系统注册表写入新的Master JOB队列设置.
        if (!strMaster.isEmpty() && strMaster != currMasterPath) {
            bUpdateValue = true;
            Settings::pSettings->setValue("master", strMaster);
            currMasterPath = strMaster;
        }

        // 向系统注册表写入新的Engine JOB队列设置.
        if (!strEngine.isEmpty() && strEngine != currEnginePath) {
            bUpdateValue = true;
            Settings::pSettings->setValue("engine", strEngine);
            currEnginePath = strEngine;
        }



        if (bUpdateValue)
        {
            emit SettingsChanged();
            bHasUpdated = true;

        }

        close();
    }
    else
    {
        QString strMaster = leMaster->text();



        bool bUpdateValue = false;

        // 向系统注册表写入新的Master JOB队列设置.
        if (!strMaster.isEmpty() && strMaster != currMasterPath) {
            bUpdateValue = true;
            Settings::pSettings->setValue("master", strMaster);
            currMasterPath = strMaster;
        }

        // 向系统注册表写入新的Engine JOB队列设置.
        if (!strMaster.isEmpty() && strMaster != currEnginePath) {
            bUpdateValue = true;
            Settings::pSettings->setValue("engine", strMaster);
            currEnginePath = strMaster;
        }



        if (bUpdateValue)
        {
            emit SettingsChanged();
            bHasUpdated = true;

        }

        close();
    }

}

void SettingsWgt::funcPathMaster()
{
    QString strPath;
    QString strMaster_ = leMaster->text();

    if (!strMaster_.isEmpty())
        strPath = QFileDialog::getExistingDirectory(nullptr, "", strMaster_);
    else
        strPath = QFileDialog::getExistingDirectory();
    if (!strPath.isEmpty())
    {
        leMaster->setText(strPath);
        leMaster->setToolTip(leMaster->text());
    }
}

void SettingsWgt::funcPathEngine()
{
    QString strPath;
    QString strEngine_ = leEngine->text();

    if (!strEngine_.isEmpty())
        strPath = QFileDialog::getExistingDirectory(nullptr, "", strEngine_);
    else
        strPath = QFileDialog::getExistingDirectory();

    if (!strPath.isEmpty()) {
        leEngine->setText(strPath);
        leEngine->setToolTip(leEngine->text());
    }
}


bool SettingsWgt::isFenbushi()
{
    if (!Settings::pSettings)
        return false;

    // 从系统注册表中获取否使用旧版HTTP版Engine(过时)
    return !Settings::pSettings->value("UseHttpServer", true).toBool();
}

void SettingsWgt::closeEvent(QCloseEvent* evt)
{

    emit SettingsClosed();
    evt->accept();


    if (!bCloseButtonClicked)
    {
        // 设置新的Master/Engine JOB队列路径后,关闭Setting对话框时打开信息提示对话框,提示需要重启Master与Engine,
        // 以使新设置的Master/Engine JOB队列路径生效.
        OpenSettingsPrompt();
    }
}

void SettingsWgt::funcClose() {
    //delete this;
    bCloseButtonClicked = true;
    close();
}

// JOB队列设置完成后,提示用户需重新运行Master/Engine,以使新设置生效.
SettingsPrompt::SettingsPrompt(QWidget* parent)
    : RoundWidget(parent)
{
    //QVBoxLayout* vlayoutTop = new QVBoxLayout(this);
    QVBoxLayout* vlayoutTop = new QVBoxLayout();
    vlayoutTop->setMargin(0);

    //vlayout = new QVBoxLayout(this);
    vlayout = new QVBoxLayout();
    vlayout->setMargin(0);

    QFrame* topFrame = new QFrame(this);

    //topFrame->setStyleSheet("background-color:#303030FF;margin:10px 13px 10px 0px;color:#CBCBCB;");
    topFrame->setStyleSheet("background-color:#303030;margin:0px;");

    QFrame* titleFrame = new QFrame(this);
    //titleFrame->setStyleSheet("background-color:#D8D8D8;margin:0px;");
    titleFrame->setStyleSheet("background-color:#303030;margin:0px;");

    QHBoxLayout* hlayout0 = new QHBoxLayout();
    hlayout0->setMargin(0);

    hlayout0->addStretch(1);

    QFont font;

    //QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
    QPixmap closePix(":/new/prefix1/skinbutton/sclose.png");

    butClose = new QPushButton(this);
    //butClose->setText("X");
    butClose->setIcon(closePix);

    //font.setPixelSize(40);
    //butClose->setFont(font);

    //butClose->setFixedSize(24, 24);   
    //butClose->setStyleSheet("border:none;background-color:rgba(0,0,0,0.3);color:white;margin:10px;");
    //butClose->setStyleSheet("background-color:#CBCBCB;margin:10px;");
    butClose->setStyleSheet("background-color:#000000;border-radius:2px;margin-right:10px;");

    hlayout0->addWidget(butClose, 0, Qt::AlignRight);
    titleFrame->setLayout(hlayout0);

    //vlayout->addLayout(hlayout0, 0);
    vlayout->addWidget(titleFrame, 0);

    lblPrompt = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblPrompt->setText("复位引擎工作队列路径后，请重启引擎（MoldAINode.exe）！");
    }
    else
    {
        lblPrompt->setText("Please restart the Moengine.exe after resetting the engine job path !");
    }

    //lblPrompt->setFont(QFont());
    lblPrompt->setStyleSheet("margin-top:8px;color:white;");

    vlayout->addWidget(lblPrompt, 0, Qt::AlignHCenter);

    lblCloseTime = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblCloseTime->setText("关闭中 3秒...");
    }
    else
    {
        lblCloseTime->setText("Closed in 3s...");
    }

    lblCloseTime->setStyleSheet("margin-bottom:37px;color:#A5A5A5;");

    vlayout->addWidget(lblCloseTime, 0, Qt::AlignHCenter);

    topFrame->setLayout(vlayout);
    vlayoutTop->addWidget(topFrame);

    setLayout(vlayoutTop);

    connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));

    pTimer = new QTimer(this);

    connect(pTimer, SIGNAL(timeout()), this, SLOT(funcTimeout()));

    pTimer->start(200);

    pTime = new QTime();
    pTime->start();
}

SettingsPrompt::~SettingsPrompt()
{

}

void SettingsPrompt::funcTimeout()
{
    int leftMsec = pTime->elapsed();
    // 从打开信息提示框时,实时显示将自动关闭信息提示窗口的剩余时间(3秒,2秒到1秒),
    // 大约3秒超时时间到时自动关闭当前信息提示窗口.
    if (leftMsec >= 3000)
    {
        close();
    }
    else if (leftMsec >= 2000)
    {
        if (BlockObject::isChineseVersion())
        {
            lblCloseTime->setText("关闭中 1秒...");
        }
        else
        {
            lblCloseTime->setText("Closed in 1s...");
        }
    }
    else if (leftMsec >= 1000)
    {
        if (BlockObject::isChineseVersion())
        {
            lblCloseTime->setText("关闭中 2秒...");
        }
        else
        {
            lblCloseTime->setText("Closed in 2s...");
        }
    }
    else
    {
        if (BlockObject::isChineseVersion())
        {
            lblCloseTime->setText("关闭中 3秒...");
        }
        else
        {
            lblCloseTime->setText("Closed in 3s...");
        }
    }
}

void SettingsPrompt::funcClose()
{
    close();
}


void SettingsPrompt::closeEvent(QCloseEvent* evt)
{
    evt->accept();


}

double generateRandDouble(int minInt, int maxInt)
{
    int diff = abs(maxInt - minInt);
    if (diff == 0)
        diff = 100;

    int m1 = qrand() % diff;

    double m2 = qrand() / 10000000.0;
    double retval = m1 + m2;

    return retval;
}

static QMap<QString, QPixmap*> iconPixmapMap;

// 打开JOB列设置界面(实际使用时,直接使用设置窗体对象创建,而不需调用该便利函数)

//void OpenSettings()
//{
//  SettingsWgt*pSettingsWindow = new SettingsWgt(nullptr);
//
//  pSettingsWindow->setWindowModality(Qt::ApplicationModal); 
//  pSettingsWindow->setAttribute(Qt::WA_DeleteOnClose);    
//
//  pSettingsWindow->resize(480, 230);
//  pSettingsWindow->show();
//}

// 打开JOB对列设置后的信息提示框(提示需重新运行Master/Engine)
void OpenSettingsPrompt()
{
    SettingsPrompt* pSettingsPrompt = new SettingsPrompt(nullptr);

    pSettingsPrompt->setWindowModality(Qt::ApplicationModal);
    pSettingsPrompt->setAttribute(Qt::WA_DeleteOnClose);

    pSettingsPrompt->resize(682, 161);
    pSettingsPrompt->show();
}
//?chy
LoadingPrompt::LoadingPrompt(QWidget* parent)
    : RoundWidget(parent)
{
    //QVBoxLayout* vlayoutTop = new QVBoxLayout(this);
    QVBoxLayout* vlayoutTop = new QVBoxLayout();
    vlayoutTop->setMargin(0);

    //vlayout = new QVBoxLayout(this);
    vlayout = new QVBoxLayout();
    vlayout->setMargin(0);

    QFrame* topFrame = new QFrame(this);

    //topFrame->setStyleSheet("background-color:#303030FF;margin:10px 13px 10px 0px;color:#CBCBCB;");
    topFrame->setStyleSheet("background-color:#303030;margin:0px;");



    lblPrompt = new QLabel(this);
    /// lblPrompt->setText("Please restart the Moengine.exe after resetting the engine job path !");
    lblPrompt->setText("Please wait to load completed!");
    //lblPrompt->setFont(QFont());
    lblPrompt->setStyleSheet("margin-top:8px;color:white;");

    vlayout->addWidget(lblPrompt, 0, Qt::AlignHCenter);

    lblCloseTime = new QLabel(this);
    lblCloseTime->setText("Loading in 0 s...");
    lblCloseTime->setStyleSheet("margin-bottom:37px;color:#A5A5A5;");

    vlayout->addWidget(lblCloseTime, 0, Qt::AlignHCenter);

    topFrame->setLayout(vlayout);
    vlayoutTop->addWidget(topFrame);

    setLayout(vlayoutTop);

    ///connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));

    pTimer = new QTimer(this);

    connect(pTimer, SIGNAL(timeout()), this, SLOT(funcTimeout()));

    pTimer->start(200);

    pTime = new QTime();
    pTime->start();
}

LoadingPrompt::~LoadingPrompt()
{

}

void LoadingPrompt::funcTimeout()
{
    int leftMsec = pTime->elapsed();
    int leftSec = leftMsec / 1000;


    int percent = 0;

    if (iTotalImagetoProcess > 0)
        percent = (int)(iCurrentImagetoProcess * 100.0 / iTotalImagetoProcess);

    ///lblCloseTime->setText(QString("Loading in %1 s...").arg(leftSec));
    lblCloseTime->setText(QString("Loading %1% (%2 / %3)...").arg(percent).arg(iCurrentImagetoProcess).arg(iTotalImagetoProcess));
}

void LoadingPrompt::funcClose()
{
    close();
}


void LoadingPrompt::closeEvent(QCloseEvent* evt)
{
    evt->accept();
}

LoadingPrompt* pLoadingPrompt = nullptr;

void OpenLoadingPrompt()
{
    if (pLoadingPrompt != nullptr)
        return;

    pLoadingPrompt = new LoadingPrompt(nullptr);

    pLoadingPrompt->setWindowModality(Qt::ApplicationModal);
    pLoadingPrompt->setAttribute(Qt::WA_DeleteOnClose);

    ///pLoadingPrompt->resize(682, 161);
    pLoadingPrompt->resize(450, 60);
    pLoadingPrompt->show();
}

void CloseLoadingPrompt()
{
    bNeedLoadingPrompt = false;

    if (pLoadingPrompt == nullptr)
        return;

    pLoadingPrompt->close();
    pLoadingPrompt = nullptr;
}

LoadingPromptV2::LoadingPromptV2(QWidget* parent, QString strInformation)
    : RoundWidget(parent), strDefault(strInformation)
{
    //QVBoxLayout* vlayoutTop = new QVBoxLayout(this);
    QVBoxLayout* vlayoutTop = new QVBoxLayout();
    vlayoutTop->setMargin(0);

    //vlayout = new QVBoxLayout(this);
    vlayout = new QVBoxLayout();
    vlayout->setMargin(0);

    QFrame* topFrame = new QFrame(this);

    //topFrame->setStyleSheet("background-color:#303030FF;margin:10px 13px 10px 0px;color:#CBCBCB;");
    topFrame->setStyleSheet("background-color:#303030;margin:0px;");



    lblPrompt = new QLabel(this);
    /// lblPrompt->setText("Please restart the Moengine.exe after resetting the engine job path !");
    ///lblPrompt->setText("Please wait to load completed!");
    lblPrompt->setText("Save Block");
    //lblPrompt->setFont(QFont());
    lblPrompt->setStyleSheet("margin-top:8px;color:white;");

    vlayout->addWidget(lblPrompt, 0, Qt::AlignHCenter);

    lblCloseTime = new QLabel(this);
    ///lblCloseTime->setText("Loading in 0 s...");
    lblCloseTime->setText(strDefault);
    lblCloseTime->setStyleSheet("margin-bottom:37px;color:#A5A5A5;");

    vlayout->addWidget(lblCloseTime, 0, Qt::AlignHCenter);

    topFrame->setLayout(vlayout);
    vlayoutTop->addWidget(topFrame);

    setLayout(vlayoutTop);

    ///connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));

    ///pTimer = new QTimer(this);

    ///connect(pTimer, SIGNAL(timeout()), this, SLOT(funcTimeout()));

    ///pTimer->start(200);

    ///pTime = new QTime();
    ///pTime->start();
}

LoadingPromptV2::~LoadingPromptV2()
{

}

void LoadingPromptV2::funcTimeout()
{
    //int leftMsec = pTime->elapsed();
    //int leftSec = leftMsec / 1000;

    //int percent = 0;

    //if (iTotalImagetoProcess > 0)
    //  percent = (int)(iCurrentImagetoProcess * 100.0 / iTotalImagetoProcess);

    ////lblCloseTime->setText(QString("Loading in %1 s...").arg(leftSec));
    //lblCloseTime->setText(QString("Loading %1% (%2 / %3)...").arg(percent).arg(iCurrentImagetoProcess).arg(iTotalImagetoProcess));
}

void LoadingPromptV2::funcClose()
{
    close();
}


void LoadingPromptV2::closeEvent(QCloseEvent* evt)
{
    evt->accept();
}

LoadingPromptV2* pLoadingPromptV2 = nullptr;

void OpenLoadingPromptV2(QString strInformation)
{
    if (pLoadingPromptV2 != nullptr)
        return;

    pLoadingPromptV2 = new LoadingPromptV2(nullptr, strInformation);

    pLoadingPromptV2->setWindowModality(Qt::ApplicationModal);
    pLoadingPromptV2->setAttribute(Qt::WA_DeleteOnClose);

    ///pLoadingPromptV2->resize(682, 161);
    pLoadingPromptV2->resize(450, 60);
    pLoadingPromptV2->show();
    bNeedLoadingPromptV2 = true;
}

void CloseLoadingPromptV2()
{
    bNeedLoadingPromptV2 = false;

    if (pLoadingPromptV2 == nullptr)
        return;

    pLoadingPromptV2->close();
    pLoadingPromptV2 = nullptr;
}

LoadingPromptV4::LoadingPromptV4(QWidget* parent, QString strInformation)
    : RoundWidget(parent), strDefault(strInformation)
{
    //QVBoxLayout* vlayoutTop = new QVBoxLayout(this);
    QVBoxLayout* vlayoutTop = new QVBoxLayout();
    vlayoutTop->setMargin(0);

    //vlayout = new QVBoxLayout(this);
    vlayout = new QVBoxLayout();
    vlayout->setMargin(0);

    QFrame* topFrame = new QFrame(this);

    //topFrame->setStyleSheet("background-color:#303030FF;margin:10px 13px 10px 0px;color:#CBCBCB;");
    topFrame->setStyleSheet("background-color:#303030;margin:0px;");

    lblPrompt = new QLabel(this);
    /// lblPrompt->setText("Please restart the Moengine.exe after resetting the engine job path !");
    ///lblPrompt->setText("Please wait to load completed!");
    lblPrompt->setText("");
    //lblPrompt->setFont(QFont());
    lblPrompt->setStyleSheet("margin-top:8px;color:white;");

    vlayout->addWidget(lblPrompt, 0, Qt::AlignHCenter);

    lblCloseTime = new QLabel(this);
    ///lblCloseTime->setText("Loading in 0 s...");
    lblCloseTime->setText(strDefault);
    lblCloseTime->setStyleSheet("padding-left:60px;margin-bottom:37px;color:#A5A5A5;font:15px \"Arial\"");
    lblCloseTime->setAlignment(Qt::AlignLeft);

    //vlayout->addWidget(lblCloseTime, 0, Qt::AlignHCenter);
    vlayout->addWidget(lblCloseTime, 0, Qt::AlignLeft);

    topFrame->setLayout(vlayout);
    vlayoutTop->addWidget(topFrame);

    setLayout(vlayoutTop);

    ///connect(butClose, SIGNAL(clicked()), this, SLOT(funcClose()));

    pTimer = new QTimer(this);

    connect(pTimer, SIGNAL(timeout()), this, SLOT(funcTimeout()));

    pTimer->start(500);

    iState = 0;
    ///pTime = new QTime();
    ///pTime->start();
}

LoadingPromptV4::~LoadingPromptV4()
{

}

void LoadingPromptV4::funcTimeout()
{
    //int leftMsec = pTime->elapsed();
    //int leftSec = leftMsec / 1000;

    //int percent = 0;

    //if (iTotalImagetoProcess > 0)
    //  percent = (int)(iCurrentImagetoProcess * 100.0 / iTotalImagetoProcess);

    ////lblCloseTime->setText(QString("Loading in %1 s...").arg(leftSec));
    //lblCloseTime->setText(QString("Loading %1% (%2 / %3)...").arg(percent).arg(iCurrentImagetoProcess).arg(iTotalImagetoProcess));

    //std::cout << iState << " | " << lblCloseTime->text().toStdString() << std::endl;

    /*if (BlockObject::isChineseVersion())
    {
        if (iState == 0)
            lblCloseTime->setText(strDefault + " .");
        else if (iState == 1)
            lblCloseTime->setText(strDefault + " ..");
        else
            lblCloseTime->setText(strDefault + " ...");
    }
    else*/
    {
        if (iState == 0)
            lblCloseTime->setText(strDefault + " .");
        else if (iState == 1)
            lblCloseTime->setText(strDefault + " . .");
        else
            lblCloseTime->setText(strDefault + " . . .");
    }

    //std::cout << iState << " || " << lblCloseTime->text().toStdString() << std::endl;

    iState++;
    if (iState == 3)
        iState = 0;
}

void LoadingPromptV4::funcClose()
{
    close();
}


void LoadingPromptV4::closeEvent(QCloseEvent* evt)
{
    evt->accept();
}

LoadingPromptV4* pLoadingPromptV4 = nullptr;

void OpenLoadingPromptV4(QString strInformation)
{
    if (pLoadingPromptV4 != nullptr)
        return;

    pLoadingPromptV4 = new LoadingPromptV4(nullptr, strInformation);

    pLoadingPromptV4->setWindowModality(Qt::ApplicationModal);
    pLoadingPromptV4->setAttribute(Qt::WA_DeleteOnClose);

    ///pLoadingPromptV2->resize(682, 161);
    pLoadingPromptV4->resize(450, 60);
    pLoadingPromptV4->show();
    bNeedLoadingPromptV4 = true;
}

void CloseLoadingPromptV4()
{
    bNeedLoadingPromptV4 = false;

    if (pLoadingPromptV4 == nullptr)
        return;

    pLoadingPromptV4->close();
    pLoadingPromptV4 = nullptr;
}

MoreSettings::MoreSettings(QWidget* parent, AI3D::CORE::ReconstructionObject* recons_object_, bool bReadOnly)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    this->recons_object_ = recons_object_;
    this->bReadOnly = bReadOnly;

    QVBoxLayout* vlMain = new QVBoxLayout();
    vlMain->setContentsMargins(0, 0, 0, 0);

    QFrame* frameTop = new QFrame(this);
    frameTop->setObjectName("#frameTop");
    frameTop->setStyleSheet(
        "QFrame {\n"
        "background-color:#323232;border-radius:8px;border:2px solid #687278;"
        "}\n"
    );

    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* hlTitle = new QHBoxLayout();
    hlTitle->setContentsMargins(20, 20, 20, 0);

    QLabel* lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblTitle->setText("更多设置");
    }
    else
    {
        lblTitle->setText("More Settings");
    }

    lblTitle->setAlignment(Qt::AlignLeft);
    lblTitle->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";border:none;");

    QFont font = lblTitle->font();
    font.setPixelSize(14);
    lblTitle->setFont(font);

    butClose = new QPushButton(this);
    butClose->setIcon(QPixmap(":/new/prefix1/skin/closeicon26.png"));
    butClose->setStyleSheet("background-color:transparent;color:white;");

    hlTitle->addWidget(lblTitle);
    hlTitle->addStretch(1);
    hlTitle->addWidget(butClose);

    QFrame* lineBelowTitle = new QFrame(this);
    lineBelowTitle->setFrameShape(QFrame::HLine);
    lineBelowTitle->setFrameShadow(QFrame::Plain);
    lineBelowTitle->setStyleSheet("border:none;background-color:rgb(91,91,91);max-height:1px;padding:0px;margin:0px;margin-left:0px;margin-right:0px;");

    QHBoxLayout* hlProcessingSettings = new QHBoxLayout();
    hlProcessingSettings->setContentsMargins(20, 0, 20, 0);

    QLabel* lblProcessingSettingsIcon = new QLabel(this);
    lblProcessingSettingsIcon->setPixmap(QPixmap(":/new/prefix1/skin/circle_nine.png"));
    lblProcessingSettingsIcon->setStyleSheet("background-color:transparent;border:none;width:6px;");

    QLabel* lblProcessingSettings = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblProcessingSettings->setText("处理设置");
    }
    else
    {
        lblProcessingSettings->setText("Processing Settings");
    }
    lblProcessingSettings->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";border:none;");
    lblProcessingSettings->setFont(font);

    hlProcessingSettings->addSpacing(14);
    hlProcessingSettings->setSpacing(10);
    hlProcessingSettings->addWidget(lblProcessingSettingsIcon);
    hlProcessingSettings->addWidget(lblProcessingSettings);
    hlProcessingSettings->addStretch(1);

    QHBoxLayout* hlGeometricPrecision = new QHBoxLayout();
    hlGeometricPrecision->setContentsMargins(20, 0, 20, 0);

    QLabel* lblGeometricPrecision = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblGeometricPrecision->setText("几何精度");
    }
    else
    {
        lblGeometricPrecision->setText("Geometric precision");
    }
    lblGeometricPrecision->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";border:none;");
    lblGeometricPrecision->setFont(font);
    lblGeometricPrecision->setFixedWidth(155);

    cbbGeometricPrecision = new QComboBox(this);

    if (BlockObject::isChineseVersion())
    {
        cbbGeometricPrecision->addItem("Extra");
        cbbGeometricPrecision->addItem("High");
        cbbGeometricPrecision->addItem("Medium");
        // note: add extra data for each item to store chinese information later.
    }
    else
    {
        cbbGeometricPrecision->addItem("Extra");
        cbbGeometricPrecision->addItem("High");
        cbbGeometricPrecision->addItem("Medium");
    }
    //cbbGeometricPrecision->addItem("Ultra");

    //cbbGeometricPrecision->setEnabled(false);
    cbbGeometricPrecision->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "   border: none;   \n"
        "   border-radius: 4px;   \n"
        "   color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   background-color:#34363A;\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 11px;\n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow {\n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png);\n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;\n"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";\n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160"
        ",160);   \n"
        "}\n"
        "\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
    ));
    cbbGeometricPrecision->setFixedHeight(33);
    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    cbbGeometricPrecision->setItemDelegate(itemDelegate);

    hlGeometricPrecision->addSpacing(30);
    hlGeometricPrecision->setSpacing(2);
    hlGeometricPrecision->addWidget(lblGeometricPrecision, 1);
    hlGeometricPrecision->addWidget(cbbGeometricPrecision, 2);
    hlGeometricPrecision->addSpacing(30);

    QHBoxLayout* hlHoleFilling = new QHBoxLayout();
    hlHoleFilling->setContentsMargins(20, 0, 20, 0);

    // @commented by chy in 20231124 算法暂时没有这个功能
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        QLabel* lblHoleFilling = new QLabel(this);
        if (BlockObject::isChineseVersion())
        {
            lblHoleFilling->setText("孔洞填充");
        }
        else
        {
            lblHoleFilling->setText("Hole-filling");
        }
        lblHoleFilling->setStyleSheet("background-color:transparent;color:white;border:none;");
        lblHoleFilling->setFont(font);
        lblHoleFilling->setFixedWidth(155);

        cbbHoleFilling = new QComboBox(this);
        if (BlockObject::isChineseVersion())
        {
            cbbHoleFilling->addItem("Fill small holes only");
            cbbHoleFilling->addItem("Fill all holes except at tile boundaries");
            // note: add extra data for each item to store chinese information later.
        }
        else
        {
            cbbHoleFilling->addItem("Fill small holes only");
            cbbHoleFilling->addItem("Fill all holes except at tile boundaries");
        }

        cbbHoleFilling->setStyleSheet(QString::fromUtf8("\n"
            "QComboBox {\n"
            "   border: none;   \n"
            "   border-radius: 4px;   \n"
            "   height:36px;\n"
            "   color: #FFFFFF;\n"
            "   font: 14px \"Arial\";\n"
            "   background-color:#34363A;\n"
            "   margin-left:0px; \n"
            "   margin-right:0px; \n"
            "   padding:0px;\n"
            "   padding-left: 11px;\n"
            "}\n"
            "QComboBox:disabled {\n"
            "   color: white;\n"
            "   background-color:gray;\n"
            "}\n"
            "QComboBox::drop-down { \n"
            "   subcontrol-position:top right;\n"
            "   subcontrol-origin:padding;\n"
            "   width:32px;\n"
            "   border:none;\n"
            "}\n"
            "QComboBox::down-arrow { \n"
            "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png);"
            "}\n"
            "\n"
            "QComboBox QAbstractItemView {\n"
            "    outline: 0px solid gray;   \n"
            "    border: none;   \n"
            "    color:#FFFFFF;\n"
            "    background-color: #131313;  \n"
            "    selection-background-color:#333333;   \n"
            "    padding-left: 0px; \n"
            "    margin-left:0px; \n"
            "    margin-right:0px; \n"
            "}\n"
            "QComboBox QAbstractScrollArea {\n"
            "    width: 10px;\n"
            "    color: black; \n"
            "    background-color:white;\n"
            "}\n"
            "\n"
            "QComboBox QAbstractItemView::item {\n"
            "    height: 38px;   \n"
            "    background-color:#3F4146;\n"
            "    color:#FFFFFF;"
            "    padding-left: 10px; \n"
            "    margin-left:0px; \n"
            "    margin-right:0px; \n"
            "    font:14px \"Arial\";"
            "}\n"
            "\n"
            "QComboBox QAbstractItemView::item:hover {\n"
            "    color: #FFFFFF;\n"
            "    background-color: #34363A;   \n"
            "}\n"
            "\n"
            "QComboBox QAbstractItemView::item:selected {\n"
            "    color: #FFFFFF;\n"
            "    background-color:#34363A;\n"
            "}\n"
            "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
            "    width: 10px;\n"
            "    background-color: #d0d2d4;  \n"
            "}\n"
            "\n"
            "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
            "    border-radius: 5px;   "
            "    background: rgb(160,160"
            ",160);   \n"
            "}\n"
            "\n"
            "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
            "    background: rgb(90, 91, 93);   \n"
            "}\n"
        ));

        cbbHoleFilling->setFixedHeight(33);
        QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
        cbbHoleFilling->setItemDelegate(itemDelegate);

        hlHoleFilling->addSpacing(30);
        hlHoleFilling->setSpacing(2);
        hlHoleFilling->addWidget(lblHoleFilling, 1);
        hlHoleFilling->addWidget(cbbHoleFilling, 2);
        hlHoleFilling->addSpacing(30);
    }

    QHBoxLayout* hlUntexturedRegions = new QHBoxLayout();
    hlUntexturedRegions->setContentsMargins(20, 0, 20, 0);

    QLabel* lblUntexturedRegions = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblUntexturedRegions->setText("无纹理区域填充模式");
    }
    else
    {
        lblUntexturedRegions->setText("Untextured Regions\nrepresentation");
    }
    lblUntexturedRegions->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";border:none;");
    lblUntexturedRegions->setFont(font);
    lblUntexturedRegions->setFixedWidth(155);

    cbbUntexturedRegions = new QComboBox(this);
    if (BlockObject::isChineseVersion())
    {
        cbbUntexturedRegions->addItem("Uniform color");
        cbbUntexturedRegions->addItem("Inpainting completion");
        // note: add extra data for each item to store chinese information later.
    }
    else
    {
        cbbUntexturedRegions->addItem("Uniform color");
        cbbUntexturedRegions->addItem("Inpainting completion");
    }

    cbbUntexturedRegions->setStyleSheet(QString::fromUtf8(
        "QComboBox { \n"
        "   border:none;\n"
        "   border-radius:4px;\n"
        "   color:#FFFFFF;\n"
        "   background-color:#34363A;\n"
        "   font:14px \"Arial\";\n"
        "   margin-left:0px;\n"
        "   margin-right:0px;\n"
        "   padding:0px;\n"
        "   padding-left:11px;\n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color:white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow {\n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png);"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "   outline:0px solid gray;\n"
        "   border:none;\n"
        "   color:#FFFFFF;\n"
        "   background-color:#131313;\n"
        "   selection-background-color:#333333;\n"
        "   padding-left:0px;\n"
        "   margin-left:0px;\n"
        "   margin-right:0px;\n"
        "}\n"
        "QComboBox QAbstractItemView::item{\n"
        "   height:38px;\n"
        "   background-color:#3F4146;"
        "   color:#FFFFFF;\n"
        "   padding-left:10px;\n"
        "   margin-left:0px;\n"
        "   margin-right:0px;\n"
        "   font:14px \"Arial\";\n"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "   color:#FFFFFF;\n"
        "   background-color:#34363A;\n"
        "}\n"
        "\n"
    ));

    QStyledItemDelegate* itemDelegate3 = new QStyledItemDelegate();
    cbbUntexturedRegions->setItemDelegate(itemDelegate3);
    cbbUntexturedRegions->setFixedHeight(33);

    hlUntexturedRegions->addSpacing(30);
    hlUntexturedRegions->setSpacing(2);
    hlUntexturedRegions->addWidget(lblUntexturedRegions, 1);
    hlUntexturedRegions->addWidget(cbbUntexturedRegions, 2);
    hlUntexturedRegions->addSpacing(30);

    QHBoxLayout* hlBottomFunction = new QHBoxLayout();
    hlBottomFunction->setContentsMargins(20, 0, 20, 20);

    butOk = new QPushButton(this);
    butCancel = new QPushButton(this);

    if (BlockObject::isChineseVersion())
    {
        butOk->setText("确定");
        butCancel->setText("取消");
    }
    else
    {
        butOk->setText("Ok");
        butCancel->setText("Cancel");
    }
    butOk->setFixedHeight(30);
    butCancel->setFixedHeight(30);
    butOk->setStyleSheet("QPushButton { background-color:#0172BE;color:white;border-radius:4px;font:14px \"Arial\";\n"
        "} QPushButton:pressed { background-color:#0161AD;position:relative; top:2px; left:2px;}\n"
        "QPushButton:disabled { background-color:#3F455C;}\n"
    );
    butCancel->setStyleSheet("QPushButton { background-color:#FFFFFF;color:#000000;border-radius:4px;font:14px \"Arial\";}\n"
        "QPushButton:pressed { background-color:#EEEEEE; color:#000000; top:2px; left:2px; }\n");

    connect(butClose, &QPushButton::clicked, this, &MoreSettings::Slot_Close);
    connect(butOk, &QPushButton::clicked, this, &MoreSettings::Slot_Ok);
    connect(butCancel, &QPushButton::clicked, this, &MoreSettings::Slot_Cancel);

    hlBottomFunction->addStretch(1);
    hlBottomFunction->addWidget(butOk, 2);
    hlBottomFunction->addWidget(butCancel, 2);
    hlBottomFunction->addStretch(1);

    vlTop->setSpacing(15);
    vlTop->addLayout(hlTitle);
    vlTop->addWidget(lineBelowTitle);
    vlTop->addLayout(hlProcessingSettings);
    vlTop->addLayout(hlGeometricPrecision);

    ///if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        vlTop->addLayout(hlHoleFilling);
    }

    vlTop->addLayout(hlUntexturedRegions);
    vlTop->addStretch(1);
    vlTop->addLayout(hlBottomFunction);

    frameTop->setLayout(vlTop);
    vlMain->addWidget(frameTop, 1);

    //setLayout(vlTop);
    setLayout(vlMain);

    if (this->recons_object_ != nullptr)
    {
        AI3D::CORE::processing_settings_s processing_settings;
        processing_settings = this->recons_object_->GetProcessingSettings();
        switch (processing_settings.level_)
        {
        case AI3D::CORE::geometric_level_e::GEO_LEVEL_EXTRA:
        default:
            cbbGeometricPrecision->setCurrentIndex(0);
            break;
        case AI3D::CORE::geometric_level_e::GEO_LEVEL_M:
            cbbGeometricPrecision->setCurrentIndex(2);
            break;
        case AI3D::CORE::geometric_level_e::GEO_LEVEL_H:
            cbbGeometricPrecision->setCurrentIndex(1);
            break;
            /*case AI3D::CORE::geometric_level_e::GEO_LEVEL_ULTRA:
                cbbGeometricPrecision->setCurrentIndex(3);
                break;*/
                //case AI3D::CORE::geometric_level_e::GEO_LEVEL_L:
                    // just ignore current option now,may need to modify it later for future use.
                //  break;
        }
        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            switch (processing_settings.hollfilling_)
            {
            case AI3D::CORE::holefilling_policy_e::HOLEFILL_SMALL:
            default:
                cbbHoleFilling->setCurrentIndex(0);
                break;

            case AI3D::CORE::holefilling_policy_e::HOLEFILL_ALL:
                cbbHoleFilling->setCurrentIndex(1);
                break;
            }
        }

        switch (processing_settings.untex_policy_)
        {
        case AI3D::CORE::untexture_policy_e::UNTEX_COLOR_INPAITING:
        default:
            cbbUntexturedRegions->setCurrentIndex(1);
            break;

        case AI3D::CORE::untexture_policy_e::UNTEX_COLOR_FILLED:
            cbbUntexturedRegions->setCurrentIndex(0);
            break;
        }

    }

    if (bReadOnly)
        butOk->setEnabled(false);

    setGeometricPrecisionExtra();
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        setHoleFillingExtra();
    }
    setUntexturedRegionsExtra();
}

MoreSettings::~MoreSettings()
{

}

void MoreSettings::setGeometricPrecisionExtra()
{
    cbbGeometricPrecision->setItemData(0, "超高精度", Qt::DisplayRole);
    cbbGeometricPrecision->setItemData(1, "高精度", Qt::DisplayRole);
    cbbGeometricPrecision->setItemData(2, "中等精度", Qt::DisplayRole);
}

void MoreSettings::setHoleFillingExtra()
{
    cbbHoleFilling->setItemData(0, "仅填充小型空洞", Qt::DisplayRole);
    cbbHoleFilling->setItemData(1, "填充非瓦片边界的所有空洞", Qt::DisplayRole);
}

void MoreSettings::setUntexturedRegionsExtra()
{
    cbbUntexturedRegions->setItemData(0, "单色填充", Qt::DisplayRole);
    cbbUntexturedRegions->setItemData(1, "自动修复", Qt::DisplayRole);
}

void MoreSettings::Slot_Ok()
{
    //  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    if (this->recons_object_ != nullptr)
    {
        AI3D::CORE::processing_settings_s processing_settings;
        processing_settings = this->recons_object_->GetProcessingSettings();

        switch (cbbGeometricPrecision->currentIndex())
        {
        case 1:
            processing_settings.level_ = AI3D::CORE::geometric_level_e::GEO_LEVEL_H;
            break;

        case 2:
            processing_settings.level_ = AI3D::CORE::geometric_level_e::GEO_LEVEL_M;
            break;

        case 0:
            processing_settings.level_ = AI3D::CORE::geometric_level_e::GEO_LEVEL_EXTRA;
            break;

            /*case 3:
                processing_settings.level_ = AI3D::CORE::geometric_level_e::GEO_LEVEL_ULTRA;
                break;*/

        default:
            break;
        }
        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            switch (cbbHoleFilling->currentIndex())
            {
            case 0:
                processing_settings.hollfilling_ = AI3D::CORE::holefilling_policy_e::HOLEFILL_SMALL;
                break;

            case 1:
                processing_settings.hollfilling_ = AI3D::CORE::holefilling_policy_e::HOLEFILL_ALL;
                break;

            default:
                break;
            }
        }
        switch (cbbUntexturedRegions->currentIndex())
        {
        case 0:
            processing_settings.untex_policy_ = AI3D::CORE::untexture_policy_e::UNTEX_COLOR_FILLED;
            break;

        case 1:
            processing_settings.untex_policy_ = AI3D::CORE::untexture_policy_e::UNTEX_COLOR_INPAITING;
            break;

        default:
            break;
        }

        this->recons_object_->SetProcessingSettings(processing_settings);
    }

    close();
}

void MoreSettings::Slot_Cancel()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    close();
}

void MoreSettings::Slot_Close()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    close();
}

void OpenMoreSettings(AI3D::CORE::ReconstructionObject* recons_object_, bool bReadOnly)
{
    ///if (pLoadingPromptV4 != nullptr)
    /// return;

    MoreSettings* pMoreSettings = new MoreSettings(nullptr, recons_object_, bReadOnly);

    pMoreSettings->setWindowModality(Qt::ApplicationModal);
    pMoreSettings->setAttribute(Qt::WA_DeleteOnClose);

    pMoreSettings->resize(500, 350);
    pMoreSettings->show();
}


ParamSettings4Production* pParamSettings4Production = nullptr;
AI3D::CORE::production_option_s ParamSettings4Production::saved_options_ = AI3D::CORE::production_option_s();
void DeleteTilesList();

ParamSettings4Production::ParamSettings4Production(AI3D::CORE::production_option_s options, QWidget* parent, QString  strTitle, AI3D::CORE::BlockObject* block_data_, AI3D::CORE::ReconstructionObject* recons_object_)
    : QDialog(parent)
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    this->strTitle = strTitle;
    this->iSelectedTabPos = 0;
    this->production_purpose = AI3D::CORE::production_purpose_e::EXPORT_3D_MESH;
    this->purpose_chosen_dirty = false;
    this->block_data_ = block_data_;
    this->recons_object = recons_object_;

    this->options_ = options;

    QVBoxLayout* vlMain = new QVBoxLayout();
    vlMain->setContentsMargins(0, 0, 0, 0);

    QFrame* frameTop = new QFrame(this);
    frameTop->setStyleSheet("QFrame { background-color:#2D3035;border-radius:8px;border:none;}");
    //frameTop->setStyleSheet("background-color:#2D3035;border-radius:8px;border:none;");

    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 0, 0, 0);
    vlTop->setSpacing(0);

    QWidget* titleWidget = new QWidget(this);
    titleWidget->setFixedHeight(68);
    titleWidget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* hlTitle = new QHBoxLayout();
    hlTitle->setSpacing(0);
    hlTitle->setContentsMargins(0, 0, 0, 0);

    lblTitle = new QLabel(this);
    //lblTitle->setText("Data Preprocess");
    lblTitle->setText(this->strTitle);
    lblTitle->setStyleSheet("background-color:transparent;color:white;margin-left:40px;font:20px \"Arial\";");
    //lblTitle->setStyleSheet("background-color:transparent;color:white;padding-left:40px;");

    QFont font = lblTitle->font();
    ///font.setPointSize(20);
    font.setPixelSize(20);

    lblTitle->setFont(font);

    ///font.setPointSize(16);
    font.setPixelSize(16);

    butNext = new QPushButton(this);
    if (BlockObject::isChineseVersion())
    {
        butNext->setText("下一步");
    }
    else
    {
        butNext->setText("Next");
    }
    butNext->setFont(font);
    butNext->setStyleSheet(
        "QPushButton { background-color:#1547F8;color:#E6FFFFFF;border:none;border-radius:6px;font:16px \"Arial\"; }"
        "QPushButton:pressed { background-color:#3F455C; }"
        "QPushButton:disabled { background-color:#3F455C; } "
    );
    //  butNext->setStyleSheet("background-color:#3F455C;color:#FFFFFF;border:none;border-radius:6px;");

    butCancel = new QPushButton(this);
    if (BlockObject::isChineseVersion()) {
        butCancel->setText("取消");
    }
    else {
        butCancel->setText("Cancel");
    }
    butCancel->setFont(font);
    //butCancel->setStyleSheet("background-color:#3F455C;color:#E6FFFFFF;border:none;border-radius:6px;");
    butCancel->setStyleSheet("background-color:#3F455C;color:#FFFFFF;border:none;border-radius:6px;font:16px \"Arial\";");

    butClose = new QPushButton(this);
    ///butClose->setText("Close");
    butClose->setIcon(QPixmap(":/new/prefix1/skin/closeicon26.png"));
    butClose->setStyleSheet("background-color:transparent;color:white;margin-right:20px;");
    //butClose->setStyleSheet("background-color:transparent;color:white;padding-right:20px;");

    butClose->setFont(font);

    hlTitle->addWidget(lblTitle);
    hlTitle->addStretch(1);
    //hlTitle->addWidget(butNext);
    //hlTitle->addWidget(butOk);
    hlTitle->addWidget(butClose);

    titleWidget->setLayout(hlTitle);

    QFrame* topLine = new QFrame(this);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Shadow::Plain);
    //topLine->setStyleSheet("width:899px;height:1px;border-radius:0px;color:#3D434E;");
    topLine->setStyleSheet("width:899px;max-height:1px;border-radius:0px;border:none;background-color:#3D434E;");

    QFrame* bottomLine = new QFrame(this);
    bottomLine->setFrameShape(QFrame::HLine);
    bottomLine->setFrameShadow(QFrame::Shadow::Plain);
    //bottomLine->setStyleSheet("width:899px;height:1px;border-radius:0px;color:#3D434E;");
    bottomLine->setStyleSheet("width:899px;max-height:1px;border-radius:0px;background-color:#3D434E;");

    QWidget* bottomWidget = new QWidget(this);
    bottomWidget->setFixedHeight(74);

    QHBoxLayout* hlBottom = new QHBoxLayout();
    hlBottom->setContentsMargins(0, 0, 0, 0);

    butNext->setFixedWidth(120);
    butNext->setFixedHeight(46);
    butCancel->setFixedWidth(120);
    butCancel->setFixedHeight(46);

    hlBottom->setSpacing(20);
    hlBottom->addStretch(20);
    hlBottom->addWidget(butNext);
    hlBottom->addWidget(butCancel);
    hlBottom->addStretch(1);

    bottomWidget->setLayout(hlBottom);

    QWidget* middleWidget = new QWidget(this);
    QHBoxLayout* hlMiddle = new QHBoxLayout();
    hlMiddle->addSpacing(0);
    hlMiddle->setSpacing(0);
    hlMiddle->setContentsMargins(0, 0, 0, 0);

    QWidget* middleLeftWidget = new QWidget(this);
    middleLeftWidget->setContentsMargins(0, 0, 0, 0);
    middleLeftWidget->setFixedWidth(190);

    ///font.setPointSize(14);
    font.setPixelSize(14);

    QVBoxLayout* vlMiddleLeft = new QVBoxLayout(middleLeftWidget);
    vlMiddleLeft->setContentsMargins(0, 0, 0, 0);
    butBasicSettings = new QPushButton(this);
    butPurpose = new QPushButton(this);
    butFormatWithOptions = new QPushButton(this);
    butSpatialReferenceSystem = new QPushButton(this);
    butTilingRange = new QPushButton(this);

    if (BlockObject::isChineseVersion()) {
        butBasicSettings->setText("常规设置");
        butPurpose->setText("目的");
        butFormatWithOptions->setText("格式/选项");
        butSpatialReferenceSystem->setText("空间参考系统");
        butTilingRange->setText("分块范围");
    }
    else {
        butBasicSettings->setText("Basic Settings");
        butPurpose->setText("Purpose");
        butFormatWithOptions->setText("Format/Options");
        butSpatialReferenceSystem->setText("Spatial reference\nsystem");
        butTilingRange->setText("Tiling Range");
    }

    //butBasicSettings->setFont(font);
    //butPurpose->setFont(font);
    //butFormatWithOptions->setFont(font);
    //butSpatialReferenceSystem->setFont(font);
    //butTilingRange->setFont(font);

    butBasicSettings->setFixedWidth(190);
    butBasicSettings->setFixedHeight(55);
    butPurpose->setFixedWidth(190);
    butPurpose->setFixedHeight(55);
    butFormatWithOptions->setFixedWidth(190);
    butFormatWithOptions->setFixedHeight(55);
    butSpatialReferenceSystem->setFixedWidth(190);
    butSpatialReferenceSystem->setFixedHeight(55);
    butTilingRange->setFixedWidth(190);
    butTilingRange->setFixedHeight(55);

    ///font.setPointSize(14);
    butBasicSettings->setStyleSheet(QString::fromUtf8(
        "QPushButton { background-color:#1E2024;color:white;margin:0px;padding:0px;border:none;font:14px \"Arial\";}\n"
        "QPushButton:disabled { color:red; }\n"
    ));

    //butBasicSettings->setFont(font);

    //"QPushButton:disabled { color:#66B5BDCA;} \n"
    butPurpose->setStyleSheet(QString::fromUtf8(
        "QPushButton { background-color:#1E2024;color:white;margin:0px;padding:0px;border:none;font:14px \"Arial\";}\n"
        "QPushButton:disabled { color:red;} \n"
    ));
    //butPurpose->setFont(font);

    butFormatWithOptions->setStyleSheet("background-color:#1E2024;color:white;margin:0px;padding:0px;border:none;font:14px \"Arial\";");
    //butFormatWithOptions->setFont(font);

    butSpatialReferenceSystem->setStyleSheet("background-color:#1E2024;color:white;margin:0px;padding:0px;border:none;font:14px \"Arial\";");
    butSpatialReferenceSystem->setFont(font);

    butTilingRange->setStyleSheet("background-color:#1E2024;color:white;margin:0px;padding:0px;border:none;font:14px \"Arial\";");
    //butTilingRange->setFont(font);

    vlMiddleLeft->addSpacing(10);

    vlMiddleLeft->setSpacing(0);
    vlMiddleLeft->addWidget(butBasicSettings);
    vlMiddleLeft->addWidget(butPurpose);
    vlMiddleLeft->addWidget(butFormatWithOptions);
    vlMiddleLeft->addWidget(butSpatialReferenceSystem);
    vlMiddleLeft->addWidget(butTilingRange);

    vlMiddleLeft->addStretch(4);

    QFrame* middleLine = new QFrame(this);
    middleLine->setFrameShape(QFrame::Shape::VLine);
    middleLine->setFrameShadow(QFrame::Shadow::Plain);
    middleLine->setStyleSheet("max-width:1px;border:none;background-color:#3D434E;padding:0px;margin:0px;");

    //QWidget *middleRightWidget = new QWidget(this);
    basicSettings = new BasicSettings(this);
    {
        //构造里边无法传递相关数据，在此处赋初值
        basicSettings->name_ = options_.name_;
        basicSettings->desination_ = options_.destination_;

        basicSettings->production_id_ = options_.id_;
        basicSettings->Init();
    }
    basicSettings->setFixedWidth(709);

    purpose = new Purpose4ProductionDefinition(this);
    purpose->setFixedWidth(709);

    formatWithOptions = new FormatWithOptions(this);
    formatWithOptions->setFixedWidth(709);

    export3DMesh_FormatWithOptions = new Export3DMesh_FormatWithOptions(this);
    auto gsd = options_.avgresolution_;
    //构造里边无法传递相关数据，在此处赋初值
    double overlap = gsd * 10;

    overlap = std::round(overlap * 100) / 100;
    export3DMesh_FormatWithOptions->defaultTileOverlap_ = overlap;
    srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(options_.cs_.definition_);

    if (srs.type == LOCAL)
    {
        export3DMesh_FormatWithOptions->unit_ = 1;
    }
    if (options_.tiles_.size() > 1)
    {
        export3DMesh_FormatWithOptions->tileOverLap_ = export3DMesh_FormatWithOptions->defaultTileOverlap_;
    }
    export3DMesh_FormatWithOptions->setFixedWidth(709);
    //因为暂时算法不支持

    export3D_Point_Cloud = new Export3D_Point_Cloud(this);
    export3D_Point_Cloud->setFixedWidth(709);
    export3D_Point_Cloud->defaultSamplingDistance_ = gsd * pow(2, 0);//@chy @attention 此处源码指数为分辨率的 level-1;待参照cc

    exportOrthophoto_DSM = new ExportOrthophoto_DSM(this);
    exportOrthophoto_DSM->setFixedWidth(709);
    exportOrthophoto_DSM->default_sampling_distance_ = gsd * pow(2, 0);//@chy @attention 此处源码指数为分辨率的 level-1;待参照cc

    //@attenting retouching 暂时不需支持
    //export3DMesh4ExternalRetouching_FormatWithOptions = new Export3DMesh4ExternalRetouching_FormatWithOptions(this);
    //export3DMesh4ExternalRetouching_FormatWithOptions->setFixedWidth(709);

    export_PointCloud_GS = new Export_PointCloud_GS(this);
    export_PointCloud_GS->setFixedWidth(709);


    spatialReferenceSystem = new SpatialReferenceSystem(this);
    spatialReferenceSystem->setFixedWidth(709);
    spatialReferenceSystem->default_definition_ = options_.cs_.definition_;
    spatialReferenceSystem->definition_ = options_.cs_.definition_;
    // fill coor_origin_ from options.
    spatialReferenceSystem->coor_origin_ = options_.cs_.origin_;
    auto srstemp = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(this->recons_object->GetATData().GetLocalSrs());
    if (srstemp.type == LOCAL_ENU)
    {
        spatialReferenceSystem->srsdefinitionvec_.insert(this->recons_object->GetATData().GetLocalSrs());
    }
    tilingRange = new TilingRange(this, this);
    tilingRange->setFixedWidth(709);

    tilesList = nullptr;

    auto tilesset = this->recons_object->GetTilesName(this->recons_object->GetProcessingSettings().bdiscard_emptytiles_);

    tilingRange->tiles_selected_.assign(tilesset.begin(), tilesset.end());
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setFixedWidth(709);

    stackedWidget->addWidget(basicSettings);
    stackedWidget->addWidget(purpose);

    stackedWidget->addWidget(formatWithOptions);

    //formatWithOptions->setVisible(false);
    //export3D_Point_Cloud->setVisible(false);
    //export3DMesh4ExternalRetouching_FormatWithOptions->setVisible(false);
    //export3DMesh_FormatWithOptions->setVisible(false);

    stackedWidget->addWidget(spatialReferenceSystem);
    stackedWidget->addWidget(tilingRange);

    stackedWidget->addWidget(export3DMesh_FormatWithOptions);

    stackedWidget->addWidget(export3D_Point_Cloud);

    stackedWidget->addWidget(exportOrthophoto_DSM);

    stackedWidget->addWidget(export_PointCloud_GS);

    //@attenting retouching 暂时不需支持
    //stackedWidget->addWidget(export3DMesh4ExternalRetouching_FormatWithOptions);

    ///BasicSettings* middleRightWidget = new BasicSettings(this);
    ///QWidget* middleRightWidget = new QWidget(this);
    ///middleRightWidget->setFixedWidth(709);

    hlMiddle->addWidget(middleLeftWidget);
    hlMiddle->addWidget(middleLine);
    ///hlMiddle->addWidget(middleRightWidget);
    hlMiddle->addWidget(stackedWidget);

    middleWidget->setLayout(hlMiddle);

    //vlTop->addLayout(hlTitle);
    vlTop->addWidget(titleWidget);

    vlTop->addWidget(topLine);

    //vlTop->addStretch(1);
    vlTop->addWidget(middleWidget, 1);

    vlTop->addWidget(bottomLine);
    //  vlTop->addStretch(1);

    vlTop->addWidget(bottomWidget);

    //topLine->setLineWidth(1);

    connect(butClose, &QPushButton::clicked, this, &ParamSettings4Production::Slot_Close);

    connect(butNext, &QPushButton::clicked, this, &ParamSettings4Production::Slot_Next);
    connect(butCancel, &QPushButton::clicked, this, &ParamSettings4Production::Slot_Cancel);

    connect(butBasicSettings, &QPushButton::clicked, this, &ParamSettings4Production::Slot_BasicSettings);
    connect(butPurpose, &QPushButton::clicked, this, &ParamSettings4Production::Slot_Purpose);
    connect(butFormatWithOptions, &QPushButton::clicked, this, &ParamSettings4Production::Slot_FormatWithOptions);
    connect(butSpatialReferenceSystem, &QPushButton::clicked, this, &ParamSettings4Production::Slot_SpatialReferenceSystem);
    connect(butTilingRange, &QPushButton::clicked, this, &ParamSettings4Production::Slot_TilingRange);

    frameTop->setLayout(vlTop);

    vlMain->addWidget(frameTop, 1);

    stackedWidget->setCurrentIndex(0);

    setLayout(vlMain);

    DisplaySelectedTab();

    // disable all tab buttons on the left side except basic-settings at the first time.
    butPurpose->setEnabled(false);
    butFormatWithOptions->setEnabled(false);
    butSpatialReferenceSystem->setEnabled(false);
    butTilingRange->setEnabled(false);
}

ParamSettings4Production::~ParamSettings4Production()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (pParamSettings4Production != nullptr)
    {
        //  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        pParamSettings4Production = nullptr;
    }

    AI3D::GUI::Refresh3DViewOfConstructionWgt();
}

void ParamSettings4Production::closeEvent(QCloseEvent* event)
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    /*pParamSettings4Production = nullptr;*/
}


void ParamSettings4Production::Slot_Next()
{
    if (iSelectedTabPos < 0)
    {
        // keep it to avoid special situation,though it is impossible now.
        return;
    }

    /// note: notice that the title on this button if swithing to chinese version.
    if (butNext->text() == "Submit" || butNext->text() == "提交")
    {
        DoNextInsideTilingRange();
        return;
    }

    switch (iSelectedTabPos)
    {
    case 0:
        // inside Basic Settings
        DoNextInsideBasicSettings();
        break;

    case 1:
        // inside Purpose
        DoNextInsidePurpose();
        break;

    case 2:
        // inside Format/Options
        DoNextInsideFormatWithOptions();
        break;

    case 3:
        // inside Spatial Reference system
        DoNextInsideSpatialReferenceSystem();
        break;

    case 4:
        // inside Tiling Range
        DoNextInsideTilingRange();
        break;

    default:
        break;
    }

    if (iSelectedTabPos == 3)
    {
        // note:change it later to meet the newest business requirements.
        //done(1);
    }
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
}

void ParamSettings4Production::Slot_Cancel()
{
    //done(2);
    DeleteTilesList();
    emit signal_done_options(false);
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
}

void ParamSettings4Production::Slot_Close()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    DeleteTilesList();
    this->close();
    emit signal_done_options(false);
}

void ParamSettings4Production::Slot_BasicSettings()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    stackedWidget->setCurrentIndex(0);
    Slot_RefreshSelectedTab();
}

void ParamSettings4Production::Slot_Purpose()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    stackedWidget->setCurrentIndex(1);
    Slot_RefreshSelectedTab();
}

void ParamSettings4Production::Slot_FormatWithOptions()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    ///stackedWidget->setCurrentIndex(2);
    switch (production_purpose)
    {
    case AI3D::CORE::EXPORT_3D_MESH:
    {
        //则用之前的
        if (!export3DMesh_FormatWithOptions->IsValid())
        {
            export3DMesh_FormatWithOptions->DefaultParams();
        }
        export3DMesh_FormatWithOptions->Init();



        export3D_Point_Cloud->SetValid(false);
        exportOrthophoto_DSM->SetValid(false);
        //export3DMesh4ExternalRetouching_FormatWithOptions->format_ = AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;

        stackedWidget->setCurrentIndex(5);
        break;
    }
    case AI3D::CORE::EXPORT_3D_POINT_CLOUD:
    {

        if (!export3D_Point_Cloud->IsValid())
        {
            export3D_Point_Cloud->DefaultParams();
        }
        export3D_Point_Cloud->Init();
        export3DMesh_FormatWithOptions->format_ = AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;;
        exportOrthophoto_DSM->SetValid(false);
        //  export3DMesh4ExternalRetouching_FormatWithOptions->format_ = AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;
        export3D_Point_Cloud->SetValid(true);

        stackedWidget->setCurrentIndex(6);
        break;
    }
    case AI3D::CORE::EXPORT_ORTHOPHOTO_DSM:
    {
        if (!exportOrthophoto_DSM->IsValid())
        {
            exportOrthophoto_DSM->DefaultParams();
        }
        exportOrthophoto_DSM->Init();
        export3DMesh_FormatWithOptions->format_ = AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;
        exportOrthophoto_DSM->SetValid(true);
        //export3DMesh4ExternalRetouching_FormatWithOptions->format_ = AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;
        export3D_Point_Cloud->SetValid(false);

        stackedWidget->setCurrentIndex(7);
        break;
    }
    /*case AI3D::CORE::EXPORT_3D_MESH_FOR_EXTERNAL_RETOUCHING:
    {
        if (!export3DMesh4ExternalRetouching_FormatWithOptions->IsValid())
        {
            export3DMesh4ExternalRetouching_FormatWithOptions->DefaultParams();
        }
        export3DMesh4ExternalRetouching_FormatWithOptions->Init();
        export3DMesh_FormatWithOptions->format_ = AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;
        exportOrthophoto_DSM->SetValid(false);
        export3DMesh4ExternalRetouching_FormatWithOptions->format_ = "";
        export3D_Point_Cloud->SetValid(false);

        stackedWidget->setCurrentIndex(8);
        break;
    }*/
    case AI3D::CORE::EXPORT_POINTCLOUD_GDGS:
    {
        //则用之前的
        if (!export_PointCloud_GS->IsValid())
        {
            std::cout << "gauss param not valid." << std::endl;
            export_PointCloud_GS->DefaultParams();
        }
        export_PointCloud_GS->Init();
        //export_PointCloud_GS->scene_type_ = AI3D::CORE::gs_scene_e::GS_SCENE_FLY;

        export3D_Point_Cloud->SetValid(false);
        exportOrthophoto_DSM->SetValid(false);

        stackedWidget->setCurrentIndex(8);
        break;
    }
    default:
        break;
    }

    Slot_RefreshSelectedTab();
}

void ParamSettings4Production::Slot_SpatialReferenceSystem()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    stackedWidget->setCurrentIndex(3);
    Slot_RefreshSelectedTab();
}

void ParamSettings4Production::Slot_TilingRange()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    stackedWidget->setCurrentIndex(4);
    Slot_RefreshSelectedTab();
}

void ParamSettings4Production::DisplaySelectedTab()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    for (int i = 0; i < 5; i++)
    {
        if (i == 0)
        {
            if (iSelectedTabPos == i)
                butBasicSettings->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#1E2024;color:white;margin:0px;padding:0px;border:none;font:14px \"Arial\"; }"
                ));
            else
                butBasicSettings->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#2D3035;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\"; }"
                    "QPushButton:disabled { color:#66B5BDCA;} "
                ));
        }
        else if (i == 1)
        {
            if (iSelectedTabPos == i)
                butPurpose->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#1E2024;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";}"
                ));
            else
                butPurpose->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#2D3035;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";}"
                    "QPushButton:disabled { color:#66B5BDCA; } "
                ));
        }
        else if (i == 2)
        {
            if (iSelectedTabPos == i)
                butFormatWithOptions->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#1E2024;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";} "
                ));
            else
                butFormatWithOptions->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#2D3035;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";}"
                    "QPushButton:disabled { color:#66B5BDCA; }"
                ));
        }
        else if (i == 3)
        {
            if (iSelectedTabPos == i)
                butSpatialReferenceSystem->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#1E2024;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";}"
                ));
            else
                butSpatialReferenceSystem->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#2D3035;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";}"
                    "QPushButton:disabled { color:#66B5BDCA; } "
                ));
        }
        else if (i == 4)
        {
            if (iSelectedTabPos == i)
                butTilingRange->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#1E2024;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";}"
                ));
            else
                butTilingRange->setStyleSheet(QString::fromUtf8(
                    "QPushButton { background-color:#2D3035;color:white;margin:0px;padding:0px;border:none; font:14px \"Arial\";}"
                    "QPushButton:disabled { color:#66B5BDCA; } "
                ));
        }
    }
}

void ParamSettings4Production::Slot_RefreshSelectedTab()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    QPushButton* butSender = static_cast<QPushButton*>(sender());
    if (butSender == nullptr)
        return;

    if (butSender == butBasicSettings)
    {
        //std::cout << "basic settings clicked." << std::endl;
        iSelectedTabPos = 0;
    }
    else if (butSender == butPurpose)
    {
        //std::cout << "purpose clicked." << std::endl;
        iSelectedTabPos = 1;
    }
    else if (butSender == butFormatWithOptions)
    {
        //std::cout << "format with options clicked." << std::endl;
        iSelectedTabPos = 2;
    }
    else if (butSender == butSpatialReferenceSystem)
    {
        //std::cout << "spatial reference system clicked." << std::endl;
        iSelectedTabPos = 3;
    }
    else if (butSender == butTilingRange)
    {
        //std::cout << "tiling range clicked." << std::endl;
        iSelectedTabPos = 4;
    }
    else  // add TilingRange switch button later.
    {
        //std::cout << "unknown click inside production definition." << std::endl;
        //iSelectedTabPos = -1;
    }

    DisplaySelectedTab();
}

void ParamSettings4Production::DoNextInsideBasicSettings()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    iSelectedTabPos = 1;
    DisplaySelectedTab();

    purpose->Init();
    stackedWidget->setCurrentIndex(1);
    butPurpose->setEnabled(true);
}

void ParamSettings4Production::DoNextInsidePurpose()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    iSelectedTabPos = 2;
    DisplaySelectedTab();

    Slot_FormatWithOptions();
    butFormatWithOptions->setEnabled(true);
}

// just for test purpose if setting to be false to skip detecting whether localSRS exists,remember to restore it to true inside release version.
static bool bAllowedToDetectLocalSRS = true;

void ParamSettings4Production::DoNextInsideFormatWithOptions()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    //note:!!! for test purpose only,remember to restore it after testing.
    ///if (block_data_ != nullptr && block_data_->GetBlockSRS().type == coord_system_type_e::LOCAL)
    if (bAllowedToDetectLocalSRS && block_data_ != nullptr && block_data_->GetBlockSRS().type == coord_system_type_e::LOCAL)
    {
        // skip srs window when being local srs.
        iSelectedTabPos = 4;
        DisplaySelectedTab();
        tilingRange->Init();
        stackedWidget->setCurrentIndex(4);
        butTilingRange->setEnabled(true);
        if (BlockObject::isChineseVersion())
            butNext->setText("提交");
        else
            butNext->setText("Submit");
    }
    else
    {
        iSelectedTabPos = 3;
        DisplaySelectedTab();

        /*if (export3DMesh_FormatWithOptions->format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_3DTILES)
        {
            spatialReferenceSystem->definition_ = BASESRS;
        }*/

        spatialReferenceSystem->Init();
        /*if (export3DMesh_FormatWithOptions->format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_3DTILES)
        {
            spatialReferenceSystem->SetSrsUnEditable();
        }*/
        stackedWidget->setCurrentIndex(3);
        butSpatialReferenceSystem->setEnabled(true);
    }
}

void ParamSettings4Production::Submit()
{
    //std::cout << "submitting inside param settings dialog." << std::endl;
    /// note: comment the following line inside release version.
    ///return;
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value jstr(rapidjson::kObjectType);
    if (production_purpose == AI3D::CORE::production_purpose_e::EXPORT_3D_MESH)
    {
        if (export3DMesh_FormatWithOptions->format_ != AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN)
        {
            options_.production_format_ = export3DMesh_FormatWithOptions->format_;
        }
        //jstr.AddMember("format", rapidjson::Value(export3DMesh_FormatWithOptions->format_.c_str(), allocator), allocator);
        if (export3DMesh_FormatWithOptions->withLod_)
        {
            jstr.AddMember("lod_type", rapidjson::Value((int)export3DMesh_FormatWithOptions->lodType_), allocator);
            int scopemode = export3DMesh_FormatWithOptions->withAcrossTile_ ? 1 : 0;
            jstr.AddMember("scope_mode", rapidjson::Value(scopemode), allocator);
        }
        if (export3DMesh_FormatWithOptions->withTexMaps_)
        {
            jstr.AddMember("include_tex_maps", rapidjson::Value(export3DMesh_FormatWithOptions->withTexMaps_), allocator);
        }

        jstr.AddMember("tex_compression", rapidjson::Value(export3DMesh_FormatWithOptions->texturecompression_), allocator);
        jstr.AddMember("max_tex_size", rapidjson::Value(export3DMesh_FormatWithOptions->max_texture_size_), allocator);
        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
        {
            if (export3DMesh_FormatWithOptions->withSkirt_)
            {
                jstr.AddMember("skirt_length", rapidjson::Value(export3DMesh_FormatWithOptions->skirtPix_), allocator);
            }
        }
        if (options_.tiles_.size() > 1)
        {
            if (export3DMesh_FormatWithOptions->tileOverLap_ >= 0)
            {
                jstr.AddMember("tileoverlap_in_metersorunit", rapidjson::Value(export3DMesh_FormatWithOptions->tileOverLap_), allocator);
            }
        }

        jstr.AddMember("texture_sharpening", rapidjson::Value(export3DMesh_FormatWithOptions->withSparping_), allocator);


    }
    else if (production_purpose == AI3D::CORE::production_purpose_e::EXPORT_3D_POINT_CLOUD)
    {
        ///export3D_Point_Cloud;
        if (export3D_Point_Cloud->format_ != AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN)
        {
            options_.production_format_ = export3D_Point_Cloud->format_;
            //jstr.AddMember("format", rapidjson::Value(export3D_Point_Cloud->format_.c_str(), allocator), allocator);
            jstr.AddMember("point_sampling_distance", rapidjson::Value(export3D_Point_Cloud->samplingDistance_), allocator);
            jstr.AddMember("point_sampling_unit", rapidjson::Value(export3D_Point_Cloud->samUnit_), allocator);
            jstr.AddMember("sampling_distance", rapidjson::Value(options_.avgresolution_), allocator);//此处是为了给算法端传数据
        }
    }
    /*else if (production_purpose == AI3D::CORE::production_purpose_e::EXPORT_3D_MESH_FOR_EXTERNAL_RETOUCHING)
    {
        options_.production_format_ = AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OBJFORRETOUCHING;
        if (export3DMesh4ExternalRetouching_FormatWithOptions->withTexMaps_)
        {
            jstr.AddMember("tex_compression", rapidjson::Value(export3DMesh4ExternalRetouching_FormatWithOptions->texturecompression_), allocator);
            jstr.AddMember("max_tex_size", rapidjson::Value(export3DMesh4ExternalRetouching_FormatWithOptions->max_texture_size_), allocator);
        }
        jstr.AddMember("texture_sharpening", rapidjson::Value(export3DMesh4ExternalRetouching_FormatWithOptions->withSparping_), allocator);
        ;
    }*/
    else if (production_purpose == AI3D::CORE::production_purpose_e::EXPORT_ORTHOPHOTO_DSM)
    {

        if (exportOrthophoto_DSM->tdommode_ == AI3D::CORE::tdom_mode_e::NORMAL)
        {
            options_.production_format_ = AI3D::CORE::production_format_e::PRODUCTION_4D_FORMAT_TDOMDSM;
        }
        else if (exportOrthophoto_DSM->tdommode_ == AI3D::CORE::tdom_mode_e::RAPIDMOSAIC)
        {
            options_.production_format_ = AI3D::CORE::production_format_e::PRODUCTION_4D_FORMAT_RAPIDTDOMDSM;
        }
        else if (exportOrthophoto_DSM->tdommode_ == AI3D::CORE::tdom_mode_e::LOW)
        {
            options_.production_format_ = AI3D::CORE::production_format_e::PRODUCTION_4D_FORMAT_MESHTDOMDSM;
        }
        /*else if (exportOrthophoto_DSM->tdommode_ == AI3D::CORE::tdom_mode_e::FASTMOSAIC)
        {
            options_.production_format_ = AI3D::CORE::production_format_e::PRODUCTION_4D_FORMAT_FASTMOSAIC;
        }*/
        jstr.AddMember("sampling_distance", rapidjson::Value(exportOrthophoto_DSM->sampling_distance_), allocator);
        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            jstr.AddMember("max_image_dim", rapidjson::Value(exportOrthophoto_DSM->max_image_dim_), allocator);
        }
        jstr.AddMember("with_tdom", rapidjson::Value(exportOrthophoto_DSM->withTDOM_), allocator);
        if (exportOrthophoto_DSM->withTDOM_)
        {
            jstr.AddMember("tdom_format", rapidjson::Value(exportOrthophoto_DSM->tdom_format_), allocator);
        }
        jstr.AddMember("with_dsm", rapidjson::Value(exportOrthophoto_DSM->withDSM_), allocator);
        if (exportOrthophoto_DSM->withDSM_)
        {
            jstr.AddMember("dsm_format", rapidjson::Value(exportOrthophoto_DSM->dsm_format_), allocator);
        }

    }
    else if (production_purpose == AI3D::CORE::production_purpose_e::EXPORT_POINTCLOUD_GDGS)
    {
        options_.production_format_ = AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_GDGS;

        //  if (export3D_Point_Cloud->format_ != AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN)
        {

            jstr.AddMember("scene", rapidjson::Value(export_PointCloud_GS->scene_type_), allocator);
            jstr.AddMember("gs_format", rapidjson::Value(export_PointCloud_GS->format_), allocator);
            jstr.AddMember("tile_mode", rapidjson::Value(recons_object->GetTilingDiscriptor()->GetParams().mode_), allocator);
            jstr.AddMember("tile_size", rapidjson::Value(recons_object->GetTilingDiscriptorMutual()->GetParamsMutual().regular_params_.tilesize_), allocator);


            block_t blockid = recons_object->GetBlockId();
            production_t productionId = GetOptions().id_;
            //std::string pid = "Block_" + std::to_string(blockid) + "-" + recons_object->GetIDString() + "-" + "-" + production->GetIDString();
            std::string pid = "Block_" + std::to_string(blockid) + "-" + recons_object->GetIDString() + "-" + "Prodcution_" + std::to_string(productionId);
            std::cout << "project id:----" << pid << std::endl;
            jstr.AddMember("pid", rapidjson::Value(pid.c_str(), allocator), allocator);
           
            /*std::string outDir = "Block_" + std::to_string(blockid) + "-" + recons_object->GetIDString() + "-" + PRODUCTION_DIR
                + "-" + production->GetIDString();
            jstr.AddMember("out_dir", rapidjson::Value(outDir.c_str(), allocator), allocator);*/
        }
    }
    srs_s  srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(spatialReferenceSystem->definition_);
    if (srs.type != LOCAL)
    {
        jstr.AddMember("srs_definition", rapidjson::Value(srs.definition.c_str(), allocator), allocator);
        options_.cs_.definition_ = srs.definition;
    }
    {
        rapidjson::Value item(rapidjson::kArrayType);
        if (srs.type != LOCAL)
        {
            Eigen::Vector3d coor_origin = Eigen::Vector3d{ 0.0,0.0,0.0 };
            if (spatialReferenceSystem->inAutoMode)
            {
                coor_origin = spatialReferenceSystem->coor_origin_;
            }
            else
            {
                coor_origin = spatialReferenceSystem->coor_origin_custom_;
            }


            item.PushBack(coor_origin.x(), allocator);
            item.PushBack(coor_origin.y(), allocator);
            item.PushBack(coor_origin.z(), allocator);
            jstr.AddMember("coordinate_origin", item, allocator);
            options_.cs_.origin_ = coor_origin;
        }
    }

    if (options_.production_format_ & PRODUCTION_RAPID)
    {
        options_.tiles_.clear();
        options_.tiles_.push_back(tilingRange->tiles_selected_.front());
    }
    else
    {
        options_.tiles_ = tilingRange->tiles_selected_;
    }
    options_.name_ = basicSettings->name_;
    options_.destination_ = AI3D::CORE::File::EnsureUnifySlash(basicSettings->desination_);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    jstr.Accept(writer);

    options_.settings_str_ = std::string(buffer.GetString());

    saved_options_ = options_;
    ///this->accept();
    DeleteTilesList();
    this->hide();
    emit signal_done_options(true);
}

void ParamSettings4Production::DoNextInsideSpatialReferenceSystem()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    iSelectedTabPos = 4;
    if (BlockObject::isChineseVersion())
        butNext->setText("提交");
    else
        butNext->setText("Submit");
    DisplaySelectedTab();
    tilingRange->Init();
    stackedWidget->setCurrentIndex(4);
    butTilingRange->setEnabled(true);
}

void ParamSettings4Production::DoNextInsideTilingRange()
{
    //std::cout << "submit now... " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    Submit();
    // sumbit now.
}

int OpenParamSettings4Production(AI3D::CORE::production_option_s& options, QString strTitle, int w, int h, AI3D::CORE::BlockObject* block_data_,
    AI3D::CORE::ReconstructionObject* recons_object_, AI3D::GUI::ConstructionWgt* pConstructionWgt)
{
    if (pParamSettings4Production != nullptr)
        return -1;
    AI3D::CORE::production_option_s tosetOptions = options;

    pParamSettings4Production = new ParamSettings4Production(options, nullptr, strTitle, block_data_, recons_object_);

    if (pConstructionWgt != nullptr)
    {
        QObject::connect(pParamSettings4Production, &ParamSettings4Production::signal_done_options, pConstructionWgt, &AI3D::GUI::ConstructionWgt::Slot_DoneParamSettings4Production);
    }

    pParamSettings4Production->setWindowModality(Qt::ApplicationModal);
    ///pParamSettings4Production->setAttribute(Qt::WA_DeleteOnClose);

    pParamSettings4Production->resize(w, h);
#if 0
    if (pParamSettings4Production->exec() == QDialog::Accepted)
    {
        options = ParamSettings4Production::GetSavedOptions();
        pParamSettings4Production = nullptr;
        return AI3D_SUCCESS;
    }
#else
    pParamSettings4Production->show();
#endif

    ///pParamSettings4Production = nullptr;
    return -1;
}

void CloseParamSettings4Production()
{
    if (pParamSettings4Production == nullptr)
        return;

    /// pParamSettings4Production->close();
    delete pParamSettings4Production;
    pParamSettings4Production = nullptr;
}

FormatWithOptions::FormatWithOptions(ParamSettings4Production* parent)
    : QWidget(parent)
{
    //Init();
}

FormatWithOptions::~FormatWithOptions()
{

}

void FormatWithOptions::Init()
{
    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 0, 42, 0);

    lblTitle = new QLabel(this);
    lblTitle->setText("Format/Options");

    lblTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:27px;\n"
        "padding-left:42px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    QHBoxLayout* hlFormat;
    hlFormat = new QHBoxLayout();
    hlFormat->setContentsMargins(42, 27, 0, 0);

    lblFormat = new QLabel(this);
    lblFormat->setText("Format:");

    lblFormat->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    cbbFormat = new QComboBox(this);
    cbbFormat->addItem("item 1");
    cbbFormat->addItem("item 2");
    cbbFormat->addItem("item 3");

    cbbFormat->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    border: 0px solid gray;   \n"
        "    border-radius: 3px;   \n"
        "    color: #000000;\n"
        "   font: 14px \"Arial\";\n"
        "   background-color:white;\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding-left: 3px\n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: 0px solid;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 50px;   \n"
        "    background-color:white;\n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: rgb(22,22,22);   \n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:rgb(22,22,22);\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160"
        ",160);   \n"
        "}\n"
        "\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
    ));


    hlFormat->addWidget(lblFormat);
    hlFormat->addWidget(cbbFormat, 1);
    hlFormat->addStretch(1);

    cbLevelOfDetail = new QCheckBox(this);
    cbLevelOfDetail->setText("Level of detail(LOD)");

    cbLevelOfDetail->setStyleSheet(QString::fromUtf8(
        "QCheckBox::indicator{width:0px;}\n"
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "margin-top:27px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    cbIncludeTextureMaps = new QCheckBox(this);
    cbIncludeTextureMaps->setText("Include Texture maps");


    cbIncludeTextureMaps->setStyleSheet(QString::fromUtf8(
        "QCheckBox::indicator{width:0px;}\n"
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "margin-top:27px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        ""));
    vlTop->addSpacing(10);
    vlTop->setSpacing(10);

    vlTop->addWidget(lblTitle);
    vlTop->addLayout(hlFormat);
    vlTop->addWidget(cbLevelOfDetail);
    vlTop->addWidget(cbIncludeTextureMaps);
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
    {
        QHBoxLayout* hlSkirt = new QHBoxLayout();
        hlSkirt->setContentsMargins(42, 27, 0, 0);

        cbSkirt = new QCheckBox(this);
        cbSkirt->setText("Skirt");

        cbSkirt->setStyleSheet(QString::fromUtf8(
            "QCheckBox {background-color:#2D3035;\n"
            "color:red;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
        ));

        lePixel = new QLineEdit(this);

        lePixel->setStyleSheet(QString::fromUtf8(
            "QLineEdit {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            ""));

        lblPixel = new QLabel(this);
        lblPixel->setText("pixel");

        lblPixel->setStyleSheet(QString::fromUtf8(
            "QLabel {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            ""));


        hlSkirt->addWidget(cbSkirt);
        hlSkirt->addWidget(lePixel, 1);
        hlSkirt->addWidget(lblPixel);
        hlSkirt->addStretch(1);
        connect(cbSkirt, &QCheckBox::clicked, this, &FormatWithOptions::Slot_Skirt);
        vlTop->addLayout(hlSkirt);
    }



    vlTop->addStretch(1);

    connect(cbbFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(Slot_Format(const QString&)));
    connect(cbLevelOfDetail, &QCheckBox::clicked, this, &FormatWithOptions::Slot_LevelOfDetail);

    connect(cbIncludeTextureMaps, &QCheckBox::clicked, this, &FormatWithOptions::Slot_IncludeTextureMaps);


    setLayout(vlTop);
}

void FormatWithOptions::Slot_Format(const QString& str)
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
}

void FormatWithOptions::Slot_LevelOfDetail()
{

}

void FormatWithOptions::Slot_IncludeTextureMaps()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
}

void FormatWithOptions::Slot_Skirt()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
}

Export3DMesh_FormatWithOptions::Export3DMesh_FormatWithOptions(ParamSettings4Production* parent)
    : QWidget(parent)
{
    //Init();
}

Export3DMesh_FormatWithOptions::~Export3DMesh_FormatWithOptions()
{

}

void Export3DMesh_FormatWithOptions::Init()
{
    if (bInited)
    {
        // note: to refresh display controls based on the lastest data.
        Reset();
        return;
    }

    vlTop = new QVBoxLayout();
    //vlTop->setContentsMargins(0, 27, 42, 0);
    vlTop->setContentsMargins(0, 27, 0, 20);
    vlTop->setSpacing(5);

    QHBoxLayout* hlTitle = new QHBoxLayout();
    hlTitle->setContentsMargins(42, 0, 0, 0);

    lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblTitle->setText("格式/选项");
    }
    else
    {
        lblTitle->setText("Format/Options");
    }

    lblTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-left:0px;\n"
        "padding-top:0px;\n"
        "font:14px \"Arial\";\n"
        "}\n"
    ));

    //lblTitle->setAlignment(Qt::AlignLeft);
    hlTitle->addWidget(lblTitle, 0, Qt::AlignLeft);
    //hlTitle->addStretch(0);

    QHBoxLayout* hlFormat;
    hlFormat = new QHBoxLayout();
    hlFormat->setContentsMargins(42, 0, 0, 0);
    hlFormat->setSpacing(5);

    lblFormat = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblFormat->setText("格式:");
    }
    else
    {
        lblFormat->setText("Format:");
    }

    lblFormat->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    // note:default at OSGB option.
    cbbFormat = new QComboBox(this);
    cbbFormat->addItem("OpenSceneGraph Binary system (OSGB)");
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        cbbFormat->addItem("3D TILES");
    }
    cbbFormat->addItem("OBJ wavefront format");
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        cbbFormat->addItem("PLY");
    }

    /*if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() != 2)
    {
            QVariant zerov(0);
            cbbFormat->setItemData(1, zerov, Qt::UserRole - 1);
            cbbFormat->setItemData(1, QBrush(QColor(192, 192, 192)), Qt::BackgroundRole);
    }*/

    int currentIndex = 0;
    ///bool bLodType = true;
    //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OSGB)
        {
            currentIndex = 0;
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_3DTILES)
        {
            currentIndex = 1;

        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OBJ)
        {
            currentIndex = 2;
            bLodType = false;
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_PLY)
        {
            currentIndex = 3;
            bLodType = false;
        }
    }
    /*else
    {
        if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OSGB)
        {
            currentIndex = 0;
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OBJ)
        {
            currentIndex = 1;
            bLodType = false;
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_PLY)
        {
            currentIndex = 2;
            bLodType = false;
        }
    }*/
    cbbFormat->setCurrentIndex(currentIndex);

    // QComboBox QAbstractItemView
    //      "    border-radius:0px;\n"  

    cbbFormat->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:36px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    cbbFormat->setItemDelegate(itemDelegate);

    hlFormat->addWidget(lblFormat);
    hlFormat->addWidget(cbbFormat, 3);
    hlFormat->addStretch(1);

    QFrame* topLine = new QFrame(this);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Shadow::Plain);
    //topLine->setStyleSheet("width:899px;height:1px;border-radius:0px;color:#3D434E;");
    topLine->setStyleSheet("max-height:1px;border-radius:0px;border:none;background-color:#3D434E;margin-left:19px;margin-right:11px;");

    if (bLodType)
    {
        cbLevelOfDetail = new QCheckBox(this);
        if (BlockObject::isChineseVersion())
        {
            cbLevelOfDetail->setText("细节级别(LOD)");
        }
        else {
            cbLevelOfDetail->setText("Level of detail(LOD)");
        }

        cbLevelOfDetail->setChecked(withLod_);

        cbLevelOfDetail->setStyleSheet(QString::fromUtf8(
            "QCheckBox {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-left:0px; \n"
            "padding-top:0px; \n"
            "margin-top:0px;\n"
            "margin-left:42px;\n"
            "font:14px \"Arial\";\n}"
            ""));
        //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            hlGenerateLODAcrossTiles = new QHBoxLayout();
            hlGenerateLODAcrossTiles->setContentsMargins(42, 0, 0, 0);
            hlGenerateLODAcrossTiles->setSpacing(5);

            lblGenerateLODAcrossTiles = new QLabel(this);
            if (BlockObject::isChineseVersion())
            {
                lblGenerateLODAcrossTiles->setText("合并根节点");
            }
            else
            {
                lblGenerateLODAcrossTiles->setText("Generate LOD across tiles");
            }
            cbGenerateLODAcrossTiles = new QCheckBox(this);

            cbGenerateLODAcrossTiles->setChecked(withAcrossTile_);

            lblGenerateLODAcrossTiles->setStyleSheet(QString::fromUtf8(
                "QLabel {background-color:#2D3035;\n"
                "color:#FFFFFF;\n"
                "padding-top:0px;\n"
                "padding-left:0px;\n"
                "margin-left:42px;\n"
                "font:14px \"Arial\";\n}"
                ""));

            cbGenerateLODAcrossTiles->setStyleSheet(QString::fromUtf8(
                "QCheckBox {background-color:#2D3035;\n"
                "color:#FFFFFF;\n"
                "padding-top:0px;\n"
                "padding-left:0px;\n"
                "font:14px \"Arial\";\n}"
                ""));

            hlGenerateLODAcrossTiles->addWidget(lblGenerateLODAcrossTiles);
            hlGenerateLODAcrossTiles->addWidget(cbGenerateLODAcrossTiles);
            hlGenerateLODAcrossTiles->addStretch(1);
        }
        hlLODType = new QHBoxLayout();
        hlLODType->setContentsMargins(42, 0, 0, 0);
        hlLODType->setSpacing(5);

        lblLODType = new QLabel(this);
        if (BlockObject::isChineseVersion()) {
            lblLODType->setText("类型");
        }
        else {
            lblLODType->setText("Type");
        }

        lblLODType->setStyleSheet(QString::fromUtf8(
            "QLabel {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "margin-left:42px;\n"
            "font:14px \"Arial\";\n}"
            ""));

        // note:default at Adaptive tree option.
        cbbLODType = new QComboBox(this);
        cbbLODType->addItem("Adaptive tree");
        cbbLODType->addItem("Quadtree");
        int lodIndex = 0;
        if (lodType_ == AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_QUADTREE)
        {
            lodIndex = 1;
        }
        cbbLODType->setCurrentIndex(lodIndex);

        cbbLODType->setStyleSheet(QString::fromUtf8("\n"
            "QComboBox {\n"
            "    background-color:#34363A;"
            "    border: 0px solid;   \n"
            "    border-radius: 4px;   \n"
            "    color: #FFFFFF;\n"
            "   font: 14px \"Arial\";\n"
            "   margin-left:0px; \n"
            "   margin-right:0px; \n"
            "   padding:0px;\n"
            "   padding-left: 13px;\n"
            "   height:36px; \n"
            "}\n"
            "QComboBox:disabled {\n"
            "   color: white;\n"
            "   background-color:gray;\n"
            "}\n"
            "QComboBox::drop-down {\n"
            "   subcontrol-position:top right;\n"
            "   subcontrol-origin:padding;\n"
            "   width:32px;\n"
            "   border:none;\n"
            "}\n"
            "QComboBox::down-arrow { \n"
            "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
            "}\n"
            "QComboBox QAbstractScrollArea {\n"
            "    width: 10px;\n"
            "    color: black; \n"
            "    background-color:white;\n"
            "}\n"
            "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
            "    width: 10px;\n"
            "    background-color: #d0d2d4;  \n"
            "}\n"
            "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
            "    border-radius: 5px;   "
            "    background: rgb(160,160,160);   \n"
            "}\n"
            "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
            "    background: rgb(90, 91, 93);   \n"
            "}\n"
            "QComboBox QAbstractItemView {\n"
            "    outline: 0px solid gray;   \n"
            "    border: none;   \n"
            "    color:#FFFFFF;\n"
            "    background-color: #131313;  \n"
            "    selection-background-color:#333333;   \n"
            "    padding-left: 0px; \n"
            "    margin-left:0px; \n"
            "    margin-right:0px; \n"
            "}\n"
            "QComboBox QAbstractItemView::item {\n"
            "    height: 38px;   \n"
            "    border:none; \n"
            "    background-color:#3F4146;\n"
            "    color:#FFFFFF;"
            "    padding-left: 10px; \n"
            "    margin-left:0px; \n"
            "    margin-right:0px; \n"
            "    font:14px \"Arial\";"
            "}\n"
            "QComboBox QAbstractItemView::item:hover {\n"
            "    color: #FFFFFF;\n"
            "    background-color: #34363A;   \n"
            "}\n"
            "QComboBox QAbstractItemView::item:selected {\n"
            "    color: #FFFFFF;\n"
            "    background-color:#34363A;\n"
            "}\n"
        ));

        QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
        cbbLODType->setItemDelegate(itemDelegate);

        hlLODType->addWidget(lblLODType);
        hlLODType->addWidget(cbbLODType);

        hlLODType->addStretch(1);

    }

    QFrame* middleLine = new QFrame(this);
    middleLine->setFrameShape(QFrame::HLine);
    middleLine->setFrameShadow(QFrame::Shadow::Plain);
    middleLine->setStyleSheet("max-height:1px;border-radius:0px;border:none;background-color:#3D434E;margin-left:39px;margin-right:11px;");

    cbIncludeTextureMaps = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbIncludeTextureMaps->setText("输出纹理");
    }
    else {
        cbIncludeTextureMaps->setText("Include texture maps");
    }
    //cbIncludeTextureMaps->setStyleSheet("#checkBox::indicator{width:0px;}");
    cbIncludeTextureMaps->setStyleSheet(QString::fromUtf8(
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "margin-top:0px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        ""));
    //if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OBJ)
    {
        //@attention因目前算法端不支持，所以此处暂不开启可勾选，即一直要包含纹理
        ///cbIncludeTextureMaps->setCheckable(false);
        withTexMaps_ = true;
    }

    cbIncludeTextureMaps->setChecked(withTexMaps_);
    hlTextureCompression = new QHBoxLayout();
    hlTextureCompression->setContentsMargins(42, 0, 0, 0);
    hlTextureCompression->setSpacing(5);

    lblTextureCompression = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblTextureCompression->setText("纹理压缩");
    }
    else
    {
        lblTextureCompression->setText("Texture compression");
    }
    ///if (withTexMaps_)
    {
        lblTextureCompression->setStyleSheet(QString::fromUtf8(
            "QLabel {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "margin-left:42px;\n"
            "font:14px \"Arial\";\n}"
            ""));

    }
    ///else
    ///{
        //todo
    ///}
    // note:default at 75%
    cbbTextureCompression = new QComboBox(this);
    cbbTextureCompression->addItem(PERCENT_100_QUALITY_JPEG);
    cbbTextureCompression->addItem(PERCENT_90_QUALITY_JPEG);
    cbbTextureCompression->addItem(PERCENT_75_QUALITY_JPEG);
    cbbTextureCompression->addItem(PERCENT_50_QUALITY_JPEG);
    int texComIndex = 2;
    if (texturecompression_ == 100)
    {
        texComIndex = 0;
    }
    else if (texturecompression_ == 90)
    {
        texComIndex = 1;
    }
    else if (texturecompression_ == 50)
    {
        texComIndex = 3;
    }

    cbbTextureCompression->setCurrentIndex(texComIndex);
    ///if (withTexMaps_)
    {

        cbbTextureCompression->setStyleSheet(QString::fromUtf8("\n"
            "QComboBox {\n"
            "    background-color:#34363A;"
            "    border: 0px solid;   \n"
            "    border-radius: 4px;   \n"
            "    color: #FFFFFF;\n"
            "   font: 14px \"Arial\";\n"
            "   margin-left:0px; \n"
            "   margin-right:0px; \n"
            "   padding:0px;\n"
            "   padding-left: 13px;\n"
            "   height:36px; \n"
            "}\n"
            "QComboBox:disabled {\n"
            "   color: white;\n"
            "   background-color:gray;\n"
            "}\n"
            "QComboBox::drop-down {\n"
            "   subcontrol-position:top right;\n"
            "   subcontrol-origin:padding;\n"
            "   width:32px;\n"
            "   border:none;\n"
            "}\n"
            "QComboBox::down-arrow { \n"
            "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
            "}\n"
            "QComboBox QAbstractScrollArea {\n"
            "    width: 10px;\n"
            "    color: black; \n"
            "    background-color:white;\n"
            "}\n"
            "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
            "    width: 10px;\n"
            "    background-color: #d0d2d4;  \n"
            "}\n"
            "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
            "    border-radius: 5px;   "
            "    background: rgb(160,160,160);   \n"
            "}\n"
            "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
            "    background: rgb(90, 91, 93);   \n"
            "}\n"
            "QComboBox QAbstractItemView {\n"
            "    outline: 0px solid gray;   \n"
            "    border: none;   \n"
            "    color:#FFFFFF;\n"
            "    background-color: #131313;  \n"
            "    selection-background-color:#333333;   \n"
            "    padding-left: 0px; \n"
            "    margin-left:0px; \n"
            "    margin-right:0px; \n"
            "}\n"
            "QComboBox QAbstractItemView::item {\n"
            "    height: 38px;   \n"
            "    border:none; \n"
            "    background-color:#3F4146;\n"
            "    color:#FFFFFF;"
            "    padding-left: 10px; \n"
            "    margin-left:0px; \n"
            "    margin-right:0px; \n"
            "    font:14px \"Arial\";"
            "}\n"
            "QComboBox QAbstractItemView::item:hover {\n"
            "    color: #FFFFFF;\n"
            "    background-color: #34363A;   \n"
            "}\n"
            "QComboBox QAbstractItemView::item:selected {\n"
            "    color: #FFFFFF;\n"
            "    background-color:#34363A;\n"
            "}\n"
        ));

        QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
        cbbTextureCompression->setItemDelegate(itemDelegate);
    }
    ///else
    ///{

    ///}

    hlTextureCompression->addWidget(lblTextureCompression);
    hlTextureCompression->addWidget(cbbTextureCompression);
    hlTextureCompression->addStretch(1);

    hlMaximumTextureSize = new QHBoxLayout();
    hlMaximumTextureSize->setContentsMargins(42, 0, 0, 0);
    hlMaximumTextureSize->setSpacing(5);

    lblMaximumTextureSize = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblMaximumTextureSize->setText("最大纹理尺寸:");
    }
    else {
        lblMaximumTextureSize->setText("Maximum texture size:");
    }

    lblMaximumTextureSize->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n"
        "margin-left:42px;}\n"
        ""));

    leMaximumTextureSize = new QLineEdit(this);

    // note:default at 1024 pixel.
    leMaximumTextureSize->setStyleSheet(QString::fromUtf8(
        "QLineEdit {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "margin-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    QIntValidator* intValidatorMaxTexSize = new QIntValidator(this);
    leMaximumTextureSize->setValidator(intValidatorMaxTexSize);

    leMaximumTextureSize->setText(QString::fromStdString(std::to_string(max_texture_size_)));

    lblMaximumTextureSizePixel = new QLabel(this);
    lblMaximumTextureSizePixel->setText("pixel");

    lblMaximumTextureSizePixel->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    hlMaximumTextureSize->addWidget(lblMaximumTextureSize);
    hlMaximumTextureSize->addWidget(leMaximumTextureSize);
    hlMaximumTextureSize->addWidget(lblMaximumTextureSizePixel);
    hlMaximumTextureSize->addStretch(1);
    if (0)
    {
        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            hlSharpening = new QHBoxLayout();
            hlSharpening->setContentsMargins(42, 0, 0, 0);

            lblSharpening = new QLabel(this);
            if (BlockObject::isChineseVersion())
                lblSharpening->setText("纹理锐化");
            else
                lblSharpening->setText("Texture sharpening");

            lblSharpening->setStyleSheet(QString::fromUtf8(
                "QLabel {background-color:#2D3035;\n"
                "color:#FFFFFF;\n"
                "padding-top:0px;\n"
                "padding-left:0px;\n"
                "margin-left:42px;\n"
                "font:14px \"Arial\";\n}"
                ""));
            //  lblSharpening->setAlignment(Qt::AlignVCenter);

            cbSharpening = new QCheckBox(this);
            //cbSharpening->setText("sss");

            cbSharpening->setStyleSheet(QString::fromUtf8(
                "QCheckBox {background-color:#2D3035;\n"
                "color:#FFFFFF;\n"
                "padding-top:0px;\n"
                "padding-left:0px;\n"
                "font:14px \"Arial\";\n}"
                ""));
            cbSharpening->setChecked(withSparping_);
            hlSharpening->setSpacing(0);
            hlSharpening->addWidget(cbSharpening, 0, Qt::AlignVCenter);
            hlSharpening->addWidget(lblSharpening, 0, Qt::AlignVCenter);

            hlSharpening->addStretch(1);
        }
    }
    QFrame* bottomLine = nullptr;

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
    {
        bottomLine = new QFrame(this);
        bottomLine->setFrameShape(QFrame::HLine);
        bottomLine->setFrameShadow(QFrame::Shadow::Plain);
        bottomLine->setStyleSheet("max-height:1px;border-radius:0px;border:none;background-color:#3D434E;margin-left:39px;margin-right:11px;");

        hlSkirt = new QHBoxLayout();
        hlSkirt->setContentsMargins(42, 0, 0, 0);
        hlSkirt->setSpacing(5);

        // note:default unchecked,enabled the next input field if checked.
        cbSkirt = new QCheckBox(this);
        if (BlockObject::isChineseVersion()) {
            cbSkirt->setText("裙边");
        }
        else {
            cbSkirt->setText("Skirt");
        }
        cbSkirt->setChecked(withSkirt_);

        cbSkirt->setStyleSheet(QString::fromUtf8(
            "QCheckBox:disabled {\n"
            "   color:gray;\n"
            "}\n"
            "QCheckBox {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            ""));

        /*
        cbSkirt->setStyleSheet(QString::fromUtf8(
            "QCheckBox { background-color:green;color:red;"
            "}"
        ));
        */

        leSkirt = new QLineEdit(this);


        QRegExpValidator* regexpValidator = new QRegExpValidator();

        leSkirt->setValidator(regexpValidator);
        // note:disabled if cbSkirt is unchecked,otherwise enabled.
        leSkirt->setStyleSheet(QString::fromUtf8(
            "QLineEdit:disabled {\n"
            "   background-color:gray;\n"
            "}\n"
            "QLineEdit {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            ""));

        //leSkirt->setInputMask("");

        leSkirt->setEnabled(cbSkirt->isChecked());
        leSkirt->setText(QString::fromStdString(std::to_string(skirtPix_)));
        lblSkirtPixel = new QLabel(this);
        if (BlockObject::isChineseVersion()) {
            lblSkirtPixel->setText("像素");
        }
        else {
            lblSkirtPixel->setText("pixel");
        }

        lblSkirtPixel->setStyleSheet(QString::fromUtf8(
            "QLabel:disabled {\n"
            "   color:gray;\n"
            "}\n"
            "QLabel {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            ""));

        hlSkirt->setSpacing(5);
        hlSkirt->addWidget(cbSkirt);
        hlSkirt->addWidget(leSkirt, 3);
        hlSkirt->addWidget(lblSkirtPixel);
        hlSkirt->addStretch(1);

        connect(cbSkirt, &QCheckBox::clicked, this, &Export3DMesh_FormatWithOptions::Slot_BeSkirt);
        connect(leSkirt, &QLineEdit::editingFinished, this, &Export3DMesh_FormatWithOptions::Slot_SkirtValue);
    }

    hlOverlap = nullptr;

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (tileOverLap_ >= 0.)
        {
            hlOverlap = new QHBoxLayout();
            hlOverlap->setContentsMargins(42, 0, 0, 0);
            hlOverlap->setSpacing(5);

            // note:default unchecked,enabled the next input field if checked.
            cbOverlap = new QCheckBox(this);
            if (BlockObject::isChineseVersion()) {
                cbOverlap->setText("块间重叠");
            }
            else {
                cbOverlap->setText("Tile overlap");
            }
            cbOverlap->setChecked(withOvelap_);

            cbOverlap->setStyleSheet(QString::fromUtf8(
                "QCheckBox {background-color:#2D3035;\n"
                "color:#FFFFFF;\n"
                "padding-top:0px;\n"
                "padding-left:0px;\n"
                "font:14px \"Arial\";\n}"
                ""));



            leOverlap = new QLineEdit(this);

            QRegExp regexp("((\\d){1,3}(\\.)?(\\d){0,1})");
            QRegExpValidator* regexpValidator = new QRegExpValidator(regexp);

            leOverlap->setValidator(regexpValidator);

            // note:disabled if cbSkirt is unchecked,otherwise enabled.
            leOverlap->setStyleSheet(QString::fromUtf8(
                "QLineEdit {background-color:#2D3035;\n"
                "color:#FFFFFF;\n"
                "padding-top:0px;\n"
                "padding-left:0px;\n"
                "font:14px \"Arial\";\n}"
                ""));

            //leSkirt->setInputMask("");
            leOverlap->setText(QString::fromStdString(std::to_string(tileOverLap_)));
            lblOverlap = new QLabel(this);
            std::string text;
            if (BlockObject::isChineseVersion()) {
                text = "米";
                if (unit_ == 1)
                {
                    text = "单位";
                }
            }
            else {
                text = "meter";
                if (unit_ == 1)
                {
                    text = "unit";
                }
            }

            lblOverlap->setText(QString::fromStdString(text));

            lblOverlap->setStyleSheet(QString::fromUtf8(
                "QLabel {background-color:#2D3035;\n"
                "color:#FFFFFF;\n"
                "padding-top:0px;\n"
                "padding-left:0px;\n"
                "font:14px \"Arial\";\n}"
                ""));

            hlOverlap->setSpacing(5);
            hlOverlap->addWidget(cbOverlap);
            hlOverlap->addWidget(leOverlap, 3);
            hlOverlap->addWidget(lblOverlap);
            hlOverlap->addStretch(1);

            connect(cbOverlap, &QCheckBox::clicked, this, &Export3DMesh_FormatWithOptions::Slot_BeOverlap);
            connect(leOverlap, &QLineEdit::editingFinished, this, &Export3DMesh_FormatWithOptions::Slot_OverlapValue);
        }
    }

    vlTop->setSpacing(20);
    //vlTop->addWidget(lblTitle);
    vlTop->addLayout(hlTitle);
    vlTop->addLayout(hlFormat);
    vlTop->addWidget(topLine);
    vlTop->addWidget(cbLevelOfDetail);

    //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        vlTop->addLayout(hlGenerateLODAcrossTiles);
    }

    vlTop->addLayout(hlLODType);
    vlTop->addWidget(middleLine);

    vlTop->addWidget(cbIncludeTextureMaps);
    vlTop->addLayout(hlTextureCompression);
    vlTop->addLayout(hlMaximumTextureSize);

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2 && 0)
    {
        vlTop->addLayout(hlSharpening);
    }
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (bottomLine)
            vlTop->addWidget(bottomLine);

    }

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
    {
        vlTop->addLayout(hlSkirt);

    }

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (tileOverLap_ >= 0. && hlOverlap)
            vlTop->addLayout(hlOverlap);
    }

    vlTop->addStretch(1);
    //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        connect(cbGenerateLODAcrossTiles, &QCheckBox::clicked, this, &Export3DMesh_FormatWithOptions::Slot_AcrossTiles);
    }
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2 && 0)
    {
        connect(cbSharpening, &QCheckBox::clicked, this, &Export3DMesh_FormatWithOptions::Slot_BeSharpenning);
    }
    connect(cbLevelOfDetail, &QCheckBox::clicked, this, &Export3DMesh_FormatWithOptions::Slot_LodChecked);
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        connect(cbIncludeTextureMaps, &QCheckBox::clicked, this, &Export3DMesh_FormatWithOptions::Slot_IncludeTexChecked);

    }
    connect(leMaximumTextureSize, &QLineEdit::editingFinished, this, &Export3DMesh_FormatWithOptions::Slot_MaxTexSize);

    connect(cbbLODType, &QComboBox::currentTextChanged, this, &Export3DMesh_FormatWithOptions::Slot_LodType);
    connect(cbbFormat, &QComboBox::currentTextChanged, this, &Export3DMesh_FormatWithOptions::Slot_Format);
    connect(cbbTextureCompression, &QComboBox::currentTextChanged, this, &Export3DMesh_FormatWithOptions::Slot_TextureCompress);



    SwitchLodChecked();
    SwitchLodType();

    // force includeTextureMaps state to being enabled and checked.
    withTexMaps_ = true;
    cbIncludeTextureMaps->setChecked(withTexMaps_);

    cbIncludeTextureMaps->setEnabled(false);

    SwitchIncTex();

    cbLevelOfDetail->setEnabled(false);

    setLayout(vlTop);

    bInited = true;
}

void Export3DMesh_FormatWithOptions::SwitchLodType()
{
    if (bLodType)
    {
        cbLevelOfDetail->setVisible(true);
        lblLODType->setVisible(true);
        cbbLODType->setVisible(true);
        //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            lblGenerateLODAcrossTiles->setVisible(true);
            cbGenerateLODAcrossTiles->setVisible(true);
            cbGenerateLODAcrossTiles->setEnabled(true);//false
            cbGenerateLODAcrossTiles->setChecked(withAcrossTile_);//false
            if (cbGenerateLODAcrossTiles->isChecked())
            {
                lblLODType->setEnabled(false);
                cbbLODType->setEnabled(false);
            }
        }


        // As bLodType becomes true,force generateLODAcrossTiles state to being unchecked and disabled,
        // and reset the state of levelOfDetail to being checked and enabled.
        ////cbLevelOfDetail->setEnabled(true);

        cbLevelOfDetail->setChecked(true);

    }
    else
    {
        cbLevelOfDetail->setEnabled(false);
        //  if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            lblGenerateLODAcrossTiles->setEnabled(false);
            cbGenerateLODAcrossTiles->setEnabled(false);
        }
        lblLODType->setEnabled(false);
        cbbLODType->setEnabled(false);
    }
}

void Export3DMesh_FormatWithOptions::SwitchLodTypeAfterFormatSelection()
{
    if (bLodType)
    {
        cbLevelOfDetail->setVisible(true);
        lblLODType->setVisible(true);
        cbbLODType->setVisible(true);
        //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            lblGenerateLODAcrossTiles->setVisible(true);
            cbGenerateLODAcrossTiles->setVisible(true);
            cbGenerateLODAcrossTiles->setEnabled(true);//false
            cbGenerateLODAcrossTiles->setChecked(withAcrossTile_);//false
            if (cbGenerateLODAcrossTiles->isChecked())
            {
                lblLODType->setEnabled(false);
                cbbLODType->setEnabled(false);
            }
        }


        // As bLodType becomes true,force generateLODAcrossTiles state to being unchecked and disabled,
        // and reset the state of levelOfDetail to being checked and enabled.
        ////cbLevelOfDetail->setEnabled(true);

        ////cbLevelOfDetail->setChecked(true);
        withLod_ = true;
    }
    else
    {
        cbLevelOfDetail->setEnabled(false);
        //  if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            lblGenerateLODAcrossTiles->setEnabled(false);
            cbGenerateLODAcrossTiles->setEnabled(false);
        }
        lblLODType->setEnabled(false);
        cbbLODType->setEnabled(false);
        withLod_ = false;
    }
}

void Export3DMesh_FormatWithOptions::Reset()
{
    int currentIndex = 0;
    ///bool bLodType = true;
    if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OSGB)
    {
        currentIndex = 0;
        bLodType = true;
    }
    else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_3DTILES)
    {
        currentIndex = 1;
        bLodType = true;
    }
    else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OBJ)
    {
        currentIndex = 2;
        bLodType = false;
    }
    else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_PLY)
    {
        currentIndex = 3;
        bLodType = false;
    }

    cbbFormat->setCurrentIndex(currentIndex);

    ////cbLevelOfDetail->setChecked(withLod_);
    cbLevelOfDetail->setChecked(bLodType);

    //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        cbGenerateLODAcrossTiles->setChecked(withAcrossTile_);
    }

    int lodIndex = 0;
    if (lodType_ == AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_QUADTREE)
    {
        lodIndex = 1;
    }

    cbbLODType->setCurrentIndex(lodIndex);

    //if (format_ == AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OBJ)
    {
        //@attention因目前算法端不支持，所以此处暂不开启可勾选，即一直要包含纹理
        ///cbIncludeTextureMaps->setCheckable(false);
        withTexMaps_ = true;
    }

    ///cbIncludeTextureMaps->setChecked(withTexMaps_);

    int texComIndex = 2;
    if (texturecompression_ == 100)
    {
        texComIndex = 0;
    }
    else if (texturecompression_ == 90)
    {
        texComIndex = 1;
    }
    else if (texturecompression_ == 50)
    {
        texComIndex = 3;
    }

    cbbTextureCompression->setCurrentIndex(texComIndex);

    leMaximumTextureSize->setText(QString::fromStdString(std::to_string(max_texture_size_)));
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2 && 0)
    {
        cbSharpening->setChecked(withSparping_);
    }

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
    {
        cbSkirt->setChecked(withSkirt_);
        leSkirt->setText(QString::fromStdString(std::to_string(skirtPix_)));
    }

    SwitchLodChecked();
    ////SwitchLodType();
    SwitchLodTypeAfterFormatSelection();

    // force includeTextureMaps state to being enabled and checked.
    withTexMaps_ = true;
    cbIncludeTextureMaps->setChecked(withTexMaps_);

    SwitchIncTex();


}


void Export3DMesh_FormatWithOptions::Slot_LodChecked()
{
    //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (cbLevelOfDetail->isChecked())
        {
            ///     cbbLODType->setEnabled(true);
            ///     cbGenerateLODAcrossTiles->setEnabled(true);
            withLod_ = true;
        }
        else
        {
            ///     cbbLODType->setEnabled(false);
            ///     cbGenerateLODAcrossTiles->setEnabled(false);
            withLod_ = false;
        }

        SwitchLodChecked();
    }
}

void Export3DMesh_FormatWithOptions::SwitchLodChecked()
{
    if (withLod_)
    {
        cbbLODType->setEnabled(true);
        lblLODType->setEnabled(true);
        //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            cbGenerateLODAcrossTiles->setEnabled(withAcrossTile_);
            cbGenerateLODAcrossTiles->setChecked(withAcrossTile_);
            if (cbGenerateLODAcrossTiles->isChecked())
            {
                lblLODType->setEnabled(false);
                cbbLODType->setEnabled(false);
            }
        }
    }
    else
    {
        cbbLODType->setEnabled(false);
        //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        cbGenerateLODAcrossTiles->setEnabled(false);
    }

}

void Export3DMesh_FormatWithOptions::DefaultParams()
{
    format_ = AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OSGB;
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        withAcrossTile_ = false;//@attention add by  chy at 20231211 因目前算法不支持，理论上是默认为true；
    }
    withLod_ = true;
    lodType_ = AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_ADAPTIVETREE;
    withTexMaps_ = true;
    texturecompression_ = 0.75;
    max_texture_size_ = 8192;
    withSparping_ = true;
    withSkirt_ = false;
    skirtPix_ = 4;//@attention add by chy ，因目前算法的逻辑未定，暂定为0；
    tileOverLap_ = defaultTileOverlap_;
}

void Export3DMesh_FormatWithOptions::ValidParams()
{

}

void Export3DMesh_FormatWithOptions::SetInValid()
{
    format_ = AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;
}

void Export3DMesh_FormatWithOptions::Slot_AcrossTiles()
{
    //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (cbGenerateLODAcrossTiles->isChecked())
        {
            std::cout << "withAcrossTile true." << std::endl;
            withAcrossTile_ = true;
            //另外两个需要关掉
            lblLODType->setEnabled(false);
            cbbLODType->setEnabled(false);
        }
        else
        {
            std::cout << "withAcrossTile false." << std::endl;
            withAcrossTile_ = false;
            lblLODType->setEnabled(true);
            cbbLODType->setEnabled(true);
        }
    }

}
void Export3DMesh_FormatWithOptions::Slot_LodType(const QString& str)
{
    if (str == "Adaptive tree")
    {
        std::cout << "choose adative tree." << std::endl;
        lodType_ = AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_ADAPTIVETREE;
    }
    else if (str == "Quadtree")
    {
        std::cout << "choose quadtree." << std::endl;
        lodType_ = AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_QUADTREE;
    }
}

bool Export3DMesh_FormatWithOptions::IsValid()
{
    return format_ != AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN;
}

void Export3DMesh_FormatWithOptions::Slot_IncludeTexChecked()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2 && 0)
    {
        if (cbIncludeTextureMaps->isChecked())
        {
            std::cout << "include tex clicked / true." << std::endl;
            ///cbbTextureCompression->setEnabled(true);
            ///leMaximumTextureSize->setEnabled(true);
            ///cbSharpening->setEnabled(true);
            withTexMaps_ = true;
        }
        else
        {
            std::cout << "include tex clicked / false." << std::endl;
            ///cbbTextureCompression->setEnabled(false);
            ///leMaximumTextureSize->setEnabled(false);
            ///cbSharpening->setEnabled(false);
            withTexMaps_ = false;
        }

        SwitchIncTex();
    }
}

void Export3DMesh_FormatWithOptions::SwitchIncTex()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (withTexMaps_)
        {
            std::cout << "switch inc tex / true." << std::endl;
            cbbTextureCompression->setEnabled(true);
            leMaximumTextureSize->setEnabled(true);
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2 && 0)
            {
                cbSharpening->setEnabled(true);
            }
        }
        else
        {
            std::cout << "switch inc tex / false." << std::endl;
            cbbTextureCompression->setEnabled(false);
            leMaximumTextureSize->setEnabled(false);
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2 && 0)
            {
                cbSharpening->setEnabled(false);
            }
        }
    }
}

void Export3DMesh_FormatWithOptions::Slot_TextureCompress()
{
    ///std::string strText = lblTextureCompression->text().QString::toStdString();
    std::string strText = cbbTextureCompression->currentText().toStdString();

    if (strText == PERCENT_75_QUALITY_JPEG)
        texturecompression_ = 75;
    else if (strText == PERCENT_100_QUALITY_JPEG)
        texturecompression_ = 100;
    else if (strText == PERCENT_90_QUALITY_JPEG)
        texturecompression_ = 90;
    else if (strText == PERCENT_50_QUALITY_JPEG)
        texturecompression_ = 50;

    std::cout << strText << " " << texturecompression_ << std::endl;
}

void Export3DMesh_FormatWithOptions::Slot_MaxTexSize()
{
    std::string strText = leMaximumTextureSize->text().toStdString();
    max_texture_size_ = std::atoi(strText.c_str());
}

void Export3DMesh_FormatWithOptions::Slot_BeSharpenning()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2 && 0)
    {
        if (cbSharpening->isChecked())
        {
            std::cout << "withSharpening true." << std::endl;
            withSparping_ = true;
        }
        else
        {
            std::cout << "withSharpening false." << std::endl;
            withSparping_ = false;
        }
    }
}
void Export3DMesh_FormatWithOptions::Slot_BeOverlap()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        if (cbOverlap->isChecked())
        {
            leOverlap->setEnabled(true);
            withOvelap_ = true;
            std::string strOverlap = leOverlap->text().QString::toStdString();
            tileOverLap_ = std::atof(strOverlap.c_str());
        }
        else
        {
            leOverlap->setEnabled(false);
            withOvelap_ = false;
        }
    }
}
void Export3DMesh_FormatWithOptions::Slot_OverlapValue()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        std::string strOverLap = leOverlap->text().toStdString();
        tileOverLap_ = std::atof(strOverLap.c_str());
    }
}
void Export3DMesh_FormatWithOptions::Slot_BeSkirt()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
    {
        if (cbSkirt->isChecked())
        {
            leSkirt->setEnabled(true);
            withSkirt_ = true;
            std::string strSkirt = leSkirt->text().QString::toStdString();
            skirtPix_ = std::atoi(strSkirt.c_str());
        }
        else
        {
            leSkirt->setEnabled(false);
            withSkirt_ = false;

        }
    }
}

void Export3DMesh_FormatWithOptions::Slot_SkirtValue()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        std::string strSkirt = leSkirt->text().toStdString();
        skirtPix_ = std::atoi(strSkirt.c_str());
    }
}

void Export3DMesh_FormatWithOptions::Slot_Format(const QString& str)
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " str " << str.QString::toStdString() << std::endl;
    std::string strFormat = str.QString::toStdString();

    if (strFormat == "OpenSceneGraph Binary system (OSGB)")
    {
        format_ = AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OSGB;
        bLodType = true;
        //// cbLevelOfDetail->setEnabled(true);

        cbLevelOfDetail->setChecked(true);
    }
    else if (strFormat == "3D TILES")
    {
        format_ = AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_3DTILES;
        bLodType = true;
        //// cbLevelOfDetail->setEnabled(true);
        cbLevelOfDetail->setChecked(true);
    }
    else if (strFormat == "OBJ wavefront format")
    {
        format_ = AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OBJ;
        bLodType = false;
        cbLevelOfDetail->setEnabled(false);
        ///cbLevelOfDetail->setVisible(false);
        cbLevelOfDetail->setChecked(false);
    }
    else if (strFormat == "PLY")
    {
        format_ = AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_PLY;
        bLodType = false;
        cbLevelOfDetail->setEnabled(false);
        ///cbLevelOfDetail->setVisible(false);
        cbLevelOfDetail->setChecked(false);
    }

    SwitchLodTypeAfterFormatSelection();
}

Export3D_Point_Cloud::Export3D_Point_Cloud(ParamSettings4Production* parent)
    : QWidget(parent)
{
    //Init();
}

Export3D_Point_Cloud::~Export3D_Point_Cloud()
{

}
void Export3D_Point_Cloud::DefaultParams()
{
    format_ = AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_LAS;
    samplingDistance_ = defaultSamplingDistance_;
    samUnit_ = 1; // 
}

void Export3D_Point_Cloud::Init()
{
    if (bInited)
    {
        Reset();
        return;
    }


    vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 27, 0, 0);

    QHBoxLayout* hlTitle = new QHBoxLayout();
    hlTitle->setContentsMargins(42, 0, 0, 0);

    lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblTitle->setText("格式/选项");
    }
    else {
        lblTitle->setText("Format/Options");
    }

    lblTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "margin-top:0px;\n"
        "margin-left:0px;\n"
        "padding:0px;\n"
        "font:14px \"Arial\";\n"
        "border:none;}\n"
    ));

    hlTitle->addWidget(lblTitle);
    hlTitle->addStretch(1);

    QFont font = lblTitle->font();
    font.setPixelSize(14);
    lblTitle->setFont(font);

    hlFormat = new QHBoxLayout();
    hlFormat->setContentsMargins(42, 0, 0, 0);

    lblFormat = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblFormat->setText("格式:");
    }
    else {
        lblFormat->setText("Format:");
    }
    lblFormat->setFont(font);

    lblFormat->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    cbbFormat = new QComboBox(this);
    ///cbbFormat->addItem("ASPRS LASER(LAS)");
    cbbFormat->addItem(POINT_CLOUD_FORMAT_LAS);
    cbbFormat->addItem(POINT_CLOUD_FORMAT_PLY);
    cbbFormat->addItem(POINT_CLOUD_FORMAT_OSGB);
    cbbFormat->setFont(font);
    if (format_ != AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN)
    {
        if (format_ == AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_LAS)
        {
            cbbFormat->setCurrentText(POINT_CLOUD_FORMAT_LAS);
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_PLY)
        {
            cbbFormat->setCurrentText(POINT_CLOUD_FORMAT_PLY);
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_OSGB)
        {
            cbbFormat->setCurrentText(POINT_CLOUD_FORMAT_OSGB);
        }
    }

    cbbFormat->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:36px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    cbbFormat->setItemDelegate(itemDelegate);

    hlFormat->addWidget(lblFormat);
    hlFormat->addWidget(cbbFormat, 4);
    hlFormat->addStretch(1);

    hlSamplingPoints = new QHBoxLayout();
    hlSamplingPoints->setContentsMargins(42, 0, 0, 0);

    lblSamplingPoints = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblSamplingPoints->setText("采样点:");
    }
    else {
        lblSamplingPoints->setText("Sampling points:");
    }
    lblSamplingPoints->setFont(font);

    lblSamplingPoints->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    leSamplingPoints = new QLineEdit(this);

    leSamplingPoints->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));
    QDoubleValidator* doubleValidator = new QDoubleValidator(this);
    leSamplingPoints->setValidator(doubleValidator);
    if (samUnit_ < 0 || samUnit_ > 1)
        samUnit_ = 1;
    std::string distance_str = samUnit_ == 1 ? std::to_string(samplingDistance_) : "1";
    leSamplingPoints->setText(QString::fromStdString(distance_str));

    leSamplingPoints->setFont(font);

    // note:default at pixel option.
    cbbSamplingPointsUnit = new QComboBox(this);
    cbbSamplingPointsUnit->addItem(SAMPLING_POINTS_UNIT_PIXEL);
    cbbSamplingPointsUnit->addItem(SAMPLING_POINTS_UNIT_METER);

    cbbSamplingPointsUnit->setFont(font);

    cbbSamplingPointsUnit->setCurrentIndex(samUnit_);

    cbbSamplingPointsUnit->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:36px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate2 = new QStyledItemDelegate();
    cbbSamplingPointsUnit->setItemDelegate(itemDelegate2);


    hlSamplingPoints->addWidget(lblSamplingPoints);
    hlSamplingPoints->addWidget(leSamplingPoints, 2);
    hlSamplingPoints->addWidget(cbbSamplingPointsUnit);
    hlSamplingPoints->addStretch(1);

    //vlTop->addSpacing(10);
    vlTop->setSpacing(27);
    //vlTop->addWidget(lblTitle);
    vlTop->addLayout(hlTitle);
    vlTop->addLayout(hlFormat);
    vlTop->addLayout(hlSamplingPoints);
    vlTop->addStretch(1);

    connect(cbbFormat, &QComboBox::currentTextChanged, this, &Export3D_Point_Cloud::Slot_Format);
    connect(cbbSamplingPointsUnit, &QComboBox::currentTextChanged, this, &Export3D_Point_Cloud::Slot_SamplingPointsUnit);
    connect(leSamplingPoints, &QLineEdit::editingFinished, this, &Export3D_Point_Cloud::Slot_SamplingPoints);

    setLayout(vlTop);
    bInited = true;
}

void Export3D_Point_Cloud::Reset()
{
    if (format_ != AI3D::CORE::production_format_e::PRODUCTION_FORMAT_UNKNOWN)
    {
        if (format_ == AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_LAS)
        {
            cbbFormat->setCurrentText(POINT_CLOUD_FORMAT_LAS);
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_PLY)
        {
            cbbFormat->setCurrentText(POINT_CLOUD_FORMAT_PLY);
        }
        else if (format_ == AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_OSGB)
        {
            cbbFormat->setCurrentText(POINT_CLOUD_FORMAT_OSGB);
        }

    }

    leSamplingPoints->setText(QString::number(samplingDistance_));

    if (samUnit_ < 0 || samUnit_ > 1)
        samUnit_ = 1;
    cbbSamplingPointsUnit->setCurrentIndex(samUnit_);
}


void Export3D_Point_Cloud::Slot_SamplingPointsUnit(const QString& str)
{
    // note:need to convert current value inside input field between meter and pixel?
    if (str == SAMPLING_POINTS_UNIT_METER)
    {
        std::cout << "choose meter." << std::endl;
        samUnit_ = 1;
        std::string distance_str = std::to_string(defaultSamplingDistance_);
        leSamplingPoints->setText(QString::fromStdString(distance_str));
    }
    else if (str == SAMPLING_POINTS_UNIT_PIXEL)
    {
        std::cout << "choose pixel." << std::endl;
        samUnit_ = 0;
        leSamplingPoints->setText("1");
    }
}

void Export3D_Point_Cloud::Slot_Format(const QString& str)
{
    if (str == POINT_CLOUD_FORMAT_LAS)
    {
        format_ = AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_LAS;
    }
    else if (str == POINT_CLOUD_FORMAT_PLY)
    {
        format_ = AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_PLY;
    }
    else if (str == POINT_CLOUD_FORMAT_OSGB)
    {

        format_ = AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_OSGB;
    }

    std::cout << "inside point cloud,choose " << format_ << "." << std::endl;
}

void Export3D_Point_Cloud::Slot_SamplingPoints()
{
    QString txt = leSamplingPoints->text();
    samplingDistance_ = std::atof(txt.toStdString().c_str());
    std::cout << "point cloud:" << leSamplingPoints->text().toStdString() << " / " << samplingDistance_ << std::endl;
}

ExportOrthophoto_DSM::ExportOrthophoto_DSM(ParamSettings4Production* parent)
    : QWidget(parent)
{
    //Init();
}

ExportOrthophoto_DSM::~ExportOrthophoto_DSM()
{

}

void ExportOrthophoto_DSM::DefaultParams()
{
    max_image_dim_ = 4096;
    withTDOM_ = true;

    tdom_format_ = AI3D::CORE::tdom_format_e::TDOM_FORMAT_TIFFGEOTIFF;
    sampling_distance_ = default_sampling_distance_;

    withDSM_ = true;
    dsm_format_ = AI3D::CORE::dsm_format_e::DSM_FORMAT_TIFFGEOTIFF;
    tdommode_ = AI3D::CORE::tdom_mode_e::NORMAL;
}

void ExportOrthophoto_DSM::Init()
{
    if (bInited)
    {
        Reset();
        return;
    }

    vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(42, 27, 42, 27);

    lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblTitle->setText("格式/选项");
    }
    else {
        lblTitle->setText("Format/Options");
    }

    lblTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "margin:0px;\n"
        "padding:0px;\n"
        "font:14px \"Arial\";\n"
        "border:none;}\n"
    ));

    QFrame* topLine = new QFrame(this);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Shadow::Plain);
    topLine->setStyleSheet("max-height:1px;border-radius:0px;border:none;background-color:#3D434E;margin-left:19px;margin-right:11px;");

    hlResolution = new QHBoxLayout();
    hlResolution->setContentsMargins(0, 0, 0, 0);

    lblResolution = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblResolution->setText("分辨率(米):");
    }
    else {
        lblResolution->setText("Resolution(meters):");
    }

    lblResolution->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    leResolution = new QLineEdit(this);
    leResolution->setStyleSheet(QString::fromUtf8(
        "QLineEdit {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));
    QDoubleValidator* dvResolution = new QDoubleValidator(this);
    leResolution->setValidator(dvResolution);
    leResolution->setText(QString::fromStdString(std::to_string(sampling_distance_)));

    hlResolution->addWidget(lblResolution);
    hlResolution->addWidget(leResolution, 1);
    hlResolution->addStretch(1);
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        hlMaximumImagePartDimension = new QHBoxLayout();
        hlMaximumImagePartDimension->setContentsMargins(0, 0, 0, 0);

        lblMaximumImagePartDimension = new QLabel(this);
        if (BlockObject::isChineseVersion()) {
            lblMaximumImagePartDimension->setText("最大图形部分尺寸(px):");
        }
        else {
            lblMaximumImagePartDimension->setText("Maximum image part dimension(px):");
        }

        lblMaximumImagePartDimension->setStyleSheet(QString::fromUtf8(
            "QLabel {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            ""));

        // note:default is 4096.
        leMaximumImagePartDimension = new QLineEdit(this);
        QIntValidator* ivDimension = new QIntValidator(this);
        leMaximumImagePartDimension->setValidator(ivDimension);

        leMaximumImagePartDimension->setText(QString::fromStdString(std::to_string(max_image_dim_)));
        leMaximumImagePartDimension->setStyleSheet(QString::fromUtf8(
            "QLineEdit {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            ""));

        hlMaximumImagePartDimension->addWidget(lblMaximumImagePartDimension);
        hlMaximumImagePartDimension->addWidget(leMaximumImagePartDimension);
        hlMaximumImagePartDimension->addStretch(1);
    }
    hlMode = new QHBoxLayout();
    hlMode->setContentsMargins(0, 0, 0, 0);

    lblMode = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblMode->setText("模式:");
    }
    else {
        lblMode->setText("Mode:");
    }

    lblMode->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    // note:default at normal option.
    cbbMode = new QComboBox(this);
    cbbMode->addItem("Normal");
    int index = 0;

    cbbMode->addItem("RapidMosaic");

    cbbMode->addItem("Low");

    //暂时不支持

        /*QVariant zerov(0);
        cbbMode->setItemData(1, zerov, Qt::UserRole - 1);
        cbbMode->setItemData(1, QBrush(QColor(192, 192, 192)), Qt::BackgroundRole);
        cbbMode->setItemData(2, zerov, Qt::UserRole - 1);
        cbbMode->setItemData(2, QBrush(QColor(192, 192, 192)), Qt::BackgroundRole);*/


    if (tdommode_ == AI3D::CORE::tdom_mode_e::RAPIDMOSAIC)
    {
        index = 1;
    }
    else if (tdommode_ == AI3D::CORE::tdom_mode_e::LOW)
    {
        index = 2;
    }
    /*if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
    {
    cbbMode->addItem("Fast");
        if (tdommode_ == AI3D::CORE::tdom_mode_e::FASTMOSAIC)
        {
            index = 3;
        }
    }*/
    cbbMode->setCurrentIndex(index);

    cbbMode->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:36px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate1 = new QStyledItemDelegate();
    cbbMode->setItemDelegate(itemDelegate1);


    hlMode->addWidget(lblMode);
    hlMode->addWidget(cbbMode);
    hlMode->addStretch(1);


    QFrame* middleLine = new QFrame(this);
    middleLine->setFrameShape(QFrame::HLine);
    middleLine->setFrameShadow(QFrame::Shadow::Plain);
    middleLine->setStyleSheet("max-height:1px;border-radius:0px;border:none;background-color:#3D434E;margin-left:39px;margin-right:11px;");

    cbOrthophoto = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbOrthophoto->setText("正交影像");
    }
    else {
        cbOrthophoto->setText("Orthophoto");
    }
    cbOrthophoto->setStyleSheet(QString::fromUtf8(
        "QCheckBox::indicator{width:0px;}\n"
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    cbOrthophoto->setChecked(withTDOM_);

    hlOrthophotoFormat = new QHBoxLayout();
    hlOrthophotoFormat->setContentsMargins(0, 0, 0, 0);

    lblOrthophotoFormat = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblOrthophotoFormat->setText("格式:");
    }
    else {
        lblOrthophotoFormat->setText("Format:");
    }

    lblOrthophotoFormat->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    //note:default at TIFF/GeoTIFF option.
    cbbOrthophotoFormat = new QComboBox(this);
    cbbOrthophotoFormat->addItem("TIFF/GeoTIFF");
    //@attention chy added comment here
    //cbbOrthophotoFormat->addItem("JPG");
    {
        int index = 0;
        if (tdom_format_ == AI3D::CORE::tdom_format_e::TDOM_FORMAT_JPEG)
        {
            index = 1;
        }
        cbbOrthophotoFormat->setCurrentIndex(index);
    }

    cbbOrthophotoFormat->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:36px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate2 = new QStyledItemDelegate();
    cbbOrthophotoFormat->setItemDelegate(itemDelegate2);

    hlOrthophotoFormat->addWidget(lblOrthophotoFormat);
    hlOrthophotoFormat->addWidget(cbbOrthophotoFormat);
    hlOrthophotoFormat->addStretch(1);

    QFrame* bottomLine = new QFrame(this);
    bottomLine->setFrameShape(QFrame::HLine);
    bottomLine->setFrameShadow(QFrame::Shadow::Plain);
    bottomLine->setStyleSheet("max-height:1px;border-radius:0px;border:none;background-color:#3D434E;margin-left:39px;margin-right:11px;");


    cbDSM = new QCheckBox(this);
    cbDSM->setText("DSM");

    cbDSM->setStyleSheet(QString::fromUtf8(
        "QCheckBox::indicator{width:0px;}\n"
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";}\n"
        ""));
    cbDSM->setChecked(withDSM_);
    hlDSMFormat = new QHBoxLayout();
    hlDSMFormat->setContentsMargins(0, 0, 0, 0);

    lblDSMFormat = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblDSMFormat->setText("格式:");
    }
    else {
        lblDSMFormat->setText("Format:");
    }

    lblDSMFormat->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    // note:default at TIFF/GeoTIFF option.
    cbbDSMFormat = new QComboBox(this);
    cbbDSMFormat->addItem("TIFF/GeoTIFF");
    index = 0;
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        cbbDSMFormat->addItem("XYZ");
        {

            if (tdom_format_ == AI3D::CORE::dsm_format_e::DSM_FORMAT_XYZ)
            {
                index = 1;
            }
        }
        cbbDSMFormat->setCurrentIndex(index);
    }

    cbbDSMFormat->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:36px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate3 = new QStyledItemDelegate();
    cbbDSMFormat->setItemDelegate(itemDelegate3);


    hlDSMFormat->addWidget(lblDSMFormat);
    hlDSMFormat->addWidget(cbbDSMFormat);
    hlDSMFormat->addStretch(1);

    //vlTop->addSpacing(27);
    vlTop->setSpacing(27);
    vlTop->addWidget(lblTitle);
    vlTop->addWidget(topLine);
    vlTop->addLayout(hlResolution);
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        vlTop->addLayout(hlMaximumImagePartDimension);
    }
    vlTop->addLayout(hlMode);
    vlTop->addWidget(middleLine);
    vlTop->addWidget(cbOrthophoto);
    vlTop->addLayout(hlOrthophotoFormat);
    vlTop->addWidget(bottomLine);
    vlTop->addWidget(cbDSM);
    vlTop->addLayout(hlDSMFormat);
    vlTop->addStretch(1);

    connect(cbbMode, &QComboBox::currentTextChanged, this, &ExportOrthophoto_DSM::Slot_ModeChanged);
    connect(leResolution, &QLineEdit::editingFinished, this, &ExportOrthophoto_DSM::Slot_SetRes);
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        connect(leMaximumImagePartDimension, &QLineEdit::editingFinished, this, &ExportOrthophoto_DSM::Slot_SetMaxImageDim);
    }

    connect(cbbOrthophotoFormat, &QComboBox::currentTextChanged, this, &ExportOrthophoto_DSM::Slot_TDOMFORMATChanged);
    connect(cbbDSMFormat, &QComboBox::currentTextChanged, this, &ExportOrthophoto_DSM::Slot_DSMFORMATChanged);
    connect(cbOrthophoto, &QCheckBox::clicked, this, &ExportOrthophoto_DSM::Slot_OrthophotoChecked);
    connect(cbDSM, &QCheckBox::clicked, this, &ExportOrthophoto_DSM::Slot_DSMChecked);

    SwitchOrthophotoChecked();
    SwitchDSMChecked();

    setLayout(vlTop);
    bInited = true;
}

void ExportOrthophoto_DSM::Slot_SetMaxImageDim()
{
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        std::string text = leMaximumImagePartDimension->text().toStdString();
        max_image_dim_ = std::atoi(text.c_str());
    }
}

void ExportOrthophoto_DSM::Slot_SetRes()
{
    std::string text = leResolution->text().toStdString();
    sampling_distance_ = std::atof(text.c_str());
}

void ExportOrthophoto_DSM::Slot_DSMFORMATChanged(const QString& str)
{
    if (str == "TIFF/GeoTIFF")
    {
        dsm_format_ = AI3D::CORE::dsm_format_e::DSM_FORMAT_TIFFGEOTIFF;
    }
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        /*else*/ if (str == "XYZ")
        {
            dsm_format_ = AI3D::CORE::dsm_format_e::DSM_FORMAT_XYZ;
        }
    }

}

void ExportOrthophoto_DSM::Slot_TDOMFORMATChanged(const QString& str)
{
    if (str == "TIFF/GeoTIFF")
    {
        tdom_format_ = AI3D::CORE::tdom_format_e::TDOM_FORMAT_TIFFGEOTIFF;
    }
    else if (str == "JPG")
    {
        tdom_format_ = AI3D::CORE::tdom_format_e::TDOM_FORMAT_JPEG;
    }
}

void ExportOrthophoto_DSM::Reset()
{
    leResolution->setText(QString::fromStdString(std::to_string(sampling_distance_)));
    int index = 0;
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        leMaximumImagePartDimension->setText(QString::fromStdString(std::to_string(max_image_dim_)));


        if (tdommode_ == AI3D::CORE::tdom_mode_e::RAPIDMOSAIC)
        {
            index = 1;
        }
        else if (tdommode_ == AI3D::CORE::tdom_mode_e::LOW)
        {
            index = 2;
        }
        /*else if (tdommode_ == AI3D::CORE::tdom_mode_e::FASTMOSAIC)
        {
            index = 3;
        }*/
    }
    cbbMode->setCurrentIndex(index);

    cbOrthophoto->setChecked(withTDOM_);

    {
        int index = 0;
        if (tdom_format_ == AI3D::CORE::tdom_format_e::TDOM_FORMAT_JPEG)
        {
            index = 1;
        }
        cbbOrthophotoFormat->setCurrentIndex(index);
    }

    cbDSM->setChecked(withDSM_);

    {
        int index = 0;
        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
        {
            if (tdom_format_ == AI3D::CORE::dsm_format_e::DSM_FORMAT_XYZ)
            {
                index = 1;
            }
        }
        cbbDSMFormat->setCurrentIndex(index);
    }

    SwitchOrthophotoChecked();
    SwitchDSMChecked();
}

void ExportOrthophoto_DSM::Slot_ModeChanged(const QString& str)
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (str == "RapidMosaic")
    {
        tdommode_ = AI3D::CORE::tdom_mode_e::RAPIDMOSAIC;
    }
    else if (str == "Normal")
    {
        tdommode_ = AI3D::CORE::tdom_mode_e::NORMAL;
    }
    else if (str == "Low")
    {
        tdommode_ = AI3D::CORE::tdom_mode_e::LOW;
    }
    /*else if (str == "Fast")
    {
        tdommode_ = AI3D::CORE::tdom_mode_e::FASTMOSAIC;
    }*/
}

void ExportOrthophoto_DSM::Slot_DSMChecked()
{
    if (cbDSM->isChecked())
    {
        std::cout << "withDSM checked." << std::endl;
        withDSM_ = true;
        ///cbbDSMFormat->setEnabled(true);
    }
    else
    {
        std::cout << "withDSM unchecked." << std::endl;
        withDSM_ = false;
        ///cbbDSMFormat->setEnabled(false);
    }

    SwitchDSMChecked();
}

void ExportOrthophoto_DSM::SwitchDSMChecked()
{
    if (withDSM_)
    {
        cbbDSMFormat->setEnabled(true);
    }
    else
    {
        cbbDSMFormat->setEnabled(false);
    }
}

void ExportOrthophoto_DSM::Slot_OrthophotoChecked()
{
    if (cbOrthophoto->isChecked())
    {
        std::cout << "tdom checked." << std::endl;
        withTDOM_ = true;
    }
    else
    {
        std::cout << "tdom unchecked." << std::endl;
        withTDOM_ = false;
    }

    SwitchOrthophotoChecked();
}

void ExportOrthophoto_DSM::SwitchOrthophotoChecked()
{
    if (withTDOM_)
    {
        cbbOrthophotoFormat->setEnabled(true);
    }
    else
    {
        cbbOrthophotoFormat->setEnabled(false);
    }
}


Export_PointCloud_GS::Export_PointCloud_GS(ParamSettings4Production* parent)
    : QWidget(parent)
{
    paramSettings4Production = parent;
}

Export_PointCloud_GS::~Export_PointCloud_GS()
{

}

void Export_PointCloud_GS::DefaultParams()
{
    std::cout << "gauss default param." << std::endl;
   /* max_image_dim_ = 4096;
    withTDOM_ = true;

    tdom_format_ = AI3D::CORE::tdom_format_e::TDOM_FORMAT_TIFFGEOTIFF;
    sampling_distance_ = default_sampling_distance_;

    withDSM_ = true;
    dsm_format_ = AI3D::CORE::dsm_format_e::DSM_FORMAT_TIFFGEOTIFF;
    tdommode_ = AI3D::CORE::tdom_mode_e::NORMAL;*/
}

void Export_PointCloud_GS::Init()
{
    std::cout << "gauss param init." << std::endl;
    if (bInited)
    {
        // note: to refresh display controls based on the lastest data.
        Reset();
        return;
    }

    vlTop = new QVBoxLayout();
    //vlTop->setContentsMargins(0, 27, 42, 0);
    vlTop->setContentsMargins(0, 27, 0, 20);
    vlTop->setSpacing(5);

    QHBoxLayout* hlTitle = new QHBoxLayout();
    hlTitle->setContentsMargins(42, 0, 0, 0);

    lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblTitle->setText("格式/选项");
    }
    else
    {
        lblTitle->setText("Format/Options");
    }

    lblTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-left:0px;\n"
        "padding-top:0px;\n"
        "font:14px \"Arial\";\n"
        "}\n"
    ));

    //lblTitle->setAlignment(Qt::AlignLeft);
    hlTitle->addWidget(lblTitle, 0, Qt::AlignLeft);
    //hlTitle->addStretch(0);

    QHBoxLayout* hlFormat;
    hlFormat = new QHBoxLayout();
    hlFormat->setContentsMargins(42, 0, 0, 0);
    hlFormat->setSpacing(5);

    lblFormat = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblFormat->setText("格式:");
    }
    else
    {
        lblFormat->setText("Format:");
    }

    lblFormat->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    // note:default at OSGB option.
    cbbFormat = new QComboBox(this);
    cbbFormat->addItem("Polygon file format(PLY)");

    /*if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() != 2)
    {
            QVariant zerov(0);
            cbbFormat->setItemData(1, zerov, Qt::UserRole - 1);
            cbbFormat->setItemData(1, QBrush(QColor(192, 192, 192)), Qt::BackgroundRole);
    }*/

    int currentIndex = 0;
    ///bool bLodType = true;
    
    cbbFormat->setCurrentIndex(currentIndex);

    // QComboBox QAbstractItemView
    //      "    border-radius:0px;\n"  

    cbbFormat->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:36px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 38px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    cbbFormat->setItemDelegate(itemDelegate);
    format_ = static_cast<AI3D::CORE::gs_3d_format_e>(currentIndex);

    hlFormat->addWidget(lblFormat);
    hlFormat->addWidget(cbbFormat, 3);
    hlFormat->addStretch(1);

    QFrame* topLine = new QFrame(this);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Shadow::Plain);
    //topLine->setStyleSheet("width:899px;height:1px;border-radius:0px;color:#3D434E;");
    topLine->setStyleSheet("max-height:1px;border-radius:0px;border:none;background-color:#3D434E;margin-left:19px;margin-right:11px;");

    paramTitle = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        paramTitle->setText("场景参数");
    }
    else {
        paramTitle->setText("Purpose of production definition");
    }

    paramTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    cbSceneFly = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbSceneFly->setText("航飞");
    }
    else
    {
        cbSceneFly->setText("fly");
    }

    cbSceneFly->setStyleSheet(QString::fromUtf8(
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        "QCheckBox:disabled { color:#4E5562; } \n"
        ""));

    cbSceneIndoor = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbSceneIndoor->setText("室内");
    }
    else
    {
        cbSceneIndoor->setText("indoor");
    }

    cbSceneIndoor->setStyleSheet(QString::fromUtf8(
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        "QCheckBox:disabled { color:#4E5562; } \n"
        ""));

    cbSceneObject = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbSceneObject->setText("单体对象");
    }
    else
    {
        cbSceneObject->setText("object");
    }

    cbSceneObject->setStyleSheet(QString::fromUtf8(
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "margin-left:42px;\n"
        "font:14px \"Arial\";\n}"
        "QCheckBox:disabled { color:#4E5562; } \n"
        ""));
  
    QFrame* bottomLine = nullptr;
 
    vlTop->setSpacing(20);
    //vlTop->addWidget(lblTitle);
    vlTop->addLayout(hlTitle);
    vlTop->addLayout(hlFormat);
    vlTop->addWidget(topLine);

    vlTop->addWidget(paramTitle);
    vlTop->addWidget(cbSceneFly);
    vlTop->addWidget(cbSceneIndoor);
    vlTop->addWidget(cbSceneObject);

    vlTop->addStretch(1);
    connect(cbSceneFly, &QCheckBox::clicked, this, &Export_PointCloud_GS::Slot_SceneFly);
    connect(cbSceneIndoor, &QCheckBox::clicked, this, &Export_PointCloud_GS::Slot_SceneIndoor);
    connect(cbSceneObject, &QCheckBox::clicked, this, &Export_PointCloud_GS::Slot_SceneObject);

    connect(cbbFormat, &QComboBox::currentTextChanged, this, &Export_PointCloud_GS::Slot_Format);

    setLayout(vlTop);

    bInited = true;
}

void Export_PointCloud_GS::Reset()
{
    std::cout << "gauss param reset." << std::endl;
    if (paramSettings4Production != nullptr)
    {
        switch (paramSettings4Production->scene_type)
        {
        case AI3D::CORE::GS_SCENE_FLY:
            cbSceneFly->setChecked(true);
            cbSceneIndoor->setChecked(false);
            cbSceneObject->setChecked(false);
            //paramSettings4Production->export3D_Point_Cloud->SetValid(false);


            //paramSettings4Production->exportOrthophoto_DSM->SetValid(false);
            //export3DMesh4ExternalRetouching_FormatWithOptions->SetValid(false);

            break;
        case AI3D::CORE::GS_SCENE_INDOOR:
            cbSceneIndoor->setChecked(true);
            cbSceneFly->setChecked(false);
            cbSceneObject->setChecked(false);

           /* paramSettings4Production->export3DMesh_FormatWithOptions->SetInValid();
            paramSettings4Production->spatialReferenceSystem->shouldSetOrigin_ = false;*/
            break;
        case AI3D::CORE::GS_SCENE_OBJECT:
            cbSceneObject->setChecked(true);
            cbSceneIndoor->setChecked(false);
            cbSceneFly->setChecked(false);
            

            //cbExport3DMesh4ExternalRetouching->setChecked(false);
            /*paramSettings4Production->spatialReferenceSystem->shouldSetOrigin_ = false;*/
            break;


        default:
            break;
        }
    }

}

void Export_PointCloud_GS::ValidParams()
{
    std::cout << "gauss param reset." << std::endl;
}

void Export_PointCloud_GS::SetInValid()
{

}

bool Export_PointCloud_GS::IsValid()
{
    std::cout << "gauss param is valid." << std::endl;
    return true;
}

void Export_PointCloud_GS::Slot_SceneFly()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (paramSettings4Production != nullptr)
    {
        if (paramSettings4Production->scene_type != AI3D::CORE::GS_SCENE_FLY)
        {
            paramSettings4Production->scene_chosen_dirty = true;
            paramSettings4Production->scene_type = AI3D::CORE::GS_SCENE_FLY;
        }
        scene_type_ = AI3D::CORE::gs_scene_e::GS_SCENE_FLY;
        Reset();
    }
}

void Export_PointCloud_GS::Slot_SceneIndoor()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (paramSettings4Production != nullptr)
    {
        if (paramSettings4Production->scene_type != AI3D::CORE::GS_SCENE_INDOOR)
        {
            paramSettings4Production->scene_chosen_dirty = true;
            paramSettings4Production->scene_type = AI3D::CORE::GS_SCENE_INDOOR;
        }
        scene_type_ = AI3D::CORE::gs_scene_e::GS_SCENE_INDOOR;
        Reset();
    }
}

void Export_PointCloud_GS::Slot_SceneObject()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (paramSettings4Production != nullptr)
    {
        if (paramSettings4Production->scene_type != AI3D::CORE::GS_SCENE_OBJECT)
        {
            paramSettings4Production->scene_chosen_dirty = true;
            paramSettings4Production->scene_type = AI3D::CORE::GS_SCENE_OBJECT;
        }
        scene_type_ = AI3D::CORE::gs_scene_e::GS_SCENE_OBJECT;
        Reset();
    }
}

void Export_PointCloud_GS::Slot_Format(const QString& str)
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    std::string strFormat = str.QString::toStdString();
    if (strFormat == "Polygon file format(PLY)")
    {
        format_ = AI3D::CORE::gs_3d_format_e::GS_3D_FORMAT_PLY;

    }
}

//Export3DMesh4ExternalRetouching_FormatWithOptions::Export3DMesh4ExternalRetouching_FormatWithOptions(ParamSettings4Production* parent)
//  : QWidget(parent)
//{
//  //Init();
//}


//  Export3DMesh4ExternalRetouching_FormatWithOptions::~Export3DMesh4ExternalRetouching_FormatWithOptions()
//  {
//
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::DefaultParams()
//  {
//      format_ = "OBJ";
//      withTexMaps_ = true;
//      texturecompression_ = 75;
//      max_texture_size_ = 8192;
//      withSparping_ = true;
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::Init()
//  {
//
//      vlTop = new QVBoxLayout();
//      vlTop->setContentsMargins(0, 27, 42, 0);
//      vlTop->setSpacing(10);
//
//      QHBoxLayout* hlTitle = new QHBoxLayout();
//      hlTitle->setContentsMargins(42, 0, 0, 0);
//
//      lblTitle = new QLabel(this);
//      lblTitle->setText("Format/Options");
//
//      lblTitle->setStyleSheet(QString::fromUtf8(
//          "QLabel {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "margin-top:0px;\n"
//          "margin-left:0px;\n"
//          "padding:0px;\n"
//          "font:14px solid;\n"
//          "border:none;}\n"
//      ));
//
//      hlTitle->addWidget(lblTitle);
//      hlTitle->addStretch(1);
//
//      QHBoxLayout* hlFormat;
//      hlFormat = new QHBoxLayout();
//      hlFormat->setContentsMargins(42, 27, 0, 0);
//      hlFormat->setSpacing(5);
//
//      lblFormat = new QLabel(this);
//      lblFormat->setText("Format:");
//
//      lblFormat->setStyleSheet(QString::fromUtf8(
//          "QLabel {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "padding-top:0px;\n"
//          "padding-left:0px;\n"
//          "font:14px solid;\n}"
//          ""));
//
//#if 0
//      cbbFormat = new QComboBox(this);
//      cbbFormat->addItem("OpenSceneGraph Binary system (OSGB)");
//      cbbFormat->addItem("3D TILES");
//      cbbFormat->addItem("OBJ wavefront format");
//      cbbFormat->addItem("PLY");
//      cbbFormat->setCurrentIndex(0);
//      cbbFormat->setEnabled(false);
//
//      cbbFormat->setStyleSheet(QString::fromUtf8("\n"
//          "QComboBox {\n"
//          "    border: 0px solid gray;   \n"
//          "    border-radius: 0px;   \n"
//          "    color: #000000;\n"
//          "   font: 14px \"Arial\";\n"
//          "   background-color:white;\n"
//          "   margin-left:0px; \n"
//          "   margin-right:0px; \n"
//          "   padding-left: 3px\n"
//          "}\n"
//          "QComboBox:disabled {\n"
//          "   color: white;\n"
//          "   background-color:gray;\n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView {\n"
//          "    outline: 0px solid gray;   \n"
//          "    border: 0px solid;   \n"
//          "    color:#FFFFFF;\n"
//          "    background-color: #131313;  \n"
//          "    selection-background-color:#333333;   \n"
//          "    padding-left: 0px; \n"
//          "    margin-left:0px; \n"
//          "    margin-right:0px; \n"
//          "}\n"
//          "QComboBox QAbstractScrollArea {\n"
//          "    width: 10px;\n"
//          "    color: black; \n"
//          "    background-color:white;\n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView::item {\n"
//          "    height: 50px;   \n"
//          "    background-color:white;\n"
//          "    padding-left: 0px; \n"
//          "    margin-left:0px; \n"
//          "    margin-right:0px; \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView::item:hover {\n"
//          "    color: #FFFFFF;\n"
//          "    background-color: rgb(22,22,22);   \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView::item:selected {\n"
//          "    color: #FFFFFF;\n"
//          "    background-color:rgb(22,22,22);\n"
//          "}\n"
//          "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
//          "    width: 10px;\n"
//          "    background-color: #d0d2d4;  \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
//          "    border-radius: 5px;   "
//          "    background: rgb(160,160"
//          ",160);   \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
//          "    background: rgb(90, 91, 93);   \n"
//          "}\n"
//      ));
//#else
//      leFormat = new QLineEdit(this);
//      leFormat->setText("OBJ wavefront format");
//      leFormat->setStyleSheet(QString::fromUtf8(
//          "QLineEdit {background-color:#34363A;\n"
//          "color:#FFFFFF;\n"
//          "margin-top:0px;\n"
//          "margin-left:0px;\n"
//          "padding:0px;\n"
//          "padding-left:11px;\n"
//          "font:14px solid;\n"
//          "border-radius:4px;\n"
//          "border:none;\n"
//          "}\n"
//      ));
//      leFormat->setFixedHeight(36);
//      leFormat->setEnabled(false);
//#endif
//
//      hlFormat->addWidget(lblFormat);
//      ///hlFormat->addWidget(cbbFormat, 3);
//      hlFormat->addWidget(leFormat, 3);
//      hlFormat->addStretch(1);
//
//      cbIncludeTextureMaps = new QCheckBox(this);
//      cbIncludeTextureMaps->setText("Include texture maps");
//      cbIncludeTextureMaps->setStyleSheet(QString::fromUtf8(
//          "QCheckBox {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "margin-top:27px;\n"
//          "margin-left:42px;\n"
//          "font:14px solid;\n}"
//          ""));
//      cbIncludeTextureMaps->setChecked(withTexMaps_);
//
//      hlTextureCompression = new QHBoxLayout();
//      hlTextureCompression->setContentsMargins(42, 27, 0, 0);
//      hlTextureCompression->setSpacing(5);
//
//      lblTextureCompression = new QLabel(this);
//      lblTextureCompression->setText("Texture compression");
//
//      lblTextureCompression->setStyleSheet(QString::fromUtf8(
//          "QLabel {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "padding-top:0px;\n"
//          "padding-left:0px;\n"
//          "margin-left:42px;\n"
//          "font:14px solid;\n}"
//          ""));
//
//      // note:default at 75% option.
//      cbbTextureCompression = new QComboBox(this);
//      cbbTextureCompression->addItem(PERCENT_100_QUALITY_JPEG);
//      cbbTextureCompression->addItem(PERCENT_90_QUALITY_JPEG);
//      cbbTextureCompression->addItem(PERCENT_75_QUALITY_JPEG);
//      cbbTextureCompression->addItem(PERCENT_50_QUALITY_JPEG);
//
//      int currIndex = 2;
//      switch (texturecompression_)
//      {
//      case 100:
//          currIndex = 0;
//          break;
//      case 90:
//          currIndex = 1;
//          break;
//      case 75:
//          currIndex = 2;
//          break;
//      case 50:
//          currIndex = 3;
//          break;
//      }
//
//      cbbTextureCompression->setCurrentIndex(currIndex);
//
//      cbbTextureCompression->setStyleSheet(QString::fromUtf8("\n"
//          "QComboBox {\n"
//          "    border: 0px solid gray;   \n"
//          "    border-radius: 0px;   \n"
//          "    color: #000000;\n"
//          "   font: 14px \"Arial\";\n"
//          "   background-color:white;\n"
//          "   margin-left:0px; \n"
//          "   margin-right:0px; \n"
//          "   padding-left: 3px\n"
//          "}\n"
//          "QComboBox:disabled {\n"
//          "   color: white;\n"
//          "   background-color:gray;\n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView {\n"
//          "    outline: 0px solid gray;   \n"
//          "    border: 0px solid;   \n"
//          "    color:#FFFFFF;\n"
//          "    background-color: #131313;  \n"
//          "    selection-background-color:#333333;   \n"
//          "    padding-left: 0px; \n"
//          "    margin-left:0px; \n"
//          "    margin-right:0px; \n"
//          "}\n"
//          "QComboBox QAbstractScrollArea {\n"
//          "    width: 10px;\n"
//          "    color: black; \n"
//          "    background-color:white;\n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView::item {\n"
//          "    height: 50px;   \n"
//          "    background-color:white;\n"
//          "    padding-left: 0px; \n"
//          "    margin-left:0px; \n"
//          "    margin-right:0px; \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView::item:hover {\n"
//          "    color: #FFFFFF;\n"
//          "    background-color: rgb(22,22,22);   \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractItemView::item:selected {\n"
//          "    color: #FFFFFF;\n"
//          "    background-color:rgb(22,22,22);\n"
//          "}\n"
//          "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
//          "    width: 10px;\n"
//          "    background-color: #d0d2d4;  \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
//          "    border-radius: 5px;   "
//          "    background: rgb(160,160"
//          ",160);   \n"
//          "}\n"
//          "\n"
//          "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
//          "    background: rgb(90, 91, 93);   \n"
//          "}\n"
//      ));
//
//      hlTextureCompression->addWidget(lblTextureCompression);
//      hlTextureCompression->addWidget(cbbTextureCompression);
//      hlTextureCompression->addStretch(1);
//
//      hlMaximumTextureSize = new QHBoxLayout();
//      hlMaximumTextureSize->setContentsMargins(42, 27, 0, 0);
//      hlMaximumTextureSize->setSpacing(5);
//
//      lblMaximumTextureSize = new QLabel(this);
//      lblMaximumTextureSize->setText("Maximum texture size:");
//
//      lblMaximumTextureSize->setStyleSheet(QString::fromUtf8(
//          "QLabel {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "padding-top:0px;\n"
//          "padding-left:0px;\n"
//          "margin-left:42px;\n"
//          "font:14px solid;\n}"
//          ""));
//
//      leMaximumTextureSize = new QLineEdit(this);
//
//      leMaximumTextureSize->setStyleSheet(QString::fromUtf8(
//          "QLineEdit {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "padding-top:0px;\n"
//          "padding-left:0px;\n"
//          "font:14px solid;\n}"
//          ""));
//      QIntValidator* intValidator = new QIntValidator(this);
//      leMaximumTextureSize->setValidator(intValidator);
//      leMaximumTextureSize->setText(QString::number(max_texture_size_));
//
//      lblMaximumTextureSizePixel = new QLabel(this);
//      lblMaximumTextureSizePixel->setText("pixel");
//
//      lblMaximumTextureSizePixel->setStyleSheet(QString::fromUtf8(
//          "QLabel {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "padding-top:0px;\n"
//          "padding-left:0px;\n"
//          "font:14px solid;\n}"
//          ""));
//
//      hlMaximumTextureSize->addWidget(lblMaximumTextureSize);
//      hlMaximumTextureSize->addWidget(leMaximumTextureSize);
//      hlMaximumTextureSize->addWidget(lblMaximumTextureSizePixel);
//      hlMaximumTextureSize->addStretch(1);
//
//      hlSharpening = new QHBoxLayout();
//      hlSharpening->setContentsMargins(42, 27, 0, 0);
//      lblSharpening = new QLabel(this);
//      lblSharpening->setText("Texture sharpening:");
//
//      lblSharpening->setStyleSheet(QString::fromUtf8(
//          "QLabel {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "padding-top:0px;\n"
//          "padding-left:0px;\n"
//          "padding-right:0px;\n"
//          "margin-left:42px;\n"
//          "font:14px solid;\n}"
//          ""));
//
//      cbSharpening = new QCheckBox(this);
//      cbSharpening->setText("");
//      cbSharpening->setStyleSheet(QString::fromUtf8(
//          "QCheckBox {background-color:#2D3035;\n"
//          "color:#FFFFFF;\n"
//          "padding-top:0px;\n"
//          "padding-left:0px;\n"
//          "margin-left:0px;\n"
//          "font:14px solid;\n}"
//          ""));
//      cbSharpening->setChecked(withSparping_);
//
//      hlSharpening->setSpacing(0);
//      hlSharpening->addWidget(lblSharpening);
//      hlSharpening->addWidget(cbSharpening);
//      hlSharpening->addStretch(1);
//
//      vlTop->setSpacing(0);
//      ///vlTop->addWidget(lblTitle);
//      vlTop->addLayout(hlTitle);
//      vlTop->addLayout(hlFormat);
//      vlTop->addWidget(cbIncludeTextureMaps);
//      vlTop->addLayout(hlTextureCompression);
//      vlTop->addLayout(hlMaximumTextureSize);
//      vlTop->addLayout(hlSharpening);
//      vlTop->addStretch(1);
//
//      connect(cbbTextureCompression, &QComboBox::currentTextChanged, this, &Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_TextureCompression);
//      connect(cbIncludeTextureMaps, &QCheckBox::clicked, this, &Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_IncludeTextureMaps);
//      connect(cbSharpening, &QCheckBox::clicked, this, &Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_SharpeningChecked);
//      connect(leMaximumTextureSize, &QLineEdit::editingFinished, this, &Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_MaxTextureSize);
//
//      SwitchIncludeTextureMaps();
//
//      setLayout(vlTop);
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::Reset()
//  {
//
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_TextureCompression(const QString & str)
//  {
//      if (str == PERCENT_100_QUALITY_JPEG)
//          texturecompression_ = 100;
//      else if (str == PERCENT_90_QUALITY_JPEG)
//          texturecompression_ = 90;
//      else if (str == PERCENT_75_QUALITY_JPEG)
//          texturecompression_ = 75;
//      else if (str == PERCENT_50_QUALITY_JPEG)
//          texturecompression_ = 50;
//
//      std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str.toStdString() << " " << texturecompression_ << std::endl;
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_IncludeTextureMaps()
//  {
//      if (cbIncludeTextureMaps->isChecked())
//      {
//          withTexMaps_ = true;
//      }
//      else
//      {
//          withTexMaps_ = false;
//      }
//
//      SwitchIncludeTextureMaps();
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::SwitchIncludeTextureMaps()
//  {
//      if (withTexMaps_)
//      {
//          cbbTextureCompression->setEnabled(true);
//          leMaximumTextureSize->setEnabled(true);
//          cbSharpening->setEnabled(true);
//      }
//      else
//      {
//          cbbTextureCompression->setEnabled(false);
//          leMaximumTextureSize->setEnabled(false);
//          cbSharpening->setEnabled(false);
//      }
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_SharpeningChecked()
//  {
//      if (cbSharpening->isChecked())
//      {
//          std::cout << "sharpening true." << std::endl;
//          withSparping_ = true;
//      }
//      else
//      {
//          std::cout << "sharpening false." << std::endl;
//          withSparping_ = false;
//      }
//  }
//
//  void Export3DMesh4ExternalRetouching_FormatWithOptions::Slot_MaxTextureSize()
//  {
//      QString str = leMaximumTextureSize->text();
//      max_texture_size_ = std::atoi(str.toStdString().c_str());
//      std::cout << "include " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << leMaximumTextureSize->text().toStdString()
//          << " / " << max_texture_size_ << std::endl;
//  }

BasicSettings::BasicSettings(ParamSettings4Production* parent)
    : QWidget(parent)
{
    //Init();
    bErrorInfoVisible = false;
    paramSettings4Production = parent;
}

BasicSettings::~BasicSettings()
{

}

void BasicSettings::Init()
{
    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 0, 0, 0);

    QWidget* firstLineWidget = new QWidget(this);
    QWidget* secondLineWidget = new QWidget(this);
    QWidget* thirdLineWidget = new QWidget(this);

    firstLineWidget->setFixedHeight(55);
    secondLineWidget->setFixedHeight(55);
    thirdLineWidget->setFixedHeight(55);

    //firstLineWidget->setVisible(false);
    //secondLineWidget->setVisible(false);
    //thirdLineWidget->setVisible(false);

    QHBoxLayout* hlID = new QHBoxLayout();
    hlID->setContentsMargins(41, 0, 62, 0);
    lblProductionTitle = new QLabel(this);

    QFont font = lblProductionTitle->font();
    font.setPixelSize(14);

    lblProductionTitle->setText("ID:");
    lblProductionTitle->setStyleSheet("background-color:transparent;color:#FFFFFF;font:14px \"Arial\";");
    //lblProductionTitle->setFont(font);

    lblProductionID = new QLabel(this);

    if (production_id_ != kInvalidProductionId)
    {
        std::string idstr = PRODUCTION_PREFIX + std::to_string(production_id_);
        QString text = QString::fromStdString(idstr);
        lblProductionID->setText(text);
    }

    lblProductionID->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblProductionID->setStyleSheet("background-color:transparent;color:#FFFFFF;font:14px \"Arial\";");
    //lblProductionID->setFont(font);

    hlID->addWidget(lblProductionTitle);
    hlID->addWidget(lblProductionID);
    hlID->addStretch(1);

    firstLineWidget->setLayout(hlID);

    QHBoxLayout* hlName = new QHBoxLayout();
    hlName->setContentsMargins(41, 0, 62, 0);
    lblName = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblName->setText("名称:");
    }
    else
    {
        lblName->setText("Name:");
    }
    lblName->setStyleSheet("background-color:transparent;color:#FFFFFF;font:14px \"Arial\";");
    lblName->setFont(font);

    leName = new QLineEdit(this);

    leName->setText(QString::fromStdString(name_));
    //leName->setFont(font);
    leName->setStyleSheet("background-color:#20242B;color:#FFFFFF;height:36px;font:14px \"Arial\";border:1px solid #404040;border-radius:2px;padding-left:12px;");
    QRegExp rx4Name("[^*?:\"<\\\\>/| ]+");
    QValidator* validator = new QRegExpValidator(rx4Name);
    leName->setValidator(validator);

    hlName->addWidget(lblName);
    hlName->addWidget(leName);
    secondLineWidget->setLayout(hlName);

    QHBoxLayout* hlDestination = new QHBoxLayout();
    hlDestination->setContentsMargins(41, 0, 62, 0);

    lblDestination = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblDestination->setText("目的路径:");
    }
    else {
        lblDestination->setText("Destination:");
    }
    lblDestination->setStyleSheet("background-color:transparent;color:#FFFFFF;font:14px \"Arial\";");
    //lblDestination->setFont(font);

    leDestination = new QLineEdit(this);
    leDestination->setText(QString::fromStdString(desination_));
    //leDestination->setFont(font);
    QRegExp rx4Name2("[^*?\"<>| ]+");
    QValidator* validator2 = new QRegExpValidator(rx4Name2);
    leDestination->setValidator(validator2);
    leDestination->setStyleSheet("background-color:#20242B;color:#FFFFFF;height:36px;font:14px \"Arial\";border:1px solid #404040;border-radius:2px;padding-left:12px;");

    butDestination = new QPushButton(this);
    //butDestination->setText("...");
    //butDestination->setFont(font);
    butDestination->setIcon(QPixmap(":/new/prefix1/skin/openfolder1818.png"));
    butDestination->setStyleSheet("background-color:#20242B;color:#FFFFFF;font:14px \"Arial\";width:41px;height:36px;border-radius:2px;border:1px solid #404040;");

    lblError = new QLabel(this);
    lblError->setText(QString("Error: \"%1\" has already existed, please choose another new name.").arg(leName->text()));
    lblError->setStyleSheet("background-color:transparent;color:#FF4B4B;font:14px \"Arial\";padding-left:41px;margin-bottom:30px;");

    hlDestination->setSpacing(0);
    //hlDestination->addStretch(1);
    hlDestination->addWidget(lblDestination);
    hlDestination->addSpacing(10);
    hlDestination->addWidget(leDestination);
    hlDestination->addSpacing(3);
    hlDestination->addWidget(butDestination);
    //hlDestination->addStretch(1);
    thirdLineWidget->setLayout(hlDestination);

    vlTop->addSpacing(10);
    //vlTop->setSpacing(25);
    vlTop->setSpacing(0);
    //vlTop->addLayout(hlID);
    //vlTop->addLayout(hlName);
    //vlTop->addLayout(hlDestination);
    vlTop->addWidget(firstLineWidget);
    vlTop->addWidget(secondLineWidget);
    vlTop->addWidget(thirdLineWidget);
    vlTop->addStretch(1);
    vlTop->addWidget(lblError);

    connect(leName, &QLineEdit::editingFinished, this, &BasicSettings::Slot_SetName);
    connect(leName, &QLineEdit::textEdited, this, &BasicSettings::Slot_TextEdited);
    connect(leDestination, &QLineEdit::editingFinished, this, &BasicSettings::Slot_SetDestination);
    connect(leDestination, &QLineEdit::textEdited, this, &BasicSettings::Slot_TextEdited);

    connect(butDestination, &QPushButton::clicked, this, &BasicSettings::Slot_ChooseFolder);

    SetErrorInfoVisible(false); // for test purpose only.
    //RefreshErrorInfo();

    setLayout(vlTop);
}

void BasicSettings::SetErrorInfoVisible(bool bVis)
{
    bErrorInfoVisible = bVis;
    RefreshErrorInfo();
}

void BasicSettings::RefreshErrorInfo()
{
    if (bErrorInfoVisible)
        lblError->setVisible(true);
    else
        lblError->setVisible(false);
}

void BasicSettings::ChangeEnabledState4NextButton()
{
    // note: checking whether name and/or destination is/are empty, and then change enabled state of next button.
    if (leName->text().isEmpty() || leDestination->text().isEmpty())
    {
        paramSettings4Production->butNext->setEnabled(false);
    }
    else
    {
        paramSettings4Production->butNext->setEnabled(true);
    }
}

void BasicSettings::Slot_TextEdited()
{
    ChangeEnabledState4NextButton();
}

//此处应该需要加相关的检查处理
void BasicSettings::Slot_SetName()
{
    name_ = leName->text().toStdString();

    bool bProductionNameExists = false;

    if (!name_.empty() && paramSettings4Production && paramSettings4Production->recons_object)
    {
        AI3D::CORE::ReconstructionObject* recons_object = paramSettings4Production->recons_object;
        auto& productions = recons_object->GetProductions();
        for (auto& production : productions)
        {
            if (production.first != this->production_id_)
            {
                if (production.second->GetName() == name_)
                {
                    bProductionNameExists = true;
                }
            }
        }
    }

    SetErrorInfoVisible(bProductionNameExists);
    ChangeEnabledState4NextButton();
}

void BasicSettings::Slot_SetDestination()
{
    desination_ = leDestination->text().toStdString();
    ChangeEnabledState4NextButton();
}

void BasicSettings::Slot_ChooseFolder()
{
    QString dstDirPath = QFileDialog::getExistingDirectory(
        this, "Choose Destination Directory", leDestination->text().isEmpty() ? "./" : leDestination->text());

    if (!dstDirPath.isEmpty())
    {
        leDestination->setText(dstDirPath);
        desination_ = leDestination->text().toStdString();

        // note: may need to adjust enabled state based on whether name and/or destionation are empty.
        ChangeEnabledState4NextButton();
        return;
    }
}

Purpose4ProductionDefinition::Purpose4ProductionDefinition(ParamSettings4Production* parent)
    : QWidget(parent)
{
    paramSettings4Production = parent;
    //Init();
}

Purpose4ProductionDefinition::~Purpose4ProductionDefinition()
{

}

void Purpose4ProductionDefinition::Init()
{
    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(42, 27, 42, 27);

    lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblTitle->setText("生产目的");
    }
    else {
        lblTitle->setText("Purpose of production definition");
    }

    lblTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        cbReferenceModel = new QCheckBox(this);
        if (BlockObject::isChineseVersion())
        {
            cbReferenceModel->setText("参考模型");
        }
        else
        {
            cbReferenceModel->setText("Reference Model");
        }
        cbReferenceModel->setEnabled(false);
        cbReferenceModel->setChecked(true);

        cbReferenceModel->setStyleSheet(QString::fromUtf8(
            "QCheckBox {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            "QCheckBox:disabled { color:#4E5562;} \n"
            ""
        ));
    }


    cbExport3DMesh = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbExport3DMesh->setText("输出三维模型");
    }
    else
    {
        cbExport3DMesh->setText("Export 3D mesh");
    }

    cbExport3DMesh->setStyleSheet(QString::fromUtf8(
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        "QCheckBox:disabled { color:#4E5562; } \n"
        ""));

    cbExportPointCloudGS = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbExportPointCloudGS->setText("输出高斯泼溅");
    }
    else
    {
        cbExportPointCloudGS->setText("Export GS");
    }

    cbExportPointCloudGS->setStyleSheet(QString::fromUtf8(
        "QCheckBox {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        "QCheckBox:disabled { color:#4E5562; } \n"
        ""));


    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        cbExport3DPointCloud = new QCheckBox(this);
        if (BlockObject::isChineseVersion()) {
            cbExport3DPointCloud->setText("输出密集点云");
        }
        else {
            cbExport3DPointCloud->setText("Export 3D point cloud");
        }

        cbExport3DPointCloud->setStyleSheet(QString::fromUtf8(
            "QCheckBox {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            "QCheckBox:disabled { color:#4E5562; } \n"
            ""));
    }

    /*if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() != 2)
    {
        cbExport3DPointCloud->setEnabled(false);
        cbExport3DPointCloud->setChecked(false);

        cbExport3DPointCloud->setStyleSheet(QString::fromUtf8(
            "QCheckBox {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            "QCheckBox:disabled { color:#4E5562;} \n"
            ""
        ));
    }*/

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        cbExportOrthophotoDSM = new QCheckBox(this);
        if (BlockObject::isChineseVersion()) {
            cbExportOrthophotoDSM->setText("输出TDOM/DSM");
        }
        else {
            cbExportOrthophotoDSM->setText("Export Orthophoto/DSM");
        }

        cbExportOrthophotoDSM->setStyleSheet(QString::fromUtf8(
            "QCheckBox {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px \"Arial\";\n}"
            "QCheckBox:disabled { color:#4E5562; } \n"
            ""));
    }

    std::string definition = paramSettings4Production->spatialReferenceSystem->definition_;
    srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(definition);
    /*if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() != 2)
    {
        if (srs.type == LOCAL)
        {
            cbExportOrthophotoDSM->setEnabled(false);
        }

    }*/
    /*if (0)
    {
        cbExport3DMesh4ExternalRetouching = new QCheckBox(this);
        cbExport3DMesh4ExternalRetouching->setText("Export 3D mesh for external retouching");

        cbExport3DMesh4ExternalRetouching->setStyleSheet(QString::fromUtf8(
            "QCheckBox {background-color:#2D3035;\n"
            "color:#FFFFFF;\n"
            "padding-top:0px;\n"
            "padding-left:0px;\n"
            "font:14px solid;\n}"
            ""));
    }*/
    //vlTop->addSpacing(10);
    vlTop->setSpacing(17);
    vlTop->addWidget(lblTitle);
    vlTop->addSpacing(16);
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        vlTop->addWidget(cbReferenceModel);
    }
    vlTop->addWidget(cbExport3DMesh);
    vlTop->addWidget(cbExportPointCloudGS);

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        vlTop->addWidget(cbExport3DPointCloud);
        vlTop->addWidget(cbExportOrthophotoDSM);
    }


    //vlTop->addWidget(cbExport3DMesh4ExternalRetouching);
    vlTop->addStretch(1);
    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        connect(cbReferenceModel, &QCheckBox::clicked, this, &Purpose4ProductionDefinition::Slot_ReferenceModel);
    }
    connect(cbExport3DMesh, &QCheckBox::clicked, this, &Purpose4ProductionDefinition::Slot_Export3DMesh);
    connect(cbExportPointCloudGS, &QCheckBox::clicked, this, &Purpose4ProductionDefinition::Slot_ExportPiontCloudGS);

    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
    {
        connect(cbExport3DPointCloud, &QCheckBox::clicked, this, &Purpose4ProductionDefinition::Slot_Export3DPointCloud);

        connect(cbExportOrthophotoDSM, &QCheckBox::clicked, this, &Purpose4ProductionDefinition::Slot_ExportOrthophotoDSM);
    }
    //connect(cbExport3DMesh4ExternalRetouching, &QCheckBox::clicked, this, &Purpose4ProductionDefinition::Slot_Export3DMesh4ExternalRetouching);

    setLayout(vlTop);

    Reset();
}

void Purpose4ProductionDefinition::Reset()
{
    if (paramSettings4Production != nullptr)
    {
        switch (paramSettings4Production->production_purpose)
        {
        case AI3D::CORE::EXPORT_3D_MESH:
            cbExport3DMesh->setChecked(true);
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
            {
                cbExport3DPointCloud->setChecked(false);

                cbExportOrthophotoDSM->setChecked(false);
            }
            //cbExport3DMesh4ExternalRetouching->setChecked(false);
            cbExportPointCloudGS->setChecked(false);
            paramSettings4Production->export3D_Point_Cloud->SetValid(false);


            //paramSettings4Production->exportOrthophoto_DSM->SetValid(false);
            //export3DMesh4ExternalRetouching_FormatWithOptions->SetValid(false);

            break;
        case AI3D::CORE::EXPORT_3D_POINT_CLOUD:
            cbExport3DMesh->setChecked(false);
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
            {
                cbExport3DPointCloud->setChecked(true);

                cbExportOrthophotoDSM->setChecked(false);
            }
            //cbExport3DMesh4ExternalRetouching->setChecked(false);
            cbExportPointCloudGS->setChecked(false);
            paramSettings4Production->export3DMesh_FormatWithOptions->SetInValid();
            paramSettings4Production->spatialReferenceSystem->shouldSetOrigin_ = false;
            break;
        case AI3D::CORE::EXPORT_ORTHOPHOTO_DSM:
            cbExport3DMesh->setChecked(false);
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
            {
                cbExport3DPointCloud->setChecked(false);

                cbExportOrthophotoDSM->setChecked(true);
            }
            cbExportPointCloudGS->setChecked(false);
            //cbExport3DMesh4ExternalRetouching->setChecked(false);
            paramSettings4Production->spatialReferenceSystem->shouldSetOrigin_ = false;
            break;
            //case AI3D::CORE::EXPORT_3D_MESH_FOR_EXTERNAL_RETOUCHING:
            //  cbExport3DMesh->setChecked(false);
            //  cbExport3DPointCloud->setChecked(false);
            //  cbExportOrthophotoDSM->setChecked(false);
            //  //cbExport3DMesh4ExternalRetouching->setChecked(true);
            //  break;

        case AI3D::CORE::EXPORT_POINTCLOUD_GDGS:
            cbExport3DMesh->setChecked(false);
            cbExportPointCloudGS->setChecked(true);
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
            {
                cbExport3DPointCloud->setChecked(false);

                cbExportOrthophotoDSM->setChecked(false);
            }
            //cbExport3DMesh4ExternalRetouching->setChecked(false);
            paramSettings4Production->spatialReferenceSystem->shouldSetOrigin_ = false;
            break;

        default:
            break;
        }
    }
}

void Purpose4ProductionDefinition::Slot_ReferenceModel()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

}
void Purpose4ProductionDefinition::Slot_ExportPiontCloudGS()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (paramSettings4Production != nullptr)
    {
        if (paramSettings4Production->production_purpose != AI3D::CORE::EXPORT_POINTCLOUD_GDGS)
        {
            paramSettings4Production->purpose_chosen_dirty = true;
            paramSettings4Production->production_purpose = AI3D::CORE::EXPORT_POINTCLOUD_GDGS;
            Reset();
        }
    }
}


void Purpose4ProductionDefinition::Slot_Export3DMesh()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (paramSettings4Production != nullptr)
    {
        if (paramSettings4Production->production_purpose != AI3D::CORE::EXPORT_3D_MESH)
        {
            paramSettings4Production->purpose_chosen_dirty = true;
            paramSettings4Production->production_purpose = AI3D::CORE::EXPORT_3D_MESH;
            Reset();
        }
    }
}

void Purpose4ProductionDefinition::Slot_Export3DPointCloud()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (paramSettings4Production != nullptr)
    {
        if (paramSettings4Production->production_purpose != AI3D::CORE::EXPORT_3D_POINT_CLOUD)
        {
            paramSettings4Production->purpose_chosen_dirty = true;
            paramSettings4Production->production_purpose = AI3D::CORE::EXPORT_3D_POINT_CLOUD;
            Reset();
        }
    }
}

void Purpose4ProductionDefinition::Slot_ExportOrthophotoDSM()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (paramSettings4Production != nullptr)
    {
        if (paramSettings4Production->production_purpose != AI3D::CORE::EXPORT_ORTHOPHOTO_DSM)
        {
            paramSettings4Production->purpose_chosen_dirty = true;
            paramSettings4Production->production_purpose = AI3D::CORE::EXPORT_ORTHOPHOTO_DSM;
            Reset();
        }
    }
}

//void Purpose4ProductionDefinition::Slot_Export3DMesh4ExternalRetouching()
//{
//  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
//  if (paramSettings4Production != nullptr)
//  {
//      if (paramSettings4Production->production_purpose != AI3D::CORE::EXPORT_3D_MESH_FOR_EXTERNAL_RETOUCHING)
//      {
//          paramSettings4Production->purpose_chosen_dirty = true;
//          paramSettings4Production->production_purpose = AI3D::CORE::EXPORT_3D_MESH_FOR_EXTERNAL_RETOUCHING;
//          Reset();
//      }
//  }
//}

void Purpose4ProductionDefinition::Slot_ExportOption()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
}

SpatialReferenceSystem::SpatialReferenceSystem(ParamSettings4Production* parent)
    : QWidget(parent)
{
    setObjectName("SpatialReferenceSystem");
    ///Init();
}

SpatialReferenceSystem::~SpatialReferenceSystem()
{

}

void SpatialReferenceSystem::Init()
{
    //  setStyleSheet(QString::fromUtf8(
    //      "#SpatialReferenceSystem { \n"
    //      "background-color:red;\n"
    //      "}\n"
    //  ));

        ///setStyleSheet(QString::fromUtf8(
        /// "{ \n"
        /// "background-color:red;\n"
        /// "}\n"
        ///));
        //this->definition_ = this->default_definition_;

    vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(41, 27, 41, 27);

    lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblTitle->setText("空间参考系统");
    }
    else
    {
        lblTitle->setText("Spatial reference system");
    }

    QFont font = lblTitle->font();
    font.setPixelSize(14);

    lblTitle->setStyleSheet(QString::fromUtf8(
        "background-color:#2D3035;\n"
        "color:white;\n"
        "border-radius:0px;\n"
        "margin-left:0px;\n"
        "padding-left:0px;\n"
        "border:none;\n"
        "font:14px \"Arial\";\n"
    ));

    lblTitle->setFont(font);

    //lblTitle->setFixedWidth(200);
    //lblTitle->setFixedHeight(60);

    QHBoxLayout* hlSRS;
    hlSRS = new QHBoxLayout();
    hlSRS->setContentsMargins(0, 0, 0, 0);

    lblSRS = new QLabel(this);
    lblSRS->setText("SRS：");
    lblSRS->setFont(font);

    lblSRS->setStyleSheet(QString::fromUtf8(
        "background-color:#2D3035;\n"
        "color:white;\n"
        "border-radius:0px;\n"
        "margin-left:0px;\n"
        "padding-left:0px;\n"
        "border:none;\n"
        "font:14px \"Arial\";\n"
    ));

    cbbSRS = new QComboBox(this);
    //cbbSRS->addItem("item 1");
    //cbbSRS->addItem("item 2");
    //cbbSRS->addItem("item 3");
    //attention  此处需要添加传进来的definition对应的名字
    InitSRS(cbbSRS);

    //cbbSRS->setStyleSheet(QString::fromUtf8(
    //  "background-color:#2D3035;\n"
    //  "color:white;\n"
    //  "border-radius:0px;\n"
    //  "margin-left:0px;\n"
    //  "padding-left:0px;\n"
    //  "border:none;\n"
    //));

    cbbSRS->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    border: 0px solid gray;   \n"
        "    border-radius: 4px;   \n"
        "    height:36px;\n"
        "    color: #FFFFFF;\n"
        "   font: 14px \"Arial\";\n"
        "   background-color:#34363A;\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 11px;\n"
        "   padding-right:0px;"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down { \n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png);"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: 0px solid;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    border-radius:4px;\n"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 28px;   \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    padding-left:10px;\n"
        "    font:14px solid #FFFFFF;"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160"
        ",160);   \n"
        "}\n"
        "\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
    ));

    /// "QComboBox QAbstractScrollArea QScrollBar::add-line:vertical,QComboBox QAbstractScrollArea QScrollBar::sub-line:vertical {\n"
    ///     "   width:0px;height:0px;"
    ///     "}\n"

    cbbSRS->setFont(font);
    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    cbbSRS->setItemDelegate(itemDelegate);

    hlSRS->setSpacing(0);
    hlSRS->addWidget(lblSRS);
    hlSRS->addWidget(cbbSRS, 3);
    hlSRS->addStretch(1);

    if (shouldSetOrigin_)
    {
        lblOriginSetting = new QLabel(this);
        if (BlockObject::isChineseVersion())
            lblOriginSetting->setText("<a style='color:#B0B5E8;font:14px Arial;' href='http://'>原点设置</a>");
        else
            lblOriginSetting->setText("<a style='color:#B0B5E8;font:14px Arial;' href='http://'>Origin Setting</a>");

        lblOriginSetting->setStyleSheet(QString::fromUtf8(
            "background-color:#2D3035;\n"
            "color:#B0B5E8;\n"
            "border-radius:0px;\n"
            "margin-left:0px;\n"
            "padding-left:0px;\n"
            "border:none;\n"
        ));

        font.setUnderline(true);
        font.setPixelSize(12);
        lblOriginSetting->setFont(font);
    }
    //  attention 这个地方需要注意如果是mesh格式为3dtile则只能是ecef，并且没有origin
    srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(definition_);
    if (srs.isValid())
    {

        ///cbbSRS->setCurrentText(QString::fromStdString(srs.name));
        cbbSRS->setCurrentText((AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(srs.name)));
    }



    ///vlTop->addSpacing(10);
    vlTop->setSpacing(24);
    vlTop->addWidget(lblTitle);
    vlTop->addLayout(hlSRS);
    //  attention 这个地方需要注意如果是如果是4d 或者pointcloud 、3dtile 格式则不需要orgin
    //shouldSetOrigin_ =true;
    if (shouldSetOrigin_)
    {
        vlTop->addWidget(lblOriginSetting);
        connect(lblOriginSetting, &QLabel::linkActivated, this, &SpatialReferenceSystem::Slot_OriginSetting);
    }
    vlTop->addStretch(1);


    connect(cbbSRS, &QComboBox::currentTextChanged, this, &SpatialReferenceSystem::Slot_SRSChanged);

    setLayout(vlTop);
}
void SpatialReferenceSystem::SetComSrsAsCurrent(QComboBox* pComboBox)
{
    //设置坐标系为当前导入数据坐标系
    //SetComBoxCurrentSrs(default_srs_);
    //需考虑增加ENU

    if (!srsdefinitionvec_.empty())
    {
        for (auto& iter : srsdefinitionvec_)
        {

            QString strenu = str2qstr(iter);
            QString strenu_ = strenu.left(3);
            if (!strenu_.compare("ENU", Qt::CaseInsensitive))
            {
                auto srstemp = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(iter);
                ///             pComboBox->addItem(QString::fromLocal8Bit(srstemp.name.c_str()));
                pComboBox->addItem((AI3D::GUI::MohackerWin::prependIndentation() + str2qstr(srstemp.name)));
            }
        }
    }

    auto default_srs = CoordinateDescriptor::GetSRSFromDefinition(definition_);
    /// int retint = pComboBox->findText(QString(default_srs.name.c_str()), Qt::MatchStartsWith);//暂时选用以开始
    int retint = pComboBox->findText((AI3D::GUI::MohackerWin::prependIndentation() + str2qstr(default_srs.name)), Qt::MatchStartsWith);//暂时选用以开始
    //@attetntion 若没有找到则赋值为 本类的 default_definition_;
    if (retint == -1)
    {
        default_srs = CoordinateDescriptor::GetSRSFromDefinition(default_definition_);
        ///     retint = pComboBox->findText(QString(default_srs.name.c_str()), Qt::MatchStartsWith);
        retint = pComboBox->findText((AI3D::GUI::MohackerWin::prependIndentation() + str2qstr(default_srs.name)), Qt::MatchStartsWith);
    }

    if (retint >= 0)
    {
        pComboBox->setCurrentIndex(retint);
        previous_srs = AI3D::GUI::MohackerWin::stripPrependIndentation(pComboBox->itemData(retint).toString());
    }
    //坐标转换
}

void SpatialReferenceSystem::InitSRS(QComboBox* pComboBox, bool bSetCurrentItem4Recent)
{
    if (pComboBox)
        pComboBox->clear();

    QStringList slDefault_Coords;
    QStringList slRecent_Coords;
    QStringList slMore_Coords;
    //add by chy @detail:此处的逻辑是默认为enu和投影的那个坐标系，并且默认显示的投影坐标系，所以这个时候wgs84需要放到recent里

#ifdef USE_AI3D_PROJ
    if (AI3D::CORE::BlockObject::isChineseVersion())
    {
        slDefault_Coords << "默认";
        slRecent_Coords << "最近";
        slMore_Coords << "更多";
        ///slMore_Coords << "空间参考系统数据库";
        slMore_Coords << (AI3D::GUI::MohackerWin::prependIndentation() + "空间参考系统数据库");
    }
    else
    {
        slDefault_Coords << "Default";
        slRecent_Coords << "Recent";
        slMore_Coords << "More";
        ///     slMore_Coords << "Spatial reference system database";
        slMore_Coords << (AI3D::GUI::MohackerWin::prependIndentation() + "Spatial reference system database");
    }
    //QStringList listCoords_default;
    /*listCoords_default << "Default";*/
    //参照cc,此处纯显示不加入库中；
    //@attention chy 把local加入 库中
    AI3D::PROJ::CoordinateReferenceSystem enucrs;
    if (!srsdefinitionvec_.empty())
    {
        for (auto& iter : srsdefinitionvec_)
        {

            QString strenu = str2qstr(iter);
            QString strenu_ = strenu.left(3);
            if (!strenu_.compare("ENU", Qt::CaseInsensitive))
            {
                enucrs.CreateFromENUDefinition(str2qstr(iter));

                ///             slDefault_Coords << QString::fromStdString(enucrs.GetDescription()/*+"("+enucrs.GetAuthID()+")"*/);
                slDefault_Coords << (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(enucrs.GetDescription()/*+"("+enucrs.GetAuthID()+")"*/));
            }
        }
    }

    AI3D::PROJ::CoordinateReferenceSystem wgs84crs(std::string("EPSG:4326"));

    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
    //  << default_definition_ << std::endl;

    AI3D::PROJ::CoordinateReferenceSystem defaultscrs(default_definition_);

    QString header = QString::fromStdString(default_definition_);

    /// slDefault_Coords << QString::fromStdString(defaultscrs.GetDescription() + "(" + defaultscrs.GetAuthID() + ")");
    slDefault_Coords << (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(defaultscrs.GetDescription() + "(" + defaultscrs.GetAuthID() + ")"));

    auto lists = AI3D::PROJ::QProj::coordinateReferenceSystemRegistry()->GetRecentCrs();

    QList< AI3D::PROJ::CoordinateReferenceSystem> filteredcrs;
    for (auto iter : lists)
    {

        AI3D::PROJ::CoordinateReferenceSystem crs(iter.GetAuthID());
        if (crs.isValid())
        {
            if (enucrs.isValid() && crs == enucrs || crs == defaultscrs)
            {
                std::cout << " same" << iter.GetAuthID() << std::endl;
            }
            else
            {
                filteredcrs << iter;
            }
        }

    }
    if (!filteredcrs.contains(wgs84crs))
    {
        if (filteredcrs.size() > 2)
            filteredcrs.insert(3, wgs84crs);
        else
        {
            filteredcrs.append(wgs84crs);
        }
    }
    int count = 0;
    //std::cout << lists.size() << std::endl;;
    for (auto iter : filteredcrs)
    {
        QString descrip = QString::fromStdString(iter.GetDescription());
        descrip.toUpper();
        ////        if (descrip.contains("ENU"))
        ////        {
        ///         slRecent_Coords << QString::fromStdString(iter.GetDescription() );
        ////            slRecent_Coords << (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription() ));
        ////        }
        ////        else
        {
            ///         slRecent_Coords << QString::fromStdString(iter.GetDescription() + "(" + iter.GetAuthID() + ")");
            slRecent_Coords << (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription() + "(" + iter.GetAuthID() + ")"));
        }

        //std::cout << " iter " << iter.GetDescription() + "(" + iter.GetAuthID() + ")" << std::endl;
        if (count == 7)
        {
            break;
        }
        count++;
    }



    pComboBox->setEditable(false);
    pComboBox->addItems(slDefault_Coords);
    pComboBox->addItems(slRecent_Coords);
    pComboBox->addItems(slMore_Coords);

    QModelIndex index_default = pComboBox->model()->index(0, 0);
    QVariant v_0(0);
    pComboBox->model()->setData(index_default, v_0, Qt::UserRole - 1);
    QModelIndex index_recent = pComboBox->model()->index(slDefault_Coords.size(), 0);
    QVariant v_2(0);
    pComboBox->model()->setData(index_recent, v_2, Qt::UserRole - 1);
    QModelIndex index_more = pComboBox->model()->index(slDefault_Coords.size() + slRecent_Coords.size(), 0);
    QVariant v_12(0);
    pComboBox->model()->setData(index_more, v_12, Qt::UserRole - 1);

    /// int retint = pComboBox->findText(QString::fromStdString(defaultscrs.GetDescription()), Qt::MatchStartsWith);//暂时选用以开始
    int retint = pComboBox->findText((AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(defaultscrs.GetDescription())), Qt::MatchStartsWith);//暂时选用以开始

    /// std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
    //      " " << defaultscrs.GetDescription() << " " << retint << std::endl;

    if (bSetCurrentItem4Recent && slRecent_Coords.size() > 1)
    {
        pComboBox->blockSignals(true);
        pComboBox->setCurrentIndex(slDefault_Coords.size() + 1);
        pComboBox->blockSignals(false);
        previous_srs = AI3D::GUI::MohackerWin::stripPrependIndentation(slRecent_Coords.at(1));
        //      std::cout << "inside SpatialReferenceSystem:" << __FILE__ << " " << __LINE__ << " "
        //          << previous_srs.toStdString() << std::endl;
    }
    else
    {
        if (retint >= 0)
        {
            pComboBox->blockSignals(true);
            pComboBox->setCurrentIndex(retint);
            pComboBox->blockSignals(false);
            ///         std::string str = "(" + defaultscrs.GetAuthID() + ")";
                        ///previous_srs = QString::fromStdString(str);
            previous_srs = pComboBox->itemData(retint, Qt::DisplayRole).toString();
            previous_srs = AI3D::GUI::MohackerWin::stripPrependIndentation(previous_srs);
            // 
            //          std::cout << "inside SpatialReferenceSystem:" << __FILE__ << " " << __LINE__ << " "
            //              << previous_srs.toStdString() << " / " << previous_srs2.toStdString() << " / " 
            //              << previous_srs21.toStdString() << std::endl;
        }
        else
        {
            //attention Add by chy 此处还没有逻辑因为不知道加啥逻辑
    //      std::cout << "inside SpatialReferenceSystem:" << __FILE__ << " " << __LINE__ << " "
    //          << previous_srs.toStdString() << std::endl;

        }
    }
#else
    if (AI3D::CORE::BlockObject::isChineseVersion())
    {
        slDefault_Coords << "默认";
        slRecent_Coords << "通用";
        slMore_Coords << "更多";

    }
    else
    {
        slDefault_Coords << "Default";
        slRecent_Coords << "Common";
        slMore_Coords << "More";

    }
    auto src_map = AI3D::CORE::CoordinateTransformer::CSG_coordinateSystem_Global();
    for (auto it = src_map.begin(); it != src_map.end(); it++)
    {
        if (it->first == "Default")
        {
            for (auto itsrcname : it->second)
            {
                slDefault_Coords << str2qstr(itsrcname.name);
            }
        }
        else if (it->first == "Common")
        {
            for (auto itsrcname : it->second)
            {
                slRecent_Coords << str2qstr(itsrcname.name);
            }
        }
        else if (it->first == "More")
        {
            for (auto itsrcname : it->second)
            {
                slMore_Coords << str2qstr(itsrcname.name);
            }
        }
    }

    pComboBox->setEditable(false);
    pComboBox->addItems(slDefault_Coords);
    pComboBox->addItems(slRecent_Coords);
    pComboBox->addItems(slMore_Coords);

    QModelIndex index_default = pComboBox->model()->index(0, 0);
    QVariant v_0(0);
    pComboBox->model()->setData(index_default, v_0, Qt::UserRole - 1);
    QModelIndex index_recent = pComboBox->model()->index(slDefault_Coords.size(), 0);
    QVariant v_2(0);
    pComboBox->model()->setData(index_recent, v_2, Qt::UserRole - 1);
    QModelIndex index_more = pComboBox->model()->index(slDefault_Coords.size() + slRecent_Coords.size(), 0);
    QVariant v_12(0);
    pComboBox->model()->setData(index_more, v_12, Qt::UserRole - 1);

    if (pComboBox->count() > 0)
    {
        SetComSrsAsCurrent(pComboBox);
    }
    //  pComboBox->setCurrentIndex(1);
#endif
}

void SpatialReferenceSystem::Reset()
{

}

void  SpatialReferenceSystem::SetSrsUnEditable()
{
    cbbSRS->setEnabled(false);
    shouldSetOrigin_ = false;
}

void SpatialReferenceSystem::Slot_OriginSetting(const QString& str)
{
    //  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << ":" << str.toStdString() << std::endl;
    //  std::cout << "before setting origin:" << this->coor_origin_.x() << " / " << this->coor_origin_.y() <<
    //      " / " << this->coor_origin_.z() << std::endl;
    OpenOriginSettings(this, nullptr);
    //  std::cout << "after setting origin:" << this->coor_origin_.x() << " / " << this->coor_origin_.y() <<
    //      " / " << this->coor_origin_.z() << std::endl;
        //QMessageBox::information(nullptr, "spatial reference system", "origin setting clicked.");
}

void SpatialReferenceSystem::Slot_SRSChanged(const QString& _str)
{
    QString str = AI3D::GUI::MohackerWin::stripPrependIndentation(_str);

    if (str == "Spatial reference system database" || str == "空间参考系统数据库")
    {
        //      std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << cbbSRS->currentIndex() << std::endl;
                ///  note!!!: just for test purpose now.
                //AI3D::PROJ::QgsCoordinateReferenceSystem*crs = new AI3D::PROJ::CoordinateReferenceSystem(srsname.toStdString());
                //QgsCoordinateReferenceSystem* crs = new QgsCoordinateReferenceSystem(srsname);
                //QgsCoordinateReferenceSystem* crs = new QgsCoordinateReferenceSystem("EPSG:4978");
                ///QgsCoordinateReferenceSystem* crs = new QgsCoordinateReferenceSystem("EPSG:4326");

        QString _srsname;

        int idx = cbbSRS->currentIndex();
        if (idx >= 0 && idx <= cbbSRS->count())
            _srsname = cbbSRS->itemData(idx, Qt::DisplayRole).toString();

        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << 
        //  " " << previous_srs.toStdString() << std::endl;

        _srsname = previous_srs;

        AI3D::PROJ::CoordinateReferenceSystem crs;
        //          crs.createFromString("EPSG:4413");

        //std::cout << "current.text:" << _srsname.toStdString() << std::endl;
        bool bFoundLocalENU = false;
        bool bFoundValidENUCrs = false;

        int startAuthIdPos = _srsname.lastIndexOf("(");
        int endAuthIdPos = _srsname.lastIndexOf(")");

        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
        //  << " " << _srsname.toStdString() << std::endl;

        if (_srsname.contains(AI3D::GUI::MohackerWin::localSRS(), Qt::CaseInsensitive))
        {
            //std::cout << "current.text1:" << _srsname.toStdString() << std::endl;
            //bFoundLocalENU = true;
            QString authId = "Local:0";
            crs.createFromString(authId);
        }
        else if (startAuthIdPos >= 0 && endAuthIdPos >= 0 && startAuthIdPos < endAuthIdPos)
        {
            QString authId = _srsname.mid(startAuthIdPos + 1, endAuthIdPos - (startAuthIdPos + 1));

            if (authId.contains("ENU", Qt::CaseInsensitive))
            {
                //                      std::cout << "current.text21:" << _srsname.toStdString() << std::endl;
                crs.CreateFromENUDefinition(authId);
                if (crs.isValid())
                {
                    //                          std::cout << "gcp dia/valid enu crs definition:" << crs.description().toStdString() << " authid:" << crs.authid().toStdString() << std::endl;
                    bFoundValidENUCrs = true;
                }
            }
            else
            {
                crs.createFromString(authId);
            }
        }
        else
        {
            crs.CreateFromENUDefinition(_srsname);
        }

        ///AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this);
        AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this, AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterHorizontal | AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterCompound, (bFoundValidENUCrs ? crs.description() : ""),
            (bFoundValidENUCrs ? crs.authid() : ""));

        connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsSelected, this, &SpatialReferenceSystem::Slot_SrsSelected);
        connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsRestore, this, &SpatialReferenceSystem::Slot_SrsRestore);
        //std::cout << "before importing gcp qgswidget." << std::endl;

        if (bFoundLocalENU)
        {
            /// 
        }
        else
        {
            qgsWidget->setCrs(crs);
        }

        //qgsWidget->setFixedSize(qMin(this->width(), 578), qMin(this->height(), 650));
        qgsWidget->setFixedSize(1130, 810);
        //qgsWidget->raise();

        qgsWidget->show();

        if (bFoundLocalENU)
            qgsWidget->selectCrsByName(QString("Local East-North-Up (ENU)"));

        //std::cout << "after importing gcp qgswidget." << std::endl;
        return;
    }

    previous_srs = str;
    //  std::cout << "inside SpatialReferenceSystem:" << __FILE__ << " " << __LINE__ << " "
    //      << previous_srs.toStdString() << std::endl;

    srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(str.toStdString());
    if (srs.isValid())
    {
        definition_ = srs.definition;
        //std::cout << "srs got/valid:" << str.toStdString() << " // " << srs.name << " // " << srs.definition << std::endl;

        //重新计算origin
        double x = 0.0;
        double z = 0.0;
        double y = 0.0;
        if (srsdefinitionvec_.size() > 0)
            AI3D::CORE::CoordinateTransformer::Transform(x, y, z, x, y, z, *srsdefinitionvec_.begin(), srs.definition);
        this->coor_origin_.x() = x;
        this->coor_origin_.y() = y;
        this->coor_origin_.z() = 0.0;

    }
    else
    {
        //std::cout << "srs got/invalid:" << str.toStdString() << std::endl;
    }

}

void SpatialReferenceSystem::Slot_SrsSelected(QString& srs)
{
    cbbSRS->blockSignals(true);
    InitSRS(cbbSRS, true);
    cbbSRS->blockSignals(false);
}

void SpatialReferenceSystem::Slot_SrsRestore()
{
    if (!previous_srs.isEmpty())
    {
        cbbSRS->blockSignals(true);
        std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << previous_srs.toStdString() << std::endl;
        cbbSRS->setCurrentText((AI3D::GUI::MohackerWin::prependIndentation() + previous_srs));
        cbbSRS->blockSignals(false);
    }
}

TilingRange::TilingRange(ParamSettings4Production* paramSettings4Production, QWidget* parent)
    : QWidget(parent)
{
    ///Init();
    this->paramSettings4Production = paramSettings4Production;
}

TilingRange::~TilingRange()
{

}

void TilingRange::Init()
{
    vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(41, 27, 41, 27);

    lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblTitle->setText("分块范围");
    }
    else {
        lblTitle->setText("Tiling Range");
    }

    lblTitle->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    //lblTitle->setStyleSheet(QString::fromUtf8(
    //  "background-color:black;\n"
    //  "color:green;\n"
    //  "margin-left:42px;\n"
    //));

    hlTiling = new QHBoxLayout();
    hlTiling->setContentsMargins(0, 0, 0, 0);

    lblTiling = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblTiling->setText("分块：已选择");
    }
    else {
        lblTiling->setText("Tiling:");
    }

    lblTiling->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    lblTiles = new QLabel(this);

    int tile_select_count = tiles_selected_.size();
    int tile_count = this->paramSettings4Production->recons_object->GetTilesName(this->paramSettings4Production->recons_object->GetProcessingSettings().bdiscard_emptytiles_).size();// tiles_.size();
    std::string strTiles = std::to_string(tile_select_count) + "/" + std::to_string(tile_count);

    lblTiles->setText(QString::fromStdString(strTiles));

    lblTiles->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:bold 14px \"Arial\";\n}"
        ""));

    lblTilingSuffix = new QLabel(this);

    if (BlockObject::isChineseVersion()) {
        lblTilingSuffix->setText("块");
    }
    else {
        lblTilingSuffix->setText("tile(s) selected.");
    }

    lblTilingSuffix->setStyleSheet(QString::fromUtf8(
        "QLabel {background-color:#2D3035;\n"
        "color:#FFFFFF;\n"
        "padding-top:0px;\n"
        "padding-left:0px;\n"
        "font:14px \"Arial\";\n}"
        ""));

    lblClickToSelect = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblClickToSelect->setText("<a style='background-color:#2D3035;color:#B0B5E8;font:12px Arial;' href='http://'>点击选择</a>");
    }
    else {
        lblClickToSelect->setText("<a style='background-color:#2D3035;color:#B0B5E8;font:12px Arial;' href='http://'>Click to select</a>");
    }

    hlTiling->setSpacing(5);
    hlTiling->addWidget(lblTiling);
    hlTiling->addWidget(lblTiles);
    hlTiling->addWidget(lblTilingSuffix);
    hlTiling->addSpacing(5);
    hlTiling->addWidget(lblClickToSelect);
    hlTiling->addStretch(1);

    //  vlTop->addSpacing(5);
    //  vlTop->setSpacing(27);
    vlTop->setSpacing(24);
    vlTop->addWidget(lblTitle);
    vlTop->addLayout(hlTiling);
    vlTop->addStretch(1);

    connect(lblClickToSelect, &QLabel::linkActivated, this, &TilingRange::Slot_ClickToSelect);

    setLayout(vlTop);
}

void TilingRange::Reset()
{
    int tile_select_count = tiles_selected_.size();
    int tile_count = this->paramSettings4Production->recons_object->GetTilesName(this->paramSettings4Production->recons_object->GetProcessingSettings().bdiscard_emptytiles_).size();// tiles_.size();
    std::string strTiles = std::to_string(tile_select_count) + "/" + std::to_string(tile_count);

    lblTiles->setText(QString::fromStdString(strTiles));
}

void TilingRange::Slot_ClickToSelect(const QString&)
{
    paramSettings4Production->hide();
    OpenTilesList(paramSettings4Production, paramSettings4Production ? paramSettings4Production->recons_object : nullptr, nullptr);
}

SpatialReferenceSystemOriginSetting* spatialReferenceSystemOriginSetting = nullptr;

void OpenOriginSettings(SpatialReferenceSystem* srs, QWidget* parent)
{
    SpatialReferenceSystemOriginSetting* pMoreSettings = new SpatialReferenceSystemOriginSetting(srs, parent);

    pMoreSettings->setWindowModality(Qt::ApplicationModal);
    pMoreSettings->setAttribute(Qt::WA_DeleteOnClose);
    pMoreSettings->setModal(true);
    //pMoreSettings->resize(400, 260);
    pMoreSettings->resize(500, 260);
    //pMoreSettings->show();
    pMoreSettings->exec();
}

SpatialReferenceSystemOriginSetting::SpatialReferenceSystemOriginSetting(SpatialReferenceSystem* srs, QWidget* parent)
    : QDialog(parent)
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    //setObjectName("spatialReferenceSystemOriginSetting");
    this->srs = srs;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    ///setStyleSheet("#spatialReferenceSystemOriginSetting { background-color:darkgreen;}");
    setStyleSheet("{ background-color:yellow;}");

    if (BlockObject::isChineseVersion()) {
        this->strTitle = "原点设置";
    }
    else {
        this->strTitle = "Origin setting";
    }

    QVBoxLayout* vlMain = new QVBoxLayout();
    vlMain->setContentsMargins(0, 0, 0, 0);

    QFrame* frameTop = new QFrame(this);
    frameTop->setStyleSheet("QFrame { background-color:#373C45;border-radius:4px;border:none;}");

    QVBoxLayout* vlTop = new QVBoxLayout();
    ///vlTop->setContentsMargins(16, 0, 20, 10);
    vlTop->setContentsMargins(16, 0, 8, 10);
    vlTop->setSpacing(0);

    QWidget* titleWidget = new QWidget(this);
    titleWidget->setFixedHeight(30);
    titleWidget->setContentsMargins(0, 0, 0, 0);
    titleWidget->setStyleSheet("background-color:transparent;");

    QHBoxLayout* hlTitle = new QHBoxLayout();
    hlTitle->setSpacing(0);
    hlTitle->setContentsMargins(0, 0, 0, 0);

    lblTitle = new QLabel(this);
    //lblTitle->setText("Data Preprocess");
    lblTitle->setText(this->strTitle);
    ///lblTitle->setStyleSheet("background-color:transparent;color:white;margin-left:40px;");
    lblTitle->setStyleSheet("background-color:transparent;color:white;margin-left:0px;font:14px \"Arial\";");

    butClose = new QPushButton(this);
    butClose->setIcon(QPixmap(":/new/prefix1/skin/close14.png"));
    //butClose->setFixedSize(QSize(14, 14));
    //butClose->setStyleSheet("background-color:#373C45;width:14px;height:14px;border:none;margin-top:8px;margin-right:0px;");
    butClose->setStyleSheet("background-color:transparent;width:14px;height:14px;border:none;margin-top:8px;margin-right:0px;");

    QFont font = lblTitle->font();
    font.setPixelSize(14);

    lblTitle->setFont(font);

    hlTitle->addWidget(lblTitle);
    hlTitle->addStretch(1);
    hlTitle->addWidget(butClose, 0, Qt::AlignTop);

    titleWidget->setLayout(hlTitle);

    ///font.setPointSize(16);
    font.setPixelSize(12);

    rbAutoSet = new QRadioButton(this);
    if (BlockObject::isChineseVersion()) {
        rbAutoSet->setText("自动设置原点");
    }
    else {
        rbAutoSet->setText("Automatically set origin");
    }
    rbAutoSet->setStyleSheet(QString::fromUtf8(
        "QRadioButton {"
        "background-color:#373C45;"
        "color:#DCE1EA;"
        "font:12px \"Arial\";"
        "}"));
    rbAutoSet->setFont(font);

    QHBoxLayout* hlAutoXYZ = new QHBoxLayout();
    QLabel* lblAutoSetX = new QLabel(this);
    lblAutoSetX->setText("X:");
    lblAutoSetX->setStyleSheet("background-color:#373C45;color:#FFFFFF;font:12px \"Arial\";");

    leAutoSetX = new QLineEdit(this);
    QDoubleValidator* doubleValidator = new QDoubleValidator(this);
    leAutoSetX->setValidator(doubleValidator);
    leAutoSetX->setReadOnly(true);
    leAutoSetX->setStyleSheet("background-color:transparent;color:#FFFFFF;border:none;font:12px \"Arial\";");

    QLabel* lblAutoSetY = new QLabel(this);
    lblAutoSetY->setText("Y:");
    lblAutoSetY->setStyleSheet("background-color:#373C45;color:#FFFFFF;font:12px \"Arial\";");

    leAutoSetY = new QLineEdit(this);
    leAutoSetY->setValidator(doubleValidator);
    leAutoSetY->setReadOnly(true);
    leAutoSetY->setStyleSheet("background-color:transparent;color:#FFFFFF;border:none;font:12px \"Arial\";");

    QLabel* lblAutoSetZ = new QLabel(this);
    lblAutoSetZ->setText("Z:");
    lblAutoSetZ->setStyleSheet("background-color:#373C45;color:#FFFFFF;font:12px \"Arial\";");

    leAutoSetZ = new QLineEdit(this);
    leAutoSetZ->setValidator(doubleValidator);
    leAutoSetZ->setReadOnly(true);
    leAutoSetZ->setStyleSheet("background-color:transparent;color:#FFFFFF;border:none;font:12px \"Arial\";");

    hlAutoXYZ->setSpacing(0);
    hlAutoXYZ->addSpacing(26);
    hlAutoXYZ->addWidget(lblAutoSetX);
    hlAutoXYZ->addSpacing(6);
    hlAutoXYZ->addWidget(leAutoSetX, 2);
    hlAutoXYZ->addSpacing(21);
    hlAutoXYZ->addWidget(lblAutoSetY);
    hlAutoXYZ->addSpacing(6);
    hlAutoXYZ->addWidget(leAutoSetY, 2);
    hlAutoXYZ->addSpacing(20);
    hlAutoXYZ->addWidget(lblAutoSetZ);
    hlAutoXYZ->addSpacing(6);
    hlAutoXYZ->addWidget(leAutoSetZ, 2);
    hlAutoXYZ->addStretch(1);

    lblAutoSetX->setFont(font);
    leAutoSetX->setFont(font);

    lblAutoSetY->setFont(font);
    leAutoSetY->setFont(font);

    lblAutoSetZ->setFont(font);
    leAutoSetZ->setFont(font);

    if (srs != nullptr)
    {
        leAutoSetX->setText(QString::number(srs->coor_origin_.x(), 'f', 6));
        leAutoSetY->setText(QString::number(srs->coor_origin_.y(), 'f', 6));
        leAutoSetZ->setText(QString::number(srs->coor_origin_.z(), 'f', 6));


    }

    QLabel* lblAutoSetDescription = new QLabel(this);
    lblAutoSetDescription->setWordWrap(false);
    font.setPixelSize(10);

    if (BlockObject::isChineseVersion())
    {
        QString str = "自动计算的原点坐标接近实际数据,可有效避免模型坐标过大从而导致进入第三方软件时产生模型精度损失";
        // note: translated it later.
        lblAutoSetDescription->setText(str);

    }
    else
    {
        lblAutoSetDescription->setText("Automatic origin is set close to the data to avoid very large internal\nmodel"
            " shapes Standard,causing accuracy loss in third-party software");
    }
    lblAutoSetDescription->setStyleSheet(QString::fromUtf8(
        "QLabel {"
        "background-color:#373C45;"
        "color:#AEAEAE;"
        "margin-left:22px;"
        "padding-left:0px;"
        "font:10px \"Arial\";"
        "}"
        ""));
    lblAutoSetDescription->setAlignment(Qt::AlignLeft);

    rbCustomSet = new QRadioButton(this);
    if (BlockObject::isChineseVersion()) {
        rbCustomSet->setText("自定义原点");
    }
    else {
        rbCustomSet->setText("Custom Origin");
    }
    rbCustomSet->setStyleSheet(QString::fromUtf8(
        "QRadioButton{"
        "background-color:#373C45;"
        "color:#DCE1EA;"
        "font:12px \"Arial\";"
        "}"));

    font.setPixelSize(12);
    rbCustomSet->setFont(font);

    QHBoxLayout* hlCustomXYZ = new QHBoxLayout();

    QLabel* lblCustomSetX = new QLabel(this);
    lblCustomSetX->setText("X:");
    leCustomSetX = new QLineEdit(this);
    lblCustomSetX->setStyleSheet("background-color:#373C45;color:#FFFFFF;");
    leCustomSetX->setValidator(doubleValidator);
    leCustomSetX->setStyleSheet(QString::fromUtf8(
        "QLineEdit {"
        "background:white;"
        "color:black;"
        "border-radius:4px;"
        "height:24px;"
        "padding-left:6px;"
        "font:10px \"Arial\";"
        "}"
        "QLineEdit:disabled {"
        "background-color:#34363A;"
        "color:#FFFFFF;"
        "}"
    ));

    QLabel* lblCustomSetY = new QLabel(this);
    lblCustomSetY->setText("Y:");
    lblCustomSetY->setStyleSheet("background-color:#373C45;color:#FFFFFF;");

    leCustomSetY = new QLineEdit(this);
    leCustomSetY->setValidator(doubleValidator);
    leCustomSetY->setStyleSheet(QString::fromUtf8(
        "QLineEdit {"
        "background-color:white;"
        "color:black;"
        "border-radius:4px;"
        "height:24px;"
        "padding-left:6px;"
        "font:10px \"Arial\";"
        "}"
        "QLineEdit:disabled {"
        "background-color:#34363A;"
        "color:#FFFFFF;"
        "}"
    ));

    QLabel* lblCustomSetZ = new QLabel(this);
    lblCustomSetZ->setText("Z:");
    lblCustomSetZ->setStyleSheet("background-color:#373C45;color:#FFFFFF;");

    leCustomSetZ = new QLineEdit(this);
    leCustomSetZ->setValidator(doubleValidator);
    leCustomSetZ->setStyleSheet(QString::fromUtf8(""
        "QLineEdit {"
        "background-color:white;"
        "color:black;"
        "border-radius:4px;"
        "height:24px;"
        "padding-left:6px;"
        "font:10px \"Arial\";"
        "}"
        "QLineEdit:disabled {"
        "background-color:#34363A;"
        "color:#FFFFFF;"
        "}"
    ));

    font.setPixelSize(10);
    leAutoSetX->setFont(font);
    leAutoSetY->setFont(font);
    leAutoSetZ->setFont(font);

    leCustomSetX->setFont(font);
    leCustomSetY->setFont(font);
    leCustomSetZ->setFont(font);

    if (srs != nullptr)
    {
        leCustomSetX->setText(QString::number(srs->coor_origin_custom_.x(), 'f', 6));
        leCustomSetY->setText(QString::number(srs->coor_origin_custom_.y(), 'f', 6));
        leCustomSetZ->setText(QString::number(srs->coor_origin_custom_.z(), 'f', 6));
    }

    hlCustomXYZ->setSpacing(0);
    hlCustomXYZ->addSpacing(26);
    hlCustomXYZ->addWidget(lblCustomSetX);
    hlCustomXYZ->addSpacing(6);
    hlCustomXYZ->addWidget(leCustomSetX, 2);
    hlCustomXYZ->addSpacing(21);
    hlCustomXYZ->addWidget(lblCustomSetY);
    hlCustomXYZ->addSpacing(6);
    hlCustomXYZ->addWidget(leCustomSetY, 2);
    hlCustomXYZ->addSpacing(20);
    hlCustomXYZ->addWidget(lblCustomSetZ);
    hlCustomXYZ->addSpacing(6);
    hlCustomXYZ->addWidget(leCustomSetZ, 2);
    hlCustomXYZ->addStretch(1);

    QHBoxLayout* hlOkCancel = new QHBoxLayout();

    butOk = new QPushButton(this);
    butCancel = new QPushButton(this);
    if (BlockObject::isChineseVersion()) {
        butOk->setText("确定");
        butCancel->setText("取消");
    }
    else {
        butOk->setText("OK");
        butCancel->setText("Cancel");
    }
    butOk->setFont(font);
    butCancel->setFont(font);

    //butOk->setFixedSize(QSize(58, 23));
    //butCancel->setFixedSize(QSize(58,23));

    butOk->setStyleSheet(QString::fromUtf8(
        "background-color:#1547F8;"
        "color:#FFFFFF;"
        "width:58px;height:23px;"
        "margin-top:5px;"
        "margin-bottom:0px;"
        "border-radius:2px;"
        "border:none;"
        "font:10px \"Arial\";"
    ));

    butCancel->setStyleSheet(QString::fromUtf8(
        "background-color:#3F455C;"
        "color:#FFFFFF;"
        "width:58px;height:23px;"
        "margin-top:5px;"
        "margin-right:12px;"
        "margin-bottom:0px;"
        "border-radius:2px;"
        "border:none;"
        "font:10px \"Arial\";"
    ));

    hlOkCancel->addSpacing(0);
    hlOkCancel->setSpacing(10);
    hlOkCancel->addStretch(1);
    hlOkCancel->addWidget(butOk);
    hlOkCancel->addWidget(butCancel);

    rbAutoSet->setChecked(true);
    rbCustomSet->setChecked(false);

    anySet();

    vlTop->setSpacing(15);
    //vlTop->addWidget(lblTitle);
    vlTop->addWidget(titleWidget);
    vlTop->addWidget(rbAutoSet, 1);
    vlTop->addLayout(hlAutoXYZ, 1);
    vlTop->addWidget(lblAutoSetDescription, 1);
    vlTop->addWidget(rbCustomSet, 1);
    vlTop->addLayout(hlCustomXYZ, 1);
    ///vlTop->addStretch(1);
    vlTop->addLayout(hlOkCancel);

    connect(butClose, &QPushButton::clicked, this, &SpatialReferenceSystemOriginSetting::Slot_Close);
    connect(butOk, &QPushButton::clicked, this, &SpatialReferenceSystemOriginSetting::Slot_OK);
    connect(butCancel, &QPushButton::clicked, this, &SpatialReferenceSystemOriginSetting::Slot_Cancel);
    connect(rbCustomSet, &QRadioButton::clicked, this, &SpatialReferenceSystemOriginSetting::Slot_CustomSet);
    connect(rbAutoSet, &QRadioButton::clicked, this, &SpatialReferenceSystemOriginSetting::Slot_AutoSet);
    connect(leAutoSetX, &QLineEdit::editingFinished, this, &SpatialReferenceSystemOriginSetting::Slot_XYZChanged);
    connect(leAutoSetY, &QLineEdit::editingFinished, this, &SpatialReferenceSystemOriginSetting::Slot_XYZChanged);
    connect(leAutoSetZ, &QLineEdit::editingFinished, this, &SpatialReferenceSystemOriginSetting::Slot_XYZChanged);
    connect(leCustomSetX, &QLineEdit::editingFinished, this, &SpatialReferenceSystemOriginSetting::Slot_XYZChanged);
    connect(leCustomSetY, &QLineEdit::editingFinished, this, &SpatialReferenceSystemOriginSetting::Slot_XYZChanged);
    connect(leCustomSetZ, &QLineEdit::editingFinished, this, &SpatialReferenceSystemOriginSetting::Slot_XYZChanged);

    frameTop->setLayout(vlTop);
    vlMain->addWidget(frameTop, 1);

    setLayout(vlMain);
}

SpatialReferenceSystemOriginSetting::~SpatialReferenceSystemOriginSetting()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#if 0
    if (pParamSettings4Production != nullptr)
    {
        std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        pParamSettings4Production = nullptr;
    }
#endif
}

void SpatialReferenceSystemOriginSetting::closeEvent(QCloseEvent* event)
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

}

void SpatialReferenceSystemOriginSetting::Slot_OK()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (srs != nullptr) {


        if (!rbCustomSet->isChecked())
        {


            srs->coor_origin_.x() = leAutoSetX->text().toDouble();
            srs->coor_origin_.y() = leAutoSetY->text().toDouble();
            srs->coor_origin_.z() = leAutoSetZ->text().toDouble();

            srs->inAutoMode = true;

        }
        else
        {
            srs->coor_origin_custom_.x() = leCustomSetX->text().toDouble();
            srs->coor_origin_custom_.y() = leCustomSetY->text().toDouble();
            srs->coor_origin_custom_.z() = leCustomSetZ->text().toDouble();

            srs->inAutoMode = false;
        }
    }

    accept();
}

void SpatialReferenceSystemOriginSetting::Slot_Cancel()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    reject();
}

void SpatialReferenceSystemOriginSetting::anySet()
{
    if (rbAutoSet->isChecked())
    {
        inAutoMode = true;
        std::cout << "autoset true." << std::endl;
    }
    else
    {
        inAutoMode = false;
        std::cout << "autoset false." << std::endl;
    }

    if (rbCustomSet->isChecked())
    {
        std::cout << "customset true." << std::endl;
        leCustomSetX->setEnabled(true);
        leCustomSetY->setEnabled(true);
        leCustomSetZ->setEnabled(true);
    }
    else
    {
        std::cout << "customset false." << std::endl;
        leCustomSetX->setEnabled(false);
        leCustomSetY->setEnabled(false);
        leCustomSetZ->setEnabled(false);
    }
}

void SpatialReferenceSystemOriginSetting::Slot_AutoSet()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    anySet();
}

void SpatialReferenceSystemOriginSetting::Slot_CustomSet()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    anySet();
}

//void SpatialReferenceSystemOriginSetting::Slot_InputValue()
//{
//}

void SpatialReferenceSystemOriginSetting::Slot_Close()
{
    std::cout << "close clicked inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    reject();
}

void SpatialReferenceSystemOriginSetting::Slot_XYZChanged()
{
    QLineEdit* pLineEditSource = dynamic_cast<QLineEdit*>(sender());
    if (pLineEditSource != nullptr)
    {

        //std::cout << "xyz value changed." << std::endl;
        QString strValue = pLineEditSource->text();
        double dValue = strValue.toDouble();
        if (pLineEditSource == leAutoSetX)
        {
            coor_origin_auto.x() = dValue;
            inAutoMode = true;
            std::cout << "autoset x value changed." << strValue.toStdString() << " " << dValue << std::endl;
        }
        else if (pLineEditSource == leAutoSetY)
        {
            coor_origin_auto.y() = dValue;
            inAutoMode = true;
            std::cout << "autoset y value changed." << strValue.toStdString() << " " << dValue << std::endl;
        }
        else if (pLineEditSource == leAutoSetZ)
        {
            coor_origin_auto.z() = dValue;
            inAutoMode = true;
            std::cout << "autoset z value changed." << strValue.toStdString() << " " << dValue << std::endl;
        }
        else if (pLineEditSource == leCustomSetX)
        {
            coor_origin_custom.x() = dValue;
            inAutoMode = false;
            std::cout << "customset x value changed." << strValue.toStdString() << " " << dValue << std::endl;
        }
        else if (pLineEditSource == leCustomSetY)
        {
            coor_origin_custom.y() = dValue;
            inAutoMode = false;
            std::cout << "customset y value changed." << strValue.toStdString() << " " << dValue << std::endl;
        }
        else if (pLineEditSource == leCustomSetZ)
        {
            coor_origin_custom.z() = dValue;
            inAutoMode = false;
            std::cout << "customset z value changed." << strValue.toStdString() << " " << dValue << std::endl;
        }
        else
        {
            std::cout << "unknown edit source." << std::endl;
        }
    }
    else
    {
        std::cout << "unknown source changed." << std::endl;
    }
}

static YesNoCancelDialog* pYesNoCancelDialog = nullptr;

YesNoCancelDialog::YesNoCancelDialog(QWidget* parent, QString strTitle)
    : QDialog(parent)
{
    std::cout << "inside ync dialog constructor." << std::endl;
    setAttribute(Qt::WA_DeleteOnClose);
    this->strTitle = strTitle;
    Init();
    //std::cout << "inside ync dialog constructor2." << std::endl;
}

YesNoCancelDialog::~YesNoCancelDialog()
{
    std::cout << "inside ync dialog deconstructor." << std::endl;
    pYesNoCancelDialog = nullptr;
}

void YesNoCancelDialog::Init()
{
    //setStyleSheet("QDialog { background-color: darkgreen; border: none; border-radius: 6px; }");

    QVBoxLayout* vlMain = new QVBoxLayout();
    vlMain->setContentsMargins(0, 0, 0, 0);

    QFrame* frameTop = new QFrame(this);
    frameTop->setStyleSheet(" background-color: #303030; border: none; border-radius: 6px; ");

    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 0, 0, 0);

    //QHBoxLayout* hlTitle = new QHBoxLayout();
    butClose = new QPushButton(this);
    //butClose->setText("Close");
    butClose->setIcon(QPixmap(":/new/prefix1/skin/close2324.png"));
    //butClose->setContentsMargins(0, 30, 68, 30);
    butClose->setStyleSheet("border:none; padding:0px;margin:0px;margin-top:15px;margin-right:12px;margin-bottom:15px;");

    lblTitle = new QLabel(this);
    lblTitle->setText(strTitle);
    lblTitle->setStyleSheet("color:#FFFFFF;font:14px solid;margin:0px;margin-top:8px;margin-bottom:37px;");

    QHBoxLayout* hlButtons = new QHBoxLayout();
    hlButtons->setContentsMargins(0, 0, 0, 27);

    butYes = new QPushButton(this);
    butNo = new QPushButton(this);
    butCancel = new QPushButton(this);

    if (BlockObject::isChineseVersion())
    {
        butYes->setText("是");
        butNo->setText("否");
        butCancel->setText("取消");
    }
    else
    {
        butYes->setText("Yes");
        butNo->setText("No");
        butCancel->setText("Cancel");
    }
    butYes->setStyleSheet("background-color:#538CCF;color:#FFFFFF;font:12px solid;width:110px;height:28px;border:none;border-radius:4px;");
    butNo->setStyleSheet("background-color:#545454;color:#FFFFFF;font:12px solid;width:110px;height:28px;border:none;border-radius:4px;");
    butCancel->setStyleSheet("background-color:#545454;color:#FFFFFF;font:12px solid;width:110px;height:28px;border:none;border-radius:4px;");

    hlButtons->setSpacing(8);
    hlButtons->addStretch(1);
    hlButtons->addWidget(butCancel);
    hlButtons->addWidget(butNo);
    hlButtons->addWidget(butYes);
    hlButtons->addStretch(1);

    vlTop->addWidget(butClose, 1, Qt::AlignRight);
    vlTop->addWidget(lblTitle, 1, Qt::AlignHCenter);
    vlTop->addLayout(hlButtons, 1);

    connect(butClose, &QPushButton::clicked, this, &YesNoCancelDialog::Slot_Close);
    connect(butYes, &QPushButton::clicked, this, &YesNoCancelDialog::Slot_Yes);
    connect(butNo, &QPushButton::clicked, this, &YesNoCancelDialog::Slot_No);
    connect(butCancel, &QPushButton::clicked, this, &YesNoCancelDialog::Slot_Cancel);

    frameTop->setLayout(vlTop);
    vlMain->addWidget(frameTop, 1);

    setLayout(vlMain);
    //pYesNoCancelDialog = this;
}

void YesNoCancelDialog::Slot_Yes()
{
    done(QMessageBox::Yes);
}

void YesNoCancelDialog::Slot_No()
{
    done(QMessageBox::No);
}

void YesNoCancelDialog::Slot_Cancel()
{
    done(QMessageBox::Cancel);
}

void YesNoCancelDialog::Slot_Close()
{
    done(QMessageBox::Cancel);
}

int OpenYesNoCancelDialog(QWidget* parent, QString strTitle, int w, int h)
{
    std::cout << "open ync dialog 1." << std::endl;
    if (pYesNoCancelDialog)
    {
        std::cout << "open ync dialog 2." << std::endl;
        return QMessageBox::Cancel;
    }

    std::cout << "open ync dialog 3." << std::endl;
    pYesNoCancelDialog = new YesNoCancelDialog(parent, strTitle);
    pYesNoCancelDialog->setWindowFlags(pYesNoCancelDialog->windowFlags() | Qt::FramelessWindowHint);
    pYesNoCancelDialog->setAttribute(Qt::WA_TranslucentBackground);

    pYesNoCancelDialog->resize(w, h);

    ///pYesNoCancelDialog->show();

    ///return QMessageBox::Yes;
    std::cout << "open ync dialog 4." << std::endl;
    return pYesNoCancelDialog->exec();
}

void CloseYesNoCancelDialog()
{
    if (!pYesNoCancelDialog)
        return;
}

static OkDialog* pOkDialog = nullptr;

OkDialog::OkDialog(QWidget* parent, QString strTitle)
    : QDialog(parent)
{
    std::cout << "inside ync dialog constructor." << std::endl;
    setAttribute(Qt::WA_DeleteOnClose);
    this->strTitle = strTitle;
    Init();
    //std::cout << "inside ync dialog constructor2." << std::endl;
}

OkDialog::~OkDialog()
{
    std::cout << "inside ync dialog deconstructor." << std::endl;
    pOkDialog = nullptr;
}

void OkDialog::Init()
{
    //setStyleSheet("QDialog { background-color: darkgreen; border: none; border-radius: 6px; }");

    QVBoxLayout* vlMain = new QVBoxLayout();
    vlMain->setContentsMargins(0, 0, 0, 0);

    QFrame* frameTop = new QFrame(this);
    frameTop->setStyleSheet(" background-color: #303030; border: none; border-radius: 6px; ");

    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 0, 0, 0);

    //QHBoxLayout* hlTitle = new QHBoxLayout();
    butClose = new QPushButton(this);
    //butClose->setText("Close");
    butClose->setIcon(QPixmap(":/new/prefix1/skin/close2324.png"));
    //butClose->setContentsMargins(0, 30, 68, 30);
    butClose->setStyleSheet("border:none; padding:0px;margin:0px;margin-top:15px;margin-right:12px;margin-bottom:15px;");

    lblTitle = new QLabel(this);
    lblTitle->setText(strTitle);
    lblTitle->setStyleSheet("color:#FFFFFF;font:14px solid;margin:0px;margin-top:8px;margin-bottom:37px;");

    QHBoxLayout* hlButtons = new QHBoxLayout();
    hlButtons->setContentsMargins(0, 0, 0, 27);

    butOk = new QPushButton(this);

    butOk->setText("OK");
    butOk->setStyleSheet("background-color:#538CCF;color:#FFFFFF;font:12px solid;width:110px;height:28px;border:none;border-radius:4px;");

    hlButtons->setSpacing(8);
    hlButtons->addStretch(1);
    hlButtons->addWidget(butOk);
    hlButtons->addStretch(1);

    vlTop->addWidget(butClose, 1, Qt::AlignRight);
    vlTop->addWidget(lblTitle, 1, Qt::AlignHCenter);
    vlTop->addLayout(hlButtons, 1);

    connect(butClose, &QPushButton::clicked, this, &OkDialog::Slot_Close);
    connect(butOk, &QPushButton::clicked, this, &OkDialog::Slot_Ok);

    frameTop->setLayout(vlTop);
    vlMain->addWidget(frameTop, 1);

    setLayout(vlMain);
    //pYesNoCancelDialog = this;
}

void OkDialog::Slot_Ok()
{
    done(QMessageBox::Yes);
}

void OkDialog::Slot_Close()
{
    done(QMessageBox::Cancel);
}

int OpenOkDialog(QWidget* parent, QString strTitle, int w, int h)
{
    std::cout << "open ync dialog 1." << std::endl;
    if (pOkDialog)
    {
        std::cout << "open ync dialog 2." << std::endl;
        return QMessageBox::Cancel;
    }

    std::cout << "open ync dialog 3." << std::endl;
    pOkDialog = new OkDialog(parent, strTitle);
    pOkDialog->setWindowFlags(pOkDialog->windowFlags() | Qt::FramelessWindowHint);
    pOkDialog->setAttribute(Qt::WA_TranslucentBackground);

    pOkDialog->resize(w, h);

    ///pYesNoCancelDialog->show();

    ///return QMessageBox::Yes;
    std::cout << "open ync dialog 4." << std::endl;
    return pOkDialog->exec();
}

void CloseOkDialog()
{
    if (!pOkDialog)
        return;
}
TilesList* pTilesList = nullptr;

TilesList::TilesList(AI3D::CORE::ReconstructionObject* recons_object_, QWidget* parent, ParamSettings4Production* paramSettings4Production)
    : QWidget(parent)
    ///: QDialog(parent)
{
    bCanBeEditable = false;
    bOsgEngineCleared = false;

    ///setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color:#2D3035;border:2px solid #2D3035;margin:0px;padding:0px;");

    this->recons_object = recons_object_;
    this->paramSettings4Production = paramSettings4Production;

    QHBoxLayout* hlMain = new QHBoxLayout();
    hlMain->setContentsMargins(2, 2, 2, 2);

    QVBoxLayout* vlMain = new QVBoxLayout();
    vlMain->setContentsMargins(0, 0, 0, 0);

    QFrame* frameTop = new QFrame(this);
    frameTop->setStyleSheet("background-color:#2D3035;margin:0px;padding:0px;border-top-left-radius:0px;border-bottom-left-radius:0px;border:none;");

    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(0, 0, 0, 0);

    QFrame* frameTitle = new QFrame(this);
    frameTitle->setContentsMargins(0, 0, 0, 0);
    frameTitle->setFixedHeight(70);
    frameTitle->setStyleSheet("QFrame { background-color:#2D3035; border-bottom-left-radius:0px;border-bottom-right-radius:0px;font:14px solid;}");

    QHBoxLayout* hlTitle = new QHBoxLayout();
    hlTitle->setContentsMargins(0, 0, 0, 0);

    //setAttribute()
    butClose = new QPushButton(this);
    ///butClose->setText("Close");
    butClose->setStyleSheet("background-color:#2D3035;width:30px;height:30px;image:url(:/new/prefix1/skin/pleftarrow.png);padding:0px;margin:0px;margin-top:20px;margin-top:20px;margin-bottom:20px;");

    QLabel* lblTitle = new QLabel(this);
    if (BlockObject::isChineseVersion())
    {
        lblTitle->setText("分块选择");
    }
    else
    {
        lblTitle->setText("Tiles Selection");
    }
    lblTitle->setStyleSheet("background-color:transparent;color:white;font:16px \"Arial\";");

    butEdit = new QPushButton(this);
    if (BlockObject::isChineseVersion())
    {
        butEdit->setText("编辑");
    }
    else
    {
        butEdit->setText("Edit");
    }

    butEdit->setStyleSheet("QPushButton { background-color:##5A5C62;color:#FFFFFF;font:14px \"Arial\";border-radius:2px;}"
    );

    butEdit->setFixedWidth(54);
    butEdit->setFixedHeight(24);

    cbPhotos = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbPhotos->setText("影像");
    }
    else {
        cbPhotos->setText("Photos");
    }
    cbPhotos->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
    if (this->recons_object->GetATData().HasImages())
    {
        cbPhotos->setChecked(true);
        cbPhotos->setEnabled(true);
    }
    else
    {
        cbPhotos->setChecked(false);
        cbPhotos->setEnabled(false);
    }

    cbTiePoints = new QCheckBox(this);
    if (BlockObject::isChineseVersion())
    {
        cbTiePoints->setText("连接点");
    }
    else {
        cbTiePoints->setText("TiePoints");
    }
    cbTiePoints->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
    if (this->recons_object->GetATData().HasTiepoints())
    {
        cbTiePoints->setChecked(true);
        cbTiePoints->setEnabled(true);
    }
    else
    {
        cbTiePoints->setChecked(false);
        cbTiePoints->setEnabled(false);
    }

    cbGCP = new QCheckBox(this);
    if (BlockObject::isChineseVersion())
    {
        cbGCP->setText("控制点");
    }
    else
    {
        cbGCP->setText("GCP");
    }
    cbGCP->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
    if (this->recons_object->GetATData().HasSurveyPoints())
    {
        cbGCP->setChecked(true);
        cbGCP->setEnabled(true);
    }
    else
    {
        cbGCP->setChecked(false);
        cbGCP->setEnabled(false);
    }

    cbTiling = new QCheckBox(this);
    if (BlockObject::isChineseVersion())
    {
        cbTiling->setText("分块");
    }
    else
    {
        cbTiling->setText("Tiling");
    }
    cbTiling->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
    if (this->recons_object->HasTiles())
    {
        cbTiling->setChecked(true);
        cbTiling->setEnabled(true);
    }
    else
    {
        cbTiling->setChecked(false);
        cbTiling->setEnabled(false);
    }

    cbROI = new QCheckBox(this);
    if (BlockObject::isChineseVersion())
    {
        cbROI->setText("兴趣区");
    }
    else
    {
        cbROI->setText("ROI");
    }
    cbROI->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");

    {
        cbROI->setChecked(true);
        cbROI->setEnabled(true);
    }

    cbConstraints = new QCheckBox(this);
    if (BlockObject::isChineseVersion()) {
        cbConstraints->setText("约束");
    }
    else {
        cbConstraints->setText("Constraints");
    }

    cbConstraints->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
    if (this->recons_object->HasConstraints())
    {
        cbConstraints->setChecked(true);
        cbConstraints->setEnabled(true);
    }
    else
    {
        cbConstraints->setChecked(false);
        cbConstraints->setEnabled(false);
    }

    {
        if (!this->recons_object->GetATDataMutual().HasControlPoints())
        {


            cbGCP->setChecked(false);
            cbGCP->setEnabled(false);
        }


        if (!this->recons_object->GetATDataMutual().HasImages())
        {
            cbPhotos->setChecked(false);
            cbPhotos->setEnabled(false);
        }

        if (!this->recons_object->GetATDataMutual().HasTiepoints())
        {
            cbTiePoints->setChecked(false);
            cbTiePoints->setEnabled(false);
        }

    }

    QFrame* middleLine = new QFrame(this);
    middleLine->setFrameShape(QFrame::Shape::VLine);
    middleLine->setFrameShadow(QFrame::Shadow::Plain);
    middleLine->setStyleSheet("max-width:1px;border:none;background-color:#14FFFFFF;padding:0px;margin:0px;height:36px;");
    middleLine->setFixedHeight(36);

    cbbSelectionMode = new QComboBox(this);


    {
        cbbSelectionMode->addItem(D3_VIEW_SELECTION_MODE_SINGLE_ITEM);
        cbbSelectionMode->addItem(D3_VIEW_SELECTION_MODE_RECTANGLE);
        cbbSelectionMode->addItem(D3_VIEW_SELECTION_MODE_POLYGON);
    }
    if (AI3D::CORE::BlockObject::isChineseVersion())
    {
        cbbSelectionMode->setItemData(0, "单选", Qt::DisplayRole);
        cbbSelectionMode->setItemData(1, "矩形框选", Qt::DisplayRole);
        cbbSelectionMode->setItemData(2, "多边形框选", Qt::DisplayRole);

    }
    cbbSelectionMode->setStyleSheet("background-color:white;color:black;font:12px \"Arial\";");

    cbbSelectionMode->setStyleSheet(QString::fromUtf8("\n"
        "QComboBox {\n"
        "    background-color:#34363A;"
        "    border: 0px solid;   \n"
        "    border-radius: 4px;   \n"
        "    color: #FFFFFF;\n"
        "   font: 12px \"Arial\";\n"
        "   margin-left:0px; \n"
        "   margin-right:0px; \n"
        "   padding:0px;\n"
        "   padding-left: 13px;\n"
        "   height:24px; \n"
        "}\n"
        "QComboBox:disabled {\n"
        "   color: white;\n"
        "   background-color:gray;\n"
        "}\n"
        "QComboBox::drop-down {\n"
        "   subcontrol-position:top right;\n"
        "   subcontrol-origin:padding;\n"
        "   width:32px;\n"
        "   border:none;\n"
        "}\n"
        "QComboBox::down-arrow { \n"
        "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png)"
        "}\n"
        "QComboBox QAbstractScrollArea {\n"
        "    width: 10px;\n"
        "    color: black; \n"
        "    background-color:white;\n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
        "    width: 10px;\n"
        "    background-color: #d0d2d4;  \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
        "    border-radius: 5px;   "
        "    background: rgb(160,160,160);   \n"
        "}\n"
        "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
        "    background: rgb(90, 91, 93);   \n"
        "}\n"
        "QComboBox QAbstractItemView {\n"
        "    outline: 0px solid gray;   \n"
        "    border: none;   \n"
        "    color:#FFFFFF;\n"
        "    background-color: #131313;  \n"
        "    selection-background-color:#333333;   \n"
        "    padding-left: 0px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "}\n"
        "QComboBox QAbstractItemView::item {\n"
        "    height: 24px;   \n"
        "    border:none; \n"
        "    background-color:#3F4146;\n"
        "    color:#FFFFFF;"
        "    padding-left: 10px; \n"
        "    margin-left:0px; \n"
        "    margin-right:0px; \n"
        "    font:14px \"Arial\";"
        "}\n"
        "QComboBox QAbstractItemView::item:hover {\n"
        "    color: #FFFFFF;\n"
        "    background-color: #34363A;   \n"
        "}\n"
        "QComboBox QAbstractItemView::item:selected {\n"
        "    color: #FFFFFF;\n"
        "    background-color:#34363A;\n"
        "}\n"
    ));

    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    cbbSelectionMode->setItemDelegate(itemDelegate);

    hlTitle->setSpacing(10);
    hlTitle->addSpacing(0);
    hlTitle->addWidget(butClose);
    hlTitle->addSpacing(20);
    hlTitle->addWidget(lblTitle);
    hlTitle->addStretch(1);

    hlTitle->addWidget(cbPhotos);
    //hlTitle->addSpacing(26);
    hlTitle->addSpacing(10);
    hlTitle->addWidget(cbTiePoints);
    hlTitle->addSpacing(10);
    hlTitle->addWidget(cbGCP);
    hlTitle->addSpacing(10);
    hlTitle->addWidget(cbTiling);
    hlTitle->addSpacing(10);
    hlTitle->addWidget(cbROI);
    hlTitle->addSpacing(10);
    hlTitle->addWidget(cbConstraints);

    //hlTitle->addSpacing(30);
    hlTitle->addSpacing(8);
    hlTitle->addWidget(middleLine);
    hlTitle->addSpacing(22);
    hlTitle->addWidget(butEdit);
    hlTitle->addSpacing(16);
    hlTitle->addWidget(cbbSelectionMode);
    hlTitle->addSpacing(30);

    frameTitle->setLayout(hlTitle);

    QWidget* widWindow = new QWidget(this);
    widWindow->setContentsMargins(0, 0, 0, 0);
    widWindow->setStyleSheet("border:none;margin:0px;padding:0px;");

    mWindow = new AI3D::GUI::MWindow(widWindow, 0, true, false, true, true, 970, 768);
    mWindow->setContentsMargins(0, 0, 0, 0);
    mWindow->setStyleSheet("border:none;margin:0px;padding:0px;");

    vlTop->addSpacing(0);
    vlTop->addWidget(frameTitle);
    ///vlTop->addWidget(widWindow, 1,Qt::AlignLeft);
    vlTop->addWidget(widWindow, 1);
    ///vlTop->addStretch(1);

    connect(butClose, &QPushButton::clicked, this, &TilesList::Slot_Close);

    frameTop->setLayout(vlTop);

    vlMain->addWidget(frameTop, 1);

    QFrame* frameRight = new QFrame(this);

    frameRight->setFixedWidth(355);
    frameRight->setStyleSheet("background-color:#20242B;margin:0px;padding:0px;border:none;");

    QVBoxLayout* vlRight = new QVBoxLayout();
    vlRight->setContentsMargins(0, 0, 0, 0);

    QLabel* lblRightTitle = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblRightTitle->setText("分块");
    }
    else {
        lblRightTitle->setText("Tile");
    }
    ///lblRightTitle->setStyleSheet("background-color:transparent;color:white;height:48px;font:14px solid;margin-left:13px;");
    ///lblRightTitle->setFixedHeight(48);
    lblRightTitle->setStyleSheet("background-color:transparent;color:white;font:16px \"Arial\";margin-left:13px;margin-top:30px;margin-bottom:22px;");

    QHBoxLayout* hlSummaryAndActions = new QHBoxLayout();
    hlSummaryAndActions->setContentsMargins(13, 0, 17, 0);

    lblTilesSummary = new QLabel(this);
    // select all the tiles when being initial.


    ///lblTilesSummary->setText("0/23");
    if (this->recons_object != nullptr)
    {
        auto tileset = this->recons_object->GetTilesName(this->recons_object->GetProcessingSettings().bdiscard_emptytiles_);
        tilescount_ = tileset.size();
        lblTilesSummary->setText(QString("%1/%2").arg(tileset.size()).arg(tilescount_));
    }
    else
        lblTilesSummary->setText("0/23");

    lblTilesSummary->setStyleSheet("background-color:transparent;color:#CCFFFFFF;font:bold 12px \"Arial\";");

    lblTilesSuffix = new QLabel(this);
    if (BlockObject::isChineseVersion()) {
        lblTilesSuffix->setText("块（已选择）");
    }
    else
    {
        lblTilesSuffix->setText("tiles selected");
    }
    lblTilesSuffix->setStyleSheet("background-color:transparent;color:#CCFFFFFF;font:12px \"Arial\";");

    lblSelectAll = new QLabel(this);
    lblInvert = new QLabel(this);

    if (BlockObject::isChineseVersion()) {
        lblSelectAll->setText("<a href='http://' style='background-color:transparent;color:#CC7492FC;font:12px Arial;'>全选</a>");
        lblInvert->setText("<a href='http://' style='background-color:transparent;color:#CC7492FC;font:12px Arial;'>反选</a>");
    }
    else {
        lblSelectAll->setText("<a href='http://' style='background-color:transparent;color:#CC7492FC;font:12px Arial;'>Select All</a>");
        lblInvert->setText("<a href='http://' style='background-color:transparent;color:#CC7492FC;font:12px Arial;'>Invert</a>");
    }

    hlSummaryAndActions->addSpacing(0);
    hlSummaryAndActions->addWidget(lblTilesSummary);
    hlSummaryAndActions->addSpacing(10);
    hlSummaryAndActions->addWidget(lblTilesSuffix);
    hlSummaryAndActions->addStretch(1);
    hlSummaryAndActions->addWidget(lblSelectAll);
    hlSummaryAndActions->addSpacing(20);
    hlSummaryAndActions->addWidget(lblInvert);

    twTiles = new QTableWidget(this);
    twTiles->setColumnCount(3);
    ///twTiles->horizontalHeader()->hide();
    twTiles->verticalHeader()->hide();
    /// twTiles->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    twTiles->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    twTiles->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList slTitles;

    if (BlockObject::isChineseVersion()) {
        slTitles << "块名称" << "状态" << "状态时间";
    }
    else {
        slTitles << "Tile" << "Status" << "State Time";
    }

    twTiles->setHorizontalHeaderLabels(slTitles);


    twTiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ///twTiles->setSelectionMode(QAbstractItemView::MultiSelection);
    twTiles->setShowGrid(false);

    twTiles->setStyleSheet("QTableWidget { background-color:transparent;margin-left:7px;margin-right:7px;margin-top:10px;margin-bottom:16px;outline:none;border-radius:2px;border:1px solid #2C2F34;}"
        "QHeaderView::section { background-color:#FF2C2F34; color:#A5A5A5; font:12px \"Arial\";border:0px solid #FF2C2F34;border-right:1px solid #FF000000;} "
        "QTableWidget::item { background-color:#FF20242B;color:#FFFFFF;font:10px \"Arial\";border:0px solid #FF2C2F34;border-bottom:1px solid #FF2C2F34;height:35px;}"
        "QTableWidget::item:selected { background-color:#FF2A4D84;}"
    );
    //@attention 原来 selected 为darkblue 觉得太难看了，所以改为blue @chy modified @20231212
    ///     "QTableWidget::item:selected { background-color:#802C2F34;}"

    //chy 2024年4月12：参照cc，如果已经选择了再次打开显示的是已经选择的

    if (this->recons_object != nullptr && this->recons_object->GetNumTiles() > 0 && false)
    {
        //auto &global_tiles_base = this->recons_object->GetTilesBase();
        auto& global_tiles_custom = this->recons_object->GetTilesCustom();
        auto tilesset = this->recons_object->GetTilesName(this->recons_object->GetProcessingSettings().bdiscard_emptytiles_);

        /*if (!pParamSettings4Production->tilingRange->tiles_selected_.empty())
        {
            tilesset.clear();
            (pParamSettings4Production->tilingRange->tiles_selected_.begin(),
                pParamSettings4Production->tilingRange->tiles_selected_.end());
            std::cout << tilesset.size() << "---- "<<  tilesseleceted.size() << " " << std::endl;
            tilesset = tilesseleceted;
            std::cout << tilesset.size() << "---+++- " << tilesseleceted.size() << " " << std::endl;

        }*/
        /*std::vector<std::string> tilestemps;
        tilestemps.assign(tilesset.begin(), tilesset.end());
        auto tilessetselect = pParamSettings4Production->tilingRange->tiles_selected_.empty() ? tilestemps : pParamSettings4Production->tilingRange->tiles_selected_;*/
        //std::vector<std::string> vecs = this->recons_object->GetOrderedTiles();
        int i = 0;
        //std::vector<image_t> tiles_selid;
        for (auto& t : tilesset)
        {
            if (!global_tiles_custom.count(t))
                continue;

            twTiles->insertRow(twTiles->rowCount());

            for (int j = 0; j < twTiles->columnCount(); j++)
            {
                QTableWidgetItem* pItem = new QTableWidgetItem();
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                pItem->setTextColor(QColor(0xff, 0xff, 0xff));
                if (j == 0)
                {
                    pItem->setText(QString::fromStdString(t));
                }
                else if (j == 1)
                {
                    if (global_tiles_custom.at(t).reference_model_status_ == tile_info_s::reconst_status_e::RE_STA_COMPLETED)
                    {
                        if (BlockObject::isChineseVersion())
                        {
                            pItem->setText("完成");
                        }
                        else
                        {
                            pItem->setText("completed");
                        }
                        pItem->setTextColor(QColor(0xa5, 0xa5, 0xa5));
                    }
                    else
                    {
                        if (BlockObject::isChineseVersion())
                        {
                            pItem->setText("未处理");
                        }
                        else
                        {
                            pItem->setText("unprocessed");
                        }
                    }
                }
                else if (j == 2)
                {
                    pItem->setText("--");
                }

                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setData(Qt::UserRole + 1, global_tiles_custom.at(t).index_);
                twTiles->setItem(twTiles->rowCount() - 1, j, pItem);


            }


            /*if (std::find(tilessetselect.begin(), tilessetselect.end(), t) != tilessetselect.end())
            {
                tiles_selid.push_back((image_t)i);
            }*/


            i++;
        }
        //Slot_SelectedTiles(tiles_selid);
    }


    vlRight->addSpacing(0);
    vlRight->setSpacing(0);
    vlRight->addWidget(lblRightTitle);
    vlRight->addLayout(hlSummaryAndActions);
    vlRight->addWidget(twTiles, 1);
    ///vlRight->addStretch(1);

    frameRight->setLayout(vlRight);

    hlMain->addSpacing(0);
    hlMain->setSpacing(0);
    hlMain->addLayout(vlMain, 1);
    hlMain->addWidget(frameRight);

    RefreshEditMode();

    ///mWindow->RenderReconstruction(recons_object);
    ///mWindow->RenderModel("D:\\osgb");

    connect(cbPhotos, &QCheckBox::clicked, this, &TilesList::Slot_SelectTypes);
    connect(cbTiePoints, &QCheckBox::clicked, this, &TilesList::Slot_SelectTypes);
    connect(cbGCP, &QCheckBox::clicked, this, &TilesList::Slot_SelectTypes);
    connect(cbTiling, &QCheckBox::clicked, this, &TilesList::Slot_SelectTypes);
    connect(cbROI, &QCheckBox::clicked, this, &TilesList::Slot_SelectTypes);
    connect(cbConstraints, &QCheckBox::clicked, this, &TilesList::Slot_SelectTypes);

    connect(cbbSelectionMode, &QComboBox::currentTextChanged, this, &TilesList::Slot_SelectionModeChanged);

    connect(lblSelectAll, &QLabel::linkActivated, this, &TilesList::Slot_SelectAll);
    connect(lblInvert, &QLabel::linkActivated, this, &TilesList::Slot_Invert);

    connect(mWindow, &AI3D::GUI::MWindow::signal_selected_tiles, this, &TilesList::Slot_SelectedTiles);

    connect(twTiles, &QTableWidget::clicked, this, &TilesList::TilesListClicked);
    connect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);

    connect(butEdit, &QPushButton::clicked, this, &TilesList::Slot_EditModeChanged);

    QTimer::singleShot(100, this, &TilesList::Slot_Display3DView);

    setLayout(hlMain);
}

TilesList::~TilesList()
{
    //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
    //  " at " << __DATE__ << " / " << __TIME__ << std::endl;
    //if(mWindow != nullptr)
    //  mWindow->getOsgEngine()->RemoveAll();
    ClearOsgData();
    pTilesList = nullptr;
}

void TilesList::SetTilesSelected()
{
    //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    // 1.clearSelectedElement(tile_type) 2.setSelectElement(tile_type);
    int iSelectedRows = 0;
    QList<QTableWidgetItem*> selectedItems = twTiles->selectedItems();
    std::vector<int> selectedIndex;
    for (int i = 0; i < twTiles->rowCount(); i++)
    {
        QTableWidgetItem* pItem = twTiles->item(i, 0);
        if (selectedItems.contains(pItem))
        {
            int index = pItem->data(Qt::UserRole + 1).toInt();
            selectedIndex.push_back(index);
            iSelectedRows++;
        }
    }

    mWindow->getOsgEngine()->ClearSelectElement(); // (ELEMENT_LAYER_TYPE::ELEMENT_TILE);
    mWindow->getOsgEngine()->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_TILE, selectedIndex);

    lblTilesSummary->setText(QString("%1/%2").arg(iSelectedRows).arg(tilescount_));
}

void TilesList::InitTileListItem()
{
    // initial codes to fill tiles fetched from the related recounstruction objec into tile list widget.
    if (!this->recons_object ||
        this->recons_object->GetNumTiles() <= 0)
        return;

    twTiles->setUpdatesEnabled(false);

    QTableWidgetItem* pItem;
    auto& global_tiles_custom = this->recons_object->GetTilesCustom();
    auto tilesset = this->recons_object->GetTilesName(this->recons_object->GetProcessingSettings().bdiscard_emptytiles_);
    int iColumnCount = twTiles->columnCount();
    int i = 0;

    for (auto& t : tilesset)
    {
        if (!global_tiles_custom.count(t))
            continue;

        twTiles->insertRow(twTiles->rowCount());

        for (int j = 0; j < iColumnCount; j++)
        {
            pItem = new QTableWidgetItem();
            pItem->setTextColor(QColor(0xff, 0xff, 0xff));

            if (j == 0)
                pItem->setText(QString::fromStdString(t));
            else if (j == 2)
                pItem->setText("--");
            else if (j == 1)
            {
                if (global_tiles_custom.at(t).reference_model_status_ == tile_info_s::reconst_status_e::RE_STA_COMPLETED)
                {
                    if (BlockObject::isChineseVersion())
                        pItem->setText("完成");
                    else
                        pItem->setText("completed");

                    pItem->setTextColor(QColor(0xa5, 0xa5, 0xa5));
                }
                else
                {
                    if (BlockObject::isChineseVersion())
                        pItem->setText("未处理");
                    else
                        pItem->setText("unprocessed");
                }
            }

            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
            pItem->setTextAlignment(Qt::AlignCenter);
            pItem->setData(Qt::UserRole + 1, global_tiles_custom.at(t).index_);
            twTiles->setItem(i, j, pItem);
        }

        i++;
    }

    twTiles->setUpdatesEnabled(true);
}

void TilesList::Reset()
{

}

void TilesList::ClearOsgData()
{
    if (bOsgEngineCleared)
        return;

    if (mWindow != nullptr)
        mWindow->getOsgEngine()->RemoveAll();
    bOsgEngineCleared = true;
}

void TilesList::TilesListClicked()
{
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    SetTilesSelected();
}

void TilesList::SetSelectionModeExtra()
{
}

void TilesList::Slot_Close()
{
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
        " at " << __DATE__ << " / " << __TIME__ << std::endl;
    //QWidget *parentWidget_ = parentWidget();

    int ret;
    if (BlockObject::isChineseVersion()) {
        ret = OpenYesNoCancelDialog(this, "你想应用新的分块选择吗?");
    }
    else {
        ret = OpenYesNoCancelDialog(this, "Do you want to apply selection changes?");
    }
    if (ret == QMessageBox::Cancel)
    {
        std::cout << "got cancel clicked." << std::endl;
        return;
    }

    if (ret == QMessageBox::Yes)
    {
        std::cout << "got yes clicked." << std::endl;
        // note: to save selected tiles into TilingRange object of Param Settings.
        std::vector<std::string> selectedTiles;
        QList<QTableWidgetItem*> selectedItems = twTiles->selectedItems();

        for (int i = 0; i < twTiles->rowCount(); i++)
        {
            QTableWidgetItem* pItem = twTiles->item(i, 0);
            if (selectedItems.contains(pItem))
            {
                selectedTiles.push_back(pItem->text().toStdString());
            }
        }

        if (pParamSettings4Production != nullptr && pParamSettings4Production->tilingRange != nullptr)
        {
            //std::cout << "before saving tiles:" << pParamSettings4Production->tilingRange->tiles_selected_.size() << std::endl;
            pParamSettings4Production->tilingRange->tiles_selected_ = selectedTiles;
            pParamSettings4Production->tilingRange->Reset();

            //std::cout << "after saving tiles:" << pParamSettings4Production->tilingRange->tiles_selected_.size() << std::endl;

            //for (auto& t : pParamSettings4Production->tilingRange->tiles_selected_)
            //  std::cout << t << std::endl;

            /// note: need it after optimization for popup window TilesList? check it later.
            /// pParamSettings4Production->tilingRange->Reset();
        }

        /// note: need it after optimization for popup window TilesList? check it later.
        /// mWindow->getOsgEngine()->RemoveAll();
    }
    else if (ret == QMessageBox::No)
    {
        std::cout << "got no clicked." << std::endl;
    }

    std::cout << "got close clicked2." << std::endl;

    hide();

    if (paramSettings4Production)
        paramSettings4Production->show();

    /// note: need it after optimization for popup window TilesList? check it later.
/// close();

    //parentWidget_->show();
}

void TilesList::RefreshEditMode()
{
    if (bCanBeEditable)
    {
        butEdit->setStyleSheet("QPushButton { background-color:#165DFF;color:#FFFFFF;font:14px \"Arial\";border-radius:2px;}"
            "QPushButton:hover { background-color: #1457EE; } ");

        //mWindow->setEnabled(true);
        cbbSelectionMode->setEnabled(true);
        twTiles->setEnabled(true);
        lblSelectAll->setEnabled(true);
        lblInvert->setEnabled(true);
        mWindow->getOsgEngine()->SetSelectType(SELECT_TYPE::SELECT_ONE);
    }
    else
    {
        butEdit->setStyleSheet("QPushButton { background-color:#5A5C62;color:#FFFFFF;font:14px \"Arial\";border-radius:2px;}");

        //mWindow->setEnabled(false);
        //mWindow只能看
        mWindow->getOsgEngine()->SetSelectType(SELECT_TYPE::SELECT_NONE);
        //mWindow->getOsgEngine()->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_TILE);

        cbbSelectionMode->setEnabled(false);
        twTiles->setEnabled(false);
        lblSelectAll->setEnabled(false);
        lblInvert->setEnabled(false);
    }
}

void TilesList::Slot_EditModeChanged()
{
    bCanBeEditable = !bCanBeEditable;
    RefreshEditMode();
}

void TilesList::SetSelectItems()
{
    //QMessageBox::information(nullptr, "select all clicked", "str:" + str);
    disconnect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
    twTiles->setSelectionMode(QTableWidget::MultiSelection);
    twTiles->clearSelection();
    if (pParamSettings4Production->tilingRange->tiles_selected_.empty())
    {
        for (int i = 0; i < twTiles->rowCount(); i++)
        {
            twTiles->selectRow(i);

        }
    }
    else
    {
        for (int i = 0; i < twTiles->rowCount(); i++)
        {
            auto tilessetselect = pParamSettings4Production->tilingRange->tiles_selected_;

            QTableWidgetItem* pItem = twTiles->item(i, 0);
            std::string tilename = pItem->text().toStdString();
            if (std::find(tilessetselect.begin(), tilessetselect.end(), tilename) != tilessetselect.end())
            {
                twTiles->selectRow(i);
            }

        }
    }

    twTiles->setSelectionMode(QTableWidget::ExtendedSelection);
    SetTilesSelected();
    connect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
}

void TilesList::Slot_SelectAll(const QString& str)
{
    //QMessageBox::information(nullptr, "select all clicked", "str:" + str);
    disconnect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
    twTiles->setSelectionMode(QTableWidget::MultiSelection);
    twTiles->clearSelection();

    for (int i = 0; i < twTiles->rowCount(); i++)
    {
        twTiles->selectRow(i);
    }

    twTiles->setSelectionMode(QTableWidget::ExtendedSelection);
    SetTilesSelected();
    connect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
}

void TilesList::Slot_Invert(const QString& str)
{
    ///QMessageBox::information(nullptr, "invert clicked", "str:" + str);
    disconnect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
    QList<QTableWidgetItem*> selectedItems = twTiles->selectedItems();
    twTiles->setSelectionMode(QTableWidget::MultiSelection);
    twTiles->clearSelection();

    for (int i = 0; i < twTiles->rowCount(); i++)
    {
        QTableWidgetItem* pItem = twTiles->item(i, 0);
        if (selectedItems.contains(pItem))
            ;
        else
            twTiles->selectRow(i);
    }

    twTiles->setSelectionMode(QTableWidget::ExtendedSelection);
    SetTilesSelected();
    connect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
}
void TilesList::SetLayerType()
{
    std::set<AI3D::VIEWER::reconst_element_e> imageLayers;
    if (cbPhotos->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_PHOTOS);

    if (cbTiePoints->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_TIEPOINTS);

    if (cbGCP->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_GCP);

    if (cbTiling->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_TILE);

    if (cbROI->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_ROI);

    if (cbConstraints->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_CONSTRAINT);

    if (mWindow != nullptr)
        mWindow->ResetImageLayerSeleted(imageLayers);
}

void TilesList::Slot_SelectTypes()
{
    std::set<AI3D::VIEWER::reconst_element_e> imageLayers;

    QCheckBox* pCheckBox = dynamic_cast<QCheckBox*>(sender());
    if (!pCheckBox)
        return;

    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << pCheckBox->text().toStdString() << std::endl;

    if (cbPhotos->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_PHOTOS);

    if (cbTiePoints->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_TIEPOINTS);

    if (cbGCP->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_GCP);

    if (cbTiling->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_TILE);

    if (cbROI->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_ROI);

    if (cbConstraints->isChecked())
        imageLayers.insert(AI3D::VIEWER::RD_ELE_CONSTRAINT);

    if (mWindow != nullptr)
        mWindow->ResetImageLayerSeleted(imageLayers);


}

void TilesList::Slot_SelectionModeChanged(const QString& str)
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << str.toStdString() << __LINE__ << std::endl;

    if (AI3D::CORE::BlockObject::isChineseVersion())
    {
        if (str == "单选")
        {
            // Single Item
            if (mWindow != nullptr)
            {
                std::cout << "set mw single mode." << std::endl;
                mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_SINGLE_MODE);
            }
        }
        else if (str == "矩形框选")
        {
            // Rectangle
            if (mWindow != nullptr)
            {
                std::cout << "set rectangle mode." << std::endl;
                mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_RECT_MODE);
            }
        }
        else if (str == "多边形框选")
        {
            // Polygon
            if (mWindow != nullptr)
            {
                std::cout << "set polygon mode." << std::endl;
                mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_POLYGON_MODE);
            }
        }
    }
    else
    {
        if ((str == D3_VIEW_SELECTION_MODE_SINGLE_ITEM))
        {
            // Single Item
            if (mWindow != nullptr)
            {
                std::cout << "set mw single mode." << std::endl;
                mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_SINGLE_MODE);
            }
        }
        else if (str == D3_VIEW_SELECTION_MODE_RECTANGLE)
        {
            // Rectangle
            if (mWindow != nullptr)
            {
                std::cout << "set rectangle mode." << std::endl;
                mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_RECT_MODE);
            }
        }
        else if (str == D3_VIEW_SELECTION_MODE_POLYGON)
        {
            // Polygon
            if (mWindow != nullptr)
            {
                std::cout << "set polygon mode." << std::endl;
                mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_POLYGON_MODE);
            }
        }
    }
}

void TilesList::Slot_Display3DView()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << std::endl;
    if (mWindow != nullptr && this->recons_object != nullptr)
    {
        LOGI("=====================Rendering,pls wait=============");
        std::cout << "======================Rendering,pls wait==================" << std::endl;
        bool bRunFinished = false;
        SetLayerType();
        auto savefunc = [&, this]()
            {

                mWindow->RenderReconstruction(this->recons_object, true);


                bRunFinished = true;
                return;
            };
        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
        {
            if (BlockObject::isChineseVersion())
            {
                OpenLoadingPromptV4("渲染中，请耐心等待");
            }
            else
            {
                OpenLoadingPromptV4("Please be patient and wait.rendering");
            }

            QFuture<void> f1 = QtConcurrent::run(savefunc);

            InitTileListItem();

            while (!bRunFinished)
            {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        else
        {
            savefunc();
        }
        SetSelectItems();
        //Slot_SelectAll("");

        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
        {
            CloseLoadingPromptV4();
        }

        LOGI("=====================Render end=============");
        std::cout << "======================Render end==================" << std::endl;
    }
}

void TilesList::Slot_SelectedTiles(std::vector<image_t>& tiles)
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    //for (auto& t : tiles)
    //{
    //  std::cout << "select tile:" << t << std::endl;
    //}

    disconnect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
    twTiles->clearSelection();
    twTiles->setSelectionMode(QTableWidget::MultiSelection);

    int iSelectedRows = 0;

    if (twTiles->rowCount() > 0)
    {
        for (int i = 0; i < twTiles->rowCount(); i++)
        {
            QTableWidgetItem* pItem = twTiles->item(i, 0);
            int index = pItem->data(Qt::UserRole + 1).toInt();
            if (std::find(tiles.begin(), tiles.end(), index) != tiles.end())
            {
                //      std::cout << "select row:" << i << " index:" << index << std::endl;
                twTiles->selectRow(i);
                iSelectedRows++;
            }
        }
    }

    lblTilesSummary->setText(QString("%1/%2").arg(iSelectedRows).arg(tilescount_));

    twTiles->setSelectionMode(QTableWidget::ExtendedSelection);
    connect(twTiles, &QTableWidget::itemSelectionChanged, this, &TilesList::Slot_ItemSelectionChanged);
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
}

void TilesList::Slot_ItemSelectionChanged()
{
    SetTilesSelected();
}

void CloseTilesList();

int OpenTilesList(ParamSettings4Production* paramSettings4Production, AI3D::CORE::ReconstructionObject* recons_object_, QWidget* parent, int w, int h)
{
    if (pTilesList != nullptr)
    {
        if (!pTilesList->isVisible())
        {
            pTilesList->show();
            pTilesList->raise();
        }

        return 0;
    }

    pTilesList = new TilesList(recons_object_, parent, paramSettings4Production);
    ///pTilesList->setWindowModality(Qt::ApplicationModal);
    pTilesList->setWindowModality(Qt::WindowModal);
    ///pTilesList->setAttribute(Qt::WA_DeleteOnClose); 
    pTilesList->setWindowFlags(pTilesList->windowFlags() | Qt::FramelessWindowHint);

    if (w < 1329)
        w = 1329;
    if (h < 842)
        h = 842;

    pTilesList->resize(w, h);
    ///int result = pTilesList->exec();

    int result = 0;
    // note:click will switch windows focus.
    pTilesList->show();

    ///CloseTilesList();

    return result;
}

void CloseTilesList()
{
    if (pTilesList == nullptr)
        return;

    pTilesList->close();
    pTilesList = nullptr;
}

void HideTilesList()
{
    if (pTilesList == nullptr)
        return;
    pTilesList->hide();
}

void DeleteTilesList()
{
    if (pTilesList == nullptr)
        return;
    pTilesList->ClearOsgData();
    delete pTilesList;
    pTilesList = nullptr;
}

UserTiePoints* userTiePoints = nullptr;

UserTiePoints::UserTiePoints(AI3D::GUI::ViewWidget* viewWidget, AI3D::CORE::Image& image, QWidget* parent)
    : QDialog(parent)
{
    this->viewWidget = viewWidget;
    this->image = image;
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout* vlMain = new QVBoxLayout();
    vlMain->setContentsMargins(0, 0, 0, 0);

    QFrame* frameMain = new QFrame(this);
    frameMain->setStyleSheet("border-radius:6px;background-color:#303030;border:none;font:14px solid;");

    QVBoxLayout* vlTop = new QVBoxLayout();
    vlTop->setContentsMargins(14, 14, 10, 29);

    QHBoxLayout* hlTitle = new QHBoxLayout();
    lblTitle = new QLabel(this);
    lblTitle->setText("Add point");
    lblTitle->setStyleSheet("background-color:transparent;color:#FFFFFF;");

    butClose = new QPushButton(this);
    butClose->setIcon(QPixmap(":/new/prefix1/skin/close2324.png"));

    hlTitle->addWidget(lblTitle);
    hlTitle->addStretch(1);
    hlTitle->addWidget(butClose);

    QFrame* lineTop = new QFrame(this);
    lineTop->setFrameShape(QFrame::HLine);
    lineTop->setFrameShadow(QFrame::Plain);
    lineTop->setStyleSheet("background-color:#484848;max-height:1px;border:none;margin:0px;padding:0px;");

    QHBoxLayout* hlName = new QHBoxLayout();
    hlName->setContentsMargins(26, 0, 26, 0);

    lblName = new QLabel(this);
    lblName->setText("Name");
    lblName->setStyleSheet("background-color:transparent;color:#FFFFFF;font:14px solid;");

    leName = new QLineEdit(this);
    //leName->setText("Tie point 1");
    {
        std::string imageName = image.GetName();
        size_t pointPos = imageName.find(".");
        if (pointPos != std::string::npos)
            imageName = imageName.substr(0, pointPos);

        ///leName->setText(QString("User_%1").arg(QString::fromStdString(image.GetName())));
        leName->setText(QString("User_%1").arg(QString::fromStdString(imageName)));
    }

    leName->setStyleSheet("background-color:#1D1D1D;color:#FFFFFF;border-radius:2px;border:none;height:32px;padding-left:12px;");

    hlName->setSpacing(16);
    hlName->addSpacing(0);
    //  hlName->addStretch(1);
    hlName->addWidget(lblName);
    hlName->addWidget(leName, 1);
    //hlName->addStretch(1);

    QHBoxLayout* hlType = new QHBoxLayout();
    hlType->setContentsMargins(26, 0, 26, 0);

    lblType = new QLabel(this);
    lblType->setText("Type");
    lblType->setStyleSheet("background-color:transparent;color:#FFFFFF;font:14px solid;");

    cbbType = new QComboBox(this);
    cbbType->addItem("User Tie Point");
    cbbType->addItem("Control Point");
    cbbType->setView(new QListView());
    cbbType->setItemDelegate(new QStyledItemDelegate(cbbType));
    //"QComboBox QAbstractItemView::item { height:26px; padding-left:62px; }"

    cbbType->setStyleSheet("QComboBox {background-color:#1D1D1D;color:#FFFFFF;font:14px solid;height:32px;padding-left:12px;}"
        "QComboBox:disabled { background-color:gray; } "
        "QComboBox::drop-down { border:none; }"
        "QComboBox::down-arrow { image:url(:/new/prefix1/skin/cb_down_arrow1516.png);margin-right:10px; } "
        "QListView {  }"
        "QListView:focus { outline:0px; } "
        "QListView::item { height:26px; padding-left:12px;}"
        "QListView::item:selected { background-color:blue; } "
        "QListView::item:!selected { background-color:white; } "
    );
    cbbType->setCurrentIndex(0);
    cbbType->setEnabled(false);

    hlType->setSpacing(16);
    hlType->addSpacing(0);
    hlType->addWidget(lblType);
    hlType->addWidget(cbbType, 1);

    QHBoxLayout* hlButtons = new QHBoxLayout();
    hlButtons->setContentsMargins(0, 0, 0, 0);

    butAdd = new QPushButton(this);
    butCancel = new QPushButton(this);

    butAdd->setText("Add");
    butCancel->setText("Cancel");

    butAdd->setStyleSheet("background-color:#545454;color:#FFFFFF;border-radius:4px;width:96px;height:28px;margin:0px;");
    butCancel->setStyleSheet("background-color:#545454;color:#FFFFFF;border-radius:4px;width:96px;height:28px;margin:0px;");

    hlButtons->setSpacing(10);
    hlButtons->addStretch(1);
    hlButtons->addWidget(butAdd);
    hlButtons->addWidget(butCancel);
    hlButtons->addStretch(1);

    vlTop->addSpacing(0);
    vlTop->setSpacing(0);
    vlTop->addLayout(hlTitle, 1);
    vlTop->addSpacing(12);
    vlTop->addWidget(lineTop);
    vlTop->addSpacing(40);
    vlTop->addLayout(hlName);
    vlTop->addSpacing(14);
    vlTop->addLayout(hlType);
    vlTop->addSpacing(24);
    vlTop->addLayout(hlButtons);

    frameMain->setLayout(vlTop);
    vlMain->addWidget(frameMain, 1);

    connect(butClose, &QPushButton::clicked, this, &UserTiePoints::Slot_Close);
    connect(butAdd, &QPushButton::clicked, this, &UserTiePoints::Slot_Add);
    connect(butCancel, &QPushButton::clicked, this, &UserTiePoints::Slot_Cancel);

    if (this->viewWidget != nullptr)
    {
        connect(this, &UserTiePoints::signal_add_user_tie_point, this->viewWidget, &AI3D::GUI::ViewWidget::signal_add_user_tie_point);
        connect(this, &UserTiePoints::signal_insert_gcp_tab, this->viewWidget, &AI3D::GUI::ViewWidget::signal_insert_gcp_tab);
    }

    userTiePoints = this;
    setLayout(vlMain);
}

UserTiePoints::~UserTiePoints()
{
    userTiePoints = nullptr;
}

void UserTiePoints::Slot_Close()
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    accept();
}

void UserTiePoints::Slot_Add()
{
    QString userPointName = leName->text();
    if (userPointName.isEmpty())
        return;

    emit signal_add_user_tie_point(this->image, userPointName);

    accept();
}

void UserTiePoints::Slot_Cancel()
{
    reject();
}

int OpenUserTiePoints(AI3D::GUI::ViewWidget* viewWidget, AI3D::CORE::Image& image, QWidget* parent, int w, int h)
{
    if (userTiePoints != nullptr)
        return 0;

    userTiePoints = new UserTiePoints(viewWidget, image, parent);
    userTiePoints->setWindowModality(Qt::ApplicationModal);
    userTiePoints->setAttribute(Qt::WA_DeleteOnClose);
    userTiePoints->setWindowFlags(userTiePoints->windowFlags() | Qt::FramelessWindowHint);

    if (w > 0 && h > 0)
        userTiePoints->resize(QSize(w, h));

    int result = userTiePoints->exec();
    return result;
}

void CloseUserTiePoints()
{
    if (!userTiePoints)
        return;

    userTiePoints->close();
    userTiePoints = nullptr;
}

// GCP ListView 定制组件
MoTableWidget::MoTableWidget(QWidget* parent, int mode)
    : QTableView(parent)
{
    setMouseTracking(true);
    bLeaved = true;
    iHoverRow = -1;
    this->mode = mode;
    // todo:change it based on alter-color(one of two alter color).
    previousHoverRowBackColor = QColor(0x28, 0x28, 0x28);
    previousHoverRow = -1;
    selectedRow = -1;

    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    //setRowCount(10);
    //setColumnCount(8);

    QFont font;
    font.setFamily("Arial");
    font.setPixelSize(14);
    font.setBold(false);
    //?chy
    origBackColor0 = QColor(0x28, 0x28, 0x28);// alt0
    origBackColor1 = QColor(0x3C, 0x3C, 0x3c);// alt1
    //  origBackColor2 = QColor(0x2A,0x4D,0x84);// sel
    //  origBackColor3 = QColor(0x46,0x64,0x94);// sel + hover
    origBackColor2 = Qt::red;// sel
    origBackColor3 = Qt::green;// sel + hover
    origBackColor4 = QColor(0x47, 0x47, 0x47);// hover color.

    // GCP ListView 头部定制组件
    MoHeaderView* moHeaderView = new MoHeaderView(Qt::Horizontal, this);

    //QStringList headerLabels;

    if (mode == 1)
    {
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            headerLabels << "" << "" << "影像名称" << "X" << "Y" << "误差[px]" << "误差[米]";
        }
        else
        {
            headerLabels << "" << "" << "Photo Name" << "X" << "Y" << "RMS[px]" << "RMS[m]";
        }
    }
    else
    {
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            headerLabels << "" << "" << "名称" << "影像数" << "类别" << "X" << "Y" << "Z"
                << tr("计算值X")
                << tr("计算值Y")
                << tr("计算值Z")
                << tr("反投影误差[px]")
                << tr("反投影物方误差[m]")
                << tr("三维误差[米]")
                << tr("水平误差[米]")
                << tr("高程误差[米]");
        }
        else
        {
            headerLabels << "" << "" << "Name" << "Photos" << "Category" << "Given X" << "Given Y" << "Given Z"
                << tr("Estimate\nX")
                << tr("Estimate\nY")
                << tr("Estimate\nZ")
                << tr("RMS of reproj.\nerror[px]")
                << tr("RMS of dis.\nerror[m]")
                << tr("3D err.[m]")
                << tr("3D horizonal err.[m]")
                << tr("3D vertical err.[m]");
        }
    }

    // 使用定制的表头组件
    setHorizontalHeader(moHeaderView);
    ///setHorizontalHeaderLabels(headerLabels);

    horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    //  horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);

    //horizontalHeader()->setStretchLastSection(true);

    // 设置GcpListView 单选且按行选.
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    //setStyleSheet("selection-background-color:rgba(0,0,0,50);");
    //setStyleSheet("");

    if (mode == 1)
    {
        // 设置GcpListView 不可编辑模式(比如切换了坐标模式)
        setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        // 设置GcpListView 可编辑
        setEditTriggers(QAbstractItemView::DoubleClicked);
    }

    ///setAlternatingRowColors(true);

    horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color:#333333;color: #A5A5A5; align:center;}");
    //  verticalHeader()->setStyleSheet("QHeaderView::section{ color: #FFFFFF; }");
    setShowGrid(false);

    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    //  font.setLetterSpacing(QFont::)
        //font.setWeight(400);

    font.setPixelSize(12);
    horizontalHeader()->setFont(font);

    pStandardItemModel = new QStandardItemModel(this);

    if (mode == 1)
    {
        colCount = col_measurement2_e::COUNT_MEASCOL;
    }
    else
    {
        colCount = col_gcp2_e::COUNT_GCPCOL;
    }

    pStandardItemModel->setColumnCount(colCount);

    // 使用定制的GcpListView Item定制组件.
    pItemDelegate = new MoDelegate(this);
    setItemDelegate(pItemDelegate);

    setModel(pStandardItemModel);

    // 表头可按列进行排序.
    setSortingEnabled(true);
    //sortItems(2);

    // 关联进入GcpListView某一行的信号(鼠标未按下时)
    connect(this, &QTableView::entered, this, &MoTableWidget::cellEntered2);
    connect(this, &QTableView::entered, pItemDelegate, &MoDelegate::cellEntered2);

    // 关键GcpListView的双击事件.
    connect(this, &QTableView::doubleClicked, this, &MoTableWidget::doubleClicked);
    connect(this, &QTableView::doubleClicked, pItemDelegate, &MoDelegate::doubleClicked);

    connect(pItemDelegate, &MoDelegate::itemModified, this, &MoTableWidget::Slot_itemModified);

    // 设置各列尺寸.
    horizontalHeader()->setDefaultSectionSize(100);//110

    if (mode == 0)
    {
        horizontalHeader()->resizeSection(0, 10);//60
        horizontalHeader()->resizeSection(1, 10);//60
        horizontalHeader()->resizeSection(2, 165);//130
        horizontalHeader()->resizeSection(3, 55);//60
    }
    else
    {
        horizontalHeader()->resizeSection(0, 10);//60
        horizontalHeader()->resizeSection(1, 10);//60
        horizontalHeader()->resizeSection(2, 185);//measurement
        horizontalHeader()->resizeSection(3, 80);
        horizontalHeader()->resizeSection(4, 80);

        horizontalHeader()->resizeSection(5, 75);
    }
    horizontalHeader()->setStretchLastSection(true);

}

// 根据选择的坐标类别,显示不同的表头
void MoTableWidget::setHeaderLabelsMode(bool bWGS84)
{
    if (mode != 0)
        return;

    //  QStringList headerLabels;
    headerLabels.clear();

    /*QStringList slHeaderLabels;*/
    /*std::cout << column_gcp2_e::COUNT_GCPCOL << std::endl;*/
    std::vector<QString>  strs(column_gcp2_e::COUNT_GCPCOL);
    strs[0] = "";
    strs[COLOR2_COL] = "";

    if (AI3D::CORE::BlockObject::isChineseVersion())
    {
        strs[NAME2_COL] = "名称";
        strs[PHOTO2_COL] = "影像数";

        strs[CATEGORY2_COL] = "类型";
        strs[RMS_PIX2_COL] = "反投影误差[px]";
        strs[ERROR3D2_COL] = "三维误差.[m]";
        strs[ERROR3D_H2_COL] = "水平误差.[米]";
        strs[ERROR3D_V2_COL] = "高程误差.[米]";

        if (bWGS84)
        {
            strs[X2_COL] = "经度";
            strs[Y2_COL] = "纬度";

            strs[Z2_COL] = "高程";
            strs[EST_X2_COL] = "经度计算值";
            strs[EST_Y2_COL] = "纬度计算值";
            strs[EST_Z2_COL] = "高程计算值";

        }
        else
        {
            strs[X2_COL] = "X";
            strs[Y2_COL] = "Y";
            strs[Z2_COL] = "Z";

            strs[EST_X2_COL] = "计算值X";
            strs[EST_Y2_COL] = "计算值Y";
            strs[EST_Z2_COL] = "计算值Z";
        }


    }
    else
    {
        strs[NAME2_COL] = "Name";
        strs[PHOTO2_COL] = "Photos";
        strs[CATEGORY2_COL] = "Category";
        strs[RMS_PIX2_COL] = "RMS of reproj.\nerror[px]";
        strs[ERROR3D2_COL] = "3D err.[m]";
        strs[ERROR3D_H2_COL] = "3D horizonal err.[m]";
        strs[ERROR3D_V2_COL] = "3D vertical err.[m]";
        if (bWGS84)
        {
            strs[X2_COL] = "Given\nLon";
            strs[Y2_COL] = "Given\nLat";
            strs[Z2_COL] = "Given\nAlt";
            strs[EST_X2_COL] = "Estimate\nLon";
            strs[EST_Y2_COL] = "Estimate\nLat";
            strs[EST_Z2_COL] = "Estimate\nAlt";
        }
        else
        {
            strs[X2_COL] = "Given X";
            strs[Y2_COL] = "Given Y";
            strs[Z2_COL] = "Given Z";
            strs[EST_X2_COL] = "Estimate\nX";
            strs[EST_Y2_COL] = "Estimate\nY";
            strs[EST_Z2_COL] = "Estimate\nZ";
        }

    }

    for (int i = 0; i < strs.size(); i++)
    {
        headerLabels << strs[i];
    }

    horizontalHeader()->repaint();
}

#if 0
void QAbstractItemView::edit(const QModelIndex& index)[slot]
    void QAbstractItemView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)[virtual protected slot]
        void QAbstractItemView::editorDestroyed(QObject* editor)[virtual protected slot]
        QModelIndex index = ui->tableView->model()->index(0, 0, QModelIndex());
    ui->tableView->edit(index);
#endif

    MoTableWidget::~MoTableWidget()
    {

    }

    //pStandardItemModel
    // 清空GcpListView 数据.
    void MoTableWidget::clearData()
    {
        //if (!pStandardItemModel)
        //  return;

        ////setUpdatesEnabled(false);

        //int row = pStandardItemModel->rowCount();
        //for (int i = 0; i < row; i++)
        //{
        //  pStandardItemModel->removeRow(0);
        //}
        if (!pStandardItemModel)
            return;

        //setUpdatesEnabled(false);

        int row = pStandardItemModel->rowCount();

        QList <int> delteRows;
        for (int i = 0; i < pStandardItemModel->rowCount(); i++)
        {
            int iLine = pStandardItemModel->item(i, 1)->text().toInt();
            delteRows.append(i);
        }
        int deleted = 0;
        for (int i = 0; i < delteRows.count(); i++)
        {
            int rowtemp = delteRows[i];
            pStandardItemModel->removeRow(rowtemp + deleted);
            --deleted;
        }
        //setUpdatesEnabled(true);
    }

    // 最新版本未使用了.?chy
    void MoTableWidget::appendRowData(gcp_list_item_st& gcpListItem)
    {
        int row = pStandardItemModel->rowCount();

        QFont font;
        font.setFamily("Arial");
        font.setPixelSize(14);
        font.setBold(false);

        //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        ///QColor color = GetColor(gcp.color_);
        for (int i = 0; i < colCount /*pStandardItemModel->columnCount()*/; i++)
        {
            QStandardItem* itemTemp = new QStandardItem("");
            itemTemp->setFont(font);
            pStandardItemModel->setItem(row, i, itemTemp);
        }

        int state = gcpListItem.color_;

        QString iconFile;
        if (state == 2)
            iconFile = ":/new/button/skinbutton/gcppixerr3";
        else if (state == 0)
            iconFile = ":/new/button/skinbutton/gcppixerr1";
        else if (state == 1)
            iconFile = ":/new/button/skinbutton/gcppixerr2";
        else
            iconFile = "";

        pStandardItemModel->item(row, COLOR2_COL)->setData(iconFile, Qt::EditRole);
        pStandardItemModel->item(row, COLOR2_COL)->setData(QString("%1.png").arg(iconFile), Qt::DisplayRole);

        //pStandardItemModel->item(row, NAME2_COL)->setData(QVariant::fromValue<int>(it->first), CRControlpointsID);
        //pStandardItemModel->item(row, NAME2_COL)->setText(QString::fromLocal8Bit(gcp.name_.c_str()));
        pStandardItemModel->item(row, NAME2_COL)->setData(gcpListItem.ControlpointsID, 276); //CRControlpointsID;
        if (gcpListItem.bHasImageId)
        {
            pStandardItemModel->item(row, NAME2_COL)->setData(gcpListItem.ControlpointsImageID, 277);
        }

        pStandardItemModel->item(row, NAME2_COL)->setData(gcpListItem.name_, Qt::EditRole);
        pStandardItemModel->item(row, NAME2_COL)->setToolTip(gcpListItem.name_);

        pStandardItemModel->item(row, PHOTO2_COL)->setData(QString::number(gcpListItem.photos_), Qt::EditRole);
        pStandardItemModel->item(row, PHOTO2_COL)->setTextAlignment(Qt::AlignCenter);

        pStandardItemModel->item(row, CATEGORY2_COL)->setData(gcpListItem.category_, Qt::EditRole);

        pStandardItemModel->item(row, X2_COL)->setData(gcpListItem.str_given_x_, Qt::EditRole);
        pStandardItemModel->item(row, Y2_COL)->setData(gcpListItem.str_given_y_, Qt::EditRole);
        pStandardItemModel->item(row, Z2_COL)->setData(gcpListItem.str_given_z_, Qt::EditRole);

        pStandardItemModel->item(row, EST_X2_COL)->setData(gcpListItem.str_esitmated_x_, Qt::EditRole);
        pStandardItemModel->item(row, EST_Y2_COL)->setData(gcpListItem.str_esitmated_y_, Qt::EditRole);
        pStandardItemModel->item(row, EST_Z2_COL)->setData(gcpListItem.str_esitmated_z_, Qt::EditRole);

        pStandardItemModel->item(row, RMS_PIX2_COL)->setData(gcpListItem.str_rms_pix_, Qt::EditRole);
        //pStandardItemModel->item(row, RMS_DIST2_COL)->setData(gcpListItem.str_rms_dis_, Qt::EditRole);
        pStandardItemModel->item(row, ERROR3D2_COL)->setData(gcpListItem.str_error_3d_, Qt::EditRole);

        pStandardItemModel->item(row, ERROR3D_H2_COL)->setData(gcpListItem.str_error_3d_xy_, Qt::EditRole);
        pStandardItemModel->item(row, ERROR3D_V2_COL)->setData(gcpListItem.str_error_3d_z_, Qt::EditRole);
    }

    void MoTableWidget::updateRowData(int row, gcp_list_item_st& gcpListItem)
    {
        int rowCount = pStandardItemModel->rowCount();

        if (rowCount <= 0 || row < 0 || row >= rowCount)
            return;


        pStandardItemModel->item(row, X2_COL)->setData(gcpListItem.str_given_x_, Qt::EditRole);
        pStandardItemModel->item(row, Y2_COL)->setData(gcpListItem.str_given_y_, Qt::EditRole);
        pStandardItemModel->item(row, Z2_COL)->setData(gcpListItem.str_given_z_, Qt::EditRole);

        pStandardItemModel->item(row, EST_X2_COL)->setData(gcpListItem.str_esitmated_x_, Qt::EditRole);
        pStandardItemModel->item(row, EST_Y2_COL)->setData(gcpListItem.str_esitmated_y_, Qt::EditRole);
        pStandardItemModel->item(row, EST_Z2_COL)->setData(gcpListItem.str_esitmated_z_, Qt::EditRole);

        pStandardItemModel->item(row, RMS_PIX2_COL)->setData(gcpListItem.str_rms_pix_, Qt::EditRole);
        //pStandardItemModel->item(row, RMS_DIST2_COL)->setData(gcpListItem.str_rms_dis_, Qt::EditRole);
        pStandardItemModel->item(row, ERROR3D2_COL)->setData(gcpListItem.str_error_3d_, Qt::EditRole);

        pStandardItemModel->item(row, ERROR3D_H2_COL)->setData(gcpListItem.str_error_3d_xy_, Qt::EditRole);
        pStandardItemModel->item(row, ERROR3D_V2_COL)->setData(gcpListItem.str_error_3d_z_, Qt::EditRole);
    }

    //?chy: 276
    int MoTableWidget::findRowByControlId(uint64_t gcp_id)
    {
        if (!pStandardItemModel)
            return -1;

        for (int row = 0; row < pStandardItemModel->rowCount(); row++)
        {
            if (pStandardItemModel->item(row, NAME2_COL)->data(276 /*CRControlpointsID*/).toInt() == gcp_id)
                return row;
        }

        return -1;
    }

    // 移除GcpListView指定行数据.
    void MoTableWidget::removeOneRow(int row)
    {

        int rowCount = pStandardItemModel->rowCount();
        if (rowCount <= 0 || row < 0 || row >= rowCount)
        {
            return;
        }
        bool bResult = pStandardItemModel->removeRow(row);
        update();
    }

    // 通过行号选中GcpListView 指定行.
    void MoTableWidget::selectOneRow(int row)
    {
        int rowCount = pStandardItemModel->rowCount();
        if (rowCount <= 0 || row < 0 || row >= rowCount)
            return;
        selectRow(row);
    }

    // 通过GcpId选中GcpListView 指定行.
    void MoTableWidget::selectOneRowByGcpId(int currentgcp_id_)
    {
        int rowCount = pStandardItemModel->rowCount();
        if (rowCount <= 0)
            return;
        for (int i = 0; i < rowCount; i++)
        {
            QStandardItem* item = pStandardItemModel->item(i, NAME2_COL);
            if (item->data(276 /*CRControlpointsID*/).toInt() == currentgcp_id_)
            {
                selectRow(i);
                break;
            }
        }
    }

    // 通过ImageId选中指定行.
    void MoTableWidget::selectOneRowByImageId(uint32_t image_id_)
    {
        int rowCount = pStandardItemModel->rowCount();
        if (rowCount <= 0)
            return;
        for (int i = 0; i < rowCount; i++)
        {
            QStandardItem* item = pStandardItemModel->item(i, NAME2_COL);
            if (item->data(277).toUInt() == image_id_)
            {
                selectRow(i);
                break;
            }
        }
    }

    // 获取指定行的GcpId.
    uint64_t MoTableWidget::getGcpIdByRow(int row)
    {
        int rowCount = pStandardItemModel->rowCount();

        if (rowCount <= 0 || row < 0 || row >= rowCount)
            return std::numeric_limits<uint64_t>::max();;

        return pStandardItemModel->item(row, NAME2_COL)->data(276 /*CRControlpointsID*/).toInt();
    }

    // 获取指定行的ImageId.
    uint32_t MoTableWidget::getImageIdByRow(int row)
    {
        int rowCount = pStandardItemModel->rowCount();

        if (rowCount <= 0 || row < 0 || row >= rowCount)
            return std::numeric_limits<uint32_t>::max();;

        return pStandardItemModel->item(row, NAME3_COL)->data(277 /*CRControlpointsImageID*/).toUInt();
    }

    void MoTableWidget::appendRowData(gcp_measurement_list_item_st& gcpListItem)
    {
        int row = pStandardItemModel->rowCount();

        //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        ///QColor color = GetColor(gcp.color_);
        for (int i = 0; i < colCount /*pStandardItemModel->columnCount()*/; i++)
        {
            QStandardItem* itemTemp = new QStandardItem("");
            pStandardItemModel->setItem(row, i, itemTemp);
        }

        int state = gcpListItem.color_;

        QString iconFile;
        if (state == 2)
            iconFile = ":/new/button/skinbutton/gcppixerr3";
        else if (state == 0)
            iconFile = ":/new/button/skinbutton/gcppixerr1";
        else if (state == 1)
            iconFile = ":/new/button/skinbutton/gcppixerr2";
        else
            iconFile = "";

        pStandardItemModel->item(row, COLOR3_COL)->setData(iconFile, Qt::EditRole);
        pStandardItemModel->item(row, COLOR3_COL)->setData(QString("%1.png").arg(iconFile), Qt::DisplayRole);

        //pStandardItemModel->item(row, NAME2_COL)->setData(QVariant::fromValue<int>(it->first), CRControlpointsID);
        //pStandardItemModel->item(row, NAME2_COL)->setText(QString::fromLocal8Bit(gcp.name_.c_str()));
        //?chy
        pStandardItemModel->item(row, NAME3_COL)->setData(gcpListItem.ControlpointsImageID, 277); //CRControlpointsID;

        pStandardItemModel->item(row, NAME3_COL)->setData(QFileInfo(gcpListItem._photo_name).fileName(), Qt::EditRole);

        pStandardItemModel->item(row, X3_COL)->setData(gcpListItem.str_x_, Qt::EditRole);
        pStandardItemModel->item(row, Y3_COL)->setData(gcpListItem.str_y_, Qt::EditRole);

        pStandardItemModel->item(row, RMS_PIX3_COL)->setData(gcpListItem.str_rms_pix_, Qt::EditRole);
        //pStandardItemModel->item(row, RMS_DIST3_COL)->setData(gcpListItem.str_rms_dis_, Qt::EditRole);
    }

    void MoTableWidget::setGcpMeasurementListData(QList<gcp_measurement_list_item_st>& gcpMeasurementListData)
    {
        if (mode != 1)
        {

            return;
        }

        //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        this->gcpMeasurementListData = gcpMeasurementListData;

        for (int i = pStandardItemModel->rowCount() - 1; i >= 0; i--)
        {
            pStandardItemModel->removeRow(i);
        }

        int gcpMeasurementListDataCount = gcpMeasurementListData.size();

        if (gcpMeasurementListDataCount <= 0)
        {
            // prepare some test data later.
            gcpMeasurementListDataCount = 10;
        }

        QFont font;
        font.setFamily("Arial");
        font.setPixelSize(14);
        font.setBold(false);

        pStandardItemModel->setRowCount(gcpMeasurementListDataCount);

        for (int i = 0; i < gcpMeasurementListDataCount; i++)
        {
            for (int j = 0; j < colCount; j++)
            {
                QStandardItem* pStandardItem = new QStandardItem("");
                pStandardItem->setTextAlignment(Qt::AlignLeft);
                pStandardItemModel->setItem(i, j, pStandardItem);
                pStandardItemModel->item(i, j)->setFont(font);
            }
        }

        if (gcpMeasurementListData.size() <= 0)
        {
            // prepare some test data.
            for (int i = 0; i < gcpMeasurementListDataCount; i++)
            {
                for (int j = 0; j < colCount; j++)
                {
                    if (j == 1)
                    {
                        int state = qrand() % 3;
                        QString iconFile;

                        if (state == 0)
                            iconFile = ":/new/button/skinbutton/gcppixerr3";
                        else if (state == 1)
                            iconFile = ":/new/button/skinbutton/gcppixerr1";
                        else
                            iconFile = ":/new/button/skinbutton/gcppixerr2";

                        pStandardItemModel->item(i, j)->setData(iconFile, Qt::EditRole);
                        pStandardItemModel->item(i, j)->setData(QString("%1.png").arg(iconFile), Qt::DisplayRole);
                    }
                    else if (j == 2)
                    {
                        // name
                        pStandardItemModel->item(i, j)->setData(QString("TY00303937777"), Qt::EditRole);
                    }

                    else
                    {
                        double other_digit = generateRandDouble(2, 200); //110.3461664f;
                        pStandardItemModel->item(i, j)->setData(QString::number(other_digit, 'f', 7), Qt::EditRole);
                    }
                } // for (int j = 0; j < colCount; j++)
            } // for (int i = 0; i < gcpListDataCount; i++)
        }
        else
        {
            // get ready data.

        }


    }

    int MoTableWidget::getMode()
    {
        return mode;
    }

    int MoTableWidget::getColCount()
    {
        return colCount;
    }

    void MoTableWidget::doubleClicked(const QModelIndex& index)
    {

    }

    void MoTableWidget::mousePressEvent(QMouseEvent* event)
    {
        QTableView::mousePressEvent(event);
    }

    void MoTableWidget::mouseReleaseEvent(QMouseEvent* event)
    {
        QTableView::mouseReleaseEvent(event);
    }

    // 更新指定行显示.
    void MoTableWidget::updateRow(int row)
    {
        //int colCount = columnCount();
        int colCount = pStandardItemModel->columnCount();
        for (int i = 0; i < colCount; i++)
        {
            QModelIndex modelIndex = model()->index(row, i);
            update(modelIndex);
        }
    }

    //?chy bLeaved
    // bLeaved 设置为当整体离开MoTableWidget的状态标志,避免处于MoTableWidget在上下端
    // 边界行移出时,不复位刚Hover的颜色状态.
    void MoTableWidget::cellEntered2(const QModelIndex& index)
    {

        bLeaved = false;
        iHoverRow = index.row();
    }
    //?chy
    void MoTableWidget::cellEntered(int row, int col)
    {

        bLeaved = false;
        iHoverRow = row;

    }

    void MoTableWidget::hideEvent(QHideEvent* hideEvent)
    {
        // process when hidden.
    }

    //?chy
    // bLeaved 设置为当整体离开MoTableWidget的状态标志,避免处于MoTableWidget在上下端
    // 边界行移出时,不复位刚Hover的颜色状态.
    void MoTableWidget::leaveEvent(QEvent* event)
    {
        //QTableViewItem* twItem = nullptr;

        bLeaved = true;

        if (iHoverRow != -1)
        {
            updateRow(iHoverRow);
        }

        iHoverRow = -1;

    }
    //?chy
    void MoTableWidget::setRowColor(int row, QColor _color)
    {

    }

    void MoTableWidget::Slot_itemModified(int row, int col, const QString& val) const
    {
        emit itemModified(row, col, val);
    }



    // 定制MoTableWidget的每个item使用该定制组件
    MoDelegate::MoDelegate(QWidget* parent)
        : QStyledItemDelegate(parent)
    {
        pTableWidget = qobject_cast<MoTableWidget*>(parent);
        iHoverRow = -1;
    }

    void MoDelegate::doubleClicked(const QModelIndex& index)
    {

    }


    // 对MoTableWidget的Item项按不同状态及图文信息进行定制显示.
    void MoDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        int row = index.row();
        int col = index.column();
        QString txt = index.data().toString();

        //return;
        //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

        if (true) //col != 2 && col != 4)
        {
            if (iHoverRow != -1 && iHoverRow == row)
            {

            }


            if (option.state & QStyle::State_Selected)
            {
                // 对于当前选中行.
                if (!pTableWidget->bLeaved && (option.state & QStyle::State_MouseOver || index.row() == iHoverRow))
                {

                    // 当前item选中状态且Hover,并且没有光标没有离开MoTableWidget.
                    //?chy
                    painter->fillRect(option.rect, QColor(0x46, 0x64, 0x94, 0xff));
                }
                else
                {

                    // 当前item选中状态,但光标已经离开MoTableWidget或者没有Hover
                    painter->fillRect(option.rect, QColor(0x2A, 0x4D, 0x84, 0xff));
                }
                //paintBackgroundHighLighted(painter, option, index);
            }
            else if (!pTableWidget->bLeaved && (option.state & QStyle::State_MouseOver || index.row() == iHoverRow))
            {

                // 当前item没有选中,但光标没有离开MoTableWidget且处于Hover.
                painter->fillRect(option.rect, QColor(0x47, 0x47, 0x47, 0xff));


            }
            else if (index.row() % 2 == 0)
            {

                // 对于其他的偶数行,显示对应的背景色.
                //paintBackgroundBase(painter, option, index);
                painter->fillRect(option.rect, QColor(0x28, 0x28, 0x28, 0xff));
            }
            else
            {


                //paintBackgroundAlternateBase(painter, option, index);
                // 对于其他的奇数行,显示对应的背景色.
                painter->fillRect(option.rect, QColor(0x3C, 0x3C, 0x3C, 0xff));
            }
        }


        if (col == 0)
        {
            // 首列显示行号.
            painter->drawText(option.rect, Qt::AlignCenter, QString::number(row + 1));
        }
        else if (col == 1)
        {
            // 第二列显示不同的状态对应的小圆圈图标.
            QString iconFile = pTableWidget->model()->index(row, col).data(Qt::DisplayRole).toString();
            if (!iconFile.isEmpty())
            {

                QPixmap pix;
                if (!QPixmapCache::find(iconFile, &pix))
                {
                    pix = QPixmap(iconFile).scaled(22, 22);
                    QPixmapCache::insert(iconFile, pix);
                }

                int rectW = option.rect.width();
                int rectH = option.rect.height();
                int iconXoff = 0;
                int iconYoff = 0;
                if (rectW > 22)
                    iconXoff = (rectW - 22) / 2;

                if (rectH > 22)
                    iconYoff = (rectH - 22) / 2;


                //      painter->drawPixmap(iconXoff,iconYoff, iconPixmap);
                painter->drawPixmap(option.rect.x() + iconXoff, option.rect.y() + iconYoff, pix);

            }
        }
        //?chy 
        /*
        else if (col == 2 || col == 4)
        {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }*/
        else
        {
            // 前两列之外显示对应的文本(EditRole属性)
            /// painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());
            QString txt = pTableWidget->model()->index(row, col).data(Qt::EditRole).toString();
            //painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());
            QRect r(option.rect);
            r.setLeft(option.rect.x() + 6);

            //      painter->drawText(option.rect, Qt::AlignLeft|Qt::AlignVCenter, txt);
            painter->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, txt);
        }
        painter->save();

        // 绘制item的右边线.
        QPen pen(QColor("#1D1D1D"), 1, Qt::SolidLine);
        painter->setPen(pen);
        painter->drawLine(option.rect.x() + option.rect.width() - 1, option.rect.y(), option.rect.x() + option.rect.width() - 1, option.rect.y() + option.rect.height() - 1);

        painter->restore();
        //QStyledItemDelegate::paint(painter, option, index);

    }

    // 创建可编辑列
    QWidget* MoDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        int row = index.row();
        int col = index.column();

        // getMode():  1:为MeasurementView; 0:为GcpListView
        if (pTableWidget && pTableWidget->getMode() == 1)
            return nullptr;

        if (col == 0 || col == COLOR2_COL || col == PHOTO2_COL || col == NAME2_COL) // col 2 is name,not need to edit.
            return nullptr;

        if (/*col == 2 ||*/ col == X2_COL || col == Y2_COL || col == Z2_COL)
        {
            // 5/6/7列为可编辑列.
            QLineEdit* leName = new QLineEdit(parent);

            leName->setStyleSheet("selection-background-color:#3572B8;selection-color:white;color:white;");
            return leName;
        }
        else if (col == CATEGORY2_COL)
        {
            ///pStandardItemModel->item(row, CATEGORY2_COL)->setData(gcpListItem.category_, Qt::EditRole);

            ///index.model()->data(index, Qt::EditRole).toString();

            QString strCategory = index.model()->data(index, Qt::EditRole).toString();
            // 第4列为下拉可选列.
            QComboBox* cbType = new QComboBox(parent);
            cbType->setStyleSheet("QComboBox QAbstractItemView::item{ min-height:50px; height:50px; font-size:14px;}"
                ""
            );

            // note:using chinese item may affect existing logic and bring unexpected behaviour.
            if (strCategory == "User Tiepoint")
            {
                cbType->addItem(tr("User Tiepoint"));
            }
            else
            {
                cbType->addItem(tr("Control point"));
                cbType->addItem(tr("Check point"));
            }

            return cbType;
        }
        else
            return nullptr; //QStyledItemDelegate::createEditor(parent, option, index);
    }

    // MoTableWidget的Model对象数据发生变化,更新Model对应Item进行显示.
    void MoDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        //QStyledItemDelegate::setEditorData(editor, index);
        if (!editor)
            return;

        QString value = index.model()->data(index, Qt::EditRole).toString();

        if (index.column() == CATEGORY2_COL)
        {
            // 若为第4列,更改第4列下拉选项.
            // need to modify the relevant logic if using chinese item for this combobox control.
            QComboBox* spin = static_cast<QComboBox*>(editor);
            if (value == "User Tiepoint")
            {
                std::cout << "utp disabled." << std::endl;
                spin->setCurrentText(value);
                spin->setEnabled(false);
            }
            else
            {
                std::cout << "utp not disabled:" << value.toStdString() << std::endl;
                spin->setCurrentText(value);
            }
        }
        else
        {
            // 对于其他可编辑的项,更新对应item的组件显示.
            QLineEdit* le = static_cast<QLineEdit*>(editor);
            le->setText(value);
        }
    }

    // 可编辑item对应的控件修改后,更新item对应的Model内部数据.
    void MoDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        if (!editor)
            return;

        if (index.column() == CATEGORY2_COL)
        {
            // 第4列选项更改,更新对应item的Model数据.
            QComboBox* comboBox = static_cast<QComboBox*>(editor);
            QString value = comboBox->currentText();
            model->setData(index, value, Qt::EditRole);
            //emit CurrentIndexChangeSignal(index.row(), index.column(), value);
            emit itemModified(index.row(), index.column(), value);
        }
        else
        {
            // 其他列若为可编辑的控件有界面数据改变时,更新对应item的Model数据.
            QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
            QString value = lineedit->text();
            model->setData(index, value, Qt::EditRole);
            emit itemModified(index.row(), index.column(), value);
        }
    }

    // 可编辑Item重设对应组件的几何尺寸.//?chy
    void  MoDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if (index.column() == CATEGORY2_COL)
        {
            QRect r(option.rect);
            r.setLeft(r.left() + 3);
            r.setTop(r.top() + 3);
            r.setRight(r.right() - 3);
            r.setBottom(r.bottom() - 3);
            //r.setBottom(r.h)

            if (editor)
            {
                editor->setGeometry(r);
                QComboBox* cbType = static_cast<QComboBox*>(editor);
                cbType->showPopup();
            }
        }
        else
        {
            int col = index.column();

            if (col == 0 || col == COLOR2_COL || col == PHOTO2_COL)
                ;
            else if (editor)
            {
                QRect r(option.rect);

                r.setLeft(r.left() + 3);
                r.setTop(r.top() + 3);
                r.setRight(r.right() - 3);
                r.setBottom(r.bottom() - 3);

                editor->setGeometry(r);
            }
        }
    }

    // 当光标移入当前item,记录对应的Hover行号.
    void MoDelegate::cellEntered(int row, int col)
    {

        iHoverRow = row;
    }

    void MoDelegate::cellEntered2(const QModelIndex& index)
    {

        iHoverRow = index.row();
    }

    QSize MoDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        if (index.column() <= 1)
        {
            if (size.width() > 50)
                return QSize(50, size.height());
            else
                return size;
        }
        else
            return size;
    }

    // MoTableWidget表头定制组件.
    MoHeaderView::MoHeaderView(Qt::Orientation orientation, QWidget* parent)
        : QHeaderView(orientation, parent)
    {
        pTableWidget = qobject_cast<MoTableWidget*>(parent);
        setStyleSheet("padding-top:5px;padding-bottom:5px;");
        setSectionsClickable(true);
    }

    MoHeaderView::~MoHeaderView()
    {


    }

    // MoTableWidget定制表头实际绘制
    void MoHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
    {

        if (logicalIndex == COLOR2_COL)
        {
            // 表头第一列位置,绘制当前行数据的状态图(不同颜色的小圆圈)
            QString iconFile = ":/new/button/skinbutton/pixerrcol.png";;// "graynor.png";
            QPixmap pix;

            if (!QPixmapCache::find(iconFile, &pix))
            {
                pix = QPixmap(iconFile).scaled(22, 22);
                QPixmapCache::insert(iconFile, pix);
            }

            //QPixmap iconPixmap = QPixmap(iconFile).scaled(22, 22);

            int rectW = rect.width();
            int rectH = rect.height();
            int iconXoff = 0;
            int iconYoff = 0;
            if (rectW > 22)
                iconXoff = (rectW - 22) / 2;

            if (rectH > 22)
                iconYoff = (rectH - 22) / 2;

            painter->fillRect(rect, QColor(0x33, 0x33, 0x33, 0xff));
            painter->drawPixmap(rect.x() + iconXoff, rect.y() + iconYoff, pix);
        }
        else
        {
            // 非第1列的表头,直接绘制对应列的已设置的表头标题文本.

            QStringList headerLabels;

            headerLabels = pTableWidget->getheaderLabels();

            if (logicalIndex != COLOR2_COL)
            {
                QString txt = headerLabels.at(logicalIndex);

                painter->save();

                QPen pen(QColor("#A5A5A5"), 1, Qt::SolidLine);
                painter->setPen(pen);

                painter->fillRect(rect, QColor(0x33, 0x33, 0x33, 0xff));

                QRect r(rect);
                r.setLeft(r.x() + 11);
                r.setTop(r.y() + 5);

                QFont font;
                font.setFamily("Arial");
                font.setPixelSize(12);
                font.setBold(false);
                //font.setLine

                painter->setFont(font);

                painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeft, txt);

                painter->restore();
            }
            else
                QHeaderView::paintSection(painter, rect, logicalIndex);
        }

        painter->save();

        QPen pen(QColor("#1D1D1D"), 1, Qt::SolidLine);
        painter->setPen(pen);
        painter->drawLine(rect.x() + rect.width() - 1, rect.y(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);

        painter->restore();

    }

    QSize MoHeaderView::sizeHint() const
    {
        QSize size = QHeaderView::sizeHint();

        return QSize(size.width(), size.height() + 10);
    }

    // GCPPreviewListView (GCP缩略图预览列表)
    MoListWidget::MoListWidget(QWidget* parent)
        : QListView(parent)
    {
        bLeaved = true;
        setMouseTracking(true);

        widgetSet.insert(this);


        previousWidth = 0;
        previousHeight = 0;

        bGenPreviewFileCompleted = true;
        bAllowUpdate = false;

        pStandardItemModel = new QStandardItemModel(this);



        setWrapping(true);
        setFlow(QListView::LeftToRight);
        setMovement(QListWidget::Static);
        setDragEnabled(false);
        setViewMode(QListView::IconMode);
        //setIconSize(QSize(100, 252));
        setIconSize(QSize(100, 125));
        //setSpacing(17);
        setSpacing(0);
        setResizeMode(QListView::Fixed);


        setModel(pStandardItemModel);

        MoListItemDelegate* pListItemDelegate = new MoListItemDelegate(this);

        setItemDelegate(pListItemDelegate);

        connect(this, &QListView::clicked, this, &MoListWidget::funcClicked);

        /*connect(this, &QListView::entered, this, &MoListWidget::cellEntered2);*/
        connect(this, &QListView::entered, pListItemDelegate, &MoListItemDelegate::cellEntered2);

        setGcpPreviewListData(QList<gcp_preview_list_item_st>());

        setEditTriggers(QAbstractItemView::NoEditTriggers);
        /// setEditTriggers(QAbstractItemView::DoubleClicked);

            // 线程缩略图生成是否完成的状态检测定时器.
        pGenPreviewFileTimer = new QTimer(this);

        connect(pGenPreviewFileTimer, &QTimer::timeout, this, &MoListWidget::funcPreviewFileTimeout);

        // //   void previewImg(QString img, int specialX, int specialY);

    }

    MoListWidget::~MoListWidget()
    {
        widgetSet.remove(this);

        if (pGenPreviewFileTimer->isActive())
            pGenPreviewFileTimer->stop();
    }



    // 根据图形id选中某一行.
    void MoListWidget::selectOneRowByImageId(uint32_t currentimage_id_)
    {
        int rowCount = pStandardItemModel->rowCount();
        if (rowCount <= 0)
            return;

        for (int i = 0; i < rowCount; i++)
        {
            QStandardItem* item = pStandardItemModel->item(i);
            if (item && item->data(277 /*CRControlpointsImageID*/).toUInt() == currentimage_id_)
            {
                QModelIndex index = model()->index(i, 0);
                setCurrentIndex(index);
                break;
            }
        }
    }


    void MoListWidget::hideEvent(QHideEvent* event)
    {

    }

    void MoListWidget::resizeEvent(QResizeEvent* event)
    {
        if (previousWidth == 0 && previousHeight == 0)
        {

            // 10 is just for vertical scrollbar width.
            hspace = (event->size().width() - 10) % 100;
            hnum = (event->size().width() - 10) / 100;

            hspace /= 2;
            QString styleStr = "padding-left:" + QString::number(hspace) + "px;padding-top:6px;padding-bottom:9px";
            //setStyleSheet("padding-left:" + QString::number(hspace) + "px;");

            setStyleSheet(styleStr);

            previousWidth = event->size().width();
            previousHeight = event->size().height();

            arrangeGroupItems();
        }
        else if (event->size().height() >= 200 && previousWidth > 0 && previousHeight > 0)
        {
            int nw = event->size().width();
            int nh = event->size().height();

            if (abs(nw - previousWidth) > 20)
            {

                hspace = (event->size().width() - 10) % 100;
                hnum = (event->size().width() - 10) / 100;

                hspace /= 2;
                QString styleStr = "padding-left:" + QString::number(hspace) + "px;padding-top:6px;padding-bottom:9px";

                setStyleSheet(styleStr);

                previousWidth = event->size().width();
                previousHeight = event->size().height();

                arrangeGroupItems();
            }
        }

    }

    int MoListWidget::getVisibleRow()
    {
        return 2;
    }

    //?chy 逻辑，顺便说一下布局
    // 计算每一行可以显示的列数.每一项的宽度为100,高度为125.
    // 滚动条为10.因此,每列可以显示的列数为总宽度减去滚动宽度10,除以每一个预览项为100的宽度.
    int MoListWidget::getVisibleCol()
    {
        return (width() - 10) / 100;
    }

    void MoListWidget::setBlockPath(const std::string& _blockPath)
    {
        this->blockPath = _blockPath;
    }

    void MoListWidget::funcClicked(const QModelIndex& index)
    {
        //  void previewImg(QString img, int specialX, int specialY);
        QString imageFile = index.data(Qt::UserRole).toString();
        //?chy 277
        image_t img_id = index.data(277).toUInt();

        //?chy 0 0 代表什么意思
        // 0 0 该处的两个值不使用.
        emit previewImg(index, imageFile, 0, 0);
        //  }

    }


    // 最新版本未使用.//?chy

    // 最新版本未实际使用,仅测试目的.

    void MoListWidget::setGcpPreviewListData(QList<gcp_preview_list_item_st>& gcpPreviewListData)
    {
        this->gcpPreviewListData = gcpPreviewListData;

        for (int i = pStandardItemModel->rowCount() - 1; i >= 0; i--)
        {
            pStandardItemModel->removeRow(i);
        }

        QFont font;
        font.setFamily("Arial");
        font.setPixelSize(12);
        font.setBold(false);


        int gcpPreviewListDataCount = gcpPreviewListData.size();

        if (gcpPreviewListDataCount <= 0)
        {
            // prepare some test data later.
            int imgCount = 76;

            char imgFilename[100];

            for (int i = 0; i < imgCount; i++)
            {
                sprintf(imgFilename, "Y:\\76\\Photos\\100_0038_%04d.jpg", i);
                QString img(imgFilename);

                if (QFileInfo(img).exists())
                {
                    imagesList.append(img);
                }
            }

            imgCount = imagesList.size();

            for (int i = 0; i < imgCount; i++)
            {
                QStandardItem* pItem = new QStandardItem();

                QString currentImage = imagesList.at(i);
                QFileInfo fileInfo(currentImage);

                pItem->setToolTip(fileInfo.fileName());
                pItem->setText(fileInfo.baseName());

                int leftTopState = qrand() % 4; // 0/1/2/3: 0=none
                int rightTopState = qrand() % 2; // 0/1

                pItem->setData(currentImage, Qt::UserRole);
                pItem->setData(leftTopState, Qt::UserRole + 1);
                pItem->setData(rightTopState, Qt::UserRole + 2);

                pItem->setFont(font);

                pStandardItemModel->appendRow(pItem);
            }

            gcpPreviewListDataCount = imgCount;
        }
        else
        {


        }

    }

    //?chy啥也没有
    void MoListWidget::init()
    {

    }

    void MoListWidget::clearData()
    {
        //setUpdatesEnabled(false);

        for (int i = pStandardItemModel->rowCount() - 1; i >= 0; i--)
        {
            pStandardItemModel->removeRow(i);
        }

        //setUpdatesEnabled(true);
    }

    //?chy1：缩略图上绘大概位置的逻辑；
    //2：此函数的第一个输入参数：窗体具体是什么
    // _pWidget是指对应的缩略图预览组件指针,该参数传到线程中;若该组件指针已经失效,不再继续处理.若有效,则在线程中持续生成缩略图并创建对应的QPixmap对象.
    static void processImageListV2(MoListWidget* _pWidget, std::string& _blockPath, QStringList& _imageFileList)
    {
        std::string blockPath = _blockPath;
        QStringList imageFileList = _imageFileList;
        std::string genPreviewFile;
        MoListWidget* pWidget = _pWidget;

        iTotalImagetoProcess = _imageFileList.size();

        for (int i = 0; i < imageFileList.size(); i++)
        {
            QString imageFile = imageFileList.at(i);
            QTime genPreviewTime;
            genPreviewTime.start();

            // 记录当前处理到的图片数位置,会在界面上动态显示处理的百分之进度,及已经处理的图片数及总的图片数.
            iCurrentImagetoProcess = i;

            // 查看此时对应的窗口控件是否存在,若不存在,退出线程的进一步图形处理.
            if (!widgetSet.contains(pWidget))
                break;

            // 生成缩略图.

            bool bGenerateState = AI3D::CORE::Image::GenPreviewImageV2(blockPath, qstr2str(imageFile), genPreviewFile);

            if (!bGenerateState)
            {
                //LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + " failed.");
            }
            else
            {
                //LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + " succ0.");
            }

            if (!genPreviewFile.empty())
            {
                //if (QFileInfo::exists(QString::fromStdString(genPreviewFile)))
                //  LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + ",exists10.");
                //else
                //  LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + ",not exists10.");


                // 根据缩略图,创建QPixmap对象,并缩放成80/60的标准比列,并缓存.

                QPixmap pix = QPixmap(str2qstr(genPreviewFile));

                //if (!pix.isNull())
                //{
                //  LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + ",pix geometry:" + std::to_string(pix.width()) + "/" 
                //      + std::to_string(pix.height()));
                //}
                //else
                //{
                //  LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + ",pix geometry null:" + std::to_string(pix.width()) + "/"
                //      + std::to_string(pix.height()));
                //}

                if (!pix.isNull() && pix.width() != 80 && pix.height() != 60)
                {
                    pix = pix.scaled(80, 60);

                }

                if (!pix.isNull())
                {
                    //LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + " succ.");

                    //?chy 此处记录两行的逻辑是？
                    // 以图形文件名及生成缩略图作为索引,缓存QPixmap对象.

                    pixmapCaches.insert(str2qstr(genPreviewFile), pix);

                    pixmapCaches.insert(imageFile, pix);

                }
                else
                {
                    //LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + " failed3.");
                    //if (QFileInfo::exists(QString::fromStdString(genPreviewFile)))
                    //  LOGI("generate preview from " + imageFile.toStdString() + " to " +  genPreviewFile + ",exists4.");
                    //else
                    //  LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + ",not exists5.");
                }
            }
            else
            {
                //LOGI("generate preview from " + imageFile.toStdString() + " to " + genPreviewFile + " failed3.");
            }

        }


        if (widgetSet.contains(pWidget))
        {
            // 若当前GcpPreviewListView存在,设置缩略图列表已经在线程中处理完毕.对应GcpListView可以刷新显示. 
            pWidget->bGenPreviewFileCompleted = true;

        }
    }
    //?chy1：此函数没用了吧
    // 最新版本代码未使用了.
    static void processImageList(std::string& _blockPath, QStringList& _imageFileList)
    {
        std::string blockPath = _blockPath;
        QStringList imageFileList = _imageFileList;
        std::string genPreviewFile;

        for (int i = 0; i < imageFileList.size(); i++)
        {
            QString imageFile = imageFileList.at(i);
            QTime genPreviewTime;


            genPreviewTime.start();

            AI3D::CORE::Image::GenPreviewImageV2(blockPath, qstr2str(imageFile), genPreviewFile);

            if (!genPreviewFile.empty())
            {

                QPixmap pix = QPixmap(str2qstr(genPreviewFile));

                if (!pix.isNull() && pix.width() != 80 && pix.height() != 60)
                {
                    pix = pix.scaled(80, 60);
                }

                if (!pix.isNull())
                {

                    QPixmapCache::insert(str2qstr(genPreviewFile), pix);

                    QPixmapCache::insert(imageFile, pix);

                }

            }

        }
    }

    // todo:check whether a previewfile exists inside previews directory.
    // 设定GcpListView需要处理的图形列表.
    void MoListWidget::setImageFileList(QStringList& _imageFileList)
    {
        //todo check !bGenPreviewFileCompleted or bGenPreviewFileCompleted ?zk

        // 若当前组件之前已经发出过待处理图形列表,新的请求无效.
        if (!bGenPreviewFileCompleted)
            return;


        bAllowUpdate = false;

        if (pGenPreviewFileTimer->isActive())
            pGenPreviewFileTimer->stop();

        imageFileList.clear();
        previewFileList.clear();

        iPreviousGenPreviewFileNum = 0;

        if (_imageFileList.size() > 0)
        {
            bGenPreviewFileCompleted = false;

            imageFileList = _imageFileList;

            // 判断是否有必要显示"Loading... x% (x / y)" 对话框.
            if (bNeedLoadingPrompt)
            {
                std::cout << " ___NEED__ loading prompt." << std::endl;
                // 需要显示图片加载进度信息.
                OpenLoadingPrompt();
            }
            else
            {
                std::cout << " ___NEED__NOT__ loading prompt." << std::endl;
            }


            //chy 是不是只能等他运行完才能下一步操作，同时此时进度条在哪显示出来
            // 此行不阻塞,processImageListV2运行时,同步执行后面语句.
            // 如果需要进度信息(bNeedLoadingPrompt),在OpenLoadingPrompt中加载显示,此处不显示进度条,
            // 会在界面上动态显示处理的百分之进度, 及已经处理的图片数及总的图片数.//？chy哪动态
            QtConcurrent::run(processImageListV2, this, getBlockPath(), imageFileList);
            //chy
            ///bGenPreviewFileCompleted = false;

            //chy 200
            // 每200毫秒,检测processImageListV2处理的图形列表是否处理完毕.
            pGenPreviewFileTimer->start(200);
        }
    }

    void MoListWidget::startGenPreviewFileWatchTimer()
    {

        // 图形列表数据准备好,可以执行图形处理,并在图形处理完毕后开始更新界面显示.
        bAllowUpdate = true;
    }

    void MoListWidget::funcPreviewFileTimeout()
    {

        // 若没有待处理图形列表,或没有完整加载完待处理的图形列表,不需要执行图形处理完毕状态检测.
        // 实际上现在逻辑基本不需要.如果上述条件不具备,不会进入启动该定时器.
        if (imageFileList.size() <= 0 || !bAllowUpdate) // though impossible.
            return;


        // 线程中的图形列表是否处理完毕.
        if (!bGenPreviewFileCompleted)
            return;

        // 图形列表已处理完毕,关闭"Loading..."对话框.

        CloseLoadingPrompt();

        if (bGenPreviewFileCompleted) // 此行多余.
        {


            // bGenPreviewFileCompleted = false;
            // 清空待处理图形列表

            imageFileList.clear();

            // 关闭图形处理列表是否结束的状态检测定时器
            if (pGenPreviewFileTimer->isActive())
                pGenPreviewFileTimer->stop();


            // 刷新GcpListView界面显示.
            update();

            bAllowUpdate = false;
        }


    }

    // 分组显示Gcp PreviewListView.
    void MoListWidget::arrangeGroupItems()
    {
        if (!pStandardItemModel || previewListMap.size() <= 0)
            return;

        pStandardItemModel->clear();

        int mapSize = previewListMap.size();
        int rowItemNum = this->width() / 100;
        rowItemNum = hnum; // use the value after resizing event.

        QStandardItem* pItem;
        int currRow = 0;
        int currCol = 0;

        int iTotalItemNum = 0;

        // add space item to enlarge the list widget's content area dimension.
        // cooperate with ModelIndex,how to process click event.

        for (auto& mapItem : previewListMap)
        {
            int listSize = mapItem->size();

            currCol = 0;
            int iRowNumInsideCurrentGroup = 0;

            for (int i = 0; i < listSize; i++)
            {


                gcp_measurement_list_item_st previewItem = mapItem->at(i);
                pItem = appendRowData(previewItem);

                if (pItem != nullptr)
                {
                    pItem->setData(currRow, Qt::UserRole + 4);
                    pItem->setData(currCol, Qt::UserRole + 5);
                    iTotalItemNum++;
                }

                pItem->setData(previewItem.estimated_x_, Qt::UserRole + 6);
                pItem->setData(previewItem.estimated_y_, Qt::UserRole + 7);
                pItem->setData(previewItem.width, Qt::UserRole + 8);
                pItem->setData(previewItem.height, Qt::UserRole + 9);

                //std::string previewItemInfo = previewItem._photo_name.toStdString() + " " + previewItem.preview_name_.toStdString() + " ex/ey:" +
                //  std::to_string(previewItem.estimated_x_) + " " + std::to_string(previewItem.estimated_y_) + " w/h:" + std::to_string(previewItem.width) + " " 
                //  + std::to_string(previewItem.height);

                ///std::cout << previewItemInfo << std::endl;

                if (i % rowItemNum == (rowItemNum - 1))
                {
                    currRow++;
                    iRowNumInsideCurrentGroup++;
                    currCol = 0;
                }
                else
                {
                    currCol++;
                }
            }

            if (currCol != 0)
            {
                for (int i = currCol; i < rowItemNum; i++)
                {
                    pItem = new QStandardItem();

                    pItem->setText("");
                    pItem->setData(98, Qt::UserRole + 1); // spacer item,just to hold space.
                    pItem->setData(-1, Qt::UserRole + 4);
                    pItem->setData(-1, Qt::UserRole + 5);

                    pStandardItemModel->appendRow(pItem);
                    currCol++;
                }

                iRowNumInsideCurrentGroup++;
                currRow++;
            }
        }


    }

    // 一次性加载设置当前Gcp PreviewList需要显示的图形列表.
    void MoListWidget::appendData(QMap<QString, QList<gcp_measurement_list_item_st>*>& previewListMap)
    {

        this->previewListMap = previewListMap;

        arrangeGroupItems();


    }

    // 不使用了.
    QStandardItem* MoListWidget::appendRowData(gcp_measurement_list_item_st& gcpListItem)
    {
        //  int row = pStandardItemModel->rowCount();

        QStandardItem* pItem = new QStandardItem();

        /// int hspace = (event->size().width() - 10) % 100;

        if (gcpListItem.color_ == 99 || gcpListItem.color_ == 98)
        {
            pItem->setText(gcpListItem._photo_name);
            pItem->setData(gcpListItem.color_, Qt::UserRole + 1);

            //pItem->setData(rightTopState, Qt::UserRole + 2);// + 4 / +5 <===> rowIndex / colIndex

            pStandardItemModel->appendRow(pItem);

            return pItem;
        }

        QFont font;
        font.setFamily("Arial");
        font.setPixelSize(12);
        font.setBold(false);

        QString currentImage = gcpListItem._photo_name;
        QFileInfo fileInfo(currentImage);

        //pItem->setToolTip(fileInfo.fileName());
        pItem->setText(fileInfo.baseName());

        int leftTopState = gcpListItem.color_; //qrand() % 4; // 0/1/2/3: 0=none
        int rightTopState = 0; //qrand() % 2; // 0/1

        if (gcpListItem.check_)
            rightTopState = 1;

        ///pItem->setToolTip(currentImage);

        pItem->setData(currentImage, Qt::UserRole);
        pItem->setData(gcpListItem.ControlpointsImageID, 277);

        pItem->setData(leftTopState, Qt::UserRole + 1);
        pItem->setData(rightTopState, Qt::UserRole + 2);
        pItem->setData(gcpListItem.preview_name_, Qt::UserRole + 3);

        pItem->setData(-1, Qt::UserRole + 4);
        pItem->setData(-1, Qt::UserRole + 5);

        pItem->setFont(font);

        pStandardItemModel->appendRow(pItem);



        return pItem;
    }

    MoListItemDelegate::MoListItemDelegate(QWidget* parent)
        : QStyledItemDelegate(parent)
    {
        pListWidget = qobject_cast<MoListWidget*>(parent);
        iHoverRow = -1;
    }

    void MoListItemDelegate::doubleClicked(const QModelIndex& index)
    {

    }

    //QFontMetrics metrics = painter->fontMetrics();
    QString getLimitedString(QFontMetrics& metrics, QString& str, int maxWidth)
    {
        if (str.length() <= 6)
            return str;

        int needWidth = metrics.width(str);
        if (needWidth <= maxWidth)
            return str;

        QString rightStr = "..." + str.right(6);

        needWidth = metrics.width(rightStr);
        if (needWidth > maxWidth)
            return str.right(6);
        else if (needWidth == maxWidth)
            return rightStr;

        int leftWidth = maxWidth - needWidth;
        QString leftStr = str.left(str.length() - 6);

        QString leftDisplayStr;
        int leftDisplayWidth = 0;

        for (int i = 0; i < leftStr.length(); i++)
        {
            needWidth = metrics.width(leftStr.left(i + 1));
            if (needWidth <= leftWidth)
            {
                leftDisplayStr = leftStr.left(i + 1);
            }
            else
                break;
        }

        return leftDisplayStr + rightStr;
    }

    QPainterPath preview_paintCross(int x, int y, int w, int h, int step)
    {
        QPainterPath path;


        path.addRect(x - w - step, y - h / 2, w, h);
        path.addRect(x - h / 2, y + step + h / 2, h, w - h / 2);
        path.addRect(x + step, y - h / 2, w, h);
        path.addRect(x - h / 2, y - w - step, h, w - h / 2);

        return path;
    }

    void preview_paintCircleCross(QPainter* painter, int x, int y, int width, int height)
    {
        //QPainter painter(this);
        //painter.save();
        ////设置反锯齿
        int Alpha_ = 255; // set default value.

        painter->save();

        QColor color = QColor(Qt::green);
        color.setAlpha(Alpha_); // 255
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::Qt4CompatiblePainting);
        //painter->setCompositionMode(QPainter::CompositionMode_Difference);

        painter->setPen(QPen(color, 2));
        QRect drawRect(x, y, width, height);


        painter->drawEllipse(drawRect);


        int w = width;
        int h = height;

        //painter->drawLine()

        painter->drawLine(QPointF(x + w / 2 - w / 4, y + h / 2), QPointF(x + w / 2 + w / 4, y + h / 2));
        painter->drawLine(QPointF(x + w / 2, y + h / 2 - h / 4), QPointF(x + w / 2, y + h / 2 + h / 4));

        painter->restore();
    }

    // qlistview中画按钮 https://www.cnblogs.com/liuruoqian/p/5503489.html
    void MoListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {


        // left-top corner:28x28,display based on state.
        // right-top check:32x32,display based on other state.
        ///return;

        if (qApp->hasPendingEvents())
        {

        }

        QTime paintTime;
        paintTime.start();

        int row = index.row();
        int col = index.column();


        painter->save();

        ///QString imageTitle = "CCTTTXXX" + index.data().toString();
        QString imageTitle = index.data().toString();
        QString imageFile = index.data(Qt::UserRole).toString();

        int leftTopState = index.data(Qt::UserRole + 1).toInt();
        int rightTopState = index.data(Qt::UserRole + 2).toInt();
        QString previewFile = index.data(Qt::UserRole + 3).toString();

        int currRow = index.data(Qt::UserRole + 4).toInt();
        int currCol = index.data(Qt::UserRole + 5).toInt();
        double estimated_x = index.data(Qt::UserRole + 6).toDouble();
        double estimated_y = index.data(Qt::UserRole + 7).toDouble();
        int imageWidth = index.data(Qt::UserRole + 8).toInt();
        int imageHeight = index.data(Qt::UserRole + 9).toInt();

        int estimated_ix = -1;
        int estimated_iy = -1;

        if (estimated_x != -DBL_MAX)
        {
            estimated_ix = (int)estimated_x;
        }

        if (estimated_y != -DBL_MAX)
        {
            estimated_iy = (int)estimated_y;
        }

        if (leftTopState == 98)
        {
            painter->restore();
            return;
        }

        // append space item while resize?
        // 1)draw photogroup box/text 2)wrap line for the last item,place photogroup at the beginning of the specified group.

        if (leftTopState == 99)
        {
            //use imageTitle and Photo group
            // use new layout row.
            // record last item's position.
            QString groupFull = imageTitle;
            QString groupBase;
            QString groupDetail;

            int pixX = (100 - 80) / 2;
            int pixY = 30 - 4;

            //QRect rect = QRect(pixX - 1 + option.rect.x(), pixY - 1 + option.rect.y(), 82, 62);
            QRect rect = QRect(pixX - 1 /* + option.rect.x()*/, pixY - 1 + option.rect.y(), 82, 62);

            QPen oldPen = painter->pen();

            QPen newPen = QPen(QColor(0xff, 0xff, 0xff), 1);

            painter->setPen(newPen);

            ///painter->drawRoundedRect(rect, 4.0, 4.0);
            painter->drawRect(rect);

            painter->setPen(oldPen);

            if (groupFull.contains("PhotoGroup", Qt::CaseInsensitive))
            {
                bool bSplitGroupOk = false;
                if (groupFull.contains("(") && groupFull.contains(")"))
                {
                    int groupFirstOpenBracketPos = groupFull.indexOf("(");
                    int groupLastCloseBracketPos = groupFull.lastIndexOf(")");
                    if (groupFirstOpenBracketPos >= 0 && groupLastCloseBracketPos >= 0 && groupFirstOpenBracketPos < groupLastCloseBracketPos)
                    {
                        bSplitGroupOk = true;

                        groupBase = "Photogroup";
                        groupDetail = groupFull.mid(groupFirstOpenBracketPos + 1, groupLastCloseBracketPos - groupFirstOpenBracketPos + 1 - 2);
                    }
                }
                else if (groupFull.contains("-"))
                {
                    int groupPos = groupFull.indexOf("-");
                    if (groupPos >= 0)
                    {
                        bSplitGroupOk = true;
                        groupBase = "Photogroup";
                        groupDetail = groupFull.mid(groupPos + 1);
                    }
                }
                else if (groupFull.contains("_"))
                {
                    int groupPos = groupFull.indexOf("_");
                    if (groupPos >= 0)
                    {
                        bSplitGroupOk = true;
                        groupBase = "Photogroup";
                        groupDetail = groupFull.mid(groupPos + 1);
                    }
                }

                if (!bSplitGroupOk)
                {
                    groupBase = "Photogroup";
                    int groupDetailPos = groupFull.indexOf(groupBase, 0, Qt::CaseInsensitive);
                    groupDetail = groupFull.mid(groupDetailPos + groupBase.length());
                }
            }
            else
            {
                // no common prefix for photo group name.
                groupBase = "Photogroup";
                groupDetail = groupFull;
            }


            int txtXOff = 4;
            int txtWidth = option.rect.width() - 2 * txtXOff;


            QRect r(rect.x(), rect.y() + 9 + 3, rect.width(), 14);
            QRect r2(rect.x(), rect.y() + 26 + 6, rect.width(), 16);


            QFont font;
            font.setFamily("Arial");
            font.setPixelSize(12);
            font.setBold(false);

            painter->setFont(font);



            QPen pen(QColor("#FFFFFF"), 1, Qt::SolidLine);
            painter->setPen(pen);

            painter->drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, groupBase);


            font.setPixelSize(14);
            font.setBold(true);
            painter->setFont(font);

            painter->drawText(r2, Qt::AlignHCenter | Qt::AlignVCenter, groupDetail);


            painter->restore();
            return;
        }




        QPixmap pix;



        //int pixX = (100 - 86) / 2;
        int pixX = (100 - 80) / 2;

        int pixY = 30 - 4;


        //QPixmap pix = pixmapCaches.find(imageFile);
        pix = pixmapCaches.value(imageFile);
        //std::cout << __FUNCTION__ << " " << __LINE__ << " " << imageFile.toStdString() << " " << previewFile.toStdString() << std::endl;

        if (option.state & QStyle::State_Selected)
        {
            QRect rect = QRect(option.rect.x(), option.rect.y(), 100, 100);
            painter->fillRect(rect, QColor(255, 255, 255, 51));
        }

        QRect rect = QRect(pixX - 1 + option.rect.x(), pixY - 1 + option.rect.y(), 82, 62);

        if (!pix.isNull())
        {
            //std::cout << __FUNCTION__ << " " << __LINE__ << " " << imageFile.toStdString() << std::endl;


            if (pix.width() == 80 && pix.height() == 60)
            {
                //  std::cout << __FUNCTION__ << " " << __LINE__ << " " << imageFile.toStdString() << std::endl;
                    //if (currRow >= 0 && currCol >= 0)
                    //{
                    //  painter->drawPixmap(pixX + currCol * 100, pixY + currRow * 100, pix);
                    //}
                    //else
                painter->drawPixmap(option.rect.x() + pixX, option.rect.y() + pixY, pix);


                //QString iconFile = "ppicon.png";
                QString iconFile = ":/new/prefix1/skin/ppicon.png";
                //QPixmap pix = QPixmap(iconFile).scaled(16, 16);
                QPixmap ppPix;
                if (!QPixmapCache::find(iconFile, &ppPix))
                {
                    //                  pix = QPixmap(iconFile).scaled(16, 16);
                    ppPix = QPixmap(iconFile);
                    QPixmapCache::insert(iconFile, ppPix);
                }


                //painter->drawPixmap(rect.x() + rect.width() - 16 - 1, rect.y() - 21 + 1, pix);

                painter->setClipRect(rect, Qt::IntersectClip);
                //缩略图绘制预测点位置
                if (estimated_ix >= 0 && estimated_iy >= 0 && imageWidth > 0 && imageHeight > 0 &&
                    estimated_ix <= imageWidth && estimated_iy <= imageHeight)
                {
                    int preview_estimated_ix = estimated_ix * 1.0 * 80 / imageWidth;
                    int preview_estimated_iy = estimated_iy * 1.0 * 60 / imageHeight;

                    painter->drawPixmap(option.rect.x() + pixX + preview_estimated_ix - 8.0, option.rect.y() + pixY + preview_estimated_iy - 8.0, ppPix);
                    //  preview_paintCircleCross(painter, option.rect.x() + pixX + preview_estimated_ix - 8.0, option.rect.y() + pixY + preview_estimated_iy - 8.0,16,16);
                }

                painter->setClipRect(rect, Qt::NoClip);
            }
            else
            {
                //std::cout << __FUNCTION__ << " " << __LINE__ << " " << imageFile.toStdString() << " " << previewFile.toStdString() << std::endl;

                int pixYoff = 0;
                if (pix.height() < 60)
                    pixYoff = (60 - pix.height()) / 2;
                int pixX2 = (100 - pix.width()) / 2;
                int pixY2 = 30 - 4 + pixYoff;
                painter->drawPixmap(option.rect.x() + pixX2, option.rect.y() + pixY2, pix);

                //QString iconFile = "ppicon.png";
                QString iconFile = ":/new/prefix1/skin/ppicon.png";
                //QPixmap pix = QPixmap(iconFile).scaled(16, 16);
                QPixmap ppPix;
                if (!QPixmapCache::find(iconFile, &ppPix))
                {
                    //                  pix = QPixmap(iconFile).scaled(16, 16);
                    ppPix = QPixmap(iconFile);
                    QPixmapCache::insert(iconFile, ppPix);
                }

                painter->setClipRect(rect, Qt::IntersectClip);

                if (estimated_ix >= 0 && estimated_iy >= 0 && imageWidth > 0 && imageHeight > 0 &&
                    estimated_ix <= imageWidth && estimated_iy <= imageHeight)
                {
                    int preview_estimated_ix = estimated_ix * 1.0 * pix.width() / imageWidth;
                    int preview_estimated_iy = estimated_iy * 1.0 * pix.height() / imageHeight;

                    //preview_paintCircleCross(painter, option.rect.x() + pixX2 + preview_estimated_ix - 8.0, option.rect.y() + pixY2 + preview_estimated_iy - 8.0, 16, 16);

                    painter->drawPixmap(option.rect.x() + pixX2 + preview_estimated_ix - 8.0, option.rect.y() + pixY2 + preview_estimated_iy - 8.0, ppPix);
                }

                painter->setClipRect(rect, Qt::NoClip);
            }
        }
        else
        {
            //std::cout << __FUNCTION__ << " " << __LINE__ << " " << imageFile.toStdString() << " " << previewFile.toStdString() << std::endl;

            QString noFile = ":/new/prefix1/skin/nophotos.png";

            if (!QPixmapCache::find(noFile, &pix))
            {
                pix = QPixmap(noFile);


            }

            if (!pix.isNull() && pix.width() > 0 && pix.height() > 0)
            {
                QPixmapCache::insert(noFile, pix);
                // todo:center nophoto.

                int pixYoff = 0;
                if (pix.height() < 60)
                    pixYoff = (60 - pix.height()) / 2;
                int pixX2 = (100 - pix.width()) / 2;
                int pixY2 = 30 - 4 + pixYoff;
                painter->drawPixmap(option.rect.x() + pixX2, option.rect.y() + pixY2, pix);

                ///         painter->drawPixmap(option.rect.x() + pixX, option.rect.y() + pixY, pix);
            }

        }



        QPen oldPen = painter->pen();

        QPen newPen = QPen(QColor(0xff, 0xff, 0xff), 1);

        painter->setPen(newPen);

        ///painter->drawRoundedRect(rect, 4.0, 4.0);
        painter->drawRect(rect);

        painter->setPen(oldPen);

        if (leftTopState != 3)
        {
            // state == 1
            QString iconFile = ":/new/prefix1/skinbutton/rcorner.png";
            QBrush tbrush = QBrush(QColor(241, 86, 86));

            if (leftTopState == 0)
            {
                iconFile = ":/new/prefix1/skinbutton/gcorner.png";
                tbrush = QBrush(QColor(100, 226, 0));
            }
            else if (leftTopState == 1)
            {
                iconFile = ":/new/prefix1/skinbutton/ycorner.png";
                tbrush = QBrush(QColor(255, 187, 7));
            }

            QRectF trect = QRectF(rect.x(), rect.y(), 14, 14);

            QPainterPath path;

            path.moveTo(trect.topLeft());
            path.lineTo(trect.topRight());
            path.lineTo(trect.bottomLeft());
            path.lineTo(trect.topLeft());



            painter->fillPath(path, tbrush);


        }

        if (rightTopState != 0)
        {
            QString iconFile = ":/new/prefix1/skinbutton/yescorner.png";
            //QPixmap pix = QPixmap(iconFile).scaled(16, 16);

            if (!QPixmapCache::find(iconFile, &pix))
            {
                pix = QPixmap(iconFile).scaled(16, 16);
                QPixmapCache::insert(iconFile, pix);
            }

            painter->drawPixmap(rect.x() + rect.width() - 16 - 1, rect.y() - 21 + 1, pix);
        }



        int txtXOff = 4;
        int txtWidth = option.rect.width() - 2 * txtXOff;
        QRect r(option.rect.x() + txtXOff, option.rect.y() + 82 + 29, txtWidth, 14);

        QFont font = painter->font();
        QFontMetrics metrics = painter->fontMetrics();

        imageTitle = getLimitedString(metrics, imageTitle, txtWidth);

        QPen pen(QColor("#FFFFFF"), 1, Qt::SolidLine);
        painter->setPen(pen);
        painter->drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, imageTitle);


        //std::cout << __FUNCTION__ << " " << __LINE__ << " " << imageFile.toStdString() << " " << previewFile.toStdString() << " " << imageTitle.toStdString() << std::endl;

        painter->restore();


    }

    //?chy
    void MoListItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {

    }

    void MoListItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {


    }

    void  MoListItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        if (editor)
        {
            std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << option.rect.x() << " " << option.rect.y() << std::endl;
            editor->setGeometry(option.rect);
        }


    }

    QSize MoListItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        return QSize(100, 125);
    }

    void MoListItemDelegate::cellEntered(int row, int col)
    {

        iHoverRow = row;
    }

    void MoListItemDelegate::cellEntered2(const QModelIndex& index)
    {
        QString imageFile = index.data(Qt::UserRole).toString();


        iHoverRow = index.row();

        QToolTip::showText(QCursor::pos(), imageFile);
    }

    //uint64_t MoListWidget::getImageIdByRow(int row)
    //{
    //  /*int rowCount = pStandardItemModel->rowCount();
    //
    //  if (rowCount <= 0 || row < 0 || row >= rowCount)
    //      return std::numeric_limits<uint64_t>::max();;
    //
    //  return pStandardItemModel->item(row, NAME3_COL)->data(CRControlpointsImageID).toInt();*/
    //}