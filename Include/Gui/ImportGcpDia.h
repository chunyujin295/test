#ifndef _AI3D_GUI_IMPORTGCP_H
#define _AI3D_GUI_IMPORTGCP_H


#include <QDialog>
#include<QStandardItemModel>
#include "ui_ImportGcpWgt.h"
#include "Gui/QSensorIntDelegate.h"

namespace AI3D
{
	namespace GUI
	{
		class ImportGcpDia : public QDialog
		{
			Q_OBJECT

		public:
			ImportGcpDia(QDialog* parent = Q_NULLPTR);
			~ImportGcpDia();
			void InitSrss(bool bSetCurrentItem4Recent = false);
			void InitTableTitleList();
			//弹出打开文件对话框
			void openFileDialog();
			void setOldFileName(QString strFile);
			QString getFilePath();
			bool showList(QString fileName);
			void showTitle(int num);
			void recordIndex(int index);
			QMap<int, QString> getRecordData() { return m_RecordMap; };
			void ReadFile();
			QList<QStringList> getPosList() { return vlist; };
			QString GetSrsName();

		signals:
			//void sig_VecString(QVector<QStringList> );
			void sign_posFileReadError();

		public slots:
			void Slot_SrsItemChanged(QString srsname);
			void Slot_SrsSelected(QString& srs);
			void Slot_SrsRestore();

		private:
			Ui::ImportGcpWgt ui;
			QString oldFileName;
			QString posFile_path;
			QSensorIntDelegate* comboxSelf;
			QList<QComboBox*> p_List;
			QMap<int, QString> m_RecordMap;
			QStandardItemModel* model;
			QList<QStringList> vlist;
			QStringList comboxitemlist;
			QStringList comboxitemlist_display;
			QString previous_srs;
		};
	}
}
#endif