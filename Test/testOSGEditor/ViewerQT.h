#ifndef VIEWERQT_H
#define VIEWERQT_H

#include "AdapterWidget.h"

//摄像机设置：
//视点，投影方法，图形窗口，

class ViewerQT : public osgViewer::Viewer, public AdapterWidget
{
public:
	ViewerQT(QWidget * parent = 0, const char * name = 0, const QGLWidget * shareWidget = 0, WindowFlags f = 0):
	  AdapterWidget( parent, name, shareWidget, f )
	  {
		  getCamera()->setViewport(new osg::Viewport(0,0,width(),height()));
		  getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width())/static_cast<double>(height()), 1.0f, 10000.0f);
		
		  getCamera()->setGraphicsContext(getGraphicsWindow());
		 
		  setThreadingModel(osgViewer::Viewer::AutomaticSelection);
		  connect(&_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
		  _timer.start(10);
		  
		  
	  }
	  virtual void paintGL()
	  {
		  frame();
	  }

protected:
	QTimer _timer;
};


#endif//VIEWERQT_H