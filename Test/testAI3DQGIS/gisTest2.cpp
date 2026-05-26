#include "gisTest2.h"
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QgsVectorLayer.h>
#include <QMessageBox>
gisTest2::gisTest2(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(600, 400);
    QMenu* fileMenu = new QMenu("file");
    // create the menus and then add the actions to them.
  // QMenu fileMenu = this->menuBar()->addMenu("File");
    QMenuBar* mbar = new QMenuBar();
    mbar->addMenu(fileMenu);
    openFileAction = new QAction("Open", this);
    this->connect(openFileAction, SIGNAL(triggered(bool)), this, SLOT(on_openFileAction_triggered()));
    fileMenu->addAction(openFileAction);

    // initialize the map canvas
    mapCanvas = new QgsMapCanvas();
    this->setCentralWidget(mapCanvas);

    mapCanvas->setCanvasColor(QColor(255, 255, 255));
    mapCanvas->setVisible(true);
    mapCanvas->enableAntiAliasing(true);

}

void gisTest2::on_openFileAction_triggered() {
    addVectorLayer();
}

void gisTest2::addVectorLayer()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open shape file"), "", "*.shp");
    QStringList temp = fileName.split('/');
    QString basename = temp.at(temp.size() - 1);
    QgsVectorLayer* vecLayer = new QgsVectorLayer(fileName, basename, "ogr");

    if (!vecLayer->isValid())
    {
        QMessageBox::critical(this, "error", QString("layer is invalid: \n") + fileName);
        return;
    }
    mapCanvas->setExtent(vecLayer->extent());
    layers.append(vecLayer);
    mapCanvas->setLayers(layers);
    mapCanvas->refresh();
}
