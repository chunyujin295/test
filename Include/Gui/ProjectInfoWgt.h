/**
  * @file      Reconstration.h
  * @brief     ProjectInfoWgt工程界面类
  * @details
  * @author    李跃
  * @attention
  */

#ifndef CPROJECTWGT_H
#define CPROJECTWGT_H

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include "qthread.h"
#include "ui_ProjectInfoWgt.h"
#include "Core/types.h"

#include <unordered_map> 

#include <map>
namespace AI3D
{
	namespace GUI
	{
		//#include "httpget.h

		class ProjectInfoWgt : public QWidget
		{
			Q_OBJECT
		public:
			explicit ProjectInfoWgt(QString proname, QString propath, QWidget* parent = 0);
			
			~ProjectInfoWgt();
			QString GetProjectName() { return  pro_name_; };
			void SetProjectInfo(std::unordered_map<std::string,std::vector<int>> projinfo);
		public:

			void ShowProjectPath(QString str);
			void ShowProjectName(QString str);

			// 更新block相关的界面元素icon
			void Update_Block_Info();
			// alke
			QString GetProjectPath();
			
		signals:
			// void setPostData_set_jobqueue(QString,QByteArray,QStringList);
		protected slots:
			// 

			//void slot_httppostError(QString);
			//void slot_httpPostDataComplete(QString, QByteArray);
			// alka 1.4.231
			void Slot_Read_Block_Info_Time();
			void OnBlockIconDoubleClicked(QTableWidgetItem* item);
			// alke
		private:
			//void setPostHttpSetJobqueue();
			// alka 1.4.231
			void Init_Widget();
			
			// alke

		private:
			QString pro_path_;
			QString pro_name_;


		private:
			Ui::CProjectInfoWgt* ui;
			QString projPath;
			// alka 1.4.231
			QTimer* read_block_info_time;
			// alke
		};

	}
}
//Q_DECLARE_METATYPE(ProjectInfoWgt)
//Q_DECLARE_METATYPE(ProjectInfoWgt*)
#endif // CPROJECTWGT_H
