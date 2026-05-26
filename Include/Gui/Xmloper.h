#ifndef CXMLOPER_H
#define CXMLOPER_H

#include <QDomElement>
#include <QString>
#include <memory>
#include <QObject>
#include <QStringList>

class XmlOper
{
public:
    XmlOper();
    bool readRecentOpen(const QString &strFile,  QStringList& recentProjectList, QString& errText);
    bool writeRecentOpen(const QString &strFile, const QStringList& recentProjectList, QString& errText);

};

#endif // CXMLOPER_H
