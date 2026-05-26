#include<QtGui>
#include<QFileDialog>
#include "MainWindow.h"

#include "Windows.h"
#include <QtCore/QFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osg/Switch>
#include <QTextStream>
#include <stdio.h>
#include <qstring.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <osg/DrawPixels>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osg/Node>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgUtil/Optimizer>
//end read image
//显示汉字
#include <osg/Depth>
#include <osgText/Text>
#include <osg/Camera>
#include <sstream>

#include"qprocess.h"

#include "Core/Application.h"
#include "Core/File.h"
#include "OSGEditor/OsgEngine.h"
void MWindow::on_actionLoadFile_triggered()
{
	std::string fileName = "D:/TestData/model20221205/fengtaikejiyuan/das/Production_1(2)/OSGB/Data/Tile_+006_+012/Tile_+006_+012_L20_0uuuu41.osgb";
	
	OsgEngine* pOsgEngine = OsgEngine::getInstance();

	pOsgEngine->initViewer();
	auto loadedModel = pOsgEngine->LoadOsgModel(fileName);
	osgUtil::Optimizer optimizer;
	optimizer.optimize(loadedModel.get());
	viewerWindow->updateTraversal();

	
	viewerWindow->setSceneData(loadedModel.get());

}


MWindow::MWindow(QWidget *parent, Qt::WindowFlags flags)
	:QMainWindow(parent, flags)
{
	ui.setupUi(this);
	//projectUI = new QProjectUI();
	meshfiles_.clear();
	
	mainLayout = new QGridLayout;
	viewerWindow = new ViewerQT;
	osg::Camera* camera = viewerWindow->getCamera();//获得渲染器中的相机
	camera->setClearColor(osg::Vec4(128.0 / 255.0, 128.0 / 255.0, 128.0 / 255.0, 0.8));//设置清除缓存区背景的颜色。RGBA格式。
	
	ui.actionMerge->setIcon(QIcon(":/File/Resource/BtnFuse.png"));
	
	QPalette p;
	p.setColor(QPalette::Background,QColor(0,0,0));
	viewerWindow->setCameraManipulator(new osgGA::DriveManipulator);//  osgGA::TrackballManipulator
	
	viewerWindow->setPalette(p);
	mainLayout->addWidget(viewerWindow,0,0);
	
	mainLayout->setContentsMargins(0,0,0,0);
	ui.centralwidget->setLayout(mainLayout);

	//窗口大小变化事件
	viewerWindow->addEventHandler( new osgGA::StateSetManipulator(viewerWindow->getCamera()->getOrCreateStateSet()) );
	viewerWindow->addEventHandler(new osgViewer::WindowSizeHandler);
	viewerWindow->addEventHandler(new osgViewer::StatsHandler);

	
	
	//添加操作器
	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
	
	keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator());
	
	keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
	keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
	keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
	viewerWindow->setCameraManipulator( keyswitchManipulator.get() );


	//添加路径记录
	viewerWindow->addEventHandler(new osgViewer::RecordCameraPathHandler);

	ui.statusBar->showMessage(QString::fromLocal8Bit("Ready"));


	progressBar = new QProgressBar(this);
	
	progressBar->setWindowFlags(Qt::Dialog);
	progressBar->setWindowModality(Qt::WindowModal);
	
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setOrientation(Qt::Horizontal);
	progressBar->setVisible(true);
	progressBar->setFixedSize(500, 100);
	progressBar->setAlignment(Qt::AlignCenter);
	progressBar->hide();

	isFirst = true;
	isSemiautoFirst = true;

    setCentralWidget(viewerWindow);

	dock = NULL;
	blockNumber = 0;
	
}


MWindow::~MWindow()
{
	mainLayout = NULL;
	viewerWindow = NULL;
	
}
