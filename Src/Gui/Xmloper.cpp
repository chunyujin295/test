#include "Gui/Xmloper.h"
#include <QFile>
#include <QTextStream>
#include "Util/TaskProcess.h"

XmlOper::XmlOper()
{
}

bool XmlOper::readRecentOpen(const QString& strFile, QStringList& recentProjectList, QString& errText)
{
    QFile file(strFile);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        errText = QObject::tr("File open failed");
        return false;
    }

    QString strErr;
    int errLine = -1;
    int errColumn = -1;
    QDomDocument doc;
    if (!doc.setContent(&file, false, &strErr, &errLine, &errColumn))
    {
        errText = QString("%1%2%3").arg(errLine).arg(errLine).arg(errColumn);
        return false;
    }

    QDomElement root = doc.documentElement();

    if (root.tagName().compare("RecentOpen", Qt::CaseInsensitive))
    {
        errText = QObject::tr("Can't find RecentOpen tag");
        return false;
    }
    QDomNode child = root.firstChild();
    while (!child.isNull())
    {
        if (!child.toElement().tagName().compare("Project", Qt::CaseInsensitive))
        {

            recentProjectList.push_back(child.toElement().text());

        }
        child = child.nextSibling();
    }

    file.close();
    return true;
}

bool XmlOper::writeRecentOpen(const QString& strFile, const QStringList& recentProjectList, QString& errText)
{
    QFile file(strFile);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        errText = QObject::tr("writeRecentOpen::file open failed");
        return false;
    }
    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);

    //append root element
    QDomElement root = doc.createElement("RecentOpen");
    doc.appendChild(root);

    for (auto projectPath : recentProjectList)
    {
        //append project
        QDomElement element = doc.createElement("Project");
        QDomText elementText;

         elementText = doc.createTextNode(projectPath);

        element.appendChild(elementText);
        root.appendChild(element);
    }

    QTextStream outStream(&file);
    doc.save(outStream, 4);

    file.close();
    return true;
}
