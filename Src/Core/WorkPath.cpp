#include "Core/WorkPath.h"
#include "Util/TaskProcess.h"


const char* GetWorkPath() {
	
	
	static std::string s_workPath;
	QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	QString m_rootPath = appDataPath + "/Local/MoldAI";

	
	if (m_rootPath.isEmpty() || !QDir(m_rootPath).exists()) {
		QString userName = qEnvironmentVariable("USERNAME");
		if (userName.isEmpty()) {
			userName = qEnvironmentVariable("USER");
		}
		m_rootPath = QString("C:/Users/%1/AppData/Local/MoldAI").arg(userName);
	}
	s_workPath = qstr2str(m_rootPath);
	return s_workPath.c_str();
}