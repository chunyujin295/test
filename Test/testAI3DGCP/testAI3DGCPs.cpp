
#include <QString>
#include <QSet>
#include <QMap>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QApplication>
#include <iostream>
#include <fstream>
#include <set>
//需要测试的几项
// 测试排序。测试删除后id
 


#include <stdio.h>
#include <proj.h>


#include <sqlite3.h>
QString  ogcWmsCrsFilterAsSqlExpression(QSet<QString>* crsFilter)
{
    QString sqlExpression = QStringLiteral("1");           // it's "SQL" for "true"
    QMap<QString, QStringList> authParts;

    if (!crsFilter)
        return sqlExpression;

    /*
       Ref: WMS 1.3.0, section 6.7.3 "Layer CRS":

       Every Layer CRS has an identifier that is a character string. Two types of
       Layer CRS identifiers are permitted: "label" and "URL" identifiers:

       Label: The identifier includes a namespace prefix, a colon, a numeric or
          string code, and in some instances a comma followed by additional
          parameters. This International Standard defines three namespaces:
          CRS, EpsgCrsId and AUTO2 [...]

       URL: The identifier is a fully-qualified Uniform Resource Locator that
          references a publicly-accessible file containing a definition of the CRS
          that is compliant with ISO 19111.
    */

    // iterate through all incoming CRSs

    const auto authIds{ *crsFilter };
    for (const QString& auth_id : authIds)
    {
        QStringList parts = auth_id.split(':');

        if (parts.size() < 2)
            continue;

        authParts[parts.at(0).toUpper()].append(parts.at(1).toUpper());
    }

    if (authParts.isEmpty())
        return sqlExpression;

    if (!authParts.isEmpty())
    {
        QString prefix = QStringLiteral(" AND (");
        for (auto it = authParts.constBegin(); it != authParts.constEnd(); ++it)
        {
            sqlExpression += QStringLiteral("%1(upper(auth_name)='%2' AND upper(auth_id) IN ('%3'))")
                .arg(prefix,
                    it.key(),
                    it.value().join(QLatin1String("','")));
            prefix = QStringLiteral(" OR ");
        }
        sqlExpression += ')';
    }

   // Q("exiting with '" + sqlExpression + "'.", 4);

    return sqlExpression;
}

#include "qgsprojectionselector.h"
#include "qgsgenericprojectionSelector.h"
int main(int argc, char** argv)
{
   

    //std::string path = "D:/QGIS/qgis-latest-ltr.tar/qgis-3.16.12/resources/srs6.db";// "D:/Code/ThirdParty/third_party/Windows/vc142/proj/data/proj.db";
    QApplication a1(argc, argv);
    QgsGenericProjectionSelector w;


    w.show();
    w.exec();
    return 0;
}