// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Gui/ControlPoint2DNode.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>



ControlPoint2DNode::ControlPoint2DNode
(
  const QPointF& pos,
  double & x,
  double & y,
  size_t id_control_point
):
  _id_control_point(id_control_point),
  _x(x),
  _y(y)
{

}
ControlPoint2DNode::ControlPoint2DNode():
	_id_control_point(0),
	_x(0.0f),
	_y(0.0f)
{
	//setFlags(ItemIsMovable | ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIgnoresTransformations);
	setFlag(ItemSendsGeometryChanges);
	setCacheMode(DeviceCoordinateCache);
	setZValue(1.);
	//setPos(pos);
}
size_t ControlPoint2DNode::controlPointId() const
{
  return _id_control_point;
}

QRectF ControlPoint2DNode::boundingRect() const
{
  return QRectF(QPointF(-52.5,-52.5),QSize(105,105));
}

QPainterPath ControlPoint2DNode::shape() const
{
 /*  QPainterPath path ;
   int x=0, y=0, w=50, h=5, step = 3;

   path.addRect(x-w-h/2-step,y-h/2,w,h);
   path.addRect(x-h/2,y+h/2+step,h,w);
   path.addRect(x+h/2+step,y-h/2,w,h);
   path.addRect(x-h/2,y-h/2-w-step,h,w);*/


	return paintCross();
}
QPainterPath ControlPoint2DNode:: paintCross(int x,int y,int w,int h,int step ) const 
{

    QPainterPath path;
    path.addRect(x-w-h/2-step,y-h/2,w,h);
    path.addRect(x-h/2,y+h/2+step,h,w);
    path.addRect(x+h/2+step,y-h/2,w,h);
    path.addRect(x-h/2,y-h/2-w-step,h,w);

    return path;

}
void ControlPoint2DNode::paint(  QPainter *painter,  const QStyleOptionGraphicsItem *option,  QWidget *widget)
{
  QRadialGradient gradient(-3, -3, 10);
  if (isSelected()){
    gradient.setCenter(3, 3);
    gradient.setFocalPoint(3, 3);
    gradient.setColorAt(1, QColor(Qt::red).light(120));
   // gradient.setColorAt(0, QColor(Qt::darkRed).light(120));
  }
  else{
    gradient.setColorAt(0, Qt::red);
    //gradient.setColorAt(1, Qt::darkRed);
  }
  painter->setBrush(gradient);
  painter->setPen(QPen(Qt::red, 0));


  painter->drawPath(paintCross());

 
}

QVariant ControlPoint2DNode::itemChange(GraphicsItemChange change,const QVariant &value)
{
  const QVariant variant = QGraphicsItem::itemChange(change, value);
  _x = scenePos().x();
  _y = scenePos().y();
  return variant;
}

void ControlPoint2DNode::mousePressEvent
(
  QGraphicsSceneMouseEvent *event
)
{
  update();
  QGraphicsItem::mousePressEvent(event);
}
void ControlPoint2DNode::mouseReleaseEvent
(
  QGraphicsSceneMouseEvent *event
)
{
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

