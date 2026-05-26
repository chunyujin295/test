#pragma once
#include <qwidget.h>

#include <QPushButton>
#include <QResizeEvent>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCloseEvent>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLCDNumber>
#include <QTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QCheckBox>


#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QListWidget>
#include <QResizeEvent>

#include <QSortFilterProxyModel>
#include <qdatetime.h>
#include <qtimer.h>

extern bool bMainWindowDestroyed;

extern bool checkVersionFromFile(QString& fileName, int& _versionCode, QString& _versionName);

extern int currentVersionCode;
extern QString currentVersionName;


