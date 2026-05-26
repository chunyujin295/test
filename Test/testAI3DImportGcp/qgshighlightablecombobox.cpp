/***************************************************************************
    qgshighlightablecombobox.cpp
     ---------------------------
    Date                 : 20/12/2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshighlightablecombobox.h"
#include <QPainter>




//void QTreeComboBoxView::mousePressEvent(QMouseEvent* event)
//{
//
//    auto curIndex = currentIndex();
//    auto rect = this->visualRect(curIndex);
//    auto buttonRect = QRect(rect.left() - 20, rect.top(), 20, rect.height());
//
//    if (buttonRect.contains(event->pos()))
//    {
//       /* if (isExpanded(curIndex)) 
//            setExpanded(curIndex, false);
//        else */
//            setExpanded(curIndex, true);
//        emit treeMousePressed(true);
//    }
//    else
//        emit treeMousePressed(false);
//}
QgsHighlightableComboBox::QgsHighlightableComboBox( QWidget *parent )
  : QComboBox( parent )
{

   
}

void QgsHighlightableComboBox::hidePopup()
{
    if (canHidePopup) QComboBox::hidePopup();
}
void QgsHighlightableComboBox::paintEvent( QPaintEvent *e )
{
  QComboBox::paintEvent( e );
  if ( mHighlight )
  {
    QPainter p( this );
    int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}

void QgsHighlightableComboBox::setHighlighted( bool highlighted )
{
  mHighlight = highlighted;
  update();
}

