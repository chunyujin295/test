// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Gui/GraphicsView.h"
#include "Core/File.h"
#include "Gui/ControlPoint2DNode.h"
#include "Gui/controlPointCircleCross.h"
//#include "stlplus3/file_system.hpp"

// Qt4 headers
#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include <QAction>
#include <QMessageBox>
#include <memory>
#include <QPen>
#include <QInputDialog>
#include <QImage>
#include <QScrollBar>
#include <filesystem>
#include <iterator>
#include <QApplication>
#include "3DViewer/qt_utils.h"
#include "Core/Logging.h"
#include "Core/File.h"

#include "Util/TaskProcess.h"

#define NEW_GV 1

namespace control_point_GUI
{
  using namespace std;
//  using namespace openMVG;
//  using namespace openMVG::sfm;

  // =========================================================================
  // Public methods
  // =========================================================================
  GraphicsView::GraphicsView(QWidget * parent)
    : QGraphicsView(parent), scene(new QGraphicsScene),mMoveStart(false),mContinuousMove(false)
    ,mMousePoint(QPoint(0,0)),posCross(QPoint(0,0))
  {
    setScene(scene);
    // The OpenGL rendering does not seem to work with too big images.
    //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    setBackgroundRole(QPalette::Dark);

#ifdef NEW_GV
#pragma message("new gv defined.")
    // 从左上角开始显示刺点图片.
    setAlignment(Qt::AlignLeft|Qt::AlignTop);
    //setAlignment(Qt::AlignCenter);
#else
#pragma message("no gv  defined.")
    setAlignment(Qt::AlignCenter);
#endif

    setCacheMode(QGraphicsView::CacheBackground);

    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
    //setDragMode(QGraphicsView::ScrollHandDrag);

    //installEventFilter(this);
	image = new QGraphicsPixmapItem;
	image->setTransformationMode(Qt::FastTransformation);
	scene->addItem(image);
	image->hide();

	CircleCross = new ControlPointCircleCross();
	scene->addItem(CircleCross);
	CircleCross->hide();

	redCrossItem = new ControlPoint2DNode();
	scene->addItem(redCrossItem);
	redCrossItem->hide();

    createActions();

	//alka
	init();
	// alke
   
  }

  void GraphicsView::resizeEvent(QResizeEvent* event)
  {
      QGraphicsView::resizeEvent(event);

      int ww = event->size().width();
      int hh = event->size().height();

#ifdef NEW_GV
      
      // check whether the scrollbar state is ok.
      ///scene->setSceneRect(QRectF(0, 0, ww, hh));
#endif
  }

  void GraphicsView::zoomIn()
  {
	// alka
	m_scale *= 1.25;
	// alke
    scale(1.25, 1.25); 
    
    update();

    savedFactor *= 1.25;

    // todo:process it later.
    // todo:need to zoom with control point / prediction point centered?
    emit ChangeScale(m_scale);
    
///    centerAt({ 0,0 });
  }

  void GraphicsView::zoomOut() { 
	  
	// alka
    
   /* if (m_scale <= 1.0)
    {
        m_scale = 1.0;
        normalSize();
    }       
    else*/
#if 0
    if (m_scale > 1.0)
#else
    if(m_scale > originalScale)
#endif
    {
        m_scale *= 0.8;
        scale(0.8, 0.8); 

        savedFactor *= 0.8;

        update();

        emit ChangeScale(m_scale);
 ///       centerAt({ 0,0 });
    }
       
	// alke
	
   
  }
  void GraphicsView::normalSize() { 
	  
	// alka
	m_scale = 1.0;
	m_pos = QPoint(0,0);
	// alke
	resetTransform();  
    /// scale(1/m_scale,1/m_scale);

    ///update(); 
  }

  void GraphicsView::removeControlPoint()
  {
      if(this->items().size() > 1)
      {
          QGraphicsItem *itemTemp =  this->items().at(0);
          scene->removeItem(itemTemp);
          delete itemTemp;

      }

  }

  // =========================================================================
  // Protected methods
  // =========================================================================
  void GraphicsView::drawBackground(QPainter *painter,
    const QRectF &rect)
  {
  }
  //?chy
  void GraphicsView::mousePressEvent (QMouseEvent* e )
  {

      if(QApplication::keyboardModifiers() == Qt::ShiftModifier)
      {
          if(e->button() == Qt::LeftButton){
			
			  //已经有图片
///			  if (image->isVisible()) 
              {
                // 获取鼠标点在场景(Scene)中的坐标值.
                 posCross =  this->mapToScene(e->pos());

                 QPointF posCross4Display = posCross;

                 //image->scenePos().x() << " " << image->scenePos().y()

                 // 保存图片在场景中的场景坐标值.
                 previousImageXoff = image->scenePos().x();
                 previousImageYoff = image->scenePos().y();

                 // 根据当前鼠标的场景坐标值与图片的场景坐标值的差值,构建posCross.
                 posCross.setX(posCross.x() - image->scenePos().x());
                 posCross.setY(posCross.y() - image->scenePos().y());

#if 1
///                 if(posCross.x() < image->boundingRect().width() &&posCross.y() < image->boundingRect().height() &&
///                        posCross.x() > 0 &&posCross.y() >0)
                 {
				///	   addNode(posCross);
                     // 根据鼠标在场景中的坐标值,增加刺点坐标节点.
                     addNode(posCross4Display);

                       std::cout << "posPoint:" << e->pos().x() << " / " << e->pos().y() << " vs " << posCross.x() << " / " << posCross.y() << std::endl
                           << " image:" << image->boundingRect().x() << " "  << image->boundingRect().y() << " image2:" << image->scenePos().x() << " " << image->scenePos().y() << std::endl
                           << " scene:" << scene->itemsBoundingRect().x() << " " << scene->itemsBoundingRect().y() << std::endl
                           << " scene2:" << scene->sceneRect().x() << " " << scene->sceneRect().y() << std::endl
                           << std::endl;

					   emit PosPoint(imageid, posCross);
                 }
#else
                 QPointF posImage = image->scenePos();

                 qreal xoff = posCross.x() - posImage.x();
                 qreal yoff = posCross.y() - posImage.y();

//                 CircleCross->setPos(viewCenterInsideScene);
                 posCross = QPointF(xoff, yoff);

                 addNode(posCross);

                 std::cout << "posPoint:" << e->pos().x() << e->pos().y();
                 emit PosPoint(imageid, posCross);
#endif
              }
          }

      }
    QGraphicsView::mousePressEvent(e);

  }


  void GraphicsView::mouseReleaseEvent(QMouseEvent *e)
  {

      mMoveStart = false;

      if(QApplication::keyboardModifiers() != Qt::ShiftModifier)
      {
          setCursor(Qt::ArrowCursor);
      }
      QGraphicsView::mouseReleaseEvent(e);

  }

  void GraphicsView::mouseMoveEvent(QMouseEvent *e)
  {


      if(e->modifiers() != Qt::ShiftModifier)
      {
          
          if(e->buttons() & Qt::LeftButton )
          {

            if(!mMoveStart)
            {

                mMoveStart = true;
                mContinuousMove = false;
                mMousePoint = e->globalPos(); 
            }
            else
            {
                setCursor(Qt::ClosedHandCursor);
                QScrollBar *scrollBarx = this->horizontalScrollBar();
                QScrollBar *scrollBary = this->verticalScrollBar();

                QPoint p = e->globalPos();
                int offsetx = p.x() - mMousePoint.x();
                int offsety = p.y() - mMousePoint.y();
                if(!mContinuousMove && (offsetx > -10 && offsetx < 10) && (offsety > -10 && offsety < 10))
                    return ;

                mContinuousMove = true;

                scrollBarx->setValue(scrollBarx->value() - offsetx);
                scrollBary->setValue(scrollBary->value() - offsety);

                mMousePoint = p;
            }
            emit MousePoint(mMousePoint);
          }
      }

      QGraphicsView::mouseMoveEvent(e);
  }

  void GraphicsView::keyPressEvent(QKeyEvent *event)
  {

      if(event->key() == Qt::Key_Shift)
      {
          setCursor(Qt::CrossCursor);
		  CircleCross->changeAlpha(50);
          for (auto &line: Lines)
          {
              line->setOpacity(0.2);
          }
      }
  }

  void GraphicsView::keyReleaseEvent(QKeyEvent *event)
  {
      if(event->key() == Qt::Key_Shift)
      {
          setCursor(Qt::ArrowCursor);
          CircleCross->changeAlpha(255);
          for (auto& line : Lines)
          {
              line->setOpacity(1);
          }
      }

  }

  void GraphicsView::wheelEvent ( QWheelEvent * e)
  {

      const int numSteps = e->delta() / 15 / 8;
      if (numSteps == 0) {
        e->ignore();
        return;
      }
      if (numSteps > 0)
        zoomIn();
      else
        zoomOut();
      e->accept();


  }

  void GraphicsView::zoom(qreal factor, QPointF centerPoint)
  {
      std::cout  << " " << __FUNCTION__ << " " << __LINE__ << " m_scale:" << m_scale << std::endl;
	// alka
	  m_scale *= factor;
    // alke
	   
    scale(factor, factor);
    centerOn(centerPoint);
  }

  // =========================================================================
  // Private methods
  // =========================================================================
  void GraphicsView::createActions()
  {

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));
    addAction(zoomInAct);

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));
    addAction(zoomOutAct);

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+1"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));
    addAction(normalSizeAct);

    //removeControlPointAct = new QAction(tr("&Remove Control Point"), this);
    //removeControlPointAct->setShortcut(tr("Del"));
    //removeControlPointAct->setEnabled(false);
    //connect(removeControlPointAct, SIGNAL(triggered()), this, SLOT(removeControlPoint()));
    //addAction(removeControlPointAct);
  }

  void GraphicsView::AddLine(const QPointF& pt1, const QPointF& pt2)
  {
     // QGraphicsLineItem* lineitem = new QGraphicsLineItem(); 
      QGraphicsLineItem* lineitem = new QGraphicsLineItem(0, 10, 20, 10);
      //lineitem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      QPen myPen;
      myPen.setWidth(0);
      myPen.setColor(QColor(Qt::green));
      //lineitem->setPen(QPen(QBrush(Qt::green), 1));
      lineitem->setPen(myPen);
      lineitem->setLine(pt1.x(), pt1.y(), pt2.x(), pt2.y());
      Lines.push_back(lineitem);
      scene->addItem(lineitem);

     // std::cout << " add line:" << Lines.size() << std::endl;

      return;


  }

  void GraphicsView::HideLine()
  {
      for (auto perline: Lines)
      {
          perline->hide();
      }
  }

  void GraphicsView::ClearLine()
  {
      for (auto line:Lines)
      {
          scene->removeItem(line);
      }
      if (Lines.size() > 0)
          Lines.clear();
  }

  void GraphicsView::ShowLine()
  {
      for (auto perline : Lines)
      {
          perline->show();
      }

  }

  //void GraphicsView::AddLines(std::vector<const QPointF& pt1, const QPointF& pt2> lines)
  //{
  //    // QGraphicsLineItem* lineitem = new QGraphicsLineItem();     
  //    QGraphicsLineItem* lineitem = new QGraphicsLineItem(0, 10, 20, 10);
  //    lineitem->setPen(QPen(QBrush(Qt::green), 5));
  //    lineitem->setLine(pt1.x(), pt1.y(), pt2.x(), pt2.y());
  //    Lines.push_back(lineitem);
  //    scene->addItem(lineitem);
  //}

  // 加入刺点大图并显示.
  void GraphicsView::addImage(const QString& qs_filename, int imageid_, float xpos, float ypos, bool bClear)
  {
      // alka reset info
///      if (bFirstImage)
   //   std::cout << __FILE__ << __FUNCTION__ << __LINE__;
      imageid = imageid_;
      //      QPixmap pixmap;
      AI3D::CORE::Bitmap bitmap;
      //qimage先读，读不出来再用bitmap，效率待测试

      bool bGotPixmapSucc = false;

      // todo:need to reset scrollbar position to zero?

      if (!pixmap.isNull() && pixmap.width() > 0 && pixmap.height() > 0 && !pictureName.isEmpty() && !qs_filename.isEmpty()
          && !pictureName.compare(qs_filename, Qt::CaseInsensitive))
      {
          // optimization for later use after first parsing.
          // 图片已经有缓冲
      }
      else
      {
          qDebug()<<qs_filename;
        
          pictureName = qs_filename;
          pixmap = QPixmap(qs_filename);


         
          if (!pixmap.isNull() && pixmap.width() > 0 && pixmap.height() > 0)
          {
              
              ;
          }

          else if (AI3D::CORE::File::IsFileExistent(qstr2str(const_cast<QString &>(qs_filename))))
          {

              if (bitmap.Read(qstr2str(const_cast<QString &>(qs_filename))))
              {
                  pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
              }
          }

          else
          {

              QString filestr = ":/new/prefix1/skin/default.png";



              bitmap.Read(qstr2str(filestr));

              pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
          }

      }

      if (!pixmap.isNull() && pixmap.width() > 0 && pixmap.height() > 0)
      {
          // 如果解析图片正确,设置标志.
          bGotPixmapSucc = true;
      }

      ///setUpdatesEnabled(false);

      std::cout << "add image:" << pixmap.width() << pixmap.height();

      if (!bGotPixmapSucc)
          return;

        //resetTransform();
      // m_scale为GraphicsView的前次缩放比列,此处将GraphicwView缩放比例复位.
      scale(1.0/m_scale, 1.0/m_scale);

///      m_scale = 1.0;

      m_pos = QPoint(0, 0);
      // alke

      bExistsNode = false;
      bExistsCircleCross = false;


///      const QPointF offset = QPointF(xpos, ypos);
      const QPointF offset = QPointF(0.0, 0.0);
      image->setPixmap(pixmap);
      image->setPos(offset);
      image->show();

      zoomInAct->setEnabled(true);
      zoomOutAct->setEnabled(true);
      normalSizeAct->setEnabled(true);

      // removeControlPointAct->setEnabled(true);
      //QMessageBox::information(this, QString::null,
     //chy tr("Cannot load QPixmap %1.").arg(qs_filename)); 此处改为显示某种图吧
     // return;
 /* else
  {
    const QPointF offset = QPointF(xpos, ypos);
    image->setPixmap(pixmap);
    image->setPos(offset);
    image->show();
    zoomInAct->setEnabled(true);
    zoomOutAct->setEnabled(true);
    normalSizeAct->setEnabled(true);
    removeControlPointAct->setEnabled(true);

  }*/

  // alkc

//      int imageWidth = (int)image->boundingRect().width();
//      int imageHeight = (int)image->boundingRect().height();
      int imageWidth = pixmap.width();
      int imageHeight = pixmap.height();

      int vw = this->width();

      int vh = this->height();

      qreal sx = 1.0;
      qreal sy = 1.0;

      // 根据可视区域的尺寸及图片的宽高,计算将图片缩放到可视区域能容纳缩放后的图片.
      if(imageWidth > 0)
          sx = 1.0 * vw / imageWidth;

      if(imageHeight > 0)
          sy = 1.0 * vh / imageHeight;

      // the scale value's caculation from caller is based on current scale.

      if (sx > sy)
      {
          m_scale = sx;
      }
      else
      {
          m_scale = sy;
      }

      std::cout << imageWidth << " " << imageHeight << " vs " << vw << " " << vh << " sx/sy: " << sx << " " << sy << " " << m_scale << std::endl;

   ///   if (bGotPixmapSucc)
      {
          // 记录图片双边缩放后都能在可视区域宽高范围内的最大缩放值.
          originalScale = m_scale;

          // savedFactor: 为前一次加入的图片的缩放比例,即新的图片使用该值保持与前一次图片同样的缩放比例.
          // 缩放因子是以图片宽高缩放后可以完全在可视区显示的缩放比例作为基准1.0.
          if (bFirstImage)
          {
                bFirstImage = false;
                savedFactor = 1.0;
          }
          else
          {
              m_scale *= savedFactor;
          }

          // 根据计算后的最大缩放值,缩放刺点大图.
          scale(m_scale, m_scale);
      }

      // todo:keeping previous scale factor while switch to different image,

      ///if (bFirstImage)
      {
///          fitInView(image, Qt::KeepAspectRatio);
          ///    update();

///          bFirstImage = false;
      }
      //else
      //{
      //    fitInView(image, Qt::KeepAspectRatio);
      //    scale(m_scale, m_scale);
      //    ///   update();
      //}

      ///setUpdatesEnabled(true);

      /*while (horizontalScrollBar()->maximum()>0 | verticalScrollBar()->maximum()>0) {
          zoomOut();
          update();
      }*/

      // alke

      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  }

  // 测试代码,现未使用
  void GraphicsView::addImageV2(const QString & qs_filename,int imageid_ ,float xpos, float ypos, bool bClear)
  {
	  // alka reset info
      if (bFirstImage)
      {
          m_scale = 1.0;
      }
      else
      {

      }


	  m_pos = QPoint(0,0);
	  // alke

	  imageid = imageid_;
///	  QPixmap pixmap;
	  AI3D::CORE::Bitmap bitmap;
      
	  //qimage先读，读不出来再用bitmap，效率待测试

      {
          pictureName = qs_filename;

          pixmap = QPixmap(qs_filename);
          if (!pixmap.isNull() && pixmap.width() > 0 && pixmap.height() > 0)
              ;
          else if (bitmap.Read(qstr2str(const_cast<QString &>(qs_filename))))
          {
              pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
          }
          else
          {
              QString filestr = QApplication::applicationDirPath();
              filestr.append(":/new/prefix1/skin/default.png");

              bitmap.Read(qstr2str(filestr));

              pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
          }
      }

      setUpdatesEnabled(false);

	  const QPointF offset = QPointF(xpos, ypos);
	  image->setPixmap(pixmap);
	  image->setPos(offset);
	  image->show();
	  zoomInAct->setEnabled(true);
	  zoomOutAct->setEnabled(true);
	  normalSizeAct->setEnabled(true);
	  removeControlPointAct->setEnabled(true);
        //QMessageBox::information(this, QString::null,
       //chy tr("Cannot load QPixmap %1.").arg(qs_filename)); 此处改为显示某种图吧
       // return;
   /* else 
    {
      const QPointF offset = QPointF(xpos, ypos);
      image->setPixmap(pixmap);
      image->setPos(offset);
	  image->show();
      zoomInAct->setEnabled(true);
      zoomOutAct->setEnabled(true);
      normalSizeAct->setEnabled(true);
      removeControlPointAct->setEnabled(true);

    }*/

    // alkc

      if (bFirstImage)
      {
          fitInView(image, Qt::KeepAspectRatio);
      ///    update();

          bFirstImage = false;
      }
      else
      {
          fitInView(image, Qt::KeepAspectRatio);
          scale(m_scale, m_scale);
       ///   update();
      }
  
      setUpdatesEnabled(true);
	/*while (horizontalScrollBar()->maximum()>0 | verticalScrollBar()->maximum()>0) {
		zoomOut();
		update();
	}*/
	
	// alke

  }


  void GraphicsView::addNode(QPointF pointf)
  { 
     
      redCrossItem->setPos(pointf);
	  redCrossItem->show();

      bExistsNode = true;
      pointNode = pointf;
  }

  void GraphicsView::hideNode()
  {
      redCrossItem->hide();
  }
  void  GraphicsView::addCircleCross(QPointF pointf) 
  {
	  CircleCross->setPos(pointf);
	  CircleCross->show();

      bExistsCircleCross = true;
      pointCircleCross = pointf;
  }

  // 测试代码,现未使用
  void GraphicsView::keepPreviousPos()
  {
      qreal xoff;
      qreal yoff;

      QPointF point = image->scenePos();
      xoff = previousImageXoff - point.x();
      yoff = previousImageYoff - point.y();
      
      //viewCenterInsideScene.y() - redCrossItem->scenePos().y();


      if (bExistsNode)
      {
         
          point = redCrossItem->scenePos();

          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          point += QPointF(xoff, yoff);

          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          redCrossItem->setPos(point);

          point = image->scenePos();
          point += QPointF(xoff, yoff);
          image->setPos(point);

          point = CircleCross->scenePos();
          point += QPointF(xoff, yoff);
          CircleCross->setPos(point);

          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
      else if (bExistsCircleCross)
      {
          point = CircleCross->scenePos();
          point += QPointF(xoff, yoff);
          CircleCross->setPos(point);

          point = image->scenePos();
          point += QPointF(xoff, yoff);
          image->setPos(point);

          point = redCrossItem->scenePos();
          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          point += QPointF(xoff, yoff);
          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          redCrossItem->setPos(point);

          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
  }

  // 居中显示刺点大图.
  void GraphicsView::centerAt(QPointF ponitf)
  {
      // place node or circleCross in the center position of parent scene.
      // todo:adjust related item simultaneously.and scrollbar related changed.

      // if has change scrollbar,don't need to center at.
      // first addNode/Circle or scale without any scrollbar'action.

 //     return;

      int vw = width();
      int vh = height();

      qreal xoff;
      qreal yoff;

      // 获取可视区域的中心的场景坐标.
      QPointF viewCenterInsideScene = mapToScene(QPoint(vw / 2, vh / 2));
   //代表有需要中心点要偏移
      if (bExistsNode)
      {
          // 如果存在刺点值
        
          // 计算可视区中心点与当前刺点的场景坐标的偏移值 => xoff,yoff.
          // 该偏移值表明将移动(刺点移到中心位置)的距离.
          xoff = viewCenterInsideScene.x() - redCrossItem->scenePos().x();
          yoff = viewCenterInsideScene.y() - redCrossItem->scenePos().y();

          // 将红色刺点移到可视区中心位置.
          redCrossItem->setPos(viewCenterInsideScene);//重新设置场景作标

          // 获取图片的场景坐标
          QPointF point = image->scenePos();

          // 将图片加上xoff/yoff,即按照红色刺点移动同样的距离,移动图片相应的距离及方向.
          point += QPointF(xoff, yoff);
          image->setPos(point);

          // 同理,移动绿色预测点,如果存在的话.
          point = CircleCross->scenePos();
          point += QPointF(xoff, yoff);
          CircleCross->setPos(point);

          // 同理,移动核线.
          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
      else if (bExistsCircleCross)
      {
        
          // 计算可视区中心点与绿色预测点的场景坐标的偏移值 => xoff,yoff.
          // 该偏移值表明将移动(刺点移到中心位置)的距离.
          xoff = viewCenterInsideScene.x() - CircleCross->scenePos().x();
          yoff = viewCenterInsideScene.y() - CircleCross->scenePos().y();

          // 将绿色预测点移到可视区中心位置.
          CircleCross->setPos(viewCenterInsideScene);

          // 将图片加上xoff/yoff,即按照绿色预测点移动同样的距离,移动图片相应的距离及方向.
          QPointF point = image->scenePos();

          point += QPointF(xoff, yoff);
          image->setPos(point);

          // 同理,移动红色刺点,如果存在的话.
          point = redCrossItem->scenePos();
          point += QPointF(xoff, yoff);
          redCrossItem->setPos(point);

          // 同理,移动核线.
          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
  }

  void GraphicsView::clear()
  {
	 // for (QGraphicsItem *item : this->items()) {
		//  delete item;
		//}
	 // scene->clear();
	  redCrossItem->hide();
	  CircleCross->hide();//20211208zhishi
	  image->hide();
      //Lines.clear();
     
      ClearLine();

      std::cout << "inside gv clear " << __LINE__ << " " << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() << std::endl;

      ///QPoint pos(0, 0);
      ///setOffset(pos);
      
      ///horizontalScrollBar()->setValue(pos.x());
      ///verticalScrollBar()->setValue(pos.y());
      /// 
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  
  }

  // alka
  QString GraphicsView::getImageName() const {
	  return pictureName;
  }

  QPoint GraphicsView::getOffset() const {
  
	  return m_pos;
  }
  double GraphicsView::getScale() const {

	  return m_scale;
  }
  void GraphicsView::setOffset(const QPoint& pos) {
      ostringstream oss;

      m_pos = pos;

      std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << m_pos.x() << "/" << m_pos.y() << " " << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() << std::endl;

      oss << m_pos.x() << "/" << m_pos.y() << " " << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString();
      LOGI(oss.str());

	  horizontalScrollBar()->setValue(pos.x());
	  verticalScrollBar()->setValue(pos.y());
     
	  update();
  }
  
  void GraphicsView::setScale(const double& value) {
      return;

      m_scale = value;
	  scale(value, value);

	  update();

      centerAt(QPointF(0, 0));
  }
  
  void GraphicsView::setScale2(const double& value) {
      return;

      m_scale = value;
      scale(value, value);

///      update();

      // todo:check it later.
//      centerAt(QPointF(0, 0));
  }

  void GraphicsView::init() {
	
	  // variables
	  m_scale = 1.0;
	  m_pos = QPoint(0,0);

	  // signal
	  connect(horizontalScrollBar(), &QScrollBar::valueChanged, [=](int value) { 
          std::cout << "hscrollbar setpos:" << value << " " << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() << std::endl;
          m_pos.setX(value);
       
      });

	  connect(verticalScrollBar(), &QScrollBar::valueChanged, [=](int value) { 
          m_pos.setY(value);
          std::cout << "vscrollbar setpos:" << value << " " << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() << std::endl;
      });

	  // state
      setStyleSheet("border:none;background-color: rgb(22, 22, 22);");
      ///verticalScrollBar()->setMaximumWidth(5);
      ///horizontalScrollBar()->setMaximumHeight(5);

      horizontalScrollBar()->setStyleSheet("QScrollBar{height:10px;}");
      verticalScrollBar()->setStyleSheet("QScrollBar{width: 10px;}");

      /// todo: change the following scrollbar policy to Automatic later.
      ///       make a comparison test for different policies to check the actual effect.
	  //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	  //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  
      setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }


  void GraphicsView::dumpInfo()
  {
      //this->scale();
      return;
      double scale_ = getScale();
      ostringstream oss;
      
      oss << "scale:" << m_scale << " scalefactor:" << savedFactor <<
          " hsb:" << horizontalScrollBar()->value() << "/" << horizontalScrollBar()->minimum() << "/" << horizontalScrollBar()->maximum() <<
          " vsb:" << verticalScrollBar()->value() << "/" << verticalScrollBar()->minimum() << "/" << verticalScrollBar()->maximum();

      oss << " gv w/h sceneRect x/y/w/h:" << width() << "/" << height() << " " << sceneRect().x() << "/" << sceneRect().y() << " " << sceneRect().width() << "/" << sceneRect().height();
      oss << " m_pos " << m_pos.x() << "/" << m_pos.y();

      oss << " image:" << image->boundingRect().x() << " " << image->boundingRect().y() << " image2:" << image->scenePos().x() << " " << image->scenePos().y() << " " 
          << image->boundingRect().width() << " " << image->boundingRect().height() << " " << image->sceneBoundingRect().width() << " " << image->sceneBoundingRect().height() 
          << " scene:" << scene->itemsBoundingRect().x() << " " << scene->itemsBoundingRect().y() << " " << scene->itemsBoundingRect().width() << " " << scene->itemsBoundingRect().height();

      oss << "redcross:" << redCrossItem->isVisible() << " " << redCrossItem->scenePos().x() << " " << redCrossItem->scenePos().y() << " " << redCrossItem->boundingRect().width() << " " << redCrossItem->boundingRect().height();

      oss << "CircleCross:" << CircleCross->isVisible() << " " << CircleCross->scenePos().x() << " " << CircleCross->scenePos().y() << " " << CircleCross->boundingRect().width() << " " << CircleCross->boundingRect().height();

      for (auto line : Lines)
      {
///          point = line->scenePos();
///         point += QPointF(xoff, yoff);
///          line->setPos(point);
            oss << "Line:" << line->isVisible() << " " << line->scenePos().x() << " " << line->scenePos().y() << " " << line->boundingRect().width() << " " << line->boundingRect().height();
      }

      oss << "LineCount:" << Lines.count();

      LOGI(oss.str());
  }

  //alke

  /////////////////////////////////////////////////////////
  /// original version for GraphicsView
  /// GraphicsViewV0: 新版本未使用
  GraphicsViewV0::GraphicsViewV0(QWidget* parent)
      : QGraphicsView(parent), scene(new QGraphicsScene), mMoveStart(false), mContinuousMove(false)
      , mMousePoint(QPoint(0, 0)), posCross(QPoint(0, 0))
  {
      setScene(scene);
      // The OpenGL rendering does not seem to work with too big images.
      //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
      setBackgroundRole(QPalette::Dark);

#ifdef NEW_GV
#pragma message("new gv defined.")
      setAlignment(Qt::AlignLeft | Qt::AlignTop);
      //setAlignment(Qt::AlignCenter);
#else
#pragma message("no gv  defined.")
      setAlignment(Qt::AlignCenter);
#endif

      setCacheMode(QGraphicsView::CacheBackground);

      setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setMouseTracking(true);
      setFocusPolicy(Qt::WheelFocus);
      //setDragMode(QGraphicsView::ScrollHandDrag);

      //installEventFilter(this);
      image = new QGraphicsPixmapItem;
      image->setTransformationMode(Qt::FastTransformation);
      scene->addItem(image);
      image->hide();

      CircleCross = new ControlPointCircleCross();
      scene->addItem(CircleCross);
      CircleCross->hide();

      redCrossItem = new ControlPoint2DNode();
      scene->addItem(redCrossItem);
      redCrossItem->hide();

      createActions();

      //alka
      init();
      // alke

  }

  void GraphicsViewV0::resizeEvent(QResizeEvent* event)
  {
      QGraphicsView::resizeEvent(event);

      int ww = event->size().width();
      int hh = event->size().height();

#ifdef NEW_GV

      // todo:check whethere the scrollbar state is ok.
      ///scene->setSceneRect(QRectF(0, 0, ww, hh));
#endif
  }

  void GraphicsViewV0::zoomIn()
  {

      // alka
      m_scale *= 1.25;
      // alke
      scale(1.25, 1.25); update();
      // todo:process it later.
      emit ChangeScale(m_scale);

      ///    centerAt({ 0,0 });
  }

  void GraphicsViewV0::zoomOut() {

      // alka



     /* if (m_scale <= 1.0)
      {
          m_scale = 1.0;
          normalSize();
      }
      else*/
      if (m_scale > 1.0)
      {
          m_scale *= 0.8;
          scale(0.8, .8);
          update();
          emit ChangeScale(m_scale);
          ///       centerAt({ 0,0 });
      }

      // alke


  }
  void GraphicsViewV0::normalSize() {

      // alka
      m_scale = 1.0;
      m_pos = QPoint(0, 0);
      // alke
      resetTransform();  update();
  }

  void GraphicsViewV0::removeControlPoint()
  {
      if (this->items().size() > 1)
      {
          QGraphicsItem* itemTemp = this->items().at(0);
          scene->removeItem(itemTemp);
          delete itemTemp;

      }

  }

  // =========================================================================
  // Protected methods
  // =========================================================================
  void GraphicsViewV0::drawBackground(QPainter* painter,
      const QRectF& rect)
  {
  }

  void GraphicsViewV0::mousePressEvent(QMouseEvent* e)
  {

      if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
      {
          if (e->button() == Qt::LeftButton) {

              //已经有图片
///			  if (image->isVisible()) 
              {
                  posCross = this->mapToScene(e->pos());

                  QPointF posCross4Display = posCross;

                  //image->scenePos().x() << " " << image->scenePos().y()

                  previousImageXoff = image->scenePos().x();
                  previousImageYoff = image->scenePos().y();

                  posCross.setX(posCross.x() - image->scenePos().x());
                  posCross.setY(posCross.y() - image->scenePos().y());

#if 1
                  ///                 if(posCross.x() < image->boundingRect().width() &&posCross.y() < image->boundingRect().height() &&
                  ///                        posCross.x() > 0 &&posCross.y() >0)
                  {
                      ///	   addNode(posCross);
                      addNode(posCross4Display);

                      std::cout << "posPoint:" << e->pos().x() << " / " << e->pos().y() << " vs " << posCross.x() << " / " << posCross.y() << std::endl
                          << " image:" << image->boundingRect().x() << " " << image->boundingRect().y() << " image2:" << image->scenePos().x() << " " << image->scenePos().y() << std::endl
                          << " scene:" << scene->itemsBoundingRect().x() << " " << scene->itemsBoundingRect().y() << std::endl
                          << " scene2:" << scene->sceneRect().x() << " " << scene->sceneRect().y() << std::endl
                          << std::endl;

                      emit PosPoint(imageid, posCross);
                  }
#else
                  QPointF posImage = image->scenePos();

                  qreal xoff = posCross.x() - posImage.x();
                  qreal yoff = posCross.y() - posImage.y();

                  //                 CircleCross->setPos(viewCenterInsideScene);
                  posCross = QPointF(xoff, yoff);

                  addNode(posCross);

                  std::cout << "posPoint:" << e->pos().x() << e->pos().y();
                  emit PosPoint(imageid, posCross);
#endif
              }
          }

      }
      QGraphicsView::mousePressEvent(e);

  }


  void GraphicsViewV0::mouseReleaseEvent(QMouseEvent* e)
  {
      mMoveStart = false;

      if (QApplication::keyboardModifiers() != Qt::ShiftModifier)
      {
          setCursor(Qt::ArrowCursor);
      }

      QGraphicsView::mouseReleaseEvent(e);
  }

  void GraphicsViewV0::mouseMoveEvent(QMouseEvent* e)
  {


      if (e->modifiers() != Qt::ShiftModifier)
      {
       
          if (e->buttons() & Qt::LeftButton)
          {
              if (!mMoveStart)
              {
                  mMoveStart = true;
                  mContinuousMove = false;
                  mMousePoint = e->globalPos();
              }
              else
              {
                  setCursor(Qt::ClosedHandCursor);
                  QScrollBar* scrollBarx = this->horizontalScrollBar();
                  QScrollBar* scrollBary = this->verticalScrollBar();

                  QPoint p = e->globalPos();
                  int offsetx = p.x() - mMousePoint.x();
                  int offsety = p.y() - mMousePoint.y();
                  if (!mContinuousMove && (offsetx > -10 && offsetx < 10) && (offsety > -10 && offsety < 10))
                      return;

                  mContinuousMove = true;

                  scrollBarx->setValue(scrollBarx->value() - offsetx);
                  scrollBary->setValue(scrollBary->value() - offsety);

                  mMousePoint = p;
              }
              emit MousePoint(mMousePoint);
          }
      }

      QGraphicsView::mouseMoveEvent(e);
  }

  void GraphicsViewV0::keyPressEvent(QKeyEvent* event)
  {

      if (event->key() == Qt::Key_Shift)
      {
          setCursor(Qt::CrossCursor);
          CircleCross->changeAlpha(50);
          for (auto& line : Lines)
          {
              line->setOpacity(0.2);
          }
      }
  }

  void GraphicsViewV0::keyReleaseEvent(QKeyEvent* event)
  {
      if (event->key() == Qt::Key_Shift)
      {
          setCursor(Qt::ArrowCursor);
          CircleCross->changeAlpha(255);
          for (auto& line : Lines)
          {
              line->setOpacity(1);
          }
      }

  }

  void GraphicsViewV0::wheelEvent(QWheelEvent* e)
  {

      const int numSteps = e->delta() / 15 / 8;
      if (numSteps == 0) {
          e->ignore();
          return;
      }
      if (numSteps > 0)
          zoomIn();
      else
          zoomOut();
      e->accept();


  }

  void GraphicsViewV0::zoom(qreal factor, QPointF centerPoint)
  {
      std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " m_scale:" << m_scale << std::endl;
      // alka
      m_scale *= factor;
      // alke

      scale(factor, factor);
      centerOn(centerPoint);
  }

  // =========================================================================
  // Private methods
  // =========================================================================
  void GraphicsViewV0::createActions()
  {

      zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
      zoomInAct->setShortcut(tr("Ctrl++"));
      zoomInAct->setEnabled(false);
      connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));
      addAction(zoomInAct);

      zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
      zoomOutAct->setShortcut(tr("Ctrl+-"));
      zoomOutAct->setEnabled(false);
      connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));
      addAction(zoomOutAct);

      normalSizeAct = new QAction(tr("&Normal Size"), this);
      normalSizeAct->setShortcut(tr("Ctrl+1"));
      normalSizeAct->setEnabled(false);
      connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));
      addAction(normalSizeAct);

      //removeControlPointAct = new QAction(tr("&Remove Control Point"), this);
      //removeControlPointAct->setShortcut(tr("Del"));
      //removeControlPointAct->setEnabled(false);
      //connect(removeControlPointAct, SIGNAL(triggered()), this, SLOT(removeControlPoint()));
      //addAction(removeControlPointAct);
  }

  void GraphicsViewV0::AddLine(const QPointF& pt1, const QPointF& pt2)
  {
      // QGraphicsLineItem* lineitem = new QGraphicsLineItem(); 
      QGraphicsLineItem* lineitem = new QGraphicsLineItem(0, 10, 20, 10);
      //lineitem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      QPen myPen;
      myPen.setWidth(0);
      myPen.setColor(QColor(Qt::green));
      //lineitem->setPen(QPen(QBrush(Qt::green), 1));
      lineitem->setPen(myPen);
      lineitem->setLine(pt1.x(), pt1.y(), pt2.x(), pt2.y());
      Lines.push_back(lineitem);
      scene->addItem(lineitem);

    //  std::cout << " add line:" << Lines.size() << std::endl;

      return;
  }

  void GraphicsViewV0::HideLine()
  {
      for (auto perline : Lines)
      {
          perline->hide();
      }
  }

  void GraphicsViewV0::ClearLine()
  {
      for (auto line : Lines)
      {
          scene->removeItem(line);
      }
      if (Lines.size() > 0)
          Lines.clear();
  }

  void GraphicsViewV0::ShowLine()
  {
      for (auto perline : Lines)
      {
          perline->show();
      }

  }

  //void GraphicsView::AddLines(std::vector<const QPointF& pt1, const QPointF& pt2> lines)
  //{
  //    // QGraphicsLineItem* lineitem = new QGraphicsLineItem();     
  //    QGraphicsLineItem* lineitem = new QGraphicsLineItem(0, 10, 20, 10);
  //    lineitem->setPen(QPen(QBrush(Qt::green), 5));
  //    lineitem->setLine(pt1.x(), pt1.y(), pt2.x(), pt2.y());
  //    Lines.push_back(lineitem);
  //    scene->addItem(lineitem);
  //}

  void GraphicsViewV0::addImage(const QString& qs_filename, int imageid_, float xpos, float ypos, bool bClear)
  {
      // alka reset info
///      if (bFirstImage)
   //   std::cout << __FILE__ << __FUNCTION__ << __LINE__;
      resetTransform();

      {
          m_scale = 1.0;
      }

      m_pos = QPoint(0, 0);
      // alke

      bExistsNode = false;
      bExistsCircleCross = false;

      pixtureName = qs_filename;
      imageid = imageid_;
      QPixmap pixmap;
      AI3D::CORE::Bitmap bitmap;
      //qimage先读，读不出来再用bitmap，效率待测试

      try
      {

          if (std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(qstr2str(const_cast<QString &>(qs_filename)))))
          {
              pixmap = QPixmap(qs_filename);
          }
          if (!pixmap.isNull() && pixmap.width() > 0 && pixmap.height() > 0)
              ;

          else if (std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(qstr2str(const_cast<QString &>(qs_filename)))) && bitmap.Read(qstr2str(const_cast<QString &>(qs_filename))))
          {
              pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
          }
          else
          {
              QString filestr = QApplication::applicationDirPath();
              filestr.append(":/new/prefix1/skin/default.png");

              bitmap.Read(qstr2str(filestr));

              pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
          }
      }
      catch (const std::filesystem::filesystem_error& fse)
      {
        std::ostringstream oss;
        oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
          LOGI(oss.str());
      }
      catch (std::exception & ex)
      {
          std::ostringstream oss;
          oss << "exception:" << ex.what();
          LOGI(oss.str());
      }

      ///setUpdatesEnabled(false);

      std::cout << "add image:" << pixmap.width() << pixmap.height();

      const QPointF offset = QPointF(xpos, ypos);
      image->setPixmap(pixmap);
      image->setPos(offset);
      image->show();
      zoomInAct->setEnabled(true);
      zoomOutAct->setEnabled(true);
      normalSizeAct->setEnabled(true);
      // removeControlPointAct->setEnabled(true);
       //QMessageBox::information(this, QString::null,
      //chy tr("Cannot load QPixmap %1.").arg(qs_filename)); 此处改为显示某种图吧
      // return;
  /* else
   {
     const QPointF offset = QPointF(xpos, ypos);
     image->setPixmap(pixmap);
     image->setPos(offset);
     image->show();
     zoomInAct->setEnabled(true);
     zoomOutAct->setEnabled(true);
     normalSizeAct->setEnabled(true);
     removeControlPointAct->setEnabled(true);

   }*/


   // alkc

      int imageWidth = (int)image->boundingRect().width();
      int imageHeight = (int)image->boundingRect().height();
      int vw = this->width();
      int vh = this->height();

      qreal sx = 1.0 * vw / imageWidth;
      qreal sy = 1.0 * vh / imageHeight;

      if (sx < sy)
      {
          m_scale = sx;
      }
      else
      {
          m_scale = sy;
      }

      std::cout << imageWidth << " " << imageHeight << " vs " << vw << " " << vh << " sx/sy: " << sx << " " << sy << " " << m_scale << std::endl;

      scale(m_scale, m_scale);

      ///if (bFirstImage)
      {
          ///          fitInView(image, Qt::KeepAspectRatio);
                    ///    update();

          ///          bFirstImage = false;
      }
      //else
      //{
      //    fitInView(image, Qt::KeepAspectRatio);
      //    scale(m_scale, m_scale);
      //    ///   update();
      //}

      ///setUpdatesEnabled(true);

      /*while (horizontalScrollBar()->maximum()>0 | verticalScrollBar()->maximum()>0) {
          zoomOut();
          update();
      }*/

      // alke

  }

  void GraphicsViewV0::addImageV2(const QString& qs_filename, int imageid_, float xpos, float ypos, bool bClear)
  {
      // alka reset info
      if (bFirstImage)
      {
          m_scale = 1.0;
      }
      else
      {

      }


      m_pos = QPoint(0, 0);
      // alke

      pixtureName = qs_filename;
      imageid = imageid_;
      QPixmap pixmap;
      AI3D::CORE::Bitmap bitmap;
      //qimage先读，读不出来再用bitmap，效率待测试

      pixmap = QPixmap(qs_filename);
      if (!pixmap.isNull() && pixmap.width() > 0 && pixmap.height() > 0)
          ;

      else if (bitmap.Read(qstr2str(const_cast<QString &>(qs_filename))))
      {
          pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
      }
      else
      {
          QString filestr = QApplication::applicationDirPath();
          filestr.append(":/new/prefix1/skin/default.png");

          bitmap.Read(qstr2str(filestr));

          pixmap = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));
      }

      setUpdatesEnabled(false);

      const QPointF offset = QPointF(xpos, ypos);
      image->setPixmap(pixmap);
      image->setPos(offset);
      image->show();
      zoomInAct->setEnabled(true);
      zoomOutAct->setEnabled(true);
      normalSizeAct->setEnabled(true);
      removeControlPointAct->setEnabled(true);
      //QMessageBox::information(this, QString::null,
     //chy tr("Cannot load QPixmap %1.").arg(qs_filename)); 此处改为显示某种图吧
     // return;
 /* else
  {
    const QPointF offset = QPointF(xpos, ypos);
    image->setPixmap(pixmap);
    image->setPos(offset);
    image->show();
    zoomInAct->setEnabled(true);
    zoomOutAct->setEnabled(true);
    normalSizeAct->setEnabled(true);
    removeControlPointAct->setEnabled(true);

  }*/

  // alkc

      if (bFirstImage)
      {
          fitInView(image, Qt::KeepAspectRatio);
          ///    update();

          bFirstImage = false;
      }
      else
      {
          fitInView(image, Qt::KeepAspectRatio);
          scale(m_scale, m_scale);
          ///   update();
      }

      setUpdatesEnabled(true);
      /*while (horizontalScrollBar()->maximum()>0 | verticalScrollBar()->maximum()>0) {
          zoomOut();
          update();
      }*/

      // alke

  }


  void GraphicsViewV0::addNode(QPointF pointf)
  {
     
      redCrossItem->setPos(pointf);
      redCrossItem->show();

      bExistsNode = true;
      pointNode = pointf;
  }

  void GraphicsViewV0::hideNode()
  {
      redCrossItem->hide();
  }
  void  GraphicsViewV0::addCircleCross(QPointF pointf)
  {
     

      CircleCross->setPos(pointf);
      CircleCross->show();

      bExistsCircleCross = true;
      pointCircleCross = pointf;
  }

  void GraphicsViewV0::keepPreviousPos()
  {
      qreal xoff;
      qreal yoff;

      QPointF point = image->scenePos();
      xoff = previousImageXoff - point.x();
      yoff = previousImageYoff - point.y();

      //viewCenterInsideScene.y() - redCrossItem->scenePos().y();


      if (bExistsNode)
      {
         
          point = redCrossItem->scenePos();

          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          point += QPointF(xoff, yoff);

          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          redCrossItem->setPos(point);

          point = image->scenePos();
          point += QPointF(xoff, yoff);
          image->setPos(point);

          point = CircleCross->scenePos();
          point += QPointF(xoff, yoff);
          CircleCross->setPos(point);

          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
      else if (bExistsCircleCross)
      {
          point = CircleCross->scenePos();
          point += QPointF(xoff, yoff);
          CircleCross->setPos(point);

          point = image->scenePos();
          point += QPointF(xoff, yoff);
          image->setPos(point);

          point = redCrossItem->scenePos();
          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          point += QPointF(xoff, yoff);
          std::cout << __FILE__ << " " << __LINE__ << " keepPreviousPos " << point.x() << " " << point.y() << std::endl;
          redCrossItem->setPos(point);

          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
  }

  void GraphicsViewV0::centerAt(QPointF ponitf)
  {
      // place node or circleCross in the center position of parent scene.
      // todo:adjust related item simultaneously.and scrollbar related changed.

      // if has change scrollbar,don't need to center at.
      // first addNode/Circle or scale without any scrollbar'action.

 //     return;

      int vw = width();
      int vh = height();

      qreal xoff;
      qreal yoff;

      QPointF viewCenterInsideScene = mapToScene(QPoint(vw / 2, vh / 2));

      if (bExistsNode)
      {
         

          xoff = viewCenterInsideScene.x() - redCrossItem->scenePos().x();
          yoff = viewCenterInsideScene.y() - redCrossItem->scenePos().y();

          redCrossItem->setPos(viewCenterInsideScene);

          QPointF point = image->scenePos();

          point += QPointF(xoff, yoff);
          image->setPos(point);

          point = CircleCross->scenePos();
          point += QPointF(xoff, yoff);
          CircleCross->setPos(point);

          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
      else if (bExistsCircleCross)
      {
          
          xoff = viewCenterInsideScene.x() - CircleCross->scenePos().x();
          yoff = viewCenterInsideScene.y() - CircleCross->scenePos().y();

          CircleCross->setPos(viewCenterInsideScene);

          QPointF point = image->scenePos();

          point += QPointF(xoff, yoff);
          image->setPos(point);

          point = redCrossItem->scenePos();
          point += QPointF(xoff, yoff);
          redCrossItem->setPos(point);

          for (auto line : Lines)
          {
              point = line->scenePos();
              point += QPointF(xoff, yoff);
              line->setPos(point);
          }
      }
  }

  void GraphicsViewV0::clear()
  {
      // for (QGraphicsItem *item : this->items()) {
         //  delete item;
         //}
      // scene->clear();
      redCrossItem->hide();
      CircleCross->hide();//20211208zhishi
      image->hide();
      //Lines.clear();

      ClearLine();

  }

  // alka
  QString GraphicsViewV0::getImageName() const {
      return pixtureName;
  }

  QPoint GraphicsViewV0::getOffset() const {

      return m_pos;
  }
  double GraphicsViewV0::getScale() const {

      return m_scale;
  }
  void GraphicsViewV0::setOffset(const QPoint& pos) {

      m_pos = pos;
      horizontalScrollBar()->setValue(pos.x());
      verticalScrollBar()->setValue(pos.y());
      
      update();
  }
  void GraphicsViewV0::setScale(const double& value) {
      m_scale = value;
      scale(value, value);

      update();

      centerAt(QPointF(0, 0));
  }

  void GraphicsViewV0::setScale2(const double& value) {
      m_scale = value;
      scale(value, value);

      ///      update();

            // todo:check it later.
      //      centerAt(QPointF(0, 0));
  }

  void GraphicsViewV0::init() {

      // variables
      m_scale = 1.0;
      m_pos = QPoint(0, 0);

      // signal
      connect(horizontalScrollBar(), &QScrollBar::valueChanged, [=](int value) {
          m_pos.setX(value);
         
          });

      connect(verticalScrollBar(), &QScrollBar::valueChanged, [=](int value) {
          m_pos.setY(value);
         
          });

      // state
      setStyleSheet("border:none;background-color: rgb(22, 22, 22);");
      ///verticalScrollBar()->setMaximumWidth(5);
      ///horizontalScrollBar()->setMaximumHeight(5);

      horizontalScrollBar()->setStyleSheet("QScrollBar{height:10px;}");
      verticalScrollBar()->setStyleSheet("QScrollBar{width: 10px;}");

      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  }
} // namespace control_point_GUI

