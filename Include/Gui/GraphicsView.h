// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef GRAPHICSVIEW_HPP
#define GRAPHICSVIEW_HPP

// Qt4 headers
#include <QMainWindow>
#include <QPointer>
#include <QGraphicsView>


//class QAction;
class QGraphicsPixmapItem;
class QGraphicsScene;
class QLabel;
class QScrollArea;
class QMenu;
class QGraphicsLineItem;
class QGraphicsRectItem;
class QGraphicsItem;
class ControlPoint2DNode;
class ControlPointCircleCross;

namespace control_point_GUI {

  class GraphicsView : public QGraphicsView
  {
      Q_OBJECT // mandatory for signals and slots

  private: /* static const variables */

  public: /* methods */
      struct pointQPoint
      {
          QPointF p1;
          QPointF p2;
      };
      GraphicsView(QWidget* parent = 0);

      void addImage(const QString& qs_filename, int controlID, float xpos = 0.f, float ypos = 0.f, bool bClear = false);
      void addImageV2(const QString& qs_filename, int controlID, float xpos = 0.f, float ypos = 0.f, bool bClear = false);

      void AddLine(const QPointF& pt1, const QPointF& pt2);
      void HideLine();
      void ClearLine();
      void ShowLine();
      void addNode(QPointF pointf);
      void hideNode();
      void addCircleCross(QPointF pointf);
      void clear();
      // void setCurrentViewId(const openMVG::IndexT index) {_current_view_id = index;}
      void centerAt(QPointF pointf);
      void keepPreviousPos();

      virtual void resizeEvent(QResizeEvent* event);

  protected: /* methods */
      void drawBackground(QPainter* painter, const QRectF& rect);

      void mousePressEvent(QMouseEvent* e);
      void mouseReleaseEvent(QMouseEvent* e);
      void mouseMoveEvent(QMouseEvent* e);

      void keyPressEvent(QKeyEvent* event);
      void keyReleaseEvent(QKeyEvent* event);
      void wheelEvent(QWheelEvent* event);

      void zoom(qreal factor, QPointF centerPoint);

  public slots:

      void removeControlPoint();
  private slots:

      void zoomIn();
      void zoomOut();
      void normalSize();

      // alka action info
  public:
      QString getImageName() const;
      QPoint getOffset() const;
      double getScale() const;
      void setOffset(const QPoint& pos);
      void setScale(const double& value);
      void setScale2(const double& value);
      void dumpInfo();

  private slots:
      void init();
  private:
      double m_scale;
      QPoint m_pos;
      // alke

  private: /* methods */
    // Interface construction methods
      void createActions();

  private: /* data members */
    // The graphics view machinery.
      QGraphicsScene* scene;
      QGraphicsPixmapItem* image;
      // Action
      QAction* open_images_action_;
      QAction* zoomInAct;
      QAction* zoomOutAct;
      QAction* normalSizeAct;
      QAction* removeControlPointAct;
      bool mMoveStart;
      bool mContinuousMove;
      QPoint mMousePoint;
      QPointF posCross;
      QString pictureName;
      ControlPoint2DNode* redCrossItem;
      ControlPointCircleCross* CircleCross;
      QList<QGraphicsLineItem*> Lines;
      int imageid = -1;
      int gcpid = -1;
      bool bFirstImage = true;

      bool bExistsNode = false;
      bool bExistsCircleCross = false;
      QPointF pointNode;
      QPointF pointCircleCross;

      qreal previousImageXoff;
      qreal previousImageYoff;
      QPixmap pixmap;

      double savedFactor;
      double originalScale;

  signals:
      void PosPoint(int imageid, QPointF point);
      // Document
      // Document & _doc;
      // Current viewed image id
      //openMVG::IndexT _current_view_id;
      void ChangeScale(double scale);
      void MousePoint(QPointF point);
  };

  //不用 old version
  class GraphicsViewV0 : public QGraphicsView
  {
      Q_OBJECT // mandatory for signals and slots

  private: /* static const variables */

  public: /* methods */
      struct pointQPoint
      {
          QPointF p1;
          QPointF p2;
      };
      GraphicsViewV0(QWidget* parent = 0);

      void addImage(const QString& qs_filename, int controlID, float xpos = 0.f, float ypos = 0.f, bool bClear = false);
      void addImageV2(const QString& qs_filename, int controlID, float xpos = 0.f, float ypos = 0.f, bool bClear = false);

      void AddLine(const QPointF& pt1, const QPointF& pt2);
      void HideLine();
      void ClearLine();
      void ShowLine();
      void addNode(QPointF pointf);
      void hideNode();
      void addCircleCross(QPointF pointf);
      void clear();
      // void setCurrentViewId(const openMVG::IndexT index) {_current_view_id = index;}
      void centerAt(QPointF pointf);
      void keepPreviousPos();

      virtual void resizeEvent(QResizeEvent* event);

  protected: /* methods */
      void drawBackground(QPainter* painter, const QRectF& rect);

      void mousePressEvent(QMouseEvent* e);
      void mouseReleaseEvent(QMouseEvent* e);
      void mouseMoveEvent(QMouseEvent* e);

      void keyPressEvent(QKeyEvent* event);
      void keyReleaseEvent(QKeyEvent* event);
      void wheelEvent(QWheelEvent* event);

      void zoom(qreal factor, QPointF centerPoint);

  public slots:

      void removeControlPoint();
  private slots:

      void zoomIn();
      void zoomOut();
      void normalSize();

      // alka action info
  public:
      QString getImageName() const;
      QPoint getOffset() const;
      double getScale() const;
      void setOffset(const QPoint& pos);
      void setScale(const double& value);
      void setScale2(const double& value);
  private slots:
      void init();
  private:
      double m_scale;
      QPoint m_pos;
      // alke

  private: /* methods */
    // Interface construction methods
      void createActions();

  private: /* data members */
    // The graphics view machinery.
      QGraphicsScene* scene;
      QGraphicsPixmapItem* image;
      // Action
      QAction* open_images_action_;
      QAction* zoomInAct;
      QAction* zoomOutAct;
      QAction* normalSizeAct;
      QAction* removeControlPointAct;
      bool mMoveStart;
      bool mContinuousMove;
      QPoint mMousePoint;
      QPointF posCross;
      QString pixtureName;
      ControlPoint2DNode* redCrossItem;
      ControlPointCircleCross* CircleCross;
      QList<QGraphicsLineItem*> Lines;
      int imageid = -1;
      int gcpid = -1;
      bool bFirstImage = true;

      bool bExistsNode = false;
      bool bExistsCircleCross = false;
      QPointF pointNode;
      QPointF pointCircleCross;

      qreal previousImageXoff;
      qreal previousImageYoff;

  signals:
      void PosPoint(int imageid, QPointF point);
      // Document
      // Document & _doc;
      // Current viewed image id
      //openMVG::IndexT _current_view_id;
      void ChangeScale(double scale);
      void MousePoint(QPointF point);
  };

} // namespace control_point_GUI

#endif /* GRAPHICSVIEW_HPP */
