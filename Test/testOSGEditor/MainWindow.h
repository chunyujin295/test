#ifndef  MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "ui_MainWindow.h"
#include <OSGDB/ConvertUTF>
#include <OSGDB/WriteFile>
#include "ViewerQT.h"
#include <osg/Point>
#include <osg/LineWidth>
#include <osgUtil/DelaunayTriangulator>
#include <osgGA/TerrainManipulator>


#include<QGridLayout>
#include<QProgressBar>

struct Location
{
	double L1,L2,L3,L4,L5,L6,L7,L8;
};


class MWindow : public QMainWindow
{
	Q_OBJECT
public:
	MWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~MWindow();
	
	
	enum viewmode_e {
		VIEWMODE_HOME = 0,
		VIEWMODE_TOP=1 ,
		VIEWMODE_BOTTOM=2, 
		VIEWMODE_LEFT =3,
		VIEWMODE_RIGHT=4 ,
		VIEWMODE_FRONT=5 ,
		VIEWMODE_BACK = 6,
		VIEWMODE_FRONTI = 7,
		VIEWMODE_BACKI = 8,
		
		
	};
private:
	Ui::MainWindow ui;
	QGridLayout * mainLayout;
	ViewerQT* viewerWindow;
	
	void addTextEditData(QString str);
		void addTextEidtListStr(QList<QString> str_list);
	
	
	bool isFirst;/** the first time construct the structure of scence. main node for mannual.*/
	bool isSemiautoFirst;/** the first time construct the structure of scence. main node for semiauto.*/
	QDockWidget *dock;
	QString baseosgpath;
	std::vector<std::string> str_osgbfilelist_;
	std::vector<std::string> str_lowestlevel_osgbfilelist_;
	std::vector<std::string> str_highestlevel_osgbfilelist_;
	QProgressBar* progressBar;
	std::vector<std::string> meshfiles_;
	void SetView(viewmode_e viewmode);
public:
	
	float polygonHeight;
	osg::ref_ptr<osg::Switch> previewFile;
	int blockNumber; 
	float expand;
	

private slots:
	
	void on_actionLoadFile_triggered();
 private:
   

//////////////////////////////////////////////////////////////////////////
private:

	

//////////////////////////////////////////////////////////////////////////
	

};
extern MWindow* gMainWindow;

#endif //MAINWINDOW_H