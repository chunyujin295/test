#ifndef _AI3D_GUI_BATCHPREPARE_H
#define _AI3D_GUI_BATCHPREPARE_H


#include <QDialog>
#include <QStandardItemModel>
#include "ui_BatchPrepareWgt.h"
#include "Gui/QSensorIntDelegate.h"
#include "Core/TaskDef.h"

namespace AI3D
{
	namespace GUI
	{
		class BatchPrepareDia : public QDialog
		{
			Q_OBJECT

		public:
			BatchPrepareDia(QDialog* parent = Q_NULLPTR);
			~BatchPrepareDia();
			void InitSrss();
			//弹出打开文件对话框
			void OpenPosFileDialog();
			void OpenGCPFileDialog();
			void OpenImageDirDialog();
			void setOldFileName(QString strFile);
			
			bool showList(QString fileName);
			void showTitle(int num);
			void recordIndex(int index);

			std::string GetImagePath();
			std::string GetPosFile();
			std::string GetPrefix();
			std::string GetNumLength();
			std::string GetStartNum();
			std::string GetGCPFile();
			preparetaskinfo_s  GetParams();
			QMap<int, QString> getRecordData() { return m_RecordMap; };
			
			QList<QStringList> getPosList() { return vlist; };
			QString GetSrsName();
		public slots:
			void Slot_SrsItemChanged();
			void Slot_GetParam();
		signals:
			//void sig_VecString(QVector<QStringList> );
			void sign_posFileReadError();
		private:
			Ui::BatchPrepareWgt ui;
			QString oldFileName;
			/*QString posFile_path;*/
			QSensorIntDelegate* comboxSelf;
			QList<QComboBox*> p_List;
			QMap<int, QString> m_RecordMap;
			QStandardItemModel* model;
			QList<QStringList> vlist;
			QStringList comboxitemlist;
			preparetaskinfo_s params_;
		};
	}
}
#endif