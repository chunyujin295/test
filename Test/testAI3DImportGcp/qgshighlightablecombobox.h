/***************************************************************************
    qgshighlightablecombobox.h
     -------------------------
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

#ifndef QGSHIGHLIGHTABLECOMBOBOX_H
#define QGSHIGHLIGHTABLECOMBOBOX_H

#include <QComboBox>
#include <QTreeWidget>
#include <QTreeView>
#include <QMouseEvent>
#include "qgscoordinatereferencesystem.h"
#include "QComboxTree.h"

#define SIP_NO_FILE

//样式表https://blog.csdn.net/qq_41673920/article/details/93495716
//class QTreeComboBoxView : public QTreeWidget
//{
//    Q_OBJECT
//public:
//    QTreeComboBoxView(QWidget* parent = Q_NULLPTR) :QTreeWidget(parent) {}
//    ~QTreeComboBoxView() {};
//protected:
//    void mousePressEvent(QMouseEvent* event) override;
//signals:
//    void treeMousePressed(bool inItem);
//};



/**
 * \class QgsHighlightableComboBox
 * \ingroup gui
 *
 * \brief A QComboBox subclass with the ability to "highlight" the edges of the widget.
 *
 * \since QGIS 3.12
 */
class  QgsHighlightableComboBox : public QComboBox
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsHighlightableComboBox with the specified parent widget.
     */
    QgsHighlightableComboBox( QWidget *parent = nullptr );

    /**
     * Returns TRUE if the combo box is currently highlighted.
     * \see setHighlighted()
     */
    bool isHighlighted() const { return mHighlight; }

    /**
     * Sets whether the combo box is currently \a highlighted.
     * \see isHighlighted()
     */
    void setHighlighted( bool highlighted );

  protected:
    void paintEvent( QPaintEvent *e ) override;
protected:
    void hidePopup() override;
private:
    bool canHidePopup = true;//允许收缩
public:
    QTreeComboBoxView* displayTreeWidget = nullptr;
  private:

    bool mHighlight = false;

};


#endif // QGSHIGHLIGHTABLECOMBOBOX_H
