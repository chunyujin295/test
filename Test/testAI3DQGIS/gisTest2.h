#pragma once
#include <QMainWindow>
#include <QList>
#include <qgsMapCanvas.h>
//#include "ui_gisTest2.h"



class gisTest2 : public QMainWindow
{
    Q_OBJECT

public:
    gisTest2(QWidget *parent = Q_NULLPTR);

private:
    // create the menus and then add the actions to them.
    QMenu *fileMenu;
    QAction *openFileAction;

    //map canvas
    QgsMapCanvas *mapCanvas;
    QList<QgsMapLayer*> layers;

    public slots:
    void on_openFileAction_triggered();
    //

public:
    void addVectorLayer();

};
