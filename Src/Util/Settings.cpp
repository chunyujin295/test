#include "Util/Settings.h"
#include <QFileDialog>
#include <QMessageBox>

#include <QDateTime>
#include <sstream>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPixmapCache>

#include <iostream>
#include <QToolTip>

#include <QBitmap>
#include <QScrollBar>
#include <QtConcurrent>

#include <QSet>
#include <QHash>
#include <QCryptographicHash>
#include "Core/Application.h"
#include <QStandardPaths>

#include "Windows.h"

QString progFileName;

QSettings* Settings::pSettings = new QSettings("HKEY_CURRENT_USER\\Software\\MoldAI\\JobQueues", QSettings::NativeFormat);

QString Settings::getMasterJobQueue()
{
	if (!pSettings)
		return "";

	return pSettings->value("master", "").toString();
}

QString Settings::getEngineJobQueue()
{
	if (!pSettings)
		return "";

	return pSettings->value("engine", "").toString();
}



bool Settings::isEngine()
{
	
	if ( progFileName.indexOf("MoldAINode", Qt::CaseInsensitive) >= 0)
		return true;

	return false;
}


static QMutex mutex;
void customMessageHandler(QtMsgType type,
	const QMessageLogContext& context,
	const QString& msg)
{
	
	QDateTime _datetime = QDateTime::currentDateTime();
	
	QString szDate = _datetime.toString("yyyy-MM-dd hh:mm:ss");
	QString txt("[" + szDate + "] ");

	switch (type)
	{
	case QtDebugMsg:
	{
		
		break;
	}
	case QtInfoMsg:
	{
		
		break;
	}
	case QtWarningMsg:
	{
		
		break;
	}
	case QtCriticalMsg:
	{
		
		break;
	}
	case QtFatalMsg:
	{
		
		
		break;
	}
	default:
	{
		
		break;
	}
	}

	txt.append(msg);

	mutex.lock();
	
	
	QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	QString m_rootPath = appDataPath + "/Local/MoldAI";

	
	if (m_rootPath.isEmpty() || !QDir(m_rootPath).exists()) {
		QString userName = qEnvironmentVariable("USERNAME");
		if (userName.isEmpty()) {
			userName = qEnvironmentVariable("USER");
		}
		m_rootPath = QString("C:/Users/%1/AppData/Local/MoldAI").arg(userName);
	}
	QString m_logsPath = m_rootPath + "/logs";
	QString logfile = m_logsPath + "/mlog.txt";
	

	QFile file(logfile);
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << txt << "\r\n";
	file.close();
#ifdef  USE_WIN_DEBUG
	
#endif
	mutex.unlock();
}