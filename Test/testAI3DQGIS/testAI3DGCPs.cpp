

//#include "gisTest2.h"
#include <qgsapplication.h>
#include <qgsprojectionselectiondialog.h>
int main(int argc, char* argv[])
{
  

    QgsApplication a(argc, argv, true);
    QgsApplication::setPrefixPath(/*"C:/OSGeo4W/apps/qgis"*/"K:/QGIS_here/OSGeo4W/apps/qgis-ltr", true);
    QgsApplication::initQgis();    //初始化QGIS应用
    //gisTest2 ;    //创建一个窗体，类似于Qt
    QgsProjectionSelectionDialog w;
   
    w.show();

    return a.exec();
}
