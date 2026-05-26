#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Gui/EngineNodeView.h"
#include "Core/BlockObject.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QDateTime>
#include <sstream>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPixmapCache>


#include <QToolTip>
#include <QApplication>
#include <QBitmap>
#include <QScrollBar>
#include <QtConcurrent>

#include <QSet>
#include <QHash>
#include <QCryptographicHash>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOption>



EngineNodeView::EngineNodeView(QWidget* parent)
	: QWidget(parent)
{
	//setStyleSheet("background-color:#303030;margin:0px;border:none;padding;0px;");
	setWindowFlags(Qt::FramelessWindowHint);

	
	QVBoxLayout* vlayoutTop = new QVBoxLayout();
	vlayoutTop->setMargin(0);


	QFrame* topFrame = new QFrame(this);

	//topFrame->setStyleSheet("background-color:#303030;margin:10px 13px 10px 0px;color:#CBCBCB;");
	topFrame->setStyleSheet("background-color:#303030;margin:0px;");

	vlayout = new QVBoxLayout();
	vlayout->setMargin(0);

	QFrame* frameTop = new QFrame(this);
	frameTop->setStyleSheet("background-color:#303030;");

	//margin:17px 23px;border - radius:4px;

	hlayout0 = new QHBoxLayout(frameTop);
	hlayout0->setContentsMargins(0, 0, 0, 0);

	QFont font;
	font.setPixelSize(14);
	font.setFamily("Arial");

	lblEngineNodeInfo = new QLabel(this);
	if (AI3D::CORE::BlockObject::isChineseVersion())
	{
		lblEngineNodeInfo->setText(str2qstr(std::string("引擎节点信息")));
	}
	else
	{
		lblEngineNodeInfo->setText("Engine node info");
	}
	lblEngineNodeInfo->setFont(font);
	lblEngineNodeInfo->setStyleSheet("color:white;margin:17px 20px;");
	//lblEngineNodeInfo->setVisible(false);

	hlayout0->addWidget(lblEngineNodeInfo, 0, Qt::AlignLeft);
	//总内存，状态 job目录
	QStringList slHeaderLabels;
	//slHeaderLabels << "Hostname" << "IP address" << "Project name";
	std::vector<QString>  strs(EngineInfoCol_e::COUNT );
	 
	if (AI3D::CORE::BlockObject::isChineseVersion())
	{
		strs[HOSTNAME_COL] = str2qstr(std::string("主机名"));
		strs[IP_COL] = str2qstr(std::string("IP地址"));
		strs[FREEMEM_COL] = str2qstr(std::string("空闲内存"));
		strs[TOTALMEM_COL] = str2qstr(std::string("总内存"));
		strs[ENGINESTATUS_COL] = str2qstr(std::string("引擎状态"));
		strs[PROJECTNAME_COL] = str2qstr(std::string("工程名"));
		strs[JOBPATH_COL] = str2qstr(std::string("工作路径"));
	}
	else
	{
		strs[HOSTNAME_COL] = "Host Name";
		strs[IP_COL] = "IP Address";
		strs[FREEMEM_COL] = "Free Memory";
		strs[TOTALMEM_COL] = "Total Memory";
		strs[ENGINESTATUS_COL] = "Engine Status";
		strs[PROJECTNAME_COL] = "Project Name";
		strs[JOBPATH_COL] = "Job Path";
	}

	for (int i = 0; i < strs.size(); i++)
	{
		slHeaderLabels << strs[i];
	}
	//slHeaderLabels << str0 << str1 << str2 << str3 << str4 << str5 << str6;
	twEngineNodeList = new QTableWidget(this);

	twEngineNodeList->setColumnCount(slHeaderLabels.count());//3
	

	

	twEngineNodeList->setHorizontalHeaderLabels(slHeaderLabels);
	twEngineNodeList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//ResizeToContents
	twEngineNodeList->horizontalHeader()->resizeSection(0, 210);
	twEngineNodeList->horizontalHeader()->resizeSection(1, 250);
	twEngineNodeList->horizontalHeader()->setStretchLastSection(true);
	twEngineNodeList->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	twEngineNodeList->horizontalHeader()->setSectionsClickable(false);
	twEngineNodeList->horizontalHeader()->setSortIndicatorShown(false);

	twEngineNodeList->verticalHeader()->hide();
	twEngineNodeList->setSelectionMode(QAbstractItemView::NoSelection);
	twEngineNodeList->setSelectionBehavior(QAbstractItemView::SelectRows);
	twEngineNodeList->setAlternatingRowColors(true);//双色显示
	twEngineNodeList->setSortingEnabled(false);
	twEngineNodeList->setShowGrid(true);
	twEngineNodeList->setFocusPolicy(Qt::NoFocus);

	twEngineNodeList->setFont(font);

	//alternate-background 切换行的颜色
	twEngineNodeList->setStyleSheet("QTableWidget{ alternate-background-color: #282828; background-color: #2B2B2B;color: white; margin:14px 20px 0px 20px;padding:0px;border:1px solid #1D1D1D;}"	
		"QHeaderView::section{ background-color:#333333;color: #FFFFFF; }"
		"QTableWidget::item{border-right:1px solid #1D1D1D;}"
	);

	twEngineNodeList->horizontalScrollBar()->setStyleSheet("QScrollBar{height:10px;}");
	twEngineNodeList->verticalScrollBar()->setStyleSheet("QScrollBar{width: 10px;}");



	hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 20, 20, 20);

	font.setPixelSize(12);

	butOK = new QPushButton(this);
	butClose = new QPushButton(this);

	if (AI3D::CORE::BlockObject::isChineseVersion())
	{
		butOK->setText(str2qstr(std::string("确定")));
		butClose->setText(str2qstr(std::string("关闭")));
	}
	else
	{
		butOK->setText("OK");
		butClose->setText("Close");
	}

	butOK->setFont(font);
	butClose->setFont(font);

	butOK->setStyleSheet("width:140px;height:28px;border-radius:6px;background-color:#538CCF;color:white;margin:0px 20px 0px 0px;");
	butClose->setStyleSheet("width:140px;height:28px;border-radius:6px;background-color:#545454;color:white;");

	QSpacerItem* spacerItem = new QSpacerItem(40, 28, QSizePolicy::Expanding, QSizePolicy::Fixed);
	hlayout->addSpacerItem(spacerItem);
	hlayout->addWidget(butOK, 0, Qt::AlignRight);
	hlayout->addWidget(butClose, 0, Qt::AlignRight);

	butOK->hide();

	vlayout->addWidget(frameTop);
	vlayout->addWidget(twEngineNodeList, 1);
	vlayout->addLayout(hlayout);

	topFrame->setLayout(vlayout);

	vlayoutTop->addWidget(topFrame);
	setLayout(vlayoutTop);

	connect(butOK, &QPushButton::clicked, this, &EngineNodeView::funcOK);
	connect(butClose, &QPushButton::clicked, this, &EngineNodeView::funcClose);

	pUpdateTimer = new QTimer();
	pGetEngineNodeInfoTimer = new QTimer();

	connect(pUpdateTimer, &QTimer::timeout, this, &EngineNodeView::funcUpdateTimeout);
	


	pUpdateTimer->start(5);
	/*pGetEngineNodeInfoTimer->start(200);*/
}

EngineNodeView::~EngineNodeView()
{
	
}

void EngineNodeView::resizeEvent(QResizeEvent* event)
{
	if (!event)
		return;

	QSize nsize(event->size().width(), event->size().height());

	QBitmap bmp(nsize);
	bmp.fill();
	QPainter painter(&bmp);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::black);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawRoundedRect(bmp.rect(), 6, 6);
	setMask(bmp);
}

void EngineNodeView::funcOK()
{
	// need to receive destroyed signal to process extra action?
	close();
}

void EngineNodeView::funcClose()
{
	close();
}

void EngineNodeView::closeEvent(QCloseEvent* evt)
{
///	emit SettingsClosed();
	
	if (pGetEngineNodeInfoTimer->isActive())
		pGetEngineNodeInfoTimer->stop();

	if (pUpdateTimer->isActive())
		pUpdateTimer->stop();

	evt->accept();


}





void EngineNodeView::funcUpdateTimeout()
{
	
	
	QMap<QString, EngineInfo_s> savedRunningEngineNodeMap;
	std::map<std::string, EngineInfo_s> info;
	std::map<job_status_e, std::vector<std::string>> joblistsmap;
	if (!JobMonitor::GetJobListsInfo(info, joblistsmap))
	{
		return;
	}
	for (auto iter : info)
	{
		savedRunningEngineNodeMap.insert(QString::fromStdString(iter.first),iter.second);
	}
	twEngineNodeList->setUpdatesEnabled(false);

	int rowCount = twEngineNodeList->rowCount();
	for (int i = rowCount - 1; i >= 0; i--)
	{
		twEngineNodeList->removeRow(i);
	}

	rowCount = savedRunningEngineNodeMap.size();

	int index = 0;
	twEngineNodeList->setRowCount(rowCount);

	QMapIterator<QString, EngineInfo_s> iterator(savedRunningEngineNodeMap);
	while (iterator.hasNext())
	{
		iterator.next();

		QString hostName = iterator.key();
		EngineInfo_s runningEngineNode = iterator.value();

		QString ipAddr = QString::fromStdString(runningEngineNode.IPAddr);


		QString projectName = str2qstr(runningEngineNode.ProjectName2);


		QString freemem = QString::fromStdString(std::to_string(runningEngineNode.FreeMem));
		QString totalmem = QString::fromStdString(std::to_string(runningEngineNode.TotalMem));
		int status = runningEngineNode.Status;
		std::string strstatus = "";
		if (status == 0)
		{
			if(AI3D::CORE::BlockObject::isChineseVersion())
				strstatus = "就绪";
			else
				strstatus = "Ready";
			
		}
		if (status == 1)
		{
			if(AI3D::CORE::BlockObject::isChineseVersion())
				strstatus = "繁忙";
			else
				strstatus = "Busy";
		}
		QString stauts = QString::fromStdString(strstatus);

		QTableWidgetItem* pItemhostName = new QTableWidgetItem(hostName);
		pItemhostName->setToolTip(hostName);
		pItemhostName->setTextAlignment(Qt::AlignCenter);
		QTableWidgetItem* pItemipAddr = new QTableWidgetItem(ipAddr);
		pItemipAddr->setToolTip(ipAddr);
		pItemipAddr->setTextAlignment(Qt::AlignCenter);
		QTableWidgetItem* pItemprojectName = new QTableWidgetItem(projectName);
		pItemprojectName->setToolTip(projectName);
		pItemprojectName->setTextAlignment(Qt::AlignCenter);

		QTableWidgetItem* pItemtotalmem = new QTableWidgetItem(totalmem);
		pItemtotalmem->setTextAlignment(Qt::AlignCenter);
		QTableWidgetItem* pItemstauts = new QTableWidgetItem(stauts);
		pItemstauts->setTextAlignment(Qt::AlignCenter);
		QTableWidgetItem* pItemjobpath = new QTableWidgetItem(Settings::getMasterJobQueue());

		pItemjobpath->setToolTip(Settings::getMasterJobQueue());
		pItemjobpath->setTextAlignment(Qt::AlignCenter);
		QTableWidgetItem* pItemfreemem = new QTableWidgetItem(freemem);
		pItemfreemem->setTextAlignment(Qt::AlignCenter);

		twEngineNodeList->setItem(index, HOSTNAME_COL, pItemhostName);
		twEngineNodeList->setItem(index, IP_COL, pItemipAddr);		
		twEngineNodeList->setItem(index, TOTALMEM_COL, pItemtotalmem);
		twEngineNodeList->setItem(index, ENGINESTATUS_COL, pItemstauts);
		twEngineNodeList->setItem(index, PROJECTNAME_COL, pItemprojectName);
		twEngineNodeList->setItem(index, JOBPATH_COL, pItemjobpath);
		twEngineNodeList->setItem(index, FREEMEM_COL, pItemfreemem);
		

		index++;
	}

	twEngineNodeList->setUpdatesEnabled(true);

	
}

void OpenEngineNodeView()
{
	EngineNodeView* pEngineNodeView = new EngineNodeView(nullptr);

	pEngineNodeView->setWindowModality(Qt::ApplicationModal);
	pEngineNodeView->setAttribute(Qt::WA_DeleteOnClose);

	pEngineNodeView->resize(760, 410);
	pEngineNodeView->show();
}

